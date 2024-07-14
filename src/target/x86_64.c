#include <stdalign.h>

#include "number.h"
#include "stb_ds.h"
#include "elf.h"

#include "compiler.h"

typedef u8 Protocol;
#define PROTO_OPTIMAL 0
#define PROTO_CDECL   1
#define PROTO_MAIN    2

static void mov_vrsp_d8_i32(u8 ** bytes, i8 d, u32 i)
{
    u8 * imm = (u8 *) &i;
    arrpush(*bytes, 0xc7);
    arrpush(*bytes, 0x44);
    arrpush(*bytes, 0x24);
    arrpush(*bytes, d);
    arrpush(*bytes, imm[0]);
    arrpush(*bytes, imm[1]);
    arrpush(*bytes, imm[2]);
    arrpush(*bytes, imm[3]);
}

static void mov_edi_vrsp_d8(u8 ** bytes, i8 d)
{
    arrpush(*bytes, 0x8b);
    arrpush(*bytes, 0x7c);
    arrpush(*bytes, 0x24);
    arrpush(*bytes, d);
}

static void mov_eax_i32(u8 ** bytes, u32 i)
{
    u8 * imm = (u8 *) &i;
    arrpush(*bytes, 0xb8);
    arrpush(*bytes, imm[0]);
    arrpush(*bytes, imm[1]);
    arrpush(*bytes, imm[2]);
    arrpush(*bytes, imm[3]);
}

static void mov_eax_vrsp_d8(u8 ** bytes, i8 d)
{
    arrpush(*bytes, 0x8b);
    arrpush(*bytes, 0x44);
    arrpush(*bytes, 0x24);
    arrpush(*bytes, d);
}

static void mov_vrsp_d8_eax(u8 ** bytes, i8 d)
{
    arrpush(*bytes, 0x89);
    arrpush(*bytes, 0x44);
    arrpush(*bytes, 0x24);
    arrpush(*bytes, d);
}

static void add_vrsp_d8_eax(u8 ** bytes, i8 d)
{
    arrpush(*bytes, 0x01);
    arrpush(*bytes, 0x44);
    arrpush(*bytes, 0x24);
    arrpush(*bytes, d);
}

static void syscall(u8 ** bytes)
{
    arrpush(*bytes, 0x0f);
    arrpush(*bytes, 0x05);
}

static void push_rbp(u8 ** bytes)
{
    arrpush(*bytes, 0x55);
}

static void push_rbx(u8 ** bytes)
{
    arrpush(*bytes, 0x53);
}

static void push_r12(u8 ** bytes)
{
    arrpush(*bytes, 0x41);
    arrpush(*bytes, 0x54);
}

static void push_r13(u8 ** bytes)
{
    arrpush(*bytes, 0x41);
    arrpush(*bytes, 0x55);
}

static void push_r14(u8 ** bytes)
{
    arrpush(*bytes, 0x41);
    arrpush(*bytes, 0x56);
}

static void push_r15(u8 ** bytes)
{
    arrpush(*bytes, 0x41);
    arrpush(*bytes, 0x57);
}

static void ret(u8 ** bytes)
{
    arrpush(*bytes, 0xc3);
}

static void pop_r15(u8 ** bytes)
{
    arrpush(*bytes, 0x41);
    arrpush(*bytes, 0x5f);
}

static void pop_r14(u8 ** bytes)
{
    arrpush(*bytes, 0x41);
    arrpush(*bytes, 0x5e);
}

static void pop_r13(u8 ** bytes)
{
    arrpush(*bytes, 0x41);
    arrpush(*bytes, 0x5d);
}

static void pop_r12(u8 ** bytes)
{
    arrpush(*bytes, 0x41);
    arrpush(*bytes, 0x5c);
}

static void pop_rbx(u8 ** bytes)
{
    arrpush(*bytes, 0x5b);
}

