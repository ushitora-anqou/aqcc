#include "aqcc.h"

struct ExeImage {
    Vector *objs;  // vector<BinaryData *>
};

typedef struct BinaryData BinaryData;
struct BinaryData {
    char *data;
    int size;
};

BinaryData *new_binary_data(char *data, int size)
{
    BinaryData *bin = (BinaryData *)safe_malloc(sizeof(BinaryData));
    bin->data = data;
    bin->size = size;
    return bin;
}

BinaryData *read_entire_binary(char *filepath)
{
    FILE *fh = fopen(filepath, "rb");
    if (fh == NULL) error("no such binary file: '%s'", filepath);

    // read the file all
    StringBuilder *sb = new_string_builder();
    int ch;
    while ((ch = fgetc(fh)) != EOF) string_builder_append(sb, ch);

    fclose(fh);

    // string_builder_size() returns the size including a null character.
    return new_binary_data(string_builder_get(sb), string_builder_size(sb) - 1);
}

ExeImage *link_objs(Vector *obj_paths)
{
    Vector *objs = new_vector();

    for (int i = 0; i < vector_size(obj_paths); i++)
        vector_push_back(objs,
                         read_entire_binary((char *)vector_get(obj_paths, i)));

    ExeImage *exe = (ExeImage *)safe_malloc(sizeof(ExeImage));
    exe->objs = objs;
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
    emit_qword_int(0x400000, 0);
    // reserved (phisical address in memory ?)
    emit_qword_int(0x400000, 0);
    // size of segment in file (placeholder)
    int filesz_addr = emitted_size();
    emit_qword_int(0, 0);
    // size of segment in memory (placeholder)
    int memsz_addr = emitted_size();
    emit_qword_int(0, 0);
    // alignment
    emit_qword_int(0x1000, 0);

    //
    // *** BODY ***
    //

    for (int i = 0; i < vector_size(exeimg->objs); i++) {
        BinaryData *obj = (BinaryData *)vector_get(exeimg->objs, i);
        for (int j = 0; j < obj->size; j++) emit_byte(obj->data[j]);
    }

    // rewrite placeholders
    int ep_offset = 0x4000b8;  // TODO
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
