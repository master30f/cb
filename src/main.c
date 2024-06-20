#include "stdio.h"

#include "lexer.c"
#include "parser.c"

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

    Node * node = parse(tokenCount, tokens);

    printNode(node, 0);

    printf("finished succesfully\n");

    return 0;
}