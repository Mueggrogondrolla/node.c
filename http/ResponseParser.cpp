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

#include <easylogging++.h>
#include <tuple>   // for tie, tuple
#include <utility> // for tuple_element<>::type, pair

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#include "ResponseParser.h"
#include "http_utils.h"

namespace http {

    ResponseParser::ResponseParser(
        const std::function<void(const std::string&, const std::string&, const std::string&)>& onResponse,
        const std::function<void(const std::map<std::string, std::string>&, const std::map<std::string, CookieOptions>&)>& onHeader,
        const std::function<void(char*, size_t)>& onContent, const std::function<void(ResponseParser&)>& onParsed,
        const std::function<void(int status, const std::string& reason)>& onError)
        : onResponse(onResponse)
        , onHeader(onHeader)
        , onContent(onContent)
        , onParsed(onParsed)
        , onError(onError) {
    }

    ResponseParser::ResponseParser(
        const std::function<void(const std::string&, const std::string&, const std::string&)>&& onResponse,
        const std::function<void(const std::map<std::string, std::string>&, const std::map<std::string, CookieOptions>&)>&& onHeader,
        const std::function<void(char*, size_t)>&& onContent, const std::function<void(ResponseParser&)>&& onParsed,
        const std::function<void(int status, const std::string& reason)>&& onError)
        : onResponse(onResponse)
        , onHeader(onHeader)
        , onContent(onContent)
        , onParsed(onParsed)
        , onError(onError) {
    }

    void ResponseParser::reset() {
        Parser::reset();
        httpVersion.clear();
        statusCode.clear();
        reason.clear();
        cookies.clear();
    }

    enum Parser::PAS ResponseParser::parseStartLine(std::string& line) {
        enum Parser::PAS PAS = Parser::PAS::HEADER;

        if (!line.empty()) {
            std::string remaining;

            std::tie(httpVersion, remaining) = httputils::str_split(line, ' ');
            std::tie(statusCode, reason) = httputils::str_split(remaining, ' ');

            onResponse(httpVersion, statusCode, reason);
        } else {
            PAS = parsingError(400, "Response-line empty");
        }
        return PAS;
    }

    enum Parser::PAS ResponseParser::parseHeader() {
        for (auto& [field, value] : Parser::headers) {
            VLOG(2) << "++ Parse header field: " << field << " = " << value;
            if (field != "set-cookie") {
                if (field == "content-length") {
                    Parser::contentLength = std::stoi(value);
                }
            } else {
                std::string cookiesLine = value;

                while (!cookiesLine.empty()) {
                    std::string cookieLine;
                    std::tie(cookieLine, cookiesLine) = httputils::str_split(cookiesLine, ',');

                    std::string cookieOptions;
                    std::string cookie;
                    std::tie(cookie, cookieOptions) = httputils::str_split(cookieLine, ';');

                    std::string name;
                    std::string value;
                    std::tie(name, value) = httputils::str_split(cookie, '=');
                    httputils::str_trimm(name);
                    httputils::str_trimm(value);

                    VLOG(2) << "++ Cookie: " << name << " = " << value;

                    std::map<std::string, CookieOptions>::iterator cookieElement;
                    bool inserted;
                    std::tie(cookieElement, inserted) = cookies.insert({name, CookieOptions(value)});

                    while (!cookieOptions.empty()) {
                        std::string option;
                        std::tie(option, cookieOptions) = httputils::str_split(cookieOptions, ';');

                        std::string name;
                        std::string value;
                        std::tie(name, value) = httputils::str_split(option, '=');
                        httputils::str_trimm(name);
                        httputils::str_trimm(value);

                        VLOG(2) << "    ++ CookieOption: " << name << " = " << value;
                        cookieElement->second.setOption(name, value);
                    }
                }
            }
        }

        Parser::headers.erase("set-cookie");

        onHeader(Parser::headers, cookies);

        enum Parser::PAS PAS = Parser::PAS::BODY;
        if (contentLength == 0) {
            parsingFinished();
            PAS = PAS::FIRSTLINE;
        }

        return PAS;
    }

    enum Parser::PAS ResponseParser::parseContent(char* content, size_t size) {
        onContent(content, size);
        parsingFinished();

        return PAS::FIRSTLINE;
    }

    enum Parser::PAS ResponseParser::parsingError(int code, const std::string& reason) {
        onError(code, reason);
        reset();

        return PAS::ERROR;
    }

    void ResponseParser::parsingFinished() {
        onParsed(*this);
    }

} // namespace http
