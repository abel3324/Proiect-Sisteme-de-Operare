#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h> 

#define PATH_LEN 256

//1 - inspector , 2 - manager , -1 - rol necunoscut 
int getRole(char *argv[],int argc){
    if(argc < 2){
        return -1;
    }
    if(strcmp(argv[1],"--role") == 0){
        if(strcmp(argv[2],"inspector") == 0){
            return 1;
        }else{
            if(strcmp(argv[2],"manager") == 0){
                return 2;
            }
        }
    }
    return -1;
}
void printRole(int n){
    if (n == 1){
        printf("inspector \n");
        return;
    }
    if (n == 2){
        printf("manager \n");
        return;
    }
    if(n == -1){
        printf("rol invalid \n");
        return;
    }
}
char* getUser(char *argv[],int argc){
    if(argc < 4){
        printf("Not enough arguments !!!");
        exit(-1);
    }
    if(strcmp(argv[3],"--user") == 0){
        return argv[4];
    }
    return "Invalid Format!!!!";
}
/*
    Command ID:
    error = -1
    add = 1
    list = 2
    view = 3
    remove_report = 4 
    update_threshold = 5 
    filter = 6
*/

int commandSelect(char *argv[],int argc){
    if (argc < 6){
        printf("Not enough arguments !!!");
        exit(-1);
    }
    if (strcmp(argv[5], "--add") == 0){
        return 1;
    }
    if (strcmp(argv[5], "--list") == 0){
        return 2;
    }
    if (strcmp(argv[5], "--view") == 0){
        return 3;
    }
    if (strcmp(argv[5], "--remove_report") == 0){
        return 4;
    }
    if (strcmp(argv[5], "--update_threshold") == 0){
        return 5;
    }
    if (strcmp(argv[5], "--filter") == 0){
        return 6;
    }
    return -1;
}
void printCommand(int n){
    if (n == 1){
        printf("add\n");
    }
    if (n == 2){
        printf("list\n");
    }
    if (n == 1){
        printf("view\n");
    }
    if (n == 1){
        printf("remove_report\n");
    }
    if (n == 1){
        printf("update_threshold\n");
    }
    if (n == 1){
        printf("filter\n");
    }
}


int main(int argc, char *argv[])
{   
    int n = getRole(argv,argc);
    printRole(n);

    char *name;
    name = getUser(argv,argc);

    printf("%s \n",name);

    int m = commandSelect(argv,argc);
    printCommand(m);

    
    return 0;
}