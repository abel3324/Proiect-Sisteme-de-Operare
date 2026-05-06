#define _POSIX_C_SOURCE 200809L

#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>      
#include <signal.h>     
#include <fcntl.h>      
#include <string.h>  


#define PID_FILE ".monitor_pid"


// spune daca programul continua sa ruleze
volatile sig_atomic_t running = 1;

void handle_sigint(int sig){

    (void)sig;
    write(STDOUT_FILENO, "monitor: shutting down\n", 23);

    unlink(PID_FILE);

    running = 0;
}

void handle_sigusr1(int sig){

    (void)sig;
    write(STDOUT_FILENO, "monitor: new report received\n", 29);
}


int main(void){
    
    struct sigaction sa_usr1;
    struct sigaction sa_int;
    
    memset(&sa_int, 0, sizeof(sa_int));
    memset(&sa_usr1, 0, sizeof(sa_usr1));

    sa_usr1.sa_handler = handle_sigusr1;
    sa_int.sa_handler = handle_sigint;

    sigaction(SIGUSR1, &sa_usr1, NULL);
    sigaction(SIGINT, &sa_int, NULL);

    // deschidem fisierul .monitor_pid
    // O_WRONLY = scriere
    // O_CREAT = daca nu exista, il creeaza
    // O_TRUNC = daca exista, il goleste
    int fd = open(PID_FILE, O_WRONLY | O_CREAT | O_TRUNC , 0644);
    if(fd == -1){
        perror("open");
        return 1;
    }

    char buffer[32];

    int len = snprintf(buffer, sizeof(buffer), "%d\n", getpid());

    //scriem pid ul in fisier
    if (write(fd, buffer, len) == -1) {
        perror("write");
        close(fd);
        return 1;
    }

    close(fd);

    printf("monitor started (pid %d)\n",getpid());

    //programul ramane pornit pana cand running devine 0
    while (running){
        //procesul doarme, se trezeste cand primeste un semnal
        pause();
    }
    


    return 0;
}