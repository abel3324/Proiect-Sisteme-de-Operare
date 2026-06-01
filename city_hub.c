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

void start_monitor() {

    pid_t hub_mon_pid = fork(); // cream hub_mon

    if (hub_mon_pid == 0) {
        // suntem in hub_mon 

        int pipe_mon[2];
        if (pipe(pipe_mon) == -1) {
            perror("pipe");
            exit(1);
        }

        pid_t mon_pid = fork(); // cream monitor_reports

        if (mon_pid < 0) {
            perror("fork monitor");
            exit(1);
        }

        if (mon_pid == 0) {
            // suntem in monitor_reports 
            close(pipe_mon[0]);
            dup2(pipe_mon[1], STDOUT_FILENO);
            close(pipe_mon[1]);
            execl("./monitor_reports", "monitor_reports", NULL);
            perror("execl monitor_reports");  // ← aceasta linie lipsea
            exit(1);
        }

        // inapoi in hub_mon 
        close(pipe_mon[1]);

        char buffer[256]; int n;
        while ((n = read(pipe_mon[0], buffer, sizeof(buffer)-1)) > 0) {
            buffer[n] = '\0';
            printf("[MONITOR]: %s", buffer);
            fflush(stdout);
            if (strstr(buffer, "EXIT:")) {
                printf("hub: monitor ended\n");
                fflush(stdout);
                break;
            }
        }

        close(pipe_mon[0]);
        waitpid(mon_pid, NULL, 0);
        exit(0);
    }

    // inapoi in city_hub 
    // hub_mon ruleaza in fundal
    printf("hub_mon pornit (pid %d)\n", hub_mon_pid);
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