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

#ifndef WEBAPP_H
#define WEBAPP_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <functional>
#include <netinet/in.h>
#include <string> // for string

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#include "Router.h"

namespace express {

    class WebApp : public Router {
    protected:
        WebApp();

        WebApp(const Router& router)
            : WebApp::Router(router) {
        }

        virtual void listen(in_port_t port, const std::function<void(int err)>& onError = nullptr) = 0;
        virtual void listen(const std::string& host, in_port_t port, const std::function<void(int err)>& onError = nullptr) = 0;

    public:
        static void init(int argc, char* argv[]);
        static void start();
        static void stop();

    private:
        static bool initialized;
    };

} // namespace express

#endif // WEBAPP_H
