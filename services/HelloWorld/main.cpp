#include <cstdio>
#include <unistd.h>

int main() {
    printf("Hello, world!");
    execv("/bin/bash", (char*[]){"/bin/bash", NULL});
}
