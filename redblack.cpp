#include "redblack.h"

RBTree::RBTree() { 
	z = new Node(black, 0, 0, 0, 0);
	z->l = z; z->r = z;
	head = new Node(black, 0, 0, 0, z); 
}

Node *
RBTree::search(int search_key)
{
	Node *x = head->r;
	while (x != z) {
		if (x->key == search_key) return x;
		x = (x->key > search_key) ? x->l : x->r;
	}
	return NULL;
}

void 
RBTree::insert(int v, int index)
{
	x = head; p = head; g = head;
	while (x != z) {
		gg = g; g = p; p = x;
		if (x->key == v) return;
		x = (x->key > v) ? x->l : x->r;
		if (x->l->b && x->r->b) this->split(v);
	}
	x = new Node(red, v, index, z, z);
	if (p->key > v) p->l = x;
	else p->r = x;
	this->split(v); head->r->b = black;
}

void
RBTree::split(int v)
{
	x->b = red; x->l->b = black; x->r->b = black;
	if (p->b) {
		g->b = red;
		if (g->key > v != p->key > v) p = this->rotate(v, g);
		x = this->rotate(v, gg);
		x->b = black;
	}
}

Node *
RBTree::rotate(int v, Node *y)
{
	Node *gc, *c;
	c = (y->key > v) ? y->l : y->r;
	if (c->key > v) {
		gc = c->l; c->l = gc->r; gc->r = c;
	}
	else {
		gc = c->r; c->r = gc->l; gc->l = c;
	}
	if (y->key > v) y->l = gc; 
	else y->r = gc;
	return gc;
}
