/*
 * Autor: Guido Barosio
 * Email: guido@bravo47.com
 * Fecha: 2024-06-08
 */

#include "routing.h"
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

/*
 * Routing uses an open-addressing hash table (FNV-1a, linear probing).
 * Each entry pre-builds the full HTTP 302 response so handle_request
 * can call send() directly without any strlen/memcpy on the hot path.
 *
 * HASH_TABLE_SIZE must be a power of 2 and > 2× the maximum number of
 * entries to keep load factor below 0.5.  256 comfortably covers the
 * 20 built-in redirects plus reasonable runtime additions.
 */
#define HASH_TABLE_SIZE 256

static const char RESPONSE_HEADER[] = "HTTP/1.1 302 Found\r\nLocation: ";
static const char RESPONSE_FOOTER[] = "\r\nContent-Length: 0\r\n\r\n";
#define RESPONSE_HEADER_LEN (sizeof(RESPONSE_HEADER) - 1)
#define RESPONSE_FOOTER_LEN (sizeof(RESPONSE_FOOTER) - 1)

typedef struct {
    char   *key;
    char   *url;
    char   *response;       /* pre-built HTTP 302 response */
    size_t  response_len;
} Redirect;

static Redirect hash_table[HASH_TABLE_SIZE]; /* zero-initialised by C */
static size_t redirects_count    = 0;
static int    routing_initialized = 0;

/* Default entries (order does not matter for a hash table) */
static const struct { const char *key; const char *url; } default_redirects[] = {
    {"amazon",    "https://www.amazon.com"},
    {"apple",     "https://www.apple.com"},
    {"bbc",       "https://www.bbc.com"},
    {"bing",      "https://www.bing.com"},
    {"cnn",       "https://www.cnn.com"},
    {"ebay",      "https://www.ebay.com"},
    {"facebook",  "https://www.facebook.com"},
    {"google",    "https://www.google.com"},
    {"guardian",  "https://www.theguardian.com"},
    {"instagram", "https://www.instagram.com"},
    {"linkedin",  "https://www.linkedin.com"},
    {"microsoft", "https://www.microsoft.com"},
    {"netflix",   "https://www.netflix.com"},
    {"nytimes",   "https://www.nytimes.com"},
    {"pinterest", "https://www.pinterest.com"},
    {"reddit",    "https://www.reddit.com"},
    {"twitter",   "https://www.twitter.com"},
    {"wikipedia", "https://www.wikipedia.org"},
    {"yahoo",     "https://www.yahoo.com"},
    {"youtube",   "https://www.youtube.com"},
};
#define DEFAULT_REDIRECTS_COUNT (sizeof(default_redirects) / sizeof(default_redirects[0]))

/* FNV-1a: fast, good distribution for short strings */
static uint32_t fnv1a(const char *key) {
    uint32_t hash = 2166136261u;
    while (*key) {
        hash ^= (uint8_t)*key++;
        hash *= 16777619u;
    }
    return hash;
}

/* Build the pre-computed HTTP 302 response for a given URL. */
static char *build_response(const char *url, size_t url_len, size_t *out_len) {
    size_t total = RESPONSE_HEADER_LEN + url_len + RESPONSE_FOOTER_LEN;
    char *resp = malloc(total);
    if (!resp) return NULL;
    char *p = resp;
    memcpy(p, RESPONSE_HEADER, RESPONSE_HEADER_LEN); p += RESPONSE_HEADER_LEN;
    memcpy(p, url, url_len);                         p += url_len;
    memcpy(p, RESPONSE_FOOTER, RESPONSE_FOOTER_LEN);
    *out_len = total;
    return resp;
}

/*
 * Find the slot for `key` using linear probing.
 * Returns the index of the matching slot or the first empty slot.
 * Returns -1 if the table is full (should never happen at <50% load).
 */
static int find_slot(const char *key) {
    uint32_t h = fnv1a(key) & (HASH_TABLE_SIZE - 1);
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        uint32_t slot = (h + (uint32_t)i) & (HASH_TABLE_SIZE - 1);
        if (hash_table[slot].key == NULL) return (int)slot;
        if (strcmp(hash_table[slot].key, key) == 0) return (int)slot;
    }
    return -1;
}

/* Internal insert used by init_routing (bypasses the init check). */
static int insert_redirect(const char *key, const char *url) {
    int slot = find_slot(key);
    if (slot < 0) return 0;

    size_t url_len = strlen(url);

    if (hash_table[slot].key != NULL) {
        /* Update existing entry */
        free(hash_table[slot].url);
        free(hash_table[slot].response);
        hash_table[slot].url = strdup(url);
        if (!hash_table[slot].url) return 0;
        hash_table[slot].response = build_response(url, url_len,
                                        &hash_table[slot].response_len);
        return hash_table[slot].response != NULL;
    }

    /* New entry */
    hash_table[slot].key = strdup(key);
    if (!hash_table[slot].key) return 0;
    hash_table[slot].url = strdup(url);
    if (!hash_table[slot].url) { free(hash_table[slot].key); hash_table[slot].key = NULL; return 0; }
    hash_table[slot].response = build_response(url, url_len, &hash_table[slot].response_len);
    if (!hash_table[slot].response) {
        free(hash_table[slot].key); hash_table[slot].key = NULL;
        free(hash_table[slot].url); hash_table[slot].url = NULL;
        return 0;
    }
    redirects_count++;
    return 1;
}

void init_routing(void) {
    if (routing_initialized) return;
    routing_initialized = 1;
    for (size_t i = 0; i < DEFAULT_REDIRECTS_COUNT; i++) {
        insert_redirect(default_redirects[i].key, default_redirects[i].url);
    }
}

int add_redirect(const char *key, const char *url) {
    if (!key || !url) return 0;
    if (!routing_initialized) init_routing();
    return insert_redirect(key, url);
}

const char *find_redirect(const char *key) {
    if (!key) return NULL;
    if (!routing_initialized) {
        init_routing();
        if (!routing_initialized) return NULL;
    }

    uint32_t h = fnv1a(key) & (HASH_TABLE_SIZE - 1);
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        uint32_t slot = (h + (uint32_t)i) & (HASH_TABLE_SIZE - 1);
        if (hash_table[slot].key == NULL) return NULL;
        if (strcmp(hash_table[slot].key, key) == 0) return hash_table[slot].url;
    }
    return NULL;
}

/*
 * Fast path: returns the pre-built HTTP 302 response and its length.
 * Eliminates strlen + memcpy from handle_request on every hit.
 * Returns NULL if the key is not found.
 */
const char *find_redirect_response(const char *key, size_t *response_len) {
    if (!key) return NULL;
    if (!routing_initialized) {
        init_routing();
        if (!routing_initialized) return NULL;
    }

    uint32_t h = fnv1a(key) & (HASH_TABLE_SIZE - 1);
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        uint32_t slot = (h + (uint32_t)i) & (HASH_TABLE_SIZE - 1);
        if (hash_table[slot].key == NULL) return NULL;
        if (strcmp(hash_table[slot].key, key) == 0) {
            if (response_len) *response_len = hash_table[slot].response_len;
            return hash_table[slot].response;
        }
    }
    return NULL;
}

void cleanup_routing(void) {
    if (!routing_initialized) return;
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        if (hash_table[i].key) {
            free(hash_table[i].key);
            free(hash_table[i].url);
            free(hash_table[i].response);
            hash_table[i].key      = NULL;
            hash_table[i].url      = NULL;
            hash_table[i].response = NULL;
            hash_table[i].response_len = 0;
        }
    }
    redirects_count     = 0;
    routing_initialized = 0;
}
