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

#include "PreferencesDialog.hh"
#include "LanguageListWidget.hh"
#include "LanguageListWidgetItem.hh"
#include "UI.hh"
#include "Core/GambitApplication.hh"
#include "Core/Preferences.hh"
#include "Core/ResourcePath.hh"
#include "sdk/Settings/Settings.hh"
#include "Utils/Cast.hh"
#include "Utils/Qt.hh"
#include <QComboBox>
#include <QLineEdit>
#include <QLocale>
#include <QMouseEvent>
#include <QPlainTextEdit>
#include <QStringListModel>
#include <cassert>
using Utils::Cast::enforce_dynamic_cast;

PreferencesDialog::PreferencesDialog(
    GambitApplication &_app,
    Preferences &_preferences,
    Settings &_settings,
    QWidget *_parent /* = 0 */,
    Qt::WindowFlags f /* = 0 */)
    : QDialog(_parent, f),
      app(_app),
      preferences(_preferences),
      settings(_settings),
      languageListWidget(0),
      whatsThisCandidate(0)
{
    setupUi();

    loadSettings();
}

void PreferencesDialog::done(int r)
{
    QDialog::done(r);

    if (r == QDialog::Accepted)
        apply();
}

bool PreferencesDialog::eventFilter(QObject *watched, QEvent *ev)
{
    QWidget *w;

    if (!watched->isWidgetType())
        goto deferToBase;

    w = enforce_dynamic_cast<QWidget *>(watched);

    switch (ev->type())
    {
    case QEvent::Enter:
    case QEvent::Leave:
    {
        if (ev->type() == QEvent::Enter)
            whatsThisCandidate = w;
        else
            whatsThisCandidate = 0;

        whatsThisTimer.start(100);
        break;
    }

    case QEvent::FocusIn:
        updateWhatsThis(true);
        break;

    default:
        break;
    }

deferToBase:
    return QDialog::eventFilter(watched, ev);
}

void PreferencesDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    switch (buttonBox->buttonRole(button))
    {
    case QDialogButtonBox::ApplyRole:
        apply();
        break;
    default:
        break;
    }
}

void PreferencesDialog::on_checkForUpdatesCheckBox_stateChanged(int state)
{
#if !defined(CONFIG_ENABLE_UPDATE_CHECKER)
    (void)state;
#else // defined(CONFIG_ENABLE_UPDATE_CHECKER)
    const bool b = state != Qt::Unchecked;

    oncePerLabel->setEnabled(b);
    updateCheckIntervalSpinBox->setEnabled(b);
    daysLabel->setEnabled(b);
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)
}

void PreferencesDialog::on_searchDepthSlider_valueChanged(int value)
{
    difficultyValueLabel->setText(QString::number(value));
}

void PreferencesDialog::on_updateCheckIntervalSpinBox_valueChanged(int value)
{
#if !defined(CONFIG_ENABLE_UPDATE_CHECKER)
    (void)value;
#else // defined(CONFIG_ENABLE_UPDATE_CHECKER)
    daysLabel->setText(value == 1 ? tr("day") : tr("days"));
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)
}

void PreferencesDialog::updateWhatsThis(bool gotFocus /* = false */)
{
    QString _whatsThis;

    QWidget *w = whatsThisCandidate;
    if (gotFocus)
        w = QApplication::focusWidget();

    if (w)
        _whatsThis = w->whatsThis();

    if (!_whatsThis.length())
        _whatsThis = PreferencesDialog::whatsThis();

    whatsThisLabel->setText(_whatsThis);
}

void PreferencesDialog::apply()
{
    QWidget *tabPage = tabWidget->currentWidget();
    if (tabPage)
        preferences.setInterfacePreferencesDialogActiveTabPage(tabPage->objectName());

    preferences.setInterfaceHighlightValidTargets(highlightValidTargetsCheckBox->checkState() == Qt::Checked);
    preferences.setInterfaceShowNotifications(showNotificationsCheckBox->checkState() == Qt::Checked);
    preferences.setInterfaceResumeGameAtStartup(resumeGameAtStartupCheckBox->checkState() == Qt::Checked);

    QLocale::Language language = languageListWidget->selectedLanguage();
    preferences.setInterfaceLanguage(language);
    languageListWidget->emphasizeSelectedLanguage();

    preferences.setEngineSearchDepth(searchDepthSlider->value());

    preferences.setGraphicsUseHardwareAcceleration(hardwareAccelerationCheckBox->checkState() == Qt::Checked);
    preferences.setGraphicsDisableAnimations(disableAnimationsCheckBox->checkState() == Qt::Checked);

#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
    preferences.setInterfaceAutomaticallyCheckForUpdates(checkForUpdatesCheckBox->checkState() == Qt::Checked);
    preferences.setInterfaceUpdateCheckIntervalDays(updateCheckIntervalSpinBox->value());
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)

    // Save the settings immediately when they are applied, so that any delays from disk I/O are
    // felt immediately, and not a little while after the dialog was closed, as by then the user
    // may want to quickly resume playing and thus at that time delays may be more annoying.
    settings.updateAndFlush();

    app.loadLanguage(language);
    retranslateUi();
}

