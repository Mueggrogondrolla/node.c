/*
 * snode.c - a slim toolkit for network communication
 * Copyright (C) 2020 Volker Christian <me@vchrist.at>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TIMEREVENTDISPATCHER_H
#define TIMEREVENTDISPATCHER_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <list>
#include <sys/time.h>

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#include "timer/Timer.h"

namespace net {

    class TimerEventDispatcher {
    public:
        TimerEventDispatcher() = default;

        struct timeval getNextTimeout();

        void dispatch();

        void remove(timer::Timer* timer);
        void add(timer::Timer* timer);

        bool empty();

        void cancelAll();

    private:
        std::list<timer::Timer*> timerList;
        std::list<timer::Timer*> addedList;
        std::list<timer::Timer*> removedList;

        class timernode_lt {
        public:
            bool operator()(const timer::Timer* t1, const timer::Timer* t2) const {
                return static_cast<timeval>(*t1) < static_cast<timeval>(*t2);
            }
        };

        bool timerListDirty = false;
    };

} // namespace net

#endif // TIMEREVENTDISPATCHER_H
