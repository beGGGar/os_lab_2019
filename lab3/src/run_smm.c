#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
// #include <sys/types.h>
// #include <sys/wait.h>

int main() {
    // pid_t pid = fork(); 
    // if (pid == 0) {
        execl("./sequential_min_max", 
            "sequential_min_max", 
            "4", "100", (char *)NULL);
        
        perror("Ошибка при запуске sequential_min_max");
    //     exit(1);
    // } else {
    //     // Ошибка при создании процесса
    //     perror("Ошибка при создании процесса");
    //     exit(1);
    // }

    return 0;
}