/*
@author: Huzaifa Naseer
*/

#include "../shell.h"
#include "../symtab/symtab.h"

int dump(int argc, char **argv)
{
    dump_local_symtab();
    return 0;
}