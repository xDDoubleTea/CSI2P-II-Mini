#include <ctype.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
For the language grammar, please refer to Grammar section on the github page:
  https://github.com/lightbulb12294/CSI2P-II-Mini1#grammar
*/

#define NOT_A_CONSTANT INT_MIN
#define MAX_LENGTH 200
typedef enum {
  ASSIGN,
  ADD,
  SUB,
  MUL,
  DIV,
  REM,
  PREINC,
  PREDEC,
  POSTINC,
  POSTDEC,
  IDENTIFIER,
  CONSTANT,
  LPAR,
  RPAR,
  PLUS,
  MINUS,
  END
} Kind;
typedef enum {
  STMT,
  EXPR,
  ASSIGN_EXPR,
  ADD_EXPR,
  MUL_EXPR,
  UNARY_EXPR,
  POSTFIX_EXPR,
  PRI_EXPR
} GrammarState;
typedef struct TokenUnit {
  Kind kind;
  int val; // record the integer value or variable name
  struct TokenUnit *next;
} Token;
typedef struct ASTUnit {
  Kind kind;
  int val; // record the integer value or variable name
  struct ASTUnit *lhs, *mid, *rhs;
} AST;

/// utility interfaces

// err marco should be used when a expression error occurs.
#define err(x)                                                                 \
  {                                                                            \
    puts("Compile Error!");                                                    \
    if (DEBUG) {                                                               \
      fprintf(stderr, "Error at line: %d\n", __LINE__);                        \
      fprintf(stderr, "Error message: %s\n", x);                               \
    }                                                                          \
    exit(0);                                                                   \
  }
// You may set DEBUG=1 to debug. Remember setting back to 0 before submit.
#define DEBUG 0
// Split the input char array into token linked list.
Token *lexer(const char *in);
// Create a new token.
Token *new_token(Kind kind, int val);
// Translate a token linked list into array, return its length.
size_t token_list_to_arr(Token **head);
// Parse the token array. Return the constructed AST.
AST *parser(Token *arr, size_t len);
// Parse the token array. Return the constructed AST.
AST *parse(Token *arr, int l, int r, GrammarState S);
// Create a new AST node.
AST *new_AST(Kind kind, int val);
// Find the location of next token that fits the condition(cond). Return -1 if
// not found. Search direction from start to end.
int findNextSection(Token *arr, int start, int end, int (*cond)(Kind));
// Return 1 if kind is ASSIGN.
int condASSIGN(Kind kind);
// Return 1 if kind is ADD or SUB.
int condADD(Kind kind);
// Return 1 if kind is MUL, DIV, or REM.
int condMUL(Kind kind);
// Return 1 if kind is RPAR.
int condRPAR(Kind kind);
// Check if the AST is semantically right. This function will call err()
// automatically if check failed.
void semantic_check(AST *now);
// Generate ASM code.
int codegen(AST *root, int depth);
// Get Identifier memory address
int get_var_addr(int c);
// Free the whole AST.
void freeAST(AST *now);

/// debug interfaces

// Print token array.
void token_print(Token *in, size_t len);
// Print AST tree.
void AST_print(AST *head);
// Check if the statement does something
int check_assign_or_inc_dec(Token *content, size_t len);
int eval_constant(AST *root);

char input[MAX_LENGTH];

char reg_holds[8] = {0};

int reg = 0;

