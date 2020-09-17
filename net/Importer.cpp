#include "Importer.h"
#include <iostream>

Importer::Importer(const char* libraryName, int flag)
{
    handle = dlopen(libraryName, flag);
}

void* Importer::GetFunctionPointer(const char* functionName)
{
    return dlsym(handle, functionName);
}

void Importer::ExecuteFunction(const char* functionName)
{
    char* error;
    
    void (*function)();
    *(void **) (&function) = GetFunctionPointer(functionName);
    
    if ((error = dlerror()) != NULL)
    {
        std::cout << error << std::endl;
    }
    else
    {
        (*function)();
    }
}

bool Importer::FunctionExists(const char* functionName)
{
    void (*function)();
    *(void **) (&function) = GetFunctionPointer(functionName);
    
    return dlerror() == NULL;
}

/**
 * known issue, this function always returns true. (The dlerror states, that the ELF header is invalid, but I have no idea why)
 */
bool Importer::LibraryExists(const char* libraryName)
{
    return dlopen(libraryName, RTLD_LAZY);
}
