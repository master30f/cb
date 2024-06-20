#include <stdio.h>
#include <assert.h>
#include <stdint.h>

typedef uint8_t ElfWidth;
#define ELF_WIDTH_32             0x01
#define ELF_WIDTH_64             0x02
#define ELF_HUMAN_WIDTHS               \
    X(ELF_WIDTH_32             , "32") \
    X(ELF_WIDTH_64             , "64") \

typedef uint8_t ElfEndianness;
#define ELF_ENDIANNESS_LITTLE    0x01
#define ELF_ENDIANNESS_BIG       0x02
#define ELF_HUMAN_ENDIANNESSES                    \
    X(ELF_ENDIANNESS_LITTLE    , "little-endian") \
    X(ELF_ENDIANNESS_BIG       , "big-endian"   ) \

typedef uint8_t ElfABI;
#define ELF_ABI_SYSTEM_V         0x00
#define ELF_ABI_HP_UX            0x01
#define ELF_ABI_NETBSD           0x02
#define ELF_ABI_LINUX            0x03
#define ELF_ABI_GNU_HURD         0x04
#define ELF_ABI_SOLARIS          0x06
#define ELF_ABI_AIX              0x07
#define ELF_ABI_IRIX             0x08
#define ELF_ABI_FREEBSD          0x09
#define ELF_ABI_TRU64            0x0a
#define ELF_ABI_NOVELL_MODESTO   0x0b
#define ELF_ABI_OPENBSD          0x0c
#define ELF_ABI_OPENVMS          0x0d
#define ELF_ABI_NONSTOP_KERNEL   0x0e
#define ELF_ABI_AROS             0x0f
#define ELF_ABI_FENIXOS          0x10
#define ELF_ABI_NUXI_CLOUDABI    0x11
#define ELF_ABI_STRATUS_VOS      0x12
#define ELF_HUMAN_ABIS                             \
    X(ELF_ABI_SYSTEM_V         , "System V"      ) \
    X(ELF_ABI_HP_UX            , "HP-UX"         ) \
    X(ELF_ABI_NETBSD           , "NetBSD"        ) \
    X(ELF_ABI_LINUX            , "Linux"         ) \
    X(ELF_ABI_GNU_HURD         , "GNU Hurd"      ) \
    X(ELF_ABI_SOLARIS          , "Solaris"       ) \
    X(ELF_ABI_AIX              , "AIX"           ) \
    X(ELF_ABI_IRIX             , "IRIX"          ) \
    X(ELF_ABI_FREEBSD          , "FreeBSD"       ) \
    X(ELF_ABI_TRU64            , "Tru64"         ) \
    X(ELF_ABI_NOVELL_MODESTO   , "Novell Modesto") \
    X(ELF_ABI_OPENBSD          , "OpenBSD"       ) \
    X(ELF_ABI_OPENVMS          , "OpenVMS"       ) \
    X(ELF_ABI_NONSTOP_KERNEL   , "NonStop Kernel") \
    X(ELF_ABI_AROS             , "AROS"          ) \
    X(ELF_ABI_FENIXOS          , "FenixOS"       ) \
    X(ELF_ABI_NUXI_CLOUDABI    , "Nuxi CloudABI" ) \
    X(ELF_ABI_STRATUS_VOS      , "Stratus VOS"   ) \

typedef uint16_t ElfType;
#define ELF_TYPE_UNKNOWN         0x0000
#define ELF_TYPE_RELOCATABLE     0x0001
#define ELF_TYPE_EXECUTABLE      0x0002
#define ELF_TYPE_SHARED_OBJECT   0x0003
#define ELF_TYPE_CORE            0x0004
#define ELF_TYPE_OS_BEGIN        0xfe00
#define ELF_TYPE_OS_END          0xfeff
#define ELF_TYPE_PROCESSOR_BEGIN 0xff00
#define ELF_TYPE_PROCESSOR_END   0xffff
#define ELF_HUMAN_TYPES                           \
    X(ELF_TYPE_UNKNOWN         , "unknown"      ) \
    X(ELF_TYPE_RELOCATABLE     , "relocatable"  ) \
    X(ELF_TYPE_EXECUTABLE      , "executable"   ) \
    X(ELF_TYPE_SHARED_OBJECT   , "shared object") \
    X(ELF_TYPE_CORE            , "core"         ) \

typedef uint16_t ElfArch;
#define ELF_ARCH_X86_32          0x0003
#define ELF_ARCH_X86_64          0x003e
#define ELF_HUMAN_ARCHS                    \
    X(ELF_ARCH_X86_32          , "x86-32") \
    X(ELF_ARCH_X86_64          , "x86-64") \

