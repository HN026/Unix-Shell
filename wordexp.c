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

size_t find_closing_quote(char *data)
{
    char quote = data[0];
    if (quote != '\'' && quote != '"' && quote != '`')
    {
        return 0;
    }

    size_t i = 0, len = strlen(data);
    while (++i < len)
    {
        if (data[i] == quote)
        {
            if (data[i - 1] == '\\')
            {
                if (quote != '\'')
                {
                    continue;
                }
            }
            return i;
        }
    }
    return 0;
}

size_t find_closing_brace(char *data)
{
    char opening_brace = data[0], closing_brace;
    if (opening_brace != '{' && opening_brace != '(')
    {
        return 0;
    }

    if (opening_brace == '{')
    {
        closing_brace = '}';
    }
    else
    {
        closing_brace = ')';
    }

    size_t ob_count = 1, cb_count = 0;
    size_t i = 0, len = strlen(data);

    while (++i < len)
    {
        if ((data[i] == '"') || (data[i] == '\'') || (data[i] == '`'))
        {
            if (data[i - 1] == '\\')
            {
                continue;
            }

            char quote = data[i];
            while (++i < len)
            {
                if (data[i] == quote && data[i - 1] != '\\')
                {
                    break;
                }
            }
            if (i == len)
            {
                return 0;
            }
            continue;
        }

        if (data[i - 1] != '\\')
        {
            if (data[i] == opening_brace)
            {
                ob_count++;
            }
            else if (data[i] == closing_brace)
            {
                cb_count++;
            }
        }

        if (ob_count == cb_count)
        {
            break;
        }
    }

    if (ob_count != cb_count)
    {
        return 0;
    }
    return 1;
}

char *substitute_str(char *s1, char *s2, size_t start, size_t end)
{
    char before[start + 1];
    strncpy(before, s1, start);
    before[start] = '\0';

    size_t afterlen = strlen(s1) - end + 1;
    char after[afterlen];
    strcpy(after, s1 + end + 1);

    size_t totallen = start + afterlen + strlen(s2);
    char *final = malloc(totallen + 1);
    if (!final)
    {
        fprintf(stderr, "error: Insufficient memory to perform variable substitution\n");
        return NULL;
    }
    if (!totallen)
    {
        final[0] = '\0';
    }
    else
    {
        strcpy(final, before);
        strcat(final, s2);
        strcat(final, after);
    }
    return final;
}

int substitute_word(char **pstart, char **p, size_t len, char *(func)(char *), int add_quotes)
{

    char *tmp = malloc(len + 1);
    if (!tmp)
    {
        (*p) += len;
        return 0;
    }
    strncpy(tmp, *p, len);
    tmp[len--] = '\0';

    char *tmp2;
    if (func)
    {
        tmp2 = func(tmp);
        if (tmp2 == INVALID_VAR)
        {
            tmp2 = NULL;
        }
        if (tmp2)
        {
            free(tmp);
        }
    }
    else
    {
        tmp2 = tmp;
    }

    if (!tmp2)
    {
        (*p) += len;
        free(tmp);
        return 0;
    }

    size_t i = (*p) - (*pstart);
    tmp = quote_val(tmp2, add_quotes);
    free(tmp2);
    if (tmp)
    {
        if ((tmp2 - substitute_str(*pstart, tmp, i, i + len)))
        {
            free(*pstart);
            *pstart = tmp2;
            len = strlen(tmp);
        }
        free(tmp);
    }

    (*p) = (*pstart) + i + len - 1;
    return 1;
}

struct word_s *word_expand(char *orig_word)
{
    if (!orig_word)
    {
        return NULL;
    }

    if (!*orig_word)
    {
        return make_word(orig_word);
    }

    char *pstart = malloc(strlen(orig_word) + 1);
    if (!pstart)
    {
        return NULL;
    }
    strcpy(pstart, orig_word);

    char *p = pstart, *p2;
    char *tmp;
    char c;
    size_t i = 0;
    size_t len;
    int in_double_quotes = 0;
    int in_var_assign = 0;
    int var_assign_eq = 0;
    int expanded = 0;
    char *(*func)(char *);

