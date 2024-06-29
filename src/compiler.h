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

typedef struct {
    InstructionType type;

    union {
        // IT_VALUE_32, IT_RET_SET_32, IT_MOVE_32. IT_ADD_32
        struct {
            usize dstSlot;

            union {
                // IT_VALUE_32
                struct {
                    u32 srcValue32;
                };
                // IT_RET_SET_32, IT_MOVE_32, IT_ADD_32
                struct {
                    usize srcSlot;
                };
            };
        };
        // IT_BEGIN_SCOPE
        struct {
            usize slotCount;
        };
    };
} Instruction;

static usize compileExpression(Instruction ** is, usize * sc, const Node * node);
static void compileStatement(Instruction ** is, usize * sc, const Node * node);

Instruction * compile(const Node * ast, usize * instructionCount);