#if !defined(CONFIG_ENABLE_UPDATE_CHECKER)
void PreferencesDialog::hideUpdateTab()
{
    int i = tabWidget->indexOf(updateTab);
    assert(i >= 0);
    tabWidget->removeTab(i);

    i = tabWidget->indexOf(updateTab);
    assert(i < 0); // Be sure it's gone.
}
#endif // !defined(CONFIG_ENABLE_UPDATE_CHECKER)

void PreferencesDialog::loadSettings()
{
    const QString &activeTabPageObjectName = preferences.interfacePreferencesDialogActiveTabPage();
    for (int i = 0; i < tabWidget->count(); ++i)
    {
        QWidget *w = tabWidget->widget(i);
        assert(w);
        assert(!w->objectName().isEmpty());
        if (w->objectName() == activeTabPageObjectName)
        {
            tabWidget->setCurrentWidget(w);
            // Don't 'break', we want to assert() that every tab page has an objectName().
            //break;
        }
    }

    highlightValidTargetsCheckBox->setChecked(preferences.interfaceHighlightValidTargets());
    showNotificationsCheckBox->setChecked(preferences.interfaceShowNotifications());
    resumeGameAtStartupCheckBox->setChecked(preferences.interfaceResumeGameAtStartup());

    searchDepthSlider->setValue(preferences.engineSearchDepth());

    hardwareAccelerationCheckBox->setChecked(preferences.graphicsUseHardwareAcceleration());
    disableAnimationsCheckBox->setChecked(preferences.graphicsDisableAnimations());

#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
    checkForUpdatesCheckBox->setChecked(preferences.interfaceAutomaticallyCheckForUpdates());
    updateCheckIntervalSpinBox->setValue(preferences.interfaceUpdateCheckIntervalDays());
    on_checkForUpdatesCheckBox_stateChanged(checkForUpdatesCheckBox->checkState());
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)

    // Use the actual current language, not the language (if any) from the settings file, because
    // these two may not always match, and it would be counter-intuitive to show for example that
    // we're using the Dutch language even though the English language is actually used (which may
    // very well happen if the Dutch language file happens to be missing and thus cannot be loaded
    // after the user tries to change the language to Dutch, however it does write the preference
    // to the settings file, and that's how it could otherwise happen that we show Dutch as the
    // active language when it's not).
    int row = languageListWidget->findLanguageRow(app.language());
    assert(row >= 0);
    languageListWidget->setCurrentRow(row);
}

void PreferencesDialog::retranslateUi()
{
    Ui::PreferencesDialog::retranslateUi(this);

    updateWhatsThis();

    QListWidgetItem *item = languageListWidget->item(0);
    item->setText(trEnglishBuiltin());
}

void PreferencesDialog::setupUi()
{
    Ui::PreferencesDialog::setupUi(this);

    setWindowTitle(UI::makeTitle(windowTitle()));

    app.installEventFilter(this);

    whatsThisTimer.setSingleShot(true);
    connect(&whatsThisTimer, SIGNAL(timeout()), this, SLOT(updateWhatsThis()));

    QPalette pal = whatsThisLabel->palette();
    pal.setBrush(QPalette::Window, pal.brush(QPalette::Midlight));
    whatsThisLabel->setAutoFillBackground(true);
    whatsThisLabel->setPalette(pal);
    whatsThisLabel->setFixedHeight(whatsThisLabel->frameWidth() * 2 +
                                   whatsThisLabel->margin() * 2 +
                                   whatsThisLabel->fontMetrics().lineSpacing() * 4);
    // Don't let Qt ever set focus on this widget. We don't want it to possibly be the widget that
    // automatically gets focus (also because we don't want to see a blinking cursor).
    // Doing this doesn't hinder usability, since the user can still select and copy the text.
    whatsThisLabel->setFocusPolicy(Qt::NoFocus);

    languageListWidget = new LanguageListWidget(languageTab);
    languageListWidget->addItem(new LanguageListWidgetItem(QLocale::English, trEnglishBuiltin()));
    languageListWidget->addItem(new LanguageListWidgetItem(QLocale::Dutch, "Nederlands"));
    languageListContainer->addWidget(languageListWidget);

    interfaceLanguageLabel->setBuddy(languageListWidget);

    searchDepthSlider->setMinimum(Preferences::SearchDepthMinimum);
    searchDepthSlider->setMaximum(Preferences::SearchDepthMaximum);
    searchDepthSlider->setPageStep(1);

#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
    updateCheckIntervalSpinBox->setMinimum(Preferences::UpdateCheckIntervalDaysMinimum);
    updateCheckIntervalSpinBox->setMaximum(Preferences::UpdateCheckIntervalDaysMaximum);
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)

#if !defined(CONFIG_ENABLE_UPDATE_CHECKER)
    hideUpdateTab();
#endif // !defined(CONFIG_ENABLE_UPDATE_CHECKER)
}

QString PreferencesDialog::trEnglishBuiltin() const
{
    return tr("English (built-in)");
}
