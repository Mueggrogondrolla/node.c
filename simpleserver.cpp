#include <iostream>
#include <dlfcn.h>
#ifndef DOXYGEN_SHOULD_SKIP_THIS

#include "./http/WebApp.h"
#include "./net/Importer.h"

#endif /* DOXYGEN_SHOULD_SKIP_THIS */


int simpleWebserver(int argc, char** argv) {
    WebApp& app = WebApp::instance("/home/voc/projects/ServerVoc/build/html");
    
    app.use("/",
            [&] (const Request& req, const Response& res, const std::function<void (void)>& next) {
                res.set("Connection", "Keep-Alive");
                next();
            });
                
    
    app.get("/",
    [&] (const Request& req, const Response& res) -> void {
        std::string uri = req.originalUrl;
                std::cout << "URL: " << uri << std::endl;
                std::cout << "Cookie: " << req.cookie("rootcookie") << std::endl;
                res.cookie("searchcookie", "cookievalue", {{"Max-Age", "3600"}, {"Path", "/search"}});
//                res.clearCookie("rootcookie");
//                res.clearCookie("rootcookie");
//                res.clearCookie("searchcookie", {{"Path", "/search"}});
        if (uri == "/") {
            res.redirect("/index.html");
        } else if (uri == "/end") {
            app.stop();
        } else {
            res.sendFile(uri);
        }
    });

    Router router;
    router.get("/search",
    [&] (const Request& req, const Response& res) -> void {
        std::cout << "URL: " << req.originalUrl << std::endl;
        std::cout << "Cookie: " << req.cookie("searchcookie") << std::endl;
        res.sendFile(req.originalUrl);
    });

    app.get("/", router);
    
    #define CERTF "/home/voc/projects/ServerVoc/certs/Volker_Christian_-_Web_-_snode.c.pem"
    #define KEYF "/home/voc/projects/ServerVoc/certs/Volker_Christian_-_Web_-_snode.c.key.pem"
    
    app.listen(8080,
    [&] (int err) -> void {
        if (err != 0) {
            perror("Listen");
        } else {
            std::cout << "snode.c listening on port 8080" << std::endl;
            app.sslListen(8088, CERTF, KEYF, "password",
            [] (int err) -> void {
                if (err != 0) {
                    perror("Listen");
                } else {
                    std::cout << "snode.c listening on port 8088" << std::endl;
                }
            });
        }
    });


    app.destroy();

    return 0;
}

/**
 * Tests the importer class
 */
void importerTests()
{
    /*
     * good cases
     */
    
    // test for an existing library
    std::cout << "Library 'libm.so' exists: " << Importer::LibraryExists("libm.so") << std::endl;
    
    // get the importer
    Importer importer = Importer("libm.so");

    // declare the type of the function pointer for a wanted function
    double (*cosine)(double);
    
    // get the pointer to the function
    *(void **) (&cosine) = importer.GetFunctionPointer("cos");
    std::cout << (*cosine)(2.0) << std::endl; // execute the funtion pointer
    
    // look, for if a function, we know exists, to exist
    std::cout << "Function 'cos' exists: " << importer.FunctionExists("cos") << std::endl; 
    
    
    
    
    /*
     * error cases
     */
    
    // look, for if a function, we know does not exist, to exist
    std::cout << "Function 'cors' exists: " << importer.FunctionExists("cors") << std::endl; 
    
    // try to execute a function with the wrong function pointer type
    *(void **) (&cosine) = importer.GetFunctionPointer("cosf");
    std::cout << (*cosine)(2.0) << std::endl; // execute the funtion pointer
    
    // test for a non-existing library
    std::cout << "Library 'Testlisb' exists: " << Importer::LibraryExists("Testlisb") << std::endl;
    
    // Try to execute a non void function directly
    importer.ExecuteFunction("cosf");
}


int main(int argc, char** argv) {
    importerTests();
    
    return simpleWebserver(argc, argv);
}
