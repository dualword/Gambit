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

#include "EventDispatcher.hh"
#include "IEventListener.hh"
#include <cstddef>

void EventDispatcher::addEventListener(IEventListener &listener)
{
    listeners.push_back(&listener);
}

void EventDispatcher::removeEventListener(IEventListener &listener)
{
    size_t size = listeners.size();
    for (size_t i = 0; i < size; ++i)
    {
        if (listeners[i] == &listener)
        {
            // Faster than erase(), but necessitates that the order of event
            // dispatching isn't important.
            listeners[i] = listeners[size - 1];
            listeners.pop_back();
            return;
        }
    }
}

void EventDispatcher::dispatchEvent(Event &ev)
{
    if (listeners.size() == 0)
        return;

    std::vector<IEventListener *>::const_iterator it = listeners.begin();
    std::vector<IEventListener *>::const_iterator end = listeners.end();
    for ( ; it != end; ++it)
        (*it)->event(ev);
}