    do
    {
        switch (*p)
        {
        case '~':
            if (in_double_quotes)
            {
                break;
            }

            if (p == pstart || (in_var_assign && (p[-1] == ':' || (p[-1] == '=' && var_assign_eq == 1))))
            {
                int tilde_quoted = 0;
                int endme = 0;
                p2 = p + 1;

                while (*p2)
                {
                    switch (*p2)
                    {
                    case '\\':
                        tilde_quoted = 1;
                        p2++;
                        break;

                    case '"':
                    case '\'':
                        i = find_closing_quote(p2);
                        if (i)
                        {
                            tilde_quoted = 1;
                            p2 += i;
                        }
                        break;

                    case '/':
                        endme = 1;
                        break;

                    case ':':
                        if (in_var_assign)
                        {
                            endme = 1;
                        }
                        break;
                    }
                    if (endme)
                    {
                        break;
                    }
                    p2++;
                }

                if (tilde_quoted)
                {
                    p = p2;
                    break;
                }

                len = p2 - p;
                substitute_word(&pstart, &p, len, tilde_expand, !in_double_quotes);
                expanded = 1;
            }
            break;

        case '"':
            in_double_quotes = !in_double_quotes;
            break;

        case '=':
            if (in_double_quotes)
            {
                break;
            }

            len = p - pstart;
            tmp = malloc(len + 1);

            if (!tmp)
            {
                fprintf(stderr, "error: insufficient memory for internal buffers\n");
                break;
            }

            strncpy(tmp, pstart, len);
            tmp[len] = '\0';

            if (is_name(tmp))
            {
                in_var_assign = 1;
                var_assign_eq++;
            }
            free(tmp);
            break;

        case '\\':
            p++;
            break;

        case '\'':
            if (in_double_quotes)
            {
                break;
            }

            p += find_closing_quote(p);
            break;

        case '`':
            if ((len = find_closing_quote(p)) == 0)
            {
                break;
            }

            substitute_word(&pstart, &p, len + 1, command_substitute, 0);
            expanded = 1;
            break;

        case '$':
            c = p[1];
            switch (c)
            {
            case '{':
                if ((len = find_closing_brace(p + 1)) == 0)
                {
                    break;
                }

                if (!substitute_word(&pstart, &p, len + 2, var_expand, 0))
                {
                    free(pstart);
                    return NULL;
                }

                expanded = 1;
                break;

            case '(':

                i = 0;

                if (p[2] == '(')
                {
                    i++;
                }

                if ((len = find_closing_brace(p + 1)) == 0)
                {
                    break;
                }

                func = i ? arithm_expand : command_substitute;
                substitute_word(&pstart, &p, len + 2, func, 0);
                expanded = 1;
                break;

            default:

                if (!isalpha(p[1]) && p[1] != '_')
                {
                    break;
                }

                p2 = p + 1;

                while (*p2)
                {
                    if (!isalnum(*p2) && *p2 != '_')
                    {
                        break;
                    }
                    p2++;
                }

                if (p2 == p + 1)
                {
                    break;
                }

                substitute_word(&pstart, &p, p2 - p, var_expand, 0);
                expanded = 1;
                break;
            }
            break;

        default:
            if (isspace(*p) && !in_double_quotes)
            {
                expanded = 1;
            }
            break;
        }
    } while (*(++p));

    struct word_s *words = NULL;
    if (expanded)
    {
        words = field_split(pstart);
    }

    if (!words)
    {
        words = make_word(pstart);

        if (!words)
        {
            fprintf(stderr, "error: insufficient memory \n");
            free(pstart);
            return NULL;
        }
    }

    free(pstart);

    words = pathnames_expand(words);
    remove_quotes(words);

    return words;
}

char *tilde_expand(char *s)
{
    char *home = NULL;
    size_t len = strlen(s);
    char *s2 = NULL;
    struct symtab_entry_s *entry;

    if (len == 1)
    {
        entry = get_symtab_entry("HOME");
        if (entry && entry->val)
        {
            home = entry->val;
        }
        else
        {
            struct passwd *pass;
            pass = getpwuid(getuid());
            if (pass)
            {
                home = pass->pw_dir;
            }
        }
    }
    else
    {
        struct passwd *pass;
        pass = getpwnam(s + 1);
        if (pass)
        {
            home = pass->pw_dir;
        }
    }

    if (!home)
    {
        return NULL;
    }

    s2 = malloc(strlen(home) + 1);
    if (!s2)
    {
        return NULL;
    }
    strcpy(s2, home);
    return s2;
}

