#include "ld.h"

struct ExeImage {
    int vaddr_offset, header_size;
    Vector *objs;  // vector<ObjectData *>
};

typedef struct ObjectData ObjectData;
struct ObjectData {
    char *data;
    int data_size, entire_size;

    char *shdr, *symtab, *strtab, *rela_text;
    int nshdr, nsymtab, nrela_text;
};

int read_byte(char *data) { return data[0] & 0xff; }

int read_word(char *data)
{
    return read_byte(data) | (read_byte(data + 1) << 8);
}

int read_dword(char *data)
{
    return read_word(data) | (read_word(data + 2) << 16);
}

// assume that obj->shdr, obj->nshdr and obj->data are already filled.
int get_section_offset(ObjectData *obj, char *name)
{
    char *section_strtab =
        obj->data +
        read_dword(obj->shdr + 0x40 * read_word(obj->data + 62) + 24);

    for (int i = 1; i < obj->nshdr; i++) {
        char *entry = obj->shdr + 0x40 * i;
        int offset = read_dword(entry + 24);
        if (strcmp(name, section_strtab + read_dword(entry)) == 0)
            return offset;
    }

    assert(0);
}

ObjectData *new_object_data(char *data, int data_size)
{
    ObjectData *obj = (ObjectData *)safe_malloc(sizeof(ObjectData));
    obj->data = data;
    obj->data_size = data_size;
    obj->entire_size = roundup(data_size, 16);
    obj->shdr = obj->symtab = obj->strtab = NULL;

    // parse data
    obj->shdr = data + read_dword(data + 40);
    char *section_strtab =
        data + read_dword(obj->shdr + 0x40 * read_word(data + 62) + 24);
    obj->nshdr = read_word(data + 60);

    for (int i = 1; i < obj->nshdr; i++) {
        char *entry = obj->shdr + 0x40 * i;
        char *offset = data + read_dword(entry + 24);
        int size = read_dword(entry + 32);
        char *name = section_strtab + read_dword(entry);
        if (obj->symtab == NULL && strcmp(name, ".symtab") == 0) {
            obj->symtab = offset;
            obj->nsymtab = size / 24;
        }
        if (obj->strtab == NULL && strcmp(name, ".strtab") == 0)
            obj->strtab = offset;
        if (obj->rela_text == NULL && strcmp(name, ".rela.text") == 0) {
            obj->rela_text = offset;
            obj->nrela_text = size / 24;
        }
    }
    assert(obj->shdr != NULL && obj->symtab != NULL && obj->strtab != NULL);

    return obj;
}

ObjectData *read_entire_binary(char *filepath)
{
    FILE *fh = fopen(filepath, "rb");
    if (fh == NULL) error("no such binary file: '%s'", filepath);

    // read the file all
    StringBuilder *sb = new_string_builder();
    int ch;
    while ((ch = fgetc(fh)) != EOF) string_builder_append(sb, ch);

    fclose(fh);

    // string_builder_size() returns the size including a null character.
    return new_object_data(string_builder_get(sb), string_builder_size(sb) - 1);
}

int search_symbol(Vector *objs, const char *name, int header_offset)
{
    int prev_offset = header_offset;

    for (int i = 0; i < vector_size(objs); i++) {
        ObjectData *obj = (ObjectData *)vector_get(objs, i);
        for (int j = 0; j < obj->nsymtab; j++) {
            char *entry = obj->symtab + 24 * j;
            if (strcmp(obj->strtab + read_dword(entry), name) != 0) continue;
            int st_info = read_byte(entry + 4), st_shndx = read_word(entry + 6),
                st_value = read_dword(entry + 8);
            if (st_shndx == 0) continue;
            return prev_offset + read_dword(obj->shdr + 0x40 * st_shndx + 24) +
                   st_value;
        }
        prev_offset += obj->entire_size;
    }

    error("undefined symbol: %s", name);
}

void link_objs_detail(Vector *objs, int header_offset)
{
    int prev_offset = header_offset;

    for (int i = 0; i < vector_size(objs); i++) {
        ObjectData *obj = (ObjectData *)vector_get(objs, i);
        for (int j = 0; j < obj->nrela_text; j++) {
            char *entry = obj->rela_text + j * 24;
            int r_offset = read_dword(entry),
                r_info_type = read_dword(entry + 8),
                r_info_symtabidx = read_dword(entry + 12),
                r_addend = read_dword(entry + 16);
            char *symtab_entry = obj->symtab + 24 * r_info_symtabidx;
            int st_shndx = read_word(symtab_entry + 6);

            int reled_addr = -1;
            if (st_shndx == 0) {  // SHN_UNDEF
                char *name = obj->strtab + read_dword(symtab_entry);
                reled_addr =
                    search_symbol(objs, name, header_offset) + r_addend;
            }
            else {
                reled_addr = prev_offset +
                             read_dword(obj->shdr + 0x40 * st_shndx + 24) +
                             read_dword(symtab_entry + 8) + r_addend;
            }
            assert(reled_addr != -1);

            int offset = r_offset + get_section_offset(obj, ".text");
            switch (r_info_type) {
                case 2: {  // R_X86_64_PC32
                    int addr = reled_addr - (prev_offset + offset);
                    obj->data[offset] = addr & 0xff;
                    obj->data[offset + 1] = (addr >> 8) & 0xff;
                    obj->data[offset + 2] = (addr >> 16) & 0xff;
                    obj->data[offset + 3] = (addr >> 24) & 0xff;
                } break;

                default:
                    assert(0);
            }
        }

        prev_offset += obj->entire_size;
    }
}

