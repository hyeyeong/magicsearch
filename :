#ifndef REDBLACK_H
#define REDBLACK_H
#include <stdlib.h>

const int black = 0;
const int red = 1;

class Node {
public:
	Node(int bb, int k, int i, Node *ll, Node *rr)	{ 
		b = bb, key = k; index = i, l = ll, r = rr; 
	};    
	int b;
	int key;
	int index;
	Node *l, *r;
};

class RBTree {
public:
	RBTree();
    Node *search(int search_key);
    void insert(int v, int index);
	void insert_p(struct person *ps);
	void split(int v);
	Node *rotate(int v, Node *y);
private:
	Node *head, *z, *gg, *g, *p, *x;
};
#endif

/* 인천대 버전 트리 두가지...
#include <stdio.h>
#include <vector>
#include <cstring>
#include <ctime>

using std::vector;

enum Color { RED, BLACK };

struct RBNode 
{
	Color color;
	char key[7];
	RBNode* left; 
	RBNode* right;
	RBNode(Color col, char* key, RBNode* leftChild, RBNode* rightChild)
		: color(col), left(leftChild), right(rightChild)
	{
		if(key != NULL)	strcpy(this->key, key);
	}	
};

class RedBlackTree
{
public:
	RedBlackTree();
	char* Search(char* searchKey);
	void Insert(char* value);
	void Release();
private:
	RBNode* head;
	RBNode* leaf;
private:
	void Split(char* value, RBNode* gg, RBNode* g, RBNode* p, RBNode* n);
	RBNode* Rotate(char* value, RBNode* gran);
	void Release(RBNode* n);
};

RedBlackTree::RedBlackTree()
{
	leaf = new RBNode(BLACK, 0, 0, 0);
	leaf->left = leaf;
	leaf->right = leaf;
	head = new RBNode(BLACK, 0, 0, leaf);
}

char* RedBlackTree::Search(char* searchKey)
{
	RBNode* x = head->right;
	
	while(x != leaf)
	{
		int result = strcmp(x->key, searchKey);
		if(result == 0)
		{
			return x->key;
		}
		x = (result > 0) ? x->left : x->right;
	}
	return NULL;
}

void RedBlackTree::Insert(char* value)
{
	RBNode* x = head;
	RBNode* p = head; //parent
	RBNode* g = head; //grandparent
	RBNode* gg; //grandgrandparent
	
	while(x != leaf)
	{
		gg = g;
		g = p;
		p = x;

		int result = strcmp(x->key, value);
		if(result == 0) return;
		x = (result > 0) ? x->left : x->right;
		if(x->left->color == RED && x->right->color == RED)
		{
			Split(value, gg, g, p, x);
		}
	}

	x = new RBNode(RED, value, leaf, leaf);
	if(strcmp(p->key, value) > 0) p->left = x;
	else p->right = x;

	Split(value, gg, g, p, x);
	head->right->color = BLACK;
}

void RedBlackTree::Split(char* value, RBNode* gg, RBNode* g, RBNode* p, RBNode* n)
{
	n->color = RED;
	n->left->color = BLACK;
	n->right->color = BLACK;

	if(p->color == RED)
	{
		g->color = RED;
		if(strcmp(g->key, value) != strcmp(p->key, value))
		{
			p = Rotate(value, g);
		}
		n = Rotate(value, gg);
		n->color = BLACK;
	}
}

RBNode* RedBlackTree::Rotate(char* value, RBNode* gran)
{
	RBNode* gc; //grandchild
	RBNode* c; //child

	int result = strcmp(gran->key, value);
	c = (result > 0) ? gran->left : gran->right;
	
	if(strcmp(c->key, value) > 0)
	{
		gc = c->left;
		c->left = gc->right;
		gc->right = c;
	}
	else
	{
		gc = c->right;
		c->right = gc->left;
		gc->left = c;
	}

	if(result > 0) gran->left = gc;
	else gran->right = gc;

	return gc;
}

void RedBlackTree::Release()
{
	Release(head->right);
	head->right = leaf;
}

void RedBlackTree::Release(RBNode* n)
{
	if(n != leaf)
	{
		Release(n->left);
		Release(n->right);
		delete n;
	}
}


struct AVLNode
{
	char key[7];
	char bf; //balance factor
	AVLNode* left;
	AVLNode* right;
};

class AVLTree
{
public:
	char* Search(char* searchKey);
	void Insert(char* value);
	void Release();
public:
	AVLTree()
		:root(NULL)
	{
	}
private:
	AVLNode* root;

	void Release(AVLNode* n);
};


char* AVLTree::Search(char* searchKey)
{
	AVLNode* x = root;
	
	while(x != NULL)
	{
		int result = strcmp(x->key, searchKey);
		if(result == 0)
		{
			return x->key;
		}
		x = (result > 0) ? x->left : x->right;
	}
	return NULL;
}

void AVLTree::Insert(char* value)
{
	if(root == NULL)
	{
		AVLNode* x = new AVLNode();
		strcpy(x->key, value);
		root = x;
		root->bf = 0;
		root->left = root->right = NULL;
		return;
	}

	AVLNode* fnz; // First NonZero node from insertion point
	AVLNode* pfnz; // parent of fnz
	AVLNode* cfnz; // child of fnz
	AVLNode* x;
	AVLNode* p; // parent of x
	
	pfnz = NULL;
	fnz = x = root;
	p = NULL;
	
	while( x != NULL)
	{
		if(x->bf != 0)
		{
			fnz = x;
			pfnz = p;
		}

		int result = strcmp(value, x->key);
		if(result < 0) 
		{
			p = x;
			x = x->left;
		}
		else if (result > 0)
		{
			p = x;
			x = x->right;
		}
		else
		{
			return;
		}
	}
	
	x = new AVLNode();
	strcpy(x->key, value);
	x->left = x->right = NULL;
	x->bf = 0;
	if(strcmp(value, p->key) < 0)
	{
		p->left = x;
	}
	else
	{
		p->right = x;
	}

	int d; // balance factor that will be added
	AVLNode* n;
	if(strcmp(value, fnz->key) > 0)
	{
		n = fnz->right;
		cfnz = n;
		d = -1;
	}
	else
	{
		n = fnz->left;
		cfnz = n;	
		d = 1;
	}

	while(n != x)
	{
		if(strcmp(value, n->key) > 0)
		{
			n->bf = -1;
			n = n->right;
		}
		else
		{
			n->bf = 1;
			n = n->left;
		}
	}
	
	if(fnz->bf == 0 || (fnz->bf + d) == 0)
	{
		fnz->bf += d;
		return;
	}

	if(d == 1)
	{
		if(cfnz->bf == 1)
		{
			fnz->left = cfnz->right;
			cfnz->right = fnz;
			fnz->bf = cfnz->bf = 0;
		}
		else
		{
			AVLNode* gcfnz = cfnz->right;//grandchild of fnz
			cfnz->right = gcfnz->left;
			fnz->left = gcfnz->right;
			gcfnz->left = cfnz;
			gcfnz->right = fnz;

			switch(gcfnz->bf)
			{
			case 1 :
				fnz->bf = -1;
				cfnz->bf = 0;
				break;
			case -1 :
				fnz->bf = 0;
				cfnz->bf = 1;
				break;
			case 0 :
				fnz->bf = cfnz->bf = 0;
				break;
			}
		}
	}
	else
	{
		if(cfnz->bf == -1)
		{
			fnz->right = cfnz->left;
			cfnz->left = fnz;
			fnz->bf = cfnz->bf = 0;
		}
		else
		{
			AVLNode* gcfnz = cfnz->left;
			cfnz->left = gcfnz->right;
			fnz->right = gcfnz->left;
			gcfnz->left = fnz;
			gcfnz->right = cfnz;

			switch(gcfnz->bf)
			{
			case 1 :
				fnz->bf = 0;
				cfnz->bf = -1;
				break;
			case -1 :
				fnz->bf = 1;
				cfnz->bf = 0;
				break;
			case 0 :
				fnz->bf = cfnz->bf = 0;
				break;
			}
		}
	}

	if(pfnz == NULL) root = cfnz;
	else if(fnz == pfnz->left) pfnz->left = cfnz;
	else if(fnz == pfnz->right) pfnz->right = cfnz;
}

void AVLTree::Release()
{
	Release(root);
	root = NULL;
}

void AVLTree::Release(AVLNode* n)
{
	if(n != NULL)
	{
		Release(n->left);
		Release(n->right);
		delete n;
	}
}

void InitStrArray(char* path, vector<char*>& array)
{
	FILE* f;

	f = fopen(path, "r");
	if(f == NULL)
	{
		printf("Error occured when the program is trying to open a file");
	}
	fseek(f, 0L, SEEK_END);
	long pos = ftell(f);
	fseek(f, 0L, SEEK_SET);
	
	long len = 7;
	char* str = new char[len];
	while(fgets(str, len, f) != NULL)
	{
		array.push_back(str);
		str = new char[len];
	}
	fclose(f);
	delete[] str;
}

RedBlackTree srb, rrb;
AVLTree savl, ravl;
vector<char*> sorted, random, keys;

void ReleaseArray(vector<char*>& array)
{
	for(int i = 0; i < array.size(); i++)
	{
		delete[] array[i];
	}	
}

double avg(double* a, int s)
{
	double sum = 0;
	for(int i = 0; i < s; i++)
	{
		sum += a[i];
	}
	return sum / s;
}

void MeasureInsertTime(char* title, vector<char*>& array, RedBlackTree* rb, AVLTree* avl)
{
	const int tries = 100;
	double data[2][100];

	for(int j = 0; j < tries; j++)
	{
		double start = clock();
		int size = array.size();
		for(int i = 0; i < size; i++)
		{
			rb->Insert(array[i]);
		}
		double end1 = clock();
		for(int i = 0; i < size; i++)
		{
			avl->Insert(array[i]);
		}
		double end2 = clock();
		data[0][j] = end1 - start;
		data[1][j] = end2 - end1;

		rb->Release();
		avl->Release();
	}

	puts(title);
	printf("Red Black Tree : %lf \n", avg(data[0], tries));
	printf("AVL Tree : %lf \n", avg(data[1], tries));
}

void MeasureSearchTime(char* title, vector<char*>& array, RedBlackTree* rb, AVLTree* avl)
{
	double data[2][100];

	for(int j = 0; j < 100; j++)
	{
		double start = clock();
		int size = array.size();
		for(int i = 0; i < size; i++)
		{
			rb->Search(array[i]);
		}
		double end1 = clock();
		for(int i = 0; i < size; i++)
		{
			rb->Search(array[i]);
		}
		double end2 = clock();
		
		data[0][j] = end1 - start;
		data[1][j] = end2 - end1;
	}

	puts(title);
	printf("Red Black Tree : %lf \n", avg(data[0], 100));
	printf("AVL Tree : %lf \n", avg(data[1], 100));
}

void test1()
{
	for(int i = 0; i < 10; i++)
	{
		printf("%s, %s", sorted[i], random[i]);
	}
}

int main()
{
	char s[] = "sorted_dict";
	char r[] = "dict";
	char k[] = "search_key";

	puts("Initializing Input Array");
	InitStrArray(s, sorted);
	InitStrArray(r, random);
	InitStrArray(k, keys);

	//test1();
	char t1[] = "Insert Time - Sorted";
	char t2[] = "Insert Time - Random";
	MeasureInsertTime(t1, sorted, &srb, &savl);
	MeasureInsertTime(t2, random, &rrb, &ravl);
	
	char t3[] = "Search Time - Sorted";
	char t4[] = "Search Time - Random";
	MeasureSearchTime(t3, keys, &srb, &savl);
	MeasureSearchTime(t4, keys, &rrb, &ravl);
	
	puts("Releasing Input Array");
	ReleaseArray(random);
	ReleaseArray(sorted);

	srb.Release();
	rrb.Release();
	savl.Release();
	ravl.Release();
	return 0;
}

*/