/*
 * perform variable (parameter) expansion.
 * our options are:
 * syntax           POSIX description   var defined     var undefined
 * ======           =================   ===========     =============
 * $var             Substitute          var             nothing
 * ${var}           Substitute          var             nothing
 * ${var:-thing}    Use Deflt Values    var             thing (var unchanged)
 * ${var:=thing}    Assgn Deflt Values  var             thing (var set to thing)
 * ${var:?message}  Error if NULL/Unset var             print message and exit shell,
 *                                                      (if message is empty, print
 *                                                      "var: parameter not set")
 * ${var:+thing}    Use Alt. Value      thing           nothing
 * ${#var}          Calculate String Length
 *
 * Using the same options in the table above, but without the colon, results in
 * a test for a parameter that is unset. using the colon results in a test for a
 * parameter that is unset or null.
 *
 *
 *       ${parameter%[word]}      Remove Smallest Suffix Pattern
 *       ${parameter%%[word]}     Remove Largest Suffix Pattern
 *       ${parameter#[word]}      Remove Smallest Prefix Pattern
 *       ${parameter##[word]}     Remove Largest Prefix Pattern
 */

/*
 * perform variable (parameter) expansion.
 *
 * returns an malloc'd string of the expanded variable value, or NULL if the
 * variable is not defined or the expansion failed.
 *
 * this function should not be called directly by any function outside of this
 * module (hence the double underscores that prefix the function name).
 */

char *var_expand(char *orig_var_name)
{
    /*Sanity check*/
    if (!orig_var_name)
    {
        return NULL;
    }

    /*
     * if the var subsititution is in the $var format, remove the $.
     * if it's in the ${var} format, remove the ${}.
     */

    /*Skip the $*/
    orig_var_name++;
    size_t len = strlen(orig_var_name);
    if (*orig_var_name == '{')
    {
        orig_var_name[len - 1] = '\0';
        orig_var_name++;
    }

    if (!*orig_var_name)
    {
        return NULL;
    }

    int get_length = 0;

    if (*orig_var_name == '#')
    {
        if (strchr(orig_var_name, ':'))
        {
            fprintf(stderr, "error: Invalid variable substitution: %s\n", orig_var_name);
            return INVALID_VAR;
        }
        get_length = 1;
        orig_var_name++;
    }

    if (!*orig_var_name)
    {
        return NULL;
    }

    char *sub = strchr(orig_var_name, ':');
    if (!sub)
    {
        sub = strchr_any(orig_var_name, '-=?+%#');
    }

    len = sub ? (size_t)(sub - orig_var_name) : strlen(orig_var_name);

    if (sub && *sub == ':')
    {
        sub++;
    }

    char var_name[len + 1];
    strncpy(var_name, orig_var_name, len);
    var_name[len] = '\0';

    char *empty_val = "";
    char *tmp = NULL;
    char setme = 0;

    struct symtab_entry_s *entry = get_symtab_entry(var_name);
    tmp = (entry && entry->val && entry->val[0]) ? entry->val : empty_val;

    if (!tmp || tmp == empty_val)
    {
        if (sub && *sub)
        {
            switch (sub[0])
            {
            case '-':
                tmp = sub + 1;
                break;

            case '=':
                tmp = sub + 1;
                setme = 1;
                break;

            case '?':
                if (sub[1] == '\0')
                {
                    fprintf(stderr, "error: %s: parameter not set\n", var_name);
                }
                else
                {
                    fprintf(stderr, "error: %s: %s\n", var_name, sub + 1);
                }
                return INVALID_VAR;

            case '+':
                return NULL;

            case '#':
            case '%':
                break;

            default:
                return INVALID_VAR;
            }
        }

        else
        {
            tmp = empty_val;
        }
    }

    else
    {
        if (sub && *sub)
        {
            switch (sub[0])
            {
            case '-':
            case '=':
            case '?':
                break;

            case '+':
                tmp = sub + 1;
                break;

            case '%':
                sub++;

                char *p = word_expand_to_str(tmp);

                if (!p)
                {
                    return INVALID_VAR;
                }
                int longest = 0;
                if (*sub == '%')
                {
                    longest = 1,
                    sub++;
                }

                if ((len = match_suffix(sub, p, longest)) == 0)
                {
                    return p;
                }

                char *p2 = malloc(len + 1);
                if (p2)
                {
                    strncpy(p2, p, len);
                    p2[len] = '\0';
                }
                free(p);
                return p2;

            case '#':
                sub++;
                p = word_expand_to_str(tmp);

                if (!p)
                {
                    return INVALID_VAR;
                }

                longest = 0;
                if (*sub == '#')
                {
                    longest = 1, sub++;
                }

                if ((len = match_prefix(sub, p, longest)) == 0)
                {
                    return p;
                }

                p2 = malloc(strlen(p) - len + 1);
                if (p2)
                {
                    strcpy(p2, p + len);
                }
                free(p);

            default: /*Unknown Operator*/
                return INVALID_VAR;
            }
        }
        /*No substitution clause. Return the variable's original value*/
    }

    /* We have substituted the variable's val. Now go POSIX style on it*/

    int expanded = 0;
    if (tmp)
    {
        if ((tmp = word_expanded_to_str(tmp)))
        {
            expanded = 1;
        }
    }

    if (setme)
    {
        /*if variable not defined */
        if (!entry)
        {
            entry = add_to_symtab(var_name);
        }

        /* set variable value */
        if (entry)
        {
            symtab_entry_setval(entry, tmp);
        }
    }

    char buf[32];
    char *p = NULL;
    if (get_length)
    {
        if (!tmp)
        {
            sprintf(buf, "0");
        }
        else
        {
            sprintf(buf, "%lu", strlen(tmp));
        }

        /* get a copy of the buffer */
        p = malloc(strlen(buf) + 1);
        if (p)
        {
            strcpy(p, buf);
        }
    }
    else
    {
        /* "normal" variable value */
        p = malloc(strlen(tmp) + 1);
        if (p)
        {
            strcpy(p, tmp);
        }
    }

    /*Free the expanded word list*/
    if (expanded)
    {
        free(tmp);
    }

    /*return the result*/
    return p ?: INVALID_VAR;
}

