/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
*/
#include <asm/uaccess.h>
#include "main.h"
#include "sorting.h"

static long
partition2(int A[], int p, int r)
{
	int x;
	int i;
	int j;
	int temp;
	int *tp;
	char tmpStr[3];
	
	x = A[r];		// ���ذ�
	i = p - 1;		// i�� ���ذ����� �������ҵ��� ���������� ������ ���� ������� �ʴ� 0�ε����� ��ġ�Ѵ�.
	for (j = p; j < r; j++) {	// j�� ���� �������� �ʴ� ���ҵ��� ���ʷ� �˻��� ������.
		if(A[j]<=x)	{	// ���ذ��� ��
			temp = A[++i];	// ���� �� i�� 1�� �����Ͽ� ���ڸ��� ����� i�� ��ġ�� �ε����� ���� ��ȯ�Ѵ�.
			A[i] = A[j];
			A[j] = temp;

			tp = searched[i]; //������ i�� 1�� �����Ͽ� ���ڸ��� ����� i�� ��ġ�� �ε����� ���� ��ȯ�Ѵ�.
			searched[i] = searched[j];
			searched[j] = tp;

			strcpy(tmpStr, (const char *)eo_string[i]);
			strcpy((char *)eo_string[i], (const char *)eo_string[j]);
			strcpy((char *)eo_string[j], tmpStr);
		}
	}
	temp = A[i+1]; //�� ���ذ��� �߽����� ������, ���ذ��� �� ���̷� ��ġ�� �̵��Ѵ�.
	A[i+1] = A[r];
	A[r] = temp;

	tp = searched[i+1]; //�� ���ذ��� �߽����� ������, ���ذ��� �� ���̷� ��ġ�� �̵��Ѵ�.
	searched[i+1] = searched[r];
	searched[r] = tp;

	strcpy(tmpStr, (const char *)eo_string[i+1]);
	strcpy((char *)eo_string[i+1], (const char *)eo_string[r]);
	strcpy((char *)eo_string[r], tmpStr);
		
	return i+1;
}

void
quickSort2(int A[], int p, int r) // �������� quicksort �Լ� �̴�.
{
	int q;
	
	if(p<r)
	{
		q=partition2(A,p,r); //1. partition �Լ��� ������.
		quickSort2(A,p,q-1); //2. �� �κ��� ���������� �����Ѵ�. -���� �κ��� �����Ѵ�.
		quickSort2(A,q+1,r); //������ �κ��� �����Ѵ�.
	}
}

/*
 * A[]�� �񱳿� ���Ǵ� �� �迭, B[]�� A[] ���� �� ���޾� ��ġ�� �ٲ��� �ϴ� �迭
 */
static long
partition3(int A[], int p, int r)
{
	int x;
	int i;
	int j;
	int temp;
	
	x = A[r];		// ���ذ�
	i = p - 1;		// i�� ���ذ����� �������ҵ��� ���������� ������ ���� ������� �ʴ� 0�ε����� ��ġ�Ѵ�.
	for (j = p; j < r; j++) {	// j�� ���� �������� �ʴ� ���ҵ��� ���ʷ� �˻��� ������.
		if (A[j] >= x)	{	// ���ذ��� ��
			temp = A[++i];	// ���� �� i�� 1�� �����Ͽ� ���ڸ��� ����� i�� ��ġ�� �ε����� ���� ��ȯ�Ѵ�.
			A[i] = A[j];
			A[j] = temp;

			temp = final[max_final][i];	// ���� �� i�� 1�� �����Ͽ� ���ڸ��� ����� i�� ��ġ�� �ε����� ���� ��ȯ�Ѵ�.
			final[max_final][i] = final[max_final][j];
			final[max_final][j] = temp;
		}
	}
	temp = A[i+1]; //�� ���ذ��� �߽����� ������, ���ذ��� �� ���̷� ��ġ�� �̵��Ѵ�.
	A[i+1] = A[r];
	A[r] = temp;

	temp = final[max_final][i+1]; //�� ���ذ��� �߽����� ������, ���ذ��� �� ���̷� ��ġ�� �̵��Ѵ�.
	final[max_final][i+1] = final[max_final][r];
	final[max_final][r] = temp;

	return i+1;
}

void
quickSort3(int A[], int p, int r) // �������� quicksort �Լ� �̴�.
{
	int q;
	
	if (p < r) {
		q = partition3(A, p, r);	//1. partition �Լ��� ������.
		quickSort3(A, p, q-1);		//2. �� �κ��� ���������� �����Ѵ�. -���� �κ��� �����Ѵ�.
		quickSort3(A, q+1, r);		//������ �κ��� �����Ѵ�.
	}
}

