/*
	REDBLACKTREE C VERSION
	AHN HYEYEONG
	REFER redblack.cpp, redblack.h
*/

//#include <stdio.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include "redblacktree.h"

/*
int main() {
	Node *searched;

	init_rbtree();
	insert(3, 1);
	insert(5, 2);
	searched = search(3);
	if(searched != NULL)
		printf("searched->index: %d\n", searched->index);

	return 0;
}
*/
void init_rbtree(void) {
	z = set_node(black, 0, 0, 0, 0);
	z->left_node = z;
	z->right_node = z;

	head = set_node(black, 0, 0, 0, z);
}

Node *search(int search_key) {
	Node *x = head->right_node;

	while (x != z) {
		if (x->key == search_key)
			return x;
		x = (x->key > search_key) ? x->left_node : x->right_node;
	}
	return NULL;
}

void insert(int value, int index) {
	x = head;
	p = head;
	g = head;

	while (x != z) {
		gg = g;
		g = p;
		p = x;

		if (x->key == value)
			return;
		x = (x->key > value) ? x->left_node : x->right_node;
		if (x->left_node->isblack && x->right_node->isblack) split(value);
	}

	x = set_node(red, value, index, z, z);
	if (p->key > value)
		p->left_node = x;
	else
		p->right_node = x;
	split(value);
	head->right_node->isblack = black;
}

void split(int value) {
	x->isblack = red;
	x->left_node->isblack = black;
	x->right_node->isblack = black;

	if (p->isblack) {
		g->isblack = red;
		if (g->key > value != p->key > value)
			p = rotate(value, g);
		x = rotate(value, gg);
		x->isblack = black;
	}
}

Node *rotate(int value, Node *y) {
	Node *gc;
	Node *c;

	c = (y->key > value) ? y->left_node : y->right_node;

	if (c->key > value) {
		gc = c->left_node;
		c->left_node = gc->right_node;
		gc->right_node = c;
	}
	else {
		gc = c->right_node;
		c->right_node = gc->left_node;
		gc->left_node = c;
	}

	if (y->key > value)
		y->left_node = gc;
	else
		y->right_node = gc;

	return gc;
}

Node *set_node(int isblack, int key, int index, Node *left, Node *right) {
	Node *temp = (Node *)vmalloc(sizeof(struct node));
	temp->isblack = isblack;
	temp->key = key;
	temp->index = index;
	temp->left_node = left;
	temp->right_node = right;

	return temp;
}
