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

#ifndef I_EVENT_DISPATCHER_HH
#define I_EVENT_DISPATCHER_HH

class Event;
class IEventListener;

class IEventDispatcher
{
public:
    virtual ~IEventDispatcher() {};

    virtual void addEventListener(IEventListener &) = 0;
    virtual void removeEventListener(IEventListener &) = 0;

private:
    virtual void dispatchEvent(Event &) = 0;
};

#endif
