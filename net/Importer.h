#ifndef IMPORTER_H
#define IMPORTER_H

#include <dlfcn.h>

/**
 * A class, that allows importing of other libraries by their name during runtime.
 * For the library to be successfully loaded, it has to be either in a standard lib folder or reachable via a relative path from the executable.
 */
class Importer
{
public:
    /**
     * Default constructor.
     * Loads the library and stores a reference to it in the handle variable.
     * When the library is loaded can be influenced by the flag. By default, the library is not actually loaded until it is needed (lazy loading).
     */
    Importer(const char* libraryName, int flag = RTLD_LAZY);
    
    /**
     * Tries to get a function with a given name from the defined library.
     * Returns a void function pointer to the loaded function. If no function with that name can be found, NULL is returned.
     */
    void* GetFunctionPointer(const char* functionName);
    
    /**
     * Tries to get the specified function and immediately execute it.
     * If the function can not be found or requires arguments, the execution is skipped.
     */
    void ExecuteFunction(const char* functionName);
    
    /**
     * Looks in the library, whether a function with a given name exists or not.
     */
    bool FunctionExists(const char* functionName);
    
    /**
     * Looks, if a library with a given name is reachable for the executable.
     */
    static bool LibraryExists(const char* libraryName);

private:
    void * handle;
};

#endif // IMPORTER_H
