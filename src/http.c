#include "angband.h"
#include "http.h"

#ifdef USE_CURL
#include <curl/curl.h>

/* Callback function for processing HTTP response data */
static size_t _write_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    http_response_t *mem = (http_response_t *)userp;
    
    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) {
        /* Out of memory */
        msg_print("<color:r>错误：HTTP 响应内存不足</color>");
        return 0;
    }
    
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;  /* Null-terminate the string */
    
    return realsize;
}

/* Function to make an HTTP request using libcurl */
bool make_http_request(const char *url, const char *post_data, http_response_t *response)
{
    CURL *curl;
    CURLcode res;
    bool success = FALSE;
    
    if (!response) return FALSE;
    
    /* Initialize response struct */
    response->data = malloc(1);
    response->size = 0;
    
    if (!response->data) return FALSE;
    
    /* In case this is the first time, initialize libcurl globally */
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    /* Get a curl handle */
    curl = curl_easy_init();
    if (curl) {
        /* Set the URL */
        curl_easy_setopt(curl, CURLOPT_URL, url);
        
        /* If a POST request, set the data */
        if (post_data) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
        }
        
        /* Send all data to this function */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_callback);
        
        /* Pass our response struct to the callback function */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response);
        
        /* Set a reasonable timeout */
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
        
        /* Set a user agent */
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "RoguelikeFansBand/1.0");
        
        /* Follow redirects */
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        /* Perform the request */
        res = curl_easy_perform(curl);
        long http_code = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
        
        /* Check for errors */
        if (res != CURLE_OK || http_code != 200) {
            if (response->data) {
                free(response->data);
                response->data = NULL;
                response->size = 0;
            }
        } else {
            success = TRUE;
        }
        
        /* Cleanup */
        curl_easy_cleanup(curl);
    }
    
    /* Always cleanup global initialization */
    curl_global_cleanup();
    
    return success;
}

#ifdef USE_CURL

/* Function to make an HTTP POST request with JSON data */
bool make_http_post(const char *url, const char *json_data, http_response_t *response)
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *header_list = NULL;
    bool success = FALSE;
    
    if (!response || !url || !json_data) return FALSE;
    
    /* Initialize response struct */
    response->data = malloc(1);
    response->size = 0;
    
    if (!response->data) return FALSE;
    
    /* Initialize libcurl globally if necessary */
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    /* Get a curl handle */
    curl = curl_easy_init();
    if (curl) {
        /* Set the URL */
        curl_easy_setopt(curl, CURLOPT_URL, url);
        
        /* Set HTTP method to POST */
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        
        /* Set JSON data */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
        
        /* Set content type to JSON */
        header_list = curl_slist_append(header_list, "Content-Type: application/json");
        header_list = curl_slist_append(header_list, "Accept: application/json");
        
        /* Set headers */
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
        
        /* Send all data to the callback function */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _write_callback);
        
        /* Pass our response struct to the callback function */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response);
        
        /* Set a reasonable timeout */
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
        
        /* Set a user agent */
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "RoguelikeFansBand/1.0");
        
        /* Follow redirects */
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        /* Perform the request */
        res = curl_easy_perform(curl);

        long http_code = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &http_code);
        
        /* Check for errors */
        if (res != CURLE_OK || http_code != 200) {
            if (response->data) {
                free(response->data);
                response->data = NULL;
                response->size = 0;
            }
        } else {
            success = TRUE;
        }
        
        /* Free the header list */
        if (header_list) {
            curl_slist_free_all(header_list);
        }
        
        /* Cleanup curl handle */
        curl_easy_cleanup(curl);
    }
    
    /* Always cleanup global initialization */
    curl_global_cleanup();
    
    return success;
}
#endif /* USE_CURL make_http_post */

#else /* USE_CURL */

/* Stub implementation when libcurl is not available */
bool make_http_request(const char *url, const char *post_data, http_response_t *response)
{
    msg_print("<color:r>网络功能不可用。编译时未找到 libcurl。</color>");
    
    if (response) {
        response->data = NULL;
        response->size = 0;
    }
    
    return FALSE;
}

/* Stub implementation of POST when libcurl is not available */
bool make_http_post(const char *url, const char *post_data, http_response_t *response)
{
    msg_print("<color:r>网络功能不可用。编译时未找到 libcurl。</color>");
    
    if (response) {
        response->data = NULL;
        response->size = 0;
    }
    
    return FALSE;
}

#endif /* USE_CURL */
