#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <sys/types.h>			// struct _stat
#include <sys/stat.h>			// _fstat()
#include <search.h>
//#include <mbstring.h>
#include <ctype.h>
//#include <windows.h>			//�ð�������

#include "main.h"
#include "sorting.h"
#include "redblack.h"
#include "hancodeconvert.h"


////////////////////////120802�� JAY
void do_caculate_close(int do_close_data[], int i);
int close_data[30];
int no_close;
//////////////////////
FILE *fp;		//������ ����

//const char db_file_name1[] = "�ٵ������Ī��.txt";
//const char db_file_name2[] = "���2.0-3.txt";
const char db_file_name3[] = "db.txt";
const char mem_db_file_name[] = "memdb.bin";
const char text_inverted_file_name[] = "text_inverted_file.bin";
const char chosung_inverted_file_name[] = "chosung_inverted_file.bin";
#define	IGNORE_NON_HANGUL

/*
 * load_db_in_memory()�� ���ؼ� ä������ ������
 */
char*			memory_db;						// ����� ��� ������ �о���� �޸�. ��� ������ ũ�⸦ ���� �������� �Ҵ��ϴ� ������� �ٲ� ����.
unsigned int*	position_in_memory;				// ����� �� ���� memory[] �迭 ������ ��𿡼� �����ϴ��� �� ��ġ�� ���� �迭. ��� ������ ���� ���� ��� �������� �Ҵ��ϴ� ����� �ٲ� ����.
unsigned char*	namepart_len;					// ��� �� ������ ��Ī�κ� ���ڿ� ����
unsigned char*	addrpart_len;					// ��� �� ������ �ּҺκ� ���ڿ� ����
unsigned int	db_line_count;					// ����� ��ü ���� ���� �����.
unsigned int	MAX_DB_LINE;					// ����� �� ���� �� ����

/*
 * �ʼ� �˻��� ���� 2-3 ���ε��� ������ �ʿ��� ������
 */
unsigned int cho3_count[MAX_3MOEUM_INDEX];	// 3�����ʼ��� ������ ����. �ε����� 3�����ʼ��� ���� �� �ִ� ��� ����� ���� 
unsigned int cho2_count[MAX_2MOEUM_INDEX];	// 2�����ʼ��� ������ ����. �ε����� 2�����ʼ��� ���� �� �ִ� ��� ����� ���� 
unsigned int cho3_start[MAX_3MOEUM_INDEX];	// cho3_inverted[]���� �� Ʈ������ �����ϴ� �ε������� �����ϴ� �迭
unsigned int cho2_start[MAX_2MOEUM_INDEX];	// cho2_inverted[]���� �� Ʃ���� �����ϴ� �ε������� �����ϴ� �迭
unsigned char *cho3_inverted;				// ������ �迭. �� �ٿ� ������ ��� 3�����ʼ��� 2�����ʼ����� �ٹ�ȣ�� ��ϵȴ�.
unsigned char *cho2_inverted;
unsigned int MAX_INVFILE3_LEN;				// 3�����ʼ� ������ �迭�� ����Ʈ ��.
unsigned int MAX_INVFILE2_LEN;				// 2�����ʼ� ������ �迭�� ����Ʈ ��.
unsigned int MAX_FORMER_LATER_CNT;
unsigned int MIN_FORMER_LATER_CNT;
int *former_list;
int *latter_list;
int **cho_final;							// MAX_FINAL_COL�� �������� �˾Ƴ��� �������� �ٲ�.
int cho_final_count[MAX_QUERY_LEN/2]={0};
int cho_max_final;

/*
 * make_rbtree()���� ä��� ����ϴ� ���� ������
 */
RBTree rbt1;
RBTree rbt2;
int max_index;

/*
 * create_inverted_file() �Լ��� ���ؼ� ä������ ������
 */
// �ؽ�Ʈ �˻���
unsigned int MAX_SEARCHED_COL;
unsigned int MAX_BYTES_IN_INVERTED_FILE;
unsigned char *inverted;
int *i_start;
int *i_count;
int **searched;
int **final;
int *first_final_degree;
int *second_final_degree;
int *third_final_degree;
int *merged_final_degree;
int *merged2_final_degree;
int *mfd;	// merged�� merged2 ��...������ �� ���� ������ �۷ι��� ����� ����.

// �ؽ�Ʈ �˻���
unsigned char eo_string[MAX_QUERY_LEN][HANGUL_BYTE+1];
int eo_count;
int searched_count[MAX_QUERY_LEN];
short eo_index[MAX_QUERY_LEN];
int final_count[MAX_QUERY_LEN];
int max_final;
int first_max_final;
int second_max_final;
int third_max_final;
int merged_max_final;

int no_of_filtered_data = 0;	//���ھ�� ���͸� �������� �� ����(18000�� ����)

//��ü DB �ߺ����̺� 
int Overlap_Count_perChar_tint[MAX_OVERLAP]; //�ߺ��� �ִ� ���� t_int
int Overlap_Count_perChar_count[MAX_OVERLAP]; //�ִ��ߺ�Ƚ��


int main() 
{	
	int query_len;
	int fFullyIncluded;
	char in_query[MAX_QUERY_LEN*2] = {" "};			// �Էµ� ���� ���ڿ��� ����� �迭
	short query_cho[MAX_JAEUM_CNT] = {0};			// �������� �ʼ��� �����Ͽ� ������ �迭
	int query_alpha[MAX_MOEUM_CNT] = {0};			// ������ ���ԵǾ� �ִ� ���ĺ����� ����Ǵ� �迭
	int query_count=0;								// ������ �ִ� ���ڵ��� ����
	int query_alpha_count = 0;						// ������ �ִ� ���ڸ� ������ ���ĺ� ������ ����

	load_db_in_memory();				// ��� �޸𸮿� �ø���, �ʼ��� 2,3�����ʼ� ID�� �����.
	
	fp = fopen("output.txt", "w");	//������ ����

	make_rbtree();						// �ϼ������� ���� ���ں� ID�� �����.
	create_inverted_file_4text();		// �ϼ����� ���ε��� ����
	create_inverted_file_4chosung();	// �ʼ��� ���ε��� ����

	

	for (;;) {
		init_variables_4text();		// �ٽ� �˻� �� �ʱ�ȭ �ؾ� �ϴ� �ϼ����� �߿� �迭���� �ʱ�ȭ�Ѵ�.
		init_variables_4chosung();	// �ٽ� �˻� �� �ʱ�ȭ �ؾ� �ϴ� �ʼ��� �߿� �迭���� �ʱ�ȭ�Ѵ�.
		do {
			printf("\n�˻�� �Է��ϼ���(����� quit): ");
			gets(in_query);	// ���鰰�� white char�� �Է¿� ���Խ�Ű�� ����
			query_len = strlen((const char *)in_query);
			if (query_len > MAX_QUERY_LEN)
				printf("main(): ���� ���̰� %d�� �Ѿ����ϴ�. �ٽ� �Է��ϼ���.", MAX_QUERY_LEN);
		} while (query_len > MAX_QUERY_LEN);
		if (!strcmp(in_query, "quit"))
			break;

		printf("[�˻� ���]\n");
		switch (CheckIfChosungIncluded(in_query)) {
		case -1:		// �ϼ��� �˻��� �ϸ� ��
			printf("CheckIfChosungIncluded()���� -1 ����: �ϼ��� ���ڸ� ���ԵǾ� ����(��ȿ����: %s).\n", in_query);
			fFullyIncluded = search_inverted_file_4text(in_query);
			scoring_and_output_4text(fFullyIncluded);
			break;
		case 1:		// �ʼ� �˻��� �ϸ� ��.
			printf("CheckIfChosungIncluded()���� 1 ����: �ʼ� ���ڸ� ���ԵǾ� ����(��ȿ����: %s).\n", in_query);
			extract_chosung_from_query(in_query, query_cho, query_alpha, &query_count, &query_alpha_count);
			if (query_count < 2) {
				printf("main(): ���� ���̰� �� ���� �̸��̾ �ʼ� �˻��� ���� �ʽ��ϴ�.\n");
				continue;
			}
			search_using_inverted_file_4chosung(query_cho, query_count, query_alpha, query_alpha_count);
			//scoring_and_output_4chosung(query_cho, query_count, query_alpha, query_alpha_count);
			fFullyIncluded = -1;
			integrated_scoring_and_output(fFullyIncluded, in_query, cho_final[cho_max_final], cho_final_count[cho_max_final]);
			break;
		case 0:		// �ϼ����� �ʼ��� ���� �ִµ� �ϼ��� ���ڰ� �� �� �̻��̾ �ϼ��� �˻� ����� ��� ���� ���ھ ��.
			printf("CheckIfChosungIncluded()���� 0 ����: �ϼ��� ���ڿ� �ʼ� ������, �ϼ��� �� �� �̻� ==> �ϼ��� �˻� ����� ���� ���ھ ��(��ȿ����: %s).\n", in_query);
			// �ϼ��� �˻� �ǽ�
			fFullyIncluded = search_inverted_file_4text(in_query);
			//scoring_and_output_4text(fFullyIncluded);
			// �ʼ� �˻� �ǽ�
			//extract_chosung_from_query(in_query, query_cho, query_alpha, &query_count, &query_alpha_count);
			//if (query_count < 2) {
			//	printf("main(): ���� ���̰� �� ���� �̸��̾ �ʼ� �˻��� ���� �ʽ��ϴ�.\n");
			//	continue;
			//}
			//search_using_inverted_file_4chosung(query_cho, query_count, query_alpha, query_alpha_count);
			//scoring_and_output_4chosung(query_cho, query_count);
			// �ϼ��� �˻� �켱 ���ھ �ǽ�
			integrated_scoring_and_output(fFullyIncluded, in_query, final[max_final], final_count[max_final]);
			break;
		case 2:		// �ϼ����� �ʼ��� ���� �ִµ� �ϼ��� ���ڰ� �� �� �̸��̾ �ʼ� �˻��� �ϰ� �� ����� ��� ���� ���ھ ��
			printf("CheckIfChosungIncluded()���� 2 ����: �ϼ��� ���ڿ� �ʼ� ������, �ϼ����� �� �� ���� ==> �ʼ� �˻� ����� ���� ���ھ ��(��ȿ����: %s).\n", in_query);
			// �ϼ��� �˻� �ǽ�
			//fFullyIncluded = search_inverted_file_4text(in_query);
			//scoring_and_output_4text(fFullyIncluded);
			// �ʼ� �˻� �ǽ�
			extract_chosung_from_query(in_query, query_cho, query_alpha, &query_count, &query_alpha_count);
			if (query_count < 2) {
				printf("main(): ���� ���̰� �� ���� �̸��̾ �ʼ� �˻��� ���� �ʽ��ϴ�.\n");
				continue;
			}
			search_using_inverted_file_4chosung(query_cho, query_count, query_alpha, query_alpha_count);
			//scoring_and_output_4chosung(query_cho, query_count);
			// �ϼ��� �˻� �켱 ���ھ �ǽ�
			fFullyIncluded = -1;
			integrated_scoring_and_output(fFullyIncluded, in_query, cho_final[cho_max_final], cho_final_count[cho_max_final]);
			break;
		default:
			printf("main(): CheckIfChosungIncluded(in_query)���� �̻��� ���� �Ѿ� �Խ��ϴ�.\n");
			break;
		}
	}

	printf("cleaning...");
	cleanup();
	printf("done.\n");
	return 0;
}// end of main()

/*
 * ����� ��� ������ ������ ������ ����� �� �迭�� ä���.
 * 1. namepart_len[�ٹ�ȣ]		�� �ٿ��� '@' ������ ��Ī�κ� ���ڿ� ���̸� ����
 * 2. addrpart_len[�ٹ�ȣ]		�� �ٿ��� '@' ������ �ּҺκ� ���ڿ� ���̸� ����
 * 3. position_in_memory[�ٹ�ȣ]	�� �� ��ü�� ������ �ִ� memory[] ���� ��ġ
 * 4. memory_db[�ʿ��Ѹ�ŭ]		����� ��� ������ ����.
 * 5. cho3_count[3�����ʼ�ID]	��� �����ϴ� �� 3�����ʼ��� 38���� ID ���� �� ����Ƚ���� ����.
 * 6. cho2_count2[2�����ʼ�ID]	��� �����ϴ� �� 2���Ӹ����� 38���� ID ���� �� ����Ƚ���� ����.
 */
void
load_db_in_memory(void)
{
	unsigned int entry_count;			// ��� �ٹ�ȣ�� ���� ���� ����
	//long p, prev;						// ��� ������ ���� �������� �˱� ���� ���
	bool naming;						// ��Ī�κ��� ���� �ִ��� �ּ� �κ��� ���� �ִ��� �����ϱ� ���� ������
	unsigned char t_char;				// ��񿡼� �� ���ھ� �о� �� �� �ӽ÷� �����
	char han[HANGUL_BYTE+1];			// �ѱ� ó�� �� ���� �ϳ��� ��� ���� ���� ����. NULL ���ڱ��� �����ϱ� ���� +1.
	char c_han[HANGUL_BYTE+1];			// �ϼ��� ���ڸ� ���������� �ٲ� �� ����ϱ� ���� ����.
	int position_in_memdb;
	const char *dbfile = db_file_name3;
	struct stat buf;					// ��� ������ ũ�⸦ �˾Ƴ��� ����
	long memory_db_size;
	char cu[20];
	size_t nRead;
	unsigned short johapcode;			// �ϼ������� ������ �ڵ�� ��ȯ�� ���� �ӽ÷� ����
	unsigned short cho_code;			// �и��� �ʼ� �ڵ带 �ӽ÷� ����
	unsigned short cho[MAX_MOEUM_CNT];	// ��Ī�κп� ������ �ʼ� �ڵ�� �迭. �� ���� ó���� �� �Ź� �����.
	unsigned short cho_ad[MAX_MOEUM_CNT];// �ּҺκп� ������ �ʼ� �ڵ�� �迭. �� ���� ó���� �� �Ź� �����.
	int c_count = 0;					// ��Ī�κ��� �ʼ� ������ ���� ���� ���
	int c_ad_count = 0;					// �ּҺκ��� �ʼ� ������ ���� ���� ���
	int c_index;						// 3�����ʼ� �Ǵ� 2�����ʼ��� 38���� ID�� ��ȯ�� �� �ӽ÷� ���. ��, 3�����ʼ� �Ǵ� 2�����ʼ��� ID�� ���� �뵵.
	
	// �̹� �޸� ��� ������� ������ ���� �޸� ��� �������� �ʰ� �о���δ�.
	FILE  *fp;
	if  ((fp = fopen(mem_db_file_name, "rb")) != NULL) {	// �޸� ��� �̹� ������� ������   
		printf("Loading the existing memory DB...");

		fread(&db_line_count, sizeof(unsigned int), 1, fp);	// db_line_count �о��
		fread(&MAX_DB_LINE, sizeof(unsigned int), 1, fp);	// MAX_DB_LINE �о��
		fread(&memory_db_size, sizeof(long), 1, fp);		// memory_db_size �о��. ���� �����ε� �۷ι��� ���� �� ��...

		// �� ���� ��ŭ �ʿ��� �迭���� �Ҵ��Ѵ�.
		position_in_memory = (unsigned int *)malloc(sizeof(unsigned int)*MAX_DB_LINE);
		namepart_len = (unsigned char *)malloc(sizeof(unsigned char)*MAX_DB_LINE);
		addrpart_len = (unsigned char *)malloc(sizeof(unsigned char)*MAX_DB_LINE);
		if (position_in_memory == NULL || namepart_len == NULL || addrpart_len == NULL) {
			fprintf(stderr, "\t��� ���� �� ��ŭ�� �ʿ�� �ϴ� �迭���� �Ҵ��ϴ� ���� �����߽��ϴ�.\n");
			exit(-3);
		}

		memory_db = (char *)malloc(memory_db_size);	// ��� ������ ũ�� ��ŭ memory_db[]�� �Ҵ��Ѵ�.
		if (memory_db == NULL) {
			//fprintf(stderr, "\tmemory_db[]�� ���� ũ�� %s ����Ʈ��ŭ �Ҵ��ϴ� ���� �����߽��ϴ�.\n", Int2Currency(memory_db_size, cu, false));
			fprintf(stderr, "failed to allocate memory_db");
			exit(-4);
		}
		//printf("\tmemory_db[]�� ũ��(���� ũ��� ����) = %s(%.3f MB).\n", Int2Currency(memory_db_size, cu, false), (double)memory_db_size/1024/1024);

		fread(position_in_memory, sizeof(unsigned int), MAX_DB_LINE, fp);	// position_in_memory_db[] �迭 �о��
		fread(namepart_len, sizeof(unsigned char), MAX_DB_LINE, fp);		// namepart_len[] �迭 �о��
		fread(addrpart_len, sizeof(unsigned char), MAX_DB_LINE, fp);		// addrpart_len[] �迭 �о��
		fread(memory_db, sizeof(char), memory_db_size, fp);					// memory_db[] �迭 �о��
		fread(cho3_count, sizeof(unsigned int), MAX_3MOEUM_INDEX, fp);		// ��� 3�����ʼ� ID���� ���� ȸ�� �迭�� cho3_count[] ����
		fread(cho2_count, sizeof(unsigned int), MAX_2MOEUM_INDEX, fp);		// ��� 2�����ʼ� ID���� ���� ȸ�� �迭�� cho2_count[] ����
		fclose(fp);	
	}
	else {	// ��� ������ �о� �޸� ��� �����Ѵ�.
		printf("Creating memory DB from raw DB ...\n");

		// ��� ������ ����
		if  ((fp = fopen(dbfile, "rb")) == NULL) {
			fprintf(stderr, "file  open  error  to  read"); 
			exit(1); 
		} 

		// ��� ���� ���� �� ������ �˾Ƴ���.
		MAX_DB_LINE = 0;
		for (;;) {
			nRead = fread(&t_char,  1,  1,  fp);
			if (nRead < 1)
				break;
			if (t_char == '\r')
				MAX_DB_LINE++;
		}
		MAX_DB_LINE++;			// �������� �� �� �� ����.
		fseek(fp, 0, SEEK_SET);
		//printf("\t%s lines found in DB file %s\n", Int2Currency(MAX_DB_LINE-1, cu, false), dbfile);


		// �� ���� ��ŭ �ʿ��� �迭���� �Ҵ��Ѵ�.
		position_in_memory = (unsigned int *)malloc(sizeof(unsigned int)*MAX_DB_LINE);
		namepart_len = (unsigned char *)malloc(sizeof(unsigned char)*MAX_DB_LINE);
		addrpart_len = (unsigned char *)malloc(sizeof(unsigned char)*MAX_DB_LINE);
		if (position_in_memory == NULL || namepart_len == NULL || addrpart_len == NULL) {
			fprintf(stderr, "\t��� ���� �� ��ŭ�� �ʿ�� �ϴ� �迭���� �Ҵ��ϴ� ���� �����߽��ϴ�.\n");
			exit(-1);
		}

		stat(dbfile, &buf);			// ��� ������ ũ�⸦ ����
		memory_db_size = buf.st_size;
		memory_db = (char *)malloc(memory_db_size);	// ��� ������ ũ�� ��ŭ memory_db[]�� �Ҵ��Ѵ�.
		if (memory_db == NULL) {
			//fprintf(stderr, "\tmemory_db[]�� ���� ũ�� %s ����Ʈ��ŭ �Ҵ��ϴ� ���� �����߽��ϴ�.\n", Int2Currency(memory_db_size, cu, false));
			exit(-1);
		}
		//printf("\tmemory_db[]�� ũ��(���� ũ��� ����) = %s(%.3f MB).\n", Int2Currency(memory_db_size, cu, false), (double)memory_db_size/1024/1024);

		entry_count = 0;
		position_in_memory[0] = 0;

		namepart_len[entry_count] = 0;
		addrpart_len[entry_count] = 0;

		naming = true;	// �̸� �κк��� �����Ѵٰ� ����

		for (;;) {
			nRead = fread(&t_char,  1,  1,  fp);
			if (nRead < 1 && feof(fp))
				break;
		
			if (t_char >= 128) {	// �ѱ��̸�
				han[0] = t_char;

				nRead = fread(&t_char,  1,  1,  fp);
				if (nRead < 1 && feof(fp))
					break;
			
				if (naming)
					namepart_len[entry_count] += 2;		// '@' �����̸� namepart_len[�ٹ�ȣ] 1 ����
				else
					addrpart_len[entry_count] += 2;	// '@' �����̸� addrpart_len[�ٹ�ȣ] 1 ����

				han[1] = t_char;
				han[2] = '\0';
				position_in_memdb = position_in_memory[entry_count] + namepart_len[entry_count] + addrpart_len[entry_count];
				memory_db[position_in_memdb - 2] = han[0];
				memory_db[position_in_memdb - 1] = han[1];

				// �ʼ� 2,3 �ε����� ���� �߰���
				HangulCodeConvert(KS, han, c_han);	// �ϼ��� �ѱ��ڵ� 16��Ʈ�� ������ 16��Ʈ�� �ٲ۴�.
				johapcode = ((unsigned char)c_han[0] << 8) | (unsigned char)c_han[1];
				cho_code = (johapcode >> 10) & 0x1f;		// ���� 10��Ʈ�� ���ּ� �ʼ� �ڵ带 �����Ѵ�.
				if (cho_code < 0x02 || cho_code > 0x14) {
					fprintf(stderr, "load_db_in_memory(): �ٹ�ȣ %d: ������ �ʼ� ���� '%s' �߰�! �����մϴ�.\n", entry_count+1, han);
					continue;
				}
				if (naming) {	// ��Ī �κ�
					cho[c_count] = cho_code;
					c_count++;
				}
				else {			// �ּ� �κ�
					cho_ad[c_ad_count] = cho_code;
					c_ad_count++;
				}
				// �ʼ� �ε��� ó���� ���� �߰��� �κ� ��
			}
			else if (t_char == '@' && naming) {		// ù '@'�� ����. �� ��Ī �κ��� ���� ����. ���� �ٿ� '@'�� �� ���´ٸ� �ܼ� �ƽ�Ű���ڷ� ó���ؾ� ��
				namepart_len[entry_count]++;			// �� ���ڰ� �� �ڸ��� ������.
				position_in_memdb = position_in_memory[entry_count] + namepart_len[entry_count] + addrpart_len[entry_count];	// ���� ��ġ
				memory_db[position_in_memdb - 1] = NULL;	// ���ڿ� ���� ǥ��

				// �ʼ� 2,3 �ε����� ���� �߰���
				if (c_count == 1) {			// ��Ī �κп� �ִ� ���ڵ��� �ʼ� ������ �� ���ۿ� ������,
					c_index = cho[0]*38*38;
					if (c_index > MAX_3MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 3�ʼ� �ε����� MAX_3MOEUM_INDEX�� �Ѿ����ϴ�. �̴� �ʼ��ڵ尡 �̻��� �� �Դϴ�.\n");
						exit(-10);
					}
					cho3_count[c_index]++;
				
					c_index = cho[0]*38;
					if (c_index > MAX_2MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 2�ʼ� �ε����� MAX_2MOEUM_INDEX�� �Ѿ����ϴ�. �̴� �ʼ��ڵ尡 �̻��� �� �Դϴ�.\n");
						exit(-10);
					}
					cho2_count[c_index]++;
				}
				else if (c_count == 2) {	// ��Ī �κп� �ִ� ���ڵ��� �ʼ� ������ �� ���̸�,
					c_index = cho[0]*38*38 + cho[1]*38;
					if (c_index > MAX_3MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 3�ʼ� �ε����� MAX_3MOEUM_INDEX�� �Ѿ����ϴ�. �̴� �ʼ��ڵ尡 �̻��� �� �Դϴ�.\n");
						exit(-10);
					}
					cho3_count[c_index]++;
				
					c_index = cho[0]*38 + cho[1];
					if (c_index > MAX_2MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 2�ʼ��ε����� MAX_2MOEUM_INDEX�� �Ѿ����ϴ�. �̴� �ʼ��ڵ尡 �̻��� �� �Դϴ�.\n");
						exit(-10);
					}
					cho2_count[c_index]++;
				}
				else {						// ��Ī �κп� �ִ� ���ڵ��� �ʼ� ������ �� ���� �̻��,
					int z;

					for (z = 0; z < c_count-2 ; z++) {	// 3�����ʼ��� ���� 38������ �����.
						c_index = cho[z]*38*38 + cho[z+1]*38 + cho[z+2];
						if (c_index > MAX_3MOEUM_INDEX) {
							fprintf(stderr, "load_db_in_memory(): 3�ʼ� �ε����� MAX_3MOEUM_INDEX�� �Ѿ����ϴ�. �̴� �ʼ��ڵ尡 �̻��� �� �Դϴ�.\n");
							exit(-10);
						}
						cho3_count[c_index]++;
					}
					for (z = 0; z < c_count-1; z++) {	// 2�����ʼ��� ���� 38������ �����.
						c_index = cho[z]*38 + cho[z+1];
						if (c_index > MAX_2MOEUM_INDEX) {
							fprintf(stderr, "load_db_in_memory(): 2�ʼ� �ε����� MAX_2MOEUM_INDEX�� �Ѿ����ϴ�. �̴� �ʼ��ڵ尡 �̻��� �� �Դϴ�.\n");
							exit(-10);
						}
						cho2_count[c_index]++;
					}
				}
				// �ʼ� �ε��� ó���� ���� �߰��� �κ� ��

				naming = false;
			}
			else if (t_char == '@' && !naming) {	// �ּҺκп��� @�� �� ������.��Ī-�ּҺκ� �����ڷ� ������� �ʰ� ��񿡸� �ְ� ��!
				addrpart_len[entry_count]++;
				position_in_memdb = position_in_memory[entry_count] + namepart_len[entry_count] + addrpart_len[entry_count];
				memory_db[position_in_memdb - 1] = t_char;
			}
			else if (t_char == '\r' && !naming) {	// �ּ� �κ��� ���ٰ� �� ���� �� ��������...
				addrpart_len[entry_count]++;	// NULL �� �ڸ� Ȯ��
				position_in_memdb = position_in_memory[entry_count] + namepart_len[entry_count] + addrpart_len[entry_count];
				memory_db[position_in_memdb - 1] = NULL;	// ���ڿ� ���� ó��

				// �ʼ� 2,3 �ε����� ���� �߰���
				if (c_ad_count == 1) {
					c_index = cho_ad[0]*38*38;
					if (c_index > MAX_3MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 3�ʼ� �ε����� MAX_3MOEUM_INDEX�� �Ѿ����ϴ�. �̴� �ʼ��ڵ尡 �̻��� �� �Դϴ�.\n");
						exit(-10);
					}
					cho3_count[c_index]++;
				
					c_index = cho_ad[0]*38;
					if (c_index > MAX_2MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 2�ʼ� �ε����� MAX_2MOEUM_INDEX�� �Ѿ����ϴ�. �̴� �ʼ��ڵ尡 �̻��� �� �Դϴ�.\n");
						exit(-10);
					}
					cho2_count[c_index]++;
				}
				else if (c_ad_count == 2) {
					c_index = cho_ad[0]*38*38 + cho_ad[1]*38;
					if (c_index > MAX_3MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 3�ʼ� �ε����� MAX_3MOEUM_INDEX�� �Ѿ����ϴ�. �̴� �ʼ��ڵ尡 �̻��� �� �Դϴ�.\n");
						exit(-10);
					}
					cho3_count[c_index]++;
				
					c_index = cho_ad[0]*38 + cho_ad[1];
					if (c_index > MAX_2MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 2�ʼ� �ε����� MAX_2MOEUM_INDEX�� �Ѿ����ϴ�. �̴� �ʼ��ڵ尡 �̻��� �� �Դϴ�.\n");
						exit(-10);
					}
					cho2_count[c_index]++;
				}
				else {
					int z;

					for (z = 0; z < c_ad_count-2 ; z++) {
						c_index = cho_ad[z]*38*38 + cho_ad[z+1]*38 + cho_ad[z+2];
						if (c_index > MAX_3MOEUM_INDEX) {
							fprintf(stderr, "load_db_in_memory(): 3�ʼ� �ε����� MAX_3MOEUM_INDEX�� �Ѿ����ϴ�. �̴� �ʼ��ڵ尡 �̻��� �� �Դϴ�.\n");
							exit(-10);
						}
						cho3_count[c_index]++;
					}
					for (z = 0; z < c_ad_count-1 ; z++) {
						c_index = cho_ad[z]*38 + cho_ad[z+1];
						if (c_index > MAX_2MOEUM_INDEX) {
							fprintf(stderr, "load_db_in_memory(): 2�ʼ� �ε����� MAX_2MOEUM_INDEX�� �Ѿ����ϴ�. �̴� �ʼ��ڵ尡 �̻��� �� �Դϴ�.\n");
							exit(-10);
						}
						cho2_count[c_index]++;
					}
				}
				c_ad_count=0;	// �� ���� �������Ƿ� �ּҺκа�
				c_count=0;		// ��Ī�κ��� �ʼ� ���� ���� ������ �����Ѵ�.
				// �ʼ� �ε��� ó���� ���� �߰��� �κ� ��

				entry_count++;
				if (entry_count >= MAX_DB_LINE) {
					fprintf(stderr, "load_db_in_memory(): ��� ���� ���� %d�� �Ѿ����ϴ�. �ø��� �ٽ� ������ �ϼ���.\n", entry_count);
					exit(-1);
				}
				// ���� ���� ������ ���� memory_db[] ���� �ʱ� ��ġ position_in_memory_db[entry_count]�� ����صд�.
				position_in_memory[entry_count] = position_in_memory[entry_count-1] + namepart_len[entry_count-1] + addrpart_len[entry_count-1];
				namepart_len[entry_count] = addrpart_len[entry_count] = 0;
				naming = true;
			}
			else if (t_char == '\r' && naming) { // �� ���� �� �����µ� ������ ��Ī �κ��� �ϰ� �־�����, �� �� �ٿ��� '@'�� ����
				namepart_len[entry_count]++;
				position_in_memdb = position_in_memory[entry_count] + namepart_len[entry_count] + addrpart_len[entry_count];
				memory_db[position_in_memdb - 1] = NULL;	// ���ڿ� ���� ǥ��

				// �ʼ� 2,3 �ε����� ���� �߰���
				if (c_count == 1) {			// ��Ī �κп� �ִ� ���ڵ��� ���� ������ �� ���ۿ� ������,
					c_index = cho[0]*38*38;
					if (c_index > MAX_3MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 3�ʼ� �ε����� MAX_3MOEUM_INDEX�� �Ѿ����ϴ�. �̴� �ʼ��ڵ尡 �̻��� �� �Դϴ�.\n");
						exit(-10);
					}
					cho3_count[c_index]++;
				
					c_index = cho[0]*38;
					if (c_index > MAX_2MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 2�ʼ� �ε����� MAX_2MOEUM_INDEX�� �Ѿ����ϴ�. �̴� �ʼ��ڵ尡 �̻��� �� �Դϴ�.\n");
						exit(-10);
					}
					cho2_count[c_index]++;
				}
				else if (c_count == 2) {	// ��Ī �κп� �ִ� ���ڵ��� ���� ������ �� ���̸�,
					c_index = cho[0]*38*38 + cho[1]*38;
					if (c_index > MAX_3MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 3�ʼ� �ε����� MAX_3MOEUM_INDEX�� �Ѿ����ϴ�. �̴� �ʼ��ڵ尡 �̻��� �� �Դϴ�.\n");
						exit(-10);
					}
					cho3_count[c_index]++;
				
					c_index = cho[0]*38 + cho[1];
					if (c_index > MAX_2MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 2�ʼ� �ε����� MAX_2MOEUM_INDEX�� �Ѿ����ϴ�. �̴� �ʼ��ڵ尡 �̻��� �� �Դϴ�.\n");
						exit(-10);
					}
					cho2_count[c_index]++;
				}
				else {						// ��Ī �κп� �ִ� ���ڵ��� ���� ������ �� ���� �̻��,
					int z;

					for (z = 0; z < c_count-2 ; z++) {	// ���� �� ��
						c_index = cho[z]*38*38 + cho[z+1]*38 + cho[z+2];
						if (c_index > MAX_3MOEUM_INDEX) {
							fprintf(stderr, "load_db_in_memory(): 3�ʼ� �ε����� MAX_3MOEUM_INDEX�� �Ѿ����ϴ�. �̴� �ʼ��ڵ尡 �̻��� �� �Դϴ�.\n");
							exit(-10);
						}
						cho3_count[c_index]++;
					}
					for (z = 0; z < c_count-1; z++) {	// ���� �� ��
						c_index = cho[z]*38 + cho[z+1];
						if (c_index > MAX_2MOEUM_INDEX) {
							fprintf(stderr, "load_db_in_memory(): 2�ʼ� �ε����� MAX_2MOEUM_INDEX�� �Ѿ����ϴ�. �̴� �ʼ��ڵ尡 �̻��� �� �Դϴ�.\n");
							exit(-10);
						}
						cho2_count[c_index]++;
					}
				}
				c_ad_count=0;	// �� ���� �������Ƿ� �ּҺκа�
				c_count=0;		// ��Ī�κ��� �ʼ� ���� ���� ������ �����Ѵ�.
				// �ʼ� �ε��� ó���� ���� �߰��� �κ� ��

				entry_count++;
				if (entry_count >= MAX_DB_LINE) {
					fprintf(stderr, "load_db_in_memory(): ��� ���� ���� %d�� �Ѿ����ϴ�. �ø��� �ٽ� ������ �ϼ���.\n", entry_count);
					exit(-1);
				}
				position_in_memory[entry_count] = position_in_memory[entry_count-1] + namepart_len[entry_count-1] + addrpart_len[entry_count-1];
				namepart_len[entry_count] = addrpart_len[entry_count] = 0;
			}
			else if (t_char == '\n')
				continue;
	#ifdef	IGNORE_NON_HANGUL	// �ʼ� �Ǵ� �߼� ������ �ʿ� ���� ���� '�ѱ��ܹ̿���' ��ũ�θ� define �ؾ� �Ѵ�.
			else {			// �ѱ��� �ƴϸ� memory_db[]���� �ְ� ���� ������ ���� �ʴ´�. �ּҺκп��� @�� �� ���� ��쵵 ����� ������.
				if (naming)
					namepart_len[entry_count]++;
				else
					addrpart_len[entry_count]++;

				position_in_memdb = position_in_memory[entry_count] + namepart_len[entry_count] + addrpart_len[entry_count];
				memory_db[position_in_memdb - 1] = isalpha(t_char) ? toupper(t_char) : t_char;	// ���ĺ��� ��� �빮�ڷ� �ٲ㼭 �޸� ��� ������
			}
	#else	// �ʼ� �Ǵ� ���� ������ �ʿ��� ��� '�ѱ��ܹ̿���' ��ũ�θ� undef �ؾ� �Ѵ�.
			else if (t_char >= '0' && t_char <= '9') {	// ����
				if (naming)
					namepart_len[entry_count]++;
				else
					addrpart_len[entry_count]++;

				position_in_memdb = position_in_memory[entry_count] + namepart_len[entry_count] + addrpart_len[entry_count];
				memory_db[position_in_memdb - 1] = t_char;

				if (naming)
					cho[c_count++] = t_char-'0'+0x15;		// '0': �ʼ��ڵ�21, '1': 22, ..., '9': 30
				else
					cho_ad[c_ad_count++] = t_char-'0'+0x15;
			}
			else { // ��ũ�� ������ �Ϲ� �ƽ�Ű ����
				if (naming)
					namepart_len[entry_count]++;
				else
					addrpart_len[entry_count]++;

				position_in_memdb = position_in_memory[entry_count] + namepart_len[entry_count] + addrpart_len[entry_count];
				memory_db[position_in_memdb - 1] = isalpha(t_char) ? toupper(t_char) : t_char;	// ���ĺ��� ��� �빮�ڷ� �ٲ㼭 �޸� ��� ������
	
				if (naming)
					cho[c_count++] = 0;			// ���ڸ� ������ �Ϲ� �ƽ�Ű���ڴ� �ʼ� �ڵ� ���� �Ұ�!!!
				else
					cho_ad[c_ad_count++] = 0;	// �׷��� �ϴ� 0�� ä���д�.
			}
	#endif
		} // end of for (;;)
		fclose(fp);
		db_line_count = entry_count;
		//printf("\t%d ����, memory_db_index = %d out of (0 ~ %d).\ndone.\n", db_line_count, position_in_memdb, memory_db_size-1);
		
		printf("\tSaving memory DB ...");
		if  ((fp = fopen(mem_db_file_name, "wb")) == NULL) {	// �޸� ��� ����. ������ ����������,   
			fprintf(stderr, "memory DB ���� ������ ���� ����!");
			exit(-20);
		}
		fwrite(&db_line_count, sizeof(unsigned int), 1, fp);				// db_line_count ����
		fwrite(&MAX_DB_LINE, sizeof(unsigned int), 1, fp);					// MAX_DB_LINE ����
		fwrite(&memory_db_size, sizeof(long), 1, fp);						// memory_db_size ����. ���� �����ε� �۷ι��� ���� �� ��...
		fwrite(position_in_memory, sizeof(unsigned int), MAX_DB_LINE, fp);	// position_in_memory_db[] ����
		fwrite(namepart_len, sizeof(unsigned char), MAX_DB_LINE, fp);		// namepart_len[] ����
		fwrite(addrpart_len, sizeof(unsigned char), MAX_DB_LINE, fp);		// addrpart_len[] ����
		fwrite(memory_db, sizeof(char), memory_db_size, fp);				// memory_db[] ����.
		fwrite(cho3_count, sizeof(unsigned int), MAX_3MOEUM_INDEX, fp);		// ��� 3�����ʼ� ID���� ���� ȸ�� �迭�� cho3_count[] ����
		fwrite(cho2_count, sizeof(unsigned int), MAX_2MOEUM_INDEX, fp);		// ��� 2�����ʼ� ID���� ���� ȸ�� �迭�� cho2_count[] ����
		fclose(fp);
		printf("Done.\n");
	}
	printf("Done.\n");
}

