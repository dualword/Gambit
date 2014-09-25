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

#ifndef GAME_CONTROLLER_TIMER_H
#define GAME_CONTROLLER_TIMER_H

#include <QBasicTimer>

class GameController;

class GameControllerTimer : public QBasicTimer
{
public:
    GameControllerTimer(GameController &);

    void useLowLatency();
    void useNormalLatency();
    void start();

private:
    GameController &gc;
    int mode;
};

#endif /* !defined(GAME_CONTROLLER_TIMER_H) */
