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

static NodeHeader * allocNode(usize inSize)
{
    usize size = inSize + sizeof(NodeHeader);

    // TODO: investigate the alignment of node bodies
    usize rem  = size % alignof(Node);
    usize pad  = rem == 0 ? 0 : alignof(Node) - rem;

    assert(nodeArenaCursor + size <= NODE_ARENA_SIZE);

    Node * node = (Node *)(nodeArena + nodeArenaCursor);
    nodeArenaCursor += size + pad;

    return node;
}

static Node * parseFunctionDeclaration(void)
{
    const Token * attributeName  = NULL;
    const Token * attributeValue = NULL;

    const Token * first = &tokens[cursor];
    if (first->type == TT_AT)
    {
        assert(cursor++ < tokenCount);

        attributeName = &tokens[cursor];
        assert(cursor++ < tokenCount && attributeName->type == TT_ID);

        assert(cursor < tokenCount && tokens[cursor++].type == TT_L_PAREN);

        attributeValue = &tokens[cursor];
        assert(cursor++ < tokenCount && attributeValue->type == TT_ID);

        assert(cursor < tokenCount && tokens[cursor++].type == TT_R_PAREN);
    }

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

    NodeHeader * nodeHeader  = allocNode(sizeof(NodeFuncDecl) + arrlen(body) * sizeof(Node *));
    NodeFuncDecl * nodeBody  = (NodeFuncDecl *) &nodeHeader->body;
    nodeHeader->type         = NT_FUNC_DECL;
    nodeHeader->begin        = first->begin;
    nodeHeader->length       = last->begin + last->length;
    nodeBody->attributeName  = attributeName;
    nodeBody->attributeValue = attributeValue;
    nodeBody->returnType     = type;
    nodeBody->name           = name;
    nodeBody->bodyLen        = arrlen(body);
    memcpy(&nodeBody->body, body, arrlen(body) * sizeof(body[0]));
    arrfree(body);

    return nodeHeader;
}

static Node * parseAtom(void)
{
    const Token * token = &tokens[cursor];

    if (cursor++ >= tokenCount || (token->type != TT_INT && token->type != TT_ID)) return NULL;

    NodeHeader * nodeHeader = allocNode(sizeof(NodeAtom));
    NodeAtom * nodeBody     = (NodeAtom *) &nodeHeader->body;
    nodeHeader->type        = NT_ATOM;
    nodeHeader->begin       = token->begin;
    nodeHeader->length      = token->length;
    nodeBody->token         = token;


    return nodeHeader;
}

static Node * parseExpression(void)
{
    Node * left = parseAtom();

    if (tokens[cursor].type == TT_PLUS)
    {
        cursor++;
        Node * right = parseExpression();

        NodeHeader * nodeHeader = allocNode(sizeof(NodeAddition));
        NodeAddition * nodeBody = (NodeAddition *) &nodeHeader->body;
        nodeHeader->type        = NT_ADDITION;
        nodeHeader->begin       = left->begin;
        nodeHeader->length      = right->begin + right->length;
        nodeBody->left          = left;
        nodeBody->right         = right;

        return nodeHeader;
    }

    return left;
}

static Node * parseReturn(void)
{
    const Token * begin = &tokens[cursor];
    if (cursor++ >= tokenCount || begin->type != TT_RETURN) return NULL;

    Node * value = parseExpression();
    assert(value);

    const Token * last = &tokens[cursor];
    assert(cursor++ < tokenCount && last->type == TT_SEMI);

    NodeHeader * nodeHeader = allocNode(sizeof(NodeReturn));
    NodeReturn * nodeBody   = (NodeReturn *) &nodeHeader->body;
    nodeHeader->type        = NT_RETURN;
    nodeHeader->begin       = begin->begin;
    nodeHeader->length      = last->begin + last->length;
    nodeBody->value         = value;

    return nodeHeader;
}

static Node * parseVariableDeclaration(void)
{
    const Token * type = &tokens[cursor];
    if (cursor++ >= tokenCount || type->type != TT_ID) return NULL;

    const Token * name = &tokens[cursor];
    if (cursor++ >= tokenCount || name->type != TT_ID) return NULL;

    if (cursor >= tokenCount || tokens[cursor++].type != TT_EQUALS) return NULL;

    Node * value = parseExpression();
    assert(value);

    const Token * last = &tokens[cursor];
    assert(cursor++ < tokenCount && last->type == TT_SEMI);

    NodeHeader * nodeHeader = allocNode(sizeof(NodeVarDecl));
    NodeVarDecl * nodeBody  = (NodeVarDecl *) &nodeHeader->body;
    nodeHeader->type        = NT_VAR_DECL;
    nodeHeader->begin       = type->begin;
    nodeHeader->length      = last->begin + last->length;
    nodeBody->type          = type;
    nodeBody->name          = name;
    nodeBody->value         = value;

    return nodeHeader;
}

static Node * parseStatement(void)
{
    Node * node     = 0;
    usize  rollback = cursor;

    if (node = parseFunctionDeclaration(), node) return node;
    cursor = rollback;
    if (node = parseReturn(), node) return node;
    cursor = rollback;
    if (node = parseVariableDeclaration(), node) return node;

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

    NodeHeader * nodeHeader  = allocNode(sizeof(NodeNamespace) + arrlen(body) * sizeof(Node *));
    NodeNamespace * nodeBody = (NodeNamespace *) &nodeHeader->body;
    nodeHeader->type         = NT_NAMESPACE;
    nodeHeader->begin        = 0;
    // TODO: make this the actual length of the program
    nodeHeader->length       = tokens[tokenCount - 1].begin + tokens[tokenCount - 1].length;
    nodeBody->name           = NULL;
    nodeBody->bodyLen        = arrlen(body);
    memcpy(&nodeBody->body, body, arrlen(body) * sizeof(body[0]));
    arrfree(body);

    return nodeHeader;
}

static inline void indent(u32 n)
{
    for (u32 i = 0; i < n; i++)
        printf("  ");
}

void printNode(const Node * node, u32 level)
{
    indent(level);

    switch (node->type)
    {
        case NT_NONE: printf("NONE\n"); break;
        case NT_NAMESPACE:
        {
            NodeNamespace * body = (NodeNamespace *) &node->body;

            printf("NAMESPACE ");

            if (body->name) printf("\"%s\"\n", body->name->string);
            else printf("null\n");

            for (usize i = 0; i < body->bodyLen; i++)
                printNode(body->body[i], level + 1);
        } break;
        case NT_FUNC_DECL:
        {
            NodeFuncDecl * body = (NodeFuncDecl *) &node->body;

            printf("FUNC_DECL %s \"%s\"\n", body->returnType->string, body->name->string);

            for (usize i = 0; i < body->bodyLen; i++)
                printNode(body->body[i], level + 1);
        } break;
        case NT_RETURN:
        {
            NodeReturn * body = (NodeReturn *) &node->body;

            printf("RETURN\n");
            printNode(body->value, level + 1);
        } break;
        case NT_ATOM:
        {
            NodeAtom * body = (NodeAtom *) &node->body;

            printf("ATOM ");

            switch (body->token->type)
            {
                case TT_INT: printf("int %ld\n", body->token->value); break;
                default:     printf("<unknown>\n");
            }
        } break;
        default: printf("<UNKNOWN>\n");
    }
}