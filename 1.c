#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <semaphore.h>
#include <stdlib.h>

#define MAX 1024
#define FILENAME "1.txt"
#define SHM_NAME_1 "/my_shared_memory_1"
#define SIZE 4096
#define SEM1 "/semaphore_1"


int is_vowel(char c) {
    c = tolower((unsigned char)c);
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' || c == 'y');
}

void remove_vowels_and_save(const char *input) {
    FILE *fp = fopen(FILENAME, "a");
    if (!fp) {
        return;
    }

    char result[MAX];
    int j = 0;

    for (int i = 0; input[i] != '\0' && j < MAX - 1; i++) {
        if (!is_vowel(input[i])) {
            result[j++] = input[i];
        }
    }
    result[j] = '\0';

    fputs(result, fp);
    fputc('\n', fp);
    fclose(fp);
}

int main(int argc, char *argv[]) {
    sem_t *sem;
    sem = sem_open(SEM1, 0);

    int fd1 = shm_open(SHM_NAME_1,O_RDWR, 0666);
    if (fd1 == -1) {
        perror("shm_open");
        return 1;
    }

    if (ftruncate(fd1, SIZE) == -1) {
        perror("ftruncate");
        return 1;
    }

    void *ptr1 = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);
    if (ptr1 == MAP_FAILED) {
        perror("mmap - error of memory");
        return 1;
    }
    int idx = 2;
    while(1){
        sem_wait(sem);
        if (strcmp((char *)ptr1, "END\n") == 0) {
            return 0 ;
        }
        idx += 1;
        printf("Content in shared memory (1): %s", (char *)ptr1);
        remove_vowels_and_save((char* )ptr1);
    }
    munmap(ptr1, SIZE);
    close(fd1);
    sem_close(sem);
    return 0;
}