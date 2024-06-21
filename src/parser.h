#pragma once

#include "number.h"
#include "lexer.h"

typedef u16 NodeType;
#define NT_NONE      0
#define NT_NAMESPACE 1
#define NT_FUNC_DECL 2
#define NT_RETURN    3
#define NT_ATOM      4

typedef struct Node_t {
    NodeType type;

    usize begin;
    usize length;

    union {
        // NT_NAMESPACE, NT_FUNC_DECL
        struct {
            union {
                // NT_FUNC_DECL
                struct {
                    const Token * returnType;
                };
            };

            const Token *   name;
            usize           bodyLen;
            struct Node_t * body[];
        };
        // NT_RETURN
        struct {
            struct Node_t * node;
        };
        // NT_ATOM
        struct {
            const Token * token;
        };
    };
} Node;

static Node * parseStatement(void);

Node * parse(usize tokenCount, const Token * tokens);

void printNode(Node * node, u32 level);