#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#include "src/elf.h"

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

char * string_ElfClass(ElfClass class)
{
    switch (class)
    {
        case ELF_CLASS_NONE: return "none";
        case ELF_CLASS_32:   return "32";
        case ELF_CLASS_64:   return "64";
        default:             return "<unknown>";
    }
}

char * string_ElfEndianness(ElfEndianness endianness)
{
    switch (endianness)
    {
        case ELF_ENDIANNESS_NONE:   return "none";
        case ELF_ENDIANNESS_LITTLE: return "little";
        case ELF_ENDIANNESS_BIG:    return "big";
        default:                    return "<unknown>";
    }
}

char * string_ElfExtension(ElfExtension extension)
{
    switch (extension)
    {
        case ELF_EXTENSION_NONE: return "";
        case ELF_EXTENSION_GNU:  return "GNU";
        default:                 return "<unknown>";
    }
}

char * string_ElfType(ElfType type)
{
    switch (type)
    {
        case ELF_TYPE_NONE:        return "none";
        case ELF_TYPE_RELOCATABLE: return "relocatable";
        case ELF_TYPE_EXECUTABLE:  return "executable";
        case ELF_TYPE_SHARED:      return "shared";
        case ELF_TYPE_CORE:        return "core";
        default:                   return "<unknown>";
    }
}

char * string_ElfSectionType(ElfSectionType sectionType)
{
    switch (sectionType)
    {
        case ELF_SECTION_TYPE_NULL:            return "null";
        case ELF_SECTION_TYPE_DATA:            return "data";
        case ELF_SECTION_TYPE_SYMBOL_TABLE:    return "symbol table";
        case ELF_SECTION_TYPE_STRING_TABLE:    return "string table";
        case ELF_SECTION_TYPE_RELOC_ADDENDS:   return "relocations with addends";
        case ELF_SECTION_TYPE_HASH_TABLE:      return "hash table";
        case ELF_SECTION_TYPE_DYNAMIC:         return "dynamic";
        case ELF_SECTION_TYPE_NOTE:            return "note";
        case ELF_SECTION_TYPE_UNINTIALIZED:    return "uninitialized data";
        case ELF_SECTION_TYPE_RELOCATION:      return "relocations";
        case ELF_SECTION_TYPE_SHLIB:           return "shlib";
        case ELF_SECTION_TYPE_DYNAMIC_SYMBOLS: return "dynamic symbol table";
        case ELF_SECTION_TYPE_INITIALIZERS:    return "initializers";
        case ELF_SECTION_TYPE_FINALIZERS:      return "finalizers";
        case ELF_SECTION_TYPE_PREINITIALIZERS: return "preinitializers";
        case ELF_SECTION_TYPE_GROUP:           return "group";
        case ELF_SECTION_TYPE_EXT_INDICES:     return "extended indices";
        default:                               return "<unknown>";
    }
}

int main(void)
{
    size_t fileSize;
    uint8_t * file = readFile("./test.o", &fileSize);

    assert(fileSize > sizeof(ElfIdentifier));
    ElfIdentifier * identifier = (ElfIdentifier *) file;

    assert(identifier->magic == ELF_MAGIC);
    assert(identifier->class == ELF_CLASS_64);

    assert(fileSize >= sizeof(ElfIdentifier) + sizeof(Elf64Header));
    Elf64Header * header = (Elf64Header *)(file + sizeof(ElfIdentifier));

    printf("%s-endian ELF%s %s object\n", string_ElfEndianness(identifier->endianness), string_ElfClass(identifier->class), string_ElfType(header->type));
    printf("%d sections\n", header->sectionHeaderCount);

    Elf64SectionHeader * sectionHeaders = (Elf64SectionHeader *)(file + header->sectionHeadersOffset);

    char * sectionNameTable = file + sectionHeaders[header->sectionNameTableIndex].offset;

    uint64_t symbolCount;
    Elf64Symbol * symbols;

    for (uint16_t i = 0; i < header->sectionHeaderCount; i++)
    {
        Elf64SectionHeader * sectionHeader = &sectionHeaders[i];

        if (sectionHeader->type == ELF_SECTION_TYPE_SYMBOL_TABLE)
        {
            symbolCount = sectionHeader->size / sectionHeader->entrySize;
            symbols = file + sectionHeader->offset;
        }

        printf(" - ");

        if (sectionHeader->flags & ELF_SECTION_FLAG_WRITE)        printf("w");
        else printf("-");
        if (sectionHeader->flags & ELF_SECTION_FLAG_ALLOC)        printf("a");
        else printf("-");
        if (sectionHeader->flags & ELF_SECTION_FLAG_EXEC)         printf("x");
        else printf("-");
        if (sectionHeader->flags & ELF_SECTION_FLAG_MERGE)        printf("m");
        else printf("-");
        if (sectionHeader->flags & ELF_SECTION_FLAG_STRINGS)      printf("s");
        else printf("-");
        if (sectionHeader->flags & ELF_SECTION_FLAG_INFO_LINK)    printf("l");
        else printf("-");
        if (sectionHeader->flags & ELF_SECTION_FLAG_LINK_ORDER)   printf("o");
        else printf("-");
        if (sectionHeader->flags & ELF_SECTION_FLAG_OS_SPECIAL)   printf("s");
        else printf("-");
        if (sectionHeader->flags & ELF_SECTION_FLAG_GROUP)        printf("g");
        else printf("-");
        if (sectionHeader->flags & ELF_SECTION_FLAG_THREAD_LOCAL) printf("t");
        else printf("-");
        if (sectionHeader->flags & ELF_SECTION_FLAG_COMPRESSED)   printf("c");
        else printf("-");

        printf(" %s \"%s\" (%d) %d\n", string_ElfSectionType(sectionHeader->type), &sectionNameTable[sectionHeader->name], sectionHeader->size, sectionHeader->offset);
    }

    printf("%d symbols\n", symbolCount);

    for (uint64_t i = 0; i < symbolCount; i++)
    {
        Elf64Symbol symbol = symbols[i];
        printf(" - \"%s\"\n", file + sectionHeaders[symbol.sectionHeaderIndex].offset + symbol.name);
    }

    size_t programSize = sectionHeaders[4].size;
    uint8_t * program = file + sectionHeaders[4].offset;

    printf("finished succesfully\n");

    return 0;
}