/*
 * �迭 A[]�� �ִ� ������ ���ذ� x �̻��� �͵��� ��� ���ʿ�, x �̸��� �͵��� ��� �����ʿ� ��ġ�Ѵ�.
 * x �̻��� �͵��� ������ �ε����� �����Ѵ�. ��, A[0...i]�� x ������ �͵��� �����ִ�. ������ �Ǿ� ���� ����. 
 */
long
partition4(int A[], int p, int r, int x)
{
	// x: ���� ���ذ�
	register int i, j;
	register int temp;
	
	//x = A[r];		// ���ذ�
	i = p - 1;		// i�� ���ذ����� �������ҵ��� ���������� ������ ���� ������� �ʴ� 0�ε����� ��ġ�Ѵ�.
	for (j = p; j <= r; j++) {	// j�� ���� �������� �ʴ� ���ҵ��� ���ʷ� �˻��� ������.
		if (A[j] >= x)	{	// ���ذ��� ��
			i++;
			if (i == j) continue;
			temp = A[i];	// ���� �� i�� 1�� �����Ͽ� ���ڸ��� ����� i�� ��ġ�� �ε����� ���� ��ȯ�Ѵ�.
			A[i] = A[j];
			A[j] = temp;

			temp = final[max_final][i];	// ���� �� i�� 1�� �����Ͽ� ���ڸ��� ����� i�� ��ġ�� �ε����� ���� ��ȯ�Ѵ�.
			final[max_final][i] = final[max_final][j];
			final[max_final][j] = temp;
		}
	}
	
	return i;
}

char *
Int2Currency(int money, char *Currency, bool fPlusSign)
{
	char OneLine[30];
	int len, i, j = 0, k = 0;
	char *result;
	bool fMinus = false;

	if (money < 0) {
		fMinus = true;
		money *= -1;
	}
	//_itoa(money, OneLine, 10);
	//sprintf(OneLine, "%s", money);
	len = strlen(OneLine);
	for (i = len - 1; i >= 0; i--) {
		Currency[j++] = OneLine[i];
		k++;
		if (k % 3 == 0)
			Currency[j++] = ',';
	}
	if (Currency[j-1] == ',')
		Currency[j-1] = 0;
	else
		Currency[j] = 0;
	if (fMinus)
		strcat(Currency, "-");
	else
		if (fPlusSign)
			strcat(Currency, "+");

	//result = _strrev(Currency);
	result = strrev(Currency);


	strcpy(Currency, result);

	return(Currency);
}

/*
 * A[]�� �񱳿� ���Ǵ� �� �迭, B[]�� A[] ���� �� ���޾� ��ġ�� �ٲ��� �ϴ� �迭
 */
static long
partition5(short A[], int p, int r)
{
	int x;
	int i;
	int j;
	int temp;
	int temp2;
	
	x = A[r];		// ���ذ�
	i = p - 1;		// i�� ���ذ����� �������ҵ��� ���������� ������ ���� ������� �ʴ� 0�ε����� ��ġ�Ѵ�.
	for (j = p; j < r; j++) {	// j�� ���� �������� �ʴ� ���ҵ��� ���ʷ� �˻��� ������.
		if (A[j] <= x)	{	// ���ذ��� ��
			temp = A[++i];	// ���� �� i�� 1�� �����Ͽ� ���ڸ��� ����� i�� ��ġ�� �ε����� ���� ��ȯ�Ѵ�.
			A[i] = A[j];
			A[j] = temp;

			temp2 = final[max_final][i];
			final[max_final][i] = final[max_final][j];
			final[max_final][j] = temp2;

			if (eo_count >= 3) {	// score ���� �� merged(2)_final_degree�� ���� ���̰� 3 �̻��� ���� �����.
				temp2 = mfd[i];
				mfd[i] = mfd[j];
				mfd[j] = temp2;
			}
		}
	}
	temp = A[i+1]; //�� ���ذ��� �߽����� ������, ���ذ��� �� ���̷� ��ġ�� �̵��Ѵ�.
	A[i+1] = A[r];
	A[r] = temp;

	temp2 = final[max_final][i+1];
	final[max_final][i+1] = final[max_final][r];
	final[max_final][r] = temp2;

	if (eo_count >= 3) {
		temp2 = mfd[i+1];
		mfd[i+1] = mfd[r];
		mfd[r] = temp2;
	}

	return i+1;
}

void
quickSort5(short A[], int p, int r) // �������� quicksort �Լ� �̴�.
{
	int q;
	
	if (p < r) {
		q = partition5(A, p, r);	//1. partition �Լ��� ������.
		quickSort5(A, p, q-1);		//2. �� �κ��� ���������� �����Ѵ�. -���� �κ��� �����Ѵ�.
		quickSort5(A, q+1, r);		//������ �κ��� �����Ѵ�.
	}
}

/*
 * A[]�� �񱳿� ���Ǵ� �� �迭, B[]�� A[] ���� �� ���޾� ��ġ�� �ٲ��� �ϴ� �迭
 * ���� ���ھ �Լ� �������� ���� ������ �ʼ� ���ھ �Լ��� �� ���� �� �� �� ���̰� ��.
 */
