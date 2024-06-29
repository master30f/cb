#pragma once

#include "number.h"
#include "lexer.h"

typedef u16 NodeType;
#define NT_NONE      0
#define NT_NAMESPACE 1
#define NT_FUNC_DECL 2
#define NT_RETURN    3
#define NT_ATOM      4
#define NT_ADDITION  5
#define NT_VAR_DECL  6

typedef struct {
    NodeType type;
    usize    begin;
    usize    length;

    u8       body[];
} NodeHeader;

typedef NodeHeader Node;

typedef struct {
    const Token * name;
    usize         bodyLen;
    const Node *  body[];
} NodeNamespace;

typedef struct {
    const Token * returnType;
    const Token * name;
    usize         bodyLen;
    const Node *  body[];
} NodeFuncDecl;

typedef struct {
    const Node * value;
} NodeReturn;

typedef struct {
    const Token * token;
} NodeAtom;

typedef struct {
    const Node * left;
    const Node * right;
} NodeAddition;

typedef struct {
    const Token * type;
    const Token * name;
    const Node *  value;
} NodeVarDecl;

static Node * parseStatement(void);

Node * parse(usize tokenCount, const Token * tokens);

void printNode(const Node * node, u32 level);