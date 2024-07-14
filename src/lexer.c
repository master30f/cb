#include "lexer.h"

#include "stdbool.h"
#include "assert.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#undef  STB_DS_IMPLEMENTATION

static inline bool isAlphabetic(u8 ch)
{
    return ('A' <= ch && ch <= 'Z') || ('a' <= ch && ch <= 'z');
}

static inline bool isNumeric(u8 ch)
{
    return '0' <= ch && ch <= '9';
}

static inline bool isAlphanumeric(u8 ch)
{
    return isAlphabetic(ch) || isNumeric(ch);
}

static inline bool isWhitespace(u8 ch)
{
    return ch <= ' ' || ch == 127;
}

static inline bool isBitLiteralWidth(u8 ch)
{
    return ch == 'b' || ch == 'q' || ch == 'o' || ch == 'x';
}

static Int intPow(Int base, Int exponent)
{
    Int result = 1;

    for (Int i = 0; i < exponent; i++) result *= base;

    return result;
}

static Int intParse(usize len, const u8 * string)
{
    Int result = 0;

    for (usize i = 0; i < len; i++)
    {
        assert(isNumeric(string[i]));
        result += (Int)(string[i] - '0') * intPow(10, len - i - 1);
    }

    return result;
}

Token * lex(usize programLen, const u8 * program, usize * tokenCount)
{
    // FIXME: memory leak! free!
    Token * tokens = NULL;

    usize cursor = 0;

    for (;;)
    {
        // skip whitespace & check EOF
        while (isWhitespace(program[cursor]) && cursor < programLen) cursor++;
        if (cursor >= programLen) break;

        usize begin = cursor;

        if (isAlphabetic(program[cursor]))
        {
            do cursor++;
            while (isAlphanumeric(program[cursor]) && cursor < programLen);

            usize length = cursor - begin;

            // FIXME: memory leak!
            u8 * string = malloc(length + 1);
            assert(string);
            string[length] = '\0';

            memcpy(string, &program[begin], length);

            if (strcmp((const char *)string, "export") == 0)
            {
                free(string);
                arrpush(tokens, ((Token){ TT_EXPORT, begin, length }));
            }
            else if (strcmp((const char *)string, "return") == 0)
            {
                free(string);
                arrpush(tokens, ((Token){ TT_RETURN, begin, length }));
            }
            else
            {
                arrpush(tokens, ((Token){ TT_ID, begin, length, .string = string }));
            }
        }
        else if (isNumeric(program[cursor]))
        {
            do cursor++;
            while (isNumeric(program[cursor]) && cursor < programLen);

            usize length = cursor - begin;

            if (length == 1 && program[begin] == '0' && isBitLiteralWidth(program[cursor]))
            {
                assert(0 && "TODO: bit literals");
            }
            else if (program[cursor] == '\'')
            {
                assert(0 && "TODO: base literals");
            }
            else if (program[cursor] == '.')
            {
                assert(0 && "TODO: float literals");
            }
            else
            {
                arrpush(tokens, ((Token){ TT_INT, begin, length, .value = intParse(length, program + begin) }));
            }
        }
        else switch (program[cursor++])
        {
            case '(': arrpush(tokens, ((Token){ TT_L_PAREN , begin , 1 })); break;
            case ')': arrpush(tokens, ((Token){ TT_R_PAREN , begin , 1 })); break;
            case '{': arrpush(tokens, ((Token){ TT_L_BRACE , begin , 1 })); break;
            case '}': arrpush(tokens, ((Token){ TT_R_BRACE , begin , 1 })); break;
            case ';': arrpush(tokens, ((Token){ TT_SEMI    , begin , 1 })); break;
            case '+': arrpush(tokens, ((Token){ TT_PLUS    , begin , 1 })); break;
            case '=': arrpush(tokens, ((Token){ TT_EQUALS  , begin , 1 })); break;
            case '@': arrpush(tokens, ((Token){ TT_AT      , begin , 1 })); break;
            default:  assert(0 && "invalid character"); break;
        }
    }

    *tokenCount = arrlen(tokens);
    return tokens;
}