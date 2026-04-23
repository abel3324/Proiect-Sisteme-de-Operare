#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

#define PATH_LEN 256

#define NAME_LEN 50
#define CATEGORY_LEN 30
#define DESC_LEN 100

typedef struct {
    int report_id;
    char inspector_name[NAME_LEN];
    double latitude;
    double longitude;
    char category[CATEGORY_LEN];
    int severity;
    time_t timestamp;
    char description[DESC_LEN];
} Report;

// 1 - inspector , 2 - manager , -1 - invalid
int getRole(char *argv[], int argc) {
    if (argc < 3) return -1;

    if (strcmp(argv[1], "--role") == 0) {
        if (strcmp(argv[2], "inspector") == 0) return 1;
        if (strcmp(argv[2], "manager") == 0) return 2;
    }

    return -1;
}

void printRole(int n) {
    if (n == 1) printf("inspector\n");
    else if (n == 2) printf("manager\n");
    else printf("invalid role\n");
}

// extrage user
char* getUser(char *argv[], int argc) {
    if (argc < 5) return NULL;

    if (strcmp(argv[3], "--user") == 0)
        return argv[4];

    return NULL;
}

/*
    -1 error
    1 add
    2 list
    3 view
    4 remove_report
    5 update_threshold
    6 filter
*/

int commandSelect(char *argv[], int argc) {
    if (argc < 6) return -1;

    if (strcmp(argv[5], "--add") == 0) return 1;
    if (strcmp(argv[5], "--list") == 0) return 2;
    if (strcmp(argv[5], "--view") == 0) return 3;
    if (strcmp(argv[5], "--remove_report") == 0) return 4;
    if (strcmp(argv[5], "--update_threshold") == 0) return 5;
    if (strcmp(argv[5], "--filter") == 0) return 6;

    return -1;
}

void printCommand(int n) {
    switch (n) {
        case 1: printf("add\n"); break;
        case 2: printf("list\n"); break;
        case 3: printf("view\n"); break;
        case 4: printf("remove_report\n"); break;
        case 5: printf("update_threshold\n"); break;
        case 6: printf("filter\n"); break;
        default: printf("invalid command\n"); break;
    }
}

// district este dupa comanda
char* getDistrict(char *argv[], int argc) {
    if (argc < 7) return NULL;
    return argv[6];
}

// creeaza fisier + seteaza permisiuni
int createFileIfNotExists(const char *path, mode_t mode) {
    int fd = open(path, O_CREAT | O_RDWR, mode);
    if (fd == -1) {
        perror("open");
        return -1;
    }

    close(fd);

    if (chmod(path, mode) == -1) {
        perror("chmod");
        return -1;
    }

    return 0;
}

// daca cfg e gol -> punem threshold default
int initDistrictConfig(const char *cfg_path) {
    int fd = open(cfg_path, O_RDWR);
    if (fd == -1) {
        perror("open cfg");
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return -1;
    }

    if (st.st_size == 0) {
        char text[] = "threshold=2\n";
        if (write(fd, text, strlen(text)) == -1) {
            perror("write");
            close(fd);
            return -1;
        }
    }

    close(fd);
    return 0;
}

// creeaza symlink active_reports-<district>
int createSymlinkForDistrict(const char *districtID) {
    char target[PATH_LEN];
    char link[PATH_LEN];

    snprintf(target, sizeof(target), "./%s/reports.dat", districtID);
    snprintf(link, sizeof(link), "./active_reports-%s", districtID);

    unlink(link); // stergem daca exista

    if (symlink(target, link) == -1) {
        perror("symlink");
        return -1;
    }

    return 0;
}

// creeaza structura district
int createDistrict(const char* districtID, const char* userRole) {

    (void)userRole;

    char dir_path[PATH_LEN];
    snprintf(dir_path, sizeof(dir_path), "./%s", districtID);

    struct stat st;

    // daca nu exista -> mkdir
    if (stat(dir_path, &st) == -1) {
        if (mkdir(dir_path, 0750) == -1) {
            perror("mkdir");
            return -1;
        }
    }

    chmod(dir_path, 0750);

    // reports.dat
    char report_path[PATH_LEN];
    snprintf(report_path, sizeof(report_path), "%s/reports.dat", dir_path);
    if (createFileIfNotExists(report_path, 0664) == -1) return -1;

    // district.cfg
    char cfg_path[PATH_LEN];
    snprintf(cfg_path, sizeof(cfg_path), "%s/district.cfg", dir_path);
    if (createFileIfNotExists(cfg_path, 0640) == -1) return -1;

    if (initDistrictConfig(cfg_path) == -1) return -1;

    // log
    char log_path[PATH_LEN];
    snprintf(log_path, sizeof(log_path), "%s/logged_district", dir_path);
    if (createFileIfNotExists(log_path, 0644) == -1) return -1;

    // symlink
    if (createSymlinkForDistrict(districtID) == -1) return -1;

    return 0;
}

