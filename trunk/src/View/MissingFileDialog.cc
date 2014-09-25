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

#include "MissingFileDialog.hh"
#include "Core/GambitApplication.hh"
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QWidget>
#include <cassert>
#include <algorithm>

MissingFileDialog &MissingFileDialog::instance()
{
    static MissingFileDialog _instance;
    return _instance;
}

void MissingFileDialog::addIfNonExistent(const char *fileName)
{
    QString s(fileName);
    if (!QFile::exists(s))
        add(s);
}

void MissingFileDialog::addIfNonExistent(const std::string &fileName)
{
    const QString &s = QString::fromUtf8(fileName.c_str());
    if (!QFile::exists(s))
        add(s);
}

void MissingFileDialog::addIfNonExistent(const QString &fileName)
{
    if (!QFile::exists(fileName))
        add(fileName);
}

void MissingFileDialog::addIfNonExistent(const std::vector<QString> &fileNames)
{
    std::vector<QString>::const_iterator it;
    for (it = fileNames.begin();
         it != fileNames.end();
         ++it)
    {
        addIfNonExistent(*it);
    }
}

// Returns:
//     'true' if the dialog was shown, 'false' otherwise.
bool MissingFileDialog::showIfNecessary(QWidget *_parent)
{
    if (!files.size())
        return false;

    QString message = QString("<p>") + tr("The following files seem to be either missing or damaged:") + "<br/>",
            detail;

    std::sort(files.begin(), files.end());

    std::vector<QString>::const_iterator it = files.begin(),
                                         end = files.end();
    for (int i = 0; it != end; ++it, ++i)
    {
        QString fileName = QDir::toNativeSeparators(*it);
        const QString &appDirPath = QDir::toNativeSeparators(QApplication::applicationDirPath());

        // Cut the application directory prefix (plus the trailing directory separator, if any)
        // from the filename, if necessary.
        if (fileName.startsWith(appDirPath))
            fileName.remove(0, appDirPath.length() + 1);

        if (i != 0)
        {
            if (i < 3)
                message += "<br/>";
            detail += "\n";
        }

        if (i < 3)
            message += fileName;
        detail += fileName;
    }

    if (files.size() > 3)
        message += "<br/>..... " + tr("and others; see the Details for the full list.");

    message += QString("</p>") +
        "<p>" +
            tr("You can download the most recent stable version of Gambit from:") + " "
            "<a href=\"" + GambitApplication::homePageUrl + "\">" +
            GambitApplication::homePageUrl + "</a>"
        "</p>";

    QMessageBox m(
        QMessageBox::Critical,
        GambitApplication::name,
        message,
        QMessageBox::Ok,
        _parent);
    m.setEscapeButton(QMessageBox::Ok);
    if (files.size() > 3)
        m.setDetailedText(detail);
    m.exec();

    // The user was notified of the missing files, so we now start with a clean slate again, so
    // that the missing files aren't shown again (perhaps incorrectly even, as the files may not
    // be missing anymore the next time we show the dialog) the next time we show the dialog.
    files.clear();

    return true;
}

void MissingFileDialog::add(const QString &fileName)
{
    std::vector<QString>::const_iterator begin = files.begin(),
                                         end = files.end(),
                                         it = std::find(begin, end, fileName);
    if (it == end)
    {
        // Only add the filename if it wasn't yet added, we don't want duplicate filenames in the
        // dialog.
        files.push_back(fileName);
    }
}