ExeImage *link_objs(Vector *obj_paths)
{
    int vaddr_offset = 0x400000, header_size = 4096,  // 64 + 56 + 8,
        header_offset = vaddr_offset + header_size;

    Vector *objs = new_vector();

    for (int i = 0; i < vector_size(obj_paths); i++)
        vector_push_back(objs,
                         read_entire_binary((char *)vector_get(obj_paths, i)));

    link_objs_detail(objs, header_offset);

    ExeImage *exe = (ExeImage *)safe_malloc(sizeof(ExeImage));
    exe->objs = objs;
    exe->vaddr_offset = vaddr_offset;
    exe->header_size = header_size;
    return exe;
}

void dump_exe_image(ExeImage *exeimg, FILE *fh)
{
    Vector *dumped = new_vector();
    set_buffer_to_emit(dumped);

    //
    // *** ELF HEADER ***
    //

    int header_offset = 0;

    // ELF magic number
    emit_dword(0x7f, 0x45, 0x4c, 0x46);
    // 64bit
    emit_byte(0x02);
    // little endian
    emit_byte(0x01);
    // original version of ELF
    emit_byte(0x01);
    // System V
    emit_byte(0x00);  // 0x03 GNU
    // padding
    emit_qword(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    // ET_REL
    emit_word(0x02, 0x00);
    // x86-64
    emit_word(0x3e, 0x00);
    // original version of ELF
    emit_dword(0x01, 0x00, 0x00, 0x00);

    // addr of entry point (placeholder)
    int ep_addr = emitted_size();
    emit_qword(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    // addr of program header table
    emit_qword(0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
    // addr of section header table
    emit_qword(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    // flag
    emit_dword(0x00, 0x00, 0x00, 0x00);

    // size of this header
    emit_word(0x40, 0x00);

    // size of program header table entry
    emit_word(0x38, 0x00);
    // number of entries in program header table
    emit_word(0x01, 0x00);

    // size of section header table entry
    emit_word(0x00, 0x00);
    // number of entries in section header table
    emit_word(0x00, 0x00);
    // index of section header entry containing section names
    emit_word(0x00, 0x00);

    int header_size = emitted_size() - header_offset;

    //
    // *** PROGRAM HEADER ***
    //

    // PT_LOAD
    emit_dword_int(1);
    // PF_X | PF_W | PF_R
    emit_dword_int(1 | 2 | 4);
    // offset
    emit_qword_int(0, 0);
    // virtual address in memory
    emit_qword_int(exeimg->vaddr_offset, 0);
    // reserved (phisical address in memory ?)
    emit_qword_int(exeimg->vaddr_offset, 0);
    // size of segment in file (placeholder)
    int filesz_addr = emitted_size();
    emit_qword_int(0, 0);
    // size of segment in memory (placeholder)
    int memsz_addr = emitted_size();
    emit_qword_int(0, 0);
    // alignment
    emit_qword_int(0x1000, 0);

    // padding
    for (int i = emitted_size(); i < exeimg->header_size; i++) emit_byte(0);
    assert(emitted_size() == exeimg->header_size);

    //
    // *** BODY ***
    //

    int body_offset = emitted_size();

    for (int i = 0; i < vector_size(exeimg->objs); i++) {
        ObjectData *obj = (ObjectData *)vector_get(exeimg->objs, i);
        for (int j = 0; j < obj->data_size; j++) emit_byte(obj->data[j]);
        for (int j = 0; j < obj->entire_size - obj->data_size; j++)
            emit_byte(0);
    }

    // rewrite placeholders
    int ep_offset = search_symbol(exeimg->objs, "_start",
                                  exeimg->vaddr_offset + exeimg->header_size);
    reemit_byte(ep_addr + 0, (ep_offset >> 0) & 0xff);
    reemit_byte(ep_addr + 1, (ep_offset >> 8) & 0xff);
    reemit_byte(ep_addr + 2, (ep_offset >> 16) & 0xff);
    reemit_byte(ep_addr + 3, (ep_offset >> 24) & 0xff);

    int filesize = emitted_size();
    reemit_byte(filesz_addr + 0, (filesize >> 0) & 0xff);
    reemit_byte(filesz_addr + 1, (filesize >> 8) & 0xff);
    reemit_byte(filesz_addr + 2, (filesize >> 16) & 0xff);
    reemit_byte(filesz_addr + 3, (filesize >> 24) & 0xff);
    reemit_byte(memsz_addr + 0, (filesize >> 0) & 0xff);
    reemit_byte(memsz_addr + 1, (filesize >> 8) & 0xff);
    reemit_byte(memsz_addr + 2, (filesize >> 16) & 0xff);
    reemit_byte(memsz_addr + 3, (filesize >> 24) & 0xff);

    // write dumped to file
    for (int i = 0; i < vector_size(dumped); i++)
        write_byte(fh, (int)vector_get(dumped, i));

    return;
}