int main() {
  while (fgets(input, MAX_LENGTH, stdin) != NULL) {
    Token *content = lexer(input);
    size_t len = token_list_to_arr(&content);
    if (len == 0)
      continue;
    if (!check_assign_or_inc_dec(content, len))
      continue;
    AST *ast_root = parser(content, len);
    if (DEBUG) {
      token_print(content, len);
      AST_print(ast_root);
    }
    semantic_check(ast_root);
    reg = 0;
    for (int i = 0; i < 8; ++i)
      reg_holds[i] = 0;
    codegen(ast_root, 0);
    reg = 0;
    free(content);
    freeAST(ast_root);
  }
  return 0;
}
int eval_constant(AST *root) {
  if (!root)
    return NOT_A_CONSTANT;
  int lhs_val, rhs_val, mid_val;
  switch (root->kind) {
  case CONSTANT:
    return root->val; // Base case

  case PLUS:
    return eval_constant(root->mid);
  case MINUS:
    mid_val = eval_constant(root->mid);
    return (mid_val == NOT_A_CONSTANT) ? NOT_A_CONSTANT : -mid_val;

  // Binary operations
  case ADD:
  case SUB:
  case MUL:
  case DIV:
  case REM:
    lhs_val = eval_constant(root->lhs);
    rhs_val = eval_constant(root->rhs);

    // If either side isn't a constant, we fail
    if (lhs_val == NOT_A_CONSTANT || rhs_val == NOT_A_CONSTANT) {
      return NOT_A_CONSTANT;
    }

    // Both sides are constant! Fold them.
    if (root->kind == ADD)
      return lhs_val + rhs_val;
    if (root->kind == SUB)
      return lhs_val - rhs_val;
    if (root->kind == MUL)
      return lhs_val * rhs_val;
    if (rhs_val == 0)
      err("Compile-time division by zero.");
    if (root->kind == DIV)
      return lhs_val / rhs_val;
    if (root->kind == REM)
      return lhs_val % rhs_val;

  case LPAR:
    return eval_constant(root->mid);

  case IDENTIFIER:
  case ASSIGN:
  case PREINC:
  case PREDEC:
  case POSTINC:
  case POSTDEC:
  default:
    return NOT_A_CONSTANT;
  }
}
int check_assign_or_inc_dec(Token *content, size_t len) {
  for (int i = 0; i < len; ++i) {
    if (content[i].kind == PREDEC || content[i].kind == PREINC ||
        content[i].kind == ASSIGN)
      return 1;
  }
  return 0;
}

Token *lexer(const char *in) {
  Token *head = NULL;
  Token **now = &head;
  for (int i = 0; in[i]; i++) {
    if (isspace(in[i])) // ignore space characters
      continue;
    else if (isdigit(in[i])) {
      (*now) = new_token(CONSTANT, atoi(in + i));
      while (in[i + 1] && isdigit(in[i + 1]))
        i++;
    } else if ('x' <= in[i] && in[i] <= 'z') // variable
      (*now) = new_token(IDENTIFIER, in[i]);
    else
      switch (in[i]) {
      case '=':
        (*now) = new_token(ASSIGN, 0);
        break;
      case '+':
        if (in[i + 1] && in[i + 1] == '+') {
          i++;
          // In lexer scope, all "++" will be labeled as PREINC.
          (*now) = new_token(PREINC, 0);
        }
        // In lexer scope, all single "+" will be labeled as PLUS.
        else
          (*now) = new_token(PLUS, 0);
        break;
      case '-':
        if (in[i + 1] && in[i + 1] == '-') {
          i++;
          // In lexer scope, all "--" will be labeled as PREDEC.
          (*now) = new_token(PREDEC, 0);
        }
        // In lexer scope, all single "-" will be labeled as MINUS.
        else
          (*now) = new_token(MINUS, 0);
        break;
      case '*':
        (*now) = new_token(MUL, 0);
        break;
      case '/':
        (*now) = new_token(DIV, 0);
        break;
      case '%':
        (*now) = new_token(REM, 0);
        break;
      case '(':
        (*now) = new_token(LPAR, 0);
        break;
      case ')':
        (*now) = new_token(RPAR, 0);
        break;
      case ';':
        (*now) = new_token(END, 0);
        break;
      default:
        err("Unexpected character.");
      }
    now = &((*now)->next);
  }
  return head;
}

Token *new_token(Kind kind, int val) {
  Token *res = (Token *)malloc(sizeof(Token));
  res->kind = kind;
  res->val = val;
  res->next = NULL;
  return res;
}

size_t token_list_to_arr(Token **head) {
  size_t res;
  Token *now = (*head), *del;
  for (res = 0; now != NULL; res++)
    now = now->next;
  now = (*head);
  if (res != 0)
    (*head) = (Token *)malloc(sizeof(Token) * res);
  for (int i = 0; i < res; i++) {
    (*head)[i] = (*now);
    del = now;
    now = now->next;
    free(del);
  }
  return res;
}

AST *parser(Token *arr, size_t len) {
  for (int i = 1; i < len; i++) {
    // correctly identify "ADD" and "SUB"
    if (arr[i].kind == PLUS || arr[i].kind == MINUS) {
      switch (arr[i - 1].kind) {
      case PREINC:
      case PREDEC:
      case IDENTIFIER:
      case CONSTANT:
      case RPAR:
        arr[i].kind = arr[i].kind - PLUS + ADD;
      default:
        break;
      }
    }
  }
  return parse(arr, 0, len - 1, STMT);
}

