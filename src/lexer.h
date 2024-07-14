#pragma once

#include "number.h"

typedef u8 TokenType;
#define TT_NONE     0
#define TT_EXPORT   1
#define TT_ID       2
#define TT_L_PAREN  3
#define TT_R_PAREN  4
#define TT_L_BRACE  5
#define TT_R_BRACE  6
#define TT_RETURN   7
#define TT_INT      8
#define TT_SEMI     9
#define TT_PLUS    10
#define TT_EQUALS  11
#define TT_AT      12

typedef struct {
    TokenType type;

    usize begin;
    usize length;

    union {
        // TT_INT
        struct {
            Int value;
        };
        // TT_ID
        struct {
            u8 * string;
        };
    };
} Token;

Token * lex(usize programLen, const u8 * program, usize * tokenCount);