/*
static long
partition6(int A[], int p, int r)
{
	int x;
	int i;
	int j;
	int temp;
	
	x = A[r];		// ���ذ�
	i = p - 1;		// i�� ���ذ����� �������ҵ��� ���������� ������ ���� ������� �ʴ� 0�ε����� ��ġ�Ѵ�.
	for (j = p; j < r; j++) {	// j�� ���� �������� �ʴ� ���ҵ��� ���ʷ� �˻��� ������.
		if (A[j] >= x)	{	// ���ذ��� ��
			temp = A[++i];	// ���� �� i�� 1�� �����Ͽ� ���ڸ��� ����� i�� ��ġ�� �ε����� ���� ��ȯ�Ѵ�.
			A[i] = A[j];
			A[j] = temp;

			temp = cho_final[cho_max_final][i];	// ���� �� i�� 1�� �����Ͽ� ���ڸ��� ����� i�� ��ġ�� �ε����� ���� ��ȯ�Ѵ�.
			cho_final[cho_max_final][i] = cho_final[cho_max_final][j];
			cho_final[cho_max_final][j] = temp;
		}
	}
	temp = A[i+1]; //�� ���ذ��� �߽����� ������, ���ذ��� �� ���̷� ��ġ�� �̵��Ѵ�.
	A[i+1] = A[r];
	A[r] = temp;

	temp = cho_final[cho_max_final][i+1]; //�� ���ذ��� �߽����� ������, ���ذ��� �� ���̷� ��ġ�� �̵��Ѵ�.
	cho_final[cho_max_final][i+1] = cho_final[cho_max_final][r];
	cho_final[cho_max_final][r] = temp;

	return i+1;
}

void
quickSort6(int A[], int p, int r) // �������� quicksort �Լ� �̴�.
{
	int q;
	
	if (p < r) {
		q = partition6(A, p, r);	//1. partition �Լ��� ������.
		quickSort6(A, p, q-1);		//2. �� �κ��� ���������� �����Ѵ�. -���� �κ��� �����Ѵ�.
		quickSort6(A, q+1, r);		//������ �κ��� �����Ѵ�.
	}
}
*/

/*
 * A[]�� �񱳿� ���Ǵ� �� �迭, B[]�� A[] ���� �� ���޾� ��ġ�� �ٲ��� �ϴ� �迭
 */
static long
partition7(short A[], int p, int r)
{
	int x;
	int i;
	int j;
	int temp;
	
	x = A[r];		// ���ذ�
	i = p - 1;		// i�� ���ذ����� �������ҵ��� ���������� ������ ���� ������� �ʴ� 0�ε����� ��ġ�Ѵ�.
	for (j = p; j < r; j++) {	// j�� ���� �������� �ʴ� ���ҵ��� ���ʷ� �˻��� ������.
		if (A[j] <= x)	{	// ���ذ��� ��
			temp = A[++i];	// ���� �� i�� 1�� �����Ͽ� ���ڸ��� ����� i�� ��ġ�� �ε����� ���� ��ȯ�Ѵ�.
			A[i] = A[j];
			A[j] = temp;

			temp = cho_final[cho_max_final][i];	// ���� �� i�� 1�� �����Ͽ� ���ڸ��� ����� i�� ��ġ�� �ε����� ���� ��ȯ�Ѵ�.
			cho_final[cho_max_final][i] = cho_final[cho_max_final][j];
			cho_final[cho_max_final][j] = temp;
		}
	}
	temp = A[i+1]; //�� ���ذ��� �߽����� ������, ���ذ��� �� ���̷� ��ġ�� �̵��Ѵ�.
	A[i+1] = A[r];
	A[r] = temp;

	temp = cho_final[cho_max_final][i+1]; //�� ���ذ��� �߽����� ������, ���ذ��� �� ���̷� ��ġ�� �̵��Ѵ�.
	cho_final[cho_max_final][i+1] = cho_final[cho_max_final][r];
	cho_final[cho_max_final][r] = temp;

	return i+1;
}

void
quickSort7(short A[], int p, int r) // �������� quicksort �Լ� �̴�.
{
	int q;
	
	if (p < r) {
		q = partition7(A, p, r);	//1. partition �Լ��� ������.
		quickSort7(A, p, q-1);		//2. �� �κ��� ���������� �����Ѵ�. -���� �κ��� �����Ѵ�.
		quickSort7(A, q+1, r);		//������ �κ��� �����Ѵ�.
	}
}

char *strrev(char *str)
{
	char *p1, *p2;
	if(!str || !*str)
		return str;
	for(p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
	{
            *p1 ^= *p2;
            *p2 ^= *p1;
            *p1 ^= *p2;
	}
	return str;
}