AST *parse(Token *arr, int l, int r, GrammarState S) {
  AST *now = NULL;
  if (l > r)
    err("Unexpected parsing range.");
  int nxt;
  switch (S) {
  case STMT:
    if (l == r && arr[l].kind == END)
      return NULL;
    else if (arr[r].kind == END)
      return parse(arr, l, r - 1, EXPR);
    else
      err("Expected \';\' at the end of line.");
  case EXPR:
    return parse(arr, l, r, ASSIGN_EXPR);
  case ASSIGN_EXPR:
    if ((nxt = findNextSection(arr, l, r, condASSIGN)) != -1) {
      now = new_AST(arr[nxt].kind, 0);
      now->lhs = parse(arr, l, nxt - 1, UNARY_EXPR);
      now->rhs = parse(arr, nxt + 1, r, ASSIGN_EXPR);
      return now;
    }
    return parse(arr, l, r, ADD_EXPR);
  case ADD_EXPR:
    if ((nxt = findNextSection(arr, r, l, condADD)) != -1) {
      now = new_AST(arr[nxt].kind, 0);
      now->lhs = parse(arr, l, nxt - 1, ADD_EXPR);
      now->rhs = parse(arr, nxt + 1, r, MUL_EXPR);
      return now;
    }
    return parse(arr, l, r, MUL_EXPR);
  case MUL_EXPR:
    if ((nxt = findNextSection(arr, r, l, condMUL)) != -1) {
      now = new_AST(arr[nxt].kind, 0);
      now->lhs = parse(arr, l, nxt - 1, MUL_EXPR);
      now->rhs = parse(arr, nxt + 1, r, UNARY_EXPR);
      return now;
    }
    return parse(arr, l, r, UNARY_EXPR);
    // TODO: Implement MUL_EXPR.
    // hint: Take ADD_EXPR as reference.
  case UNARY_EXPR:
    if (arr[l].kind == PLUS || arr[l].kind == MINUS || arr[l].kind == PREINC ||
        arr[l].kind == PREDEC) {
      now = new_AST(arr[l].kind, 0);
      now->mid = parse(arr, l + 1, r, UNARY_EXPR);
      return now;
    }
    return parse(arr, l, r, POSTFIX_EXPR);
    // TODO: Implement UNARY_EXPR.
    // hint: Take POSTFIX_EXPR as reference.
  case POSTFIX_EXPR:
    if (arr[r].kind == PREINC || arr[r].kind == PREDEC) {
      // translate "PREINC", "PREDEC" into "POSTINC", "POSTDEC"
      now = new_AST(arr[r].kind - PREINC + POSTINC, 0);
      now->mid = parse(arr, l, r - 1, POSTFIX_EXPR);
      return now;
    }
    return parse(arr, l, r, PRI_EXPR);
  case PRI_EXPR:
    if (findNextSection(arr, l, r, condRPAR) == r) {
      now = new_AST(LPAR, 0);
      now->mid = parse(arr, l + 1, r - 1, EXPR);
      return now;
    }
    if (l == r) {
      if (arr[l].kind == IDENTIFIER || arr[l].kind == CONSTANT)
        return new_AST(arr[l].kind, arr[l].val);
      err("Unexpected token during parsing.");
    }
    err("No token left for parsing.");
  default:
    err("Unexpected grammar state.");
  }
}

AST *new_AST(Kind kind, int val) {
  AST *res = (AST *)malloc(sizeof(AST));
  res->kind = kind;
  res->val = val;
  res->lhs = res->mid = res->rhs = NULL;
  return res;
}

int findNextSection(Token *arr, int start, int end, int (*cond)(Kind)) {
  int par = 0;
  int d = (start < end) ? 1 : -1;
  for (int i = start; (start < end) ? (i <= end) : (i >= end); i += d) {
    if (arr[i].kind == LPAR)
      par++;
    if (arr[i].kind == RPAR)
      par--;
    if (par == 0 && cond(arr[i].kind) == 1)
      return i;
  }
  return -1;
}

int condASSIGN(Kind kind) { return kind == ASSIGN; }

