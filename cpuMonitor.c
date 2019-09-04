#include "headers/Headers.h"

#define FALSE 0
#define TRUE 1

int runCount = 1;

char token[500] = "";
char configLocation[200] = "/root/.config";

void process()
{
        int count = 0;
        double total = 0;
        char body[256];
        long double a[4], b[4], loadavg;
        FILE *file;
        char *response;

        for(;;) {
                file = fopen("/proc/stat", "r");
                fscanf(file, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
                fclose(file);
                wait(500);

                file = fopen("/proc/stat", "r");
                fscanf(file, "%*s %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2], &b[3]);
                fclose(file);

                total += ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3])) * 100;
                count++;

                if(count == runCount) {
                        char* dateString = nowString();
                        sprintf(body, "{\"value\": %.2f, \"date\": \"%s\", \"userId\":\"%s\", \"type\":\"cpuUsage\"}", (total / runCount), dateString, "user");
                        free(dateString);


                        response = post("https://tensorboard.onwarddb.ca/cpuUsage/cpuUsage", body);

                        free(response);

                        count = 0;
                        total = 0;
                }
        }
}

void configuration()
{
        char *key, *value;
        FILE *file;
        char line[100];
        char* rest;

        strcat(configLocation, "/vanguard.conf");

        file = fopen(configLocation, "r");

        if(file == NULL) {
                printf("File null\n");
                exit(1);
        }

        while(fgets(line, 100, file) != NULL) {
                if(strncmp(line, "#", 1) != 0) {
                        rest = line;
                        rest = strtok_r(rest, "\n", &rest);
                        key = strtok_r(rest, ":", &rest);
                        value = strtok_r(rest, ":", &rest);

                        if(strcmp(key, "token") == 0) {
                                strcpy(token, value);
                        } else if (strcmp(key, "runCount") == 0) {
                                runCount = atoi(value);
                        }
                }
        }

        return;
}

int main(int argc, char** argv)
{
        int debug = FALSE;

        for(int i = 1; i < argc; i++) {
                if(strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-debug") == 0 || strcmp(argv[i], "-d") == 0) {
                        debug = TRUE;
                }else if (strcmp(argv[i], "--config")  == 0 || strcmp(argv[i], "-config")  == 0 || strcmp(argv[i], "-c") == 0 ) {
                        i++;
                        strcpy(configLocation, argv[i]);
                }else{
                        Usage(argv[i]);
                }
        }

        configuration();

        if(!debug) {
                // Our process ID and Session ID
                pid_t pid, sid;

                // Fork off the parent process
                pid = fork();
                if (pid < 0) {
                        exit(EXIT_FAILURE);
                }
                // If we got a good PID, then
                // we can exit the parent process.
                if (pid > 0) {
                        exit(EXIT_SUCCESS);
                }

                // Change the file mode mask
                umask(0);

                // Create a new SID for the child process
                sid = setsid();
                if (sid < 0) {
                        // Log the failure
                        exit(EXIT_FAILURE);
                }

                // Change the current working directory
                if ((chdir("/")) < 0) {
                        // Log the failure
                        exit(EXIT_FAILURE);
                }

                // Close out the standard file descriptors
                close(STDIN_FILENO);
                close(STDOUT_FILENO);
                close(STDERR_FILENO);
        }

        process();

        exit(EXIT_SUCCESS);
}
