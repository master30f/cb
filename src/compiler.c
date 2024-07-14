#include <stdbool.h>

#include "compiler.h"

#include "assert.h"
#include "stb_ds.h"

static void compileNamespace(Context c, const Node * node)
{
    assert(node->type == NT_NAMESPACE);
    NodeNamespace * body = (NodeNamespace *) &node->body;

    for (usize i = 0; i < body->bodyLen; i++)
    {
        compileStatement(c, body->body[i]);
    }
}

static void compileFunctionDeclaration(Context c, const Node * node)
{
    assert(node->type == NT_FUNC_DECL);
    NodeFuncDecl * body = (NodeFuncDecl *) &node->body;

    // FIXME: memory leak!
    Instruction * instructions = NULL;
    usize slotCount = *c.sc;

    u8 * proto;
    if (body->attributeName != NULL && strcmp((char *) body->attributeName->string, "proto") == 0)
    {
        assert(body->attributeValue != NULL);

        proto = body->attributeValue->string;
    }
    else if (strcmp((char *) body->name->string, "main") == 0)
        proto = (u8 *) "main";
    else
        proto = (u8 *) "default";

    Context context = c;
    context.is = &instructions;
    context.sc = &slotCount;
    context.pt = proto;
    for (usize i = 0; i < body->bodyLen; i++)
    {
        compileStatement(context, body->body[i]);
    }


    arrpush(*c.is, ((Instruction){ IT_BEGIN_SCOPE, .slotCount = slotCount - *c.sc }));

    arrpush(*c.is, ((Instruction){ IT_FUNC_BEGIN, .jmpProtocol = proto }));

    // FIXME: eww
    for (usize i = 0; i < arrlenu(instructions); i++)
    {
        arrpush(*c.is, instructions[i]);
    }
    arrfree(instructions);

    arrpush(*c.ft, ((Symbol){ body->name->string, arrlenu(*c.is) }));
    
    arrpush(*c.is, ((Instruction){ IT_END_SCOPE }));
}

static void compileReturn(Context c, const Node * node)
{
    assert(node->type == NT_RETURN);
    NodeReturn * body = (NodeReturn *) &node->body;

    if (body->value != NULL)
    {
        usize srcSlot = compileExpression(c, body->value);
        arrpush(*c.is, ((Instruction){ IT_RET_MOVE_32, .srcSlot = srcSlot, .dstSlot = 0, .movProtocol = c.pt }));
    }

    arrpush(*c.is, ((Instruction){ IT_RETURN, .jmpProtocol = c.pt }));
}

static void compileVariableDeclaration(Context c, const Node * node)
{
    assert(node->type == NT_VAR_DECL);
    NodeVarDecl * body = (NodeVarDecl *) &node->body;

    assert(body->value != NULL);

    usize srcSlot = compileExpression(c, body->value);
    usize dstSlot = (*c.sc)++;
    arrpush(*c.is, ((Instruction){ IT_MOVE_32, .srcSlot = srcSlot, .dstSlot = dstSlot }));

    arrpush(*c.st, ((Symbol){ .name = body->name->string, .value = dstSlot }));
}

static usize compileAtom(Context c, const Node * node)
{
    assert(node->type == NT_ATOM);
    NodeAtom * body = (NodeAtom *) &node->body;

    const Token * token = body->token;

    usize outSlot = (*c.sc)++;

    switch (token->type)
    {
        case TT_INT:
        {
            arrpush(*c.is, ((Instruction){ IT_VALUE_32, .dstSlot = outSlot, .srcValue32 = (u32) token->value }));
        } break;
        case TT_ID:
        {
            usize srcSlot;
            bool found = false;

            for (usize i = 0; i < arrlenu(*c.st); i++)
            {
                if (strcmp((const char *) token->string, (const char *)((*c.st)[i].name)) == 0)
                {
                    srcSlot = (*c.st)[i].value;
                    found = true;
                    break;
                }
            }

            assert(found);

            arrpush(*c.is, ((Instruction){ IT_MOVE_32, .dstSlot = outSlot, .srcSlot = srcSlot }));
        } break;
        default: assert(0 && "TODO:");
    }

    return outSlot;
}

static usize compileAddition(Context c, const Node * node)
{
    assert(node->type == NT_ADDITION);
    NodeAddition * body = (NodeAddition *) &node->body;

    usize leftSlot  = compileExpression(c, body->left);
    usize rightSlot = compileExpression(c, body->right);

    usize outSlot = (*c.sc)++;

    arrpush(*c.is, ((Instruction){ IT_MOVE_32, .dstSlot = outSlot, .srcSlot = leftSlot }));
    arrpush(*c.is, ((Instruction){ IT_ADD_32, .dstSlot = outSlot, .srcSlot = rightSlot }));

    return outSlot;
}

static usize compileExpression(Context c, const Node * node)
{
    switch (node->type)
    {
        case NT_ATOM:     return compileAtom(c, node);
        case NT_ADDITION: return compileAddition(c, node);
        default: assert(0 && "TODO:");
    }
}

static void compileStatement(Context c, const Node * node)
{
    switch (node->type)
    {
        case NT_NAMESPACE: compileNamespace(c, node); break;
        case NT_FUNC_DECL: compileFunctionDeclaration(c, node); break;
        case NT_RETURN:    compileReturn(c, node); break;
        case NT_VAR_DECL:  compileVariableDeclaration(c, node); break;
        default:           assert(0 && "TODO:");
    }
}

Instruction * compile(const Node * ast, Symbol ** functionTable, usize * functionCount, usize * instructionCount)
{
    // FIXME: memory leak!
    Instruction * instructionStream = NULL;
    Symbol * symbolTable = NULL;
    Symbol * ft = NULL;
    usize slotCount = 0;

    compileNamespace((Context){ &instructionStream, &slotCount, &symbolTable, &ft, NULL }, ast);

    *functionTable = ft;
    *functionCount = arrlenu(ft);
    *instructionCount = arrlen(instructionStream);
    return instructionStream;
}