int condADD(Kind kind) { return kind == ADD || kind == SUB; }

int condMUL(Kind kind) { return kind == MUL || kind == DIV || kind == REM; }

int condRPAR(Kind kind) { return kind == RPAR; }

void semantic_check(AST *now) {
  if (now == NULL)
    return;
  // Left operand of '=' must be an identifier or identifier with one or more
  // parentheses.
  if (now->kind == ASSIGN) {
    AST *tmp = now->lhs;
    while (tmp->kind == LPAR)
      tmp = tmp->mid;
    if (tmp->kind != IDENTIFIER)
      err("Lvalue is required as left operand of assignment.");
  }
  // Operand of INC/DEC must be an identifier or identifier with one or more
  // parentheses.
  if (now->kind == POSTDEC || now->kind == POSTINC) {
    while (now->mid->kind == LPAR) {
      now = now->mid;
    }
    if (now->mid->kind != IDENTIFIER)
      err("Operand of increment/decrement must be an identifier.");
  } else if (now->kind == PREDEC || now->kind == PREINC) {
    while (now->mid->kind == LPAR) {
      now = now->mid;
    }
    if (now->mid->kind != IDENTIFIER)
      err("Operand of increment/decrement must be an identifier.");
  }
  semantic_check(now->lhs);
  semantic_check(now->mid);
  semantic_check(now->rhs);
  // TODO: Implement the remaining semantic_check code.
  // hint: Follow the instruction above and ASSIGN-part code to implement.
  // hint: Semantic of each node needs to be checked recursively (from the
  // current node to lhs/mid/rhs node).
}

int get_var_addr(int c) {
  if (c == 'x')
    return 0;
  if (c == 'y')
    return 4;
  if (c == 'z')
    return 8;
  err("Unexcepted identifier");
  return -1;
}

int get_iden_reg(int iden) { return iden - 'x'; }

void invalidate_reg(int iden, int reg_num) {
  for (int i = 0; i < 8; ++i) {
    if (reg_holds[i] == iden && i != reg_num) {
      reg_holds[i] = 0;
    }
  }
}

