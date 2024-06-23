#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

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

#define ELF_MAGIC 0x464c457f

typedef uint8_t ElfClass;
#define ELF_CLASS_NONE 0
#define ELF_CLASS_32   1
#define ELF_CLASS_64   2

typedef uint8_t ElfEndianness;
#define ELF_ENDIANNESS_NONE   0
#define ELF_ENDIANNESS_LITTLE 1
#define ELF_ENDIANNESS_BIG    2

typedef uint8_t ElfExtension;
#define ELF_EXTENSION_NONE 0
#define ELF_EXTENSION_GNU  3

typedef uint16_t ElfType;
#define ELF_TYPE_NONE        0
#define ELF_TYPE_RELOCATABLE 1
#define ELF_TYPE_EXECUTABLE  2
#define ELF_TYPE_SHARED      3
#define ELF_TYPE_CORE        4

typedef uint16_t ElfArch;
#define ELF_ARCH_NONE    0
#define ELF_ARCH_X86_32  3
#define ELF_ARCH_AARCH32 40
#define ELF_ARCH_X86_64  62
#define ELF_ARCH_AARCH64 183

typedef uint8_t  ElfVersion8;
typedef uint32_t ElfVersion32;
#define ELF_VERSION_NONE 0
#define ELF_VERSION_1    1

typedef uint32_t ElfSectionType;
#define ELF_SECTION_TYPE_NULL            0
#define ELF_SECTION_TYPE_DATA            1
#define ELF_SECTION_TYPE_SYMBOL_TABLE    2
#define ELF_SECTION_TYPE_STRING_TABLE    3
#define ELF_SECTION_TYPE_RELOC_ADDENDS   4
#define ELF_SECTION_TYPE_HASH_TABLE      5
#define ELF_SECTION_TYPE_DYNAMIC         6
#define ELF_SECTION_TYPE_NOTE            7
#define ELF_SECTION_TYPE_UNINTIALIZED    8
#define ELF_SECTION_TYPE_RELOCATION      9
#define ELF_SECTION_TYPE_SHLIB           10
#define ELF_SECTION_TYPE_DYNAMIC_SYMBOLS 11
#define ELF_SECTION_TYPE_INITIALIZERS    14
#define ELF_SECTION_TYPE_FINALIZERS      15
#define ELF_SECTION_TYPE_PREINITIALIZERS 16
#define ELF_SECTION_TYPE_GROUP           17
#define ELF_SECTION_TYPE_EXT_INDICES     18

typedef uint64_t ElfSectionFlags;
#define ELF_SECTION_FLAG_WRITE        0x001
#define ELF_SECTION_FLAG_ALLOC        0x002
#define ELF_SECTION_FLAG_EXEC         0x004
#define ELF_SECTION_FLAG_MERGE        0x010
#define ELF_SECTION_FLAG_STRINGS      0x020
#define ELF_SECTION_FLAG_INFO_LINK    0x040
#define ELF_SECTION_FLAG_LINK_ORDER   0x080
#define ELF_SECTION_FLAG_OS_SPECIAL   0x100
#define ELF_SECTION_FLAG_GROUP        0x200
#define ELF_SECTION_FLAG_THREAD_LOCAL 0x400
#define ELF_SECTION_FLAG_COMPRESSED   0x800

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

typedef struct {
    uint32_t      magic;
    ElfClass      class;
    ElfEndianness endianness;
    ElfVersion8   version;
    ElfExtension  extension;
    uint8_t       extensionVersion;
    uint8_t       padding[7];
} ElfIdentifier;

typedef struct {
    ElfType      type;
    ElfArch      arch;
    ElfVersion32 version;
    uint64_t     entry;
    uint64_t     programHeadersOffset;
    uint64_t     sectionHeadersOffset;
    uint32_t     flags;
    uint16_t     headerSize;
    uint16_t     programHeaderSize;
    uint16_t     programHeaderCount;
    uint16_t     sectionHeaderSize;
    uint16_t     sectionHeaderCount;
    uint16_t     sectionNameTableIndex;
} Elf64Header;

typedef struct {
    uint32_t        name;
    ElfSectionType  type;
    ElfSectionFlags flags;
    uint64_t        address;
    uint64_t        offset;
    uint64_t        size;
    uint32_t        link;
    uint32_t        info;
    uint64_t        addressAlign;
    uint64_t        entrySize;
} Elf64SectionHeader;

int main(void)
{
    size_t fileSize;
    uint8_t * file = readFile("./min.o", &fileSize);

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

    for (uint16_t i = 0; i < header->sectionHeaderCount; i++)
    {
        Elf64SectionHeader * sectionHeader = &sectionHeaders[i];

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

        printf(" %s \"%s\"\n", string_ElfSectionType(sectionHeader->type), &sectionNameTable[sectionHeader->name]);
    }

    printf("finished succesfully\n");

    return 0;
}