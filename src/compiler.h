#pragma once

#include "number.h"
#include "parser.h"

typedef u8 InstructionType;
#define IT_NONE       0
#define IT_VALUE_32   1
#define IT_RET_SET_32 2
#define IT_RETURN     3

typedef struct {
    InstructionType type;

    union {
        // IT_VALUE_32, IT_RET_SET_32
        struct {
            usize dstSlot;

            union {
                // IT_VALUE_32
                struct {
                    u32 srcValue32;
                };
                // IT_RET_SET_32
                struct {
                    usize srcSlot;
                };
            };
        };
    };
} Instruction;

static usize compileExpression(const Node * node);
static void compileStatement(const Node * node);

Instruction * compile(const Node * ast, usize * instructionCount);