/*
 * ����� ��Ī �κп� ���ؼ���, �����ϴ� ��� unique ���ڵ鿡 ���� ID�� ����� ���� �۾���.
 * �ּҺκ��� 2,3 �ʼ� �ε��� ���� �� ���ԵǾ���. 
 */
void
make_rbtree(void)
{
	Node *result;
	int cur, len;
	int t_int=0;
	unsigned char han[HANGUL_BYTE+1];
	unsigned char t_char;
	int count;

	//�ߺ����� ���� �ذ�κ�
	int total_chrnum_indb=0;//db�� �ִ� �� ���� ���� 
	int Max_t_int =0;//t_int �ִ밪
	
	bool new_t_int = false;//�̹� �ִ� t_int������ �˻�
	bool thatchar = false;//����ε� ���ڿ��� Ȯ�� //�� ������ ���� ��� null���� ������ ���������� �ߺ��� �߰��ȴ�. 
	
	
	//�� ���ڵ� �ߺ�
	int Overlap_Search_perLine[MAX_ONELINE][2];//�� ���ڵ��� �ߺ��˻�
	bool newchar_perLine = true; //�� ���ڵ��� �ߺ��������� Ȯ��
	int index_of_Overlap_Search_perLine;//�ε���
	
	//��ü DB �ߺ� ���̺� ������
	bool newoverlapchar = true; //��ü DB�� �ߺ��������� Ȯ��
	int index_of_Overlap_Count_perChar=0;//�ε���

	
	for(int i=0; i<MAX_ONELINE; i++){//���Ű��� ������ for���� �ʱ�ȭ���� for�� �Ʒ��� �������� �־���� 
		Overlap_Search_perLine[i][0]=0;
		Overlap_Search_perLine[i][1]=0;
	}
	for(int i=0; i<MAX_OVERLAP;i++){
		Overlap_Count_perChar_tint[i]=0;
		Overlap_Count_perChar_count[i]=0;
	}
	//�ߺ����� ���� �ذ�κг�

	unsigned char output_test[HANGUL_BYTE];//������ ���
	

	// ��� ���� ��Ī�κ� ���ڵ��� d��� RBƮ���� �ִ´�
	for (unsigned int i = 0; i < db_line_count; i++) {	// ����� ��� �ٿ� ���ؼ� ������ ����.
		
		//�ߺ����� ���� �ذ�κ�
		index_of_Overlap_Search_perLine= 0;//�� ���ڵ� �ߺ��˻� �ε��� �ʱ�ȭ
			
		for(int i=0; i<MAX_ONELINE; i++){
			//�� ���ڵ� �ߺ��˻� �迭 �ʱ�ȭ
			Overlap_Search_perLine[i][0]=0;
			Overlap_Search_perLine[i][1]=0;
		}
		//�ߺ����� ���� �ذ�κг�
				
		cur = position_in_memory[i];		// �� ���� ������ ����� memory_db[] ���� ���� ��ġ
		len = namepart_len[i];	// �� �ٿ� ���Ե� �̸� �κ�('@' �� �κ�)�� ����

		for (;;) {
			t_char = memory_db[cur++]; 
			len--;		// �� ���� �� �̸� �κ��� ���̸� �� ������.
			

			//�ߺ����� ���� �ذ�κ�
			thatchar = false;
			//��

			if ((unsigned int)t_char >= 128) {
				han[0] = t_char;
				
				t_char = memory_db[cur++]; 
				len--;

				han[1] = t_char;
				han[2] = '\0';

				t_int = (int)(*(unsigned short*)han);
				
				total_chrnum_indb++;
				
				if(Max_t_int<t_int)
					Max_t_int=t_int;
				

				//�ߺ����� ���� �ذ�κ�
				thatchar = true;
				//��

				rbt1.insert(t_int, 0);			// ����-��Ʈ���� ���� �ϳ��� �����Ѵ�. �ٵ� �ڵ��� �յ� ����Ʈ�� �ڹٲ�� ���µ� �̰� �̻��� ����.

				
			}
			else { // ��ũ�� ������ �Ϲ� �ƽ�Ű ����
				if (t_char != 0) {				// �̸��κ��̳� �ּҺκ� ���ڿ� �� �ڿ� �ִ� �� ���ڸ� �����ϱ� ����.
					t_int = (int)t_char;
					rbt1.insert(t_int, 0);       // ����-��Ʈ���� ����
					
					//�ߺ����� ���� �ذ�κ�
					thatchar = true;
					//��
				}
			}

			
			//�ߺ����� ���� �ذ�κ�		
			//�� ���ڵ� ���� �ߺ����� Ȯ���ϴ� �κ�
			if(thatchar == true){
				newchar_perLine = true;
				
				for(int i=0; i<index_of_Overlap_Search_perLine; i++){
					if(Overlap_Search_perLine[i][0]==t_int){//�ߺ������̸� �����Ϳ� ����������Ŵ
						Overlap_Search_perLine[i][1]++;
						newchar_perLine = false;
						break;
					}
				}

				if(newchar_perLine == true){//�ߺ����ڰ� �ƴϸ� ���Ӱ� �߰���
					Overlap_Search_perLine[index_of_Overlap_Search_perLine][0]=t_int;
					Overlap_Search_perLine[index_of_Overlap_Search_perLine][1]++;
					index_of_Overlap_Search_perLine++;
				}
							
			}
			//�ߺ����� ���� �ذ�κг�

			if (len <= 0)
				break;
		} // end of for (;;)
		

		//�ߺ����� ���� �ذ�κ�
		for(int i=0; i<MAX_ONELINE; i++){
			//��ü DB �ߺ� ���ڿ� �� ���ڵ忡�� �˾Ƴ� ���ڵ� �����ϴ� �κ�
			if(Overlap_Search_perLine[i][1]>1){//�ߺ����ڶ��
				newoverlapchar = true;
				for(int t=0; t<index_of_Overlap_Count_perChar; t++)//�̹� �ߺ����ڷ� ����Ǿ� ������
					if(Overlap_Count_perChar_tint[t]==Overlap_Search_perLine[i][0]){
						newoverlapchar = false;
						if(Overlap_Count_perChar_count[t]<Overlap_Search_perLine[i][1])
							Overlap_Count_perChar_count[t]=Overlap_Search_perLine[i][1];
						break;
					}
				if(newoverlapchar == true){//���ο� �ߺ������̸�
					Overlap_Count_perChar_tint[index_of_Overlap_Count_perChar]=Overlap_Search_perLine[i][0];
					Overlap_Count_perChar_count[index_of_Overlap_Count_perChar]=Overlap_Search_perLine[i][1];
					index_of_Overlap_Count_perChar++;
				}
			}
		}
		//�ߺ����� ���� �ذ�κг�

	} // end of for (unsigned int i=0; i < db_line_count; i++)
	//printf("\n");
	count = 0;

	
	//�ߺ����� ���� �ذ�κ�
	han[0] = 0;//�迭 ���ʱ�ȭ
	han[1] = 0;
	han[2] = 0;
	
	//���
	printf("db�� �ִ� �� ���� ���� total_chrnum_indb = %d\n", total_chrnum_indb);
	printf("db�� �ִ� �� ����ū ���ڹ�ȣ Max_t_int = %d\n", Max_t_int);
	//printf("2000���� �迭 �� ������ á���� index_of_dbindex_int = %d\n", index_of_dbindex_int);

	q_sort(Overlap_Count_perChar_tint, Overlap_Count_perChar_count, 0, MAX_OVERLAP-1);//��ü DB�� �ߺ����� �迭�� sort��
	
	int counting = 0;//��ü DB �ߺ����� �迭�� ������ų�� ���//sorting�߱� ������ ����
	int overlap_num = 0;//�ߺ�Ƚ����ŭ �ε����� ������ �ø��� ���� ���
	//�ߺ����� ���� �ذ�κг�



	for (unsigned int i = 0; i < (unsigned int)0xFFFF; i++) {	// ��� ����(�ѱ� �Ǵ� �ƽ�Ű) �ڵ�鿡 ���ؼ�...�ϼ��� �ڵ��� ���� �� �ڵ尡 0xFDFF(65,023)
		result = rbt1.search(i);    // ù��° ����-��Ʈ������ ã�ƺ�
		if (result != NULL) {
			
			//�ߺ����� ���� �ذ�κ�
			while (Overlap_Count_perChar_tint[counting]<i){//��ü DB �ߺ����� �迭�� ������Ű�ٰ�
				counting++;
			}
			if(Overlap_Count_perChar_tint[counting]==i){//�ߺ����ڶ�� ������
				overlap_num = Overlap_Count_perChar_count[counting];//��ŭ �ε����� �߰��ؾ��ϴ��� ����
			}else{
				overlap_num = 1;
			}
			//�ߺ����� ���� �ذ�κг�


			for(int t = 0; t<overlap_num; t++){
				rbt2.insert(i, count);       // �ι�° ����-��Ʈ���� ����
				count++;
			}
		}
	}

	max_index = count;
	printf("���� �� Ʈ������ �� max_index = %d\n", max_index);

}

//�ߺ����� ���� �ذ�κ�(quick sort)
//������ �迭 1���� ���ڷ� ������ ���⿡���� �迭 2���� ���ڷ� ���´�.
void q_sort(int numbers_t_int[],int numbers_count[], int left, int right)
{
	int pivot, l_hold, r_hold, pivot_count;
	l_hold = left;
	r_hold = right;
	pivot = numbers_t_int[left];
	pivot_count = numbers_count[left];//�ι�° �迭

	while(left<right)
	{
		while((numbers_t_int[right] >= pivot) && (left < right))
			right--;
		if(left != right)
		{
			numbers_t_int[left] = numbers_t_int[right];
			numbers_count[left] = numbers_count[right];//�ι�° �迭
			left++;
		}
		while((numbers_t_int[left] <= pivot) && (left < right))
			left++;
		if(left != right)
		{
			numbers_t_int[right] = numbers_t_int[left];
			numbers_count[right] = numbers_count[left];//�ι�° �迭
			right--;
		}
	}
	numbers_t_int[left] = pivot;
	numbers_count[left] = pivot_count;//�ι�° �迭
	
	pivot = left;
	left = l_hold;
	right = r_hold;
	if(left<pivot)
		q_sort(numbers_t_int,numbers_count, left, pivot-1);
	if(right>pivot)
		q_sort(numbers_t_int,numbers_count, pivot+1, right);
}
//�ߺ����� ���� �ذ�κ�(quick sort)��


/*
 * ����� ��Ī�κп� �����ϴ� ��� ���ڵ鿡 ���ؼ� �ؽ�Ʈ �˻��� ���� ���ε����� �����.
 */
void
create_inverted_file_4text(void)
{
	register int index, i;
	int cur, len;
	int t_int;
	unsigned char han[HANGUL_BYTE+1];
	unsigned char t_char;
	Node *result;
	unsigned long inverted_index;
	unsigned int max_i_start = 0;
	unsigned int max_i_count;
	int *i_cur;

	i_count = (int *)malloc(max_index*sizeof(int));		// �۷ι� �迭. �˻� �� ����.
	i_start = (int *)malloc(max_index*sizeof(int));		// �۷ι� �迭. �˻� �� ����.
	i_cur = (int *)malloc(max_index*sizeof(int));		// ���� �迭. �˻� �� ��� �� ��.
	if (i_count == NULL || i_start == NULL || i_cur == NULL) {
		fprintf(stderr, "i_count[], i_start[], �Ǵ� i_cur[] �迭�� ���� %d���� �Ҵ��ϴ� ���� �����߽��ϴ�.\n", max_index);
		exit(-3);
	}

	FILE  *fp;
	if  ((fp = fopen(text_inverted_file_name, "rb")) != NULL) {	// �ϼ����� �������� �̹� ������� ������
		int tmp_max_index;

		printf("Loading the existing text inverted file...");
		
		fread(&tmp_max_index, sizeof(int), 1, fp);							// max_index ���� �о��
	
		if (tmp_max_index != max_index) {
			fprintf(stderr, "����� �ؽ�Ʈ�� �����Ͽ��� max_index ���� �߸��Ǿ����ϴ�. ������ ���ϴ�.\n");
			exit(-4);
		}
		fread(&MAX_SEARCHED_COL, sizeof(unsigned int), 1, fp);					// MAX_SEARCHED_COL ���� �о��
		fread(&MAX_BYTES_IN_INVERTED_FILE, sizeof(unsigned int), 1, fp);		// MAX_BYTES_IN_INVERTED_FILE ���� �о��
		fread(i_count, sizeof(int), max_index, fp);								// i_count[] �迭 �о��
		fread(i_start, sizeof(int), max_index, fp);								// i_start[] �迭 �о��
		inverted = (unsigned char *)malloc(sizeof(unsigned char)*MAX_BYTES_IN_INVERTED_FILE);
		
		if (inverted == NULL) {
			fprintf(stderr, "inverted[] �迭 �Ҵ翡 �����߽��ϴ�. �޸𸮰� �����մϴ�.\n");
			exit(-3);
		}
		fread(inverted, sizeof(unsigned char), MAX_BYTES_IN_INVERTED_FILE, fp);	// inverted[] �迭 �о��
		fclose(fp);	
		printf("done.\n");
			
	}
	else {
		for (i = 0; i < max_index; i++)
			i_count[i] = 0;

		//�ߺ����� ���� �ذ�κ�
		//��� �� ���ڵ�ȿ� �ߺ����θ� Ȯ���ϱ� ���� ������
		int Overlap_Search_perLine[MAX_ONELINE][2];
		bool newchar_perLine = true;
		
		int overlapnum;//�� ���ڵ忡�� �ߺ�Ƚ��

		bool create_inverted_switch = true;//���ε��� ������ �ѱ۰� �ѱ��� �ƴ� ������ �۾������ �ٸ��� ��������
		bool thatchar = false; //����ε� ���ڿ��� Ȯ�� //�� ������ ���� ��� null���� ������ ���������� �ߺ��� �߰��ȴ�. 
		//�ߺ����� ���� �ذ�κг�

		for (unsigned int z = 0; z < db_line_count; z++) {
			cur = position_in_memory[z];		// �� ���� ������ ����� memory_db[] ���� ���� ��ġ
			len = namepart_len[z];	// �� �ٿ� ���Ե� �̸� �κ�('@' �� �κ�)�� ����

			//�ߺ����� ���� �ذ�κ�
			int index_of_Overlap_Search_perLine= 0;//�� ���ڵ� �ߺ����� �迭 �ʱ�ȭ
			for(int i=0; i<MAX_ONELINE; i++){
				Overlap_Search_perLine[i][0]=0;
				Overlap_Search_perLine[i][1]=0;
			}
			//�ߺ����� ���� �ذ�κг�

			for (;;) {
				t_char = memory_db[cur++]; 
				len--;

				//�ߺ����� ���� �ذ�κ�
				thatchar = false;//�ʱ�ȭ
				//��

				if ((unsigned int)t_char >= 128) {
					han[0] = t_char;
				
					t_char = memory_db[cur++]; 
					len--;

					han[1] = t_char;
					han[2] = '\0';

					t_int = (int)(*(unsigned short*)han);

					//�ߺ����� ���� �ذ�κ�
					thatchar = true;
					//��
				}
				else { // ��ũ�� ������ �Ϲ� �ƽ�Ű ����
					if (t_char != 0) {				// �̸��κ��̳� �ּҺκ� ���ڿ� �� �ڿ� �ִ� �� ���ڸ� �����ϱ� ����.
						t_int = (int)t_char;

						//�ߺ����� ���� �ذ�κ�
						thatchar = true;
						//��
					}
				}

				//�ߺ����� ���� �ذ�κ�
				if(thatchar == true){
					newchar_perLine = true;
					overlapnum = 0;//�ߺ����ڸ� ������ �ε����� �ݿ��ϱ� ���� �߰�
					for(int i=0; i<index_of_Overlap_Search_perLine; i++){
						if(Overlap_Search_perLine[i][0]==t_int){//�ߺ������� ���
							Overlap_Search_perLine[i][1]++;
							newchar_perLine = false;
							//�ߺ����ڸ� ������ �ε����� �ݿ��ϱ� ���� �߰�
							overlapnum = Overlap_Search_perLine[i][1]-1;//-1�߰� => �����ذ�   
							break;
						}
					}

					if(newchar_perLine == true){//���ο� ������ ���
						Overlap_Search_perLine[index_of_Overlap_Search_perLine][0]=t_int;
						Overlap_Search_perLine[index_of_Overlap_Search_perLine][1]++;
						index_of_Overlap_Search_perLine++;
					}
				//�ߺ����� ���� �ذ�κг�

					
					if (result = rbt2.search(t_int)) {	
							index = result->index;
							
							//�ߺ����� ���� �ذ�κ�
							index = index + overlapnum;//�ߺ���ŭ �ε�������
							//��

							i_count[index]++;
					}
					else
						index = -1;
					}
					

				if (len <= 0)
					break;
			} // end of for (;;)
		} // for (unsigned int z = 0; z < db_line_count; z++)


		i_start[0] = 0;
		max_i_count = i_count[0];
		for (i = 1; i < max_index; i++) {	// ��� ������ ��� ���ڿ� ����...
			i_start[i] = i_start[i-1] + i_count[i-1]*3;		// �� ����Ʈ(24��Ʈ)�� ����Ѵٴ� ���� 2^24 = �� 1600�� �ٱ����� ��� ����ϰڴٴ� �ǹ�!!!
			if ((unsigned int)(i_start[i]+i_count[i]*3) > max_i_start)
				max_i_start = i_start[i] + i_count[i]*3;	// inverted[] ������ ���ݱ��� �ʿ��ϴٰ� �Ǵܵ� ������ ��+1 �ε���, �� ũ��
			if ((unsigned int)i_count[i] > max_i_count)
				max_i_count = i_count[i];		// � ���ڰ� ��� ������ �ٹ�ȣ�� ������ ���� ���� ��� �ٹ�ȣ ����
		}
		MAX_SEARCHED_COL = max_i_count;
		printf("\tinverted[]�� ũ��(max_i_start) = %d.\n", max_i_start);
		printf("\t���� ���� �ٿ��� ������ ������ ���� �ٹ�ȣ ����(max_i_count) = %d.\n", MAX_SEARCHED_COL);

		MAX_BYTES_IN_INVERTED_FILE = max_i_start;
		inverted = (unsigned char *)malloc(sizeof(unsigned char)*MAX_BYTES_IN_INVERTED_FILE);
		if (inverted == NULL) {
			fprintf(stderr, "inverted[] �迭 �Ҵ翡 �����߽��ϴ�. �̴� �ɰ��� �� �Դϴ�.\n");
			exit(-3);
		}
					
		// ���� ���ڰ� ���� �� ������ ��� ������ ���� ���� ����� ��� ���ο��� �����ߴ��� �˾ƾ� �Ѵ�. �� �� (����) �� ���ڸ� �����ϱ� ���� ������ 
		for (i = 0; i < max_index; i++)
			i_cur[i] = 0;

		printf("�ؽ�Ʈ Inverted[] ���� ����...");
		
		
		// --------------------------------------------------- ������ ������ ----------------------------
		for (unsigned int i = 0; i < db_line_count; i++) {
			cur = position_in_memory[i];		// �� ���� ������ ����� memory_db[] ���� ���� ��ġ
			len = namepart_len[i];	// �� �ٿ� ���Ե� �̸� �κ�('@' �� �κ�)�� ����
			
			
			//�ߺ����� ���� �ذ�κ�
			int index_of_Overlap_Search_perLine= 0;//�� ���ڵ� �ߺ����� �迭 �ʱ�ȭ
			for(int t=0; t<MAX_ONELINE; t++){
				Overlap_Search_perLine[t][0]=0;
				Overlap_Search_perLine[t][1]=0;
			}
			//�ߺ����� ���� �ذ�κг�
			
			for (;;) {
				t_char = memory_db[cur++];
				len--;
				
				//�ߺ����� ���� �ذ�κ�
				thatchar = false;//�ʱ�ȭ
				//��
				
				if ((unsigned int)t_char >= 128) {
					han[0] = t_char;
				
					t_char = memory_db[cur++]; 
					len--;

					han[1] = t_char;
					han[2] = '\0';

					t_int = (int)(*(unsigned short*)han);

					//�ߺ����� ���� �ذ�κ�
					create_inverted_switch =true;
					thatchar = true;
					//��

					//�ߺ����� ���� �ذ��� ���� �ּ�ó���� �κ�
					/*if (result = rbt2.search(t_int)) {	
						index = result->index;  
						//fprintf(p_file0, "(%d)\n", index);
						// �� ���ڰ� ���� �� ������ �ٵ� ������ ���� ���� ����� �� ��ȣ�� inverted �迭�� �־�д�.
						// ���� �� ������ i_start[] �� �ε���(�����Ʈ������ ã�ƺ��� �� �� ����)�� �˸� ���� ����� �� �� �ٿ� �����ߴ��� �� �� ����.
						// ��, � ���ڰ� ������ �� ���ڸ� �����Ʈ������ �˻��Ͽ� �ε����� �˾Ƴ���, �ε����� �˸� ���, ���, ��� �ٿ��� ���������� �� �� �ִ�.
						// �� �ٹ�ȣ ������ ���� 3����Ʈ�� ����. �׷��� �뷫 1600���� �̻��� Ŀ���� �� �ִ�. ���� ����� �� ���� 2^24�� ������ 3�� 4�� �ٲ���� ��.
						inverted_index = i_start[index] + i_cur[index]*3;
						if (inverted_index > (unsigned long)MAX_BYTES_IN_INVERTED_FILE-3) {
							fprintf(stderr, "inverted[] �迭���� �����÷ο찡 �����ϴ�(�����ڵ� -4). inverted_index = %d\n", inverted_index);
							exit(-4);
						}
						inverted[inverted_index] = (i >> 16) & 255; 
						inverted[inverted_index + 1] = (i >> 8) & 255;
						inverted[inverted_index + 2] = i & 255;
						i_cur[index]++;		// ���� ���ڰ� ���� �� �ִ� ��� ���̾� �����ϱ� ���� inverted �迭���� �ٷ� ���� ��ġ�� ����.
					}*/
				}
				else { // ��ũ�� ������ �Ϲ� �ƽ�Ű ����
					if (t_char != 0) {				// �̸��κ��̳� �ּҺκ� ���ڿ� �� �ڿ� �ִ� �� ���ڸ� �����ϱ� ����.
						t_int = (int)t_char;

						//�ߺ����� ���� �ذ�κ�
						create_inverted_switch =false;
						thatchar = true;
						//��

						//�ߺ����� ���� �ذ��� ���� �ּ�ó���� �κ�
						/*if (result = rbt2.search(t_int)) {
							index = result->index;  
							inverted_index = i_start[index] + i_cur[index]*3;
							if (inverted_index > MAX_BYTES_IN_INVERTED_FILE-3) {
								fprintf(stderr, "inverted[] �迭���� �����÷ο찡 �����ϴ�(�����ڵ� -5). inverted_index = %d\n", inverted_index);
								exit(-5);
							}
							inverted[inverted_index] = (i >> 16) & 255; 
							inverted[inverted_index + 1] = (i >> 8) & 255; 
							inverted[inverted_index + 2] = i & 255; 
							i_cur[index]++;
						}*/
					}
				}


				//�ߺ����� ���� �ذ�κ�
				if(thatchar == true){
					newchar_perLine = true;
					overlapnum = 0;//�ߺ����ڸ� ������ �ε����� �ݿ��ϱ� ���� �߰�
					for(int t=0; t<index_of_Overlap_Search_perLine; t++){
						if(Overlap_Search_perLine[t][0]==t_int){//�ߺ������� ���
							Overlap_Search_perLine[t][1]++;
							newchar_perLine = false;
							//�ߺ����ڸ� ������ �ε����� �ݿ��ϱ� ���� �߰�
							overlapnum = Overlap_Search_perLine[t][1]-1;// -1�߰� => �����ذ�
							break;
						}
					}
					if(newchar_perLine == true){//���ο� ������ ���
						Overlap_Search_perLine[index_of_Overlap_Search_perLine][0]=t_int;
						Overlap_Search_perLine[index_of_Overlap_Search_perLine][1]++;
						index_of_Overlap_Search_perLine++;
					}
				//�ߺ����� ���� �ذ�κг�
				
					if (result = rbt2.search(t_int)) {
						index = result->index; 

						//�ߺ����� ���� �ذ�κ�
						index = index + overlapnum;
						//��

						inverted_index = i_start[index] + i_cur[index]*3;

						if(create_inverted_switch ==true){
							if (inverted_index > (unsigned long)MAX_BYTES_IN_INVERTED_FILE-3) {
								fprintf(stderr, "inverted[] �迭���� �����÷ο찡 �����ϴ�(�����ڵ� -4). inverted_index = %d\n", inverted_index);
								exit(-4);
							}else{}
						}
						else{
							if (inverted_index > MAX_BYTES_IN_INVERTED_FILE-3) {
									fprintf(stderr, "inverted[] �迭���� �����÷ο찡 �����ϴ�(�����ڵ� -5). inverted_index = %d\n", inverted_index);
									exit(-5);
							}else{}
						}
						
						inverted[inverted_index] = (i >> 16) & 255; 
						inverted[inverted_index + 1] = (i >> 8) & 255; 
						inverted[inverted_index + 2] = i & 255; 
						i_cur[index]++;
					}
				}

				if (len <= 0)		// � �� ���� name �κ� ó���� �������� ���� �ٷ� ����.
					break;
			} // end of for (;;)
		}// end of for (unsigned int i=0; i < entry_count; i++) {
		// -------------------------------------- �����ϻ����� �� --------------------------------------
		printf("done.\n");
	
		free(i_cur);	// �� �����Ƿ� ��ȯ�ص� ��.
		
		fp = fopen(text_inverted_file_name, "wb");	// �ϼ����� �������� ���Ͽ� �����Ͽ� ����� �׳� �о���� �� �ֵ��� �Ѵ�
		if (fp == NULL) {
			fprintf(stderr, "�ؽ�Ʈ�� ������ ������ ���� ���� ���� ����!\n");
			exit(-5);
		}

		printf("Saving the text inverted file...");

		fwrite(&max_index, sizeof(int), 1, fp);										// max_index ���� ����
		fwrite(&MAX_SEARCHED_COL, sizeof(unsigned int), 1, fp);						// MAX_SEARCHED_COL ���� ����
		fwrite(&MAX_BYTES_IN_INVERTED_FILE, sizeof(unsigned int), 1, fp);			// MAX_BYTES_IN_INVERTED_FILE ���� ����
		fwrite(i_count, sizeof(int), max_index, fp);								// i_count[] �迭 ����
		fwrite(i_start, sizeof(int), max_index, fp);								// i_start[] �迭 ����
		fwrite(inverted, sizeof(unsigned char), MAX_BYTES_IN_INVERTED_FILE, fp);	// inverted[] �迭 ����
		fclose(fp);	
		printf("done.\n");
	}

	// �˻� �� ����� searched[MAX_QUERY_LEN][MAX_SEARCHED_COL]�� �̸� �Ҵ��صд�.
	searched = (int **)malloc(sizeof(int *)*MAX_QUERY_LEN);
	if (searched == NULL) {
		fprintf(stderr, "searched[] �迭 �Ҵ翡 �����߽��ϴ�. �̴� �ɰ��� �� �Դϴ�.\n");
		exit(-3);
	}
	for (i = 0; i < MAX_QUERY_LEN; i++) {
		searched[i] = (int *)malloc(sizeof(int)*MAX_SEARCHED_COL);
		if (searched[i] == NULL) {
			fprintf(stderr, "searched[][] �迭 �Ҵ翡 �����߽��ϴ�. �̴� �ɰ��� �� �Դϴ�.\n");
			exit(-3);
		}
	}

	// �˻� �� ����� final[MAX_QUERY_LEN][MAX_SEARCHED_COL]�� �̸� �Ҵ��صд�.
	final = (int **)malloc(sizeof(int *)*MAX_QUERY_LEN);
	if (final == NULL) {
		fprintf(stderr, "final[] �迭 �Ҵ翡 �����߽��ϴ�. �̴� �ɰ��� �� �Դϴ�.\n");
		exit(-3);
	}
	for (i = 0; i < MAX_QUERY_LEN; i++) {
		final[i] = (int *)malloc(sizeof(int)*MAX_SEARCHED_COL);
		if (final[i] == NULL) {
			fprintf(stderr, "final[][] �迭 �Ҵ翡 �����߽��ϴ�. �̴� �ɰ��� �� �Դϴ�.\n");
			exit(-3);
		}
	}
}

