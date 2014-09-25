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

#ifndef GAMBIT_APPLICATION_HH
#define GAMBIT_APPLICATION_HH

#include <QApplication>
#include <QLocale>
#include <QTranslator>

class NamedLock;

class GambitApplication : public QApplication
{
    Q_OBJECT

public:
    static const char author[],
                      name[],
                      homePageUrl[];

    GambitApplication(int &, char **);
    ~GambitApplication();

    QString autoResumeGameFileName() const;
    bool haveAutoResumeLock() const;
    void releaseAutoResumeLock();
    void tryAcquireAutoResumeLock();

    QString configDirPath() const;
    QString configFilePath() const;
    QLocale defaultLocale() const;
    void determineConfigDirToUse() const;
    QLocale::Language language() const;
    void loadLanguage(QLocale::Language);
    bool notify(QObject *receiver, QEvent *_event);
    QString savedGamesDirPath() const;

private:
    static const char configFileName[];

    QTranslator qtTranslator,
                appTranslator;
    QLocale::Language language_;
    mutable bool haveDeterminedConfigDirToUse;
    mutable QString configDirPath_;
    mutable QString autoResumeGameFileName_;
    mutable QString savedGamesDirPath_;
    NamedLock *autoResumeLock_;
};

#endif
