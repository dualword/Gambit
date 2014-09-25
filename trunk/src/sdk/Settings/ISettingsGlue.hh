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
 * Interface for the glue object which is used to integrate the Settings class into the
 * application. It functions as a simple layer between the Settings class and the application, to
 * make the Settings class easier to reuse.
 */

#ifndef I_SETTINGS_GLUE_HH
#define I_SETTINGS_GLUE_HH

class Settings;

class ISettingsGlue
{
public:
    virtual ~ISettingsGlue() {};

    virtual void cancelScheduledUpdateAndFlush() = 0;
    virtual void scheduleUpdateAndFlush() = 0;
    virtual void setSettings(Settings *settings) = 0;
};

#endif
