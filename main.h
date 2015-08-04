#define	MAX_MOEUM_CNT		60					// 한 줄에 등장할 수 있는 모음 개수. 디비 내 임의의 한 줄의 최대 길이로 해야 함.
#define	MAX_JAEUM_CNT		60					// 한 줄에 등장할 수 있는 초성 개수. 디비 내 임의의 한 줄의 최대 길이로 해야 함.
#define	MAX_3MOEUM_INDEX	(38*38*38)			// 3연속모음들에 부여할 수 있는 ID(index)의 최대 개수
#define	MAX_2MOEUM_INDEX	(38*38)				// 2연속모음들에 부여할 수 있는 ID(index)의 최대 개수
#define	HANGUL_BYTE			2					// 한글 코드의 바이트 수
#define	MAX_QUERY_LEN		30					// 입력될 수 있는 쿼리의 최대 길이. 한글은 2바이트이므로 한글만 들어오면 NULL까지 고려하면 24자까지 가능함
#define	MAX_THESAURUS_CNT	18					// 현재 샘플로 넣어 놓은 시소러스 라인 수.
#define	MAX_EACH_THESAURUS	3					// 시소러스 한 개체 당 최대 몇 개의 동일의미 문자열이 들어갈 수 있는가
//#define	TROUBLE_COUNT		30000				// 등장 줄번호 수가 이걸 넘으면 검색 시 시간을 잡아 먹을 수 있으니 그냥 넘어가자.
#define	MAX_OUT_LINES		20					// 스코어링 후 출력할 최대 개수
#define	MAX_MINUS_POINT		90

//중복글자 문제 해결부분
#define MAX_OVERLAP    1500   //전체글자 중       레코드내 중복이 있는 글자의 최대 개수
#define MAX_ONELINE    100    //레코드내 글자 중  레코드내 중복이 있는 글자의 최대 개수
//중복글자 문제 해결부분끝
//수식화
#define MAX_BLOCK      10     //블록 최대 개수
//수식화끝



extern unsigned int MAX_DB_LINE;
extern char *memory_db;							// 디비의 모든 라인을 읽어들일 메모리. 디비 파일의 크기를 보고 동적으로 할당하는 방식으로 바꿀 예정.
extern unsigned int *position_in_memory;		// 디비의 각 줄이 memory[] 배열 내에서 어디에서 시작하는지 그 위치를 담을 배열. 디비 파일의 라인 수를 세어서 동적으로 할당하는 방식을 바꿀 예정.
extern unsigned char *namepart_len;				// 디비 각 라인의 명칭부분 문자열 길이
extern unsigned char *addrpart_len;				// 디비 각 라인의 주소부분 문자열 길이
extern unsigned int db_line_count;				// 디비의 전체 라인 수가 저장됨.

extern int **searched;
extern unsigned char eo_string[MAX_QUERY_LEN][HANGUL_BYTE+1];
extern int **final;
extern int max_final;
extern int *mfd;
extern int eo_count;

extern unsigned int cho3_count[MAX_3MOEUM_INDEX];	// 3연속초성들 각각의 개수. 인덱스는 3연속초성이 가질 수 있는 모든 경우의 수임 
extern unsigned int cho2_count[MAX_2MOEUM_INDEX];	// 2연속초성들 각각의 개수. 인덱스는 2연속초성이 가질 수 있는 모든 경우의 수임 
extern unsigned int cho3_start[MAX_3MOEUM_INDEX];	// cho3_inverted[]에서 각 트리플이 시작하는 인덱스값을 저장하는 배열
extern unsigned int cho2_start[MAX_2MOEUM_INDEX];	// cho2_inverted[]에서 각 튜플이 시작하는 인덱스값을 저장하는 배열
extern unsigned char *cho3_inverted;				// 역파일 배열. 각 줄에 등장한 모든 3연속초성과 
extern unsigned char *cho2_inverted;				// 2연속초성들의 줄번호가 기록된다.
extern unsigned int MAX_INVFILE3_LEN;				// 3연속초성 역파일 배열의 바이트 수.
extern unsigned int MAX_INVFILE2_LEN;				// 2연속초성 역파일 배열의 바이트 수.
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

//중복글자 문제 해결부분(quick sort)
void q_sort(int numbers_t_int[],int numbers_count[], int left, int right); 
//중복글자 문제 해결부분(quick sort)끝

extern void scoring_and_output_4text(int fFullyIncluded);
