#ifndef NISSE_H_
#define NISSE_H_

/*
** nisse, the not intricate serialised s expressions.
**
** do #define NISSE_NO_ERROR to disable errors on stdout
**
** NOTE: does not support escaped characters in strings!
**      However strings will include whatever binary junk you put into them except `
**      Basically you only have "raw" strings
*/

/*
** TODO:
** Perhaps some better error messages with line indication and so on.
** Better programatic error handling.
*/

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

enum nisse_types {
        NISSE_TYPE_NONE,
        NISSE_TYPE_INT,    // 10
        NISSE_TYPE_FLOAT,  // 1.000000
        NISSE_TYPE_STRING, // string or `string with whitespace`
        NISSE_TYPE_ARRAY, // (....) or (name ......) for 'tagged' variables
};

typedef struct nisse_data_entry_s {
        enum nisse_types type;
        union {
                int i;
                float f;
                char* str;
                struct nisse_data_entry_s* nde;
        };

        int is_str_allocated;

        //////////////////
        // NISSE_TYPE_ARRAY stuff
        int nde_len;
        int is_nde_allocated;
        int len_is_fixed;

        // for pretty serialisation
        int new_line_at_start;
        int new_line_at_end;
        int new_line_at_end_of_subsequent_elements;
} nde_t;

// anonymous nde macros
#define nisse_andei(__i) {.type = NISSE_TYPE_INT, .i = __i}
#define nisse_andef(__f) {.type = NISSE_TYPE_FLOAT, .f = __f}
#define nisse_andes(__s) {.type = NISSE_TYPE_STRING, .str = __s}
#define nisse_andea(__count, ...) {.type = NISSE_TYPE_ARRAY, .nde_len = __count, .nde = (nde_t[]){__VA_ARGS__}}
#define nisse_andeanl(__count, ...) {.new_line_at_end_of_subsequent_elements = 1, .type = NISSE_TYPE_ARRAY, .nde_len = __count, .nde = (nde_t[]){__VA_ARGS__}}

// tagged nde macros
#define nisse_tndei(__name, __i) {.type = NISSE_TYPE_ARRAY, .nde_len = 2, .nde = (nde_t[]){nisse_andes(__name), nisse_andei(__i)}}
#define nisse_tndef(__name, __f) {.type = NISSE_TYPE_ARRAY, .nde_len = 2, .nde = (nde_t[]){nisse_andes(__name), nisse_andef(__f)}}
#define nisse_tndes(__name, __s) {.type = NISSE_TYPE_ARRAY, .nde_len = 2, .nde = (nde_t[]){nisse_andes(__name), nisse_andes(__s)}}
#define nisse_tndea(__name, __count, ...) nisse_andea(__count + 1, nisse_andes(__name), __VA_ARGS__)
#define nisse_tndeanl(__name, __count, ...) nisse_andeanl(__count + 1, nisse_andes(__name), __VA_ARGS__)

extern int nisse_nde_fits_format(nde_t* nde, nde_t* fmt);
// shallow comapre, does not support anonymous string arrays
// if nde is array:
// > will check if named structs are present in fmt
// > will check if anonymous variables have the right type for their given index in nde
// >                                      (direct index in nde, not like in c literals)
// > if it is anonymous and fmt has tagged array at that index with a single variable,
// >                                                       it will check them instead
// else:
// > check if type is the same

extern nde_t* nisse_nde_get_value(nde_t* nde, int* len); // returns nde->nde + 1 if nde->nde is string
extern nde_t* nisse_nde_get_tagged(nde_t* nde, const char* tag);
extern nde_t* nisse_nde_get_index(nde_t* nde, int index); // has bounds checking

extern int nisse_write_to_file(char* filename, const nde_t nde);

extern nde_t nisse_parse_file(char* filename);
extern nde_t nisse_parse_memory(char* mem, int sz);

extern void nisse_free_nde(nde_t* nde);
extern nde_t nisse_dup_nde(nde_t* nde, int free_old_nde);







#ifdef NISSE_IMPL