/*
 * �ʼ� 2,3�ε����� ���ε��� ������
 * create_inverted_file_4chosung(void);
 */
void
create_inverted_file_4chosung(void)
{
	int m, k, w, i;						// ���� �ε��� ����
	unsigned int z;						//        ;;
	int cur = 0;						// memory[] �迭 ������ �� ���� �����ϴ� ��ġ�� ������ �� ���.
	short cho[MAX_MOEUM_CNT];			// ��Ī�κп� ������ ���� �ڵ�� �迭. �� ���� ó���� �� �Ź� �����.
	short cho_ad[MAX_MOEUM_CNT];		// �ּҺκп� ������ ���� �ڵ�� �迭. �� ���� ó���� �� �Ź� �����.
	short c_count = 0;					// ��Ī�κ��� ���� ������ ���� ���� ���
	short c_ad_count = 0;				// �ּҺκ��� ���� ������ ���� ���� ���
	char han[HANGUL_BYTE+1];			// �ѱ� ó�� �� ���� �ϳ��� ��� ���� ���� ����. NULL ���ڱ��� �����ϱ� ���� +1.
	char c_han[HANGUL_BYTE+1];			// �ϼ��� ���ڸ� ���������� �ٲ� �� ����ϱ� ���� ����.
	unsigned short johapcode;			// �ڵ� ��ȯ �� �ӽ÷� ���
	unsigned short cho_code;			// �ʼ� �и� �� �ӽ÷� ���
	int c_index;						// 3�����ʼ� �Ǵ� 2�����ʼ��� 38���� ID�� ��ȯ�� �� �ӽ÷� ���. ��, 3�����ʼ� �Ǵ� 2�����ʼ��� ID�� ���� �뵵.
	unsigned int inverted_index, inverted2_index;
	unsigned int max_cho3_start = 0, max_cho3_count = 0;
	unsigned int max_cho2_start = 0, max_cho2_count = 0;
	char currency[20], currency2[20];	// ���ڸ� õ ������ �޸��� �ֱ� ���� �迭
	FILE* fp;
	//static int tmp1=0, tmp2=0, tmp3=0;
	unsigned int c_cur[MAX_3MOEUM_INDEX];	// 3�����ʼ� �����Ͽ� �ٹ�ȣ�� ����� �� ���� 3�����ʼ��� ���� �� ������ ��� �� ���� ��ġ�� ������ų �� ���
	unsigned int c_cur2[MAX_2MOEUM_INDEX];	// 2�����ʼ� �����Ͽ� �ٹ�ȣ�� ����� �� ���� 2�����ʼ��� ���� �� ������ ��� �� ���� ��ġ�� ������ų �� ���

	if  ((fp = fopen(chosung_inverted_file_name, "rb")) != NULL) {	// �ʼ��� �������� �̹� ������� ������
		printf("Loading the �ʼ��� inverted file...");

		fread(&MAX_FORMER_LATER_CNT, sizeof(unsigned int), 1, fp);		// MAX_FORMER_LATER_CNT ���� ����
		fread(&MIN_FORMER_LATER_CNT, sizeof(unsigned int), 1, fp);		// MIN_FORMER_LATER_CNT ���� ����
		fread(&MAX_INVFILE3_LEN, sizeof(unsigned int), 1, fp);			// MAX_INVFILE3_LEN ���� ����
		fread(&MAX_INVFILE2_LEN, sizeof(unsigned int), 1, fp);			// MAX_INVFILE2_LEN ���� ����
		fread(cho3_start, sizeof(unsigned int), MAX_3MOEUM_INDEX, fp);	// cho3_start[] �迭 ����
		fread(cho2_start, sizeof(unsigned int), MAX_2MOEUM_INDEX, fp);	// cho2_start[] �迭 ����
		cho3_inverted = (unsigned char *)malloc(MAX_INVFILE3_LEN);
		cho2_inverted = (unsigned char *)malloc(MAX_INVFILE2_LEN);
		if (cho3_inverted == NULL || cho2_inverted == NULL) {
			fprintf(stderr, "\tcho3_inverted[] �Ǵ� cho2_inverted[] �迭 �Ҵ翡 ����. �޸� ������ �̴ϴ�.\n");
			exit(-4);
		}
		fread(cho3_inverted, sizeof(unsigned char), MAX_INVFILE3_LEN, fp);	// cho3_inverted[] �迭 ����
		fread(cho2_inverted, sizeof(unsigned char), MAX_INVFILE2_LEN, fp);	// cho2_inverted[] �迭 ����
		fclose(fp);	
		printf("done.\n");
	}
	else {
		printf("�ʼ� 2,3 ���ε��� ��������...\n");
		memset(c_cur, 0, sizeof(int)*MAX_3MOEUM_INDEX);		// �ʱⰪ�� �ݵ�� 0�̾�� ��.
		memset(c_cur2, 0, sizeof(int)*MAX_2MOEUM_INDEX);	// �ʱⰪ�� �ݵ�� 0�̾�� ��.
		cho3_start[0] = 0;
		cho2_start[0] = 0;

		for (m = 1; m < MAX_3MOEUM_INDEX; m++) {	// ��� ������ 3�����ʼ� ID�鿡 ����
			cho3_start[m] = cho3_start[m-1] + cho3_count[m-1]*3;
			if ((cho3_start[m] + cho3_count[m]*3) > max_cho3_start)
				max_cho3_start = cho3_start[m] + cho3_count[m]*3;	// cho3_inverted[] �迭�� �ִ� ���̸� �˾Ƴ���.
			if (cho3_count[m] > max_cho3_count)
				max_cho3_count = cho3_count[m];	// ���� �ٹ�ȣ�� ���� ������ �ִ� 3�����ʼ��� �ٹ�ȣ ������ �˾Ƴ���.
		}
		for (m = 1; m < MAX_2MOEUM_INDEX; m++) {	// ��� ������ 2�����ʼ� ID�鿡 ����
			cho2_start[m] = cho2_start[m-1] + cho2_count[m-1]*3;
			if ((cho2_start[m]+cho2_count[m]) > max_cho2_start)
				max_cho2_start = cho2_start[m]+cho2_count[m];	// cho2_inverted[] �迭�� �ִ� ���̸� �˾Ƴ���.
			if (cho2_count[m] > max_cho2_count)
				max_cho2_count = cho2_count[m];	// ���� �ٹ�ȣ�� ���� ������ �ִ� 2�����ʼ��� �ٹ�ȣ ������ �˾Ƴ���.
		}
		if (max_cho3_count > max_cho2_count) {
			MAX_FORMER_LATER_CNT = max_cho3_count;
			MIN_FORMER_LATER_CNT = max_cho2_count;
		}
		else {
			MAX_FORMER_LATER_CNT = max_cho2_count;
			MIN_FORMER_LATER_CNT = max_cho3_count;
		}

		//printf("\tcho3_inverted[]�� ũ��: %s\n\tcho2_inverted[]�� ũ��: %s\n", Int2Currency(max_cho3_start, currency, false), Int2Currency(max_cho2_start, currency2, false));
		//printf("\tcho3_count[]�� �ִ밪 = %s\n\tcho2_count[]�� �ִ밪 = %s\n", Int2Currency(max_cho3_count, currency, false), Int2Currency(max_cho2_count, currency2, false));
		//printf("\tMAX_FORMER_LATER_CNT = %s\n\tMIN_FORMER_LATER_CNT = %s\n", Int2Currency(MAX_FORMER_LATER_CNT, currency, false), Int2Currency(MIN_FORMER_LATER_CNT, currency2, false));

		MAX_INVFILE3_LEN = max_cho3_start;
		cho3_inverted = (unsigned char *)malloc(MAX_INVFILE3_LEN);
		MAX_INVFILE2_LEN = max_cho2_start;
		cho2_inverted = (unsigned char *)malloc(MAX_INVFILE2_LEN);
		if (cho3_inverted == NULL || cho2_inverted == NULL) {
			fprintf(stderr, "\tcho3_inverted[] �Ǵ� cho2_inverted[] �迭 �Ҵ翡 �����߽��ϴ�. �̴� �ɰ��� �����Դϴ�.\n");
			exit(-4);
		}

		for (z = 0; z < db_line_count; z++) {		// ��� �ٿ� ���ؼ�
			cur = position_in_memory[z];	// memory[] �迭 ������ �� ���� ���� ��ġ
			c_count = 0;
			c_ad_count = 0;

			for (k = 0; k < namepart_len[z];) {
				han[0] = memory_db[cur++];
				if ((unsigned short)han[0] >= 128) {
					han[1] = memory_db[cur++];
					han[2] = '\0';
					HangulCodeConvert(KS, han, c_han);		// �ϼ��� �ѱ��ڵ� 16��Ʈ�� ������ 16��Ʈ�� �ٲ۴�.
					johapcode = ((unsigned char)c_han[0] << 8) | (unsigned char)c_han[1];
					cho_code = (johapcode >> 10) & 0x1f;	// ���� 10��Ʈ�� ���ּ� �ʼ� �ڵ带 �����Ѵ�.
					cho[c_count++] = cho_code;
					k += 2;									// �ѱ��̴ϱ� 2����Ʈ ����
				}
#ifdef	IGNORE_NON_HANGUL
				else
					k++;
#else
				else if (han[0] >= '0' && han[0] <= '9') {	// ���ڴ� �ʼ��� �� �ڵ� '��'(0x14)�� ������ ��ġ�Ѵٰ� ����
					cho[c_count++] = han[0] - '0' + 0x15;
					k++;
				}
				else if (han[0] == 0)						// �޸� ��񿡼� �̸��κа� �ּҺκ� ���� �� ���ڸ� �� �־����Ƿ� �����Ѵ�.
					k++;
				else {										// ���ڸ� ������ ���ĺ� ���� �ƽ�Ű �ڵ�� �ʼ� ������ �Ұ���! �ʼ� �ڵ忡 0�� �־�д�.
					cho[c_count++] = 0;
					k++;
				}
#endif
				if (c_count >= MAX_MOEUM_CNT-1) {
					fprintf(stderr, "\tcho[] �迭���� �����÷ο찡 �߻��߽��ϴ�.\n");
					exit(-3);
				}
			}
			// ��� ��Ī �κп� �����ϴ� �ʼ����� �迭 �Ϸ�

			for (w = 0; w < addrpart_len[z];) {
				han[0] = memory_db[cur++];
				if ((unsigned short)han[0] >= 128) {
					han[1] = memory_db[cur++];
					han[2] = '\0';
					HangulCodeConvert(KS, han, c_han);		// �ϼ��� �ѱ��ڵ� 16��Ʈ�� ������ 16��Ʈ�� �ٲ۴�.
					johapcode = ((unsigned char)c_han[0] << 8) | (unsigned char)c_han[1];
					cho_code = (johapcode >> 10) & 0x1f;	// ���� 10��Ʈ�� ���ּ� �ʼ� �ڵ带 �����Ѵ�.
					cho_ad[c_ad_count++] = cho_code;
					w += 2;									// �ѱ��̴ϱ� 2����Ʈ ����
				}
#ifdef	IGNORE_NON_HANGUL
				else
					w++;
#else
				else if (han[0] >= '0' && han[0] <= '9') {	// ����
					cho_ad[c_ad_count++] = han[0] - '0' + 0x15;
					w++;
				}
				else if (han[0] == 0)						// �޸� ��񿡼� �̸��κа� �ּҺκ� ���� �� ���ڸ� �� �־����Ƿ� �����Ѵ�.
					w++;
				else{
					cho_ad[c_ad_count++] = 0;				// ��Ÿ �ƽ�Ű �ڵ�� �ʼ� ������ �Ұ���! �ʼ� �ڵ忡 0�� �־�д�.
					w++;
				}
#endif
				if (c_ad_count >= MAX_MOEUM_CNT-1) {
					fprintf(stderr, "\tcho_ad[] �迭���� �����÷ο찡 �߻��߽��ϴ�.\n");
					exit(-3);
				}
			}
			// ��� �ּ� �κп� �����ϴ� �ʼ����� �迭 �Ϸ�
		
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
			if (c_count == 1) {	// �� ���� �ʼ� ������ 1�̸�, �����ζ� 3�����ʼ� ID�� 2�����ʼ� ID�� �����
								// �ű⿡ �ش��ϴ� inverted[]�� �ٹ�ȣ 24��Ʈ�� �ִ´�.
				c_index = cho[0]*38*38;
				inverted_index = cho3_start[c_index] + c_cur[c_index]*3;
				if (inverted_index >= MAX_INVFILE3_LEN-2) {
					fprintf(stderr, "�ٹ�ȣ %d: cho3_inverted[] �迭���� �����÷ο찡 �߻��߽��ϴ�(%u). �ø�����.\n", z+1, inverted_index);
					exit(-2);
				}
				cho3_inverted[inverted_index] = (z >> 16) & 255; 
				cho3_inverted[inverted_index + 1] = (z >> 8) & 255;
				cho3_inverted[inverted_index + 2] = z & 255; 
				c_cur[c_index]++;

				c_index = cho[0]*38;
				inverted2_index = cho2_start[c_index] + c_cur2[c_index]*3;
				if (inverted2_index >= MAX_INVFILE2_LEN-2) {
					fprintf(stderr, "�ٹ�ȣ %d: cho2_inverted[] �迭���� �����÷ο찡 �߻��߽��ϴ�(%u). �ø�����.\n", z+1, inverted2_index);
					exit(-2);
				}
				cho2_inverted[inverted2_index] = (z >> 16) & 255; 
				cho2_inverted[inverted2_index + 1] = (z >> 8) & 255; 
				cho2_inverted[inverted2_index + 2] = z & 255;
				c_cur2[c_index]++;
			}
			else if (c_count == 2) {	// �� ���� ���� ���� ������ 2�̸�, �����ζ� 3���Ӹ��� ID�� ����� ���⿡ �ش��ϴ� inverted[]�� �ٹ�ȣ�� �ְ�
										// 2���Ӹ��� ID�� �翬�� �� ����� ���� ����� �ű⿡ �ش��ϴ� inverted[]�� �ٹ�ȣ 24��Ʈ�� �ִ´�.
				c_index = cho[0]*38*38 + cho[1]*38;
				inverted_index = cho3_start[c_index] + c_cur[c_index]*3;
				if (inverted_index >= MAX_INVFILE3_LEN-2) {
					fprintf(stderr, "�ٹ�ȣ %d: cho3_inverted[] �迭���� �����÷ο찡 �߻��߽��ϴ�(%u). �ø�����.\n", z+1, inverted_index);
					exit(-2);
				}
				cho3_inverted[inverted_index] = (z >> 16) & 255; 
				cho3_inverted[inverted_index + 1] = (z >> 8) & 255; 
				cho3_inverted[inverted_index + 2] = z & 255; 
				c_cur[c_index]++;	

				c_index = cho[0]*38 + cho[1];
				inverted2_index = cho2_start[c_index] + c_cur2[c_index]*3;
				if (inverted2_index >= MAX_INVFILE2_LEN-2) {
					fprintf(stderr, "�ٹ�ȣ %d: cho2_inverted[] �迭���� �����÷ο찡 �߻��߽��ϴ�(%u). �ø�����.\n", z+1, inverted2_index);
					exit(-2);
				}
				cho2_inverted[inverted2_index] = (z >> 16) & 255; 
				cho2_inverted[inverted2_index + 1] = (z >> 8) & 255; 
				cho2_inverted[inverted2_index + 2] = z & 255; 
				c_cur2[c_index]++;	
			}
			else if (c_count >= 3) {	// �� ���� ���� �ʼ� ������ �� �� �̻��̸�, ��� 3���� �ʼ� ���հ� ��� 2���� �ʼ� ���յ��� ID�� ����ϰ�, inverted[] ���� �� �ڸ���
					// �ٹ�ȣ 24��Ʈ�� �ִ´�.
				for (i = 0; i < c_count-2; i++) {	// ��� 3�����ʼ� ���� ������ �� ��ȣ�� inverted[] �迭�� �ִ´�.
					c_index = cho[i]*38*38 + cho[i+1]*38 + cho[i+2];
					inverted_index = cho3_start[c_index] + c_cur[c_index]*3;
					if (inverted_index >= MAX_INVFILE3_LEN-2) {
						fprintf(stderr, "�ٹ�ȣ %d: cho3_inverted[] �迭���� �����÷ο찡 �߻��߽��ϴ�(%u). �ø�����.\n", z+1, inverted_index);
						exit(-2);
					}
					cho3_inverted[inverted_index] = (z >> 16) & 255; 
					cho3_inverted[inverted_index + 1] = (z >> 8) & 255; 
					cho3_inverted[inverted_index + 2] = z & 255; 
					c_cur[c_index]++;
				}
				for (i = 0; i < c_count-1 ; i++) {	// ��� 2�����ʼ� ���� ������ �� ��ȣ�� inverted[] �迭�� �ִ´�.
					c_index = cho[i]*38 + cho[i+1];
					inverted2_index = cho2_start[c_index] + c_cur2[c_index]*3;
					if (inverted2_index >= MAX_INVFILE2_LEN-2) {
						fprintf(stderr, "�ٹ�ȣ %d: cho2_inverted[] �迭���� �����÷ο찡 �߻��߽��ϴ�(%u). �ø�����.\n", z+1, inverted2_index);
						exit(-2);
					}
					cho2_inverted[inverted2_index] = (z >> 16) & 255; 
					cho2_inverted[inverted2_index + 1] = (z >> 8) & 255; 
					cho2_inverted[inverted2_index + 2] = z & 255;
					//if (c_index == 426) {
					//	printf("%d <= (%d)", z, inverted2_index);
					//}
					c_cur2[c_index]++;
				}
			}
		
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// �ּ� �κп� ���ؼ��� �Ȱ��� ������� �������� �����.
			if (c_ad_count == 1) {
				c_index = cho_ad[0]*38*38;
				inverted_index = cho3_start[c_index] + c_cur[c_index]*3;
				if (inverted_index >= MAX_INVFILE3_LEN-2) {
					fprintf(stderr, "�ٹ�ȣ %d: cho3_inverted[] �迭���� �����÷ο찡 �߻��߽��ϴ�(%u). �ø�����.\n", z+1, inverted_index);
					exit(-2);
				}
				cho3_inverted[inverted_index] = (z >> 16) & 255; 
				cho3_inverted[inverted_index + 1] = (z >> 8) & 255; 
				cho3_inverted[inverted_index + 2] = z & 255; 
				c_cur[c_index]++;

				c_index = cho_ad[0]*38;
				inverted2_index = cho2_start[c_index] + c_cur2[c_index]*3;
				if (inverted2_index >= MAX_INVFILE2_LEN-2) {
					fprintf(stderr, "�ٹ�ȣ %d: cho2_inverted[] �迭���� �����÷ο찡 �߻��߽��ϴ�(%u). �ø�����.\n", z+1, inverted2_index);
					exit(-2);
				}
				cho2_inverted[inverted2_index] = (z >> 16) & 255; 
				cho2_inverted[inverted2_index + 1] = (z >> 8) & 255; 
				cho2_inverted[inverted2_index + 2] = z & 255; 
				c_cur2[c_index]++;
			}
			else if (c_ad_count == 2) {
				c_index = cho_ad[0]*38*38+ cho_ad[1]*38;
				inverted_index = cho3_start[c_index] + c_cur[c_index]*3;
				if (inverted_index >= MAX_INVFILE3_LEN-2) {
					fprintf(stderr, "�ٹ�ȣ %d: cho3_inverted[] �迭���� �����÷ο찡 �߻��߽��ϴ�(%u). �ø�����.\n", z+1, inverted_index);
					exit(-2);
				}
				cho3_inverted[inverted_index] = (z >> 16) & 255; 
				cho3_inverted[inverted_index + 1] = (z >> 8) & 255; 
				cho3_inverted[inverted_index + 2] = z & 255; 
				//if (c_index == 701)
				//	fprintf(p_file1,"%d\n",z);
				c_cur[c_index]++;	

				c_index = cho_ad[0]*38+ cho_ad[1];
				inverted2_index = cho2_start[c_index] + c_cur2[c_index]*3;
				if (inverted2_index >= MAX_INVFILE2_LEN-2) {
					fprintf(stderr, "�ٹ�ȣ %d: cho2_inverted[] �迭���� �����÷ο찡 �߻��߽��ϴ�(%u). �ø�����.\n", z+1, inverted2_index);
					exit(-2);
				}
				cho2_inverted[inverted2_index] = (z >> 16) & 255; 
				cho2_inverted[inverted2_index + 1] = (z >> 8) & 255; 
				cho2_inverted[inverted2_index + 2] = z & 255; 
				c_cur2[c_index]++;	
			}
			else if (c_ad_count >= 3) {
				for (i = 0; i < c_ad_count-2 ; i++) {
					c_index = cho_ad[i]*38*38 + cho_ad[i+1]*38 + cho_ad[i+2];
					inverted_index = cho3_start[c_index] + c_cur[c_index]*3;
					if (inverted_index >= MAX_INVFILE3_LEN-2) {
						fprintf(stderr, "�ٹ�ȣ %d: cho3_inverted[] �迭���� �����÷ο찡 �߻��߽��ϴ�(%u). �ø�����.\n", z+1, inverted_index);
						exit(-2);
					}
					cho3_inverted[inverted_index] = (z >> 16) & 255; 
					cho3_inverted[inverted_index + 1] = (z >> 8) & 255; 
					cho3_inverted[inverted_index + 2] = z & 255; 
					c_cur[c_index]++;
				
				}
				for (i = 0; i < c_ad_count-1 ; i++) {
					c_index = cho_ad[i]*38 + cho_ad[i+1];
					inverted2_index = cho2_start[c_index] + c_cur2[c_index]*3;
					if (inverted2_index >= MAX_INVFILE2_LEN-2) {
						fprintf(stderr, "�ٹ�ȣ %d: cho2_inverted[] �迭���� �����÷ο찡 �߻��߽��ϴ�(%u). �ø�����.\n", z+1, inverted2_index);
						exit(-2);
					}
					cho2_inverted[inverted2_index] = (z >> 16) & 255; 
					cho2_inverted[inverted2_index + 1] = (z >> 8) & 255; 
					cho2_inverted[inverted2_index + 2] = z & 255; 
					c_cur2[c_index]++;
				}
			}
			//if (tmp1 != cho2_inverted[cho2_start[426]] || tmp2 != cho2_inverted[cho2_start[426]+1] || tmp3 != cho2_inverted[cho2_start[426]+2]) {
			//	printf("%d %d %d (%d %d %d)\n", z+1, cho2_count[426], cho2_start[426], cho2_inverted[cho2_start[426]], cho2_inverted[cho2_start[426]+1], cho2_inverted[cho2_start[426]+2]);
			//	tmp1 = cho2_inverted[cho2_start[426]]; tmp2 = cho2_inverted[cho2_start[426]+1]; tmp3 = cho2_inverted[cho2_start[426]+2];
			//}
		} // end of for (z = 0; z < db_line_count; z++)
		printf("Done.\n");
		// printf("cur : %d\n", cur);
		//printf("%d %d (%d %d %d)\n", cho2_count[426], cho2_start[426], cho2_inverted[cho2_start[426]], cho2_inverted[cho2_start[426]+1], cho2_inverted[cho2_start[426]+2]);

		// ������ �ʼ��˻��� ���ε����� ���� ������ ���Ͽ� �����صд�.
		fp = fopen(chosung_inverted_file_name, "wb");	// �ϼ����� �������� ���Ͽ� �����Ͽ� ����� �׳� �о���� �� �ֵ��� �Ѵ�
		if (fp == NULL) {
			fprintf(stderr, "�ʼ��� ������ ������ ���� ���� ���� ����!\n");
			exit(-6);
		}

		printf("Saving the �ʼ��� inverted file...");

		fwrite(&MAX_FORMER_LATER_CNT, sizeof(unsigned int), 1, fp);		// MAX_FORMER_LATER_CNT ���� ����
		fwrite(&MIN_FORMER_LATER_CNT, sizeof(unsigned int), 1, fp);		// MIN_FORMER_LATER_CNT ���� ����
		fwrite(&MAX_INVFILE3_LEN, sizeof(unsigned int), 1, fp);			// MAX_INVFILE3_LEN ���� ����
		fwrite(&MAX_INVFILE2_LEN, sizeof(unsigned int), 1, fp);			// MAX_INVFILE2_LEN ���� ����
		fwrite(cho3_start, sizeof(unsigned int), MAX_3MOEUM_INDEX, fp);	// cho3_start[] �迭 ����
		fwrite(cho2_start, sizeof(unsigned int), MAX_2MOEUM_INDEX, fp);	// cho2_start[] �迭 ����
		fwrite(cho3_inverted, sizeof(unsigned char), MAX_INVFILE3_LEN, fp);	// cho3_inverted[] �迭 ����
		fwrite(cho2_inverted, sizeof(unsigned char), MAX_INVFILE2_LEN, fp);	// cho2_inverted[] �迭 ����
		fclose(fp);	
		printf("done.\n");
	}

	// ���߿� �˻� ��ƾ���� ����� �ٹ�ȣ ���� �迭�� ���� �Ҵ��Ѵ�.
	former_list = (int *)malloc(sizeof(int)*MAX_FORMER_LATER_CNT);
	latter_list = (int *)malloc(sizeof(int)*MAX_FORMER_LATER_CNT);
	if (former_list == NULL || latter_list == NULL) {
		fprintf(stderr, "\tformer_list[] �Ǵ� later_list[] �Ҵ翡 �����߽��ϴ�. �̴� �ɰ��� �� �Դϴ�.\n");
		exit(-7);
	}

	// �ʼ� �˻� �� ����� cho_final[MAX_QUERY_LEN/2][MIN_FORMER_LATER_CNT]�� �̸� �Ҵ��صд�.
	cho_final = (int **)malloc(sizeof(int *)*MAX_QUERY_LEN/2);
	if (cho_final == NULL) {
		fprintf(stderr, "cho_final[] �迭 �Ҵ翡 �����߽��ϴ�. �̴� �ɰ��� �� �Դϴ�.\n");
		exit(-3);
	}
	for (i = 0; i < MAX_QUERY_LEN/2; i++) {
		cho_final[i] = (int *)malloc(sizeof(int)*MIN_FORMER_LATER_CNT);
		if (cho_final[i] == NULL) {
			fprintf(stderr, "cho_final[%d][] �迭 �Ҵ翡 �����߽��ϴ�. �̴� �ɰ��� �� �Դϴ�.\n", i);
			exit(-3);
		}
	}
}

/*
 * �Է� ���� ���� ���ڿ� in[]���� �ʼ��鸸 �����Ͽ� cho_query[]�� ä���.
 * ������ ���ڰ� ������ �ʼ� �ڵ�� �ùķ��̼��Ͽ� cho_query[]�� ä���ش�.
 * �׷��� ���ڰ� �ƴ� ���ĺ��� ������ ������ ��� query_alpha[]�� ä���д�.
 * extract_chosung_from_query(in_query, query_cho, query_alpha, &query_count, &query_alpha_count);
 */
void
extract_chosung_from_query(const char *in, short *in_cho, int *in_alpha, int *qc, int *qac)
{
	int in_count = 0;					// ������ �ʼ��� ����
	int alpha_count = 0;				// ������ ���ĺ��� ����
	char han[HANGUL_BYTE+1];			// �ѱ� ó�� �� ���� �ϳ��� ��� ���� ���� ����. NULL ���ڱ��� �����ϱ� ���� +1.
	char c_in[HANGUL_BYTE+1];			// �ϼ��� ���ڸ� ���������� �ٲ� �� ����ϱ� ���� ����.
	int i;
	unsigned short johapcode;			// �ڵ� ��ȯ �� �ӽ÷� ���
	unsigned short cho_code;			// �ʼ� �и� �� �ӽ÷� ���

	in_count=0;

	for (i = 0; in[i] != '\0'; i++) {
		if ((unsigned int) in[i] >= 128) {
			han[0] = in[i];
			han[1] = in[i+1];
			han[2]='\0';
			//printf("0x%4x ", (short)*(short *)han);
			HangulCodeConvert(KS, han, c_in);
			johapcode = ((unsigned char)c_in[0] << 8) | (unsigned char)c_in[1]; 
			cho_code = (johapcode >> 10) & 0x1f;	// �ʼ� ����
			if (cho_code < 0x02 || cho_code > 0x14) {
				fprintf(stderr, "extract_chosung_from_query(): �������� ������ �ʼ� ���� '%s' �߰�! �����մϴ�.\n", han);
				i++;
				continue;
			}
			in_cho[in_count++] = cho_code;
			i++;						
		}
		// ���� ���������� �ѱ� �̿��� ������ ó�� �� ��
#ifndef IGNORE_NON_HANGUL
		else if (in[i] >= '0' && in[i] <= '9')		// ���ڴ� �ʼ� 5��Ʈ�ڵ��� ���� �� �κ��� Ȱ��
			in_cho[in_count++] = in[i] - '0' + 0x15;
		else if ('a' <= in[i] && in[i] <= 'z') {	// �ҹ��� ���ĺ�
			in_cho[in_count++] = 0;
			in_alpha[alpha_count++] = in[i]-32;		// �빮�ڷ� �ٲپ� ���� ������ ��������
		} else {
			in_cho[in_count++] = 0;
			in_alpha[alpha_count++] = in[i];		// ���ĺ��� ���� ������ ����
		}
#else
		else if ('a' <= in[i] && in[i] <= 'z')	// �ҹ��� ���ĺ�
			in_alpha[alpha_count++] = in[i]-32;		// �빮�ڷ� �ٲپ� ���� ������ ��������
		else
			in_alpha[alpha_count++] = in[i];		// ���ĺ��� ���� ������ ����
#endif
	} // end of for (i = 0; in[i] != '\0'; i++)

	//printf("\n���������� ��ȯ�� ����: %d��(", in_count);
	//for (i = 0; i < in_count; i++)
	//	printf("0x%x ", in_cho[i]);
	//printf(")\n");

	*qc = in_count;		// query_count�� �Ѱ���
	*qac = alpha_count;	// query_alpha_count�� �Ѱ���
}