static void pop_rbp(u8 ** bytes)
{
    arrpush(*bytes, 0x5d);
}

usize addString(char ** stringTable, const char * string)
{
    usize stringLen = strlen(string) + 1;
    usize offset = arrlenu(*stringTable);
    arrsetlen(*stringTable, offset + stringLen);
    memcpy(*stringTable + offset, string, stringLen);
    return offset;
}

usize addSymbol(Elf64Symbol ** symbolTable, Elf64Symbol symbol)
{
    usize offset = arrlenu(*symbolTable) * sizeof(Elf64Symbol);
    arrpush(*symbolTable, symbol);
    return offset;
}

Protocol resolveProtocol(u8 * string)
{
    if (strcmp((char *) string, "optimal") == 0)
        return PROTO_OPTIMAL;
    else if (strcmp((char *) string, "default") == 0 || strcmp((char *) string, "c") == 0 || strcmp((char *) string, "cdecl") == 0)
        return PROTO_CDECL;
    else if (strcmp((char *) string, "main") == 0)
        return PROTO_MAIN;
    else
        assert(0);
}

u8 * translate(const Instruction * instructions, usize instructionCount, Symbol * functionTable, usize functionCount, usize * byteCount)
{
    // FIXME: memory leak!
    u8 * program = NULL;

    i8 stackPointer = 0;

    usize slotCount = 0;
    usize allocatedSlots = 0;
    // TODO: this should be i64!
    i8 * slots = NULL;

    usize currentFunction = 0;
    u64 functionAddresses[functionCount];

    for (usize i = 0; i < instructionCount; i++)
    {
        Instruction instruction = instructions[i];

        switch (instruction.type)
        {
            case IT_BEGIN_SCOPE:
            {
                functionAddresses[currentFunction++] = arrlenu(program);

                slotCount = instruction.slotCount;
                allocatedSlots = 0;
                stackPointer = 0;
                // FIXME: memory leak!
                slots = malloc(slotCount * sizeof(slots[0]));
                assert(slots);
            } break;
            case IT_END_SCOPE: break;
            case IT_VALUE_32:
            {
                if (instruction.dstSlot >= allocatedSlots)
                {
                    assert(instruction.dstSlot == allocatedSlots);

                    allocatedSlots++;
                    stackPointer -= 4;
                    slots[instruction.dstSlot] = stackPointer;
                }

                mov_vrsp_d8_i32(&program, stackPointer, instruction.srcValue32);
            } break;
            case IT_FUNC_BEGIN:
            {
                switch (resolveProtocol(instruction.jmpProtocol))
                {
                    case PROTO_MAIN:
                        // FIXME: follow abi
                        break;
                    case PROTO_CDECL:
                    {
                        // FIXME: exactly follow https://gitlab.com/x86-psABIs/x86-64-ABI (§3.2)
                        push_rbp(&program);
                        push_rbx(&program);
                        push_r12(&program);
                        push_r13(&program);
                        push_r14(&program);
                        push_r15(&program);
                    } break;
                    default: assert(0 && "TODO:");
                }
            } break;
            case IT_RET_MOVE_32:
            {
                assert(instruction.srcSlot < allocatedSlots);

                switch (resolveProtocol(instruction.movProtocol))
                {
                    case PROTO_MAIN:
                        mov_edi_vrsp_d8(&program, slots[instruction.srcSlot]); break;
                    case PROTO_CDECL:
                        mov_eax_vrsp_d8(&program, slots[instruction.srcSlot]); break;
                    default:
                        assert(0 && "TODO:");
                }
            } break;
            case IT_RETURN:
            {
                switch (resolveProtocol(instruction.jmpProtocol))
                {
                    case PROTO_MAIN:
                    {
                        // FIXME: follow abi
                        mov_eax_i32(&program, 0x3c);
                        syscall(&program);
                    } break;
                    case PROTO_CDECL:
                    {
                        // FIXME: exactly follow https://gitlab.com/x86-psABIs/x86-64-ABI (§3.2)
                        pop_r15(&program);
                        pop_r14(&program);
                        pop_r13(&program);
                        pop_r12(&program);
                        pop_rbx(&program);
                        pop_rbp(&program);
                        ret(&program);
                    } break;
                    default: assert(0 && "TODO:");
                }
            } break;
            case IT_MOVE_32:
            {
                assert(instruction.srcSlot < allocatedSlots);

                if (instruction.dstSlot >= allocatedSlots)
                {
                    assert(instruction.dstSlot == allocatedSlots);

                    allocatedSlots++;
                    stackPointer -= 4;
                    slots[instruction.dstSlot] = stackPointer;
                }

                mov_eax_vrsp_d8(&program, slots[instruction.srcSlot]);
                mov_vrsp_d8_eax(&program, slots[instruction.dstSlot]);
            } break;
            case IT_ADD_32:
            {
                assert(instruction.srcSlot < allocatedSlots);
                assert(instruction.dstSlot < allocatedSlots);

                mov_eax_vrsp_d8(&program, slots[instruction.srcSlot]);
                add_vrsp_d8_eax(&program, slots[instruction.dstSlot]);
            } break;
            default: assert(0 && "TODO:");
        }
    }

    char * stringTable = NULL;

    usize nullString = addString(&stringTable, "");
    usize strtabName = addString(&stringTable, ".strtab");
    usize symtabName = addString(&stringTable, ".symtab");
    usize textName   = addString(&stringTable, ".text");

    usize functionNames[functionCount];
    for (usize i = 0; i < functionCount; i++)
    {
        functionNames[i] = addString(&stringTable, (char *) functionTable[i].name);
    }

    Elf64Symbol * symbolTable = NULL;

    addSymbol(&symbolTable, (Elf64Symbol)
    {
        .name               = nullString,
        .info               = ELF_SYMBOL_INFO(ELF_SYMBOL_BINDING_LOCAL, ELF_SYMBOL_TYPE_NONE),
        .other              = ELF_SYMBOL_OTHER(ELF_SYMBOL_VISIBILITY_DEFAULT),
        .sectionHeaderIndex = 1,
        .value              = 0,
        .size               = 0
    });

    for (usize i = 0; i < functionCount; i++)
    {
        addSymbol(&symbolTable, (Elf64Symbol)
        {
            .name               = functionNames[i],
            .info               = ELF_SYMBOL_INFO(ELF_SYMBOL_BINDING_GLOBAL, ELF_SYMBOL_TYPE_FUNCTION),
            .other              = ELF_SYMBOL_OTHER(ELF_SYMBOL_VISIBILITY_DEFAULT),
            .sectionHeaderIndex = 3,
            .value              = functionAddresses[i],
            // TODO:
            .size               = 0
        });
    }

    usize identifierOffset   = 0;
    usize headerOffset       = identifierOffset + sizeof(ElfIdentifier);
    usize nullHeaderOffset   = headerOffset + sizeof(Elf64Header);
    usize strtabHeaderOffset = nullHeaderOffset + sizeof(Elf64SectionHeader);
    usize symtabHeaderOffset = strtabHeaderOffset + sizeof(Elf64SectionHeader);
    usize textHeaderOffset   = symtabHeaderOffset + sizeof(Elf64SectionHeader);
    usize strtabOffset       = textHeaderOffset + sizeof(Elf64SectionHeader);
    usize symtabOffset       = strtabOffset + arrlenu(stringTable) * sizeof(char);
    usize textOffset         = symtabOffset + arrlenu(symbolTable) * sizeof(Elf64Symbol);
    usize fileSize           = textOffset + arrlen(program);

    // FIXME: memory leak!
    u8 * bytes = malloc(fileSize);
    assert(bytes);

    ElfIdentifier * ident   = (ElfIdentifier *)(bytes + identifierOffset);
    ident->magic            = ELF_MAGIC;
    ident->class            = ELF_CLASS_64;
    ident->endianness       = ELF_ENDIANNESS_LITTLE;
    ident->version          = ELF_VERSION_1;
    ident->extension        = ELF_EXTENSION_NONE;
    ident->extensionVersion = 0;

    Elf64Header * header              = (Elf64Header *)(bytes + headerOffset);
    header->type                      = ELF_TYPE_RELOCATABLE;
    header->arch                      = ELF_ARCH_X86_64;
    header->version                   = ELF_VERSION_1;
    header->entry                     = 0;
    header->programHeadersOffset      = 0;
    header->sectionHeadersOffset      = nullHeaderOffset;
    header->flags                     = 0;
    header->headerSize                = sizeof(Elf64Header);
    header->programHeaderSize         = 0;
    header->programHeaderCount        = 0;
    header->sectionHeaderSize         = sizeof(Elf64SectionHeader);
    header->sectionHeaderCount        = 4;
    header->sectionNameTableIndex     = 1;

    Elf64SectionHeader * nullHeader   = (Elf64SectionHeader *)(bytes + nullHeaderOffset);
    nullHeader->name                  = nullString;
    nullHeader->type                  = ELF_SECTION_TYPE_NULL;
    nullHeader->flags                 = 0;
    nullHeader->address               = 0;
    nullHeader->offset                = 0;
    nullHeader->size                  = 0;
    nullHeader->link                  = 0;
    nullHeader->info                  = 0;
    nullHeader->addressAlign          = 0;
    nullHeader->entrySize             = 0;

    Elf64SectionHeader * strtabHeader = (Elf64SectionHeader *)(bytes + strtabHeaderOffset);
    strtabHeader->name                = strtabName;
    strtabHeader->type                = ELF_SECTION_TYPE_STRING_TABLE;
    strtabHeader->flags               = 0;
    strtabHeader->address             = 0;
    strtabHeader->offset              = strtabOffset;
    strtabHeader->size                = arrlenu(stringTable);
    strtabHeader->link                = 0;
    strtabHeader->info                = 0;
    strtabHeader->addressAlign        = 0;
    strtabHeader->entrySize           = 0;

    Elf64SectionHeader * symtabHeader = (Elf64SectionHeader *)(bytes + symtabHeaderOffset);
    symtabHeader->name                = symtabName;
    symtabHeader->type                = ELF_SECTION_TYPE_SYMBOL_TABLE;
    symtabHeader->flags               = 0;
    symtabHeader->address             = 0;
    symtabHeader->offset              = symtabOffset;
    symtabHeader->size                = arrlenu(symbolTable) * sizeof(Elf64Symbol);
    symtabHeader->link                = 1;
    symtabHeader->info                = 1;
    symtabHeader->addressAlign        = alignof(Elf64Symbol);
    symtabHeader->entrySize           = sizeof(Elf64Symbol);

    Elf64SectionHeader * textHeader   = (Elf64SectionHeader *)(bytes + textHeaderOffset);
    textHeader->name                  = textName;
    textHeader->type                  = ELF_SECTION_TYPE_DATA;
    textHeader->flags                 = ELF_SECTION_FLAG_ALLOC | ELF_SECTION_FLAG_EXEC;
    textHeader->address               = 0;
    textHeader->offset                = textOffset;
    textHeader->size                  = arrlenu(program);
    textHeader->link                  = 0;
    textHeader->info                  = 0;
    textHeader->addressAlign          = 0;
    textHeader->entrySize             = 0;

    memcpy(bytes + strtabOffset, stringTable, arrlenu(stringTable));

    memcpy(bytes + symtabOffset, symbolTable, arrlenu(symbolTable) * sizeof(Elf64Symbol));

    memcpy(bytes + textOffset, program, arrlen(program));

    *byteCount = fileSize;
    return bytes;
}