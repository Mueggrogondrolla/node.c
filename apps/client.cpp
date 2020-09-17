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

#include "legacy/Client.h"

#include "EventLoop.h"
#include "ServerResponse.h"
#include "tls/Client.h"

#include <cstring>
#include <easylogging++.h>
#include <iostream>
#include <openssl/x509v3.h>

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#define CERTF "/home/voc/projects/ServerVoc/certs/calisto.home.vchrist.at_-_snode.c_-_client.pem"
#define KEYF "/home/voc/projects/ServerVoc/certs/Volker_Christian_-_Web_-_snode.c_-_client.key.encrypted.pem"
#define KEYFPASS "snode.c"
#define SERVERCAFILE "/home/voc/projects/ServerVoc/certs/Volker_Christian_-_Root_CA.crt"

int main(int argc, char* argv[]) {
    net::EventLoop::init(argc, argv);

    http::legacy::Client legacyClient(
        [](net::socket::legacy::SocketConnection* socketConnection) -> void {
            VLOG(0) << "-- OnConnect";
            VLOG(0) << "     Server: " + socketConnection->getRemoteAddress().host() + "(" + socketConnection->getRemoteAddress().ip() +
                           "):" + std::to_string(socketConnection->getRemoteAddress().port());
            VLOG(0) << "     Client: " + socketConnection->getLocalAddress().host() + "(" + socketConnection->getLocalAddress().ip() +
                           "):" + std::to_string(socketConnection->getLocalAddress().port());
        },
        [](const http::ServerResponse& serverResponse) -> void {
            VLOG(0) << "-- OnResponse";
            VLOG(0) << "     Status:";
            VLOG(0) << "       " << serverResponse.httpVersion;
            VLOG(0) << "       " << serverResponse.statusCode;
            VLOG(0) << "       " << serverResponse.reason;

            VLOG(0) << "     Headers:";
            for (auto [field, value] : *serverResponse.headers) {
                VLOG(0) << "       " << field + " = " + value;
            }

            VLOG(0) << "     Cookies:";
            for (auto [name, cookie] : *serverResponse.cookies) {
                VLOG(0) << "       " + name + " = " + cookie.getValue();
                for (auto [option, value] : cookie.getOptions()) {
                    VLOG(0) << "         " + option + " = " + value;
                }
            }

            char* body = new char[serverResponse.contentLength + 1];
            memcpy(body, serverResponse.body, serverResponse.contentLength);
            body[serverResponse.contentLength] = 0;

            VLOG(1) << "     Body:\n----------- start body -----------\n" << body << "------------ end body ------------";

            delete[] body;
        },
        []([[maybe_unused]] net::socket::legacy::SocketConnection* socketConnection) -> void {
            VLOG(0) << "-- OnDisconnect";
            VLOG(0) << "     Server: " + socketConnection->getRemoteAddress().host() + "(" + socketConnection->getRemoteAddress().ip() +
                           "):" + std::to_string(socketConnection->getRemoteAddress().port());
            VLOG(0) << "     Client: " + socketConnection->getLocalAddress().host() + "(" + socketConnection->getLocalAddress().ip() +
                           "):" + std::to_string(socketConnection->getLocalAddress().port());
        });

    http::tls::Client tlsClient(
        [](net::socket::tls::SocketConnection* socketConnection) -> void {
            VLOG(0) << "-- OnConnect";
            VLOG(0) << "     Server: " + socketConnection->getRemoteAddress().host() + "(" + socketConnection->getRemoteAddress().ip() +
                           "):" + std::to_string(socketConnection->getRemoteAddress().port());
            VLOG(0) << "     Client: " + socketConnection->getLocalAddress().host() + "(" + socketConnection->getLocalAddress().ip() +
                           "):" + std::to_string(socketConnection->getLocalAddress().port());

            X509* server_cert = SSL_get_peer_certificate(socketConnection->getSSL());
            if (server_cert != NULL) {
                int verifyErr = SSL_get_verify_result(socketConnection->getSSL());

                VLOG(0) << "     Server certificate: " + std::string(X509_verify_cert_error_string(verifyErr));

                char* str = X509_NAME_oneline(X509_get_subject_name(server_cert), 0, 0);
                VLOG(0) << "        Subject: " + std::string(str);
                OPENSSL_free(str);

                str = X509_NAME_oneline(X509_get_issuer_name(server_cert), 0, 0);
                VLOG(0) << "        Issuer: " + std::string(str);
                OPENSSL_free(str);

                // We could do all sorts of certificate verification stuff here before deallocating the certificate.

                GENERAL_NAMES* subjectAltNames =
                    static_cast<GENERAL_NAMES*>(X509_get_ext_d2i(server_cert, NID_subject_alt_name, NULL, NULL));

                int32_t altNameCount = sk_GENERAL_NAME_num(subjectAltNames);
                VLOG(0) << "        Subject alternative name count: " << altNameCount;
                for (int32_t i = 0; i < altNameCount; ++i) {
                    GENERAL_NAME* generalName = sk_GENERAL_NAME_value(subjectAltNames, i);
                    if (generalName->type == GEN_URI) {
                        std::string subjectAltName =
                            std::string(reinterpret_cast<const char*>(ASN1_STRING_get0_data(generalName->d.uniformResourceIdentifier)),
                                        ASN1_STRING_length(generalName->d.uniformResourceIdentifier));
                        VLOG(0) << "           SAN (URI): '" + subjectAltName;
                    } else if (generalName->type == GEN_DNS) {
                        std::string subjectAltName =
                            std::string(reinterpret_cast<const char*>(ASN1_STRING_get0_data(generalName->d.dNSName)),
                                        ASN1_STRING_length(generalName->d.dNSName));
                        VLOG(0) << "           SAN (DNS): '" + subjectAltName;
                    } else {
                        VLOG(0) << "           SAN (Type): '" + std::to_string(generalName->type);
                    }
                }
                sk_GENERAL_NAME_pop_free(subjectAltNames, GENERAL_NAME_free);

                X509_free(server_cert);
            } else {
                VLOG(0) << "     Server certificate: no certificate";
            }
        },
        [](const http::ServerResponse& serverResponse) -> void {
            VLOG(0) << "-- OnResponse";
            VLOG(0) << "     Status:";
            VLOG(0) << "       " << serverResponse.httpVersion;
            VLOG(0) << "       " << serverResponse.statusCode;
            VLOG(0) << "       " << serverResponse.reason;

            VLOG(0) << "     Headers:";
            for (auto [field, value] : *serverResponse.headers) {
                VLOG(0) << "       " << field + " = " + value;
            }

            VLOG(0) << "     Cookies:";
            for (auto [name, cookie] : *serverResponse.cookies) {
                VLOG(0) << "       " + name + " = " + cookie.getValue();
                for (auto [option, value] : cookie.getOptions()) {
                    VLOG(0) << "         " + option + " = " + value;
                }
            }

            char* body = new char[serverResponse.contentLength + 1];
            memcpy(body, serverResponse.body, serverResponse.contentLength);
            body[serverResponse.contentLength] = 0;

            VLOG(1) << "     Body:\n----------- start body -----------\n" << body << "------------ end body ------------";

            delete[] body;
        },
        []([[maybe_unused]] net::socket::tls::SocketConnection* socketConnection) -> void {
            VLOG(0) << "-- OnDisconnect";
            VLOG(0) << "     Server: " + socketConnection->getRemoteAddress().host() + "(" + socketConnection->getRemoteAddress().ip() +
                           "):" + std::to_string(socketConnection->getRemoteAddress().port());
            VLOG(0) << "     Client: " + socketConnection->getLocalAddress().host() + "(" + socketConnection->getLocalAddress().ip() +
                           "):" + std::to_string(socketConnection->getLocalAddress().port());
        },
        {{"caFile", SERVERCAFILE}});

    legacyClient.get({{"host", "localhost"}, {"port", 8080}, {"path", "/index.html"}}, [](int err) -> void {
        if (err != 0) {
            PLOG(ERROR) << "OnError: " << err;
        }
    }); // Connection:keep-alive\r\n\r\n"

    legacyClient.get({{"host", "localhost"}, {"port", 8080}, {"path", "/index.html"}}, [](int err) -> void {
        if (err != 0) {
            PLOG(ERROR) << "OnError: " << err;
        }
    }); // Connection:keep-alive\r\n\r\n"

    tlsClient.get({{"host", "localhost"}, {"port", 8088}, {"path", "/index.html"}}, [](int err) -> void {
        if (err != 0) {
            PLOG(ERROR) << "OnError: " << err;
        }
    }); // Connection:keep-alive\r\n\r\n"

    tlsClient.get({{"host", "localhost"}, {"port", 8088}, {"path", "/index.html"}}, [](int err) -> void {
        if (err != 0) {
            PLOG(ERROR) << "OnError: " << err;
        }
    }); // Connection:keep-alive\r\n\r\n"

    net::EventLoop::start();

    return 0;
}