int
search_inverted_file_4text(const char *query_str)
{
	int index;
	int cur, len;
	int idx0, idx1;
	int i, j, k, s, e;
	int t_int;
	unsigned char t_char;
	Node *result;
	int first_chunk_eo_count, second_chunk_eo_count, third_chunk_eo_count;
	//int trouble_warning;

	len = strlen(query_str);
	eo_count = 0;


	//�ߺ����� ���� �ذ�κ�
	int Overlap_Search_perLine[MAX_ONELINE][2];//��� �� ���ڵ�ȿ� �ߺ����θ� Ȯ���ϱ� ���� ������
	bool newchar_perLine = true;
	int overlapnum;//�� ���ڵ忡�� �ߺ�Ƚ��

	int index_of_Overlap_Search_perLine= 0;
	for(int i=0; i<MAX_ONELINE; i++){
		Overlap_Search_perLine[i][0]=0;
		Overlap_Search_perLine[i][1]=0;
	}
	
	bool test_overlap_t_int = false;//DB �ִ� �ߺ� �ʰ�����
	int test_overlap_count = 0;//DB �ִ� �ߺ� ����
	//�ߺ����� ���� �ذ�κг�


	cur = 0;
	for (;;) {
		t_char = query_str[cur++];
		len--;
		if ((unsigned int)t_char >= 128) {
			eo_string[eo_count][0] = t_char;
				
			t_char = query_str[cur++];
			len--;

			eo_string[eo_count][1] = t_char;
			eo_string[eo_count][2] = '\0';

			t_int = (int)(*(unsigned short*)eo_string[eo_count]);
			
			//�ߺ����� ���� �ذ��� ���� �ּ�ó���� �κ�
			/*
			if (result = rbt2.search(t_int)) {	
				index = result->index;
				eo_index[eo_count++] = index;
			}
			else {
				index = -1;
				//printf("'%s' ignored!\n", eo_string[eo_count]);
			}//*/

			//if (len <= 0)
			//	break;
		}
		else { // ��ũ�� ������ �Ϲ� �ƽ�Ű ����
			if (isalpha(t_char))
				t_char = toupper(t_char);
				eo_string[eo_count][0] = t_char;
				eo_string[eo_count][1] = NULL;
				// eo_string[eo_count][2] = '\0';
				
				t_int = (int)t_char;

				//�ߺ����� ���� �ذ��� ���� �ּ�ó���� �κ�
				/*
				if (result = rbt2.search(t_int)) {
					index = result->index;
					eo_index[eo_count++] = index;
				}
				else {
					index = -1;
					//printf("'%s': ��� ���� ����. ������!\n", eo_string[eo_count]);
					//eo_index[eo_count++] = index;
				}//*/
		}
		
		//�ߺ����� ���� �ذ�κ�
		newchar_perLine = true;
		overlapnum = 0;
		for(int i=0; i<index_of_Overlap_Search_perLine; i++){
			if(Overlap_Search_perLine[i][0]==t_int){//�ߺ������� ���
				/*//�� �̷��� �Ǵ°����� �𸣰ڴµ� DB�� �ߺ� �̻��� ������ �־��൵ ���������� ����
				//�� �ߺ� 3���� �ִ��� '�����'��� �Է������� ���� �ε����� ħ������ �˾Ҵµ� ħ������ ����
				//������ �̺κ��� ���ָ� �� �ߺ� 3���� ���ڵ���� ������ 4���Ǵµ� �� ���ָ� ������ 3���� ����
				for(int s =0; s<MAX_OVERLAP; s++)
					if(Overlap_Count_perChar_tint[s]==t_int){
						test_overlap_t_int=true;
						test_overlap_count=Overlap_Count_perChar_count[s];
						break;
					}
				if(test_overlap_t_int==true && test_overlap_count>Overlap_Search_perLine[i][1]){
				//*/

					Overlap_Search_perLine[i][1]++;
					newchar_perLine = false;
					overlapnum = Overlap_Search_perLine[i][1]-1;//-1�߰� => �����ذ�
					break;
				//}
			}
		}
		
		if(newchar_perLine == true){//���ο� ������ ���
			Overlap_Search_perLine[index_of_Overlap_Search_perLine][0]=t_int;
			Overlap_Search_perLine[index_of_Overlap_Search_perLine][1]++;
			index_of_Overlap_Search_perLine++;
		}
		//�ߺ����� ���� �ذ�κг�
		
		if (result = rbt2.search(t_int)) {	
				index = result->index;
				
					//�ߺ����� ���� �ذ�κ�
					index = index + overlapnum;
					//��
				eo_index[eo_count++] = index;
			}
			else {
				index = -1;
				//printf("'%s' ignored!\n", eo_string[eo_count]);
			}//

		if (len <= 0)
			break;
	} // end of for (;;)
	//printf("search_inverted_file_4text(): ������ �ִ� ���� ���� = %d\n", eo_count);

	// ����
	for (i = 0; i < eo_count; i++) {	// ������ ���Ե� �� ���ڿ� ���� ������ ����.
		index = eo_index[i];	// ������ ���Ե� �� ���ڵ��� RB Ʈ�� �� �ε����� �����´�.
		
		searched_count[i] = 0;
		if (index == -1)			// ��� ���� ���ڰ� ������ ������
			continue;
		for (int j = 0; j < i_count[index]; j++) {	// �� ���ڰ� ��񿡼� ������ Ƚ�� ��ŭ �ݺ�
			if ((unsigned int)searched_count[i] >= MAX_SEARCHED_COL) {
				printf("searched[%d][]�� �� ������ ���ڶ��ϴ�. �� ��ȣ ���� �ڸ��� �����ϴ�. �ø�����.\n", i);
				exit(-1);
			}
			// inverted �迭���� �� ���ڰ� ��� �ִ� ��� �� ��ȣ 24��Ʈ(3 ����Ʈ)�� �����´�.
			searched[i][searched_count[i]] = 0;
			 
			t_int = (int)inverted[i_start[index] + 3*j]; // �����Ͽ��� �ٹ�ȣ ������Ʈ �� �ֻ��� byte �о� ��
			searched[i][searched_count[i]] += t_int << 16;
			 
			t_int = (int)inverted[i_start[index] + 3*j + 1]; 
			searched[i][searched_count[i]] += t_int << 8;

			t_int = (int)inverted[i_start[index] + 3*j + 2];
			searched[i][searched_count[i]++] += t_int;
			// ���� ��� searched[0][0] == 37, searched[0][1] == 90 �̸�,
			// '�����ڵ���'��� ������ ����, '��'�̶�� ���ڰ� ������ ù ���� 37�̰�, �ι�° ���� 90�̶�� ����.
			// ��, searched[1][0] == 37�̰�, searched[1][1] = 900 �̸�, '��'��� ���ڰ� ������ ù ���� 37�̰� �� ��° ���� 900 �̶�� �ǹ�
		}
	}
	//printf("eo_string[][]�� ũ�� = %d\n", sizeof(eo_string));
	//memcpy((void *)eo_string_org, (const void *)eo_string, sizeof(eo_string));	// ����Ƚ�� ������������ �����ϱ� ���� ���� ���ڿ� ������ �����صд�.
	//quickSort2(searched_count, 0, eo_count-1);
	//printf("���� Ƚ�� �������� ����: ");
	//for (i = 0; i < eo_count; i++)
	//	printf("(%s, %d) ", eo_string[i], searched_count[i]);
	//printf("\n");

	// ���� �����̿� ���� ������ ���� ó���Ѵ�.
	/*
	 * ���� ���� <= 2 : �� ���� ��� �����ϴ� �� ��ȣ ã�� ��.
	 * ���� ���� >= 3 && <= 5 : ������ �� chunk�� ���� ó��.
	 * ���� ���� >= 6 : ������ �� chunk�� ���� ó��.
	 */
	// 1st chunk �˻� ����: ������ ù ��° ���ڿ� �� ��° ���ڰ� ��� �����ϴ� �ٹ�ȣ ���� ã�´�.
	max_final = 0;
	idx0 = 0;
	idx1 = 1;
	// search[0][...]�� search[1][...]�� ��� �ִ� �� ��ȣ���� ��� ���ĵǾ� �־�� ��.
	i = j = 0;
	if (searched_count[idx0] != 0 && searched_count[idx1] != 0) {	// ������ ù��° ���ڿ� �ι�° ���ڰ� �����ϴ� �ٵ��� ������ 0�� �ƴϸ�,
 		for (;;) {
			if (searched[idx0][i] > searched[idx1][j])
				j++;
			else if (searched[idx0][i] < searched[idx1][j])
				i++;
			else {
				if ((unsigned int)final_count[max_final] >= MAX_SEARCHED_COL) {
					printf("final[%d][]�� �� ������ ���ڶ��ϴ�. �� ��ȣ ���� �ڸ��� �����ϴ�. �ø�����.\n", max_final);
					exit(-1);
				}
				final[max_final][final_count[max_final]++] = searched[idx0][i];
				i++;
				j++;
			}
			if (i == searched_count[idx0] || j == searched_count[idx1])	// ������ ù ��° ���ڿ� �� ��° ���� �� ��� ���̶� �� ���� �� ��ȣ�� �� �ôٸ�
				break;
		}
	}
	else {
		//fprintf(stderr, "��� ���� ���ڰ� �ԷµǾ��ų� ������ ���̰� �� ���� �̸��Դϴ�. �˻��� �� �����ϴ�.\n");
		return(-1);
	}
	//printf("search_inverted_file_4text(): ù �� ���� '%s', '%s' ��� ���� �� ���� = %d, max_final = %d.\n", eo_string[idx0], eo_string[idx1], final_count[max_final], max_final);

	if (eo_count >= 3 && eo_count <= 5)
		first_chunk_eo_count = (eo_count + 1) / 2;
	else if (eo_count >= 6)
		first_chunk_eo_count = (eo_count + 2) / 3;
	else
		first_chunk_eo_count = eo_count;

	//printf("first_chunk_eo_count = %d.\n", first_chunk_eo_count);
	for (idx0 = 2; idx0 < first_chunk_eo_count; idx0++) {
		i = j = 0;
		if (searched_count[idx0] != 0 && final_count[max_final] != 0) {	
 			for (;;) {
				if (searched[idx0][i] > final[max_final][j])
					j++;
				else if (searched[idx0][i] < final[max_final][j])
					i++;
				else {
					if ((unsigned int)final_count[max_final+1] >= MAX_SEARCHED_COL) {
						printf("final[%d][]�� �� ������ ���ڶ��ϴ�. �� ��ȣ ���� �ڸ��� �����ϴ�. �ø�����.\n", max_final);
						exit(-1);
					}
					final[max_final+1][final_count[max_final+1]++] = searched[idx0][i];
					i++;
					j++;
				}
				if (i == searched_count[idx0] || j == final_count[max_final])
					break;
			} // end of for (;;)
		}
		if (final_count[max_final+1] > 0) {
			max_final++;
			//printf("1st chunk %d��° ���� '%s'�� �����ϴ� �� ���� = %d. max_final = %d\n", idx0+1, eo_string[idx0], final_count[max_final], max_final);
		}
		else
			//printf("1st chunk %d��° ���� '%s'�� �����ϴ� �� ���� = %d. max_final�� ������ %d.\n", idx0+1, eo_string[idx0], final_count[max_final], max_final)
			;
	}
	first_max_final = max_final;	// 1st chunk �˻����� ������ ù��° ����� �ִ� ���ڵ��� �����ϴ� �� ��ȣ���� ���� �ִ� final[] ���� ��ġ�� ����ص�.

	if (eo_count < 3) {		// ���� ���̰� �����ں��� ������ 2nd, 3rd ûũ�� ���� �˻��� �� �ʿ䰡 ����
		if (eo_count == (max_final+2))	// �� ����¥�� ���� ���� �ٹ�ȣ���� �߰ߵ�.
			return 0;
		else
			return -1;
	}

	// 2nd chunk �˻� ����: ������ �ι�° ûũ�� (���) �����ϴ� �ٹ�ȣ ���� ã�´�.
	max_final++;
	if (eo_count == 3) {	// ���� ���̰� 3�̸� �� �� ���ڸ� �������...
		idx0 = eo_count - 2;
		idx1 = eo_count - 1;
	}
	else if (eo_count >= 4) {	// 4 �̻��̸� �ι�° ûũ ó�� ���ڿ� �� ���� ���ڸ� �������...
		idx0 = first_chunk_eo_count;
		idx1 = first_chunk_eo_count + 1;
	}
	// search[idx0][...]�� search[idx1][...]�� ��� �ִ� �� ��ȣ���� ��� ���ĵǾ� �־�� ��.
	i = j = 0;
	if (searched_count[idx0] != 0 && searched_count[idx1] != 0) {	// �� ��° ûũ�� ù��° ���ڿ� �ι�° ���ڰ� �����ϴ� �ٵ��� ������ 0�� �ƴϸ�,
 		for (;;) {
			if (searched[idx0][i] > searched[idx1][j])
				j++;
			else if (searched[idx0][i] < searched[idx1][j])
				i++;
			else {
				if ((unsigned int)final_count[max_final] >= MAX_SEARCHED_COL) {
					printf("final[%d][]�� �� ������ ���ڶ��ϴ�. �� ��ȣ ���� �ڸ��� �����ϴ�. �ø�����.\n", max_final);
					exit(-1);
				}
				final[max_final][final_count[max_final]++] = searched[idx0][i];
				i++;
				j++;
			}
			if (i == searched_count[idx0] || j == searched_count[idx1])	// ������ ù ��° ���ڿ� �� ��° ���� �� ��� ���̶� �� ���� �� ��ȣ�� �� �ôٸ�
				break;
		}
	}
	else {
		//fprintf(stderr, "��� ���� ���ڰ� �ԷµǾ��ų� ������ ���̰� �� ���� �̸��Դϴ�. �˻��� �� �����ϴ�.\n");
		return(-1);
	}
	//printf("2nd chunk �� ���� '%s', '%s' ��� ���� �� ���� = %d, max_final = %d.\n", eo_string[idx0], eo_string[idx1], final_count[max_final], max_final);

	if (eo_count == 3)
		second_chunk_eo_count = 2;
	else if (eo_count >= 4 && eo_count <= 5)
		second_chunk_eo_count = eo_count / 2;
	else if (eo_count >= 6)
		second_chunk_eo_count = (eo_count + 1) / 3;
	else
		second_chunk_eo_count = eo_count;

	//printf("second_chunk_eo_count = %d.\n", second_chunk_eo_count);
	for (idx0 = first_chunk_eo_count+2; idx0 <= (first_chunk_eo_count+second_chunk_eo_count-1); idx0++) {
		i = j = 0;
		if (searched_count[idx0] != 0 && final_count[max_final] != 0) {	
 			for (;;) {
				if (searched[idx0][i] > final[max_final][j])
					j++;
				else if (searched[idx0][i] < final[max_final][j])
					i++;
				else {
					if ((unsigned int)final_count[max_final+1] >= MAX_SEARCHED_COL) {
						printf("final[%d][]�� �� ������ ���ڶ��ϴ�. �� ��ȣ ���� �ڸ��� �����ϴ�. �ø�����.\n", max_final);
						exit(-1);
					}
					final[max_final+1][final_count[max_final+1]++] = searched[idx0][i];
					i++;
					j++;
				}
				if (i == searched_count[idx0] || j == final_count[max_final])
					break;
			} // end of for (;;)
		}
		if (final_count[max_final+1] > 0) {
			max_final++;
			//printf("%d��° ���� '%s'�� �����ϴ� �� ���� = %d. max_final = %d\n", idx0+1, eo_string[idx0], final_count[max_final], max_final);
		}
		else
			//printf("%d��° ���� '%s'�� �����ϴ� �� ���� = %d. max_final�� ������ %d.\n", idx0+1, eo_string[idx0], final_count[max_final], max_final)
			;
	}
	second_max_final = max_final;	// 2nd chunk �˻����� 2nd chunk ���ڵ��� �����ϴ� �� ��ȣ���� ���� �ִ� final[] ���� ��ġ�� ����ص�.
	
	if (eo_count == 3) {	// ���� ���̰� 3�̸�, ������ ���ڿ� ù��° ���ڰ� ��� ������ 3rd chunk�� ���� �˻��ؾ� ��.
		idx0 = 0;
		idx1 = 2;
	}
	else if (eo_count == 4) {	// ���� ���̰� 4�̸�, 1st Chunk�� 2nd Chunk���� �ѱ��ھ� ��� 3rd chunk�� ��� �˻��ؾ� ��.
		idx0 = 1;	// 0�� ����
		idx1 = 2;	// 3�� ����. ���� �ð� ����� ��� ������ �� �غ� ���� ����.
	}
	else if (eo_count == 5) {	// ���� ���̰� 5�̸�, 1st Chunk�� 2nd Chunk���� �ѱ��ھ� ��� 3rd chunk�� ��� �˻��ؾ� ��.
		idx0 = 2;	// 0�̳� 1�� ����
		idx1 = 3;	// 4�� ����. ���� �ð� ����� ��� ������ �� �غ� ���� ����.
	}
	else if (eo_count >= 6) {	// ���� ���̰� 6 �̻��̸� ���� ���� �ִ� 3rd chunk�� ó���ؾ� ��.
		idx0 = first_chunk_eo_count + second_chunk_eo_count;
		idx1 = idx0 + 1;
	}

	max_final++;
	// search[idx0][...]�� search[idx1][...]�� ��� �ִ� �� ��ȣ���� ��� ���ĵǾ� �־�� ��.
	i = j = 0;
	if (searched_count[idx0] != 0 && searched_count[idx1] != 0) {	// 3rd chunk�� ù��° ���ڿ� �ι�° ���ڰ� �����ϴ� �ٵ��� ������ 0�� �ƴϸ�,
 		for (;;) {
			if (searched[idx0][i] > searched[idx1][j])
				j++;
			else if (searched[idx0][i] < searched[idx1][j])
				i++;
			else {
				if ((unsigned int)final_count[max_final] >= MAX_SEARCHED_COL) {
					printf("final[%d][]�� �� ������ ���ڶ��ϴ�. �� ��ȣ ���� �ڸ��� �����ϴ�. �ø�����.\n", max_final);
					exit(-1);
				}
				final[max_final][final_count[max_final]++] = searched[idx0][i];
				i++;
				j++;
			}
			if (i == searched_count[idx0] || j == searched_count[idx1])	// ������ ù ��° ���ڿ� �� ��° ���� �� ��� ���̶� �� ���� �� ��ȣ�� �� �ôٸ�
				break;
		}
	}
	else {
		//fprintf(stderr, "��� ���� ���ڰ� �ԷµǾ��ų� ������ ���̰� �� ���� �̸��Դϴ�. �˻��� �� �����ϴ�.\n");
		return(-1);
	}
	//printf("3rd chunk �� ���� '%s', '%s' ��� ���� �� ���� = %d, max_final = %d.\n", eo_string[idx0], eo_string[idx1], final_count[max_final], max_final);

	if (eo_count >= 7) {	// ������ 7���� �̻��̸� 3rd Chunk�� ���� �� �� ���ڰ� ���� ���̹Ƿ� �̸� ó���Ѵ�. //120706
		third_chunk_eo_count = eo_count / 3;
		//printf("third_chunk_eo_count = %d.\n", third_chunk_eo_count);
		for (idx0 = first_chunk_eo_count+second_chunk_eo_count+2; idx0 <= (first_chunk_eo_count+second_chunk_eo_count+third_chunk_eo_count-1); idx0++) {
			i = j = 0;
			if (searched_count[idx0] != 0 && final_count[max_final] != 0) {	
 				for (;;) {
					if (searched[idx0][i] > final[max_final][j])
						j++;
					else if (searched[idx0][i] < final[max_final][j])
						i++;
					else {
						if ((unsigned int)final_count[max_final+1] >= MAX_SEARCHED_COL) {
							printf("final[%d][]�� �� ������ ���ڶ��ϴ�. �� ��ȣ ���� �ڸ��� �����ϴ�. �ø�����.\n", max_final);
							exit(-1);
						}
						final[max_final+1][final_count[max_final+1]++] = searched[idx0][i];
						i++;
						j++;
					}
					if (i == searched_count[idx0] || j == final_count[max_final])
						break;
				} // end of for (;;)
			}
			if (final_count[max_final+1] > 0) {
				max_final++;
				printf("%d��° ���� '%s'�� �����ϴ� �� ���� = %d. max_final = %d\n", idx0+1, eo_string[idx0], final_count[max_final], max_final);
			}
			else
				printf("%d��° ���� '%s'�� �����ϴ� �� ���� = %d. max_final�� ������ %d.\n", idx0+1, eo_string[idx0], final_count[max_final], max_final);
		}
	} // end of if (eo_count >= 6)
	third_max_final = max_final;	// 3rd chunk �˻����� 3rd chunk ���ڵ��� �����ϴ� �� ��ȣ���� ���� �ִ� final[] ���� ��ġ�� ����ص�.

	/*
	 * ������ �̻��� �����̸� ���� ù ������ ������ �ٹ�ȣ���� ������ ����(������ �� �κ�)�� ���� ������ ���ؾ� �Ѵ�.
	 * final[l2r_max_final][...]���� ������ ù ���� ���ڿ����� ������ �ٹ�ȣ���� ��� �ִ�.
	 * ���� (l2r_max_final+2)�� final[l2r_max_final][...]�� ��� �ִ� �� �ٹ�ȣ���� �����̴�.
	 * ���� l2r_max_final�� 2���ٸ� final[l2r_max_final][...]�� �ִ� �ٹ�ȣ���� ������ 4�̴�. ��, ���� �� �ױ��ڰ� (���) ������ �� ��ȣ���� ������ �ִ� ���̴�.
	 * 
	 * 
	 */
	if (first_final_degree != NULL) free(first_final_degree);
	first_final_degree = (int *)calloc(final_count[first_max_final], sizeof(int));	// ���� 1st Chunk�� ��� ������ �ٹ�ȣ���� ������ ������ �迭�� ���� �Ҵ�. ���߿� free ���Ѿ� ��.
	if (first_final_degree == NULL) {
		fprintf(stderr, "first_final_degree[] �迭 �Ҵ翡 �����߽��ϴ�. �����մϴ�.\n");
		exit(-13);
	}

	if (second_final_degree != NULL) free(second_final_degree);
	second_final_degree = (int *)calloc(final_count[second_max_final], sizeof(int));	// ���� 2nd Chunk�� ��� ������ �ٹ�ȣ���� ������ ������ �迭�� ���� �Ҵ�. ���߿� free ���Ѿ� ��.
	if (second_final_degree == NULL) {
		fprintf(stderr, "second_final_degree[] �迭 �Ҵ翡 �����߽��ϴ�. �����մϴ�.\n");
		exit(-13);
	}
	//printf("first_max_final = %d, second_max_final = %d.\n", first_max_final, second_max_final);

	if (merged_final_degree != NULL) free(merged_final_degree);
	merged_final_degree = (int *)calloc(final_count[first_max_final]+final_count[second_max_final], sizeof(int));
	if (merged_final_degree == NULL) {
		fprintf(stderr, "merged_final_degree[] �迭 �Ҵ翡 �����߽��ϴ�. �����մϴ�.\n");
		exit(-13);
	}

	if (third_final_degree != NULL) free(third_final_degree);
	third_final_degree = (int *)calloc(final_count[third_max_final], sizeof(int));	// ���� 3rd Chunk�� ��� ������ �ٹ�ȣ���� ������ ������ �迭�� ���� �Ҵ�. ���߿� free ���Ѿ� ��.
	if (third_final_degree == NULL) {
		fprintf(stderr, "third_final_degree[] �迭 �Ҵ翡 �����߽��ϴ�. �����մϴ�.\n");
		exit(-13);
	}
	//printf("third_max_final = %d.\n", third_max_final);

	if (merged2_final_degree != NULL) free(merged2_final_degree);
	merged2_final_degree = (int *)calloc(final_count[first_max_final]+final_count[second_max_final]+final_count[third_max_final], sizeof(int));
	if (merged2_final_degree == NULL) {
		fprintf(stderr, "merged2_final_degree[] �迭 �Ҵ翡 �����߽��ϴ�. �����մϴ�.\n");
		exit(-13);
	}
		
	for (i = 0; i < final_count[first_max_final]; i++)						// 1st chunk ���ڵ��� (���)������ �ٹ�ȣ���� ������ �ʱ�ȭ �Ѵ�.
		first_final_degree[i] = (first_max_final + 2);
	for (i = 0; i < final_count[second_max_final]; i++)						// 2nd chunk ���ڵ��� (���)������ �ٹ�ȣ���� ������ �ʱ�ȭ �Ѵ�.
		second_final_degree[i] = second_max_final - first_max_final + 1;
	if (eo_count >= 3 && eo_count <= 5) {
		for (i = 0; i < final_count[third_max_final]; i++)					// 3rd chunk ���ڵ��� (���)������ �ٹ�ȣ���� ������ �ʱ�ȭ �Ѵ�.
			third_final_degree[i] = 2;
	}
	else if (eo_count >= 6) {
		for (i = 0; i < final_count[third_max_final]; i++)					// 3rd chunk ���ڵ��� (���)������ �ٹ�ȣ���� ������ �ʱ�ȭ �Ѵ�.
			third_final_degree[i] = third_max_final - second_max_final + 1;
	}

	// 1st chunk�� �� �̿��� ���ڵ� ������ ������ ����Ѵ�.
	// ���� 3�߷��� ������ ���� ���� �˰����� ����Ͽ� �ӵ��� ���δ�.
	for (j = first_chunk_eo_count; j < eo_count; j++) {		// ���� 2nd chunk�� �ִ� ��� ���ڵ� ������ ����
		i = k = 0;
		if (final_count[first_max_final] != 0 && searched_count[j] != 0) {	
 			for (;;) {
				if (searched[j][k] > final[first_max_final][i]) i++;		// �ٹ�ȣ ���� ���� ������Ŵ
				else if (searched[j][k] < final[first_max_final][i]) k++;	// �ٹ�ȣ ���� ���� ������Ŵ
				else {														// ���� �ٹ�ȣ�� �߰ߵ�. ������ ������Ű�� �� �� ������Ŵ.
					first_final_degree[i]++;
					i++; k++;
				}
				if (i == final_count[first_max_final] || k == searched_count[j])	// ��� �� ���� ���� �������� ����������.
					break;
			} // end of for (;;)
		}
	}
	//printf("1st chunk�� �� �̿��� ���ڵ� ������ ���� ��� �Ϸ�.\n");

	// 2nd chunk�� 1st chunk ���ڵ� ������ ������ ����Ѵ�.
	/*
	trouble_warning = 0;
	if (final_count[second_max_final] > TROUBLE_COUNT)
		trouble_warning = 1;
	if (eo_count == 3) s = 2;	// ���� ���̰� 3�̸� 2:1 �׸��� 1:2 �̷��� �ڸ��� ������...
	else s = 1;
	//printf("final_count[second_max_final] = %d, then %d ~ %d.\n", final_count[second_max_final], first_chunk_eo_count-s, 0);
	for (i = 0; i < final_count[second_max_final]; i++) {		// ���� 2nd chunk ���ڵ��� (���)������ �� �ٹ�ȣ�鿡 ����
		for (j = first_chunk_eo_count-s; j >= 0; j--) {			// ���� 1st chunk�� �ִ� ��� ���ڵ� ������ ����
			if (trouble_warning && (searched_count[j] > TROUBLE_COUNT))
				continue;
			for (k = 0; k < searched_count[j]; k++) {			// ���� 1st chunk�� �ִ� ���ڰ� 2nd chunk���� ������ ��� �ٹ�ȣ���� ã�´�.
				if (final[second_max_final][i] == searched[j][k]) {	// �ٹ�ȣ�� ���� �� �߰�. �� �ٹ�ȣ�� ������ 1 ������Ų��.
					second_final_degree[i]++;
					break;
				}
				if (final[second_max_final][i] < searched[j][k])	// �� �ڴ� �� �� �ʿ䰡 �����Ƿ�
					break;
			}
		}
	}
	*/
	/*
	 * 2nd chunk�� 1st chunk ���ڵ� ������ ������ ����Ѵ�.
	 * ���� 3�߷��� ������ ���� ���� �˰����� ����Ͽ� �ӵ��� ���δ�.
	 */
	if (eo_count == 3) s = 2;	// ���� ���̰� 3�̸� 2:1 �׸��� 1:2 �̷��� �ڸ��� ������...
	else s = 1;
	for (j = first_chunk_eo_count-s; j >= 0; j--) {			// ���� 1st chunk�� �ִ� ��� ���ڵ� ������ ����
		i = k = 0;
		if (final_count[second_max_final] != 0 && searched_count[j] != 0) {	
 			for (;;) {
				if (searched[j][k] > final[second_max_final][i]) i++;		// �ٹ�ȣ ���� ���� ������Ŵ
				else if (searched[j][k] < final[second_max_final][i]) k++;	// �ٹ�ȣ ���� ���� ������Ŵ
				else {														// ���� �ٹ�ȣ�� �߰ߵ�. ������ ������Ű�� �� �� ������Ŵ.
					second_final_degree[i]++;
					i++; k++;
				}
				if (i == final_count[second_max_final] || k == searched_count[j])	// ��� �� ���� ���� �������� ����������.
					break;
			} // end of for (;;)
		}
	}
	//printf("2nd chunk�� 1st chunk ���ڵ� ������ ���� ��� �Ϸ�.\n");
	// ���� 3rd Chunk���� ó���Ϸ� ����.
	if (eo_count == 3) {
		// 3rd Chunk�� ������ ���ڵ� ���� ������ ����Ѵ�.
		j = 1;		// 3rd chunk�� ���� ���� ���� �� �Ѱ��� ������ �ε���.
		i = k = 0;
		if (final_count[third_max_final] != 0 && searched_count[j] != 0) {	
 			for (;;) {
				if (searched[j][k] > final[third_max_final][i]) i++;		// �ٹ�ȣ ���� ���� ������Ŵ
				else if (searched[j][k] < final[third_max_final][i]) k++;	// �ٹ�ȣ ���� ���� ������Ŵ
				else {														// ���� �ٹ�ȣ�� �߰ߵ�. ������ ������Ű�� �� �� ������Ŵ.
					third_final_degree[i]++;
					i++; k++;
				}
				if (i == final_count[third_max_final] || k == searched_count[j])	// ��� �� ���� ���� �������� ����������.
					break;
			} // end of for (;;)
		}
	}
	else if (eo_count == 4) {
		// 3rd Chunk�� ������ ���ڵ� ���� ������ ����Ѵ�.
		for (j = 0; j < eo_count; j+=3) {			// ���� 3rd chunk�� ���� ���� ���� ��� ���ڵ� ������ ����(�ε��� 0�� 3�� ����)
			i = k = 0;
			if (final_count[third_max_final] != 0 && searched_count[j] != 0) {	
 				for (;;) {
					if (searched[j][k] > final[third_max_final][i]) i++;		// �ٹ�ȣ ���� ���� ������Ŵ
					else if (searched[j][k] < final[third_max_final][i]) k++;	// �ٹ�ȣ ���� ���� ������Ŵ
					else {														// ���� �ٹ�ȣ�� �߰ߵ�. ������ ������Ű�� �� �� ������Ŵ.
						third_final_degree[i]++;
						i++; k++;
					}
					if (i == final_count[third_max_final] || k == searched_count[j])	// ��� �� ���� ���� �������� ����������.
						break;
				} // end of for (;;)
			}
		}
	}
	else if (eo_count == 5) {
		// 3rd Chunk�� ������ ���ڵ� ���� ������ ����Ѵ�.
		for (j = 0; j < eo_count; j++) {			// ���� 3rd chunk�� ���� ���� ���� ��� ���ڵ� ������ ����(�ε��� 0, 1, 4�� ����)
			if (j == 2 || j == 3)
				continue;
			i = k = 0;
			if (final_count[third_max_final] != 0 && searched_count[j] != 0) {	
 				for (;;) {
					if (searched[j][k] > final[third_max_final][i]) i++;		// �ٹ�ȣ ���� ���� ������Ŵ
					else if (searched[j][k] < final[third_max_final][i]) k++;	// �ٹ�ȣ ���� ���� ������Ŵ
					else {														// ���� �ٹ�ȣ�� �߰ߵ�. ������ ������Ű�� �� �� ������Ŵ.
						third_final_degree[i]++;
						i++; k++;
					}
					if (i == final_count[third_max_final] || k == searched_count[j])	// ��� �� ���� ���� �������� ����������.
						break;
				} // end of for (;;)
			}
		}
	}
	else if (eo_count >= 6) {
		// 2nd chunk�� 3rd chunk ���ڵ� ������ ������ ����Ѵ�.
		/*
		trouble_warning = 0;
		if (final_count[second_max_final] > TROUBLE_COUNT)
			trouble_warning = 1;
		for (i = 0; i < final_count[second_max_final]; i++) {							// ���� 2nd chunk ���ڵ��� (���)������ �� �ٹ�ȣ�鿡 ����
			for (j = first_chunk_eo_count+second_chunk_eo_count; j < eo_count; j++) {	// ���� 3rd chunk�� �ִ� ��� ���ڵ� ������ ����
				if (trouble_warning && (searched_count[j] > TROUBLE_COUNT))
					continue;
				for (k = 0; k < searched_count[j]; k++) {								// ���� 3rd chunk�� �ִ� ���ڰ� 2nd chunk���� ������ ��� �ٹ�ȣ���� ã�´�.
					if (final[second_max_final][i] == searched[j][k]) {					// �ٹ�ȣ�� ���� �� �߰�. �� �ٹ�ȣ�� ������ 1 ������Ų��.
						second_final_degree[i]++;
						break;
					}
					if (final[second_max_final][i] < searched[j][k])	// �� �ڴ� �� �� �ʿ䰡 �����Ƿ�
						break;
				}
			}
		}
		*/
		/*
		 * 2nd chunk�� 3rd chunk ���ڵ� ������ ������ ����Ѵ�.
		 * ���� 3�߷��� ������ ���� ���� �˰����� ����Ͽ� �ӵ��� ���δ�.
		 */
		for (j = first_chunk_eo_count+second_chunk_eo_count; j < eo_count; j++) {	// ���� 3rd chunk�� �ִ� ��� ���ڵ� ������ ����
			i = k = 0;
			if (final_count[second_max_final] != 0 && searched_count[j] != 0) {	
 				for (;;) {
					if (searched[j][k] > final[second_max_final][i]) i++;		// �ٹ�ȣ ���� ���� ������Ŵ
					else if (searched[j][k] < final[second_max_final][i]) k++;	// �ٹ�ȣ ���� ���� ������Ŵ
					else {														// ���� �ٹ�ȣ�� �߰ߵ�. ������ ������Ű�� �� �� ������Ŵ.
						second_final_degree[i]++;
						i++; k++;
					}
					if (i == final_count[second_max_final] || k == searched_count[j])	// ��� �� ���� ���� �������� ����������.
						break;
				} // end of for (;;)
			}
		}
		//printf("2nd chunk�� 3rd chunk ���ڵ� ������ ���� ��� �Ϸ�.\n");

		// 3rd chunk�� �� ���� ���ڵ� ������ ������ ����Ѵ�.
		/*
		trouble_warning = 0;
		if (final_count[third_max_final] > TROUBLE_COUNT)
			trouble_warning = 1;
		for (i = 0; i < final_count[third_max_final]; i++) {						// ���� 3rd chunk ���ڵ��� (���)������ �� �ٹ�ȣ�鿡 ����
			for (j = first_chunk_eo_count+second_chunk_eo_count-1; j >= 0; j--) {	// ���� �� ���ݿ� �ִ� ��� ���ڵ� ������ ����
				if (trouble_warning && (searched_count[j] > TROUBLE_COUNT))
					continue;
				for (k = 0; k < searched_count[j]; k++) {							// ���� �� ���ݿ� �ִ� � �� ���ڰ� ������ ��� �ٹ�ȣ��� ���غ���.
					if (final[third_max_final][i] == searched[j][k]) {				// �ٹ�ȣ�� ���� �� �߰�. �� �ٹ�ȣ�� ������ 1 ������Ų��.
						third_final_degree[i]++;
						break;
					}
					if (final[third_max_final][i] < searched[j][k])	// �� �ڴ� �� �� �ʿ䰡 �����Ƿ�
						break;
				}
			}
		}
		*/
		/*
		 * 3rd chunk�� �� ���� ���ڵ� ������ ������ ����Ѵ�.
		 * ���� 3�߷��� ������ ���� ���� �˰����� ����Ͽ� �ӵ��� ���δ�.
		 */
		for (j = first_chunk_eo_count+second_chunk_eo_count-1; j >= 0; j--) {	// ���� �� ���ݿ� �ִ� ��� ���ڵ� ������ ����
			i = k = 0;
			if (final_count[third_max_final] != 0 && searched_count[j] != 0) {	
 				for (;;) {
					if (searched[j][k] > final[third_max_final][i]) i++;		// �ٹ�ȣ ���� ���� ������Ŵ
					else if (searched[j][k] < final[third_max_final][i]) k++;	// �ٹ�ȣ ���� ���� ������Ŵ
					else {														// ���� �ٹ�ȣ�� �߰ߵ�. ������ ������Ű�� �� �� ������Ŵ.
						third_final_degree[i]++;
						i++; k++;
					}
					if (i == final_count[third_max_final] || k == searched_count[j])	// ��� �� ���� ���� �������� ����������.
						break;
				} // end of for (;;)
			}
		}
	}
	//printf("3rd chunk�� �� ���� ���ڵ� ������ ���� ��� �Ϸ�.\n");
	//printf("second_final_degree[] ���� �Ϸ�\n");
	//for (i = 0; i < final_count[r2l_max_final]; i++)
	//	printf("%d ", second_final_degree[i]);
	//printf("\n");
	// �������� ���� ���� first�� second�� ��ģ��. �׷��鼭 ��ġ�� �ٹ�ȣ�� ���ŵ� ���̴�.
	max_final++;
	i = j = 0;
	if (final_count[first_max_final] != 0 && final_count[second_max_final] != 0) {	// ������ ù��° ���ڿ� �ι�° ���ڰ� �����ϴ� �ٵ��� ������ 0�� �ƴϸ�,
 		for (;;) {
			if ((unsigned int)final_count[max_final] >= MAX_SEARCHED_COL) {
				printf("final[%d][]�� �� ������ ���ڶ��ϴ�. �� ��ȣ ���� �ڸ��� �����ϴ�. �ø�����.\n", max_final);
				exit(-1);
			}
			if (final[first_max_final][i] > final[second_max_final][j]) {
				final[max_final][final_count[max_final]] = final[second_max_final][j];
				merged_final_degree[final_count[max_final]++] = second_final_degree[j];
				j++;
			}
			else if (final[first_max_final][i] < final[second_max_final][j]) {
				final[max_final][final_count[max_final]] = final[first_max_final][i];
				merged_final_degree[final_count[max_final]++] = first_final_degree[i];
				i++;
			}
			else {	// �ٹ�ȣ�� ������ ������ ���� ���� ����
				if (first_final_degree[i] >= second_final_degree[j]) {
					final[max_final][final_count[max_final]] = final[first_max_final][i];
					merged_final_degree[final_count[max_final]++] = first_final_degree[i];
				}
				else {
					final[max_final][final_count[max_final]] = final[second_max_final][j];
					merged_final_degree[final_count[max_final]++] = second_final_degree[j];
				}
				i++;
				j++;
			}
			// ���� �� ó��
			if (i == final_count[first_max_final]) {
				for (; j < final_count[second_max_final]; j++) {
					final[max_final][final_count[max_final]] = final[second_max_final][j];
					merged_final_degree[final_count[max_final]++] = second_final_degree[j];
				}
				break;
			}
			if (j == final_count[second_max_final]) {
				for (; i < final_count[first_max_final]; i++) {
					final[max_final][final_count[max_final]] = final[first_max_final][i];
					merged_final_degree[final_count[max_final]++] = first_final_degree[i];
				}
				break;
			}
		}
	}
	else {
		// 1st chunk�� 2nd chunk �� �ٹ�ȣ�� �ϳ��� ���� ��� �߰�
		//printf("1st chunk�� 2nd chunk �� �ٹ�ȣ�� �ϳ��� ���� ����� �ֱ���.\n");
		if (final_count[first_max_final] != 0) {		// 2nd chunk�� �����ϴ� �ٹ�ȣ�� ����. 1st chunk�� merged�� ó��
			final_count[max_final] = final_count[first_max_final];
			memcpy(final[max_final], final[first_max_final], sizeof(int)*final_count[first_max_final]);
			memcpy(merged_final_degree, first_final_degree, sizeof(int)*final_count[first_max_final]);
			//printf("1st chunk�� merged�� �����Ͽ� ����մϴ�.\n");
		}
		else if (final_count[second_max_final] != 0) {	// 1st chunk�� �����ϴ� �ٹ�ȣ�� ����. 2nd chunk�� merged�� ó��
			final_count[max_final] = final_count[second_max_final];
			memcpy(final[max_final], final[second_max_final], sizeof(int)*final_count[second_max_final]);
			memcpy(merged_final_degree, second_final_degree, sizeof(int)*final_count[second_max_final]);
			//printf("2nd chunk�� merged�� �����Ͽ� ����մϴ�.\n");
		}
		else {											// 1st & 2nd chunk�� ��� ��������. merged�� ����ְ� ��.
			// �ƹ��͵� �� �� ����.
		}
	}
	//printf("degree 1�� merge �Ϸ�. max_final = %d, final_count[max_final] = %d.\n", max_final, final_count[max_final]);

	// merge�� �� �� ���ؾ� ��.
	merged_max_final = max_final;
	max_final++;
	i = j = 0;
	if (final_count[third_max_final] != 0 && final_count[merged_max_final] != 0) {	// ���� first, second ��ģ �ٹ�ȣ�� 3rd chunk �ٹ�ȣ���� ��ģ��.
 		for (;;) {
			if ((unsigned int)final_count[max_final] >= MAX_SEARCHED_COL) {
				fprintf(stderr, "final[%d][]�� �� ������ ���ڶ��ϴ�. �� ��ȣ ���� �ڸ��� �����ϴ�. �ø�����.\n", max_final);
				exit(-1);
			}
			if (final[third_max_final][i] > final[merged_max_final][j]) {
				final[max_final][final_count[max_final]] = final[merged_max_final][j];
				merged2_final_degree[final_count[max_final]++] = merged_final_degree[j];
				j++;
			}
			else if (final[third_max_final][i] < final[merged_max_final][j]) {
				final[max_final][final_count[max_final]] = final[third_max_final][i];
				merged2_final_degree[final_count[max_final]++] = third_final_degree[i];
				i++;
			}
			else {	// �ٹ�ȣ�� ������ ������ ���� ���� ����
				if (third_final_degree[i] >= merged_final_degree[j]) {
					final[max_final][final_count[max_final]] = final[third_max_final][i];
					merged2_final_degree[final_count[max_final]++] = third_final_degree[i];
				}
				else {
					final[max_final][final_count[max_final]] = final[merged_max_final][j];
					merged2_final_degree[final_count[max_final]++] = merged_final_degree[j];
				}
				i++;
				j++;
			}
			// ���� �� ó��
			if (i == final_count[third_max_final]) {
				for (; j < final_count[merged_max_final]; j++) {
					final[max_final][final_count[max_final]] = final[merged_max_final][j];
					merged2_final_degree[final_count[max_final]++] = merged_final_degree[j];
				}
				break;
			}
			if (j == final_count[merged_max_final]) {
				for (; i < final_count[third_max_final]; i++) {
					final[max_final][final_count[max_final]] = final[third_max_final][i];
					merged2_final_degree[final_count[max_final]++] = third_final_degree[i];
				}
				break;
			}
		}
	}
	else {
		// 3rd chunk�� merged chunk �� �ٹ�ȣ�� �ϳ��� ���� ��� �߰�
		//printf("3rd chunk�� merged chunk �� �ٹ�ȣ�� �ϳ��� ���� ����� �ֱ���.\n");
		if (final_count[third_max_final] != 0) {		// merged chunk�� �����ϴ� �ٹ�ȣ�� ����. 3rd chunk�� merged2�� ó��
			final_count[max_final] = final_count[third_max_final];
			memcpy(final[max_final], final[third_max_final], sizeof(int)*final_count[third_max_final]);
			memcpy(merged2_final_degree, third_final_degree, sizeof(int)*final_count[third_max_final]);
			//printf("3rd chunk�� merged2�� �����Ͽ� ����մϴ�.\n");
		}
		else if (final_count[merged_max_final] != 0) {	// 3rd chunk�� �����ϴ� �ٹ�ȣ�� ����. merged chunk�� merged2�� ó��
			final_count[max_final] = final_count[merged_max_final];
			memcpy(final[max_final], final[merged_max_final], sizeof(int)*final_count[merged_max_final]);
			memcpy(merged2_final_degree, merged_final_degree, sizeof(int)*final_count[merged_max_final]);
			//printf("merged chunk�� merged2�� �����Ͽ� ����մϴ�.\n");
		}
		else {											// 1st & 2nd chunk�� ��� ��������. merged�� ����ְ� ��.
			// �ƹ��͵� �� �� ����.
		}
	}
	//printf("degree merge ���� 2 �Ϸ�. max_final = %d, final_count[max_final] = %d.\n", max_final, final_count[max_final]);

	if (final_count[max_final] <= 0)
		return -1;

	// ���� ���� ������������ �ٹ�ȣ�� �����Ѵ�.
	//quickSort3(merged2_final_degree, 0, final_count[max_final]-1);

	//////////////////////////////////////////////////////[JAY]120919�� �ٲ�
	/*
	i = 2;							// �ְ� ���� �� ���� ������ �̾Ƴ���.
	k = eo_count;					// �̷��� �ְ� ������ ���� ������.
	s = 0;
	e = final_count[max_final]-1;
	int prevj = -1;
	while (i && k) {
		j = partition4(merged2_final_degree, s, e, k);
		if (j >= s) {	// ���� k �̻��� �ٹ�ȣ���� ����.
			printf("���� %d�� �ٹ�ȣ %d�� partition ����\n", k, j-prevj);
			prevj = j;
			i--;
			s = j+1;	// ������ k �̸��� �κп� ���ؼ��� ������ ���� �ٽ� ã�´�.
			if (s >= e)
				break;
		}
		k--;			// ������ �ϳ� ����.
	};
	*/
	i = eo_count;					// ��� ������ �̾Ƴ���.
	k = eo_count;					// �̷��� �ְ� ������ ���� ������.
	s = 0;
	e = final_count[max_final]-1;
	int prevj = -1;

	//�ĺ��� ���͸��� �ּ�����(min_chasu) �����ϴ� �κ� - JAY
	int min_chasu = 0;
	if(eo_count<=4){
		min_chasu = 2;			//���� �ּ� 2���� �¾ƾ���
	}else{
		min_chasu = eo_count-2;	//���� 3�� Ʋ���ű����� ����
	}

	while (i && k) {
		j = partition4(merged2_final_degree, s, e, k);
		if (j >= s) {	// ���� k �̻��� �ٹ�ȣ���� ����.
			printf("���� %d�� �ٹ�ȣ %d�� partition ����\n", k, j-prevj);
			if(k == min_chasu){
				if(j>18000){	//���͸��� ������ ������ 18000�� ������ ���߹Ƿ�, ������������ ������ ���� �κ� - JAY
					no_of_filtered_data = prevj;
					printf("2.���͸��� �����Ͱ��� : %d, �ּ����� : %d\n", no_of_filtered_data, min_chasu);
				}else{
					no_of_filtered_data = j;
					printf("1.���͸��� �����Ͱ��� : %d, �ּ����� : %d\n", no_of_filtered_data, min_chasu);
				}
			}
			prevj = j;
			i--;
			s = j+1;	// ������ k �̸��� �κп� ���ؼ��� ������ ���� �ٽ� ã�´�.
			if (s >= e){
				printf("\n[[%d, %d]]\n", s, final_count[max_final]-1);
				break;
			}
		}
		k--;			// ������ �ϳ� ����.
	};
	////////////////////////////////////////////////////////
	printf("merged2_final_degree[] �������� ���� �Ϸ�\n");
	if (merged2_final_degree[0] >= eo_count)
		return 1;
	else
		return 0;
}