//citeste un sir de la tastatura si scoate '\n'
void readLine(char *buffer, int size){
    if(fgets(buffer,size,stdin)!=NULL){
        buffer[strcspn(buffer,"\n")]='\0';
    }
}
// gaseste urmatorul id disponibil
int getNextReportID(const char *report_path) {
    int fd = open(report_path, O_RDONLY);
    if (fd == -1) {
        perror("open reports.dat");
        return -1;
    }

    Report r;
    int max_id = 0;

    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (r.report_id > max_id) {
            max_id = r.report_id;
        }
    }

    close(fd);
    return max_id + 1;
}
//verificam daca rolul poate scrie in reports.dat
int canAddToReports(const char *report_path, int role) {
    struct stat st;

    if (stat(report_path, &st) == -1) {
        perror("stat reports.dat");
        return 0;
    }

    // manager = owner -> are nevoie de write pe owner
    if (role == 2) {
        if (st.st_mode & S_IWUSR) return 1;
        return 0;
    }

    // inspector = group -> are nevoie de write pe group
    if (role == 1) {
        if (st.st_mode & S_IWGRP) return 1;
        return 0;
    }

    return 0;
}

// scrie in log o operatie
int logAction(const char *districtID, const char *roleText, const char *user, const char *action) {
    char log_path[PATH_LEN];
    snprintf(log_path, sizeof(log_path), "./%s/logged_district", districtID);

    int fd = open(log_path, O_WRONLY | O_APPEND);
    if (fd == -1) {
        perror("open log");
        return -1;
    }

    time_t now = time(NULL);
    char line[256];

    int len = snprintf(line, sizeof(line), "%ld %s %s %s\n", now, roleText, user, action);

    if (write(fd, line, len) == -1) {
        perror("write log");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

// adauga un raport nou
int addReport(const char *districtID, const char *user, int role) {
    char report_path[PATH_LEN];
    snprintf(report_path, sizeof(report_path), "./%s/reports.dat", districtID);

    if (canAddToReports(report_path, role) == 0) {
        printf("permission denied for add on reports.dat\n");
        return -1;
    }

    int fd = open(report_path, O_WRONLY | O_APPEND);
    if (fd == -1) {
        perror("open reports.dat");
        return -1;
    }

    Report r;
    memset(&r, 0, sizeof(Report));

    r.report_id = getNextReportID(report_path);
    if (r.report_id == -1) {
        close(fd);
        return -1;
    }

    strncpy(r.inspector_name, user, NAME_LEN - 1);
    r.timestamp = time(NULL);

    printf("latitude: ");
    if (scanf("%lf", &r.latitude) != 1) {
        printf("invalid latitude\n");
        close(fd);
        return -1;
    }

    printf("longitude: ");
    if (scanf("%lf", &r.longitude) != 1) {
        printf("invalid longitude\n");
        close(fd);
        return -1;
    }

    getchar(); // consuma '\n' ramas dupa scanf

    printf("category: ");
    readLine(r.category, CATEGORY_LEN);

    printf("severity (1/2/3): ");
    if (scanf("%d", &r.severity) != 1) {
        printf("invalid severity\n");
        close(fd);
        return -1;
    }

    getchar(); // consuma '\n'

    printf("description: ");
    readLine(r.description, DESC_LEN);

    if (write(fd, &r, sizeof(Report)) != sizeof(Report)) {
        perror("write report");
        close(fd);
        return -1;
    }

    close(fd);

    if (role == 1) logAction(districtID, "inspector", user, "add");
    if (role == 2) logAction(districtID, "manager", user, "add");

    printf("report added successfully with id %d\n", r.report_id);
    return 0;
}

//conversie permisiuni
void permissionsToString(mode_t mode, char *perm) {
    perm[0] = (mode & S_IRUSR) ? 'r' : '-';
    perm[1] = (mode & S_IWUSR) ? 'w' : '-';
    perm[2] = (mode & S_IXUSR) ? 'x' : '-';

    perm[3] = (mode & S_IRGRP) ? 'r' : '-';
    perm[4] = (mode & S_IWGRP) ? 'w' : '-';
    perm[5] = (mode & S_IXGRP) ? 'x' : '-';

    perm[6] = (mode & S_IROTH) ? 'r' : '-';
    perm[7] = (mode & S_IWOTH) ? 'w' : '-';
    perm[8] = (mode & S_IXOTH) ? 'x' : '-';

    perm[9] = '\0';
}
//verificare daca rolul poate sa citeasca
int canReadReports(const char *report_path, int role) {
    struct stat st;

    if (stat(report_path, &st) == -1) {
        perror("stat reports.dat");
        return 0;
    }

    // manager = owner
    if (role == 2) {
        if (st.st_mode & S_IRUSR) return 1;
        return 0;
    }

    // inspector = group
    if (role == 1) {
        if (st.st_mode & S_IRGRP) return 1;
        return 0;
    }

    return 0;
}
//functie de afisare a unui raport 
void printReport(Report r) {
    printf("ID: %d\n", r.report_id);
    printf("Inspector: %s\n", r.inspector_name);
    printf("Latitude: %.2lf\n", r.latitude);
    printf("Longitude: %.2lf\n", r.longitude);
    printf("Category: %s\n", r.category);
    printf("Severity: %d\n", r.severity);
    printf("Timestamp: %ld\n", r.timestamp);
    printf("Description: %s\n", r.description);
    printf("-----------------------------\n");
}
int listReports(const char *districtID, int role) {
    char report_path[PATH_LEN];

    // construim path-ul catre reports.dat
    if (snprintf(report_path, sizeof(report_path), "./%s/reports.dat", districtID) >= PATH_LEN) {
        printf("path too long\n");
        return -1;
    }

    // verificam daca rolul are voie sa citeasca
    if (canReadReports(report_path, role) == 0) {
        printf("permission denied for list on reports.dat\n");
        return -1;
    }

    struct stat st;

    // luam info despre fisier (permisiuni, size, timp)
    if (stat(report_path, &st) == -1) {
        perror("stat reports.dat");
        return -1;
    }

    char perm[10];

    // convertim permisiunile in format rw-rw-r--
    permissionsToString(st.st_mode, perm);

    // luam timpul ultimei modificari
    char *time_text = ctime(&st.st_mtime);

    // scoatem '\n' de la final daca exista
    if (time_text != NULL) {
        time_text[strcspn(time_text, "\n")] = '\0';
    }

    // afisam info despre fisier
    printf("reports.dat info:\n");
    printf("Permissions: %s\n", perm);
    printf("Size: %ld bytes\n", st.st_size);
    printf("Last modified: %s\n", time_text ? time_text : "unknown");
    printf("\n");

    
    int fd = open(report_path, O_RDONLY);
    if (fd == -1) {
        perror("open reports.dat");
        return -1;
    }

    Report r;
    int found = 0;

    // citim fiecare raport din fisier
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        printReport(r);   
        found = 1;
    }

    close(fd);

    // daca nu exista rapoarte
    if (!found) {
        printf("No reports found.\n");
    }

    return 0;
}

int main(int argc, char *argv[]) {

    int role = getRole(argv, argc);
    char *user = getUser(argv, argc);
    int command = commandSelect(argv, argc);
    char *district = getDistrict(argv, argc);

    if (role == -1) {
        printf("invalid role\n");
        return 1;
    }

    if (user == NULL) {
        printf("invalid user\n");
        return 1;
    }

    if (command == -1) {
        printf("invalid command\n");
        return 1;
    }

    if (district == NULL) {
        printf("invalid district\n");
        return 1;
    }

    printf("role: "); printRole(role);
    printf("user: %s\n", user);
    printf("command: "); printCommand(command);
    printf("district: %s\n", district);

    if (createDistrict(district, argv[2]) == -1) {
    printf("error creating district\n");
    return 1;
    }

    if (createDistrict(district, argv[2]) == -1) {
        printf("error creating district\n");
        return 1;
    }

    if (command == 1) {
        if (addReport(district, user, role) == -1) {
            printf("add failed\n");
            return 1;
        }
    } else if (command == 2) {
        if (listReports(district, role) == -1) {
            printf("list failed\n");
            return 1;
        }
    } else {
        printf("command not implemented yet\n");
    }

return 0;

}