void Log(char *msg, const int error)
{
        FILE *file;
        char filePath[1000];
        char message[2000];

        char *dateString = nowString();
        sprintf(message, "%s %s", dateString, msg);

        if(!error) {
                strcpy(filePath, "/var/log/daemon_log");
        } else {
                strcpy(filePath, "/var/log/daemon_error");
        }

        file = fopen(filePath, "ab+");

        if (file == NULL) {
                // Unable to open file hence exit
                printf("\nUnable to open '%s' file.\n", filePath);
                printf("Please check whether file exists and you have write privilege.\n");
                exit(EXIT_FAILURE);
        }

        strcat(message, "\n");
        fputs(message, file);

        if(error) {
                sprintf(message, "%s %s", dateString, "Sleeping for 1 minute.\n");
                fputs(message, file);
                fclose(file);
                wait(60000);

                free(dateString);
        }else{
                fclose(file);
                free(dateString);
        }
        return;
}
