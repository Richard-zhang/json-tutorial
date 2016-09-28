#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <math.h>
#include <stdio.h>

#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0)
#define ISDIGIT(ch)         ( (ch) >= '0' && (ch) <= '9')
#define ISDIGIT1TO9(ch)     ( (ch) >= '1' && (ch) <= '9')
#define ISSPACE(ch)         ((ch) == '\0' || (ch) == ' ' || (ch) == '\n' || (ch) == '\r' || (ch) == '\t')
typedef struct {
    const char* json;
}lept_context;

static void lept_parse_whitespace(lept_context* c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, char literal) {
    EXPECT(c, literal);
    switch (literal) {
        case 't':
            if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
                return LEPT_PARSE_INVALID_VALUE;
            break;
        case 'f':
            if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
                return LEPT_PARSE_INVALID_VALUE;
            break;
        case 'n':
            if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
                return LEPT_PARSE_INVALID_VALUE;
            break;
        default :
            perror("FUCK");
            exit(EXIT_FAILURE);

    }

    c->json += (literal == 'f') ? 4 : 3;
    switch(literal) {
        case 't':
            v->type = LEPT_TRUE;
            break;
        case 'f':
            v->type = LEPT_FALSE;
            break;
        case 'n':
            v->type = LEPT_NULL;
            break;
        default:
            perror("fuck");
            exit(EXIT_FAILURE);
    }

    return LEPT_PARSE_OK;
}

static int lept_parse_true(lept_context* c, lept_value* v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}

static int lept_is_valid_exp(char *cur) {
    assert(!ISDIGIT(*cur));
    if(ISSPACE(*cur)) return 0;

    if(*cur == 'e' || *cur == 'E') {
        cur++;
        if(*cur == '+' || *cur == '-') cur++;
        if(!ISDIGIT(*cur)) return 1;
    } else {
        return 1;
    }

    while(ISDIGIT(*cur)) {
        cur++;
    }

    return !ISSPACE(*cur);
}

static int lept_is_valid_frac_or_exp(char* cur) {
    assert(!ISDIGIT(*cur));
    if(ISSPACE(*cur)) return 0;

    if(*cur == '.') {
        cur++;
        if(!ISDIGIT(*cur)) return 1;
        while(ISDIGIT(*cur)) cur++;
    }

    return lept_is_valid_exp(cur);
}


static int lept_parse_number(lept_context* c, lept_value* v) {
    char* end;
    const char* cur = c->json;

    if(*cur == '-') cur++;

    if(ISDIGIT1TO9(*cur)) {
        while(ISDIGIT(*cur)) cur++;
    } else if (ISDIGIT(*cur)) {
        assert(*cur == '0');
        cur++;
        if(*cur != '.' && !ISSPACE(*cur)) return LEPT_PARSE_ROOT_NOT_SINGULAR;
    } else {
        return LEPT_PARSE_INVALID_VALUE;
    }

    if(lept_is_valid_frac_or_exp(cur)) return LEPT_PARSE_INVALID_VALUE;

    v->n = strtod(c->json, &end);
    if (c->json == end)
        return LEPT_PARSE_INVALID_VALUE;
    if (v->n == HUGE_VAL || v->n == -HUGE_VAL)
        return LEPT_PARSE_NUMBER_TOO_BIG;
    c->json = end;
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, 't');
        case 'f':  return lept_parse_literal(c, v, 'f');
        case 'n':  return lept_parse_literal(c, v, 'n');
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
