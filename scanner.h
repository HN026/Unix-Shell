/*
@author: Huzaifa Naseer
*/

#ifndef SCANNER_H
#define SCANNER_H

struct token_s {
  struct source_s *src; /*Source of input*/
  int text_len;         /*Length of token text*/
  char *text;           /*token text*/
};

/*The special EOF token, which indicates the end of input*/
extern struct token_s eof_token;

struct token_s *tokenize(struct source_s *src);
void free_token(struct token_s *tok);

#endif