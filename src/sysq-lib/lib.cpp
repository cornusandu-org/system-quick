#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <system_error>

struct ServiceMetadata {
    unsigned char boot_priority;
    const char* path;
};

extern "C" ServiceMetadata* parse_so(const char* path) {
    return 0x0;
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
