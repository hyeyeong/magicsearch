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
	
	x = A[r];		// 기준값
	i = p - 1;		// i는 기준값보다 작은원소들의 끝지점으로 지금은 앞의 사용하지 않는 0인덱스에 위치한다.
	for (j = p; j < r; j++) {	// j가 아직 정해지지 않는 원소들을 차례로 검사해 나간다.
		if(A[j]<=x)	{	// 기준값과 비교
			temp = A[++i];	// 작을 시 i는 1을 증가하여 한자리를 만든뒤 i가 위치한 인덱스로 값을 교환한다.
			A[i] = A[j];
			A[j] = temp;

			tp = searched[i]; //작을시 i는 1을 증가하여 한자리를 만든뒤 i가 위치한 인덱스로 값을 교환한다.
			searched[i] = searched[j];
			searched[j] = tp;

			strcpy(tmpStr, (const char *)eo_string[i]);
			strcpy((char *)eo_string[i], (const char *)eo_string[j]);
			strcpy((char *)eo_string[j], tmpStr);
		}
	}
	temp = A[i+1]; //다 기준값을 중심으로 나뉜뒤, 기준값을 그 사이로 위치를 이동한다.
	A[i+1] = A[r];
	A[r] = temp;

	tp = searched[i+1]; //다 기준값을 중심으로 나뉜뒤, 기준값을 그 사이로 위치를 이동한다.
	searched[i+1] = searched[r];
	searched[r] = tp;

	strcpy(tmpStr, (const char *)eo_string[i+1]);
	strcpy((char *)eo_string[i+1], (const char *)eo_string[r]);
	strcpy((char *)eo_string[r], tmpStr);
		
	return i+1;
}

void
quickSort2(int A[], int p, int r) // 오름차순 quicksort 함수 이다.
{
	int q;
	
	if(p<r)
	{
		q=partition2(A,p,r); //1. partition 함수로 나눈다.
		quickSort2(A,p,q-1); //2. 각 부분을 독립적으로 정렬한다. -왼쪽 부분을 정렬한다.
		quickSort2(A,q+1,r); //오른쪽 부분을 정렬한다.
	}
}

/*
 * A[]가 비교에 사용되는 주 배열, B[]는 A[] 정렬 시 덩달아 위치가 바뀌어야 하는 배열
 */
static long
partition3(int A[], int p, int r)
{
	int x;
	int i;
	int j;
	int temp;
	
	x = A[r];		// 기준값
	i = p - 1;		// i는 기준값보다 작은원소들의 끝지점으로 지금은 앞의 사용하지 않는 0인덱스에 위치한다.
	for (j = p; j < r; j++) {	// j가 아직 정해지지 않는 원소들을 차례로 검사해 나간다.
		if (A[j] >= x)	{	// 기준값과 비교
			temp = A[++i];	// 작을 시 i는 1을 증가하여 한자리를 만든뒤 i가 위치한 인덱스로 값을 교환한다.
			A[i] = A[j];
			A[j] = temp;

			temp = final[max_final][i];	// 작을 시 i는 1을 증가하여 한자리를 만든뒤 i가 위치한 인덱스로 값을 교환한다.
			final[max_final][i] = final[max_final][j];
			final[max_final][j] = temp;
		}
	}
	temp = A[i+1]; //다 기준값을 중심으로 나뉜뒤, 기준값을 그 사이로 위치를 이동한다.
	A[i+1] = A[r];
	A[r] = temp;

	temp = final[max_final][i+1]; //다 기준값을 중심으로 나뉜뒤, 기준값을 그 사이로 위치를 이동한다.
	final[max_final][i+1] = final[max_final][r];
	final[max_final][r] = temp;

	return i+1;
}

