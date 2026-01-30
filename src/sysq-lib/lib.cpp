#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <system_error>
#include <dlfcn.h>
#include <cstdlib>
#include <cstring>

enum class ServiceType : unsigned char {
    Type_BIN = 1,
    Type_SO  = 2,
};

struct ServiceMetadata {
    unsigned char boot_priority;
    const char* path;
    ServiceType type;
};

extern "C" ServiceMetadata* parse_so(void* handle) {
    void* tmp = NULL;
    ServiceMetadata *data = (ServiceMetadata*)malloc(sizeof(ServiceMetadata));
    memset(data, 0, sizeof(ServiceMetadata));

    tmp = dlsym(handle, "boot_priority");
    if (tmp) data->boot_priority = *(char*)tmp;
    else     data->boot_priority = 1;

    // TODO: There is no way this works. Fix it.
    tmp = (void*)*(char**)dlsym(handle, "path");
    if (tmp && data->path == NULL) data->path = (const char*)tmp;
    tmp = (void*)*(char**)dlsym(handle, "bpath");
    if (tmp && data->path == NULL) data->path = (const char*)tmp;
    tmp = (void*)*(char**)dlsym(handle, "loc");
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

namespace fs = std::filesystem;

extern "C" std::vector<std::string> get_files(const char* path) {
    std::vector<std::string> files;
    std::error_code ec;

    // Passing 'ec' to the constructor prevents exceptions if the path is invalid
    auto it = fs::directory_iterator(path, ec);
    if (ec) return {}; // Return empty vector on error

    for (const auto& entry : it) {
        if (fs::is_regular_file(entry, ec)) {
            files.push_back(entry.path().filename().string());
        }
    }

    return files;
}