#ifndef NISSE_NO_ERROR
static void
nisse_eprintf(const char* fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
}
#else // NISSE_NO_ERROR
#define nisse_eprintf(...)
#endif // NISSE_NO_ERROR

int
nisse_nde_fits_format(nde_t* nde, nde_t* fmt)
{
        if (!nde) return 0;

        if (nde->type == NISSE_TYPE_ARRAY) {
                for (int i = nde->nde_len && nde->nde->type == NISSE_TYPE_STRING; i < nde->nde_len; i++) {
                        if (nde->nde[i].type == NISSE_TYPE_ARRAY && nde->nde[i].nde_len && nde->nde[i].nde->type == NISSE_TYPE_STRING) {
                                if (!nisse_nde_get_tagged(fmt, nde->nde[i].nde->str)) return 0;
                        } else {
                                nde_t* res = nisse_nde_get_index(fmt, i);
                                if (!res) return 0;
                                if (nde->nde[i].type != NISSE_TYPE_ARRAY && res->type == NISSE_TYPE_ARRAY && res->nde_len == 2) {
                                        if (res->nde[1].type != nde->nde[i].type)
                                                return 0;
                                } else if (nde->nde[i].type != res->type) {
                                        return 0;
                                }
                        }
                }
                return 1;
        }
        return nde->type == fmt->type;
}

nde_t*
nisse_nde_get_value(nde_t* nde, int* len)
{
        if (!nde) return NULL;
        if (len) *len = 1;
        if (nde->type != NISSE_TYPE_ARRAY) return nde;

        if (!nde->nde || nde->nde_len < 2 || nde->nde->type != NISSE_TYPE_STRING) return nde->nde;
        if (len) *len = nde->nde_len - 1;
        return nde->nde + 1;
}

nde_t*
nisse_nde_get_tagged(nde_t* nde, const char* tag)
{
        if (!nde || nde->type != NISSE_TYPE_ARRAY || !nde->nde) return NULL;

        for (int i = 0; i < nde->nde_len; i++)
                if (nde->nde[i].type == NISSE_TYPE_ARRAY && nde->nde[i].nde_len && nde->nde[i].nde->type == NISSE_TYPE_STRING) {
                        if (strcmp(tag, nde->nde[i].nde->str) == 0)
                                return nde->nde + i;
                }
        return NULL;
}

nde_t*
nisse_nde_get_index(nde_t* nde, int index)
{
        if (!nde || nde->type != NISSE_TYPE_ARRAY || !nde->nde || index < 0 || index >= nde->nde_len)
                return NULL;
        return nde->nde + index;
}

static
void nisse_write_nde(FILE* fd, const nde_t* nde, int root, int indents, int new_line_start, int first)
{
        if (!nde) {
                nisse_eprintf("NISSE ERROR: nde was NULL\n");
                return;
        }

        if ((new_line_start && (!first || root)) || nde->new_line_at_start) {
                fprintf(fd, "\n");
                for (int i = 0; i < indents; i++)
                        fprintf(fd, "    ");
        } else if (!root && !first) {
                fprintf(fd, " ");
        }

        if (nde->type == NISSE_TYPE_ARRAY) {
                fprintf(fd, "(");
        } else if (root) {
                nisse_eprintf("NISSE ERROR: a root nde was not an array\n");
                return;
        }

        if (nde->type == NISSE_TYPE_NONE) {
                nisse_eprintf("NISSE ERROR: skipping writing an uninitialized member\n");
        } else if (nde->type == NISSE_TYPE_INT) {
                fprintf(fd, "%d",   nde->i);
        } else if (nde->type == NISSE_TYPE_FLOAT)  {
                fprintf(fd, "%f",   nde->f);
        } else if (nde->type == NISSE_TYPE_STRING) {
                int res = 0;
                for (int i = 0; i < strlen(nde->str); i++) {
                        if (i == 0 && memchr("1234567890.-", nde->str[i], sizeof("1234567890.-"))) {
                                res = 1;
                                break;
                        }
                        if (memchr("\n '\t\v()\r", nde->str[i], sizeof("\n '\t\v()\r"))) {
                                res = 1;
                                break;
                        }
                }
                if (strchr(nde->str, '`')) {
                        nisse_eprintf("NISSE ERROR: string '%s' contains a `, nisse does not support this\n", nde->str);
                } else {
                        if (res) fprintf(fd, "`%s`", nde->str);
                        else     fprintf(fd, "%s", nde->str);
                }
        } else if (nde->type == NISSE_TYPE_ARRAY) {
                for (int i = 0; i < nde->nde_len; i++)
                        nisse_write_nde(fd, nde->nde + i, 0, indents + 1, nde->new_line_at_end_of_subsequent_elements, i == 0);
        }

        if (nde->new_line_at_end_of_subsequent_elements) {
                fprintf(fd, "\n");
                for (int i = 0; i < indents; i++)
                        fprintf(fd, "    ");
        }
        if (nde->type == NISSE_TYPE_ARRAY)
                fprintf(fd, ")");
}