/*
 * search_using_inverted_file_4chosung(query_cho, query_count, query_alpha, query_alpha_count);
 * ������ 3���� �̸��̸� ������ �߻��ϹǷ� ���� ���̰� ������ �̻��� ������ ȣ���� ��
 */
void
search_using_inverted_file_4chosung(short *in_eo, int in_count, int *in_alpha, int in_alpha_count)
{
	int i, j;
	unsigned int k;
	int former, latter;
	unsigned int cur;
	unsigned short johapcode;
	unsigned short cho_code;
	char han[HANGUL_BYTE+1];			// �ѱ� ó�� �� ���� �ϳ��� ��� ���� ���� ����. NULL ���ڱ��� �����ϱ� ���� +1.
	char c_han[HANGUL_BYTE+1];			// �ϼ��� ���ڸ� ���������� �ٲ� �� ����ϱ� ���� ����.
	int cur_final = 0;
	int cur_entry = 0;
	unsigned int a, b;

	if (in_count < 3) {
		former = in_eo[0]*38 + in_eo[1];
		cur = cho2_start[former];
		for (k = 0; k < cho2_count[former]; k++) {	// �� ù 2�����ʼ��� �����ϴ� ��� �ٹ�ȣ�� ����
			cho_final[cur_final][cho_final_count[cur_final]++] = (cho2_inverted[cur]<<16) + (cho2_inverted[cur+1]<<8) + (cho2_inverted[cur+2]);	// �� 2�����ʼ��� ���� �� ��ȣ���� ���´�.
			cur = cur + 3;			// �ٹ�ȣ �ϳ��� 3����Ʈ(24��Ʈ)�̴ϱ�
		}
	}
	else {
		former = in_eo[0]*38 + in_eo[1];											// ������ �� ù 2�����ʼ�ID�� ����Ѵ�.
		latter = in_eo[in_count-3]*38*38 + in_eo[in_count-2]*38 + in_eo[in_count-1];	// ������ �� ù 3�����ʼ�ID�� ����Ѵ�.
		//printf("former: %d , latter : %d\n",former, latter);
		//printf("former_c: %d , latter_c : %d\n",cho2_count[former], cho3_count[latter]);
	
		cur = cho2_start[former];		// ������ ù 2�����ʼ��� ���� �ٹ�ȣ ù ��ġ�� �˾Ƴ���.

		//printf("cur_final = %d\n", cur_final);
		for (k = 0; k < cho2_count[former]; k++) {	// �� ù 2�����ʼ��� �����ϴ� ��� �ٹ�ȣ�� ����
			former_list[k] = (cho2_inverted[cur]<<16) + (cho2_inverted[cur+1]<<8) + (cho2_inverted[cur+2]);	// �� 2�����ʼ��� ���� �� ��ȣ���� ���´�.
			cur = cur + 3;			// �ٹ�ȣ �ϳ��� 3����Ʈ(24��Ʈ)�̴ϱ�
			//printf("%d <= (%d)", former_list[k], cur-3);
			//printf("%d ",former_list[k]);
		}
		//printf("former_list[]�� %d��.\n", cho2_count[former]);

		cur = cho3_start[latter];		// ������ �� 3�����ʼ��� ���� �ٹ�ȣ ù ��ġ�� �˾Ƴ���.
		for (k = 0; k < cho3_count[latter]; k++) {	// �� �� 3�����ʼ��� �����ϴ� ��� �ٹ�ȣ�� ����
			a = 0; b = 0;

			b = (int)cho3_inverted[cur];
			a = b << 16;
			b = (int)cho3_inverted[cur+1];
			a += b << 8;
			b = (int)cho3_inverted[cur+2];
			a += b;
			latter_list[k] = a;		// �� 3�����ʼ��� ���� �� ��ȣ���� ���´�.
			cur = cur + 3;
		//	fprintf(p_file2, "%d\n",latter_list[k]);
		}
		//printf("latter_list[]�� %d��.\n", cho3_count[latter]);

		//	���� Ʃ��, ���� Ʈ���� ����
		i = 0;
		j = 0;	// �� �ڵ尡 ��� ���װ� ������ ����. �����찡 �� ���� �߰���.
		if (cho2_count[former] != 0 && cho3_count[latter] != 0) {
			for (;;) {
				if (former_list[i] > latter_list[j])
					j++;
				else if (former_list[i] < latter_list[j])
					i++;
				else {	// ��ġ�� �ٹ�ȣ �߰�
					cho_final[cur_final][cho_final_count[cur_final]++] = former_list[i];
					i++;
					j++;
				}
				if (i == cho2_count[former] || j == cho3_count[latter])
					break;
			}
		}
		//printf("Ʃ��, Ʈ���� count %d\n", cho_final_count[cur_final]);
		if (cho_final_count[cur_final] == 0) {	// Ʃ��, Ʈ���� ��ġ�� ���� ������
			if (cho3_count[latter] > 0) {
				cho_final_count[cur_final] = cho3_count[latter];
				for (a = 0; a < cho3_count[latter]; a++)
					cho_final[cur_final][a] = latter_list[a];
				//printf("==> �� Ʈ���� only�� ��ȯ: cho_final_count[%d] = %d.\n", cur_final, cho_final_count[cur_final]);
			}
			else if (cho2_count[former] > 0) {
				cho_final_count[cur_final] = cho2_count[former];
				for (a = 0; a < cho2_count[former]; a++)
					cho_final[cur_final][a] = former_list[a];
				//printf("==> �� Ʃ�� only�� ��ȯ: cho_final_count[%d] = %d.\n", cur_final, cho_final_count[cur_final]);
			}
		}
		//for (int c = 0; c < cho_final_count[cur_final]; c++) {
		//	printf("%s\n", &memory_db[position_in_memory[cho_final[cur_final][c]]]);
		//}

		cur_final++;		// ���� �� ù 3�����ʼ��� �� ù 2�����ʼ��� ��� ���� �ִ� �ٹ�ȣ���� �˻��Ϸ� ����. ��, ù Ʈ����, Ʃ���� �˻��Ϸ� ����.
		//printf("cur_final = %d\n", cur_final);

		former = in_eo[0]*38*38 + in_eo[1]*38 + in_eo[2];		// ������ �� ù 3�����ʼ�ID�� ����Ѵ�.
		latter = in_eo[in_count-2]*38 + in_eo[in_count-1];		// ������ �� ù 2�����ʼ�ID�� ����Ѵ�.
		//printf("former: %d , latter : %d\n",former, latter);
		//printf("former_c: %d , latter_c : %d\n",cho3_count[former], cho2_count[latter]);

		cur = cho2_start[latter];		// ������ �� ù 2�����ʼ��� ���� �ٹ�ȣ���� ����Ǿ� �ִ� ������ ���� ù ��ġ�� �˾Ƴ���.
		for (k = 0; k < cho2_count[latter]; k++) { // �� �� ù 2�����ʼ��� �����ϴ� ������ŭ ��������.
			a = 0; b = 0;

			b = (int)cho2_inverted[cur];
			a = b << 16;
			b = (int)cho2_inverted[cur+1];
			a += b << 8;
			b = (int)cho2_inverted[cur+2];
			a += b;
			latter_list[k] = a;
			cur = cur + 3;
			//fprintf(p_file2,"%d\n",latter_list[k]);
		}
		//printf("latter_list[]�� %d��.\n", cho2_count[latter]);

		cur = cho3_start[former];		// ������ �� ù 3�����ʼ��� ���� �ٹ�ȣ���� ����Ǿ� �ִ� ������ ���� ù ��ġ�� �˾Ƴ���.
		for (k = 0; k < cho3_count[former]; k++) {	// �� �� ù 3�����ʼ��� �����ϴ� ������ŭ ��������.
			a = 0; b = 0;

			b = (int)cho3_inverted[cur];
			a = b << 16;
			b = (int)cho3_inverted[cur+1];
			a += b << 8;
			b = (int)cho3_inverted[cur+2];
			a += b;
			former_list[k] = a;
			cur = cur + 3;
		}
		//printf("former_list[]�� %d��.\n", cho3_count[former]);

		//	���� Ʈ����, ���� Ʃ�� ���� ==> ���ĵǾ� �ִ� �� �迭�� ��ģ��.
		i = 0;
		j = 0;
		if (cho3_count[former] != 0 && cho2_count[latter] != 0) {
			for (;;) {
				if (former_list[i] > latter_list[j])
					j++;
				else if (former_list[i] < latter_list[j])
					i++;
				else {	// ��ġ�� �ٹ�ȣ �߰�
					cho_final[cur_final][cho_final_count[cur_final]++] = former_list[i];
					i++;
					j++;
				}
				if (i == cho3_count[former] || j == cho2_count[latter])
					break;
			}
		}
		//printf("Ʈ����, Ʃ�� count %d\n", cho_final_count[cur_final]);
		//for (int c = 0; c < cho_final_count[cur_final]; c++) {
		//	printf("%s\n", &memory_db[position_in_memory[cho_final[cur_final][c]]]);
		//}
		if (cho_final_count[cur_final] == 0) {	// Ʈ����, Ʃ�� ��ġ�� ���� ������
			if (cho3_count[former] > 0) {
				cho_final_count[cur_final] = cho3_count[former];
				for (a = 0; a < cho3_count[former]; a++)
					cho_final[cur_final][a] = former_list[a];
				//printf("==> �� Ʈ���� only�� ��ȯ: cho_final_count[%d] = %d.\n", cur_final, cho_final_count[cur_final]);
			}
			else if (cho2_count[latter] > 0) {
				cho_final_count[cur_final] = cho2_count[latter];
				for (a = 0; a < cho2_count[latter]; a++)
					cho_final[cur_final][a] = latter_list[a];
				//printf("==> �� Ʃ�� only�� ��ȯ: cho_final_count[%d] = %d.\n", cur_final, cho_final_count[cur_final]);
			}
		}

		cur_final++;	// cho_final[0][...]�� cho_final[1][...]�� ä�����Ƿ�, cho_final[2][...]�� ä�췯 ����. ���� ���� ä������ �ñ�������.����
		//printf("cur_final = %d\n", cur_final);
				
		//	������+������ ���� ==> ���� �ÿ��� ���� �˰����� �̿��Ѵ�. ���ĵǾ� �ִ� �� �迭�� ��ġ�� �˰���.
		i = 0, j = 0;
		if (cho_final_count[cur_final-2] > 0 && cho_final_count[cur_final-1] > 0) {	// ������ �Ÿ��� �־��...
			for (;;) {
				if (cho_final[cur_final-2][i] > cho_final[cur_final-1][j]) {
					cho_final[cur_final][cho_final_count[cur_final]++] = cho_final[cur_final-1][j];
					j++;
				}
				else if (cho_final[cur_final-2][i] < cho_final[cur_final-1][j]) {
					cho_final[cur_final][cho_final_count[cur_final]++] = cho_final[cur_final-2][i];
					i++;
				}
				else {	// ��ġ�� ���� �� ���� ����Ѵ�.
					cho_final[cur_final][cho_final_count[cur_final]++] = cho_final[cur_final-2][i];
					i++, j++;
				}
				if (i >= cho_final_count[cur_final-2] || j >= cho_final_count[cur_final-1])
					break;
			}
		}
		// ���� �ٵ� ����
		if (i == cho_final_count[cur_final-2]) {
			for(;j < cho_final_count[cur_final-1]; j++)
				cho_final[cur_final][cho_final_count[cur_final]++] = cho_final[cur_final-1][j];
		}
		else if (j == cho_final_count[cur_final-1]) {
			for(;i < cho_final_count[cur_final-2]; i++)
				cho_final[cur_final][cho_final_count[cur_final]++] = cho_final[cur_final-2][i];
		}
		//printf("���� �� cho_final_count[cur_final] = %d\n", cho_final_count[cur_final]);
	}

 	// ������ 6��¥ �ʰ��� ��쿡�� Ʃ��...Ʈ����, �Ǵ� Ʈ����...Ʃ�� ���̿� ���ڰ� ���� ���̹Ƿ� �װ͵��� ó������.
	for (i = 0; i < in_count-6; i++) {
		int found_more;

		found_more = 0;
		johapcode = in_eo[i+3];	// �׹�°, �ټ���°.... �ʼ��� ������.
		for(int j = 0; j < cho_final_count[cur_final]; j++) {  // cho_final_count[2]�� ������ŭ. ��, ������+������ ��� ���� ���� ��ο� ����
			cur_entry = cho_final[cur_final][j];	// �ٹ�ȣ �ϳ� ������

			for (int z = 0; z < namepart_len[cur_entry]; z++) {	// ������ "������+������"�� ������ �ٿ��� ��Ī �κп� ���ؼ���
				if ((unsigned short)memory_db[position_in_memory[cur_entry]+z] >= 128) {	// �ѱ��϶� -> �ʼ��� �̾Ƽ� temp2�� ����
					han[0] = memory_db[position_in_memory[cur_entry]+z]; 
					han[1] = memory_db[position_in_memory[cur_entry]+z+1];
					han[2]='\0';			//////////// ���̾�! �ٵ� ���δ� �׷���. ���⿡ '\n'�� ����? ���� ����!!!
					HangulCodeConvert(KS, han, c_han);
					cho_code = ((unsigned char)c_han[0] << 8) | ((unsigned char)c_han[1]); 
					cho_code = (cho_code >> 10) & 0x1f;
					z++;		// �ѱ��̹Ƿ� �� ����Ʈ �� ����
				}
#ifndef	IGNORE_NON_HANGUL
				else if ((unsigned short)memory_db[position_in_memory[cur_entry]+z] >= '0' && (unsigned short)memory_db[position_in_memory[cur_entry]+z] <= '9')	// ����
					cho_code = (unsigned short)memory_db[position_in_memory[cur_entry]+z] - '0' + 0x15;
				else if (memory_db[position_in_memory[cur_entry]+z] == 0)	// �� �� �� �����̸� ����.
					continue;
				else 		// ���ĺ�
					cho_code = 0;
#else
				else if (memory_db[position_in_memory[cur_entry]+z] == 0)	// �� �� �� �����̸� ����.
					continue;
				else
					cho_code = memory_db[position_in_memory[cur_entry]+z];
#endif
				if (johapcode == cho_code) {	// �ʼ��� ������ cho_final[]�� cur_entry�� �߰�
					cho_final[cur_final+1][cho_final_count[cur_final+1]++] = cur_entry;
					found_more = 1;
					break;
				}
			} // end of for (int z = 0; z < namepart_len[cur_entry]; z++)
		} // end of for(int j = 0; j < cho_final_count[cur_final]; j++)
		if (found_more)
			cur_final++;
		//printf("cur_final(7�����̻�) = %d\n", cur_final);
		//printf("cho_final_count[cur_final] = %d\n", cho_final_count[cur_final]);
	} // end of for (i = 0; i < in_count-6; i++)

	// ������ ���ĺ��� ���ԵǾ� �־����� �� ���ĺ����� �����ϰ� �ִ� �ٹ�ȣ���� ������ �����Ѵ�.
	// cho_final[cur_final][...]�� cur_final�� �����ʿ� ���� �� ������ �� �����ϰ� �����ϰ� �ִ� �� ��ȣ���� ����. 
	for (int xx = 0; xx < in_alpha_count; xx++) {  // ���� ���ڰ����� in_count �� ���ĺ��� ������ query_alpha_count ��ŭ
		int found_more;

		found_more = 0;
		johapcode = in_alpha[xx];		// ������ ���ԵǾ� �ִ� ���ĺ��� �����´�.
		//printf("cur_final %d, cho_final_count[cur_final] %d\n",cur_final, cho_final_count[cur_final]);
		
		for (int ii = 0; ii < cho_final_count[cur_final]; ii++) {
			cur_entry = cho_final[cur_final][ii];						
			
			for (int z = 0; z < namepart_len[cur_entry]; z++) { //cur_entry���� name��ŭ��
				// �ҹ��ڸ�
				if((unsigned short)memory_db[position_in_memory[cur_entry]+z] >= 'a' && (unsigned short)memory_db[position_in_memory[cur_entry]+z] <= 'z')
					cho_code = (unsigned short)memory_db[position_in_memory[cur_entry]+z] - 32;	// �빮�ڷ� �ٲپ� ����
				else if (memory_db[position_in_memory[cur_entry]+z] == 0)	// �� �� �� �����̸� ����.
					continue;
				else
					cho_code = (unsigned short)memory_db[position_in_memory[cur_entry]+z];

				if (johapcode == cho_code) {	// ���ĺ��� ��ġ�ϸ�
					cho_final[cur_final+1][cho_final_count[cur_final+1]++] = cur_entry;
					found_more = 1;
					break;
				}
			} // end of for(int z = 0; z < namepart_len[cur_entry]; z++)
		} // end of for (int ii = 0; ii < cho_final_count[cur_final]; ii++)
		if (found_more)
			cur_final++;
		//printf("cur_final(���ĺ�ó����) = %d\n", cur_final);
	}

	cho_max_final = cur_final;
	//printf("cho_max_final = %d, cho_final_count[cho_cur_final] = %d\n",cho_max_final, cho_final_count[cho_max_final]);
}

