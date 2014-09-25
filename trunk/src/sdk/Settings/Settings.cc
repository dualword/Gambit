/*
    Written by Jelle Geerts (jellegeerts@gmail.com).

    To the extent possible under law, the author(s) have dedicated all
    copyright and related and neighboring rights to this software to
    the public domain worldwide. This software is distributed without
    any warranty.

    You should have received a copy of the CC0 Public Domain Dedication
    along with this software.
    If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include "Settings.hh"
#include "Core/GeneralException.hh"
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <sstream>
#include <vector>

#ifdef _WIN32
# define CFG_FILE_OFLAG_BASE O_CREAT | O_BINARY
# define CFG_FILE_MODE S_IRUSR | S_IWUSR
#else /* !defined(_WIN32) */
# define CFG_FILE_OFLAG_BASE O_CREAT
# define CFG_FILE_MODE S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
#endif /* !defined(_WIN32) */

static std::string boolToString(bool value)
{
    return value ? "true" : "false";
}

static bool stringToBool(const std::string &value, bool defaultValue)
{
    if (value == "true")
        return true;
    else if (value == "false")
        return false;
    else
        return defaultValue;
}

static std::string intToString(int value)
{
    std::stringstream stream;
    stream << value;
    return stream.str();
}

static int stringToInt(const std::string &value, int defaultValue)
{
    std::stringstream stream(value);
    int i;
    if (stream >> i)
        return i;
    else
        return defaultValue;
}

Settings::Settings(ISettingsGlue &_glue, const std::string &_fileName)
    : glue(_glue), fileName(_fileName), fd(-1), pendingFlush(false), readOnly(false)
{
    glue.setSettings(this);

#ifdef _WIN32
    reopen(false /* For reading. */);
#else /* !defined(_WIN32) */
    fd = open(fileName.c_str(), CFG_FILE_OFLAG_BASE | O_RDWR, CFG_FILE_MODE);
    if (fd < 0)
    {
        (void)(fd = open(fileName.c_str(), CFG_FILE_OFLAG_BASE | O_RDONLY, CFG_FILE_MODE));
        readOnly = true;
    }
#endif /* !defined(_WIN32) */

    readAndMerge();
}

Settings::~Settings()
{
    flush();

    close();
}

bool Settings::getBool(const std::string &key, bool defaultValue) const
{
    const std::string defaultValueStr = boolToString(defaultValue);
    const std::string value = get(key, defaultValueStr);
    return stringToBool(value, defaultValue);
}

void Settings::setBool(const std::string &key, bool value)
{
    return set(key, boolToString(value));
}

int Settings::getInt(const std::string &key, int defaultValue) const
{
    const std::string defaultValueStr = intToString(defaultValue);
    const std::string value = get(key, defaultValueStr);
    return stringToInt(value, defaultValue);
}

void Settings::setInt(const std::string &key, int value)
{
    return set(key, intToString(value));
}

std::string Settings::getString(const std::string &key, const std::string &defaultValue) const
{
    return get(key, defaultValue);
}

void Settings::setString(const std::string &key, const std::string &value)
{
    set(key, value);
}

void Settings::remove(const std::string &key)
{
    if (settings.remove(key))
    {
        removedKeys.insert(key);

        pendingFlush = true;
        requestUpdate();
    }
}

void Settings::updateAndFlush()
{
    // In case we were called by someone other than SettingsGlue, then we should cancel the
    // scheduled updateAndFlush() call, to avoid superfluous I/O.
    glue.cancelScheduledUpdateAndFlush();

    if (fd < 0)
        return;

    /* TODO:
    if (readOnly && file has _not_ changed since last read)
    {
        // There is no new data to be read, and we can't write, so we're done.
        return;
    }
*/

    // TODO: make this work
#if 0
    bool doRead = false;

    if (file has changed since last read)
        doRead = true;
    else
    {
        // Simply use the in-memory file contents (leave them as-is).
    }

    if (doRead)
#endif
        readAndMerge();

    flush();
}