int
nisse_write_to_file(char* filename, const nde_t nde)
{
        if (nde.type != NISSE_TYPE_ARRAY) return 0;

        FILE* fd = fopen(filename, "wb");
        if (!fd) {
                nisse_eprintf("NISSE ERROR: unable to open file\n");
                return 0;
        }

        for (int i = 0; i < nde.nde_len; i++)
                nisse_write_nde(fd, nde.nde + i, 1, 0, i > 0, 1);

        fclose(fd);
        return 1;
}

nde_t
nisse_parse_file(char* filename)
{
        FILE* file = fopen(filename, "rb");
        if (!file) {
                nisse_eprintf("NISSE ERROR: unable to open file\n");
                return (nde_t){0};
        }

        fseek(file, 0L, SEEK_END);
        int readsize = ftell(file);
        rewind(file);

        char* buffer = malloc(readsize);
        if (!buffer) return (nde_t){0};

        fread(buffer, 1, readsize, file);
        fclose(file);

        return nisse_parse_memory(buffer, readsize);
}

static int
nisse_seek_whitespace(char* mem, int index, int sz)
{
        while (index < sz && !memchr("\n '\t()\v\r", mem[index], sizeof("\n '\t()\v\r")))
                index++;
        return index;
}

static int
nisse_parse_memory_array(char* mem, int sz, nde_t* nde) {
        int index = 0;
        if (index >= sz) return index;
        if (mem[index] == '(') index++;
        if (index >= sz) return index;

        nde->type = NISSE_TYPE_ARRAY;
        nde->is_nde_allocated = 1;
        int new_line_on_all_elements = 1;

        while (index < sz) {
                int last_index = index;

                while (index < sz && memchr("\n \t\v\r", mem[index], sizeof("\n \t\v\r")))
                        index++;

                if (mem[index] == ')') {
                        if (nde->nde_len <= 1) new_line_on_all_elements = 0;
                        break;
                }

                int i = ++nde->nde_len - 1;
                nde->nde = realloc(nde->nde, sizeof(*nde->nde) * nde->nde_len);
                nde->nde[i] = (nde_t){0};

                if (index != last_index && memchr(mem + last_index, '\n', index - last_index))
                        nde->nde[i].new_line_at_start = 1;

                if (memchr("1234567890.-", mem[index], sizeof("1234567890.-"))) {
                        int number_end = nisse_seek_whitespace(mem, index, sz);

                        char tmp = mem[number_end];
                        mem[number_end] = 0;
                        if (memchr(mem + index, '.', number_end - index)) /* float */ {
                                nde->nde[i].f = atof(mem + index);
                                nde->nde[i].type = NISSE_TYPE_FLOAT;
                        } else /* int */ {
                                nde->nde[i].i = atoi(mem + index);
                                nde->nde[i].type = NISSE_TYPE_INT;
                        }
                        mem[number_end] = tmp;
                        index = number_end;
                } else if (mem[index] == '(') /* array */ {
                        index += nisse_parse_memory_array(mem + index, sz - index, nde->nde + i);
                } else if (mem[index] == '`') /* string */ {
                        char* end;
                        int times = 0;
                        do {
                                times++;
                                end = (sz - index - 1 < 0) ? NULL : memchr(mem + index + times, '`', sz - index - times);
                        } while (end && end[-1] == '\\');
                        if (!end) {
                                nisse_eprintf("NISSE ERROR: unable to find closing quote\n");
                                return sz;
                        }
                        size_t strsz = (end) - (mem + index + 1);

                        nde->nde[i].str = malloc(strsz + 1);
                        nde->nde[i].is_str_allocated = 1;

                        memcpy(nde->nde[i].str, mem + index + 1, strsz);
                        nde->nde[i].str[strsz] = 0;

                        index += strsz + 2;
                        nde->nde[i].type = NISSE_TYPE_STRING;
                } else /* string without " " */ {
                        int end = nisse_seek_whitespace(mem, index, sz);

                        size_t strsz = (end - index);

                        nde->nde[i].str = malloc(strsz + 1);
                        nde->nde[i].is_str_allocated = 1;

                        memcpy(nde->nde[i].str, mem + index, strsz);
                        nde->nde[i].str[strsz] = 0;

                        index += strsz;
                        nde->nde[i].type = NISSE_TYPE_STRING;
                }

                if (mem[index] == '\n' && nde->nde[i].type == NISSE_TYPE_ARRAY && !nde->nde[i].new_line_at_start)
                        nde->nde[i].new_line_at_end = 1;

                if ((!nde->nde[i].new_line_at_start || nde->nde[i].new_line_at_end) && i != 0)
                        new_line_on_all_elements = 0;
        }
        nde->new_line_at_end_of_subsequent_elements = new_line_on_all_elements;

        if (index >= sz) {
                nisse_eprintf("NISSE ERROR: unable to find closing parenthesis\n");
                return sz;
        }
        return index + 1;
}

