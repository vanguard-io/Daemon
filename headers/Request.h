#include <curl/curl.h>

struct string {
        char *ptr;
        size_t len;
};

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
        size_t new_len = s->len + size*nmemb;
        s->ptr = realloc(s->ptr, new_len+1);
        if (s->ptr == NULL) {
                fprintf(stderr, "realloc() failed\n");
                exit(EXIT_FAILURE);
        }
        memcpy(s->ptr+s->len, ptr, size*nmemb);
        s->ptr[new_len] = '\0';
        s->len = new_len;

        return size*nmemb;
}

void init_string(struct string *s) {
        s->len = 0;
        s->ptr = malloc(s->len+1);
        if (s->ptr == NULL) {
                fprintf(stderr, "malloc() failed\n");
                exit(EXIT_FAILURE);
        }
        s->ptr[0] = '\0';
}

char* request (const char *host, const char *method, const char *body)
{
        CURL *curl;
        CURLcode res;
        struct curl_slist *headers = NULL;

        struct string s;
        init_string(&s);

        // In windows, this will init the winsock stuff
        curl_global_init(CURL_GLOBAL_ALL);

        // get a curl handle
        curl = curl_easy_init();
        if(curl) {
                // First set the URL that is about to receive our POST
                curl_easy_setopt(curl, CURLOPT_URL, host);

                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

                if(strcmp(method, "POST") == 0) {
                        headers = curl_slist_append(headers, "Content-Type: application/json");
                        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
                        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
                }

                // Enable SSL checking
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

                // Perform the request, res will get the return code
                res = curl_easy_perform(curl);

                // Check for errors
                if(res != CURLE_OK) {
                        char error_message[1000];
                        sprintf(error_message, "curl_easy_perform() failed: %s", curl_easy_strerror(res));
                        Log(error_message, 1);
                }

                printf("%s\n", s.ptr);
                // always cleanup
                curl_easy_cleanup(curl);
        }

        curl_global_cleanup();

        return s.ptr;
}

char* get(const char *host)
{
    return request(host, "GET", NULL);
}

char* post(const char *host, const char* body)
{
    return request(host, "POST", body);
}
