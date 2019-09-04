#include "headers/Headers.h"
#include <stdlib.h>

#define FALSE 0
#define TRUE 1

char host[200] = "";

void process()
{
    FILE *fp;
    char *command;
    char *line;
    char *ptr;
    char *response;
    char body[256];

    int newLineCount = 0;
    int oldLineCount = 0;

    line = malloc(200 * sizeof(char));
    command = malloc(200 * sizeof(char));

    strcpy(command, "wc -l /var/log/apache2/other_vhosts_access.log");

    for (;;)
    {
        fp = popen(command, "r");
        fgets(line, "%s", fp);

        ptr = strtok(line, " ");

        oldLineCount = newLineCount;

        newLineCount = atoi(line);

        if (oldLineCount == 0 && newLineCount != 0)
        {
            printf("oldLineCount is %d %s\n", oldLineCount, line);
            continue;
        }

        char *dateString = nowString();

        sprintf(body, "{\"value\": %d, \"date\": \"%s\", \"host\":\"%s\"}", (newLineCount - oldLineCount), dateString, host);

        response = post("https://elastic:ISxQZ6RIWdxTGd7S@elasticsearch.jmir.pub/requests/requestsPerSecond", body);

        free(response);
        free(dateString);
        wait(1000);
    }
}

int main(int argc, char **argv)
{
    int debug = FALSE;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--debug") == 0 || strcmp(argv[i], "-debug") == 0 || strcmp(argv[i], "-d") == 0)
        {
            debug = TRUE;
        }
        else if (strcmp(argv[i], "--host") == 0 || strcmp(argv[i], "-host") == 0 || strcmp(argv[i], "-h") == 0)
        {
            i++;
            strcpy(host, argv[i]);
        }
        else
        {
            Usage(argv[i]);
        }
    }

    if (!debug)
    {
        // Our process ID and Session ID
        pid_t pid, sid;

        // Fork off the parent process
        pid = fork();
        if (pid < 0)
        {
            exit(EXIT_FAILURE);
        }
        // If we got a good PID, then
        // we can exit the parent process.
        if (pid > 0)
        {
            exit(EXIT_SUCCESS);
        }

        // Change the file mode mask
        umask(0);

        // Create a new SID for the child process
        sid = setsid();
        if (sid < 0)
        {
            // Log the failure
            exit(EXIT_FAILURE);
        }

        // Change the current working directory
        if ((chdir("/")) < 0)
        {
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