nde_t
nisse_parse_memory(char* mem, int sz)
{
        int index = 0;
        nde_t nde = {.type = NISSE_TYPE_ARRAY};
        nde.nde_len = 0;
        while (index < sz) {
                char* new_pos = memchr(mem + index, '(', sz - index);
                if (!new_pos) return nde;

                index += (int)(new_pos - (mem + index));

                nde.nde_len += 1;
                nde.nde = realloc(nde.nde, sizeof(*nde.nde) * nde.nde_len);
                nde.nde[nde.nde_len - 1] = (nde_t){0};

                index += nisse_parse_memory_array(mem + index, sz - index, nde.nde + (nde.nde_len - 1));
        }
        if (nde.nde_len) nde.is_nde_allocated = 1;
        return nde;
}

void
nisse_free_nde(nde_t* nde)
{

        if (nde->type == NISSE_TYPE_STRING && nde->is_str_allocated) {
                free(nde->str);
        } else if (nde->type == NISSE_TYPE_ARRAY) {
                for (int i = 0; i < nde->nde_len; i++) nisse_free_nde(nde->nde + i);
                if (nde->is_nde_allocated) free(nde->nde);
        }
}

nde_t
nisse_dup_nde(nde_t* nde, int free_old_nde)
{
        if (nde->type == NISSE_TYPE_STRING) {
                char* str = strdup(nde->str);
                if (free_old_nde && nde->is_str_allocated) free(nde->str);
                nde->is_str_allocated = 1;
                nde->str = str;
        } else if (nde->type == NISSE_TYPE_ARRAY) {
                nde_t* n = malloc(nde->nde_len * sizeof(*n));
                memcpy(n, nde->nde, nde->nde_len * sizeof(*n));
                if (free_old_nde && nde->is_nde_allocated) free(nde->nde);
                nde->nde = n;
                nde->is_nde_allocated = 1;
                for (int i = 0; i < nde->nde_len; i++) nisse_dup_nde(nde->nde + i, free_old_nde);
        }
        return *nde;
}

#endif // NISSE_IMPL

#endif // NISSE_H_
