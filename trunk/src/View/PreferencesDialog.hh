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

#ifndef PREFERENCES_DIALOG_HH
#define PREFERENCES_DIALOG_HH

#include "../../ui_PreferencesDialog.h"

#include <QTimer>

class GambitApplication;
class LanguageListWidget;
class Preferences;
class Settings;

class PreferencesDialog : public QDialog, private Ui::PreferencesDialog
{
    Q_OBJECT

public:
    PreferencesDialog(
        GambitApplication &,
        Preferences &,
        Settings &,
        QWidget * = 0,
        Qt::WindowFlags = 0);

public slots:
    void done(int);

protected:
    bool eventFilter(QObject *, QEvent *);

private slots:
    void on_buttonBox_clicked(QAbstractButton *);
    void on_checkForUpdatesCheckBox_stateChanged(int);
    void on_searchDepthSlider_valueChanged(int);
    void on_updateCheckIntervalSpinBox_valueChanged(int);

private slots:
    void updateWhatsThis(bool = false);

private:
    void apply();
#if !defined(CONFIG_ENABLE_UPDATE_CHECKER)
    void hideUpdateTab();
#endif // !defined(CONFIG_ENABLE_UPDATE_CHECKER)
    void loadSettings();
    void retranslateUi();
    void setupUi();
    QString trEnglishBuiltin() const;

    GambitApplication &app;
    Preferences &preferences;
    Settings &settings;
    LanguageListWidget *languageListWidget;
    QWidget *whatsThisCandidate;
    QTimer whatsThisTimer;
};

#endif
