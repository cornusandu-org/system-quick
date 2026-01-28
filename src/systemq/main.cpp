#include <cstdlib>
#include <cstdio>
#include <dlfcn.h>

struct ServiceMetadata {
    unsigned char boot_priority;
    const char* path;
};

using parse_so_fn = ServiceMetadata*(*)(const char*);

parse_so_fn parse_so = NULL;

int main() {
    printf("sysq: started execution\n");

    printf("sysq: loading sysq_lib\n");
    void* sysq_lib = dlopen("/pboot/sysq-lib/sysqlib.so", RTLD_NOW);
    printf("sysq: obtained sysq_lib at %p\n", sysq_lib);
    if (sysq_lib == NULL) exit(-1);
    parse_so = (parse_so_fn)dlsym(sysq_lib, "parse_so");
    printf("sysq: obtained sysq_lib->parse_so at %p\n", parse_so);
    if (parse_so == NULL) exit(-1);
}
