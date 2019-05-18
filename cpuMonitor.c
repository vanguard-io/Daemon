#include "headers/Headers.h"

#define milSec 500

#define FALSE 0
#define TRUE 1

void process()
{
        int count = 0;
        double total = 0;
        char body[256];
        char loadAvgString[256];
        long double a[4], b[4], loadavg;
        FILE *file;

        time_t t;
        struct tm * timeInfo;
        char timeString [20];
        char dateString [20];

        struct timespec ts;
        ts.tv_sec = milSec / 1000;
        ts.tv_nsec = (milSec % 1000) * 1000000;

        for(;;) {
                file = fopen("/proc/stat", "r");
                fscanf(file, "%*s %Lf %Lf %Lf %Lf", &a[0], &a[1], &a[2], &a[3]);
                fclose(file);
                nanosleep(&ts, NULL);

                file = fopen("/proc/stat", "r");
                fscanf(file, "%*s %Lf %Lf %Lf %Lf", &b[0], &b[1], &b[2], &b[3]);
                fclose(file);

                loadavg = ((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3])) * 100;

                total += loadavg;
                count++;

                if(count == 9) {
                        sprintf(loadAvgString, "%.2f", (total / 10));

                        strcpy(body, "{\"value\": ");
                        strcat(body, loadAvgString);
                        strcat(body, ", ");
                        strcat(body, "\"date\": \"");

                        time(&t);
                        timeInfo = localtime(&t);
                        strftime (dateString, sizeof(dateString), "%F", timeInfo);
                        strftime (timeString, sizeof(timeString), "%T", timeInfo);
                        strcat(dateString, "T");
                        strcat(dateString, timeString);
                        strcat(body, dateString);

                        strcat(body, "\"}");

                        request("localhost", 80, "POST", "/", body);

                        count = 0;
                        total = 0;
                }
        }
}

int main(int argc, char** argv)
{
        int debug = FALSE;

        for(int i = 0; i < argc; i++) {
                if(strcmp(argv[i], "debug")) {
                        debug = TRUE;
                }
        }

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
