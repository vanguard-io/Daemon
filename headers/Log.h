void Log(const char *message, const int error)
{
        FILE *file;
        char filePath[1000];

        if(!error) {
                printf("%s\n", "not an error");
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

        fputs(message, file);

        fclose(file);

        if(error) {
                printf("%s\n", "We broke");
                exit(1);
        }

}
