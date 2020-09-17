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

#ifndef RESPONSEPARSER_H
#define RESPONSEPARSER_H

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include <cstddef>
#include <functional>
#include <map>
#include <string>

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#include "CookieOptions.h"
#include "Parser.h"

namespace http {

    class ResponseParser : public Parser {
    public:
        ResponseParser(
            const std::function<void(const std::string&, const std::string&, const std::string&)>& onResponse,
            const std::function<void(const std::map<std::string, std::string>&, const std::map<std::string, CookieOptions>&)>& onHeader,
            const std::function<void(char*, size_t)>& onContent, const std::function<void(ResponseParser&)>& onParsed,
            const std::function<void(int status, const std::string& reason)>& onError);

        ResponseParser(
            const std::function<void(const std::string&, const std::string&, const std::string&)>&& onResponse,
            const std::function<void(const std::map<std::string, std::string>&, const std::map<std::string, CookieOptions>&)>&& onHeader,
            const std::function<void(char*, size_t)>&& onContent, const std::function<void(ResponseParser&)>&& onParsed,
            const std::function<void(int status, const std::string& reason)>&& onError);

        enum Parser::PAS parseStartLine(std::string& line) override;
        enum Parser::PAS parseHeader() override;
        enum Parser::PAS parseContent(char* content, size_t size) override;
        enum Parser::PAS parsingError(int code, const std::string& reason) override;

        void reset() override;

    protected:
        void parsingFinished();

        std::string httpVersion;
        std::string statusCode;
        std::string reason;
        std::map<std::string, CookieOptions> cookies;

        std::function<void(const std::string&, const std::string&, const std::string&)> onResponse;
        std::function<void(const std::map<std::string, std::string>&, const std::map<std::string, CookieOptions>&)> onHeader;
        std::function<void(char*, size_t)> onContent;
        std::function<void(ResponseParser&)> onParsed;
        std::function<void(int status, const std::string& reason)> onError;
    };

} // namespace http

#endif // RESPONSEPARSER_H
