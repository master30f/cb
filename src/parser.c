#include "parser.h"

#include "stdlib.h"
#include "assert.h"
#include "stdalign.h"
#include "stdio.h"

#include "stb_ds.h"

#define NODE_ARENA_SIZE 1000

usize         tokenCount;
const Token * tokens;
usize         cursor;
u8 *          nodeArena;
usize         nodeArenaCursor;

static Node * allocNode(usize size)
{
    usize rem  = size % alignof(Node);
    usize pad  = rem == 0 ? 0 : alignof(Node) - rem;

    assert(nodeArenaCursor + size <= NODE_ARENA_SIZE);

    Node * node = (Node *)(nodeArena + nodeArenaCursor);
    nodeArenaCursor += size + pad;

    return node;
}

static Node * parseFunctionDeclaration(void)
{
    const Token * type = &tokens[cursor];
    if (cursor++ >= tokenCount || type->type != TT_ID) return NULL;

    const Token * name = &tokens[cursor];
    if (cursor++ >= tokenCount || name->type != TT_ID) return NULL;

    if (cursor >= tokenCount || tokens[cursor++].type != TT_L_PAREN) return NULL;

    assert(cursor < tokenCount && tokens[cursor++].type == TT_R_PAREN);

    assert(cursor < tokenCount && tokens[cursor++].type == TT_L_BRACE);

    Node ** body = NULL;
    while (cursor < tokenCount && tokens[cursor].type != TT_R_BRACE) arrpush(body, parseStatement());

    const Token * last = &tokens[cursor];
    assert(cursor < tokenCount && tokens[cursor++].type == TT_R_BRACE);

    Node * node      = allocNode(sizeof(Node) + arrlen(body) * sizeof(Node *));
    node->type       = NT_FUNC_DECL;
    node->begin      = type->begin;
    node->length     = last->begin + last->length;
    node->returnType = type;
    node->name       = name;
    node->bodyLen    = arrlen(body);
    memcpy(&node->body, body, arrlen(body) * sizeof(body[0]));

    arrfree(body);
    return node;
}

static Node * parseExpression(void)
{
    const Token * token = &tokens[cursor];

    if (tokens[cursor++].type != TT_INT) return NULL;

    Node * node  = allocNode(sizeof(Node));
    node->type   = NT_ATOM;
    node->begin  = token->begin;
    node->length = token->length;
    node->token  = token;

    return node;
}

static Node * parseReturn(void)
{
    const Token * begin = &tokens[cursor];

    if (tokens[cursor++].type != TT_RETURN) return NULL;

    Node * value = parseExpression();

    const Token * last = &tokens[cursor];
    assert(tokens[cursor++].type == TT_SEMI);

    Node * node  = allocNode(sizeof(Node));
    node->type   = NT_RETURN;
    node->begin  = begin->begin;
    node->length = last->begin + last->length;
    node->node   = value;

    return node;
}

static Node * parseStatement(void)
{
    Node * node     = 0;
    usize  rollback = cursor;

    if (node = parseFunctionDeclaration(), node) return node;
    cursor = rollback;
    if (node = parseReturn(), node) return node;

    assert(0 && "invalid syntax");
}

Node * parse(usize inTokenCount, const Token * inTokens)
{
    tokenCount      = inTokenCount;
    tokens          = inTokens;
    cursor          = 0;
    // FIXME: memory leak! free!
    // FIXME: allocate more arenas if memory runs out!
    nodeArena       = malloc(NODE_ARENA_SIZE);
    nodeArenaCursor = 0;
    assert(nodeArena);

    Node ** body = NULL;

    while (cursor < tokenCount)
    {
        arrpush(body, parseStatement());
    }

    Node * node   = allocNode(sizeof(Node) + arrlen(body) * sizeof(Node *));
    node->type    = NT_NAMESPACE;
    node->begin   = 0;
    // TODO: make this the actual length of the program
    node->length  = tokens[tokenCount - 1].begin + tokens[tokenCount - 1].length;
    node->name    = NULL;
    node->bodyLen = arrlen(body);
    memcpy(&node->body, body, arrlen(body) * sizeof(body[0]));

    arrfree(body);
    return node;
}

static inline void indent(u32 n)
{
    for (u32 i = 0; i < n; i++)
        printf("  ");
}

void printNode(Node * node, u32 level)
{
    indent(level);

    switch (node->type)
    {
        case NT_NONE: printf("NONE\n"); break;
        case NT_NAMESPACE:
        {
            printf("NAMESPACE ");

            if (node->name) printf("\"%s\"\n", node->name->string);
            else printf("null\n");

            for (usize i = 0; i < node->bodyLen; i++)
                printNode(node->body[i], level + 1);
        } break;
        case NT_FUNC_DECL:
        {
            printf("FUNC_DECL %s \"%s\"\n", node->returnType->string, node->name->string);

            for (usize i = 0; i < node->bodyLen; i++)
                printNode(node->body[i], level + 1);
        } break;
        case NT_RETURN:
        {
            printf("RETURN\n");
            printNode(node->node, level + 1);
        } break;
        case NT_ATOM:
        {
            printf("ATOM ");

            switch (node->token->type)
            {
                case TT_INT: printf("int %lld\n", node->token->value); break;
                default: printf("<unknown>\n");
            }
        } break;
        default: printf("<UNKNOWN>\n");
    }
}