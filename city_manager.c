#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h> 

#define PATH_LEN 256
//FILTER
//1 - inspector , 2 - manager , -1 - rol necunoscut 
int getRole(char *argv[], int argc) {
    if (argc < 2) {
        return -1;
    }
    if (strcmp(argv[1], "--role") == 0) {
        if (strcmp(argv[2], "inspector") == 0) {
            return 1;
        } else if (strcmp(argv[2], "manager") == 0) {
            return 2;
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

int commandSelect(char *argv[], int argc) {
    if (argc < 6) {
        printf("Not enough arguments !!!\n\n\n\n");
        return -1;
    }
    if (strcmp(argv[5], "--add") == 0) {
        return 1;
    }
    if (strcmp(argv[5], "--list") == 0) {
        return 2;
    }
    if (strcmp(argv[5], "--view") == 0) {
        return 3;
    }
    if (strcmp(argv[5], "--remove_report") == 0) {
        return 4;
    }
    if (strcmp(argv[5], "--update_threshold") == 0) {
        return 5;
    }
    if (strcmp(argv[5], "--filter") == 0) {
        return 6;
    }
    return -1;
}
void printCommand(int n) {
    switch(n) {
        case 1:
            printf("add\n");
            break;
        case 2:
            printf("list\n");
            break;
        case 3:
            printf("view\n");
            break;
        case 4:
            printf("remove_report\n");
            break;
        case 5:
            printf("update_threshold\n");
            break;
        case 6:
            printf("filter\n");
            break;
        default:
            printf("Invalid command !!! \n");
            break;
    }
}
//FILTER
//CREATE DISTRICT 
int createDistrict(const char* districtID,const char* userRole){
    char dir_path[PATH_LEN];
    snprintf(dir_path, sizeof(dir_path), "./%s", districtID);

    //verificam daca exista deja directorul
    struct stat st = {0};
    if(stat(dir_path,&st) == -1){
        //cream directorul
        if(mkdir(dir_path,0750) == -1){
            perror("Error creating directory ! \n");
            return -1;
        }
    } 
    //cream fisierele necesare
    char report_path[PATH_LEN];
    snprintf(report_path,sizeof(report_path),"%s/reports.dat",dir_path);
    int report_fd = open(report_path,O_CREAT | O_RDWR,0644);
    if(report_fd == -1){
        perror("Error creating reports.dat");
        return -1;
    }
    close(report_fd);

    //Cream fisierul district.cfg
    char cfg_path[PATH_LEN];
    snprintf(cfg_path,sizeof(cfg_path),"%s/distrcit.cfg",dir_path);
    int cfg_fd = open(cfg_path,O_CREAT | O_RDWR,0640);
    if(cfg_fd == -1){
        perror("Error creating district.cfg");
        return -1;
    }
    close(cfg_fd);

    //Cream fisierul de log
    char log_path[PATH_LEN];
    snprintf(log_path,sizeof(log_path),"%s/logged_district",dir_path);
    int log_fd = open(log_path,O_CREAT | O_RDWR,0644);
    if(log_fd == -1){
        perror("Error creating logged_district");
        return -1;
    }
    close(log_fd);

    //Setam permisiunile necesare pt fisiere si directoare
    if(chmod(dir_path,0750) == -1){
        perror("Error setting permisions for directory");
        return -1;
    }
    return 0;

}

int main(int argc, char *argv[])
{   
    const char *districtID = "test_district";  // folosește un district de test
    const char *userRole = "manager";          // testează cu un rol de manager

    printf("Running test: Creating district '%s'...\n", districtID);
    int result = createDistrict(districtID, userRole);

    if(result == 0) {
        printf("District '%s' created successfully!\n", districtID);
    } else {
        printf("Error creating district '%s'!\n", districtID);
    }

    return 0;

    
    return 0;
}