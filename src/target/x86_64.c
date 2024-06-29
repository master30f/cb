#include <stdalign.h>

#include "number.h"
#include "stb_ds.h"
#include "elf.h"

#include "compiler.h"

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

u8 * translate(const Instruction * instructions, usize instructionCount, usize * byteCount)
{
    // FIXME: memory leak!
    u8 * program = NULL;

    i8 stackPointer = 0;

    usize slotCount = 0;
    usize allocatedSlots = 0;
    // TODO: this should be i64!
    i8 * slots = NULL;

    for (usize i = 0; i < instructionCount; i++)
    {
        Instruction instruction = instructions[i];

        switch (instruction.type)
        {
            case IT_BEGIN_SCOPE:
            {
                slotCount = instruction.slotCount;
                allocatedSlots = 0;
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
            case IT_RET_MOVE_32:
            {
                assert(instruction.dstSlot < allocatedSlots);

                mov_edi_vrsp_d8(&program, slots[instruction.srcSlot]);
            } break;
            case IT_RETURN:
            {
                mov_eax_i32(&program, 0x3c);
                syscall(&program);
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

    char strtab[] = "\0.strtab\0.symtab\0.text\0_start";

    usize identifierOffset   = 0;
    usize headerOffset       = identifierOffset + sizeof(ElfIdentifier);
    usize nullHeaderOffset   = headerOffset + sizeof(Elf64Header);
    usize strtabHeaderOffset = nullHeaderOffset + sizeof(Elf64SectionHeader);
    usize symtabHeaderOffset = strtabHeaderOffset + sizeof(Elf64SectionHeader);
    usize textHeaderOffset   = symtabHeaderOffset + sizeof(Elf64SectionHeader);
    usize strtabOffset       = textHeaderOffset + sizeof(Elf64SectionHeader);
    usize nullSymOffset      = strtabOffset + sizeof(strtab);
    usize startSymOffset     = nullSymOffset + sizeof(Elf64Symbol);
    usize textOffset         = startSymOffset + sizeof(Elf64Symbol);
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

    Elf64Header * header          = (Elf64Header *)(bytes + headerOffset);
    header->type                  = ELF_TYPE_RELOCATABLE;
    header->arch                  = ELF_ARCH_X86_64;
    header->version               = ELF_VERSION_1;
    header->entry                 = textOffset;
    header->programHeadersOffset  = 0;
    header->sectionHeadersOffset  = nullHeaderOffset;
    header->flags                 = 0;
    header->headerSize            = sizeof(Elf64Header);
    header->programHeaderSize     = 0;
    header->programHeaderCount    = 0;
    header->sectionHeaderSize     = sizeof(Elf64SectionHeader);
    header->sectionHeaderCount    = 4;
    header->sectionNameTableIndex = 1;

    Elf64SectionHeader * nullHeader   = (Elf64SectionHeader *)(bytes + nullHeaderOffset);
    nullHeader->name                  = 0;
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
    strtabHeader->name                = 1;
    strtabHeader->type                = ELF_SECTION_TYPE_STRING_TABLE;
    strtabHeader->flags               = 0;
    strtabHeader->address             = 0;
    strtabHeader->offset              = strtabOffset;
    strtabHeader->size                = sizeof(strtab);
    strtabHeader->link                = 0;
    strtabHeader->info                = 0;
    strtabHeader->addressAlign        = 0;
    strtabHeader->entrySize           = 0;

    Elf64SectionHeader * symtabHeader = (Elf64SectionHeader *)(bytes + symtabHeaderOffset);
    symtabHeader->name                = 9;
    symtabHeader->type                = ELF_SECTION_TYPE_SYMBOL_TABLE;
    symtabHeader->flags               = 0;
    symtabHeader->address             = 0;
    symtabHeader->offset              = nullSymOffset;
    symtabHeader->size                = 2 * sizeof(Elf64Symbol);
    symtabHeader->link                = 1;
    symtabHeader->info                = 1;
    symtabHeader->addressAlign        = alignof(Elf64Symbol);
    symtabHeader->entrySize           = sizeof(Elf64Symbol);

    Elf64SectionHeader * textHeader   = (Elf64SectionHeader *)(bytes + textHeaderOffset);
    textHeader->name                  = 17;
    textHeader->type                  = ELF_SECTION_TYPE_DATA;
    textHeader->flags                 = ELF_SECTION_FLAG_ALLOC | ELF_SECTION_FLAG_EXEC;
    textHeader->address               = 0;
    textHeader->offset                = textOffset;
    textHeader->size                  = arrlen(program);
    textHeader->link                  = 0;
    textHeader->info                  = 0;
    textHeader->addressAlign          = 0;
    textHeader->entrySize             = 0;

    memcpy(bytes + strtabOffset, strtab, sizeof(strtab));

    Elf64Symbol * nullSym        = (Elf64Symbol *)(bytes + nullSymOffset);
    nullSym->name                = 0;
    nullSym->info                = ELF_SYMBOL_INFO(ELF_SYMBOL_BINDING_LOCAL, ELF_SYMBOL_TYPE_NONE);
    nullSym->other               = ELF_SYMBOL_OTHER(ELF_SYMBOL_VISIBILITY_DEFAULT);
    nullSym->sectionHeaderIndex  = 1;
    nullSym->value               = 0;
    nullSym->size                = 0;

    Elf64Symbol * startSym       = (Elf64Symbol *)(bytes + startSymOffset);
    startSym->name               = 23;
    startSym->info               = ELF_SYMBOL_INFO(ELF_SYMBOL_BINDING_GLOBAL, ELF_SYMBOL_TYPE_FUNCTION);
    startSym->other              = ELF_SYMBOL_OTHER(ELF_SYMBOL_VISIBILITY_DEFAULT);
    startSym->sectionHeaderIndex = 3;
    startSym->value              = 0;
    startSym->size               = sizeof(program);

    memcpy(bytes + textOffset, program, arrlen(program));

    *byteCount = fileSize;
    return bytes;
}