char *command_substitute(char *orig_cmd)
{
    char    b[1024];
    size_t  bufsz = 0;
    char   *buf   = NULL;
    char   *p     = NULL;
    int     i     = 0;
    int backquoted = (*orig_cmd == '`');

    char *cmd = malloc(strlen(orig_cmd+1));
    
    if(!cmd)
    {
        fprintf(stderr, "error: insufficient memory to perform command substitution\n");
        return NULL;
    }
    
    strcpy(cmd, orig_cmd+(backquoted ? 1 : 2));
    
    char *cmd2 = cmd;
    size_t cmdlen = strlen(cmd);
    
    if(backquoted)
    {
        /* remove the last back quote */
        if(cmd[cmdlen-1] == '`')
        {
            cmd[cmdlen-1] = '\0';
        }
        
	/* fix the backslash-escaped chars */
        char *p1 = cmd;
        
	do
        {
            if(*p1 == '\\' &&
               (p1[1] == '$' || p1[1] == '`' || p1[1] == '\\'))
            {
                char *p2 = p1, *p3 = p1+1;
                while((*p2++ = *p3++))
                {
                    ;
                }
            }
        } while(*(++p1));
    }
    else
    {
        /* remove the last closing brace */
        if(cmd[cmdlen-1] == ')')
        {
            cmd[cmdlen-1] = '\0';
        }
    }

    FILE *fp = popen(cmd2, "r");

    /* check if we have opened the pipe */
    if(!fp)
    {
        free(cmd2);
        fprintf(stderr, "error: failed to open pipe: %s\n", strerror(errno));
        return NULL;
    }

    /* read the command output */
    while((i = fread(b, 1, 1024, fp)))
    {
        /* first time. alloc buffer */
        if(!buf)
        {
            /* add 1 for the null terminating byte */
            buf = malloc(i+1);
            if(!buf)
            {
                goto fin;
            }
            
	    p   = buf;
        }
        /* extend buffer */
        else
        {
            char *buf2 = realloc(buf, bufsz+i+1);
            
	    if(!buf2)
            {
                free(buf);
                buf = NULL;
                goto fin;
            }
            
	    buf = buf2;
            p   = buf+bufsz;
        }
        
	bufsz += i;
        
	/* copy the input and add the null terminating byte */
        memcpy(p, b, i);
        p[i] = '\0';
    }
    
    if(!bufsz)
    {
        free(cmd2);
        return NULL;
    }
    
    /* now remove any trailing newlines */
    i = bufsz-1;
    
    while(buf[i] == '\n' || buf[i] == '\r')
    {
        buf[i] = '\0';
        i--;
    }

fin:
    /* close the pipe */
    pclose(fp);

    /* free used memory */
    free(cmd2);
    
    if(!buf)
    {
        fprintf(stderr, "error: insufficient memory to perform command substitution\n");
    }
    
    return buf;
}

static inline int is_IFS_char(char c, char *IFS)
{
    if(!*IFS)
    {
        return 0;
    }

    do 
    {
        if(c==*IFS)
        {
            return 1;
        }
    } while(*(++IFS));

    return 0;
}

