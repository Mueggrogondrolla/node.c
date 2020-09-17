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

#include "legacy/WebApp.h"
#include "middleware/StaticMiddleware.h"
#include "tls/WebApp.h"

#include <easylogging++.h>
#include <openssl/x509v3.h>

#endif /* DOXYGEN_SHOULD_SKIP_THIS */

#define CERTF "/home/voc/projects/ServerVoc/certs/calisto.home.vchrist.at_-_snode.c_-_server.pem"
#define KEYF "/home/voc/projects/ServerVoc/certs/Volker_Christian_-_Web_-_snode.c_-_server.key.encrypted.pem"
#define KEYFPASS "snode.c"
#define CLIENTCAFILE "/home/voc/projects/ServerVoc/certs/Volker_Christian_-_Root_CA.crt"

#define SERVERROOT "/home/voc/projects/ServerVoc/doc/html"

using namespace express;

Router routes() {
    Router router;

    router.get("/test/:variable(\\d)/:uri", [] APPLICATION(req, res) {
        std::cout << "TEST" << std::endl;
    });

    // http://localhost:8080/account/123/perfectNDSgroup
    router.get("/account/:userId(\\d*)/:username", [] APPLICATION(req, res) {
        VLOG(0) << "Show account of";
        VLOG(0) << "UserId: " << req.params["userId"];
        VLOG(0) << "UserName: " << req.params["userName"];

        std::string response = "<html>"
                               "  <head>"
                               "    <title>Response from snode.c</title>"
                               "  </head"
                               "  <body>"
                               "    <h1>Regex return</h1>"
                               "    <ul>"
                               "      <li>UserId: " +
                               req.params["userId"] +
                               "      </li>"
                               "      <li>UserName: " +
                               req.params["userName"] +
                               "      </li>"
                               "    </ul>"
                               "  </body>"
                               "</html>";

        res.send(response);
    });

    // http://localhost:8080/asdf/d123e/jklö/hallo
    router.get("/asdf/:testRegex1(d\\d{3}e)/jklö/:testRegex2", [] APPLICATION(req, res) {
        VLOG(0) << "Testing Regex";
        VLOG(0) << "Regex1: " << req.params["testRegex1"];
        VLOG(0) << "Regex2: " << req.params["testRegex2"];

        std::string response = "<html>"
                               "  <head>"
                               "    <title>Response from snode.c</title>"
                               "  </head"
                               "  <body>"
                               "    <h1>Regex return</h1>"
                               "    <ul>"
                               "      <li>Regex 1: " +
                               req.params["testRegex1"] +
                               "      </li>"
                               "      <li>Regex 2: " +
                               req.params["testRegex2"] +
                               "      </li>"
                               "    </ul>"
                               "  </body>"
                               "</html>";

        res.send(response);
    });

    // http://localhost:8080/search/buxtehude123
    router.get("/search/:search", [] APPLICATION(req, res) {
        VLOG(0) << "Show Search of";
        VLOG(0) << "Search: " << req.params["search"];
        VLOG(0) << "Queries: " << req.query("test");

        res.send(req.params["search"]);
    });

    router.use([] APPLICATION(req, res) {
        res.status(404).send("Not found: " + req.url);
    });

    return router;
}

