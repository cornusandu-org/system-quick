#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <system_error>
#include <dlfcn.h>
#include <cstdlib>
#include <cstring>

#if defined(__GNUC__) || defined(__clang__)
    #define EXPORT extern "C" __attribute__((visibility("default"), aligned(64)))
#elif defined(_MSC_VER)
    #define EXPORT extern "C" __declspec(dllexport) __declspec(align(64))
    #else
    #define EXPORT extern "C"
#endif


enum class ServiceType : unsigned char {
    Type_BIN = 1,
    Type_SO  = 2,
};

struct ServiceMetadata {
    unsigned char boot_priority;
    const char* path;
    ServiceType type;
};

EXPORT ServiceMetadata* parse_so(void* handle) {
    void* tmp = NULL;
    ServiceMetadata *data = (ServiceMetadata*)malloc(sizeof(ServiceMetadata));
    memset(data, 0, sizeof(ServiceMetadata));

    tmp = dlsym(handle, "boot_priority");
    if (tmp) data->boot_priority = *(char*)tmp;
    else     data->boot_priority = 1;

    // TODO: There is no way this works. Fix it.
    tmp = dlsym(handle, "path");
    if (tmp && data->path == NULL) data->path = (const char*)tmp;
    tmp = dlsym(handle, "bpath");
    if (tmp && data->path == NULL) data->path = (const char*)tmp;
    tmp = dlsym(handle, "loc");
    if (tmp && data->path == NULL) data->path = (const char*)tmp;

    if (data->path == NULL) return 0x0;

    tmp = dlsym(handle, "type");
    if (!tmp) data->type = ServiceType::Type_BIN;
    else {
        if (*(unsigned char*)tmp ==      (unsigned char)ServiceType::Type_BIN)     data->type = ServiceType::Type_BIN;
        else if (*(unsigned char*)tmp == (unsigned char)ServiceType::Type_SO) data->type = ServiceType::Type_SO;
        else                                                 data->type = ServiceType::Type_BIN;     
    }

    return data;
}


