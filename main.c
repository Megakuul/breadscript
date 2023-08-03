#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <cith.h>

/**
 If the lexer, parser or evaluator fails,
 it will write the error into this string
 and return its function with a error
*/
static cstring ERROR_STR;

/* Lexer */

static size_t CurIndex = 0; 

static cstring CurIdentifier;

static double CurNum;

typedef enum {
  tok_eof = -1,
  tok_identifier = -2,
  tok_num = -3,
  tok_fun = -4,
  tok_print = -5,

  tok_invalid = -400
} Token;

static char getnextchar(char* str) {
  char charbuf = str[CurIndex];
  CurIndex++;
  return charbuf;
}

static Token getnexttoken(cstring* str) {

  static char LastChar = ' ';

  while (isspace(LastChar))
    LastChar=getnextchar(str.str);

  if (isalpha(LastChar)) {
    strsetc(CurIdentifier, LastChar);
   
    while (isalnum(LastChar=getnextchar(str.str))) {
      straddc(CurIdentifier, LastChar);

      if (CurIdentifier=="fun")
	return tok_fun;
      if (CurIdentifier=="say")
	return tok_print;
      return tok_identifier;
    }
  }

  if (isdigit(LastChar) || LastChar == '.') {
    cstring numstr;
    strinit(&numstr, NULL);
    do {
      straddc(&numstr, LastChar);
      LastChar = getnextchar(str.str);
    } while(isdigit(LastChar) || LastChar == '.');
    char* endptr;
    CurNum = strtod(numstr.str, &endptr);
    strfree(numstr);
    if (*endptr != '\0') {
      // TODO: use a sprinter to push it to the errorstring and return -400
      // If you read this Im implementing a usable sprintf in cith
      fprintf(stderr, "Invalid char [%c] at [%zu]", *endptr, CurIndex);
      return tok_invalid;
    }
    return tok_num;
  }

  if (LastChar == '~') {
    do {
      LastChar = getnextchar(str.str);
    } while (LastChar!=EOF && LastChar != '~');

    if (LastChar!=EOF) {
      return getnexttoken(str);
    }
  }

  if (LastChar==EOF)
    return tok_eof;

  return LastChar;
}

/* Lexer */

int main(int argc, char *argv[]) {
  
  if (argc!=2) {
    fprintf(stderr, "usage: breadscript [script_path]\n");
    fprintf(stderr, "Missing 1 argument\n");
    exit(1);
  }

  FILE *sf = fopen(argv[1], "r");
  if (!sf) {
    fprintf(stderr, "Failed to open script: [%s]", argv[1]);
    exit(1);
  }

  fseek(sf, 0, SEEK_END);
  long sf_size = ftell(sf);
  fseek(sf, 0, SEEK_SET);

  cstring scstr;
  strinit(&scstr, NULL);
  cres res = strcap(&scstr, sf_size + 1);
  if (res.e!=NULL) {
    fprintf(stderr, "Failed to allocate raw script");
    exit(1);
  }
  
  fread(scstr.str, 1, sf_size, sf);
  fclose(sf);
  scstr.str[sf_size] = '\0';

  strinit(&CurIdentifier);
  printf("Code: \n\n%s\n", scstr.str);

  
  strfree(&scstr);
}
