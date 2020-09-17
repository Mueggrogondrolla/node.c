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

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <sys/socket.h> // for recv

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#include "socket/legacy/SocketReader.h"

namespace net::socket::legacy {

    ssize_t SocketReader::read(char* junk, size_t junkLen) {
        return ::recv(getFd(), junk, junkLen, 0);
    }

}; // namespace net::socket::legacy
