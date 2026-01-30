#include <cstdlib>
#include <cstdio>
#include <dlfcn.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <sys/wait.h>
#include <sys/mman.h>
#include <cstring>

namespace fs = std::filesystem;

enum class ServiceType : unsigned char {
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

void _onfail(int code) {
    pid_t forked_pid = fork();
    if (forked_pid) {
        while (true) {
            int status;
            pid_t exited_pid = waitpid(-1, &status, 0);
            if (exited_pid == forked_pid) exit(code);
        }
    }
    else {
        execv("/bin/bash", (char*[]){"/bin/bash", NULL});
        printf("sysq-bash: execv failed\n");
        exit(1);
    }
}

int main() {
    printf("sysq: started execution\n");

    printf("sysq: loading sysq_lib\n");
    void* sysq_lib = dlopen("/pboot/sysq-lib/sysqlib.so", RTLD_NOW);
    printf("sysq: obtained sysq_lib at %p\n", sysq_lib);
    if (sysq_lib == NULL) _onfail(1);

    parse_so = (parse_so_fn)dlsym(sysq_lib, "parse_so");
    printf("sysq: obtained sysq_lib->parse_so at %p\n", parse_so);
    if (parse_so == NULL) _onfail(1);

    get_files = (get_files_fn)dlsym(sysq_lib, "get_files");
    printf("sysq: obtained sysq_lib->get_files at %p\n", get_files);
    if (get_files == NULL) _onfail(1);

    std::vector<std::string> services = get_files("/pboot/systemq-data");

    if (services.empty()) {
        printf("sysq: no services found\nsysq: starting bash\n");
        _onfail(1);
    }

    ServiceMetadata* metadata = (ServiceMetadata*)mmap(NULL, services.size() * sizeof(ServiceMetadata), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);  // TODO: Make this use mmap() + Shared Memory
    memset(metadata, 0, services.size() * sizeof(ServiceMetadata));

    volatile size_t i = 0;
    for (std::string service_path : services) {
        volatile const char* volatile new_path = NULL;
        new_path = (volatile const char* volatile) mmap(NULL, _SC_PAGESIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
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
            memcpy((void*)new_path, md->path, strlen(md->path) + 1);
            metadata[i].path = (const char*)new_path;
            exit(0);
        }
        i++;
    }

    volatile unsigned char has_started_any_service = 0;
    volatile size_t service_no = services.size();

    for (size_t i = 0; i < service_no; i++) {
        ServiceMetadata* meta = metadata + i;
        if (!meta->path) continue;
        if (meta->boot_priority == 0) continue;
        if (meta->type == ServiceType::Type_BIN) {
            pid_t pid = fork();
            if (!pid) {
                execv(meta->path, (char*[]){(char*)meta->path, NULL});
            }
        }
    }
}
