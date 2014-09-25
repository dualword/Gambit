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

/*
 * See 'ISettingsGlue.hh' for more information on what this is used for.
 */

#ifndef SETTINGS_GLUE_HH
#define SETTINGS_GLUE_HH

#include "ISettingsGlue.hh"
#include "Settings.hh"
#include <QObject>
#include <QTimer>

class SettingsGlue : private QObject, public ISettingsGlue
{
    Q_OBJECT

public:
    SettingsGlue();

    void cancelScheduledUpdateAndFlush();
    void scheduleUpdateAndFlush();

private slots:
    void cb_scheduleUpdateAndFlush();

private:
    void setSettings(Settings *_settings);

    Settings *settings;
    bool cancelUpdateAndFlush,
         isUpdateAndFlushScheduled;
    QTimer timer;
};

#endif