int codegen(AST *root, int depth) {
  if (!root) {
    return -1;
  }
  int const_val = eval_constant(root);
  if (const_val != NOT_A_CONSTANT) {
    // This entire branch folds to a single value
    // Generate code to just load that value.
    int reg_dest = reg++;
    if (const_val >= 0)
      printf("add r%d 0 %d\n", reg_dest, const_val);
    else
      printf("sub r%d 0 %d\n", reg_dest, -const_val);
    return reg_dest;
  }
  int reg_lhs, reg_rhs, reg_dest, reg_mid;
  int var_addr;
  int reg_temp;
  AST *lhs_var;
  // printf("reg = %d\n", reg);
  // printf("kind = %d\n", root->kind);
  switch (root->kind) {
  case CONSTANT:
    reg_dest = reg++;
    if (root->val >= 0) {
      printf("add r%d 0 %d\n", reg_dest, root->val);
    } else {
      printf("sub r%d 0 %d\n", reg_dest, -root->val);
    }
    return reg_dest;
  case IDENTIFIER:
    reg_dest = reg++;
    for (int i = 0; i < 8; ++i) {
      if (reg_holds[i] == root->val) {
        printf("add r%d r%d 0\n", reg_dest, i);
        return reg_dest;
      }
    }
    var_addr = get_var_addr(root->val);
    printf("load r%d [%d]\n", reg_dest, get_var_addr(root->val));
    if (reg_dest < 8) {
      reg_holds[reg_dest] = root->val;
    }

    return reg_dest;
  case ASSIGN:
    reg_rhs = codegen(root->rhs, depth + 1);

    lhs_var = root->lhs;
    while (lhs_var->kind == LPAR) { // Handle cases like (((x))) = ...
      lhs_var = lhs_var->mid;
    }
    var_addr = get_var_addr(lhs_var->val);

    printf("store [%d] r%d\n", var_addr, reg_rhs);
    if (reg_rhs >= 0 && reg_rhs < 8)
      reg_holds[reg_rhs] = lhs_var->val;
    invalidate_reg(lhs_var->val, reg_rhs);
    // printf("reg_lhs = %d\n", reg_lhs);
    // printf("reg_rhs = %d\n", reg_rhs);
    return reg_rhs;
  case ADD:
  case SUB:
  case MUL:
    if ((root->lhs->kind == CONSTANT && root->lhs->val == 0) ||
        (root->rhs->kind == CONSTANT && root->rhs->val == 0)) {
      reg_dest = reg++;
      printf("add r%d 0 0\n", reg_dest);
      if (reg_dest >= 0 && reg_dest < 8)
        reg_holds[reg_dest] = 0;
      return reg_dest;
    }
  case DIV:
  case REM:
    reg_lhs = codegen(root->lhs, depth + 1);
    reg_rhs = codegen(root->rhs, depth + 1);
    if (root->kind == ADD)
      printf("add r%d r%d r%d\n", reg_lhs, reg_lhs, reg_rhs);
    else if (root->kind == SUB)
      printf("sub r%d r%d r%d\n", reg_lhs, reg_lhs, reg_rhs);
    else if (root->kind == MUL)
      printf("mul r%d r%d r%d\n", reg_lhs, reg_lhs, reg_rhs);
    else if (root->kind == DIV)
      printf("div r%d r%d r%d\n", reg_lhs, reg_lhs, reg_rhs);
    else
      printf("rem r%d r%d r%d\n", reg_lhs, reg_lhs, reg_rhs);
    reg--;
    if (reg_lhs >= 0 && reg_lhs < 8) {
      reg_holds[reg_lhs] = 0;
      reg_holds[reg_rhs] = 0;
    }
    return reg_lhs;
  case PREINC:
    lhs_var = root->mid;
    while (lhs_var->kind == LPAR)
      lhs_var = lhs_var->mid;
    var_addr = get_var_addr(lhs_var->val);
    reg_mid = reg++;
    printf("load r%d [%d]\n", reg_mid, var_addr);
    printf("add r%d r%d 1\n", reg_mid, reg_mid);
    printf("store [%d] r%d\n", var_addr, reg_mid);
    if (reg_mid >= 0 && reg_mid < 8)
      reg_holds[reg_mid] = lhs_var->val;
    invalidate_reg(lhs_var->val, reg_mid);
    return reg_mid;
  case PREDEC:
    lhs_var = root->mid;
    while (lhs_var->kind == LPAR)
      lhs_var = lhs_var->mid;
    var_addr = get_var_addr(lhs_var->val);
    reg_mid = reg++;
    printf("load r%d [%d]\n", reg_mid, var_addr);
    printf("sub r%d r%d 1\n", reg_mid, reg_mid);
    printf("store [%d] r%d\n", var_addr, reg_mid);
    if (reg_mid >= 0 && reg_mid < 8)
      reg_holds[reg_mid] = lhs_var->val;
    invalidate_reg(lhs_var->val, reg_mid);
    return reg_mid;
  case POSTINC:
    lhs_var = root->mid;
    while (lhs_var->kind == LPAR)
      lhs_var = lhs_var->mid;
    var_addr = get_var_addr(lhs_var->val);

    reg_dest = reg++;
    printf("load r%d [%d]\n", reg_dest, var_addr);

    reg_temp = reg++;
    printf("add r%d r%d 1\n", reg_temp, reg_dest); // reg_temp holds new value

    printf("store [%d] r%d\n", var_addr, reg_temp);

    reg--; // Frees reg_temp
    if (reg_dest >= 0 && reg_dest < 8)
      reg_holds[reg_dest] = lhs_var->val;
    invalidate_reg(lhs_var->val, reg_dest);
    return reg_dest;
  case POSTDEC:
    lhs_var = root->mid;
    while (lhs_var->kind == LPAR)
      lhs_var = lhs_var->mid;
    var_addr = get_var_addr(lhs_var->val);

    reg_dest = reg++;
    printf("load r%d [%d]\n", reg_dest, var_addr);

    reg_temp = reg++;
    printf("sub r%d r%d 1\n", reg_temp, reg_dest); // reg_temp holds new value

    printf("store [%d] r%d\n", var_addr, reg_temp);

    reg--;

    if (reg_dest >= 0 && reg_dest < 8)
      reg_holds[reg_dest] = lhs_var->val;
    invalidate_reg(lhs_var->val, reg_dest);
    return reg_dest;
  case LPAR:
    return codegen(root->mid, depth + 1);
  case RPAR:
    return codegen(root->mid, depth + 1);
  case PLUS:
    return codegen(root->mid, depth + 1);
  case MINUS:
    reg_mid = codegen(root->mid, depth + 1);
    printf("sub r%d 0 r%d\n", reg_mid, reg_mid);
    if (reg_mid >= 0 && reg_mid < 8)
      reg_holds[reg_mid] = 0;
    return reg_mid;
  case END:
    return 0;
  default:
    err("Unexpected AST node during codegen.");
    return -1;
  }
  // TODO: Implement your codegen in your own way.
  // You may modify the function parameter or the return type, even the whole
  // structure as you wish.
}

