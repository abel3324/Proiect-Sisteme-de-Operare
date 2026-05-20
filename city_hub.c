#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 256
#define MAX_ARGS 32


// functia porneste monitorul
// si citeste mesajele lui prin pipe

void start_monitor(){
    // fd[0] = capat de citire
    // fd[1] = capat de scriere
    int fd[2];

    // cream pipe-ul
    if (pipe(fd) == -1) {
        perror("pipe");
        return;
    }

    //cream procesul copil

    pid_t pid = fork();
    if(pid < 0){
        perror("fork");
        return;
    }

    //copilul
    if(pid == 0){

        //copilul nu citeste din pipe
        close(fd[0]);

        // redirectam stdout spre pipe
        // orice printf merge acum in pipe
        dup2(fd[1], STDOUT_FILENO);

        // inchidem descriptorul vechi
        close(fd[1]);

        //inlocuim procesul copil cu monitor_reports
        execl("./monitor_reports","monitor_reports", NULL);

        perror("execl");
        exit(1);
    }

    //parintele nu scrie in pipe
    close(fd[1]);

    char buffer[256];
    int n;

    //citim mesajele venite de la monitor
    while ((n = read(fd[0],buffer,sizeof(buffer) - 1)) > 0){

        buffer[n] = '\0';
        
        printf("[MONITOR] : %s", buffer);

        // daca monitorul s-a inchis
        if (strstr(buffer, "EXIT:")) {

            printf("hub: monitor ended\n");

            break;
        }
        close(fd[0]);

        //asteptam terminarea copilului
        waitpid(pid, NULL, 0);
    }
    
}

// functia porneste scorer pentru fiecare district
void calculate_scores(int count, char *districts[]){
    for (int i = 0; i < count; i++) {

        int fd[2];

        // cream pipe
        if (pipe(fd) == -1) {
            perror("pipe");
            continue;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            continue;
        }

        // copilul
        if (pid == 0) {

            // copilul nu citeste
            close(fd[0]);

            // stdout -> pipe
            dup2(fd[1], STDOUT_FILENO);

            close(fd[1]);

            // rulam scorer
            execl("./scorer","scorer",districts[i],NULL);

            perror("execl");
            exit(1);
        }

        // parintele nu scrie
        close(fd[1]);

        char buffer[512];

        int n;

        printf("\n=== %s ===\n", districts[i]);

        // citim rezultatul scorerului
        while ((n = read(fd[0],buffer,sizeof(buffer) - 1)) > 0) {

            buffer[n] = '\0';

            printf("%s", buffer);
        }

        close(fd[0]);

        // asteptam copilul
        waitpid(pid, NULL, 0);
    }
}

int main()
{
    char line[MAX_LINE];

    while (1) {

        printf("city_hub> ");

        fflush(stdout);

        // citim comanda
        if (fgets(line,
                  sizeof(line),
                  stdin) == NULL) {

            break;
        }

        // scoatem \n
        line[strcspn(line, "\n")] = '\0';


        // exit
        if (strcmp(line, "exit") == 0) {

            break;
        }


        // start_monitor
        if (strcmp(line, "start_monitor") == 0) {

            start_monitor();

            continue;
        }


        // calculate_scores
        if (strncmp(line,
                    "calculate_scores",
                    16) == 0) {

            char *districts[MAX_ARGS];

            int count = 0;

            // spargem linia in cuvinte
            char *token = strtok(line, " ");

            // sarim peste calculate_scores
            token = strtok(NULL, " ");

            // luam toate districtele
            while (token != NULL) {

                districts[count++] = token;

                token = strtok(NULL, " ");
            }

            calculate_scores(count, districts);

            continue;
        }


        printf("unknown command\n");
    }

    return 0;
}