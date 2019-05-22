#include "headers/Headers.h"

#define FALSE 0
#define TRUE 1

void process()
{
        int count = 0;
        double total = 0;
        char body[256];
        long double a[4], b[4], loadavg;
        FILE *file;

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

                if(count == 9) {
                        char* dateString = nowString();
                        sprintf(body, "{\"value\": %.2f, \"date\": \"%s\"}", (total / 10), dateString);
                        free(dateString);

                        request("https://onwarddb.ca", body);

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
