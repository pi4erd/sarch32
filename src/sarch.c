#include <sarch.h>

#include <stdio.h>
#include <stdlib.h>

#define CAST_TO_FLOAT(U) *(float*)&U

static const char* instruction_names[] = {

};

SArch32 *SArch32_new(ReadFunc read, WriteFunc write)
{
    SArch32* result = (SArch32*)malloc(sizeof(SArch32));

    result->read = read;
    result->write = write;

    return result;
}

void SArch32_destroy(SArch32 *sarch)
{
    free(sarch);
}
