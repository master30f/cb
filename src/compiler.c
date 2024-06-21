#include "compiler.h"

#include "assert.h"
#include "stb_ds.h"

Instruction * instructions;

// TODO: must be a stack
usize unoccupiedSlot;

static void compileNamespace(const Node * node)
{
    assert(node->type == NT_NAMESPACE);

    for (usize i = 0; i < node->bodyLen; i++)
    {
        compileStatement(node->body[i]);
    }
}

static void compileFunctionDeclaration(const Node * node)
{
    assert(node->type == NT_FUNC_DECL);

    for (usize i = 0; i < node->bodyLen; i++)
    {
        compileStatement(node->body[i]);
    }
}

static void compileReturn(const Node * node)
{
    assert(node->type == NT_RETURN);

    if (node->node != NULL)
    {
        usize outSlot = compileExpression(node->node);
        arrpush(instructions, ((Instruction){ IT_RET_SET_32, .srcSlot = outSlot, .dstSlot = 0 }));
    }

    arrpush(instructions, ((Instruction){ IT_RETURN }));
}

static usize compileAtom(const Node * node)
{
    assert(node->type == NT_ATOM);

    const Token * token = node->token;

    usize outSlot = ~0;

    switch (token->type)
    {
        case TT_INT:
        {
            outSlot = unoccupiedSlot++;
            arrpush(instructions, ((Instruction){ IT_VALUE_32, .dstSlot = outSlot, .srcValue32 = (u32) token->value }));
        } break;
        default: assert(0 && "TODO:");
    }

    return outSlot;
}

static usize compileExpression(const Node * node)
{
    switch (node->type)
    {
        case NT_ATOM: return compileAtom(node);
        default: assert(0 && "TODO:");
    }
}

static void compileStatement(const Node * node)
{
    switch (node->type)
    {
        case NT_NAMESPACE: compileNamespace(node); break;
        case NT_FUNC_DECL: compileFunctionDeclaration(node); break;
        case NT_RETURN:    compileReturn(node); break;
        default: assert(0 && "TODO:");
    }
}

Instruction * compile(const Node * ast, usize * instructionCount)
{
    // FIXME: memory leak!
    instructions = NULL;
    unoccupiedSlot = 0;

    compileNamespace(ast);

    *instructionCount = arrlen(instructions);
    return instructions;
}