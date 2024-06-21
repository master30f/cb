#include "stdio.h"

#include "lexer.c"
#include "parser.c"
#include "compiler.c"

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
    if (readSize != (size_t) size) printf("file '%s' partially read (%lld B out of %ld B)", fileName, readSize, size);

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

    usize instructionCount;
    Instruction * instructions = compile(ast, &instructionCount);

    for (usize i = 0; i < instructionCount; i++)
    {
        Instruction inst = instructions[i];

        switch (inst.type)
        {
            case IT_VALUE_32:   printf("value32 &%lld, %d\n", inst.dstSlot, inst.srcValue32); break;
            case IT_RET_SET_32: printf("retSet32 &%lld, &%lld\n", inst.dstSlot, inst.srcSlot); break;
            case IT_RETURN:     printf("return\n"); break;
        }
    }

    printf("finished succesfully\n");

    return 0;
}