/*
@author: Huzaifa Naseer
*/

#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "node.h"

char *search_path(char *file);
int do_exec_cmd(int argc, char **argv);
int do_command(struct node_s *node);

#endif