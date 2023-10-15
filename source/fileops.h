#pragma once

#include "core.h"
#include <glad/glad.h>

#define fops_buffer_size 4096
extern char fops_buffer[fops_buffer_size];

void fops_read(const char *file_path);
