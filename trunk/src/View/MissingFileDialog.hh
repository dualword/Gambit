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

#ifndef MISSING_FILE_DIALOG_HH
#define MISSING_FILE_DIALOG_HH

#include <QObject>
#include <string>
#include <vector>

class QWidget;

class MissingFileDialog : private QObject
{
    Q_OBJECT

public:
    static MissingFileDialog &instance();

    void addIfNonExistent(const char *fileName);
    void addIfNonExistent(const std::string &fileName);
    void addIfNonExistent(const QString &fileName);
    void addIfNonExistent(const std::vector<QString> &fileNames);
    bool showIfNecessary(QWidget *_parent);

private:
    MissingFileDialog() {}
    ~MissingFileDialog() {}

    void add(const QString &fileName);

    std::vector<QString> files;
};

#endif