void freeAST(AST *now) {
  if (now == NULL)
    return;
  freeAST(now->lhs);
  freeAST(now->mid);
  freeAST(now->rhs);
  free(now);
}

void token_print(Token *in, size_t len) {
  const static char KindName[][20] = {
      "Assign", "Add",  "Sub",  "Mul",   "Div",        "Rem",
      "Inc",    "Dec",  "Inc",  "Dec",   "Identifier", "Constant",
      "LPar",   "RPar", "Plus", "Minus", "End"};
  const static char KindSymbol[][20] = {
      "'='",    "'+'",    "'-'", "'*'", "'/'", "'%'", "\"++\"", "\"--\"",
      "\"++\"", "\"--\"", "",    "",    "'('", "')'", "'+'",    "'-'"};
  const static char format_str[] = "<Index = %3d>: %-10s, %-6s = %s\n";
  const static char format_int[] = "<Index = %3d>: %-10s, %-6s = %d\n";
  for (int i = 0; i < len; i++) {
    switch (in[i].kind) {
    case LPAR:
    case RPAR:
    case PREINC:
    case PREDEC:
    case ADD:
    case SUB:
    case MUL:
    case DIV:
    case REM:
    case ASSIGN:
    case PLUS:
    case MINUS:
      fprintf(stderr, format_str, i, KindName[in[i].kind], "symbol",
              KindSymbol[in[i].kind]);
      break;
    case CONSTANT:
      fprintf(stderr, format_int, i, KindName[in[i].kind], "value", in[i].val);
      break;
    case IDENTIFIER:
      fprintf(stderr, format_str, i, KindName[in[i].kind], "name",
              (char *)(&(in[i].val)));
      break;
    case END:
      fprintf(stderr, "<Index = %3d>: %-10s\n", i, KindName[in[i].kind]);
      break;
    default:
      fputs("=== unknown token ===", stderr);
    }
  }
}

void AST_print(AST *head) {
  static char indent_str[MAX_LENGTH] = "  ";
  static int indent = 2;
  const static char KindName[][20] = {
      "Assign",      "Add",         "Sub",        "Mul",
      "Div",         "Rem",         "PreInc",     "PreDec",
      "PostInc",     "PostDec",     "Identifier", "Constant",
      "Parentheses", "Parentheses", "Plus",       "Minus"};
  const static char format[] = "%s\n";
  const static char format_str[] = "%s, <%s = %s>\n";
  const static char format_val[] = "%s, <%s = %d>\n";
  if (head == NULL)
    return;
  char *indent_now = indent_str + indent;
  indent_str[indent - 1] = '-';
  fprintf(stderr, "%s", indent_str);
  indent_str[indent - 1] = ' ';
  if (indent_str[indent - 2] == '`')
    indent_str[indent - 2] = ' ';
  switch (head->kind) {
  case ASSIGN:
  case ADD:
  case SUB:
  case MUL:
  case DIV:
  case REM:
  case PREINC:
  case PREDEC:
  case POSTINC:
  case POSTDEC:
  case LPAR:
  case RPAR:
  case PLUS:
  case MINUS:
    fprintf(stderr, format, KindName[head->kind]);
    break;
  case IDENTIFIER:
    fprintf(stderr, format_str, KindName[head->kind], "name",
            (char *)&(head->val));
    break;
  case CONSTANT:
    fprintf(stderr, format_val, KindName[head->kind], "value", head->val);
    break;
  default:
    fputs("=== unknown AST type ===", stderr);
  }
  indent += 2;
  strcpy(indent_now, "| ");
  AST_print(head->lhs);
  strcpy(indent_now, "` ");
  AST_print(head->mid);
  AST_print(head->rhs);
  indent -= 2;
  (*indent_now) = '\0';
}
