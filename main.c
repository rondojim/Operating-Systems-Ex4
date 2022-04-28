#include "quic.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

int main(int argc, char *argv[]){
    char *origindir, *destdir;
    int ok = 0, Delete = 0;
    for(int i=1; i<argc; ++i){
        if(argv[i][0] != '-'){
            if(!ok){
                origindir = malloc(sizeof(char) * (strlen(argv[i]) + 5));
                strcpy(origindir, argv[i]);
                ok = 1;
            }
            else{
                destdir = malloc(sizeof(char) * (strlen(argv[i]) + 5));
                strcpy(destdir, argv[i]);
            }
        }
        else if(argv[i][1] == 'd') Delete = 1;
    }

    mkdir(destdir, 0777);
    int total = 0, bytes = 0;
    clock_t begin = clock();

    int copied = quic(origindir, destdir, &total, &bytes, Delete);

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    printf("there are %d file/directories in the hierachy\n", total);
    printf("number of entities copied is %d\n", copied);
    printf("copied %d bytes in %lf at %lf bytes/sec\n", bytes, time_spent, bytes/time_spent);
    return 0;
}