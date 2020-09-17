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

#ifndef SOCKETREADER_H
#define SOCKETREADER_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <cerrno>
#include <cstddef> // for size_t
#include <functional>
#include <sys/types.h> // for ssize_t

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#include "ReadEventReceiver.h"

#define MAX_READ_JUNKSIZE 16384

namespace net::socket {

    template <typename SocketT>
    class SocketReader
        : public ReadEventReceiver
        , virtual public SocketT {
    public:
        using Socket = SocketT;

        SocketReader() = delete;

        explicit SocketReader(const std::function<void(const char* junk, ssize_t junkLen)>& onRead,
                              const std::function<void(int errnum)>& onError)
            : onRead(onRead)
            , onError(onError) {
        }

        ~SocketReader() override {
            if (ReadEventReceiver::isEnabled()) {
                ReadEventReceiver::disable();
            }
        }

        void readEvent() override {
            errno = 0;

            // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays, modernize-avoid-c-arrays)
            static char junk[MAX_READ_JUNKSIZE];

            ssize_t ret = read(junk, MAX_READ_JUNKSIZE);

            if (ret > 0) {
                onRead(junk, ret);
            } else if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
                ReadEventReceiver::disable();
                onError(ret == 0 ? 0 : errno);
            }
        }

    protected:
        virtual ssize_t read(char* junk, size_t junkLen) = 0;

    private:
        std::function<void(const char* junk, ssize_t junkLen)> onRead;
        std::function<void(int errnum)> onError;
    };

} // namespace net::socket

#endif // SOCKETREADER_H