std::string Settings::get(const std::string &key, const std::string &defaultValue) const
{
    return settings.get(key, defaultValue);
}

void Settings::set(const std::string &key, const std::string &value)
{
    settings.set(key, value);

    pendingFlush = true;
    requestUpdate();
}

void Settings::close()
{
    if (fd < 0)
        return;

    ::close(fd);
    fd = -1;
}

void Settings::flush()
{
    if (pendingFlush)
    {
        printf("Settings: flushed to storage.\n");
        write();
        sync();
        pendingFlush = false;
    }
}

void Settings::sync() const
{
    if (fd < 0 || readOnly)
        return;

#ifdef _WIN32
    (void)_commit(fd);
#else /* !defined(_WIN32) */
    (void)::fsync(fd);
#endif /* !defined(_WIN32) */
}

SettingsContainer Settings::merge(const SettingsContainer &settingsFromFile, const SettingsContainer &newSettings) const
{
#ifdef DEBUG
    if (fd < 0)
    {
        // If we don't have a file descriptor, the passed 'settingsFromFile'
        // are most likely empty and merging is useless (hence later on we
        // check whether the 'settingsFromFile' are empty and simply return
        // 'newSettings' in that case).
        printf("SettingsContainer: merge() called but we don't have a file descriptor.\n");
    }
#endif

    SettingsContainer mergedSettings = settingsFromFile;
    std::vector<std::string> mergedKeys;

    if (newSettings.str().length() == 0)
        return settingsFromFile;
    else if (settingsFromFile.str().length() == 0)
        return newSettings;

    SettingsContainer::const_iterator it = newSettings.constBegin(),
        end = newSettings.constEnd();
    for ( ; it != end; ++it)
    {
        std::vector<std::string>::const_iterator _begin = mergedKeys.begin(),
            _end = mergedKeys.end();
        if (std::find(_begin, _end, it->key()) != _end)
        {
            // Ignore duplicates, only the first key should be used, otherwise
            // we'd overwrite a key that was already set, which could lead to
            // problems, since the SettingsContainer::set() function only
            // updates the first key (by design), meaning that if one used the
            // Preferences dialog to change a setting, one would lose the
            // changed setting if the last occurrence (a duplicate) of the key
            // had a different value.
            printf("Settings: ignoring duplicate key '%s' (value=[%s]).\n",
                    it->key().c_str(), it->value().c_str());
        }
        else
        {
            // NOTE:
            // Do _not_ first remove duplicates (if any), leave them as-is.
            // Removing them is considered data loss, and we don't want that.

            mergedSettings.set(it->key(), it->value());
            mergedKeys.push_back(it->key());
        }
    }

    // Any keys removed at run-time should be removed from the settings file as well.
    removed_keys_type::const_iterator keysIt = removedKeys.begin(),
        keysEnd = removedKeys.end();
    for ( ; keysIt != keysEnd; ++keysIt)
    {
        mergedSettings.remove(*keysIt);
    }

    return mergedSettings;
}

void Settings::requestUpdate()
{
    // We schedule a call to updateAndFlush(), such that users of our class can
    // call the set() function multiple times without flushing to storage
    // multiple times, which would cause unnecessary delays. This way, we will
    // flush to storage only once, even if set() is called multiple times.
    glue.scheduleUpdateAndFlush();
}

std::string Settings::read()
{
    std::string contents;
    ssize_t bytesRead;

    if (fd < 0)
        return contents;

    struct stat stat;
    if (fstat(fd, &stat) < 0)
        return contents;

    char *data = new char[stat.st_size + 1];

    if (lseek(fd, 0, SEEK_SET) < 0)
        goto done;

    bytesRead = ::read(fd, data, stat.st_size);
    if (bytesRead < 0 || bytesRead != stat.st_size)
        contents = "";
    else
    {
        data[stat.st_size] = '\0';
        contents = data;
    }

done:
    delete[] data;
    return contents;
}

