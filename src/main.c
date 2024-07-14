#include "stdio.h"

#include "lexer.c"
#include "parser.c"
#include "compiler.c"
#include "target/x86_64.c"

uint8_t * readFile(const char * fileName, size_t * dataSize)
{
    FILE * file = fopen(fileName, "rb");
    assert(file);

    assert(fseek(file, 0, SEEK_END) == 0);
    long size = ftell(file);
    assert(size != -1 && size != 0);
    assert(fseek(file, 0, SEEK_SET) == 0);

    uint8_t * data = malloc(size * sizeof(uint8_t));
    assert(data);

    size_t readSize = fread(data, sizeof(uint8_t), size, file);

    *dataSize = readSize;
    // TODO: handle partial reads
    if (readSize != (size_t) size) printf("file '%s' partially read (%ld B out of %ld B)", fileName, readSize, size);

    fclose(file);

    return data;
}

int main(void)
{
    usize programLen;
    u8 *  program = readFile("./test.cb", &programLen);

    usize   tokenCount;
    Token * tokens = lex(programLen, program, &tokenCount);

    Node * ast = parse(tokenCount, tokens);

    Symbol * functionTable;
    usize functionCount;

    usize instructionCount;
    Instruction * instructions = compile(ast, &functionTable, &functionCount, &instructionCount);


    /*for (usize i = 0; i < instructionCount; i++)
    {
        Instruction inst = instructions[i];

        switch (inst.type)
        {
            case IT_VALUE_32:    printf("value32 &%ld, %d\n", inst.dstSlot, inst.srcValue32); break;
            case IT_RET_MOVE_32: printf("retMove32 &%ld, &%ld\n", inst.dstSlot, inst.srcSlot); break;
            case IT_RETURN:      printf("return\n"); break;
            case IT_BEGIN_SCOPE: printf("beginScope %ld\n", inst.slotCount); break;
            case IT_END_SCOPE:   printf("endScope\n"); break;
            case IT_MOVE_32:     printf("move32 &%ld, &%ld\n", inst.dstSlot, inst.srcSlot); break;
            case IT_ADD_32:      printf("add32 &%ld, &%ld\n", inst.dstSlot, inst.srcSlot); break;
            default:             printf("unknown\n"); break;
        }
    }
    printf("\n");*/

    usize byteCount;
    u8 * bytes = translate(instructions, instructionCount, functionTable, functionCount, &byteCount);

    FILE * file = fopen("./test.o", "wb");
    assert(file);

    assert(byteCount == fwrite(bytes, 1, byteCount, file));

    fclose(file);

    printf("finished succesfully\n");

    return 0;
}