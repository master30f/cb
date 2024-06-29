#ifndef ELF_H_
#define ELF_H_

#include <stdint.h>

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
#define ELF_SECTION_FLAG_WRITE          0x001
#define ELF_SECTION_FLAG_ALLOC          0x002
#define ELF_SECTION_FLAG_EXEC           0x004
#define ELF_SECTION_FLAG_MERGE          0x010
#define ELF_SECTION_FLAG_STRINGS        0x020
#define ELF_SECTION_FLAG_INFO_LINK      0x040
#define ELF_SECTION_FLAG_LINK_ORDER     0x080
#define ELF_SECTION_FLAG_OS_SPECIAL     0x100
#define ELF_SECTION_FLAG_GROUP          0x200
#define ELF_SECTION_FLAG_THREAD_LOCAL   0x400
#define ELF_SECTION_FLAG_COMPRESSED     0x800

#define ELF_SYMBOL_BINDING_LOCAL        0
#define ELF_SYMBOL_BINDING_GLOBAL       1
#define ELF_SYMBOL_BINDING_WEAK         2

#define ELF_SYMBOL_TYPE_NONE            0
#define ELF_SYMBOL_TYPE_OBJECT          1
#define ELF_SYMBOL_TYPE_FUNCTION        2
#define ELF_SYMBOL_TYPE_SECTION         3
#define ELF_SYMBOL_TYPE_FILE            4
#define ELF_SYMBOL_TYPE_COMMON          5
#define ELF_SYMBOL_TYPE_THREAD_LOCAL    6

#define ELF_SYMBOL_VISIBILITY_DEFAULT   0
#define ELF_SYMBOL_VISIBILITY_INTERNAL  1
#define ELF_SYMBOL_VISIBILITY_HIDDEN    2
#define ELF_SYMBOL_VISIBILITY_PROTECTED 3


#define ELF_SYMBOL_BINDING(info)       ((info) >> 4)
#define ELF_SYMBOL_TYPE(info)          ((info) & 0xf)
#define ELF_SYMBOL_INFO(binding, type) (((binding) << 4) | ((type) & 0xf))

#define ELF_SYMBOL_VISIBILITY(other) ((other) & 0x3)
#define ELF_SYMBOL_OTHER(visibility) ((visibility) & 0x3)

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

typedef struct {
    uint32_t name;
    uint8_t  info;
    uint8_t  other;
    uint16_t sectionHeaderIndex;
    uint64_t value;
    uint64_t size;
} Elf64Symbol;

#endif