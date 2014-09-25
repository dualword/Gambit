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

// Not SETTINGS_CONTAINER_HH as this may be used by the backends.
#ifndef BACKENDS_SETTINGS_CONTAINER_HH
#define BACKENDS_SETTINGS_CONTAINER_HH

#if defined(CONFIG_SETTINGS_BACKEND_USE_BOOST)
# include "Boost/SettingsContainer.hh"
#elif defined(CONFIG_SETTINGS_BACKEND_USE_QT)
# include "Qt/SettingsContainer.hh"
#endif /* defined(CONFIG_SETTINGS_BACKEND_USE_QT) */

#endif
