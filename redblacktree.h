/*
	REDBLACKTREE C VERSION
	AHN HYEYEONG
	REFER redblack.cpp, redblack.h
*/

#ifndef REDBLACKTREE_H
#define REDBLACKTREE_H
//#include <stdlib.h>

const int black = 0;
const int red = 1;

typedef struct node {
	int isblack;
	int key;
	int index;

	struct node *left_node;
	struct node *right_node;
} Node;

Node *head;
Node *z;
Node *gg;
Node *g;
Node *p;
Node *x;

void init_rbtree(void);
Node *search(int search_key);
void insert(int value, int index);
void split(int value);
Node *rotate(int value, Node *y);

Node *set_node(int isblack, int key, int index, Node *left, Node *right);

#endif
