#define	MAX_MOEUM_CNT		60					// �� �ٿ� ������ �� �ִ� ���� ����. ��� �� ������ �� ���� �ִ� ���̷� �ؾ� ��.
#define	MAX_JAEUM_CNT		60					// �� �ٿ� ������ �� �ִ� �ʼ� ����. ��� �� ������ �� ���� �ִ� ���̷� �ؾ� ��.
#define	MAX_3MOEUM_INDEX	(38*38*38)			// 3���Ӹ����鿡 �ο��� �� �ִ� ID(index)�� �ִ� ����
#define	MAX_2MOEUM_INDEX	(38*38)				// 2���Ӹ����鿡 �ο��� �� �ִ� ID(index)�� �ִ� ����
#define	HANGUL_BYTE			2					// �ѱ� �ڵ��� ����Ʈ ��
#define	MAX_QUERY_LEN		30					// �Էµ� �� �ִ� ������ �ִ� ����. �ѱ��� 2����Ʈ�̹Ƿ� �ѱ۸� ������ NULL���� ����ϸ� 24�ڱ��� ������
#define	MAX_THESAURUS_CNT	18					// ���� ���÷� �־� ���� �üҷ��� ���� ��.
#define	MAX_EACH_THESAURUS	3					// �üҷ��� �� ��ü �� �ִ� �� ���� �����ǹ� ���ڿ��� �� �� �ִ°�
//#define	TROUBLE_COUNT		30000				// ���� �ٹ�ȣ ���� �̰� ������ �˻� �� �ð��� ��� ���� �� ������ �׳� �Ѿ��.
#define	MAX_OUT_LINES		20					// ���ھ �� ����� �ִ� ����
#define	MAX_MINUS_POINT		90

//�ߺ����� ���� �ذ�κ�
#define MAX_OVERLAP    1500   //��ü���� ��       ���ڵ峻 �ߺ��� �ִ� ������ �ִ� ����
#define MAX_ONELINE    100    //���ڵ峻 ���� ��  ���ڵ峻 �ߺ��� �ִ� ������ �ִ� ����
//�ߺ����� ���� �ذ�κг�
//����ȭ
#define MAX_BLOCK      10     //��� �ִ� ����
//����ȭ��



extern unsigned int MAX_DB_LINE;
extern char *memory_db;							// ����� ��� ������ �о���� �޸�. ��� ������ ũ�⸦ ���� �������� �Ҵ��ϴ� ������� �ٲ� ����.
extern unsigned int *position_in_memory;		// ����� �� ���� memory[] �迭 ������ ��𿡼� �����ϴ��� �� ��ġ�� ���� �迭. ��� ������ ���� ���� ��� �������� �Ҵ��ϴ� ����� �ٲ� ����.
extern unsigned char *namepart_len;				// ��� �� ������ ��Ī�κ� ���ڿ� ����
extern unsigned char *addrpart_len;				// ��� �� ������ �ּҺκ� ���ڿ� ����
extern unsigned int db_line_count;				// ����� ��ü ���� ���� �����.

extern int **searched;
extern unsigned char eo_string[MAX_QUERY_LEN][HANGUL_BYTE+1];
extern int **final;
extern int max_final;
extern int *mfd;
extern int eo_count;

extern unsigned int cho3_count[MAX_3MOEUM_INDEX];	// 3�����ʼ��� ������ ����. �ε����� 3�����ʼ��� ���� �� �ִ� ��� ����� ���� 
extern unsigned int cho2_count[MAX_2MOEUM_INDEX];	// 2�����ʼ��� ������ ����. �ε����� 2�����ʼ��� ���� �� �ִ� ��� ����� ���� 
extern unsigned int cho3_start[MAX_3MOEUM_INDEX];	// cho3_inverted[]���� �� Ʈ������ �����ϴ� �ε������� �����ϴ� �迭
extern unsigned int cho2_start[MAX_2MOEUM_INDEX];	// cho2_inverted[]���� �� Ʃ���� �����ϴ� �ε������� �����ϴ� �迭
extern unsigned char *cho3_inverted;				// ������ �迭. �� �ٿ� ������ ��� 3�����ʼ��� 
extern unsigned char *cho2_inverted;				// 2�����ʼ����� �ٹ�ȣ�� ��ϵȴ�.
extern unsigned int MAX_INVFILE3_LEN;				// 3�����ʼ� ������ �迭�� ����Ʈ ��.
extern unsigned int MAX_INVFILE2_LEN;				// 2�����ʼ� ������ �迭�� ����Ʈ ��.
extern int **cho_final;								// 
extern int cho_max_final;

void load_db_in_memory(void);
void make_rbtree(void);
void create_inverted_file_4text(void);
void create_inverted_file_4chosung(void);
void extract_chosung_from_query(const char *in, short *in_cho, int *in_alpha, int *qc, int *qac);
void search_using_inverted_file_4chosung(short *in_eo, int in_count, int *in_alpha, int in_alpha_count);
//void scoring_and_output_4chosung(short *query_cho, int query_count, int *in_alpha, int in_alpha_count);

extern void init_variables_4text(void);
void init_variables_4chosung(void);
void cleanup(void);
int search_inverted_file_4text(const char *query_str);
void scoring_and_output_4text(int fFullyIncluded);
int CalSimilarity(const unsigned char *str, int no_str);
//int CalSimilarity(const unsigned char *str);
int CalMixedSimilarity(const unsigned char query_Array[MAX_QUERY_LEN][HANGUL_BYTE+1], int query_count, const unsigned char *str);
int StrToStrArray(const unsigned char *str, unsigned char strArray[MAX_QUERY_LEN][HANGUL_BYTE+1]);
int StrToEoArray(const unsigned char *str, unsigned char strArray[MAX_QUERY_LEN][HANGUL_BYTE+1]);
int CheckIfChosungIncluded(char *query_str);

void do_caculate_close(int do_close_data[], int i);

void integrated_scoring_and_output(int fFullyIncluded, const char *in_query, int *linenumbers, int line_count);
int is_partially_matched(const char *qs, const char *rs);

//�ߺ����� ���� �ذ�κ�(quick sort)
void q_sort(int numbers_t_int[],int numbers_count[], int left, int right); 
//�ߺ����� ���� �ذ�κ�(quick sort)��

extern void scoring_and_output_4text(int fFullyIncluded);
