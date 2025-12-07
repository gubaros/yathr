/*
 * Autor: Guido Barosio
 * Email: guido@bravo47.com
 * Fecha: 2024-06-08
 */

#include "routing.h"
#include <string.h>
#include <stddef.h>

typedef struct {
    const char *key;
    const char *url;
} Redirect;

static const Redirect redirects[] = {
    {"google", "https://www.google.com"},
    {"facebook", "https://www.facebook.com"},
    {"youtube", "https://www.youtube.com"},
    {"amazon", "https://www.amazon.com"},
    {"wikipedia", "https://www.wikipedia.org"},
    {"twitter", "https://www.twitter.com"},
    {"instagram", "https://www.instagram.com"},
    {"linkedin", "https://www.linkedin.com"},
    {"reddit", "https://www.reddit.com"},
    {"pinterest", "https://www.pinterest.com"},
    {"netflix", "https://www.netflix.com"},
    {"yahoo", "https://www.yahoo.com"},
    {"bing", "https://www.bing.com"},
    {"ebay", "https://www.ebay.com"},
    {"microsoft", "https://www.microsoft.com"},
    {"apple", "https://www.apple.com"},
    {"cnn", "https://www.cnn.com"},
    {"bbc", "https://www.bbc.com"},
    {"nytimes", "https://www.nytimes.com"},
    {"guardian", "https://www.theguardian.com"},
    {NULL, NULL}
};

const char *find_redirect(const char *key) {
    for (int i = 0; redirects[i].key != NULL; i++) {
        if (strcmp(redirects[i].key, key) == 0) {
            return redirects[i].url;
        }
    }
    return NULL;
}