typedef uint32_t ElfSHType;
#define ELF_SH_TYPE_UNUSED               0x00000000
#define ELF_SH_TYPE_DATA                 0x00000001
#define ELF_SH_TYPE_LINKER_SYMBOL_TABLE  0x00000002
#define ELF_SH_TYPE_STRING_TABLE         0x00000003
#define ELF_SH_TYPE_RELOCATIONS_ADDENDS  0x00000004
#define ELF_SH_TYPE_SYMBOL_HASH_TABLE    0x00000005
#define ELF_SH_TYPE_LINKING_INFO         0x00000006
#define ELF_SH_TYPE_NOTE                 0x00000007
#define ELF_SH_TYPE_BSS                  0x00000008
#define ELF_SH_TYPE_RELOCATIONS          0x00000009
#define ELF_SH_TYPE_SHLIB                0x0000000a
#define ELF_SH_TYPE_LOADER_SYMBOL_TABLE  0x0000000b
#define ELF_SH_TYPE_CONSTRUCTORS         0x0000000e
#define ELF_SH_TYPE_DESTRUCTORS          0x0000000f
#define ELF_SH_TYPE_PRECONSTRUCTORS      0x00000010
#define ELF_SH_TYPE_GROUP                0x00000011
#define ELF_SH_TYPE_SECTION_INDEX_TABLE  0x00000012
#define ELF_SH_TYPE_COUNT                0x00000013
#define ELF_SH_TYPE_OS_BEGIN             0x60000000
#define ELF_SH_TYPE_OS_END               0x6fffffff
#define ELF_SH_TYPE_PROCESSOR_BEGIN      0x70000000
#define ELF_SH_TYPE_PROCESSOR_END        0x7fffffff
#define ELF_HUMAN_SH_TYPES                                               \
    X(ELF_SH_TYPE_UNUSED               , "unused")                       \
    X(ELF_SH_TYPE_DATA                 , "data")                         \
    X(ELF_SH_TYPE_LINKER_SYMBOL_TABLE  , "dynamic linker table")         \
    X(ELF_SH_TYPE_STRING_TABLE         , "string table")                 \
    X(ELF_SH_TYPE_RELOCATIONS_ADDENDS  , "relocations with addends")     \
    X(ELF_SH_TYPE_SYMBOL_HASH_TABLE    , "symbol hash table")            \
    X(ELF_SH_TYPE_LINKING_INFO         , "dynamic linking info")         \
    X(ELF_SH_TYPE_NOTE                 , "notes")                        \
    X(ELF_SH_TYPE_BSS                  , "bss")                          \
    X(ELF_SH_TYPE_RELOCATIONS          , "relocations")                  \
    X(ELF_SH_TYPE_SHLIB                , "shlib")                        \
    X(ELF_SH_TYPE_LOADER_SYMBOL_TABLE  , "dynamic loader symbol table")  \
    X(ELF_SH_TYPE_CONSTRUCTORS         , "constructors")                 \
    X(ELF_SH_TYPE_DESTRUCTORS          , "destructors")                  \
    X(ELF_SH_TYPE_PRECONSTRUCTORS      , "pre-constructors")             \
    X(ELF_SH_TYPE_GROUP                , "section group")                \
    X(ELF_SH_TYPE_SECTION_INDEX_TABLE  , "extended section index table") \
    X(ELF_SH_TYPE_COUNT                , "defined type count")           \

typedef uint32_t ElfSHFlags32;
typedef uint64_t ElfSHFlags64;
#define ELF_SH_FLAG_WRITABLE            0x00000001
#define ELF_SH_FLAG_ALLOCATE            0x00000002
#define ELF_SH_FLAG_EXECUTABLE          0x00000004
#define ELF_SH_FLAG_MERGE               0x00000010
#define ELF_SH_FLAG_STRINGS             0x00000020
#define ELF_SH_FLAG_INFO_LINK           0x00000040
#define ELF_SH_FLAG_PRESERVE_ORDER      0x00000080
#define ELF_SH_FLAG_OS_NONCONFORMING    0x00000100
#define ELF_SH_FLAG_GROUP               0x00000200
#define ELF_SH_FLAG_TLS                 0x00000400
#define ELF_SH_FLAG_OS_MASK             0x0ff00000
#define ELF_SH_FLAG_PROCESSOR_MASK      0xf0000000

const uint8_t * string_ElfWidth(ElfWidth x)
{
    switch (x)
    {
#define X(name, human) case name: return human;
    ELF_HUMAN_WIDTHS
#undef X
    }

    return NULL;
}

