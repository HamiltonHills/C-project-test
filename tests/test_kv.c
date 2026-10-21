#include "kv.h"

#include <stdio.h>
#include <string.h>

static int failures;

#define CHECK(cond)                                                       \
    do {                                                                  \
        if (!(cond)) {                                                    \
            fprintf(stderr, "%s:%d: CHECK(%s) failed\n", __FILE__,        \
                    __LINE__, #cond);                                     \
            failures++;                                                   \
        }                                                                 \
    } while (0)

#define CHECK_STR(actual, expected)                                       \
    do {                                                                  \
        const char *a_ = (actual);                                        \
        if (!a_ || strcmp(a_, (expected)) != 0) {                         \
            fprintf(stderr, "%s:%d: got \"%s\", want \"%s\"\n", __FILE__, \
                    __LINE__, a_ ? a_ : "(null)", (expected));            \
            failures++;                                                   \
        }                                                                 \
    } while (0)

static void test_basic(void)
{
    kv_config *cfg = NULL;
    CHECK(kv_parse("name = demo\nport=8080\n", &cfg, NULL) == KV_OK);
    CHECK(kv_count(cfg) == 2);
    CHECK_STR(kv_get(cfg, "name"), "demo");
    CHECK_STR(kv_get(cfg, "port"), "8080");
    CHECK(kv_get(cfg, "missing") == NULL);
    kv_free(cfg);
}

static void test_comments_blank_and_crlf(void)
{
    kv_config *cfg = NULL;
    CHECK(kv_parse("# comment\r\n\r\n; also\r\n  a =  1 \r\n", &cfg, NULL) == KV_OK);
    CHECK(kv_count(cfg) == 1);
    CHECK_STR(kv_get(cfg, "a"), "1");
    kv_free(cfg);
}

static void test_override_and_empty_value(void)
{
    kv_config *cfg = NULL;
    CHECK(kv_parse("a=1\nb=\na=2", &cfg, NULL) == KV_OK);
    CHECK(kv_count(cfg) == 2);
    CHECK_STR(kv_get(cfg, "a"), "2");
    CHECK_STR(kv_get(cfg, "b"), "");
    kv_free(cfg);
}

static void test_value_keeps_inner_equals(void)
{
    kv_config *cfg = NULL;
    CHECK(kv_parse("url = http://x/?a=b", &cfg, NULL) == KV_OK);
    CHECK_STR(kv_get(cfg, "url"), "http://x/?a=b");
    kv_free(cfg);
}

static void test_syntax_errors(void)
{
    kv_config *cfg = NULL;
    size_t line = 0;
    CHECK(kv_parse("a=1\nno equals here\n", &cfg, &line) == KV_ERR_SYNTAX);
    CHECK(line == 2);
    CHECK(kv_parse("ok=1\n\n = value\n", &cfg, &line) == KV_ERR_SYNTAX);
    CHECK(line == 3);
}

static void test_empty_input(void)
{
    kv_config *cfg = NULL;
    CHECK(kv_parse("", &cfg, NULL) == KV_OK);
    CHECK(kv_count(cfg) == 0);
    kv_free(cfg);
}

static void test_load(void)
{
    kv_config *cfg = NULL;
    CHECK(kv_load("/nonexistent/kv.conf", &cfg, NULL) == KV_ERR_IO);

    const char *path = "test_load.conf";
    FILE *f = fopen(path, "wb");
    CHECK(f != NULL);
    if (!f)
        return;
    fputs("from = file\n", f);
    fclose(f);
    CHECK(kv_load(path, &cfg, NULL) == KV_OK);
    CHECK_STR(kv_get(cfg, "from"), "file");
    kv_free(cfg);
    remove(path);
}

static void test_strerror(void)
{
    CHECK_STR(kv_strerror(KV_OK), "ok");
    CHECK_STR(kv_strerror(KV_ERR_SYNTAX), "syntax error");
    CHECK_STR(kv_strerror(KV_ERR_NOMEM), "out of memory");
    CHECK_STR(kv_strerror(KV_ERR_IO), "cannot read file");
}

int main(void)
{
    test_basic();
    test_comments_blank_and_crlf();
    test_override_and_empty_value();
    test_value_keeps_inner_equals();
    test_syntax_errors();
    test_empty_input();
    test_load();
    test_strerror();

    if (failures) {
        fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }
    puts("all tests passed");
    return 0;
}
