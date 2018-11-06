#include "cc.h"

Vector *SIMPLE_generate_code(Vector *asts)
{
    Vector *ret = new_vector();
    vector_push_back(ret, NULL);
    return ret;
}

void SIMPLE_dump_code(SIMPLECode *code, FILE *fh)
{
    fprintf(fh, "MOV R0, 10\n");
    fprintf(fh, "HLT\n");
    return;
}
