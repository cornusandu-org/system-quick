struct ServiceMetadata {
    unsigned char boot_priority;
    const char* path;
};

extern "C" ServiceMetadata* parse_so(const char* path) {
    return 0x0;
}
