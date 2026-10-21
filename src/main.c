/* kvget FILE KEY  - prints KEY's value from a key=value config file.
 * kvget FILE      - prints how many keys the file defines.
 * Exits 1 if the key is missing, 2 on a usage or parse error. */
#include "kv.h"

#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "usage: %s FILE [KEY]\n", argv[0]);
        return 2;
    }

    kv_config *cfg = NULL;
    size_t line = 0;
    kv_status st = kv_load(argv[1], &cfg, &line);
    if (st == KV_ERR_SYNTAX) {
        fprintf(stderr, "%s:%zu: %s\n", argv[1], line, kv_strerror(st));
        return 2;
    }
    if (st != KV_OK) {
        fprintf(stderr, "%s: %s\n", argv[1], kv_strerror(st));
        return 2;
    }

    int rc = 0;
    if (argc == 2) {
        printf("%zu\n", kv_count(cfg));
    } else {
        const char *value = kv_get(cfg, argv[2]);
        if (value)
            puts(value);
        else
            rc = 1;
    }
    kv_free(cfg);
    return rc;
}
