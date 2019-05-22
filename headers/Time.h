char * nowString()
{
    time_t t;
    struct tm * timeInfo;
    char timeString [20];
    char dateString [20];
    char* fullString = malloc(sizeof(char) * 48);

    time(&t);
    timeInfo = gmtime(&t);

    strftime (dateString, sizeof(dateString), "%F", timeInfo);
    strftime (timeString, sizeof(timeString), "%T", timeInfo);

    sprintf(fullString, "%sT%sZ", dateString, timeString);

    return fullString;
}

void wait(int milSec)
{
    struct timespec ts;
    ts.tv_sec = milSec / 1000;
    ts.tv_nsec = (milSec % 1000) * 1000000;
    nanosleep(&ts, NULL);
    return;
}
