#include <cstdlib>
#include <cstdio>
#include <dlfcn.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <sys/wait.h>

namespace fs = std::filesystem;

enum class ServiceType : size_t {
    Type_BIN = 1,
    Type_SO  = 2,
};

struct ServiceMetadata {
    unsigned char boot_priority;
    const char* path;
    ServiceType type;
};

using parse_so_fn = ServiceMetadata*(*)(void*);
using get_files_fn = std::vector<std::string>(*)(const char*);

parse_so_fn parse_so = NULL;
get_files_fn get_files = NULL;

pid_t reap_zombies() {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        return pid;
    }
    return 0;
}

pid_t reap_zombies_blocking() {
    int status;
    pid_t pid;

    pid = waitpid(-1, &status, 0);
    return pid;
}

int main() {
    printf("sysq: started execution\n");

    printf("sysq: loading sysq_lib\n");
    void* sysq_lib = dlopen("/pboot/sysq-lib/sysqlib.so", RTLD_NOW);
    printf("sysq: obtained sysq_lib at %p\n", sysq_lib);
    if (sysq_lib == NULL) exit(1);

    parse_so = (parse_so_fn)dlsym(sysq_lib, "parse_so");
    printf("sysq: obtained sysq_lib->parse_so at %p\n", parse_so);
    if (parse_so == NULL) exit(1);

    get_files = (get_files_fn)dlsym(sysq_lib, "get_files");
    printf("sysq: obtained sysq_lib->get_files at %p\n", get_files);
    if (get_files == NULL) exit(1);

    std::vector<std::string> services = get_files("/pboot/systemq-data");

    if (services.empty()) {
        printf("sysq: no services found\nsysq: starting bash\n");
        if (fork()) {
            while (reap_zombies_blocking()) {;;}
        }
        else {
            execv("/bin/bash", (char*[]){"/bin/bash", NULL});
            printf("sysq-bash: execv failed\n");
            exit(1);
        }
    }

    ServiceMetadata* metadata = (ServiceMetadata*)calloc(services.size(), sizeof(ServiceMetadata));  // TODO: Make this use mmap() + Shared Memory

    volatile size_t i = 0;
    for (std::string service_path : services) {
        pid_t read_pid = fork();
        if (read_pid) {
            int status;
            waitpid(read_pid, &status, 0);
        } else {
            void* service = dlopen(service_path.c_str(), RTLD_NOW);
            if (!service) exit(1);
            ServiceMetadata* md = parse_so(service);
            if (!md) exit(1);
            metadata[i] = *md;
            exit(0);
        }
        i++;
    }
}
