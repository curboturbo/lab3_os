#include <stdio.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define MAX 1024
#define SHM_NAME_1 "/my_shared_memory_1"
#define SHM_NAME_2 "/my_shared_memory_2"
#define SIZE 4096

#define SEM1 "/semaphore_1"
#define SEM2 "/semaphore_2"

int probability() {
    int prob = rand() % 10;
    return (prob < 8) ? 1 : 0;  
}

int main(){
    int fd1 = shm_open(SHM_NAME_1, O_CREAT | O_RDWR, 0666);
    int fd2 = shm_open(SHM_NAME_2, O_CREAT | O_RDWR, 0666);

    if (fd1 == -1 || fd2 == -1) {
        perror("shm_open");
        return 1;
    }

    if (ftruncate(fd1, SIZE) == -1 || ftruncate(fd2, SIZE) == -1) {
        perror("ftruncate");
        return 1;
    }

    void *ptr1 = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);
    void *ptr2 = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);
    if (ptr1 == MAP_FAILED || ptr2 == MAP_FAILED) {
        perror("mmap - error of memory");
        return 1;
    }


    sem_t *sem = sem_open(SEM1, O_CREAT, 0666, 0);
    sem_t *sem2 = sem_open(SEM2,O_CREAT,0666,0);
    
    pid_t cp1 = fork();
    if (cp1 == 0){
        printf("was loadede\n");
        execlp("./1", "1", NULL);
        perror("C1: execlp 1 failed");
        printf("was endedn");
        exit(1);
    } else {
        pid_t cp2 = fork();
        if (cp2 == 0){
            execlp("./2", "2", NULL);
            perror("C2: execlp 2 failed");
            exit(1);
        } else {
            char input[MAX];
            while (fgets(input, MAX, stdin)) { 
                input[strcspn(input, "\n")] = '\n';
                if (strcmp(input, "END\n") == 0) {
                    strcpy(ptr1, "END\n");
                    sem_post(sem);
                    strcpy(ptr2, "END\n");
                    sem_post(sem2);
                    break;
                }
                if (probability() == 1) {
                    printf("send data first file -> \n");
                    strcpy(ptr1, input);
                    sem_post(sem);
                }
                else {
                    printf("send data second file -> \n");
                    strcpy(ptr2, input);
                    sem_post(sem2);
                }
            }
            wait(NULL);
            wait(NULL);
        }
    }

    if (munmap(ptr1, SIZE) == -1 || munmap(ptr2, SIZE) == -1) {
        perror("munmap");
        return 1;
    }
    close(fd1);
    close(fd2);
    shm_unlink(SHM_NAME_1);
    shm_unlink(SHM_NAME_2);

    return 0;
}
