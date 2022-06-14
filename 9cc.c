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
bool consume(char *op){
  if(token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len)){
    return false;
  }
  token = token->next;
  return true;
}

//次のトークンが期待している記号のときには、トークンを１つ読み進めて真を返す。
//それ以外の場合にはエラーを報告する。
void expect(char *op){
  if(token->kind != TK_RESERVED ||  strlen(op) != token->len || memcmp(token->str, op, token->len)){
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
 Token *new_token(TokenKind kind, Token *cur, char *str, int len){
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

//第2引数の文字数分、2つの引数を比較し同じなら1異なる文字列なら0を返す。
bool startswith(char *p, char *q){
  return memcmp(p, q, strlen(q)) == 0;
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
    if(startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">=")){
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    //＋、ー、*、/、(、)、<、>を記号(TK_RESERVED)にトークナイズ
    if(strchr("+-*()/<>",*p)){
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    //数字の場合、数字(TK_NUM)にトークナイズ
    if(isdigit(*p)){
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10);
      cur->len = p - q;
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);
  
  //一番最初のトークンを返す
  return head.next;
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
