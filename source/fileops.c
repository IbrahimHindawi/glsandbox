#include <assert.h>
#include "fileops.h"


void fops_read(const char *file_path) {
    FILE *fileptr;
    // fileptr = fopen(file_path, "r");

#ifdef _MSC_VER
    fopen_s(&fileptr, file_path, "r");
#else
    fileptr = fopen(file_path, "r");
#endif
    fseek(fileptr, 0L, SEEK_END);
    u64 filesize = ftell(fileptr);
    fseek(fileptr, 0L, SEEK_SET);
    assert(filesize <= fops_buffer_size);
    if (fileptr != NULL) {
        size_t newlen = fread(fops_buffer, sizeof(char), fops_buffer_size, fileptr);
        if (ferror(fileptr) != 0) {
            fputs("error reading file!", stderr);
        } else {
            fops_buffer[newlen++] = '\0';
        }
    }
    fclose(fileptr);
}
