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

#ifndef SETTINGS_HH
#define SETTINGS_HH

#include "ISettingsGlue.hh"
#include "Backends/SettingsContainer.hh"
#include <set>
#include <string>
#ifdef _WIN32
# include <windows.h>
#endif /* defined(_WIN32) */

class Settings
{
public:
    Settings(ISettingsGlue &, const std::string &);
    ~Settings();

    bool getBool(const std::string &, bool) const;
    void setBool(const std::string &, bool);

    int getInt(const std::string &, int) const;
    void setInt(const std::string &, int);

    std::string getString(const std::string &, const std::string &) const;
    void setString(const std::string &, const std::string &);

    void remove(const std::string &);

    void updateAndFlush();

private:
    std::string get(const std::string &, const std::string &) const;
    void set(const std::string &, const std::string &);

    void close();
    void flush();
    void sync() const;
    SettingsContainer merge(const SettingsContainer &, const SettingsContainer &) const;
    void requestUpdate();
    std::string read();
    void readAndMerge();
#ifdef _WIN32
    void reopen(bool);
#endif /* defined(_WIN32) */
    void write();

private:
    ISettingsGlue &glue;
    std::string fileName;
    int fd;
    SettingsContainer settings;
    bool pendingFlush;
    bool readOnly;

    typedef std::set<std::string> removed_keys_type;
    removed_keys_type removedKeys;
};

#endif
