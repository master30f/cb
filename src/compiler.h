#pragma once

#include "number.h"
#include "parser.h"

typedef u8 InstructionType;
#define IT_NONE        0
#define IT_VALUE_32    1
#define IT_RET_MOVE_32 2
#define IT_RETURN      3
#define IT_BEGIN_SCOPE 4
#define IT_END_SCOPE   5
#define IT_MOVE_32     6
#define IT_ADD_32      7
#define IT_FUNC_BEGIN  8

typedef struct {
    InstructionType type;

    union {
        // IT_VALUE_32, IT_RET_SET_32, IT_MOVE_32. IT_ADD_32
        struct {
            usize dstSlot;

            union {
                // IT_RET_SET_32, IT_MOVE_32, IT_ADD_32
                struct {
                    usize srcSlot;

                    union {
                        // IT_RET_SET_32
                        struct {
                            u8 * movProtocol;
                        };
                    };
                };
                // IT_VALUE_32
                struct {
                    u32 srcValue32;
                };
            };
        };
        // IT_BEGIN_SCOPE
        struct {
            usize slotCount;
        };
        // IT_FUNC_BEGIN, IT_RETURN
        struct {
            u8 * jmpProtocol;
        };
    };
} Instruction;

typedef struct {
    u8 *  name;
    usize value;
} Symbol;

typedef struct {
    Instruction ** is;
    usize *        sc;
    Symbol **      st;
    Symbol **      ft;
    u8 *           pt;
} Context;

static usize compileExpression(Context c, const Node * node);
static void compileStatement(Context c, const Node * node);

Instruction * compile(const Node * ast, Symbol ** functionTable, usize * functionCount, usize * instructionCount);