const uint8_t * string_ElfEndianness(ElfEndianness x)
{
    switch (x)
    {
#define X(name, human) case name: return human;
    ELF_HUMAN_ENDIANNESSES
#undef X
    }

    return NULL;
}

const uint8_t * string_ElfABI(ElfABI x)
{
    switch (x)
    {
#define X(name, human) case name: return human;
    ELF_HUMAN_ABIS
#undef X
    }

    return NULL;
}

// TODO: support ELF_TYPE_OS_BEGIN..ELF_TYPE_OS_END
const uint8_t * string_ElfType(ElfType x)
{
    switch (x)
    {
#define X(name, human) case name: return human;
    ELF_HUMAN_TYPES
#undef X
    }

    return NULL;
}

const uint8_t * string_ElfArch(ElfArch x)
{
    switch (x)
    {
#define X(name, human) case name: return human;
    ELF_HUMAN_ARCHS
#undef X
    }

    return NULL;
}

const uint8_t * string_ElfSHType(ElfSHType x)
{
    switch (x)
    {
#define X(name, human) case name: return human;
    ELF_HUMAN_SH_TYPES
#undef X
    }

    return NULL;
}

typedef uint32_t Magic;

#define ELF_MAGIC 0x464c457f

typedef struct __attribute__((packed)) {
    ElfWidth      width;
    ElfEndianness endianness;
    uint8_t       version;
    ElfABI        abi;
    uint8_t       abiVersion;
    uint8_t       padding[7];
} ElfIdentifier;

// TODO: support big-endian
typedef struct __attribute__((packed)) {
    ElfType  type;
    ElfArch  arch;
    uint32_t version;
    uint32_t entry;
    uint32_t programHeaderTable;
    uint32_t sectionHeaderTable;
    uint32_t flags;
    uint16_t size;
    uint16_t programHeaderSize;
    uint16_t programHeaderCount;
    uint16_t sectionHeaderSize;
    uint16_t sectionHeaderCount;
    uint16_t nameSectionHeaderIndex;
} ELFHeader32;

typedef struct __attribute__((packed)) {
    ElfType  type;
    ElfArch  arch;
    uint32_t version;
    uint64_t entry;
    uint64_t programHeaderTable;
    uint64_t sectionHeaderTable;
    uint32_t flags;
    uint16_t size;
    uint16_t programHeaderSize;
    uint16_t programHeaderCount;
    uint16_t sectionHeaderSize;
    uint16_t sectionHeaderCount;
    uint16_t nameSectionHeaderIndex;
} ElfHeader64;

typedef struct __attribute__((packed)) {
    uint32_t     nameIndex;
    ElfSHType    type;
    ElfSHFlags64 flags;
    uint64_t     address;
    uint64_t     offset;
    uint64_t     size;
    uint32_t     link;
    uint32_t     info;
    uint64_t     alignment;
    uint64_t     entrySize;
} ElfSectionHeader64;

int main(void)
{
    FILE * file = fopen("./min.o", "rb");
    assert(file);

    Magic magic;
    assert(1 == fread(&magic, sizeof(magic), 1, file));

    // TODO: support PE
    assert(magic == ELF_MAGIC);

    ElfIdentifier identifier;
    assert(1 == fread(&identifier, sizeof(identifier), 1, file));

    // TODO: support ELF32
    assert(identifier.width == ELF_WIDTH_64);

    ElfHeader64 header;
    assert(1 == fread(&header, sizeof(header), 1, file));

    assert(header.sectionHeaderSize == sizeof(ElfSectionHeader64));

    printf("ELF%s %s %s %s\n", string_ElfWidth(identifier.width), string_ElfEndianness(identifier.endianness), string_ElfArch(header.arch), string_ElfType(header.type));
    
    fseek(file, header.sectionHeaderTable, SEEK_SET);

    /*fseek(file, header.sectionHeaderTable + header.nameSectionHeaderIndex*sizeof(ElfSectionHeader64), SEEK_SET);

    ElfSectionHeader64 nameSectionHeader;
    assert(1 == fread(&nameSectionHeader, sizeof(nameSectionHeader), 1, file));*/

    for (uint16_t i = 0; i < header.sectionHeaderCount; i++)
    {
        ElfSectionHeader64 sectionHeader;
        assert(1 == fread(&sectionHeader, sizeof(sectionHeader), 1, file));

        printf(" - %s (%d)\n", string_ElfSHType(sectionHeader.type), sectionHeader.size);
    }

    fclose(file);
    return 0;
}