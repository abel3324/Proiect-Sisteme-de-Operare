#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>



#define NAME_LEN      50
#define CATEGORY_LEN  30
#define DESC_LEN     100
#define PATH_LEN     256

// numarul maxim de inspectori per district
#define MAX_INSPECTORS 100



typedef struct {
    int    report_id;
    char   inspector_name[NAME_LEN];
    double latitude;
    double longitude;
    char   category[CATEGORY_LEN];
    int    severity;
    time_t timestamp;
    char   description[DESC_LEN];
} Report;



  
  //score = suma nivelurilor de severitate din toate rapoartele lui

typedef struct {
    char name[NAME_LEN];
    int  score;
    int  report_count;
} InspectorScore;


int main(int argc, char *argv[]) {

    // scorer primeste numele districtului ca prim argument
    if (argc < 2) {
        fprintf(stderr, "Usage: scorer <district_name>\n");
        return 1;
    }

    // construim calea spre reports.dat al districtului
    char path[PATH_LEN];
    snprintf(path, sizeof(path), "./%s/reports.dat", argv[1]);

    // deschidem fisierul binar pentru citire
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        // daca fisierul nu exista, afisam un mesaj clar
        fprintf(stderr, "scorer: nu pot deschide %s: ", path);
        perror("");
        return 1;
    }

    // array de scoruri per inspector
    InspectorScore scores[MAX_INSPECTORS];
    int count = 0;

    Report r;

    // citim rapoartele unul cate unul (fixed-size records)
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {

        // cautam daca inspectorul are deja un entry in array
        int found = 0;

        for (int i = 0; i < count; i++) {
            if (strcmp(scores[i].name, r.inspector_name) == 0) {
                // inspector deja cunoscut — adunam severitatea
                scores[i].score += r.severity;
                scores[i].report_count++;
                found = 1;
                break;
            }
        }

        // inspector nou — il adaugam in array
        if (!found && count < MAX_INSPECTORS) {
            strncpy(scores[count].name, r.inspector_name, NAME_LEN - 1);
            scores[count].name[NAME_LEN - 1] = '\0';
            scores[count].score        = r.severity;
            scores[count].report_count = 1;
            count++;
        }
    }

    close(fd);

    // daca nu exista rapoarte in district
    if (count == 0) {
        printf("  (niciun raport in districtul %s)\n", argv[1]);
        return 0;
    }

    // afisam scorul fiecarui inspector
    // stdout-ul este redirectat de city_hub prin pipe, deci nu trebuie fflush
    for (int i = 0; i < count; i++) {
        printf("  Inspector: %-25s | rapoarte: %2d | scor total: %d\n",
               scores[i].name,
               scores[i].report_count,
               scores[i].score);
    }

    return 0;
}