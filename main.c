#include <assert.h>
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
  tok_var = -5,
  tok_print = -6,
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
    if (strcmp(CurIdentifier.str, "var") == 0)
      return tok_var;
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
    if (*endptr != EOS) {
      // TODO: Use getPositionFromIndex function to determine the exact location in the file
      // https://github.com/Megakuul/to-compiler/blame/main/src/logger.cpp
      strcsprintf(&ERROR_STR, "Invalid char [%c] at [%zu]", *endptr, CurIndex);
      strfree(&numstr);
      goto exit;
    }
    strfree(&numstr);
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

typedef struct Node {
  enum { CONSTANT_NODE, DECL_NODE, ASSIGN_NODE, REF_NODE, BINOP_NODE, FUNC_NODE, CALL_NODE, C_CALL_NODE } type;
  union {
    struct {
      double value;
    } constant;
    
    struct {
      char* name;
      struct Node *value;
    } decl;

    struct {
      char* name;
      struct Node *value;
    } assign;
    
    struct {
      char* name;
    } ref;
    
    struct {
      enum { ADD, SUB, MUL, DIV } op;
      struct Node *left;
      struct Node *right;
    } binop;
    
    struct {
      char* name;
      struct Node **body;
      int cbody;
    } func;
    
    struct {
      char* name;
    } call;
    
    struct {
      char* library;
      char* function;
      struct Node **args;
      int cargs;
    } c_call;
  };
} Node;


void ParseDecl(cstring *src, Node *pRoot) {
  
  ReadNextTok(src);
  assert(CurToken==tok_identifier);
  
  Node* pVar = malloc(sizeof(Node));
  pVar->type = DECL_NODE;
  pVar->decl.name = malloc(strlen(CurIdentifier.str) + 1);
  strcpy(pVar->decl.name, CurIdentifier.str);
  
  ReadNextTok(src);
  
  if (CurToken=='=') {
    ReadNextTok(src);
    switch (CurToken) {
    case tok_num:
      pVar->decl.value = malloc(sizeof(Node));
      pVar->decl.value->type = CONSTANT_NODE;
      pVar->decl.value->constant.value = CurNum;
      break;
      
    case tok_identifier:
      pVar->decl.value = malloc(sizeof(Node));
      pVar->decl.value->type = REF_NODE;
      pVar->decl.value->ref.name = malloc(strlen(CurIdentifier.str) + 1);
      strcpy(pVar->decl.value->ref.name, CurIdentifier.str);
      break;

    default:
      strcsprintf(&ERROR_STR, "Invalid token in declaration at [%zu]", CurIndex);
      goto exit;
    }
  } else {
    pVar->decl.value = NULL;
  }
  
  pRoot->func.body = realloc(pRoot->func.body, pRoot->func.cbody * sizeof(Node*));
  pRoot->func.body[pRoot->func.cbody] = pVar;
  pRoot->func.cbody++;
}

void ParseFunction(cstring *src, Node *pRoot) {

}

void ParsePrint(cstring *src, Node *pRoot) {

}

/* Parser */

/* Driver */

static void MainLoop(cstring *src, Node *pRoot) {
  ReadNextTok(src);
  while(1) {
    switch (CurToken) {
    case tok_eof:
      return;
    case tok_var:
      printf("Parse Variable\n");
      ParseDecl(src, pRoot);
      break;
    case tok_fun:
      printf("Parse Function\n");
      ParseFunction(src, pRoot);
      break;
    case tok_print:
      printf("Parse Print\n");
      ParsePrint(src, pRoot);
      break;
    default:
      printf("Parse oTHER\n");
      break;
    }
    ReadNextTok(src);
  }
}

/* Driver */


/* Evaluator */

void PrintAST(Node* root, int layer) {
  char prefix[layer + 1];
  for (int i = 0; i < layer; i++) {
    prefix[i] = '\t';
  }
  prefix[layer] = '\0';
  
  switch (root->type) {
  case CONSTANT_NODE:
    printf("%sCONSTANT('%lf')\n", prefix, root->constant.value);
    break;
  case DECL_NODE:
    printf("%sDECLARATION('%s'):\n", prefix, root->decl.name);
    PrintAST(root->decl.value, layer+1);
    break;
  case ASSIGN_NODE:
    printf("%sASSIGNMENT('%s')\n", prefix, root->assign.name);
    PrintAST(root->assign.value, layer+1);
    break;
  case REF_NODE:
    printf("%sREFERENCE('%s')\n", prefix, root->ref.name);
    break;
  case BINOP_NODE:
    printf("%sBINOP('%i'):\n", prefix, root->binop.op);
    PrintAST(root->binop.left, layer+1);
    PrintAST(root->binop.right, layer+1);
    break;
  case FUNC_NODE:
    printf("%sFUNCTION_DECL('%s'):\n", prefix, root->func.name);
    for (int i = 0; i < root->func.cbody; i++) {
      PrintAST(root->func.body[i], layer+1);
    }
    break;
  case CALL_NODE:
    printf("%sFUNCTION_CALL('%s')\n", prefix, root->call.name);
    break;
  default:
    printf("%sOTHER\n", prefix);
    break;
  }
}

/* Evaluator */

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
  printf("---Code---\n%s\n---Code---\n", scstr.str);

  Node *pRoot = malloc(sizeof(Node));
  pRoot->type = FUNC_NODE;
  pRoot->func.name = "main";
  pRoot->func.body = NULL;
  pRoot->func.cbody = 0;
  
  MainLoop(&scstr, pRoot);

  printf("\nTraversing AST:\n");
  PrintAST(pRoot, 0);
  printf("Exited\n");


 exit:
  int exit = 0;
  if (ERROR_STR.str!=NULL) {
    exit = 1;
    fprintf(stderr, "[ Error ]\n%s\n", ERROR_STR.str);
  }
  
  free(pRoot);
  strfree(&scstr);
  strfree(&ERROR_STR);
  strfree(&CurIdentifier);

  return exit;
}