int main(int argc, char** argv) {
    WebApp::init(argc, argv);

    legacy::WebApp legacyApp;

    legacyApp.use(routes());

    legacyApp.listen(8080, [](int err) -> void {
        if (err != 0) {
            PLOG(FATAL) << "listen on port 8080 " << std::to_string(err);
        } else {
            VLOG(0) << "snode.c listening on port 8080 for legacy connections";
        }
    });

    legacyApp.onConnect([](net::socket::legacy::SocketConnection* socketConnection) -> void {
        VLOG(0) << "OnConnect:";
        VLOG(0) << "\tClient: " + socketConnection->getRemoteAddress().host() + "(" + socketConnection->getRemoteAddress().ip() +
                       "):" + std::to_string(socketConnection->getRemoteAddress().port());
        VLOG(0) << "\tServer: " + socketConnection->getLocalAddress().host() + "(" + socketConnection->getLocalAddress().ip() +
                       "):" + std::to_string(socketConnection->getLocalAddress().port());
    });

    legacyApp.onDisconnect([](net::socket::legacy::SocketConnection* socketConnection) -> void {
        VLOG(0) << "OnDisconnect:";
        VLOG(0) << "\tClient: " + socketConnection->getRemoteAddress().host() + "(" + socketConnection->getRemoteAddress().ip() +
                       "):" + std::to_string(socketConnection->getRemoteAddress().port());
        VLOG(0) << "\tServer: " + socketConnection->getLocalAddress().host() + "(" + socketConnection->getLocalAddress().ip() +
                       "):" + std::to_string(socketConnection->getLocalAddress().port());
    });

    tls::WebApp tlsApp({{"certChain", CERTF}, {"keyPEM", KEYF}, {"password", KEYFPASS}});

    tlsApp.use(routes());

    tlsApp.listen(8088, [](int err) -> void {
        if (err != 0) {
            PLOG(FATAL) << "listen on port 8088 " << std::to_string(err);
        } else {
            VLOG(0) << "snode.c listening on port 8088 for SSL/TLS connections";
        }
    });

    tlsApp.onConnect([](net::socket::tls::SocketConnection* socketConnection) -> void {
        VLOG(0) << "OnConnect:";
        VLOG(0) << "\tServer: " + socketConnection->getLocalAddress().host() + "(" + socketConnection->getLocalAddress().ip() +
                       "):" + std::to_string(socketConnection->getLocalAddress().port());
        VLOG(0) << "\tClient: " + socketConnection->getRemoteAddress().host() + "(" + socketConnection->getRemoteAddress().ip() +
                       "):" + std::to_string(socketConnection->getRemoteAddress().port());

        X509* client_cert = SSL_get_peer_certificate(socketConnection->getSSL());

        if (client_cert != NULL) {
            int verifyErr = SSL_get_verify_result(socketConnection->getSSL());

            VLOG(0) << "\tClient certificate: " + std::string(X509_verify_cert_error_string(verifyErr));

            char* str = X509_NAME_oneline(X509_get_subject_name(client_cert), 0, 0);
            VLOG(0) << "\t   Subject: " + std::string(str);
            OPENSSL_free(str);

            str = X509_NAME_oneline(X509_get_issuer_name(client_cert), 0, 0);
            VLOG(0) << "\t   Issuer: " + std::string(str);
            OPENSSL_free(str);

            // We could do all sorts of certificate verification stuff here before deallocating the certificate.

            GENERAL_NAMES* subjectAltNames = static_cast<GENERAL_NAMES*>(X509_get_ext_d2i(client_cert, NID_subject_alt_name, NULL, NULL));

            int32_t altNameCount = sk_GENERAL_NAME_num(subjectAltNames);
            VLOG(0) << "\t   Subject alternative name count: " << altNameCount;
            for (int32_t i = 0; i < altNameCount; ++i) {
                GENERAL_NAME* generalName = sk_GENERAL_NAME_value(subjectAltNames, i);
                if (generalName->type == GEN_URI) {
                    std::string subjectAltName =
                        std::string(reinterpret_cast<const char*>(ASN1_STRING_get0_data(generalName->d.uniformResourceIdentifier)),
                                    ASN1_STRING_length(generalName->d.uniformResourceIdentifier));
                    VLOG(0) << "\t      SAN (URI): '" + subjectAltName;
                } else if (generalName->type == GEN_DNS) {
                    std::string subjectAltName = std::string(reinterpret_cast<const char*>(ASN1_STRING_get0_data(generalName->d.dNSName)),
                                                             ASN1_STRING_length(generalName->d.dNSName));
                    VLOG(0) << "\t      SAN (DNS): '" + subjectAltName;
                } else {
                    VLOG(0) << "\t      SAN (Type): '" + std::to_string(generalName->type);
                }
            }
            sk_GENERAL_NAME_pop_free(subjectAltNames, GENERAL_NAME_free);

            X509_free(client_cert);
        } else {
            VLOG(0) << "\tClient certificate: no certificate";
        }
    });

    tlsApp.onDisconnect([](net::socket::tls::SocketConnection* socketConnection) -> void {
        VLOG(0) << "OnDisconnect:";
        VLOG(0) << "\tServer: " + socketConnection->getLocalAddress().host() + "(" + socketConnection->getLocalAddress().ip() +
                       "):" + std::to_string(socketConnection->getLocalAddress().port());
        VLOG(0) << "\tClient: " + socketConnection->getRemoteAddress().host() + "(" + socketConnection->getRemoteAddress().ip() +
                       "):" + std::to_string(socketConnection->getRemoteAddress().port());
    });

    WebApp::start();

    return 0;
}
