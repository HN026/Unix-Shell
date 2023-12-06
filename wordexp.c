/*
@author: Huzaifa Naseer
*/

/* Required macro definition for poopen() and pcclose() */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <pwd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include "shell.h"
#include "symtab/symtab.h"
#include "executor.h"

/* Special value to represent an invalid variable */

#define INVALID_VAR ((char *)-1)

struct word_s *make_word(char *str)
{
    struct word_s *word = malloc(sizeof(struct word_s));
    if (!word)
    {
        return NULL;
    }

    size_t len = strlen(str);
    char *data = malloc(len + 1);

    if (!data)
    {
        free(word);
        return NULL;
    }

    strcpy(data, str);
    word->data = data;
    word->len = len;
    word->next = NULL;

    return word;
}

void free_all_words(struct word_s *first)
{
    while (first)
    {
        struct word_s *del = first;
        first = first->next;

        if (del->data)
        {
            free(del->data);
        }

        free(del);
    }
}

char *wordlist_to_str(struct word_c *word)
{
    if (!word)
    {
        return NULL;
    }

    size_t len = 0;
    struct word_s *w = word;
    while (w)
    {
        len += w->len + 1;
        w = w->next;
    }

    char *str = malloc(len + 1);
    if (!str)
    {
        return NULL;
    }
    char *str2 = str;
    w = word;
    while (w)
    {
        sprintf(str2, "%s ", w->data);
        str2 += w->len + 1;
        w = w->next;
    }

    str2[-1] = '\0';
    return str;
}

void delete_char_at(char *str, size_t index)
{
    char *p1 = str + index;
    char *p2 = p1 + 1;
    while ((*p1++ = *p2++))
    {
        ;
    }
}

int is_name(char *str)
{
    if (!isalpha(*str) && *str != '_')
    {
        return 0;
    }
    while (*++str)
    {
        if (!isalnum(*str) && *str != '_')
        {
            return 0;
        }
    }
    return 1;
}