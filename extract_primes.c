/*************************************************************
* Author:		Joshua Hardy
* Filename:		extract_primes.c
* Date Created:	1/6/2018
**************************************************************/

#include <stdio.h>
#include <stdlib.h> //strtoull()
#include <string.h>

int main(int argc, char **argv)
{
    char *input_file = argv[1];
    char *output_file = argv[2];
    FILE *f_in = NULL;
    FILE *f_out = NULL;
    size_t file_size = 0;
    char *str_input = NULL;
    char *str_walk = NULL;
    char *str_save = NULL;
    char *str_number = NULL;
    f_in = fopen(input_file, "r");
    f_out = fopen(output_file, "w");
    
    if(f_in == NULL || f_out == NULL)
    {
        fprintf(stdout, "Failed to open input or output file.\n");
        exit(EXIT_FAILURE);
    }

    fseek(f_in, 0, SEEK_END);
    file_size = ftell(f_in);
    fseek(f_in, 0, SEEK_SET);

    str_input = (char*) malloc(sizeof(char) * (file_size + 1));
    fread(str_input, sizeof(char), file_size, f_in);
    fclose(f_in);
    str_input[file_size] = 0;
    str_save = str_walk = str_input;

    while(*str_walk)
    {
        while(*str_input == ' ' && *str_input != 0)
            str_input++;

        str_walk = str_input;

        while(*str_walk != ' ' && *str_walk != 0)
            str_walk++;

        str_number = strndup(str_input, (str_walk - str_input));

        fwrite(str_number, sizeof(char), strlen(str_number), f_out);
        fwrite("\n", sizeof(char), 1, f_out);

        free(str_number);
        str_input = str_walk;
    }

    free(str_save);
    fclose(f_out);

    return 0;
}