void
quickSort3(int A[], int p, int r) // 내림차순 quicksort 함수 이다.
{
	int q;
	
	if (p < r) {
		q = partition3(A, p, r);	//1. partition 함수로 나눈다.
		quickSort3(A, p, q-1);		//2. 각 부분을 독립적으로 정렬한다. -왼쪽 부분을 정렬한다.
		quickSort3(A, q+1, r);		//오른쪽 부분을 정렬한다.
	}
}

/*
 * 배열 A[]에 있는 값들을 기준값 x 이상인 것들은 모두 왼쪽에, x 미만인 것들은 모두 오늘쪽에 배치한다.
 * x 이상인 것들의 마지막 인덱스를 리턴한다. 즉, A[0...i]에 x 이하인 것들이 몰려있다. 정렬은 되어 있지 않음. 
 */
long
partition4(int A[], int p, int r, int x)
{
	// x: 분할 기준값
	register int i, j;
	register int temp;
	
	//x = A[r];		// 기준값
	i = p - 1;		// i는 기준값보다 작은원소들의 끝지점으로 지금은 앞의 사용하지 않는 0인덱스에 위치한다.
	for (j = p; j <= r; j++) {	// j가 아직 정해지지 않는 원소들을 차례로 검사해 나간다.
		if (A[j] >= x)	{	// 기준값과 비교
			i++;
			if (i == j) continue;
			temp = A[i];	// 작을 시 i는 1을 증가하여 한자리를 만든뒤 i가 위치한 인덱스로 값을 교환한다.
			A[i] = A[j];
			A[j] = temp;

			temp = final[max_final][i];	// 작을 시 i는 1을 증가하여 한자리를 만든뒤 i가 위치한 인덱스로 값을 교환한다.
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
 * A[]가 비교에 사용되는 주 배열, B[]는 A[] 정렬 시 덩달아 위치가 바뀌어야 하는 배열
 */
static long
partition5(short A[], int p, int r)
{
	int x;
	int i;
	int j;
	int temp;
	int temp2;
	
	x = A[r];		// 기준값
	i = p - 1;		// i는 기준값보다 작은원소들의 끝지점으로 지금은 앞의 사용하지 않는 0인덱스에 위치한다.
	for (j = p; j < r; j++) {	// j가 아직 정해지지 않는 원소들을 차례로 검사해 나간다.
		if (A[j] <= x)	{	// 기준값과 비교
			temp = A[++i];	// 작을 시 i는 1을 증가하여 한자리를 만든뒤 i가 위치한 인덱스로 값을 교환한다.
			A[i] = A[j];
			A[j] = temp;

			temp2 = final[max_final][i];
			final[max_final][i] = final[max_final][j];
			final[max_final][j] = temp2;

			if (eo_count >= 3) {	// score 정렬 시 merged(2)_final_degree는 쿼리 길이가 3 이상일 때만 사용함.
				temp2 = mfd[i];
				mfd[i] = mfd[j];
				mfd[j] = temp2;
			}
		}
	}
	temp = A[i+1]; //다 기준값을 중심으로 나뉜뒤, 기준값을 그 사이로 위치를 이동한다.
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
quickSort5(short A[], int p, int r) // 오름차순 quicksort 함수 이다.
{
	int q;
	
	if (p < r) {
		q = partition5(A, p, r);	//1. partition 함수로 나눈다.
		quickSort5(A, p, q-1);		//2. 각 부분을 독립적으로 정렬한다. -왼쪽 부분을 정렬한다.
		quickSort5(A, q+1, r);		//오른쪽 부분을 정렬한다.
	}
}

/*
 * A[]가 비교에 사용되는 주 배열, B[]는 A[] 정렬 시 덩달아 위치가 바뀌어야 하는 배열
 * 통합 스코어링 함수 도입으로 인해 없어진 초성 스코어링 함수가 안 쓰여 이 놈도 안 쓰이게 됨.
 */
/*
static long
partition6(int A[], int p, int r)
{
	int x;
	int i;
	int j;
	int temp;
	
	x = A[r];		// 기준값
	i = p - 1;		// i는 기준값보다 작은원소들의 끝지점으로 지금은 앞의 사용하지 않는 0인덱스에 위치한다.
	for (j = p; j < r; j++) {	// j가 아직 정해지지 않는 원소들을 차례로 검사해 나간다.
		if (A[j] >= x)	{	// 기준값과 비교
			temp = A[++i];	// 작을 시 i는 1을 증가하여 한자리를 만든뒤 i가 위치한 인덱스로 값을 교환한다.
			A[i] = A[j];
			A[j] = temp;

			temp = cho_final[cho_max_final][i];	// 작을 시 i는 1을 증가하여 한자리를 만든뒤 i가 위치한 인덱스로 값을 교환한다.
			cho_final[cho_max_final][i] = cho_final[cho_max_final][j];
			cho_final[cho_max_final][j] = temp;
		}
	}
	temp = A[i+1]; //다 기준값을 중심으로 나뉜뒤, 기준값을 그 사이로 위치를 이동한다.
	A[i+1] = A[r];
	A[r] = temp;

	temp = cho_final[cho_max_final][i+1]; //다 기준값을 중심으로 나뉜뒤, 기준값을 그 사이로 위치를 이동한다.
	cho_final[cho_max_final][i+1] = cho_final[cho_max_final][r];
	cho_final[cho_max_final][r] = temp;

	return i+1;
}

void
quickSort6(int A[], int p, int r) // 내림차순 quicksort 함수 이다.
{
	int q;
	
	if (p < r) {
		q = partition6(A, p, r);	//1. partition 함수로 나눈다.
		quickSort6(A, p, q-1);		//2. 각 부분을 독립적으로 정렬한다. -왼쪽 부분을 정렬한다.
		quickSort6(A, q+1, r);		//오른쪽 부분을 정렬한다.
	}
}
*/

/*
 * A[]가 비교에 사용되는 주 배열, B[]는 A[] 정렬 시 덩달아 위치가 바뀌어야 하는 배열
 */
static long
partition7(short A[], int p, int r)
{
	int x;
	int i;
	int j;
	int temp;
	
	x = A[r];		// 기준값
	i = p - 1;		// i는 기준값보다 작은원소들의 끝지점으로 지금은 앞의 사용하지 않는 0인덱스에 위치한다.
	for (j = p; j < r; j++) {	// j가 아직 정해지지 않는 원소들을 차례로 검사해 나간다.
		if (A[j] <= x)	{	// 기준값과 비교
			temp = A[++i];	// 작을 시 i는 1을 증가하여 한자리를 만든뒤 i가 위치한 인덱스로 값을 교환한다.
			A[i] = A[j];
			A[j] = temp;

			temp = cho_final[cho_max_final][i];	// 작을 시 i는 1을 증가하여 한자리를 만든뒤 i가 위치한 인덱스로 값을 교환한다.
			cho_final[cho_max_final][i] = cho_final[cho_max_final][j];
			cho_final[cho_max_final][j] = temp;
		}
	}
	temp = A[i+1]; //다 기준값을 중심으로 나뉜뒤, 기준값을 그 사이로 위치를 이동한다.
	A[i+1] = A[r];
	A[r] = temp;

	temp = cho_final[cho_max_final][i+1]; //다 기준값을 중심으로 나뉜뒤, 기준값을 그 사이로 위치를 이동한다.
	cho_final[cho_max_final][i+1] = cho_final[cho_max_final][r];
	cho_final[cho_max_final][r] = temp;

	return i+1;
}

void
quickSort7(short A[], int p, int r) // 오름차순 quicksort 함수 이다.
{
	int q;
	
	if (p < r) {
		q = partition7(A, p, r);	//1. partition 함수로 나눈다.
		quickSort7(A, p, q-1);		//2. 각 부분을 독립적으로 정렬한다. -왼쪽 부분을 정렬한다.
		quickSort7(A, q+1, r);		//오른쪽 부분을 정렬한다.
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
