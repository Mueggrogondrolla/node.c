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

#ifndef SOCKETCONNECTIONBASE_H
#define SOCKETCONNECTIONBASE_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#include "AttributeInjector.h"

namespace net::socket {

    class SocketConnectionBase {
    public:
        SocketConnectionBase() = default;
        SocketConnectionBase(const SocketConnectionBase&) = delete;
        SocketConnectionBase& operator=(const SocketConnectionBase&) = delete;

        virtual ~SocketConnectionBase() = default;

        virtual void enqueue(const char* junk, size_t junkLen) = 0;
        virtual void enqueue(const std::string& data) = 0;

        virtual void end(bool instantly = false) = 0;

        template <utils::InjectedAttribute Attribute>
        constexpr void setContext(Attribute& attribute) const {
            protocol.setAttribute<Attribute>(attribute);
        }

        template <utils::InjectedAttribute Attribute>
        constexpr void setContext(Attribute&& attribute) const {
            protocol.setAttribute<Attribute>(attribute);
        }

        template <utils::InjectedAttribute Attribute>
        constexpr bool getContext(std::function<void(Attribute&)> onFound) const {
            return protocol.getAttribute<Attribute>([&onFound](Attribute& attribute) -> void {
                onFound(attribute);
            });
        }

        template <utils::InjectedAttribute Attribute>
        constexpr void getContext(std::function<void(Attribute&)> onFound, std::function<void(const std::string&)> onNotFound) const {
            return protocol.getAttribute<Attribute>(
                [&onFound](Attribute& attribute) -> void {
                    onFound(attribute);
                },
                [&onNotFound](const std::string& msg) -> void {
                    onNotFound(msg);
                });
        }

    private:
        utils::SingleAttributeInjector protocol;
    };

} // namespace net::socket

#endif // SOCKETCONNECTIONBASE_H