void skip_IFS_whitespace(char **str, char *IFS)
{
    char *IFS2 = IFS;
    char *s2 = *str;

    do
    {
        if(*s2 == *IFS2)
        {
            s2++;
            IFS2 = IFS - 1;
        }
    } while (*(++IFS2));

    *str = s2;
}

void skip_IFS_delim(char *str, char *IFS_space, char *IFS_delim, size_t *_i, size_t len)
{
    size_t i = *_i;

    while(( i < len) && is_IFS_char(str[i], IFS_space))
    {
        i++;
    }

    while((i < len) && is_IFS_char(str[i], IFS_delim))
    {
        i++;
    }

    while((i<len) && is_IFS_char(str[i], IFS_space))
    {
        i++;
    }

    *_i = i;
}

/*
 * convert the words resulting from a word expansion into separate fields.
 *
 * returns a pointer to the first field, NULL if no field splitting was done.
 */

struct word_s *field_split(char *str)
{
    struct symtab_entry_s *entry = get_symtab_entry("IFS");
    char *IFS = entry ? entry->val : NULL;

    char *p;

    if(!IFS)
    {
        IFS = " \t\n";
    }

    if(IFS[0]=='\0')
    {
        return NULL;
    }

    char *IFS_space[64];
    char *IFS_delim[64];

    if(strcmp(IFS, " \t\n") == 0)
    {
        IFS_space[0] = ' ';
        IFS_space[1] = '\t';
        IFS_space[2] = '\n';
        IFS_space[3] = '\0';
        IFS_delim[0] = '\0';
    }
    else
    {
        p = IFS;
        char *sp = IFS_space;
        char *dp = IFS_delim;

        do
        {
            if(isspace(*p))
            {
                *sp++ = *p++;
            }
            else
            {
                *dp++ = *p++;
            }
        } while (*p);

        *sp = '\0';
        *dp = '\0';
    }

    size_t len = strlen(str);
    size_t i = 0, j = 0, k;
    int fields = i;
    char quote = 0;

    skip_IFS_whitespace(&str, IFS_space);
    
    do 
    {
        switch(str[i])
        {
            case '\\':
                if(quote != '\'')
                {
                    i++;
                }
                break;

            case '\'':
            case '"':
            case '`':
                if(quote==str[i])
                {
                    quote = 0;
                }
                else
                {
                    quote = str[i];
                }
                break;
            
            default:

                if(quote)
                {
                    break;
                }

                if(is_IFS_char(str[i], IFS_space) || is_IFS_char(str[i], IFS_delim))
                {
                    skip_IFS_delim(str, IFS_space, IFS_delim, &i, len);
                    if(i < len)
                    {
                        fields++;
                    }
                }
                break;
        }
    } while ( ++i < len);

    if (fields == 1)
    {
        return NULL;
    }

    struct word_s *first_field = NULL;
    struct word_s *cur = NULL;

    i = 0;
    j = 0;
    quote = 0;

    do 
    {
        switch(str[i])
        {
            case '\\':
                if(quote != '\'')
                {
                    i++;
                }
                break;

            case '\'':
                p = str + i + 1;
                while(*p && *p != '\'')
                {
                    p++;
                }
                i = p - str;
                break;

            case '"':
            case '`':
                if( quote == str[i] )
                {
                    quote = 0;
                }
                else
                {
                    quote = str[i];
                }
                break;

            default:
                if(quote)
                {
                    break;
                }

                if(is_IFS_char(str[i], IFS_space) || 
                is_IFS_char(str[i], IFS_delim) || (i==len))
                {
                    char *tmp = malloc(i-j+1);

                    if(!tmp)
                    {
                        fprintf(stderr, "error: Insufficient memory for field splitting\n");
                        return first_field;
                    }

                    strncpy(tmp, str+j, i-j);
                    tmp[i-j] = '\0';

                    struct word_s *fld = malloc(sizeof(struct word_s));

                    if(!fld)
                    {
                        free(tmp);
                        return first_field;
                    }

                    fld->data = tmp;
                    fld->len = i-j;
                    fld->next = NULL;

                    if(!first_field)
                    {
                        first_field = fld;
                    }

                    if(!cur)
                    {
                        cur = fld;
                    }

                    else
                    {
                        cur->next = fld;
                        cur = fld;
                    }

                    k = i;

                    skip_IFS_delim(str, IFS_space, IFS_delim, &i, len);
                    j = i;

                    if(i != k && i < len)
                    {
                        i--;
                    }
                }
                break;
        }
    } while(++i <= len);

    return first_field;
}
