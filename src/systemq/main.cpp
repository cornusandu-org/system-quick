#include <cstdlib>
#include <cstdio>
#include <dlfcn.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <sys/wait.h>

namespace fs = std::filesystem;

struct ServiceMetadata {
    unsigned char boot_priority;
    const char* path;
};

using parse_so_fn = ServiceMetadata*(*)(const char*);
using get_files_fn = std::vector<std::string>(*)(const char*);

parse_so_fn parse_so = NULL;
get_files_fn get_files = NULL;

void reap_zombies() {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {

    }
}

void reap_zombies_blocking() {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, 0)) > 0) {

    }
}

int main() {
    printf("sysq: started execution\n");

    printf("sysq: loading sysq_lib\n");
    void* sysq_lib = dlopen("/pboot/sysq-lib/sysqlib.so", RTLD_NOW);
    printf("sysq: obtained sysq_lib at %p\n", sysq_lib);
    if (sysq_lib == NULL) exit(-1);

    parse_so = (parse_so_fn)dlsym(sysq_lib, "parse_so");
    printf("sysq: obtained sysq_lib->parse_so at %p\n", parse_so);
    if (parse_so == NULL) exit(-1);

    get_files = (get_files_fn)dlsym(sysq_lib, "get_files");
    printf("sysq: obtained sysq_lib->get_files at %p\n", get_files);
    if (get_files == NULL) exit(-1);
}
