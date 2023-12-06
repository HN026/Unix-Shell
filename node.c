/*
@author: Huzaifa Naseer
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "shell.h"
#include "node.h"
#include "parser.h"

/*The new_node() function creates a new node and sets it's type field*/
struct node_s *new_node(enum node_type_e type)
{
    struct node_s *node = malloc(sizeof(struct node_s));

    if (!node)
    {
        return NULL;
    }

    memset(node, 0, sizeof(struct node_s));
    node->type = type;

    return node;
}

/* add_child_node() function expands the AST of a command by adding a new child node and incrementing the root node's children field. If the root node has no children, the new child is assigned to the first_child field of the root node. Otherwise, the child is appended to the end of the children's list.*/
void add_child_node(struct node_s *parent, struct node_s *child)
{
    if (!parent || !child)
    {
        return;
    }

    if (!parent->first_child)
    {
        parent->first_child = child;
    }
    else
    {
        struct node_s *sibling = parent->first_child;

        while (sibling->next_sibling)
        {
            sibling = sibling->next_sibling;
        }

        sibling->next_sibling = child;
        child->prev_sibling = sibling;
    }
    parent->children++;
}

/* set_node_val_str() function sets a node's value to the given string. It copies the string to a newly allocated memory space, then sets the val_type and val.str fields accordingly.*/
void set_node_val_str(struct node_s *node, char *val)
{
    node->val_type = VAL_STR;

    if (!val)
    {
        node->val.str = NULL;
    }
    else
    {
        char *val2 = malloc(strlen(val) + 1);

        if (!val2)
        {
            node->val.str = NULL;
        }
        else
        {
            strcpy(val2, val);
            node->val.str = val2;
        }
    }
}

/*The free_node_tree() function frees the memory used by a node structure.*/
void free_node_tree(struct node_s *node)
{
    if (!node)
    {
        return;
    }

    struct node_s *child = node->first_child;

    while (child)
    {
        struct node_s *next = child->next_sibling;
        free_node_tree(child);
        child = next;
    }

    if (node->val_type == VAL_STR)
    {
        if (node->val.str)
        {
            free(node->val.str);
        }
    }
    free(node);
}