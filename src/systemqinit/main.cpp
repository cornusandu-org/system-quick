#include <cstdlib>
#include <sys/stat.h>
#include <cstdio>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <sys/mount.h>

void* map_file_to_memory(const char *filename, size_t *size) {
    int fd = open(filename, O_RDONLY);
    if (fd < 0) return NULL;

    // Get size
    struct stat st;
    fstat(fd, &st);
    *size = st.st_size;

    // Map the file into memory
    void *map = mmap(NULL, *size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd); // We can close the file descriptor; the mapping stays

    if (map == MAP_FAILED) return NULL;
    return map;
}

int main() {
    mkdir("/dev/pts", 0755); 
    
    printf("mount() returned %d;\n", mount("devtmpfs", "/dev", "devtmpfs", 0, NULL) );
    printf("mount() returned %d;\n", mount("proc", "/proc", "proc", 0, NULL)        );
    printf("mount() returned %d;\n", mount("sysfs", "/sys", "sysfs", 0, NULL)       );
    printf("mount() returned %d;\n", mount("devpts", "/dev/pts", "devpts", 0, NULL) );
    

    const char* sysq_path = "/pboot/systemq/sysq";
    size_t size;

    void* sysq_content = map_file_to_memory(sysq_path, &size);

    if (sysq_content == NULL) {
        printf("systemqinit: mmap returned %p", sysq_content);
        exit(-1);
    }

    if (memcmp(sysq_content, (const void*)"\x7f\x45\x4c\x46", 4)) {
        printf("systemqinit: invalid sysq binary; must be ELF\n");
        exit(-1);
    }

    pid_t pid = fork();
    if (pid == 0) {
        printf("systemqinit: fully initialised sysq\n");
    } else {
        execv(sysq_path, (char *[]){(char*)sysq_path, NULL});
    }
}
