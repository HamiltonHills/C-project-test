#include "kv.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *key;
    char *value;
} kv_entry;

struct kv_config {
    kv_entry *entries;
    size_t count;
    size_t cap;
};

static char *dup_range(const char *start, const char *end)
{
    size_t len = (size_t)(end - start);
    char *s = malloc(len + 1);
    if (s) {
        memcpy(s, start, len);
        s[len] = '\0';
    }
    return s;
}

static void trim(const char **start, const char **end)
{
    while (*start < *end && isspace((unsigned char)**start))
        (*start)++;
    while (*end > *start && isspace((unsigned char)(*end)[-1]))
        (*end)--;
}

static kv_entry *find(const kv_config *cfg, const char *key)
{
    for (size_t i = 0; i < cfg->count; i++)
        if (strcmp(cfg->entries[i].key, key) == 0)
            return &cfg->entries[i];
    return NULL;
}

/* Takes ownership of key and value. */
static kv_status put(kv_config *cfg, char *key, char *value)
{
    kv_entry *existing = find(cfg, key);
    if (existing) {
        free(key);
        free(existing->value);
        existing->value = value;
        return KV_OK;
    }
    if (cfg->count == cfg->cap) {
        size_t cap = cfg->cap ? cfg->cap * 2 : 8;
        kv_entry *grown = realloc(cfg->entries, cap * sizeof *grown);
        if (!grown) {
            free(key);
            free(value);
            return KV_ERR_NOMEM;
        }
        cfg->entries = grown;
        cfg->cap = cap;
    }
    cfg->entries[cfg->count].key = key;
    cfg->entries[cfg->count].value = value;
    cfg->count++;
    return KV_OK;
}

static kv_status parse_line(kv_config *cfg, const char *start, const char *end)
{
    trim(&start, &end);
    if (start == end || *start == '#' || *start == ';')
        return KV_OK;

    const char *eq = memchr(start, '=', (size_t)(end - start));
    if (!eq)
        return KV_ERR_SYNTAX;

    const char *key_start = start, *key_end = eq;
    const char *val_start = eq + 1, *val_end = end;
    trim(&key_start, &key_end);
    trim(&val_start, &val_end);
    if (key_start == key_end)
        return KV_ERR_SYNTAX;

    char *key = dup_range(key_start, key_end);
    char *value = dup_range(val_start, val_end);
    if (!key || !value) {
        free(key);
        free(value);
        return KV_ERR_NOMEM;
    }
    return put(cfg, key, value);
}

kv_status kv_parse(const char *text, kv_config **out, size_t *err_line)
{
    kv_config *cfg = calloc(1, sizeof *cfg);
    if (!cfg)
        return KV_ERR_NOMEM;

    size_t line = 1;
    const char *p = text;
    while (*p) {
        const char *nl = strchr(p, '\n');
        const char *end = nl ? nl : p + strlen(p);
        kv_status st = parse_line(cfg, p, end);
        if (st != KV_OK) {
            if (err_line)
                *err_line = line;
            kv_free(cfg);
            return st;
        }
        if (!nl)
            break;
        p = nl + 1;
        line++;
    }
    *out = cfg;
    return KV_OK;
}

kv_status kv_load(const char *path, kv_config **out, size_t *err_line)
{
    FILE *f = fopen(path, "rb");
    if (!f)
        return KV_ERR_IO;

    char *buf = NULL;
    size_t len = 0, cap = 0;
    char chunk[4096];
    size_t n;
    while ((n = fread(chunk, 1, sizeof chunk, f)) > 0) {
        if (len + n + 1 > cap) {
            size_t want = cap ? cap * 2 : sizeof chunk;
            while (want < len + n + 1)
                want *= 2;
            char *grown = realloc(buf, want);
            if (!grown) {
                free(buf);
                fclose(f);
                return KV_ERR_NOMEM;
            }
            buf = grown;
            cap = want;
        }
        memcpy(buf + len, chunk, n);
        len += n;
    }
    int read_failed = ferror(f);
    fclose(f);
    if (read_failed) {
        free(buf);
        return KV_ERR_IO;
    }

    if (!buf)
        return kv_parse("", out, err_line);
    buf[len] = '\0';
    kv_status st = kv_parse(buf, out, err_line);
    free(buf);
    return st;
}

const char *kv_get(const kv_config *cfg, const char *key)
{
    const kv_entry *e = find(cfg, key);
    return e ? e->value : NULL;
}

size_t kv_count(const kv_config *cfg)
{
    return cfg->count;
}

void kv_free(kv_config *cfg)
{
    if (!cfg)
        return;
    for (size_t i = 0; i < cfg->count; i++) {
        free(cfg->entries[i].key);
        free(cfg->entries[i].value);
    }
    free(cfg->entries);
    free(cfg);
}

const char *kv_strerror(kv_status status)
{
    switch (status) {
    case KV_OK:
        return "ok";
    case KV_ERR_SYNTAX:
        return "syntax error";
    case KV_ERR_NOMEM:
        return "out of memory";
    case KV_ERR_IO:
        return "cannot read file";
    }
    return "unknown error";
}
