/*
@author: Huzaifa Naseer
*/

#ifndef PARSER_H
#define PARSER_H

#include "scanner.h" /* struct token_s*/
#include "source.h"  /* struct source_s*/

struct node_s *parse_command(struct token_s *tok);

#endif