void Settings::readAndMerge()
{
    if (fd < 0)
        return;

    SettingsContainer settingsFromFile = read();
    settings = merge(settingsFromFile, settings);
}

#ifdef _WIN32
void Settings::reopen(bool readOrWrite /* false = read-only, true = write-only */)
{
    /*
     * NOTE:
     * On Win32, we use CreateFile() so other processes can also access the
     * file, which with open() wouldn't be possible. Also, after using
     * _open_osfhandle(), only close() or CloseHandle() has to be used to close
     * the file handle, but *not* both.
     */

    close();

    DWORD desiredAccess = readOrWrite ? GENERIC_WRITE : GENERIC_READ;
    HANDLE hFile = CreateFile(fileName.c_str(), desiredAccess,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0,
        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile == INVALID_HANDLE_VALUE)
        fd = -1;
    else
        fd = _open_osfhandle(reinterpret_cast<intptr_t>(hFile), 0);
}
#endif /* defined(_WIN32) */

void Settings::write()
{
    if (fd < 0 || readOnly)
        return;

#ifdef _WIN32
    reopen(true /* For writing. */);
#endif /* defined(_WIN32) */

    if (lseek(fd, 0, SEEK_SET) < 0)
        goto done;

    if (::write(fd, settings, settings.str().length()) < 0)
        goto done;

    if (ftruncate(fd, settings.str().length()) < 0)
    {
        // Ignoring ftruncate() is not desired because the settings file might
        // become inconsistent (it might already be since ::write() succeeded
        // and ftruncate() might have been necessary) on continued use.
        throw GeneralException("Settings: failed truncating settings file after writing.");
    }

done:
#ifdef _WIN32
    reopen(false /* For reading. */);
#else /* !defined(_WIN32) */
    (void)0; /* prevents `error: expected primary-expression before '}' token' */
#endif /* !defined(_WIN32) */
}

#ifdef TEST_SETTINGS

#ifdef NDEBUG
# error "Compile me without NDEBUG defined."
#endif /* defined(NDEBUG) */

#include "SettingsGlue.hh"
#include <cassert>

// Since we use QTimer::singleShot (in SettingsGlue.cc), we will get the following warning from Qt
// when any of the methods that modifies the settings will try to schedule an updateAndFlush() call
// using SettingsGlue::scheduleUpdateAndFlush():
//   "QObject::startTimer: QTimer can only be used with threads started with QThread"
// However, we can ignore this, and simply call updateAndFlush() manually.

int main(void)
{
    const char filename[] = "test.ini";

    SettingsGlue settingsGlue;
    Settings settings(settingsGlue, filename);

    settings.setString("stringtest.greeting", "dlrow olleh");
    assert(settings.getString("stringtest.greeting", "") == "dlrow olleh");
    settings.setString("stringtest.greeting", "hello world");
    assert(settings.getString("stringtest.greeting", "") == "hello world");
    settings.setString("stringtest.greeting", "");
    assert(settings.getString("stringtest.greeting", "hello world") == "");

    assert(settings.getInt("integer.does_not_exist", -0xC0FFEE) == -0xC0FFEE);
    settings.setInt("integer.test", 99911);
    assert(settings.getInt("integer.test", -1) == 99911);
    settings.setInt("integer.test2", -8771);
    assert(settings.getInt("integer.test2", -1) == -8771);

    settings.setBool("boolean.0", true);
    assert(settings.getBool("boolean.0", false) == true);
    assert(settings.getBool("boolean.does_not_exist", false) == false);
    assert(settings.getBool("boolean.does_not_exist", true) == true);

    settings.updateAndFlush();

    printf("Tests succeeded. Please manually verify the contents of the file '%s'.\n", filename);

    return 0;
}

#endif /* defined(TEST_SETTINGS) */