void
scoring_and_output_4text(int fFullyIncluded)
{
	int i, cur, k, z;
	short *score;	// �� �˻� ����� ������ �󸶳� �������� ��Ÿ���� ���� �迭
	int print_cnt;

	mfd = merged2_final_degree;

	if (fFullyIncluded < 0)
		return;
	else if (fFullyIncluded) {
		printf("--------------- ��Ī�κ� ���� ���� ��� ---------------\n");
		printf(" �ٹ�ȣ ���� �˻����(���ھ�)\n");
		/********************************************************************************JAY ��ü���������..
		for (i = 0; mfd[i] >= mfd[0]; i++);		// �������� ������ ������ ��´�.
		k = i;
		printf("�ְ����� ����: %d.\n", k);

		for (; mfd[i] >= mfd[k]; i++);			// �� ���� ������ ������ ��´�.
		z = i;
		printf("2.[[[[%d, %d]]]]", k, z);
		printf("���� ���� ����: %d.\n", z-k);
		*/
		//////////////
		/*
		for (i = 0; mfd[i] >= mfd[0]; i++);		// �������� ������ ������ ��´�.
		k = i;
		for (i = k; i < final_count[max_final]; i++) {
			if(i>18000){
				break;
			}
		}
		z = i;	// ����� �˻� ����� ������ �˾Ƴ���.
		printf("[[[**2**]]]", k, z);
		printf("���� ���� ����: %d.\n", z-k);
		*/
		//////////////
		k = 0;
		i = -1;
		do{
			
			//�ִ� 18,000���� �ҷ�����
			if(i>18000){
				printf("�ε�� �����Ͱ� 18000���� �ѽ��ϴ�.\n");
				break;
			}
			/*
			// ��Ƽ�Ŵ� �ȵ� �������� ���� �ҷ�����
			
			if(mfd[i]<0){
				printf("minus[%d]\n", i);
				break;
			}
			*/

			i++;

			if(mfd[i] < mfd[k]){
				printf("����: %d[%d-%d].\n", i-k, mfd[i], mfd[k]);
				//��Ƽ�Ŵ� �� ���������� �ҷ�����
				/*
				if(i > final_count[max_final]-1){
					printf("����������[%d]\n", i);
					break;
				}
				*/
				//���͸��� �ּ�����(min_chasu)������ �ҷ�����
				if(i > no_of_filtered_data-1){
					printf("����������[%d]\n", i);
					break;
				}
				k = i;
			}
		}while(1);
		z = i;
		printf("�ѷε尳��: %d.\n", z);

		score = (short *)malloc(sizeof(short)*z);
		if (!score ) {
			fprintf(stderr, "scoring_and_output_4text(): score[] �迭 �Ҵ� ����!\n");
			return;
		}

/////////////////JAY
		/*
		for (i = 0; i < k; i++) {			// �������� ������ ���ؼ�.
			cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
			score[i] = CalSimilarity((const unsigned char *)&memory_db[cur], i);	// �˻� ����� ���� ���� ��ġ���� ����Ѵ�.
		}
		quickSort5(score, 0, k-1);	// ���� ���ھ� ���� �������� ����
		for (i = k; i < z; i++) {			// �� ���� ������ ���ؼ�.
			cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
			score[i] = CalSimilarity((const unsigned char *)&memory_db[cur], i);	// �˻� ����� ���� ���� ��ġ���� ����Ѵ�.
		}
		quickSort5(score, k, z-1);	// ���� ���ھ� ���� �������� ����
		*/
/////////////////JAY

		//������ ������ �ʴ´ٰ� ������
		for (i = 0; i < z; i++) {	//���� ����������.
			cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
			score[i] = CalSimilarity((const unsigned char *)&memory_db[cur], i);	// �˻� ����� ���� ���� ��ġ���� ����Ѵ�.
		}
		quickSort5(score, 0, z-1);	// ���� ���ھ� ���� �������� ����
		
/////////////////JAY

		print_cnt = 0;
		for (i = 0; i < z; i++) {
			if (score[i] <= MAX_MINUS_POINT)	// ������ MAX_NINUS_POINT ������ ���� �� ������ ����.
				print_cnt++;
		}
		if (print_cnt >= MAX_OUT_LINES) {		// ������ MAX_NINUS_POINT ������ �ٷθ� MAX_OUT_LINES ���� ����� �� ������
			int extra_print_cnt = 0;

			for (i = 0; i < z; i++) {
				if (score[i] <= MAX_MINUS_POINT) {
					printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
					cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
					printf("%s(%d)\n", &memory_db[cur], score[i]);

					extra_print_cnt++;
					if (extra_print_cnt >= MAX_OUT_LINES)
						break;
				}
			}
		}
		else {	// ������ MAX_NINUS_POINT ������ �ٷθ� MAX_OUT_LINES ���� ����ϱ⿡�� ������.
			int extra_print_cnt = 0;

			for (i = 0; i < z; i++) {
				if (score[i] <= MAX_MINUS_POINT) {
					printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
					cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
					printf("%s(%d)\n", &memory_db[cur], score[i]);
				}
			}

			for (i = 0; i < z; i++) {
				if (score[i] > MAX_MINUS_POINT) {
					printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
					cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
					printf("%s(%d)\n", &memory_db[cur], score[i]);

					extra_print_cnt++;
					if (extra_print_cnt >= (MAX_OUT_LINES - print_cnt))
						break;
				}
			}
		}
		printf("------------- ��Ī�κ� ���� ���� ��� ��(%d��) -------------\n", z);
		free(score);
	}
	else {
		// ����� �Դ��� ���� ���̰� 3 �̸��̸� ���� ������ ����
		if (eo_count >= 3) {
			printf("----- ��Ī�κ� �κ� ���� ��� -----\n");
			printf(" �ٹ�ȣ ���� �˻����(���ھ�)\n");
		}
		else {
			printf("---------- ��Ī�κ� ���� ���� ��� ----------\n");
			printf(" �ٹ�ȣ �˻����(���ھ�)\n");
		}
		/*
		for (i = 0; i < final_count[max_final]; i++) {
			if (eo_count >= 3) {	// ���� ���̰� �� ���� �̻��� ������ merged_final_degree[]�� ����� ����
				if (mfd[i] < mfd[0])		// �ְ� ������ �� ���� ���������� ����.
					break;
			}
		}
		k = i;	// ����� �˻� ����� ������ �˾Ƴ���.
		printf("�ְ����� ����: %d.\n", k);
		for (i = k; i < final_count[max_final]; i++) {
			//*******************************************************************************************JAY : ��������� �ٺ��� ���� �ϴ� ��������.120828
			if (eo_count >= 3) {	// ���� ���̰� �� ���� �̻��� ������ merged_final_degree[]�� ����� ����
				if (mfd[i] < mfd[k])		// �ְ� ������ �� ���� ���������� ����.
					break;
			}
			
			if(i>18000){
				break;
			}
		}
		*/
		/////////////////////////////[JAY]120919
		if (eo_count >= 3){		// ���� ���̰� �� ���� �̻��� ������ merged_final_degree[]�� ����� ����
			k = 0;
			i = -1;
			do{
				
				//�ִ� 18,000���� �ҷ�����
				if(i>18000){
					printf("18000[%d]\n", i);
					break;
				}
				/*
				// ��Ƽ�Ŵ� �ȵ� �������� ���� �ҷ�����
			
				if(mfd[i]<0){
					printf("minus[%d]\n", i);
					break;
				}
				*/

				i++;

				if(mfd[i] < mfd[k]){
					printf("����: %d[%d-%d].\n", i-k, mfd[i], mfd[k]);
					//��Ƽ�Ŵ� �� ���������� �ҷ�����
					/*
					if(i > final_count[max_final]-1){
						printf("����������[%d]\n", i);
						break;
					}
					*/
					//���͸��� �ּ�����(min_chasu)������ �ҷ�����
					if(i > no_of_filtered_data-1){
						printf("����������[%d]\n", i);
						break;
					}
					k = i;
				}
			}while(1);
		}
		else{
			for (i = 0; i < final_count[max_final]; i++) {
				//�ִ� 18,000���� �ҷ�����
				if(i>18000){
					printf("�ε�� �����Ͱ� 18000���� �ѽ��ϴ�.\n");
					break;
				}
			}
		}

		z = i;
		printf("�ѷε尳��: %d.\n", z);

		printf("[**1**]\n");
		/////////////////////////////
		score = (short *)malloc(sizeof(short)*z);
		if (!score ) {
			fprintf(stderr, "scoring_and_output_4text(): score[] �迭 �Ҵ� ����!\n");
			return;
		}
		/////////////////JAY
		/*
		for (i = 0; i < k; i++) {			// �������� ������ ���ؼ�.
			cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
			score[i] = CalSimilarity((const unsigned char *)&memory_db[cur], i);	// �˻� ����� ���� ���� ��ġ���� ����Ѵ�.
		}
		quickSort5(score, 0, k-1);	// ���� ���ھ� ���� �������� ����
		for (i = k; i < z; i++) {			// �� ���� ������ ���ؼ�.
			cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
			score[i] = CalSimilarity((const unsigned char *)&memory_db[cur], i);	// �˻� ����� ���� ���� ��ġ���� ����Ѵ�.
		}
		quickSort5(score, k, z-1);	// ���� ���ھ� ���� �������� ����
		*/
/////////////////JAY

		//������ ������ �ʴ´ٰ� ������
		for (i = 0; i < z; i++) {	//���� ����������.
			cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
			score[i] = CalSimilarity((const unsigned char *)&memory_db[cur], i);	// �˻� ����� ���� ���� ��ġ���� ����Ѵ�.
		}
		quickSort5(score, 0, z-1);	// ���� ���ھ� ���� �������� ����
		
/////////////////JAY

		print_cnt = 0;
		for (i = 0; i < z; i++) {
			if (score[i] <= MAX_MINUS_POINT)	// ������ MAX_NINUS_POINT ������ ���� �� ������ ����.
				print_cnt++;
		}

		if (print_cnt >= MAX_OUT_LINES) {		// ������ MAX_NINUS_POINT ������ �ٷθ� MAX_OUT_LINES ���� ����� �� ������
			int extra_print_cnt = 0;

			for (i = 0; i < z; i++) {
				if (score[i] <= MAX_MINUS_POINT) {
					if (eo_count >= 3) {	// ���� ���̰� �� ���� �̻��� ������ merged_final_degree[]�� ����� ����
						printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
					}
					else {					// ���� ���̰� �� ������ ������ ���� ����� �ʿ� �����Ƿ� mfd �迭�� ������ �ʴ´�.
						printf("%7d   ", final[max_final][i]+1);
					}
					cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
					printf("%s(%d)\n", &memory_db[cur], score[i]);

					extra_print_cnt++;
					if (extra_print_cnt >= MAX_OUT_LINES)
						break;
				}
			}
		}
		else {	// ������ MAX_NINUS_POINT ������ �ٷθ� MAX_OUT_LINES ���� ����ϱ⿡�� ������.
			int extra_print_cnt = 0;

			for (i = 0; i < z; i++) {
				if (score[i] <= MAX_MINUS_POINT) {
					if (eo_count >= 3) {	// ���� ���̰� �� ���� �̻��� ������ merged_final_degree[]�� ����� ����
						printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
					}
					else {					// ���� ���̰� �� ������ ������ ���� ����� �ʿ� �����Ƿ� mfd �迭�� ������ �ʴ´�.
						printf("%7d   ", final[max_final][i]+1);
					}
					cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
					printf("%s(%d)\n", &memory_db[cur], score[i]);
				}
			}

			for (i = 0; i < z; i++) {
				if (score[i] > MAX_MINUS_POINT) {
					if (eo_count >= 3) {	// ���� ���̰� �� ���� �̻��� ������ merged_final_degree[]�� ����� ����
						printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
					}
					else {					// ���� ���̰� �� ������ ������ ���� ����� �ʿ� �����Ƿ� mfd �迭�� ������ �ʴ´�.
						printf("%7d   ", final[max_final][i]+1);
					}
					cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
					printf("%s(%d)\n", &memory_db[cur], score[i]);

					extra_print_cnt++;
					if (extra_print_cnt >= (MAX_OUT_LINES - print_cnt))
						break;
				}
			}
		}

		if (eo_count >= 3)
			printf("------------- (�������� >= 3) ��Ī�κ� �κ� ���� ��� ��(%d��) -------------\n", z);
		else
			printf("------------- (�������� < 3) ��Ī�κ� ���� ���� ��� ��(%d��) -------------\n", z);

		free(score);
	}
	/*
	if (!fFullyIncluded) {	// �����ϸ����δ� ������ ���� �����ϰ� �ִ� ������ ã�� �� ���� ��, �ּұ����� Ȯ�� �˻�
		// ���� ������ �� �� ���ڰ� ���ÿ� �����ϴ� �ٹ�ȣ�� ������ ����. 
		for (i = 1; i < MAX_QUERY_LEN; i++)
			final_count[i] = 0; // �ſ� �߿�
		
		max_final = 0;
		for (j = 2; j < eo_count; j++) {
			unsigned char former, latter;

			former = eo_string[j][0];
			latter = eo_string[j][1];

			for (i = 0; i < final_count[j-2]; i++) {
				cur = position_in_memory[final[j-2][i]];
				len = namepart_len[final[j-2][i]] + addrpart_len[final[j-2][i]];
														// address ��� �κи� ���� �� ���� ���� ����.
				for (; ;) {
					t_char = memory_db[cur++]; 
					len--;
					if ((unsigned int)t_char >= 128) {
						han[0] = t_char;
						t_char = memory_db[cur++];
						len--;

						if (han[0] == former && t_char == latter) {
							final[j-1][final_count[j-1]++] = final[j-2][i];
							max_final = j-1;
							break;
						}
					}
					else { // ��ũ�� ������ �Ϲ� �ƽ�Ű ����
						if (t_char == former) {
							final[j-1][final_count[j-1]++] = final[j-2][i];
							max_final = j-1;
							break;
						}
					}
					if (len <= 0)
						break;
				} // end of for (; ;)
			} // end of for (i = 0; i < final_count[j-2]; i++)
		} // end of for (j = 2; j < eo_count; j++)

		// �����찡 �߰��� �κ�!
		// ����: �̸� �κи����� ���� ���Ե� ���� ���� ��
		printf("--------------- �ּҺκб��� Ȯ�� �˻� ���(%d ��) ---------------\n", final_count[max_final]);
		for (i = 0; i < final_count[max_final]; i++) {	// ���� ���ڿ��� ����� ��� �ٿ� ���� �����ϴ� ��� �ٿ� ���ؼ�...
			printf("�ٹ�ȣ %d: ", final[max_final][i]+1);
			cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
			len = namepart_len[final[max_final][i]] + addrpart_len[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� �̸� �κ� + �ּ� �κ� ����
			for (;;) {
				t_char = memory_db[cur++]; 
				len--;
				if ((unsigned int)t_char >= 128) {
					han[0] = t_char;
					t_char = memory_db[cur++];
					len--;

					han[1] = t_char;
					han[2] = '\0';

					t_int = (int)(*(unsigned short*)han);
					printf("%s", han);
				}
				else { // ��ũ�� ������ �Ϲ� �ƽ�Ű ����
					t_int = (int)t_char;
					printf("%c", t_char);
				}
				if (len <= 0)
					break;
			} // end of for (;;)
			printf("\n");
		} // end of for (i = 0; i < final_count[max_final]; i++)

	} // end of else {
	*/
}

/*
 * scoring_search_results(query_cho, query_count, query_alpha, query_alpha_count);
 * ���� ���ھ �Լ��� ���� ��������Ƿ� �ʿ� ���� �ڸ�Ʈ ó����.
 */
/*
void
scoring_and_output_4chosung(short *query_cho, int query_count, int *in_alpha, int in_alpha_count)
{
	int score=0;
	short c_count = 0;					// ��Ī�κ��� �ʼ� ������ ���� ���� ���
	short c_alpha_count = 0;
	//short c_ad_count = 0;				// �ּҺκ��� �ʼ� ������ ���� ���� ���
	//short c_ad_alpha_count = 0;
	int cur_entry = 0;
	int cur;
	char han[HANGUL_BYTE+1];			// �ѱ� ó�� �� ���� �ϳ��� ��� ���� ���� ����. NULL ���ڱ��� �����ϱ� ���� +1.
	char c_han[HANGUL_BYTE+1];			// �ϼ��� ���ڸ� ���������� �ٲ� �� ����ϱ� ���� ����.
	unsigned short johapcode;
	int i;
	short namepart_cho[MAX_JAEUM_CNT]={0};
	char namepart_alpha[MAX_JAEUM_CNT] = {0};
	//short addrpart_cho[MAX_JAEUM_CNT]={0};
	//char addrpart_alpha[MAX_JAEUM_CNT] = {0};
	bool used[60]={false};
	int *cho_score;

	// ���ھ� ���� �� ����� cho_score[](cho_final_count[cho_max_final] ��)�� �̸� �Ҵ��صд�.
	cho_score = (int *)malloc(sizeof(int)*cho_final_count[cho_max_final]);
	if (cho_score == NULL) {
		fprintf(stderr, "cho_score[] �迭 �Ҵ� ����. �޸𸮰� �����ؼ� �ϰ̴ϴ�.\n");
		exit(-10);
	}
	//memset(cho_score, 0, sizeof(int)*cho_final_count[cho_max_final]);	// �̸� Ŭ�����صд�.

	//***************************** score ����*************************************************
	for (int f = 0; f < cho_final_count[cho_max_final]; f++) { // cho_final_count[cho_max_final]�� ������ŭ
		score = 0;
		c_count = 0;		// ��Ī �κ� ���̸� ���� �뵵
		//c_ad_count = 0;		// �ּ� �κ� ���̸� ���� �뵵
		// cur = 0;
		cur_entry = cho_final[cho_max_final][f];	// ���� ���γѹ��� cur_entry�� �־���
		cur = position_in_memory[cur_entry];		// ���� ������ ��� �ִ� memory_db[] �迭 ���� ��ġ
		
		for (int w = 0; w < 30; w++)
			used[w] = false;
		
		// �ʼ��˻��� �ٹ�ȣ cur_entry�� �ش��ϴ� ��Ī�κ� ���ڿ����� �ʼ� �̾��ִ� for��
		int q;
		for (q = 0; q < namepart_len[cur_entry]; q++) { // �� ���� ��Ī �κ� ���� ��ŭ
			if ((unsigned short)memory_db[cur] >= 128) {	// �ѱ��̸�
				han[0] = memory_db[cur++]; 
				han[1] = memory_db[cur++];
				han[2] = '\0';
				HangulCodeConvert(KS, han, c_han);
				johapcode = ((unsigned char)c_han[0] << 8) | ((unsigned char)c_han[1]); 
				namepart_cho[c_count] = (johapcode >> 10) & 0x1f;		// �ʼ��� �����Ѵ�.
				c_count++;
				q++;	// �ѱ��̴ϱ� �� ����Ʈ �� �������ش�.
			}
			else	// ���ڴ� �ƴ� �ƽ�Ű�ڵ�
				if (memory_db[cur] != 0)		// ���ڿ� �� ���� �� ���ڴ� ������ ���Ե��� �ʰ� �ؾ� ��.
					namepart_alpha[c_alpha_count++] = memory_db[cur++];
		}
		
		// �ʼ��˻��� �ٹ�ȣ cur_entry�� �ش��ϴ� �ּҺκ� ���ڿ����� �ʼ� �̾��ִ� for��
		// �ּҺκ� ���ڿ��� ������ ���ϴ� ���� �ϴ��� �����Ѵ�. �ϼ��������� �װ� �� �ϰ� ����.
		// �ּ� �κп� ���� �˻��� �ϴ� �� �ϹǷ� �ڸ�Ʈ ó�� ����.
		//for (q = 0; q < addrpart_len[cur_entry]; q++) {	// �� ������ �ּҺκ� ���� ��ŭ
		//	if ((unsigned short)memory_db[cur] >= 128) {	// �ѱ��̸�
		//		han[0] = memory_db[cur++]; 
		//		han[1] = memory_db[cur++];
		//		han[2] = '\0';
		//		HangulCodeConvert(KS, han, c_han);
		//		johapcode = ((unsigned char)c_han[0] << 8) | ((unsigned char)c_han[1]); 
		//		addrpart_cho[c_ad_count] = (johapcode >> 10) & 0x1f;	// �ʼ� ����
		//		c_ad_count++;
		//		q++;	// �ѱ��̴ϱ� �� ����Ʈ �� �����Ѵ�.
		//	}
		//	else
		//		if (memory_db[cur] != 0)		// ���ڿ� �� ���� �� ���ڴ� ������ ���Ե��� �ʰ� �ؾ� ��.
		//			addrpart_alpha[c_ad_alpha_count++] = memory_db[cur++];
		//}

		for (i = 0; i < query_count; i++) {	// ������ ���� ������ ������ŭ
			for (int b = 0; b < c_count; b++) {	// �� ������ �����ϰ� �ִٰ� �˻��� ���� ��Ī �κ� �ʼ� ������ŭ
				if (query_cho[i] == namepart_cho[b]) {	// �� �ٿ��� ������ �ʼ��� ���� ��ġ���� �� ����
					for (int u = 1; u < c_count-b && u < query_count-i; u++) {
						if (query_cho[i+u] == namepart_cho[b+u]) {		// ���� �ʼ��� ������
							used[b+u] = true;
							score += 10;
						}
						else
							break;
					} // end of for (int u = 1; u < c_count-b && u < in_count-i; u++)
				}
			} // end of for (int b = 0; b < c_count; b++)

			// �ּ� �κп� ���� ���ھ�� �ϴ� �����ϱ�� �ؼ� �ڸ�Ʈ ó�� ����.
			//for (int r = 0; r < c_ad_count; r++) {
			//	if (query_cho[i] == addrpart_cho[r]) {
			//		score += 10;
			//		for (int u = 1; u < c_ad_count-r && u < query_count-i; u++) { 
			//			if (query_cho[i+u] == addrpart_cho[r+u]) {
			//				used[r+u] = true;
			//				score += 10;
			//			}
			//			else
			//				break;
			//		} // end of for (int u = 1; u < c_ad_count-r && u < in_count-i; u++)
			//	}
			//} // end of for (int r = 0; r < c_ad_count; r++)
		} // end of for (i = 0; i < query_count; i++)
		
		for (int l = 0; l < c_count; l++)	// ��Ī �κ� ���� ���� ��ŭ
			if (used[l] == false)	// ������ ���� �� �� �ʼ� �ϳ��� 10���� ����
				score -= 10;

		cho_score[f] = score;	// �� ���� ���� ������ �����.
	}

	printf("quickSort(cho_score, 0, %d)...", cho_final_count[cho_max_final]-1);
	quickSort6(cho_score, 0, cho_final_count[cho_max_final]-1);	// cho_score[]�� �������� �����ϸ鼭 cho_final[cho_max_final][...]�� ���� �ٲ��ش�.
	printf("done.\n");

	// ���� �˻� ��� ������ִ� �κ�
	for (i = 0; i < cho_final_count[cho_max_final]; i++) {
		int cur;
		int len;

		cur = position_in_memory[cho_final[cho_max_final][i]];
		len = namepart_len[cho_final[cho_max_final][i]];
		
		printf("%7d %5d�� %s", cho_final[cho_max_final][i]+1, cho_score[i], &memory_db[cur]);

		if (addrpart_len[cho_final[cho_max_final][i]])
			printf("@%s", &memory_db[cur+len]);

		printf("\n");
		if (i >= 99)
			break;
	} // end of for (int f = 0; f < cho_final_count[cho_max_final]; f++)
	free(cho_score);
}
*/

/*
 * �� �˻� ����� ���ļ� ���ھ �Ѵ�.
 */
