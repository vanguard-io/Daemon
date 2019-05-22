#include <curl/curl.h>

void request (const char *host, const char *body)
{
        CURL *curl;
        CURLcode res;
        struct curl_slist *headers = NULL;

        // In windows, this will init the winsock stuff
        curl_global_init(CURL_GLOBAL_ALL);

        // get a curl handle
        curl = curl_easy_init();
        if(curl) {
                // First set the URL that is about to receive our POST
                curl_easy_setopt(curl, CURLOPT_URL, host);

                // Set content type header
                headers = curl_slist_append(headers, "Content-Type: application/json");
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

                // Now specify the POST data
                curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);

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

                // always cleanup
                curl_easy_cleanup(curl);
        }

        curl_global_cleanup();

        return;
}
