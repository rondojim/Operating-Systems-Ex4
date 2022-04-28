#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

char** LS(char* dir){
    struct dirent **names;
    int sz = scandir(dir, &names, NULL, alphasort);
    if(sz == -1) return NULL;
    char **namesl = malloc(sizeof(char*) * sz);
    int c = 0;

    namesl[0] = malloc(sizeof(char) * 20);

    
    for(int i=0; i<sz; ++i){
        if(names[i]->d_name[0] == '.'){
            free(names[i]);
            continue;
        }
        c++;
        namesl[c] = malloc(strlen(names[i]->d_name) * sizeof(char));
        strcpy(namesl[c], names[i]->d_name);
        free(names[i]);
    }
    free(names);

    sprintf(namesl[0], "%d", c);
    return namesl;
}

int is_directory(char *path){
    struct stat info;
    stat(path, &info);

    return S_ISDIR(info.st_mode);
}

char* concPath(char *s1, char *s2){
    char *s3 = malloc((strlen(s1) + strlen(s2) + 5) * sizeof(char));
    sprintf(s3, "%s/%s", s1, s2);
    return s3;
}

int same(char *path1, char *path2){
    struct stat a;
    struct stat t;

    stat(path1, &a);
    stat(path2, &t);

    if(a.st_size != t.st_size)
        return 0;
    
    return (t.st_mtime >= a.st_mtime);
}

#define SIZE 30
#define PERM 0644

/* ##################################################################
    
    o parakatw kwdikas gia to mycopyfile einai apo tis 
                parousiaseis tou kurioy Deli 

   ################################################################# */

int mycopyfile(char *name1, char *name2, int BUFFSIZE){
    int infile, outfile;
    ssize_t nread;
    char buffer[BUFFSIZE];

    if((infile = open(name1, O_RDONLY)) == -1)
        return -1;
    
    if((outfile = open(name2, O_WRONLY | O_CREAT | O_TRUNC, PERM)) == -1)
        return -2;

    while((nread = read(infile, buffer, BUFFSIZE)) > 0){
        if(write(outfile, buffer, nread) < nread){
            close(infile);
            close(outfile);
            return -3;
        }
    }
    close(infile);
    close(outfile);
    if(nread == -1) return -4;
    return 0;
}

/* ##################################################################
    
            Edw teleiwnei o kwdikas tou kuriou deli

   ################################################################# */

int getsize(char *path){
    struct stat a;
    stat(path, &a);

    return a.st_size;
}

int copy(char *path1, char *name, char *path2, int *total, int *bytes){
    int cnt_copy = 0;
    //printf("%s %s %s\n", path1, name, path2);
    char *pathname1 = concPath(path1, name);
    char *pathname2 = concPath(path2, name);

    //printf("Pathnames: %s %s %s\n", path1, name, path2);
    if(is_directory(pathname1)){
        //puts("it is a directory");
        if(!is_directory(path2)){
            printf("Error, cant copy folder to file\n");
            return 0;
        }
        // copy folder to folder
        mkdir(pathname2, 0777);
        *bytes += getsize(pathname2);
        cnt_copy++;
        printf("Created directory %s\n", pathname2);    
        char **names = LS(pathname1);
        int sz = atoi(names[0]);
        *total += sz;
        for(int i=1; i<=sz; ++i){
            cnt_copy += copy(pathname1, names[i], pathname2, total, bytes);
        }
    }
    else{
        //puts("it is not a directory");
        if(is_directory(path2)){
            // copy file to folder
            //puts("[it is a directory]");
            cnt_copy += copy(path1, name, pathname2, total, bytes);
        }
        else{
            // copy file to file
            char *namepath = concPath(path1, name);
            mycopyfile(namepath, path2, 4096);
            *bytes += getsize(path2);
            cnt_copy++;
            printf("%s\n", name);
        }
    }
    return cnt_copy;
}


void ls_remove(char *path){
    if(!is_directory(path)){
        remove(path);
        return;
    }
    char **names = LS(path);
    int sz = atoi(names[0]);
    for(int i=1; i<=sz; ++i){
        char *namepath = concPath(path, names[i]);
        if(is_directory(namepath)) ls_remove(namepath);
        else remove(namepath);
    }
    remove(path);
}

int quic(char *origindir, char *destdir, int *total, int *bytes, int Delete){
    char **names1 = LS(origindir);
    char **names2 = LS(destdir);
    int sz1 = 0, sz2 = 0;
    if(names1) sz1 = atoi(names1[0]);
    if(names2) sz2 = atoi(names2[0]);
    *total += sz1;

    int cnt_copy = 0;

    for(int i=1; i<=sz1; ++i){
        char *path1 = concPath(origindir, names1[i]);
        struct stat info1;
        struct stat info2;
        int found = 0;
        stat(path1, &info1);
        char *path2;
        
        for(int j=1; j<=sz2; ++j){
            path2 = concPath(destdir, names2[j]);
            stat(path2, &info2);
            //printf("---[%s] %ld %ld\n", path2, info1.st_ino, info2.st_ino);
            //printf("[%s] [%s]\n", names1[i], names2[j]);
            if(strcmp(names1[i], names2[j]) == 0){
                found = 1;
                break;
            }
        }

        //printf("%s found %d\n", path1, found);
        if(!found){
            cnt_copy += copy(origindir, names1[i], destdir, total, bytes);
        }
        else{
            if(is_directory(path1)) quic(path1, path2, total, bytes, Delete);
            else if(!same(path1, path2)){
                cnt_copy += copy(origindir, names1[i], destdir, total, bytes);
            }
        }
    }

    if(Delete){
        for(int j=1; j<=sz2; ++j){
            char *path2 = concPath(destdir, names2[j]);
            char *path1;
            int found = 0;
            for(int i=1; i<=sz1; ++i){
                path1 = concPath(origindir, names1[i]);
                if(strcmp(names1[i], names2[j]) == 0){
                    found = 1;
                    break;
                }
            }
            if(!found){
                printf("not found %s %s\n", names2[j], path2);
                ls_remove(path2);
            }
        }
    }
    return cnt_copy;
}
