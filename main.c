#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <cith.h>

// End of String
#define EOS '\0'

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

static int getnexttoken(cstring* str) {
  
  static int LastChar = ' ';

  while (isspace(LastChar))
    LastChar=getnextchar(str->str);
  
  if (isalpha(LastChar)) {
    strsetc(&CurIdentifier, LastChar);
    while (isalnum(LastChar=getnextchar(str->str)))
      straddc(&CurIdentifier, LastChar);
    
    if (strcmp(CurIdentifier.str, "fun") == 0)
      return tok_fun;
    if (strcmp(CurIdentifier.str, "say") == 0)
      return tok_print;
    return tok_identifier;
  }

  if (isdigit(LastChar) || LastChar == '.') {
    cstring numstr;
    strinit(&numstr, NULL);
    do {
      straddc(&numstr, LastChar);
      LastChar = getnextchar(str->str);
    } while(isdigit(LastChar) || LastChar == '.');
    char* endptr;
    CurNum = strtod(numstr.str, &endptr);
    strfree(&numstr);
    if (*endptr != EOS) {
      // TODO: Use getPositionFromIndex function to determine the exact location in the file
      // https://github.com/Megakuul/to-compiler/blame/main/src/logger.cpp
      strcsprintf(&ERROR_STR, "Invalid char [%c] at [%zu]", *endptr, CurIndex);
      return tok_invalid;
    }
    return tok_num;
  }
  
  if (LastChar == '~') {
    do {
      LastChar = getnextchar(str->str);
    } while (LastChar!=EOF && LastChar != '~' && LastChar!=EOS);
    
    if (LastChar!=EOF && LastChar!=EOS) {
      // Eat the closing symbol
      LastChar = getnextchar(str->str);
      return getnexttoken(str);
    }
  }

  if (LastChar==EOF || LastChar==EOS)
    return tok_eof;

  int ThisChar = LastChar;
  LastChar = getnextchar(str->str);
  return ThisChar;
}

/* Lexer */

/* Parser */

static int CurToken;
static int ReadNextTok(cstring *str) {
  return CurToken = getnexttoken(str);
}
/* Parser */

/* Driver */

static void MainLoop(cstring *src) {
  ReadNextTok(src);
  while(1) {
    switch (CurToken) {
    case tok_invalid:
    case tok_eof:
      return;
    case tok_fun:
      printf("Parse Function\n");
      break;
    case tok_print:
      printf("Parse Print\n");
      break;
    default:
      printf("Parse Top Level\n");
      break;
    }
    ReadNextTok(src);
  }
}

/* Driver */

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

  strinit(&CurIdentifier, NULL);
  strinit(&ERROR_STR, NULL);
  printf("Code: \n\n%s\n", scstr.str);

  MainLoop(&scstr);

  int exit = 0;
  if (ERROR_STR.str!=NULL) {
    exit = 1;
    fprintf(stderr, "[ Error ]\n%s\n", ERROR_STR.str);
  }
  
  strfree(&scstr);
  strfree(&ERROR_STR);
  strfree(&CurIdentifier);

  return exit;
}
