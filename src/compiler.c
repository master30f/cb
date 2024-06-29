#include "compiler.h"

#include "assert.h"
#include "stb_ds.h"

static void compileNamespace(Instruction ** is, usize * sc, const Node * node)
{
    assert(node->type == NT_NAMESPACE);
    NodeNamespace * body = (NodeNamespace *) &node->body;

    for (usize i = 0; i < body->bodyLen; i++)
    {
        compileStatement(is, sc, body->body[i]);
    }
}

static void compileFunctionDeclaration(Instruction ** is, usize * sc, const Node * node)
{
    assert(node->type == NT_FUNC_DECL);
    NodeFuncDecl * body = (NodeFuncDecl *) &node->body;

    // FIXME: memory leak!
    Instruction * instructionStream = NULL;
    usize slotCount = *sc;

    for (usize i = 0; i < body->bodyLen; i++)
    {
        compileStatement(&instructionStream, &slotCount, body->body[i]);
    }

    arrpush(*is, ((Instruction){ IT_BEGIN_SCOPE, .slotCount = slotCount - *sc }));

    // FIXME: eww
    for (usize i = 0; i < arrlenu(instructionStream); i++)
    {
        arrpush(*is, instructionStream[i]);
    }
    arrfree(instructionStream);

    arrpush(*is, ((Instruction){ IT_END_SCOPE }));
}

static void compileReturn(Instruction ** is, usize * sc, const Node * node)
{
    assert(node->type == NT_RETURN);
    NodeReturn * body = (NodeReturn *) &node->body;

    if (body->value != NULL)
    {
        usize srcSlot = compileExpression(is, sc, body->value);
        arrpush(*is, ((Instruction){ IT_RET_MOVE_32, .srcSlot = srcSlot, .dstSlot = 0 }));
    }

    arrpush(*is, ((Instruction){ IT_RETURN }));
}

static void compileVariableDeclaration(Instruction ** is, usize * sc, const Node * node)
{
    assert(node->type == NT_VAR_DECL);
    NodeVarDecl * body = (NodeVarDecl *) &node->body;

    assert(body->value != NULL);

    usize srcSlot = compileExpression(is, sc, body->value);
    usize dstSlot = (*sc)++;
    arrpush(*is, ((Instruction){ IT_MOVE_32, .srcSlot = srcSlot, .dstSlot = dstSlot }));
}

static usize compileAtom(Instruction ** is, usize * sc, const Node * node)
{
    assert(node->type == NT_ATOM);
    NodeAtom * body = (NodeAtom *) &node->body;

    const Token * token = body->token;

    usize outSlot = (*sc)++;

    switch (token->type)
    {
        case TT_INT:
        {
            arrpush(*is, ((Instruction){ IT_VALUE_32, .dstSlot = outSlot, .srcValue32 = (u32) token->value }));
        } break;
        default: assert(0 && "TODO:");
    }

    return outSlot;
}

static usize compileAddition(Instruction ** is, usize * sc, const Node * node)
{
    assert(node->type == NT_ADDITION);
    NodeAddition * body = (NodeAddition *) &node->body;

    usize leftSlot  = compileExpression(is, sc, body->left);
    usize rightSlot = compileExpression(is, sc, body->right);

    usize outSlot = (*sc)++;

    arrpush(*is, ((Instruction){ IT_MOVE_32, .dstSlot = outSlot, .srcSlot = leftSlot }));
    arrpush(*is, ((Instruction){ IT_ADD_32, .dstSlot = outSlot, .srcSlot = rightSlot }));

    return outSlot;
}

static usize compileExpression(Instruction ** is, usize * sc, const Node * node)
{
    switch (node->type)
    {
        case NT_ATOM:     return compileAtom(is, sc, node);
        case NT_ADDITION: return compileAddition(is, sc, node);
        default: assert(0 && "TODO:");
    }
}

static void compileStatement(Instruction ** is, usize * sc, const Node * node)
{
    switch (node->type)
    {
        case NT_NAMESPACE: compileNamespace(is, sc, node); break;
        case NT_FUNC_DECL: compileFunctionDeclaration(is, sc, node); break;
        case NT_RETURN:    compileReturn(is, sc, node); break;
        case NT_VAR_DECL:  compileVariableDeclaration(is, sc, node); break;
        default:           assert(0 && "TODO:");
    }
}

Instruction * compile(const Node * ast, usize * instructionCount)
{
    // FIXME: memory leak!
    Instruction * instructionStream = NULL;
    usize slotCount = 0;

    compileNamespace(&instructionStream, &slotCount, ast);

    *instructionCount = arrlen(instructionStream);
    return instructionStream;
}