#ifndef KV_H
#define KV_H

#include <stddef.h>

/* A parsed key=value config. Lines are `key = value`; blank lines and lines
 * starting with '#' or ';' are ignored. Whitespace around keys and values is
 * trimmed. A later key overrides an earlier one. */
typedef struct kv_config kv_config;

typedef enum {
    KV_OK = 0,
    KV_ERR_SYNTAX,   /* a non-comment line without '=' or with an empty key */
    KV_ERR_NOMEM,
    KV_ERR_IO
} kv_status;

/* Parses text. On KV_ERR_SYNTAX, *err_line (if non-NULL) is the 1-based line. */
kv_status kv_parse(const char *text, kv_config **out, size_t *err_line);

kv_status kv_load(const char *path, kv_config **out, size_t *err_line);

/* Returns the value for key, or NULL. The pointer stays valid until kv_free. */
const char *kv_get(const kv_config *cfg, const char *key);

size_t kv_count(const kv_config *cfg);

void kv_free(kv_config *cfg);

const char *kv_strerror(kv_status status);

#endif
