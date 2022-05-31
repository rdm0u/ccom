#include "9cc.h"



//現在着目しているトークン
Token *token;

//エラーを報告するための関数
//printfと同じ引数を取る
/*
void error(char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}
*/

//入力プログラム
char *user_input;

//エラー箇所を報告する
void error_at(char *loc, char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); //pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

//次のトークンが期待している記号ときには、トークンを一つ読み進めて真を返す。
//それ以外の場合には偽を返す。
bool consume(char op){
  if(token->kind != TK_RESERVED || token->str[0] != op){
    return false;
  }
  token = token->next;
  return true;
}

//次のトークンが期待している記号のときには、トークンを１つ読み進めて真を返す。
//それ以外の場合にはエラーを報告する。
void expect(char op){
  if(token->kind != TK_RESERVED || token->str[0] != op){
    error_at(token->str, "'%c'ではありません", op);
  }
  token = token->next;
}


//次のトークンが数値の場合、トークンを１つ読み進めてその数値を返す。
//それ以外の場合にはエラーを報告する
int expect_number(){
  if(token->kind != TK_NUM){
    error_at(token->str, "数ではありません");
  }
  int val = token->val;
  token= token->next;
  return val;
}

bool at_eof(){
  return token->kind == TK_EOF;
}

//トークナイザ
//新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str){
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

//入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p){
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while(*p){
    //空白文字をスキップ
    if(isspace(*p)){
      p++;
      continue;
    }

    //＋またはーを記号(TK_RESERVED)にトークナイズ
    if(strchr("+-*()/",*p)){
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    //数字の場合、数字(TK_NUM)にトークナイズ
    if(isdigit(*p)){
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p);
  
  //一番最初のトークンを返す
  return head.next;
}

//構文木
//新しいノードを作成する関数
Node *new_node(NodeKind kind, Node *lhs, Node* rhs){
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val){
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

//パーサ
Node *expr(){
  Node *node = mul();

  for(;;){
    if(consume('+')){
      node = new_node(ND_ADD, node, mul());
    }else if(consume('-')){
      node = new_node(ND_SUB, node, mul());
    }else{
      return node;
    }
  }
}

Node *mul(){
  Node *node = primary();

  for(;;){
    if(consume('*')){
      node = new_node(ND_MUL, node, primary());
    }else if(consume('/')){
      node = new_node(ND_DIV, node, primary());
    }else{
      return node;
    }
  }
}


Node *primary(){
  //次のトークンが”(”なら”(”expr”)”のはず
  if(consume('(')){
    Node *node = expr();
    expect(')');
    return node;
  }

  //そうでなければ数値のはず
  return new_node_num(expect_number());
}

//抽象木構文をスタックマシンに変換
void gen(Node *node){
  if(node->kind == ND_NUM){
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch(node->kind){
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;

  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;

  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;

  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  }

  printf("  push rax\n");

  
}


int main(int argc, char ** argv){
  if(argc != 2){
    fprintf(stderr, "引数の個数が正しく有ありません\n");
    return 1;
  }

  //入力プログラム
  user_input = argv[1];

  //トークナイズしてパースする
  token = tokenize(argv[1]);
  Node *node = expr();


  //アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");


  //抽象構文木を下りながらアセンブリのコード生成
  gen(node);

  //スタックトップに式全体の値が残っているはずなので
  //それをRAXにロードして関数からの返り値とする
  printf("  pop rax\n");
  
  printf("  ret\n");
  return 0;
}
