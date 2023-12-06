/*
@author: Huzaifa Naseer
*/

#ifndef NODE_H
#define NODE_H

enum node_type_e
{
    NODE_COMMAND, /* Command */
    NODE_VAR,     /* variable name (or simple, a word)*/
};

enum val_type_e
{
    VAL_SINT = 1, /* signed integer */
    VAL_UINT,     /* unsigned integer */
    VAL_SSLONG,   /* signed long long */
    VAL_ULLONG,   /* unsigned long long */
    VAL_FLOAT,    /* float */
    VAL_LDOUBLE,  /* long double */
    VAL_CHR,      /* char */
    VAL_STR,      /* str, (char pointer)*/
};

union symval_u
{
    long sint;
    unsigned long uint;
    long long sllong;
    unsigned long long ullong;
    double sfloat;
    long double ldouble;
    char chr;
    char *str;
};

struct node_s
{
    enum node_type_e type;                      /* node type */
    enum val_type_e val_type;                   /* value type */
    union symval_u val;                         /* value */
    int children;                               /* Number of child nodes */
    struct node_s *first_child;                 /* first child node */
    struct node_s *next_sibling, *prev_sibling; /* Pointers to next/prev siblings*/
};

struct node_s *new_node(enum node_type_e type);
void add_child_node(struct node_s *parent, struct node_s *child);
void free_node_tree(struct node_s *node);
void set_node_val_str(struct node_s *node, char *val);

#endif