void
integrated_scoring_and_output(int fFullyIncluded, const char *in_query, int *linenumbers, int line_count)
{
	int strCnt;
	unsigned int cur;
	unsigned char strArray[MAX_QUERY_LEN][HANGUL_BYTE+1];
	int k, i, z;
	short *score;
	int print_cnt;

	strCnt = StrToEoArray((const unsigned char *)in_query, strArray);	// ������ ������ �ϼ��� ���ڴ� �״��, �ϼ��� �ʼ��ڵ�� ���������� �ٲ۴�.
	//for (int i = 0; i < strCnt; i++) {
	//	if (strArray[i][2] == '\0')			// �ϼ��� ������ ����
	//		printf("%s ", strArray[i]);
	//	else if (strArray[i][2] == '\1')	// �ʼ� ������ �ڵ� ����
	//		printf("0x%02x ", strArray[i][0]);
	//}
	printf("\n");

	if (fFullyIncluded >= 0) {	// �ϼ��� �˻� ����� ������� ���� ���ھ�� �ǽ���.
		mfd = merged2_final_degree;
		if (fFullyIncluded) {	// ��������
			for (i = 0; mfd[i] >= mfd[0]; i++);		// �������� ������ ������ ��´�.
			k = i;
			//printf("�ְ� ���� ����: %d.\n", k);
			for (i = k; mfd[i] >= mfd[k]; i++);	// ���� ������ ������ ��´�.
			z = i;
			//printf("���� ���� ����: %d.\n", z-k);
			score = (short *)malloc(sizeof(short)*z);
			if (!score ) {
				fprintf(stderr, "integrated_scoring_and_output(): score[] �迭 �Ҵ� ����!\n");
				return;
			}
			for (i = 0; i < k; i++) {			// �������� ������ ���ؼ�.
				cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
				score[i] = CalMixedSimilarity(strArray, strCnt, (const unsigned char *)&memory_db[cur]);	// �˻� ����� ���� ���� ��ġ���� ����Ѵ�.
			}
			quickSort5(score, 0, k-1);	// ���� ���ھ� ���� �������� ����
			for (i = k; i < z; i++) {			// ���� ������ ���ؼ�.
				cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
				score[i] = CalMixedSimilarity(strArray, strCnt, (const unsigned char *)&memory_db[cur]);	// �˻� ����� ���� ���� ��ġ���� ����Ѵ�.
			}
			quickSort5(score, k, z-1);	// ���� ���ھ� ���� �������� ����
			printf("--------------- �ؽ�Ʈ �˻� ��� ���� ���ھ ��� ---------------\n");
			printf(" �ٹ�ȣ ���� �˻����(���ھ�)\n");

			print_cnt = 0;
			for (i = 0; i < z; i++) {
				if (score[i] <= MAX_MINUS_POINT)	// ������ MAX_NINUS_POINT ������ ���� �� ������ ����.
					print_cnt++;
			}
			if (print_cnt >= MAX_OUT_LINES) {		// ������ MAX_NINUS_POINT ������ �ٷθ� MAX_OUT_LINES ���� ����� �� ������
				int extra_print_cnt = 0;

				for (i = 0; i < z; i++) {			// �������� ���������� ����.
					if (score[i] <= MAX_MINUS_POINT) {
						printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
						cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
						printf("%s(%d)\n", &memory_db[cur], score[i]);

						extra_print_cnt++;
						if (extra_print_cnt >= MAX_OUT_LINES)
							break;
					}
				}
			}
			else {	// ������ MAX_MINUS_POINT ������ �ٷθ� MAX_OUT_LINES ���� ����ϱ⿡�� ������.
				int extra_print_cnt = 0;

				for (i = 0; i < z; i++) {		// �ϴ� ������ MAX_MINUS_POINT ������ �ٵ��� ���� ����Ѵ�.
					if (score[i] <= MAX_MINUS_POINT) {
						printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
						cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
						printf("%s(%d)\n", &memory_db[cur], score[i]);
					}
				}

				for (i = 0; i < z; i++) {
					if (score[i] > MAX_MINUS_POINT) {	// ������ MAX_MINUS_POINT �ʰ��� �ٵ��� ����ϵ� MAX_OUT_LINES�� ���� �ʰ� �Ѵ�.
						printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
						cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
						printf("%s(%d)\n", &memory_db[cur], score[i]);

						extra_print_cnt++;
						if (extra_print_cnt >= (MAX_OUT_LINES - print_cnt))
							break;
					}
				}
			}
			printf("------------- �ؽ�Ʈ �˻�(��Ī�κ� ���� ����) ��� ���� ���ھ ���(%d��) -------------\n", z);
			free(score);
		}
		else {					// �κ�����
			for (i = 0; i < final_count[max_final]; i++) {
				if (eo_count >= 3) {	// ���� ���̰� �� ���� �̻��� ������ merged_final_degree[]�� ����� ����
					if (mfd[i] < mfd[0])	// �ְ����������� ����.
						break;
				}
			}
			k = i;	// ����� �˻� ����� ������ �˾Ƴ���.
			//printf("�ְ� ���� ����: %d.\n", k);
			for (i = k; i < final_count[max_final]; i++) {
				if (eo_count >= 3) {	// ���� ���̰� �� ���� �̻��� ������ merged_final_degree[]�� ����� ����
					if (mfd[i] < mfd[k])	// ���� �������� ����.
						break;
				}
			}
			z = i;	// ����� �˻� ����� ������ �˾Ƴ���.
			//printf("���� ���� ����: %d.\n", z-k);
			score = (short *)malloc(sizeof(short)*z);
			if (!score ) {
				fprintf(stderr, "integrated_scoring_and_output(): score[] �迭 �Ҵ� ����!\n");
				return;
			}
			for (i = 0; i < k; i++) {			// �������� ������ ���ؼ�.
				cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
				score[i] = CalMixedSimilarity(strArray, strCnt, (const unsigned char *)&memory_db[cur]);	// �˻� ����� ���� ���� ��ġ���� ����Ѵ�.
			}
			quickSort5(score, 0, k-1);	// ���� ���ھ� ���� �������� ����
			for (i = k; i < z; i++) {			// ���� ������ ���ؼ�.
				cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
				score[i] = CalMixedSimilarity(strArray, strCnt, (const unsigned char *)&memory_db[cur]);	// �˻� ����� ���� ���� ��ġ���� ����Ѵ�.
			}
			quickSort5(score, k, z-1);	// ���� ���ھ� ���� �������� ����
			printf("--------------- �ؽ�Ʈ �˻� ��� ���� ���ھ ��� ---------------\n");
			if (eo_count >= 3)
				printf(" �ٹ�ȣ ���� �˻����(���ھ�)\n");
			else
				printf(" �ٹ�ȣ �˻����(���ھ�)\n");

			print_cnt = 0;
			for (i = 0; i < z; i++) {
				if (score[i] <= MAX_MINUS_POINT)	// ������ MAX_NINUS_POINT ������ ���� �� ������ ����.
					print_cnt++;
			}

			if (print_cnt >= MAX_OUT_LINES) {		// ������ MAX_NINUS_POINT ������ �ٷθ� MAX_OUT_LINES ���� ����� �� ������
				int extra_print_cnt = 0;

				for (i = 0; i < z; i++) {
					if (score[i] <= MAX_MINUS_POINT) {
						if (eo_count >= 3) {	// ���� ���̰� �� ���� �̻��� ������ merged_final_degree[]�� ����� ����
							printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
						}
						else {					// ���� ���̰� �� ������ ������ ���� ����� �ʿ� �����Ƿ� mfd �迭�� ������ �ʴ´�.
							printf("%7d   ", final[max_final][i]+1);
						}
						cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
						printf("%s(%d)\n", &memory_db[cur], score[i]);

						extra_print_cnt++;
						if (extra_print_cnt >= MAX_OUT_LINES)
							break;
					}
				}
			}
			else {	// ������ MAX_MINUS_POINT ������ �ٷθ� MAX_OUT_LINES ���� ����ϱ⿡�� ������.
				int extra_print_cnt = 0;

				for (i = 0; i < z; i++) {		// �ϴ� ������ MAX_MINUS_POINT ������ �ٵ��� �� ����ϰ�,
					if (score[i] <= MAX_MINUS_POINT) {
						if (eo_count >= 3) {	// ���� ���̰� �� ���� �̻��� ������ merged_final_degree[]�� ����� ����
							printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
						}
						else {					// ���� ���̰� �� ������ ������ ���� ����� �ʿ� �����Ƿ� mfd �迭�� ������ �ʴ´�.
							printf("%7d   ", final[max_final][i]+1);
						}
						cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
						printf("%s(%d)\n", &memory_db[cur], score[i]);
					}
				}

				for (i = 0; i < z; i++) {		// �׷��� MAX_OUT_LINES�� �� ä���� ������ ���̹Ƿ� �����Ѵ�.
					if (score[i] > MAX_MINUS_POINT) {
						if (eo_count >= 3) {	// ���� ���̰� �� ���� �̻��� ������ merged_final_degree[]�� ����� ����
							printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
						}
						else {					// ���� ���̰� �� ������ ������ ���� ����� �ʿ� �����Ƿ� mfd �迭�� ������ �ʴ´�.
							printf("%7d   ", final[max_final][i]+1);
						}
						cur = position_in_memory[final[max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
						printf("%s(%d)\n", &memory_db[cur], score[i]);

						extra_print_cnt++;
						if (extra_print_cnt >= (MAX_OUT_LINES - print_cnt))
							break;
					}
				}
			}
			if (eo_count >= 3)
				printf("------------- �ؽ�Ʈ �˻�(���� ���� 3 �̻�. ��Ī�κ� �κ� ����) ��� ���� ���ھ ���(%d��) -------------\n", z);
			else
				printf("------------- �ؽ�Ʈ �˻�(���� ���� 3 �̸�. ��Ī�κ� ���� ����) ��� ���� ���ھ ���(%d��) -------------\n", z);
			free(score);
		}
	}
	else {						// �ʼ� �˻� ����� ������� ���� ���ھ�� �ǽ���.
		k = cho_final_count[cho_max_final];
		score = (short *)malloc(sizeof(short)*k);
		if (!score ) {
			fprintf(stderr, "integrated_scoring_and_output(): score[] �迭 �Ҵ� ����!\n");
			return;
		}
		for (i = 0; i < k; i++) {			// �˻��� ��� ����� ���ؼ�.
			cur = position_in_memory[cho_final[cho_max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
			score[i] = CalMixedSimilarity(strArray, strCnt, (const unsigned char *)&memory_db[cur]);	// �˻� ����� ���� ���� ��ġ���� ����Ѵ�.
		}
		quickSort7(score, 0, k-1);	// ���� ���ھ� ���� �������� ����
		printf("--------------- �ʼ� �˻� ��� ���� ���ھ ��� ---------------\n");
		printf(" �ٹ�ȣ  �˻����(���ھ�)\n");

		if (k > MAX_OUT_LINES)
			k = MAX_OUT_LINES;
		for (i = 0; i < k; i++) {
			printf("%7d  ", cho_final[cho_max_final][i]+1);
			cur = position_in_memory[cho_final[cho_max_final][i]];	// �� �ٹ�ȣ�� �ش��ϴ� ���� ��� ������ ����Ǿ� �ִ� ��ġ
			printf("%s(%d)\n", &memory_db[cur], score[i]);
		}
		printf("------------- �ʼ� �˻� ��� ���� ���ھ ���(%d��) -------------\n", cho_final_count[cho_max_final]);
		free(score);
	}
}

/*
 * ���� ���ڿ��� �˻��� ���ڿ� ���� ���絵�� ����Ѵ�. �ϼ��� �˻������� ����ؾ� ��.
 * ���� ��: ����. 0: ��ġ, Ŭ ���� ����ġ���� ���� ����.
 * �Է� str: �˻��� ���ڿ�. ���� ���ڿ��� ep_string[][]�� �̹� ����
 */
//CalMixedSimilarity�� �ʼ��� Ȥ�� �ʼ��� ���ԵǾ�����쿡 ���� ����ȭ �Լ��̰�
//CalSimilarity�� �ϼ����� ��츸 ���� ����ȭ �Լ� 
int
CalSimilarity(const unsigned char *str, int no_str)
{
	///*
	//�ð�������
	//LARGE_INTEGER liCounter1, liCounter2, liFrequency;
	//QueryPerformanceFrequency(&liFrequency); // ���ļ�(1�ʴ� �����Ǵ� ī��Ʈ��)�� ���Ѵ�.

	int strCount;
	unsigned char strArray[MAX_QUERY_LEN][HANGUL_BYTE+1];
	strCount = StrToStrArray(str, strArray);

///////////////���� JAY
	int i, j;
	int query_same[MAX_QUERY_LEN][5];
	int found_match_no;

	//�� �迭�� �ֱ� ����
	int query_same_array[30];
	for(i=0; i<30; i++){
		query_same_array[i] = -5;
	}
	int query_same_array_isChanged[30];
	int array_match_no;

	//KEB ����ȭ 
	//JAY�� 5�� �Ѱ� 5->10���� �÷��� �׸��� MAX_BLOCK���� ���ȭ��Ŵ
	//���� ���Ǿ� ���������꿡�� �����÷���߻��ؼ�(����� 24222���� 5���̻��� �����Ͱ� ����)
	//�ߺ��ܾ ���������� �� �迭�� ũ�� ����
	//�ߺ��ܾ�: �ߺ����ھƴ� ex) �ٷιٷ�, ������� ���
	int close_list[MAX_BLOCK];
	int estmLevel =0;//�����ܰ踦 ����ȭ ��Ű�� ���� ����
	//����ȭ��
	

	//�ϼ����� ������ �ڵ�� ��ȯ�ϱ� ���� ������ ����
	unsigned short johapcode;
	unsigned int cho_code_q, jung_code_q, jong_code_q;
	unsigned int cho_code_d, jung_code_d, jong_code_d;
	char han_query[HANGUL_BYTE+1];				// �ѱ� ó�� �� ���� �ϳ��� ��� ���� ���� ����. NULL ���ڱ��� �����ϱ� ���� +1.
	char han_data[HANGUL_BYTE+1];
	char c_han_query[HANGUL_BYTE+1];			// �ϼ��� ���ڸ� ���������� �ٲ� �� ����ϱ� ���� ����.
	char c_han_data[HANGUL_BYTE+1];

	//������ ���� ����
	int isChangedValue[30];						//������ ������ ��������		//30����, �������� �ִ밳���� �����Ѵ�.
	for(i=0; i<20; i++){
		isChangedValue[i] = 0;
	}
	char ChangedValue[30][3];					//������ �� �� ����			// Q:Ŭ D:ũ �϶�, 'Ŭ'����ȴ�.
	

	
	//������ ����
	fprintf(fp, "%d.\t",no_str);
	for(i=0;i<strCount;i++){
		fprintf(fp, "%s",strArray[i]);
	}
	fprintf(fp, "\n");
	

	//����, ��ġ�Ǵ� ���� ��ġ �迭 �ۼ� ��Ʈ
	array_match_no = 1;

	for(i=0; i<eo_count; i++){
		found_match_no = 1;
		query_same_array[0] = -1;

		for(j=0; j<strCount; j++){
			// 1) �� ���ڰ� ���� ���
			if (!strcmp((const char *)eo_string[i], (const char *)strArray[j])){
				query_same[i][found_match_no]=j;	//������ i��° ���ڿ� ��ġ�Ǵ� ������ ������ ��ġ�� �ִ´�.
				found_match_no++;

				//�ѹ迭 �ȿ� �ֱ�
				query_same_array[array_match_no] = j;
				query_same_array_isChanged[array_match_no] = 0;	//0�� false
				array_match_no++;
			}
			// 2) �� ���ڰ� �ٸ� ���
			else{
				if ((unsigned short)eo_string[i][0] >= 128 && (unsigned short)strArray[j][0] >= 128) {	// ������ �����Ͱ� �ѱ��̸�
					
					//����, �������� ��/��/���� ����
					han_data[0] = strArray[j][0]; 
					han_data[1] = strArray[j][1];
					han_data[2] = '\0';
					HangulCodeConvert(KS, han_data, c_han_data);
					johapcode = ((unsigned char)c_han_data[0] << 8) | ((unsigned char)c_han_data[1]); 
					cho_code_d = (johapcode >> 10) & 0x1f;		// �������� �ʼ� ����
					jung_code_d = (johapcode >> 5) & 0x1f;		// �������� �߼� ����
					jong_code_d = johapcode & 0x1f;				// �������� ���� ����

					//����, ������ ��/��/���� ����
					han_query[0] = eo_string[i][0]; 
					han_query[1] = eo_string[i][1];
					han_query[2] = '\0';
					HangulCodeConvert(KS, han_query, c_han_query);
					johapcode = ((unsigned char)c_han_query[0] << 8) | ((unsigned char)c_han_query[1]); 
					cho_code_q = (johapcode >> 10) & 0x1f;		// ������ �ʼ� ����
					jung_code_q = (johapcode >> 5) & 0x1f;		// ������ �߼� ����
					jong_code_q = johapcode & 0x1f;				// ������ ���� ����
					
					int cho_same = 0;	//�ʼ��� ���ų� ��/��, ��/��, ��/��, ��/��, ��/�� �ϰ��
					if(cho_code_q == cho_code_d
						|| ((cho_code_q == 3 && cho_code_d == 17) || (cho_code_q == 17 && cho_code_d == 3))		//���� ��
						|| ((cho_code_q == 6 && cho_code_d == 18) || (cho_code_q == 18 && cho_code_d == 6))		//���� ��
						|| ((cho_code_q == 10 && cho_code_d == 19) || (cho_code_q == 19 && cho_code_d == 10))	//���� ��
						|| ((cho_code_q == 16 && cho_code_d == 15) || (cho_code_q == 15 && cho_code_d == 16))	//���� ��
						|| ((cho_code_q == 11 && cho_code_d == 12) || (cho_code_q == 12 && cho_code_d == 11))	//���� ��
						){
							cho_same = 1;
					}

					//�ʼ��� ���ų� ��/��, ��/��, ��/��, ��/��, ��/�� �ϰ��
					if(cho_same == 1){
						//  (1) �߼��� ���� ��
						if( jung_code_q == jung_code_d ){
							//������ ���� ��
							if( jung_code_q == jung_code_d && jong_code_q == jong_code_d ){
								query_same[i][found_match_no]=j;	//������ i��° ���ڿ� ��ġ�Ǵ� ������ ������ ��ġ�� �ִ´�.
								found_match_no++;

								//�ѹ迭 �ȿ� �ֱ�
								query_same_array[array_match_no] = j;
								query_same_array_isChanged[array_match_no] = 1;	//1�� true (��, ���� ���ڰ� �ƴ϶�, ������ �����̴�.)
								array_match_no++;
								//������ ���̶�� ���� ǥ��
								isChangedValue[j] = 1;
								//������ �� ����
								sprintf(ChangedValue[j], "%s", eo_string[i]);
							}
							//���� �ϳ��� ������ ����, �ϳ��� ������ ���϶�
							if((jong_code_q == 9 && jong_code_d == 1) || ( jong_code_q == 1 && jong_code_d == 9)){ //1�� ������ ������, 9�� '��'������ �ǹ�
								query_same[i][found_match_no]=j;	//������ i��° ���ڿ� ��ġ�Ǵ� ������ ������ ��ġ�� �ִ´�.
								found_match_no++;

								//�ѹ迭 �ȿ� �ֱ�
								query_same_array[array_match_no] = j;
								query_same_array_isChanged[array_match_no] = 1;	//1�� true (��, ���� ���ڰ� �ƴ϶�, ������ �����̴�.)
								array_match_no++;
								//������ ���̶�� ���� ǥ��
								isChangedValue[j] = 1;
								//������ �� ����
								sprintf(ChangedValue[j], "%s", eo_string[i]);
							}
							//����ȭ �߰��κ�(������/�������� �����ϱ� ����)
							//������ ��/�� Ȥ�� ��/���϶� 
							else if((jong_code_q == 5 && jong_code_d == 9) || ( jong_code_q == 9 && jong_code_d == 5)){
								query_same[i][found_match_no]=j;	//������ i��° ���ڿ� ��ġ�Ǵ� ������ ������ ��ġ�� �ִ´�.
								found_match_no++;

								//�ѹ迭 �ȿ� �ֱ�
								query_same_array[array_match_no] = j;
								query_same_array_isChanged[array_match_no] = 1;	//1�� true (��, ���� ���ڰ� �ƴ϶�, ������ �����̴�.)
								array_match_no++;
								//������ ���̶�� ���� ǥ��
								isChangedValue[j] = 1;
								//������ �� ����
								sprintf(ChangedValue[j], "%s", eo_string[i]);

								//�����ܰ� ����ȭ
								estmLevel ++;//�� �������� ���� �������� ��Ÿ��. �����ܰ迡�� �����ϴµ� ���
							}
							//�������� vs ���� ��
							else if((jong_code_q == 21 && jong_code_d == 1) || ( jong_code_q == 1 && jong_code_d == 21)){
								query_same[i][found_match_no]=j;	//������ i��° ���ڿ� ��ġ�Ǵ� ������ ������ ��ġ�� �ִ´�.
								found_match_no++;

								//�ѹ迭 �ȿ� �ֱ�
								query_same_array[array_match_no] = j;
								query_same_array_isChanged[array_match_no] = 1;	//1�� true (��, ���� ���ڰ� �ƴ϶�, ������ �����̴�.)
								array_match_no++;
								//������ ���̶�� ���� ǥ��
								isChangedValue[j] = 1;
								//������ �� ����
								sprintf(ChangedValue[j], "%s", eo_string[i]);
							}
						}
						//  (2) �߼��� �ٸ� ��
						else{
							// (3) ������ ���� ��
							if(jong_code_q == jong_code_d){ 
								//  �ʼ��� ��(14)/��(15)/��(16)�϶�,
								if( (cho_code_q == 14) || (cho_code_q == 15) || (cho_code_q == 16) ){
									if(((jung_code_q == 13 && jung_code_d == 19) || ( jung_code_q == 19 && jung_code_d == 13))		//13=��, 19=��
										||((jung_code_q == 3 && jung_code_d == 5) || ( jung_code_q == 5 && jung_code_d == 3))		//3=��, 5=��
										||((jung_code_q == 20 && jung_code_d == 26) || ( jung_code_q == 26 && jung_code_d == 20))	//20=��, 26=��
										||((jung_code_q == 7 && jung_code_d == 11) || ( jung_code_q == 11 && jung_code_d == 7))){	//7=��, 11=��	
											query_same[i][found_match_no]=j;	//������ i��° ���ڿ� ��ġ�Ǵ� ������ ������ ��ġ�� �ִ´�.
											found_match_no++;

											//�ѹ迭 �ȿ� �ֱ�
											query_same_array[array_match_no] = j;
											query_same_array_isChanged[array_match_no] = 1;	//1�� true (��, ���� ���ڰ� �ƴ϶�, ������ �����̴�.)
											array_match_no++;
											//������ ���̶�� ���� ǥ��
											isChangedValue[j] = 1;
											//������ �� ����
											sprintf(ChangedValue[j], "%s", eo_string[i]);
									}
								}
								//���� �ϳ��� �߼��� ���̰�, �ϳ��� ���϶�
								if((jung_code_q == 4 && jung_code_d == 10) || ( jung_code_q == 10 && jung_code_d == 4)){	//4=��, 10=��
									query_same[i][found_match_no]=j;	//������ i��° ���ڿ� ��ġ�Ǵ� ������ ������ ��ġ�� �ִ´�.
									found_match_no++;

									//�ѹ迭 �ȿ� �ֱ�
									query_same_array[array_match_no] = j;
									query_same_array_isChanged[array_match_no] = 1;	//1�� true (��, ���� ���ڰ� �ƴ϶�, ������ �����̴�.)
									array_match_no++;
									//������ ���̶�� ���� ǥ��
									isChangedValue[j] = 1;
									//������ �� ����
									sprintf(ChangedValue[j], "%s", eo_string[i]);
								}
								//���� �ϳ��� �߼��� ���̰�, �ϳ��� ���϶�
								else if((jung_code_q == 5 && jung_code_d == 19) || ( jung_code_q == 19 && jung_code_d == 5)){	//5=��, 19=��
									query_same[i][found_match_no]=j;	//������ i��° ���ڿ� ��ġ�Ǵ� ������ ������ ��ġ�� �ִ´�.
									found_match_no++;

									//�ѹ迭 �ȿ� �ֱ�
									query_same_array[array_match_no] = j;
									query_same_array_isChanged[array_match_no] = 1;	//1�� true (��, ���� ���ڰ� �ƴ϶�, ������ �����̴�.)
									array_match_no++;
									//������ ���̶�� ���� ǥ��
									isChangedValue[j] = 1;
									//������ �� ����
									sprintf(ChangedValue[j], "%s", eo_string[i]);
								}
								//���� �ϳ��� �߼��� ���̰�, �ϳ��� ���϶�
								else if(jung_code_q == 3 && jung_code_d == 7 || ( jung_code_q == 7 && jung_code_d == 3)){ //3=��, 7=��
									query_same[i][found_match_no]=j;	//������ i��° ���ڿ� ��ġ�Ǵ� ������ ������ ��ġ�� �ִ´�.
									found_match_no++;

									//�ѹ迭 �ȿ� �ֱ�
									query_same_array[array_match_no] = j;
									query_same_array_isChanged[array_match_no] = 1;	//1�� true (��, ���� ���ڰ� �ƴ϶�, ������ �����̴�.)
									array_match_no++;
									//������ ���̶�� ���� ǥ��
									isChangedValue[j] = 1;
									//������ �� ����
									sprintf(ChangedValue[j], "%s", eo_string[i]);
								}
								//����ȭ �߰��κ�(������/�������� �����ϱ� ����)
								//�� �� ������ ���̰�,       //����ȭ�߰� Ȥ�� ������ ���̰�
								//���� �ϳ��� �߼��� ���̰�, �ϳ��� ���϶�
								//else if( (jong_code_q == 5 && jong_code_d == 5)|| (jong_code_q == 23 && jong_code_d == 23) ){ //5=��      ////����ȭ �߰� ���� ������ ���ΰ͵� ����
								else if( (jong_code_q == 5 && jong_code_d == 5)){ //5=��  
									if( ( jung_code_q == 7 && jung_code_d == 13 ) || ( jung_code_q == 13 && jung_code_d == 7) ){ //7=��, 13=��
										query_same[i][found_match_no]=j;	//������ i��° ���ڿ� ��ġ�Ǵ� ������ ������ ��ġ�� �ִ´�.
										found_match_no++;

										//�ѹ迭 �ȿ� �ֱ�
										query_same_array[array_match_no] = j;
										query_same_array_isChanged[array_match_no] = 1;	//1�� true (��, ���� ���ڰ� �ƴ϶�, ������ �����̴�.)
										array_match_no++;
										//������ ���̶�� ���� ǥ��
										isChangedValue[j] = 1;
										//������ �� ����
										sprintf(ChangedValue[j], "%s", eo_string[i]);
										
										//�����ܰ� ����ȭ
										estmLevel ++;//�� �������� ���� �������� ��Ÿ��. �����ܰ迡�� �����ϴµ� ���
									}
								}
							}
						}
					}
					else{
						//  (1-2) �ʼ� �ϳ��� ��(20)/�������� ��(19)�϶�,
						if(cho_code_q == 20 && cho_code_d == 19){		//�����ʼ� ��(20), �������ʼ� ��(19)
							if((jung_code_q == 22 && (jung_code_d == 4 || jung_code_d == 10))	//22=��, 4=�� 10=��
								|| (jung_code_q == 14 && jung_code_d == 3)						//14=��, 3=��
								|| (jung_code_q == 20 && jung_code_d == 27)						//20=��, 27=��
								|| (jung_code_q == 23 && jung_code_d == 29)						//23=��, 29=l
								){
									query_same[i][found_match_no]=j;	//������ i��° ���ڿ� ��ġ�Ǵ� ������ ������ ��ġ�� �ִ´�.
									found_match_no++;

									//�ѹ迭 �ȿ� �ֱ�
									query_same_array[array_match_no] = j;
									query_same_array_isChanged[array_match_no] = 1;	//1�� true (��, ���� ���ڰ� �ƴ϶�, ������ �����̴�.)
									array_match_no++;
									//������ ���̶�� ���� ǥ��
									isChangedValue[j] = 1;
									//������ �� ����
									sprintf(ChangedValue[j], "%s", eo_string[i]);
							}
						}else if(cho_code_q == 19 && cho_code_d == 20){	//�����ʼ� ��(19), �������ʼ� ��(20)
							if(((jung_code_q == 4 || jung_code_q == 10) && jung_code_d == 22)	//22=��, 4=�� 10=��
								|| (jung_code_q == 3 && jung_code_d == 14)						//14=��, 3=��
								|| (jung_code_q == 27 && jung_code_d == 20)						//20=��, 27=��
								|| (jung_code_q == 29 && jung_code_d == 23)						//23=��, 29=l
								){
									query_same[i][found_match_no]=j;	//������ i��° ���ڿ� ��ġ�Ǵ� ������ ������ ��ġ�� �ִ´�.
									found_match_no++;

									//�ѹ迭 �ȿ� �ֱ�
									query_same_array[array_match_no] = j;
									query_same_array_isChanged[array_match_no] = 1;	//1�� true (��, ���� ���ڰ� �ƴ϶�, ������ �����̴�.)
									array_match_no++;
									//������ ���̶�� ���� ǥ��
									isChangedValue[j] = 1;
									//������ �� ����
									sprintf(ChangedValue[j], "%s", eo_string[i]);
							}
						}
						
						//����ȭ �߰��κ�(������/�������� �����ϱ� ����)
						if((cho_code_q == 4 && cho_code_d == 7) || (cho_code_q == 7 && cho_code_d == 4)){//�ʼ� ��/�� or  ��/��
							if((jong_code_q == 23 && jong_code_d == 23)){ //������ �Ѵ� ��
								if( ( jung_code_q == 7 && jung_code_d == 13 ) || ( jung_code_q == 13 && jung_code_d == 7) ){// �߼� ��/�� or ��/��
									query_same[i][found_match_no]=j;	//������ i��° ���ڿ� ��ġ�Ǵ� ������ ������ ��ġ�� �ִ´�.
									found_match_no++;

									//�ѹ迭 �ȿ� �ֱ�
									query_same_array[array_match_no] = j;
									query_same_array_isChanged[array_match_no] = 1;	//1�� true (��, ���� ���ڰ� �ƴ϶�, ������ �����̴�.)
									array_match_no++;
									//������ ���̶�� ���� ǥ��
									isChangedValue[j] = 1;
									//������ �� ����
									sprintf(ChangedValue[j], "%s", eo_string[i]);
								}
							}
						}
					}
					cho_same = 0;	//�ʱ�ȭ
				}
			}	

			//�������� ������ ���ڸ� ���� ��
			if(j==(strCount-1)){
				if(found_match_no == 1){
					query_same[i][0] = 1;	//��ġ�Ǵ� ���ڰ� ���, '�ѹ迭'�� ������ �ڸ��� 1�� �����ϹǷ�, 1�� �Է��Ѵ�.
					query_same[i][1] = -1;
					//�ѹ迭 �ȿ� �ֱ�
					query_same_array[array_match_no] = -1;
					array_match_no++;
				}else{
					query_same[i][0]=found_match_no-1;	//query_same[i][0] ���� ������ i��° ���ڿ� ��ġ�Ǵ� ������ ������ ������ ����ȴ�.
				}
			}
		}
	}
	
	//�� �迭 �ȿ� ������ �ִ밪
	int max_array = array_match_no;
	if(max_array > 30){
		printf("\n\n�迭�� �ִ밪�� 30�̹Ƿ�, ���̻� ����� �� �����ϴ�.\n\n");
		return 0;
	}

	/////////// <<<< ���� ���� ���� >>>> ///////////
	int simple_method_score = 0;	// [����] ��� ������

	int pure_nf = 0;//������ ���� ���ڼ�


	//����ȭ�κ�
	//�Ʒ����ʹ� JAY�� �� �κ��� �Ϻ� �����԰� �� �帧�� ��������
	//���� ������
	
	//(1)���Ǿ�/
	int Qmatch =0;//���Ǿ� ���� �� �����Ϳ� �ִ� ���Ǿ� ������ ����
	int Qrest =0;//���Ǿ �ߺ����ڰ� ���� ��� ����
	//Q:�Ƿη��׸�    D1:�Ʒθ���ũ����(Qmatch=4) > D2:�Ƿηξ����(Qmatch=3)
	//���ڿ����絵������ �����ܰ� ������ ���ձ�� ���꿡�� �� �ߺ����� ���̵� ������� ���߱� ������ ���� ����
		//�����Ϳ� �ߺ����ڰ� ���� ���
		//Q:�Ƿ��׸�      D1:�Ƿηξ���� �� ��쿣 ���� �ҽ����¿����� ���� ����
	int Qestm = 0;//������ ���ڼ�
	
	//(2)�����ͱ���
	int Dlen =0;//�������� ����

	//(3)����
	int Qalone =0;//���ڰ���
	int isNotSeq =0;//���ڼ���

	//(4)���ں��
	int BLcount=0;//��ϰ���
	int BLorder=0;//��ϼ���
	int FstBL =0;//ù��Ͽ���

	
	for(i=0;i<eo_count; i++){
		if(query_same[i][1] == -1){
			pure_nf++;
		}
	}

	
	//����ȭ : �ϴ� �̰� ����� � ������ ������ ���� �ΰ� �ȿ� simple���κи� �ּ�ó��
	if((eo_count >4) && (pure_nf >= (eo_count+2))){//eo_count-3�̿��µ� ���� -> �̰Ͷ����� Q:�Ƿη��׸� D:�Ƿηξ���� �� �з���
		//simple_method_score +=2000;
	}else if((eo_count <=4) && (pure_nf >2 )){
		//simple_method_score +=2000;
	}else{
	

		//������ ����
		fprintf(fp, "\t");
		for(i=0;i<max_array;i++){
			fprintf(fp, "[%d] ",query_same_array[i]);
		}
		fprintf(fp, "\t");
	
		for(i=0;i<strCount;i++){
			if(isChangedValue[i]==1){
				fprintf(fp, "[%s] ", ChangedValue[i]);
			}else{
				fprintf(fp, "[%d] ", isChangedValue[i]);
			}
		}
	
		fprintf(fp, "\n");
		//


		//////////////////////////////////
		//(4)���ں��
		//1. ��ϱ��ϱ�
		int no_of_cur, no_of_next;
		int k;

		for(i=0; i<eo_count-1; i++){
			if(i==0){
				no_of_cur = query_same[0][0];
				no_of_next = no_of_cur + query_same[1][0];
			}else{
				no_of_cur = no_of_next;
				no_of_next = no_of_cur + query_same[i+1][0];
			}
		
			for(j=(no_of_cur-(query_same[i][0])+1); j<no_of_cur+1; j++){ //j 1~2
				for(k=(no_of_next-(query_same[i+1][0])+1); k<no_of_next+1; k++){ //k 3
					if(j==k){
					}else{
						if((query_same_array[k]-query_same_array[j])==1 && (query_same_array[k] != -1) && (query_same_array[j] != -1)){
							close_data[j]=k;
						}else{
						}
					}
				}
			}
		}

		//����, 120815
		int frag_start[10], frag_end[10];	//fragmentation �� ���� ���ڰ�
		int iii = 0;
		/////////////

		//����, ��ġ(block) ã�� �κ� (iii�� block�� ����)
		int m = 0;
		
		for(i=0; i<max_array+2; i++){ 
			if(close_data[i]>0){
				no_close = 0;
				do_caculate_close(close_data, i);
				close_list[++m] = no_close+1;		//close_list[0] : ���Ӹ�Ī ��ġ�� �� ����� �����Ѵ�.
													//          [1~5] : �ϳ��� ���Ӹ�Ī ��ġ�� �� ����ڰ� ���ӵǴ��� �����Ѵ�.
				
				//����, fragmentation���� �Է�
				frag_start[iii] = query_same_array[i];
				frag_end[iii] = query_same_array[i] + no_close;
				iii++;
			}
		}
		
		close_list[0] = m;
		
		


		//2. ��ġ�� ��� ����
		int mostbigblock =0;//���� ū ũ���� ���
		int tmp_close_list[MAX_BLOCK];

		int fn, fn2, fn3;//for���� int������
		
		//tmp_close_list�迭 �ʱ�ȭ [0]:��ϰ��� [1~]:�����ũ��
		for(fn =0; fn<MAX_BLOCK; fn++)
			tmp_close_list[fn]=0;
		for(fn =0; fn<m; fn++)
			tmp_close_list[fn+1]=close_list[fn+1];

		//tmp_strArray�迭 �ʱ�ȭ   
		int tmp_strArray[30];
		for(fn =0; fn<30; fn++)
			tmp_strArray[fn]=-1;

		//��ϰ� ��ħ�� Ȯ���ϱ� ���� ������
		int blstart =0;
		bool isdubble = false; 

		//������ �������
		int delBL[5];
		for(fn=0; fn<5; fn++)
			delBL[fn]=0;
		int index_delBL=0;


		int delblock =0;
		for(fn =0; fn<m; fn++){
			mostbigblock=0;

			//���� ū ũ���� ��Ϻ��� ���
			for(fn2 =0; fn2<m; fn2++){
				if(mostbigblock < tmp_close_list[fn2+1]){
					mostbigblock = tmp_close_list[fn2+1];
					delblock = fn2+1;
				}
			}
			
			//��Ĩ���� Ȯ��
			blstart= frag_start[delblock-1];
			isdubble = false; 
		
			for(fn2 =0; fn2<mostbigblock; fn2++){
				//����ϰ� �ִ� ����� ������ ����� ��ϰ� ��ħ ���� Ȯ��
				//�ѱ��ڶ� ��ġ�� isdubble�� true�� �Ǿ� ����������̵�
				//���� : ���� ū ù ����� ��쿡�� if�������� �ȵ�
				if(tmp_strArray[blstart]==1)
					isdubble = true;
				blstart++;
			}

			//��ħ���ο� ���� �����������
			blstart= frag_start[delblock-1];
			
			if(isdubble == true){//������� ����� ���
				//������ �������
				delBL[index_delBL] = delblock-1;
				index_delBL++;
			}
			else{//���� ��� ����� �ƴ� ���
				for(fn3 =0; fn3<mostbigblock; fn3++){
					tmp_strArray[blstart]=1;
					blstart++;
				}
			}

			//�۾����� ��� tmp_close_list���� ����
			tmp_close_list[delblock]=0;
		}


		//��� ����
		int delcount = index_delBL;
		
		if(delcount>0){//0�̻��̴� == ���� ����� �ִ�
			int tmp_frag_start[10];
			int tmp_frag_end[10];
			
			//�����ʱ�ȭ
			for(fn=0; fn<MAX_BLOCK; fn++){
				tmp_frag_start[fn]=0;
				tmp_frag_end[fn]=0;
				tmp_close_list[fn]=0;
			}
			index_delBL=0;

			//������� ��� ����ǥ���ϱ�
			for(fn=0;fn<delcount;fn++){
				frag_start[delBL[index_delBL]]=-1;
				frag_end[delBL[index_delBL]]=-1;
				close_list[delBL[index_delBL]+1]=-1;
				index_delBL++;
			}

			//tmp_�迭�鿡 ���� �����Ŵ
			//������ ��ϵ��� �迭 �߰��� ���� ��� �߰��� ��� �Ǵϱ� ��°����ֱ� ����
			int tmp_index=0;
			for(fn=0;fn<m;fn++){
				if(frag_start[fn]!=-1){
					tmp_frag_start[tmp_index]=frag_start[fn];
					tmp_frag_end[tmp_index]=frag_end[fn];
					tmp_close_list[tmp_index+1]=close_list[fn+1];
					tmp_index++;

					frag_start[fn]=0;
					frag_end[fn]=0;
					close_list[fn+1]=0;
				}
				else{
					frag_start[fn]=0;
					frag_end[fn]=0;
					close_list[fn+1]=0;
				}
			}

			//��� ��� ������ ��ϻ��� ����
			iii= iii-delcount;
			m = m-delcount;
			close_list[0] = m;

			for(fn=0;fn<m;fn++){
				frag_start[fn]=tmp_frag_start[fn];
				frag_end[fn]=tmp_frag_end[fn];
				close_list[fn+1]=tmp_close_list[fn+1];
			}

		}

		//��ϰ��� ���
		BLcount = close_list[0];



		//3. ��ϼ��� ���ϱ�
		if(close_list[0]>1){
			int bl_ct = close_list[0]-1;

			for(int ti=0; ti<bl_ct;ti++)
				if(frag_start[ti]>frag_start[ti+1])
					//������� �ȵ� ���, ��ϼ����� �ڹٲ���ִ� ���
					BLorder++;
		}

		//(4)���ں�� ��
		/////////////////////////////





		////////////////////
		//(3)���� 


		//�� ����� ������ ���ԵǾ����� Ȥ�� Not_Found���� �Ǵ� + ������ ���ڵ� ����ϴ� �κ�
		// ����, [���� : 1] NOT_FOUND + [���� : 2] ������ ���� ����
		int isMatching = -1;
		int start = 0;
		int isInQuery[MAX_QUERY_LEN];
		for(i=0; i<MAX_QUERY_LEN; i++){
			isInQuery[i] = 0;
		}
		int isAlreadyMatch[30];		//���ڵ� NOT_FOUND�� �߰��ϴ� ����		//30�� ������ ��. ������ ������ �ִ밳����ŭ �ʿ��ϴ�.
		for(i=0; i<30; i++){
			isAlreadyMatch[i] = 0;
		}
		int t;
		int changed_value = 0;
		int isSame = 0;
		int no_changed_value = 0;	//�����ϰ� �ִ� �������� ����
		int tmp_no_changed_value = 0;

		for(k=0; k<eo_count; k++){
			for(i=0; i<iii; i++){
				//����, block�� ù��° ���� ��
				//���� ���ڿ� ���� ��
				if( !strcmp((const char *)eo_string[k], (const char *)strArray[frag_start[i]]) ){
					start= k+1;
					isSame = 1;
				}
				//������ ���ڿ� ���� ��
				else if(isChangedValue[frag_start[i]] == 1){
					if( !strcmp((const char *)eo_string[k], (const char *)ChangedValue[frag_start[i]]) ){	
						start= k+1;
						isSame = 1;
						tmp_no_changed_value++;
					}
				}

				//����, block�� ������ ���� ��
				if(isSame == 1){
					for(j=frag_start[i]+1; j<frag_end[i]+1; j++){
						if(!strcmp((const char *)eo_string[start], (const char *)strArray[j]) && (isMatching!=0) ){
							isMatching = 1;
							start++;
						}else if(isChangedValue[j] == 1){
							if(!strcmp((const char *)eo_string[start], (const char *)ChangedValue[j]) && (isMatching!=0) ){
								isMatching = 1;
								start++;
								tmp_no_changed_value++;
							}else{
								isMatching = 0;
								tmp_no_changed_value = 0;
							}
						}else{
							isMatching = 0;
							tmp_no_changed_value = 0;
						}
					}
					if(isMatching==1){
						for(m=start-1; m>= ((start-1)-(frag_end[i]-frag_start[i])); m--){
							isInQuery[m] = 1;
						}
						for(m=frag_start[i]; m<frag_end[i]+1; m++){
							isAlreadyMatch[m] = 1;
						}
						isMatching=-1;
						if(tmp_no_changed_value > 0){
							no_changed_value += tmp_no_changed_value;
							tmp_no_changed_value = 0;
						}
					}else{
						isMatching = -1;
					}
				}
				isSame = 0;
			}
		}

		//���ڵ� NOT_FOUND�� �߰��ϴ� �κ�
		for(i=0; i<eo_count; i++){
			if(isInQuery[i]==0){
				for(j=0; j<strCount; j++){
					if(isAlreadyMatch[j]==0){
						if( !strcmp((const char *)eo_string[i], (const char *)strArray[j]) ){
							isInQuery[i] = 2;
							//����ȭ�߰�
							isAlreadyMatch[j]=2;
						}else if(isChangedValue[j] == 1){
							if(!strcmp((const char *)eo_string[i], (const char *)ChangedValue[j]) ){	
								isInQuery[i] = 2;
								no_changed_value++;
								//����ȭ�߰�
								isAlreadyMatch[j]=2;
							}
						}
					}
				}
			}
		}
		
		//������ ���� �� ���Ǿ �ִ� ������ ������ ���� ���ϱ�
		//�ߺ����� ���� �ذ��� ���� �߰� Q:�Ƿη��׸� D:�Ʒθ���ũ����
		for(int tj=0; tj<30;tj++){
			if(isAlreadyMatch[tj]!=0)
				Qrest++;
		}


		////////////////////////////////[������ġ Ȯ���ϴ� �κ�//120919]
		int front_isSequential = 0;
		int rear_isSequential = 0;
		int isNotSequential = eo_count;		//�����ͻ� �ִ� ���������� ��ġ�� ������� �ִ��� ���θ� Ȯ���Ͽ� > �� ���� ����
	
		int isSequential[MAX_QUERY_LEN];	//��ġ�� ������� ��ġ�Ǿ����� ���� Ȯ�ο�.
		for(i=0; i<MAX_QUERY_LEN; i++){
			isSequential[i] = 0;
		}
	
		for(i=0; i<eo_count; i++){
			if(isInQuery[i] == 2){	//���ڱ����϶��� ��ġ Ȯ����
				for(j=1; j<=query_same[i][0]; j++){
					//�ձ��ڿ� ��
					if(i==0){	//ù������ ���, ���������ڸ� ����.
						if(front_isSequential==0){
							if(query_same[eo_count-1][1] == -1){			//������ ���ڰ� �����Ϳ� ���� ������ ��
								for(k=1; k<=query_same[eo_count-2][0]; k++){
									if(query_same[i][j]<query_same[eo_count-2][k]){
										front_isSequential = 1;
										break;
									}
								}
							}else{
								for(k=1; k<=query_same[eo_count-1][0]; k++){
									if(query_same[i][j]<query_same[eo_count-1][k]){
										front_isSequential = 1;
										break;
									}
								}
							}
						}
					}else{
						if(front_isSequential==0){
							if((query_same[i-1][1] == -1) && (i>1)){			//���� ���ڰ� �����Ϳ� ���� ������ ��
								for(k=1; k<=query_same[i-2][0]; k++){
									if(query_same[i][j]>query_same[i-2][k]){
										front_isSequential = 1;
										break;
									}
								}
							}else{
								for(k=1; k<=query_same[i-1][0]; k++){
									if(query_same[i][j]>query_same[i-1][k]){
										front_isSequential = 1;
										break;
									}
								}
							}
						}
					}
				
					//�ޱ��ڿ� ��
					if(i==eo_count-1){//������������ ���, ù���ڿ� ���Ѵ�.
						if(rear_isSequential==0){
							if(query_same[0][1] == -1){	//ù���ڰ� �����Ϳ� ���� ������ ��
								for(k=1; k<=query_same[1][0]; k++){
									if(query_same[i][j]>query_same[1][k]){
										rear_isSequential = 1;
										break;
									}
								}
							}else{
								for(k=1; k<=query_same[0][0]; k++){
									if(query_same[i][j]>query_same[0][k]){
										rear_isSequential = 1;
										break;
									}
								}
							}
						}
					}else{
						if(rear_isSequential==0){
							if((query_same[i+1][1] == -1) && (i<eo_count-2)){	//���� ���ڰ� �����Ϳ� ���� ������ ��
								for(k=1; k<=query_same[i+2][0]; k++){
									if(query_same[i][j]<query_same[i+2][k]){
										rear_isSequential = 1;
										break;
									}
								}
							}else{
								for(k=1; k<=query_same[i+1][0]; k++){
									if(query_same[i][j]<query_same[i+1][k]){
										rear_isSequential = 1;
										break;
									}
								}
							}
						}
					}
				}
				if((front_isSequential==1) && (rear_isSequential==1)){
					isNotSequential--;
					isSequential[i] = 1;
				}else{
				}
				front_isSequential = 0;
				rear_isSequential = 0;
			}else if(isInQuery[i]==1){	//block�� ��� 
				isNotSequential--;
				isSequential[i] = 1;
			}else if(isInQuery[i]==0){	//���ڰ� ���� ���
				isNotSequential--;
			}
		}
		fprintf(fp, "\t[����Ʋ����]%d", isNotSequential);
		fprintf(fp, "\n");
		for(i=0; i<eo_count; i++){
			fprintf(fp, "[%d]", isSequential[i]);
		}
		//������ ����//////////////////////////////////
		fprintf(fp, "\n");
		for(i=0; i<iii; i++){
			fprintf(fp, "[%d->%d]", frag_start[i], frag_end[i]);
		}
		fprintf(fp, "\n");
		for(i=0; i<eo_count; i++){
			fprintf(fp, "[%d]", isInQuery[i]);
		}
		fprintf(fp, "\n");
		for(i=0; i<strCount; i++){
			fprintf(fp, "[%d]", isAlreadyMatch[i]);
		}
		fprintf(fp, "\n");



		isNotSeq = isNotSequential;
		//(3) ���� ��
		/////////////////////////////



		///////////////////////
		//(4)���ں��
		//FstBL ù��Ͽ���Ȯ��
		int isInBlock = 0;
		for(i=0; i<iii; i++){//������ ù���� Ȯ��
			if(frag_start[i]==0){
				isInBlock = 1;
			}
		}
		if(isInQuery[0] == 1){//���Ǿ� ù���� Ȯ��
			if(isInBlock == 1){
				isInBlock = 2;	//����/������ ù���� ��� block�� ����
			}else{
				isInBlock = 1;	//����or������ �� �ϳ��� ù���ڰ� block�� ����
			}
		}

		//���� ù���� Ȯ�� �߰�???
		//isAlready�� Ȯ���� isInQuery���� �ٽ� Ȯ��
		//isInAlone�߰�, Fstalone�߰�

		if(isInBlock == 1){
			FstBL = 25;
		}else if(isInBlock ==2){
			FstBL = 50;
		}
		//(4)���ں�� ��
		////////////////////////


		/////////////////////
		//(1)���Ǿ�
		
		for(i=0; i<strCount; i++){
			if(isInQuery[i]==1)
				Qmatch++;
			else if(isInQuery[i]==2){
				Qmatch++;
				Qalone++;
			}else{}
		}

		if(Qmatch>Qrest)
			Qmatch=Qrest;
		//Qmatch<Qrest�� ���� ������ Qmatch���� �̿��ϴ°Ŵϱ� ������� �ʾƵ���
		
		Qestm = no_changed_value+estmLevel;

		//(1)���Ǿ� ��
		//////////////////////

		
		//////////////////////
		//(2)�����ͱ���
		if(strCount>eo_count)
			Dlen = strCount-eo_count;
		else
			Dlen = eo_count-strCount;
		//(2)�����ͱ��̳�
		//////////////////////
		
	}


		//�������� ��� //������ ����
		
		simple_method_score= 0;
		
		/* 10��4��
		simple_method_score = simple_method_score - Qmatch*2*10000/eo_count + (2*10000/eo_count)*(Qestm/eo_count) 
			+ 1000*Dlen/eo_count
			+ Qalone*200 + BLcount*100 +BLorder*100  +isNotSeq*2000 
			-FstBL*100;//*/
		///*10�� 2��
		simple_method_score = simple_method_score - Qmatch*2*100/eo_count + (2*100/eo_count)*(Qestm/eo_count) 
			+ 10*Dlen/eo_count
			+ Qalone*2 + BLcount +BLorder  +isNotSeq*20 
			-FstBL;//*/

		//����������
		/*(1) ���Ǿ�
		simple_method_score = simple_method_score - Qmatch*2*100/eo_count + (2*100/eo_count)*(Qestm/eo_count);//*/

		/*(2) �����ͱ���
		simple_method_score = -200;
		simple_method_score = simple_method_score + 10*Dlen/eo_count;//*/

		/*(3) ����
		simple_method_score = -200;
		simple_method_score = simple_method_score + Qalone*2 +isNotSeq*20;//*/

		/*(4) ���
		simple_method_score = -200;
		simple_method_score = simple_method_score + BLcount +BLorder-FstBL;//*/

	
		fprintf(fp, "\n>> %d\n",simple_method_score);

		return simple_method_score;

}

/*
 * ������ ���ڿ��� �����ġ������ �󸶳� ��Ī�Ǵ��� ����ϴ� �Լ�
 * �Է� do_close_data[]: ���ͽ�Ʈ�� �˰����� �迭�� �̿��Ͽ� 1024�� ���� ����(���Ӹ�Ī�Ǵ�) �迭���� �����س� �迭
*/
void do_caculate_close(int do_close_data[], int i){
	if(do_close_data[i] > 0){
		//printf("%d, ", do_close_data[i]);
		no_close++;
		do_caculate_close(do_close_data, do_close_data[i]);
		close_data[i] = 0;
	}
}

/*
 * ���� ���ڿ��� �˻��� ���ڿ� ���� ���絵�� ����Ѵ�. 
 * ���� ��: ����. 0: ��ġ, Ŭ ���� ����ġ���� ���� ����.
 * �Է� str: �˻��� ���ڿ�, query_Array: ���� ���ڿ�(������ �ϼ��� ���ڿ� �ʼ� ���ڰ� ���� ���� �� ����)
 *      query_count: ������ ���� ����
 */
int
CalMixedSimilarity(const unsigned char query_Array[MAX_QUERY_LEN][HANGUL_BYTE+1], int query_count, const unsigned char *str)
{

	int qi = 0, ri = 0, strCount;
	unsigned char strArray[MAX_QUERY_LEN][HANGUL_BYTE+1];
	char han[HANGUL_BYTE+1];			// �ѱ� ó�� �� ���� �ϳ��� ��� ���� ���� ����. NULL ���ڱ��� �����ϱ� ���� +1.
	char c_han[HANGUL_BYTE+1];			// �ϼ��� ���ڸ� ���������� �ٲ� �� ����ϱ� ���� ����.
	unsigned short johapcode;
	int minus_point = 0;
	int in_one_eo, fPartiallyMatched;
	int keep_matching = 0;
	int isSame, fBothWanCode;
	unsigned char cho_code;

	strCount = StrToStrArray(str, strArray);	// �˻��� ��� ���ڿ����״� �̸� ���� ������ �и��Ѵ�.
	minus_point = (query_count > strCount) ? (query_count - strCount) : (strCount - query_count);	// ���� ���̸�ŭ �����Ѵ�.
	in_one_eo = 0;
	fPartiallyMatched = 0;

	while (1) {
		if (query_Array[qi][2] == '\1') {	// ������ �ڵ�� �ʼ��� �ִ� ����
			if ((unsigned short)strArray[ri][0] >= 128) {	// �ѱ��̸�
				han[0] = strArray[ri][0]; 
				han[1] = strArray[ri][1];
				han[2] = '\0';
				HangulCodeConvert(KS, han, c_han);
				johapcode = ((unsigned char)c_han[0] << 8) | ((unsigned char)c_han[1]); 
				cho_code = (johapcode >> 10) & 0x1f;		// �ʼ��� �����Ѵ�.
				isSame = query_Array[qi][0] == cho_code;
			}
			else // �������� �ʼ� �ڵ�� ���ϴ� �˻� ��� ���ڰ� �ѱ��� �ƴ�. ������ �ٸ� ����.
				isSame = 0;
			fBothWanCode = 0;
		}
		else {								// (query_Array[qi][2] == '\0'), �� ������ �ϼ��� �ڵ��
			isSame = !strcmp((const char *)query_Array[qi], (const char *)strArray[ri]);
			fBothWanCode = 1;	// ������ ������ڿ� �� �ִ� �� ���ڰ� �� �� �ϼ��� �ڵ���.
		}

		if (isSame) {			// ���� �����̸�
			qi++; ri++;			// ���� ���� �� �� �����Ѵ�.
			if (keep_matching)	// �����ؼ� ��Ī�� ���ڰ� �����ϰ� ������ ����Ѵ�.
				keep_matching *= 2;
			else
				keep_matching = 1;	// '���� ��Ī ���θ� ��Ÿ���� �÷���'�� on.
			minus_point -= keep_matching;
			in_one_eo = 0;	// 'query_Array[qi] ���ڰ� strArray[ri] ������ ���� �߰� �� ��'�� �ǹ��ϴ� �÷��׸� ������.
			fPartiallyMatched = 0;
		}
		else {	// �ٸ� ���ڸ� �����ϰ�, ��� �ε����� �����Ѵ�.
			keep_matching = 0;	// '���� ��Ī ���θ� ��Ÿ���� �÷���'�� off.
			minus_point += 6;
			if (fBothWanCode && is_partially_matched((const char *)query_Array[qi], (const char *)strArray[ri])) {
				fPartiallyMatched = 1;
				minus_point -= 6;
			}
			ri++;
			if (++in_one_eo >= strCount) {	// query_Array[qi] ���ڰ� �ᱹ strArray[ri] ������ �߰� �� ��.
				if (!fPartiallyMatched)
					minus_point += 20;
				qi++;
				in_one_eo = 0;
				fPartiallyMatched = 0;
			}
		}
		if (ri >= strCount) { 	// ��� �ε����� �����ߴµ� �����ڸ� �Ѿ���� wrap around �Ѵ�.
			ri = 0;
			keep_matching = 0;	// wrap around �Ǿ����ϱ� ���� ��Ī�� ���� ����
		}
		if (qi >= query_count)	// ���� �ε����� �����ߴµ� ���� �����ڸ� �Ѿ���� �����Ѵ�.
			break;
	}

	return minus_point;

}

/*
 * ���� ���ڿ����� ���� ������ �и��Ͽ� �����Ѵ�. �ϼ��� �˻������� �����.
 * �ʼ��� ���Ե� ���ڿ��� ���ؼ��� ������� �� ��! ���� �� ��.
 */
int
StrToStrArray(const unsigned char *str, unsigned char strArray[MAX_QUERY_LEN][HANGUL_BYTE+1])
{
	int len, cur, str_count;
	unsigned char t_char;

	len = strlen((const char *)str);
	str_count = 0;
	cur = 0;

	for (;;) {
		t_char = str[cur++];
		len--;
		if ((unsigned int)t_char >= 128) {	// �ѱ�
			strArray[str_count][0] = t_char;
				
			t_char = str[cur++];
			len--;

			strArray[str_count][1] = t_char;
			strArray[str_count++][2] = '\0';
		}
		else { // ��ũ�� ������ �Ϲ� �ƽ�Ű ����
			if (isalpha(t_char))
				t_char = toupper(t_char);
			strArray[str_count][0] = t_char;
			strArray[str_count][1] = '\0';
			strArray[str_count++][2] = '\0';
		}
		if (len <= 0 || str_count >= MAX_QUERY_LEN)
			break;
	} // end of for (;;)

	return str_count;
}

/*
 * ���� ���ڿ��� ���� ������ �и��Ͽ� �����Ѵ�.
 * �ϼ����� �ʼ��� ���ԵǾ� �ִ� ���� ���ڿ��̾ �۵���.
 * ���� ���ڿ��� ���ؼ��� ȣ���ؾ� ��. ������ �ʼ��� ���ԵǾ� ������
 * ������ �ʼ� �ڵ�� ��ȯ�Ͽ� ������.
 * �Է�: str: ���� ���ڿ� ����. ���: strArray: �� ������ ������ ��´�.
 */
int
StrToEoArray(const unsigned char *str, unsigned char strArray[MAX_QUERY_LEN][HANGUL_BYTE+1])
{
	int i, j, str_count;
	union code {
		unsigned short fullcode;
		unsigned char halfcode[2];
	} code;
	char han[HANGUL_BYTE+1];			// �ѱ� ó�� �� ���� �ϳ��� ��� ���� ���� ����. NULL ���ڱ��� �����ϱ� ���� +1.
	char c_han[HANGUL_BYTE+1];			// �ϼ��� ���ڸ� ���������� �ٲ� �� ����ϱ� ���� ����.
	unsigned short johapcode;

	str_count = 0;

	for (i = 0; str[i] != '\0'; i++) {
		if ((unsigned int)str[i] >= 128) {		// �ѱ�
			code.halfcode[1] = str[i];
			code.halfcode[0] = str[i+1];
			for (j = 0; j < 19; j++) {		// ���� �ڵ尡 �ϼ��� �ʼ� �ڵ忡 �ִ� ������ �˻�
				if ((unsigned int)code.fullcode == Johap[j])
					break;
			}
			if (j < 19)	{	// �߰��� ���� ���� ���̹Ƿ� �ϼ����ڵ�� �ʼ��� �Էµ� ����. ���������� �ٲ㼭 strArray�� �ִ´�.
				han[0] = str[i];
				han[1] = str[i+1];
				han[2] = '\0';
				HangulCodeConvert(KS, han, c_han);
				johapcode = ((unsigned char)c_han[0] << 8) | ((unsigned char)c_han[1]); 
				strArray[str_count][0] = (johapcode >> 10) & 0x1f;	// �ʼ��� �����Ѵ�.
				strArray[str_count][1] = '\0';						// filler
				strArray[str_count++][2] = '\1';					// ������ �ʼ� �ڵ��� ǥ��
			}
			else {			// �ϼ��� �ʼ� �ڵ尡 �ƴϹǷ� ������ �ϼ��� ������(��� ���� ������ ���ɼ��� 0%. �̹� �ɷ��� ����)
				strArray[str_count][0] = code.halfcode[1];
				strArray[str_count][1] = code.halfcode[0];
				strArray[str_count++][2] = '\0';
			}
			i++;	// �ѱ��̴ϱ� �� ����Ʈ �� ����
		}
		else {	// �Ϲ� �ƽ�Ű �ڵ� ����
			if (isalpha(str[i]))	// ���ĺ��̸� �빮�ڷ� �ٲ� �ְ�
				strArray[str_count][0] = toupper(str[i]);
			else					// �ƴϸ� �״�� �ִ´�.
				strArray[str_count][0] = str[i];
			strArray[str_count][1] = '\0';		// filler
			strArray[str_count++][2] = '\0';	// ������ �ʼ��� �ƴ϶�� ǥ��
		}
	} // end of for (i = 0; query_str[i] != '\0'; i++)

	return str_count;
}

// �Է�: ���� ���ڿ�
// ���: -1: ��� �ϼ��� ���ڸ� �ִ� ���
//        1: ��� �ʼ� ���ڸ� �ִ� ���
//		  0: �ϼ����� �ʼ��� �����µ� �ϼ��� ���ڰ� �� ���� �̻��̾ �ϼ��� �˻��� ���� �� �� �ʼ� �˻��� �϶�.
//		  2: �ϼ����� �ʼ��� �����µ� �ϼ��� ���ڰ� �� ���� �̸��̾ �ʼ� �˻��� ���� �� �� �ϼ��� �˻��� �϶�.
//		 ������ ���� �ϼ��� ���ڰ� ���� �� ���Ե� ��� �� ó�� ���ڸ� �����ϰ� ��� �ʼ����� �ٲ��.
int
CheckIfChosungIncluded(char *query_str)
{
	int wansung_code_count = 0;			// ������ �ϼ��� ���� ����
	int chosung_code_count = 0;			// ������ �ʼ� ���� ����
	union code {
		unsigned short fullcode;
		unsigned char halfcode[2];
	} code;
	register int i, j, k;
	int t_int, t_int_prev;
	Node *result;
	char han[HANGUL_BYTE+1];			// �ѱ� ó�� �� ���� �ϳ��� ��� ���� ���� ����. NULL ���ڱ��� �����ϱ� ���� +1.
	char c_han[HANGUL_BYTE+1];			// �ϼ��� ���ڸ� ���������� �ٲ� �� ����ϱ� ���� ����.
	unsigned short johapcode;
	unsigned short cho_code;

	for (i = 0; query_str[i] != '\0'; i++) {
		if ((unsigned int) query_str[i] >= 128) {
			code.halfcode[1] = query_str[i];
			code.halfcode[0] = query_str[i+1];
			//printf("0x%x ", code.fullcode);
			for (j = 0; j < 19; j++) {		// ���� �ڵ尡 �ϼ��� �ʼ� �ڵ忡 �ִ� ������ �˻�
				if ((unsigned int)code.fullcode == Johap[j])
					break;
			}
			if (j < 19)	// �߰��� ���� ���� ���̹Ƿ� �ʼ��� �Էµ� ����.
				chosung_code_count++;
			else {		// �ʼ� �ڵ忡 �������Ƿ� �ϼ��� ���ڰ� �ԷµǾ��ų� ��� ���� ���ڰ� �Էµ� ����.
				t_int = (int)(*(unsigned short*)&query_str[i]);
				if (result = rbt2.search(t_int)) {	// ������ �ϼ��� ������

					//�ߺ����ڹ��� �ذ�κ�
					//////////////////////////////////JAY
					// ������ ���⼭
					// wansung_code_count++;
					// �ϰ� ���´µ�, ������ �׷��� �� �ǰ�,
					// �ڽ��ϰ� ���� ���ڰ� �� �κп� �ִ��� Ȯ���ϰ� ������ �ڽ��� �ʼ� �ڵ�� �ٲ۴�.
					/*
					for (k = 0; k < i; k++) {	// ������ �� ������ �պκп� �ڱ�� ���� ���ڰ� �ִ��� �˻��Ѵ�.
						if ((unsigned int)query_str[k] >= 128) {
							t_int_prev = (int)(*(unsigned short*)&query_str[k]);
							if (t_int == t_int_prev) {	// ���� �����̸� ���� ���ڸ� �ʼ����� �ٲ۴�.
								han[0] = query_str[i];
								han[1] = query_str[i+1];
								han[2] = '\0';
								HangulCodeConvert(KS, han, c_han);
								johapcode = ((unsigned char)c_han[0] << 8) | ((unsigned char)c_han[1]); 
								cho_code = (johapcode >> 10) & 0x1f;	// �ʼ� ����
								if (cho_code < 0x02 || cho_code > 0x14) {
									fprintf(stderr, "CheckIfChosungIncluded(): �������� ������ �ʼ� ���� '%s' �߰�! �����մϴ�.\n", han);
									k++;
									continue;
								}
								//(*(unsigned short*)&query_str[i]) = (unsigned short)Johap[cho_code-2];	// �ʼ������� �̷���� �ϼ��� �ڵ带 ��� ������ ��ü�Ѵ�.
								query_str[i] = (unsigned char)(Johap[cho_code-2] >> 8);	// �ʼ������� �̷���� �ϼ��� �ڵ带 ��� ������ ��ü�Ѵ�.
								query_str[i+1] = (unsigned char)(Johap[cho_code-2] & 0x00ff);	// �ʼ������� �̷���� �ϼ��� �ڵ带 ��� ������ ��ü�Ѵ�.
								break;
							}
							k++;
						}
					}
					if (k >= i)	// ���� ���ڰ� ������ �ϼ��� ���ڼ��� �ϳ� ������Ų��.
						wansung_code_count++;
					else		// �ϼ��� ���ڰ� �ߺ����� ���� �ʼ� �ϼ��� �ڵ�� �ٲ������ �ʼ� ���ڼ� �ϳ��� ������Ų��.
						chosung_code_count++;
					*/
					//////////////////////////////////JAY

					wansung_code_count++;	//���� ���ڰ� ���������ö�, CalMixedSimilarity�� �ǹǷ�, JAY�� �Ҷ� �׳� �ϼ������� ó���Ѵ�.

				}
			}
			i++;	// �ѱ��̴ϱ� �� ����Ʈ �� ����
		}
		else
			wansung_code_count++;
	} // end of for (i = 0; query_str[i] != '\0'; i++)
	//printf("\n");
	if (chosung_code_count == 0)	// ��� �ϼ��� ���ڸ� ���� ���
		return -1;
	if (wansung_code_count == 0)	// ��� �ʼ� ���ڸ� ���� ���
		return 1;
	// ������� ����, ������ ���̰ų�(�̰� ��ܷ� ��) ������ �ϼ���, �ʼ��� ���� ����. 
	if (wansung_code_count >= 2)	// �����µ� �ϼ��� ���ڰ� 2�� �̻��̸� �ϼ��� �˻� �� �� ����� �ʼ� �˻� ����� ��ģ �ٹ�ȣ�鿡 ��� ���ھ�� �϶�.
		return 0;
	else							// �����µ� �ϼ��� ���ڰ� 2�� �̸��̸� �ʼ� �˻� �� �� ����� ��� ���ھ�� �϶�.
		return 2;

	return 3;
}

// �Է�: qs: ������ �ִ� � ����(�ϼ���), rs:�˻� ����� ���� � �ٿ� �ִ� �� ����(�ϼ���)
// �����: �� ���ڰ� ������ ������ �ʼ�, �߼�, ���� �� �� �� �̻��� ������ 1�� �����Ѵ�.
//         �ʼ��� �߼��� �ִ� ���� �߼�(����)�� ������ 1�� �����Ѵ�.
int is_partially_matched(const char *qs, const char *rs)
{
	char c_han[HANGUL_BYTE+1];			// �ϼ��� ���ڸ� ���������� �ٲ� �� ����ϱ� ���� ����.
	unsigned short johapcode;
	unsigned short qs_cho, qs_moeum, qs_jong;
	unsigned short rs_cho, rs_moeum, rs_jong;
	int point = 0;

	if ((unsigned short)qs[0] < 128 || (unsigned short)rs[0] < 128)		// �� �� �ϳ��� �ѱ��� �ƴϸ� 0�� ����.
		return 0;

	// qs���� �ʼ�, �߼�, ������ �����Ѵ�.
	HangulCodeConvert(KS, qs, c_han);
	johapcode = ((unsigned char)c_han[0] << 8) | ((unsigned char)c_han[1]); 
	qs_cho = (johapcode >> 10) & 0x001f;	// �ʼ��� �����Ѵ�.
	qs_moeum = (johapcode >> 5) & 0x001f;	// �߼��� �����Ѵ�.
	qs_jong = johapcode & 0x001f;			// �߼��� �����Ѵ�.

	// rs���� �ʼ�, �߼�, ������ �����Ѵ�.
	HangulCodeConvert(KS, rs, c_han);
	johapcode = ((unsigned char)c_han[0] << 8) | ((unsigned char)c_han[1]); 
	rs_cho = (johapcode >> 10) & 0x001f;	// �ʼ��� �����Ѵ�.
	rs_moeum = (johapcode >> 5) & 0x001f;	// �߼��� �����Ѵ�.
	rs_jong = johapcode & 0x001f;			// �߼��� �����Ѵ�.

	if (qs_cho && rs_cho && qs_cho == rs_cho) point++;
	if (qs_moeum && rs_moeum && qs_moeum == rs_moeum) point++;
	if (qs_jong && rs_jong && qs_jong == rs_jong) point++;
	if (point == 1) {	// ������ �� �ϳ��� ����.
		if (!(qs_jong || rs_jong))	// ������ �� �� ���� ���ڿ��ٸ�  partially matched!
			return 1;
		else
			return 0;
	}
	else if (point == 2)	// ������ �� �� ���� ���ٸ� partially matched!
		return 1;
	else					// ������ �� ���� ���� ����.
		return 0;
}

void
init_variables_4text(void)
{
	for (int i = 0; i < MAX_QUERY_LEN; i++) {
		for (unsigned int j = 0; j < MAX_SEARCHED_COL; j++) {
			final[i][j] = 0;
			searched[i][j] = 0;
		}
		final_count[i] = 0;
		searched_count[i] = 0;
		eo_index[i] = 0;
	}
}

void
init_variables_4chosung(void)
{
	int i;

	for(i = 0; i < MAX_QUERY_LEN/2; i++) {
		cho_final_count[i] = 0;
		for(unsigned int j = 0; j < MIN_FORMER_LATER_CNT; j++) {
			cho_final[i][j] = 0;
		}
	}
	memset(former_list, 0, sizeof(int)*MAX_FORMER_LATER_CNT);
	memset(latter_list, 0, sizeof(int)*MAX_FORMER_LATER_CNT);
	cho_max_final = 0;
}

void
cleanup(void)
{
	// �ϼ��� �˻��� ���� �۷ι� �޸� �������� ��ȯ�Ѵ�.
	free(position_in_memory);
	free(namepart_len);
	free(addrpart_len);
	free(memory_db);

	free(i_count);
	free(i_start);
	free(inverted);
	for (int i = 0; i < MAX_QUERY_LEN; i++) {
		free(searched[i]);
		free(final[i]);
	}
	free(searched);
	free(final);

	free(first_final_degree);
	free(second_final_degree);
	free(merged_final_degree);
	free(third_final_degree);
	free(merged2_final_degree);

	// �ʼ� �˻��� ���� �۷ι� �޸� �������� ��ȯ�Ѵ�.
	free(cho3_inverted);
	free(cho2_inverted);
	free(former_list);
	free(latter_list);
	for (int i = 0; i < MAX_QUERY_LEN/2; i++) {
		free(cho_final[i]);
	}
	free(cho_final);
}
