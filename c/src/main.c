#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lexer.h"

const char *get_file_extension(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename)
        return "";
    return dot + 1;
}

int has_extenstion(const char *filename, const char *ext)
{
    const char *file_ext = get_file_extension(filename);
    return strcmp(file_ext, ext) == 0;
}

char *read_file(const char *filename)
{
    FILE *source_file = fopen(filename, "r");
    if (!source_file)
    {
        perror("Failed to open source file");
        return NULL;
    }

    if (!has_extenstion(filename, "ai"))
    {
        fprintf(stderr, "Error: Source file must have a .ai extension\n");
        fclose(source_file);
        return NULL;
    }

    fseek(source_file, 0, SEEK_END);
    long file_size = ftell(source_file);
    fseek(source_file, 0, SEEK_SET);

    char *src_code = (char *)malloc(file_size + 1);
    if (!src_code)
    {
        perror("Failed to allocate memory for source code");
        fclose(source_file);
        return NULL;
    }

    size_t read_size = fread(src_code, 1, file_size, source_file);
    src_code[read_size] = '\0';
    fclose(source_file);

    return src_code;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        printf("Usage: %s <source_file>\n", argv[0]);
        return 1;
    }

    char *src_code = read_file(argv[1]);
    if (!src_code)
    {
        return 1;
    }

    struct Lexer lexer;
    lexer_init(&lexer, src_code);
    lexer_lex(&lexer);
    lexer_print_toks(&lexer);
}