#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

//プロトタイプ宣言

//エラーを報告するための関数
void error_at(char *loc, char *fmt, ...); //エラー箇所を報告する
bool consume(char op); //次のトークンが期待している記号ときには、トークンを一つ読み進めて真を返す。
void expect(char op); //次のトークンが期待している記号のときには、トークンを１つ読み進めて真を返す。
int expect_number(); //次のトークンが数値の場合、トークンを１つ読み進めてその数値を返す。
bool at_eof();


//トークン
//トークンの種類
typedef enum {
	      TK_RESERVED, //記号
	      TK_NUM,      //整数
	      TK_EOF,      //入力の終わりを表すトークン
}TokenKind;

//トークン型
typedef struct Token Token;

struct Token{
  TokenKind kind; //トークンの型
  Token *next;    //次の入力トークン
  int val;        //kindがTK_NUMの場合、その数値
  char *str;      //トークン文字列
};
Token *new_token(TokenKind kind, Token *cur, char *str); //新しいトークンを作成してcurにつなげる
Token *tokenize(char *p); //入力文字列pをトークナイズしてそれを返す


//構文木

//抽象構文木のノードの種類
typedef enum{
	     ND_ADD, // +
	     ND_SUB, // -
	     ND_MUL, // *
	     ND_DIV, // /
	     ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

//抽象構文木のノードの型
struct Node{
  NodeKind kind; //ノードの型
  Node *lhs;     //左辺 left-hand side
  Node *rhs;     //右辺 right-hand side
  int val;       //kindがND_NUMの場合のみ使う
};

Node *new_node(NodeKind kind, Node *lhs, Node* rhs); //新しいノードを作成する関数
Node *new_node_num(int val);

//パーサ
Node *expr();
Node *mul();
Node *unary();
Node *primary();

//抽象木構文をスタックマシンに変換
void gen(Node *node);
