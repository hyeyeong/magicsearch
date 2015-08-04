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
//#include <windows.h>			//시간측정용

#include "main.h"
#include "sorting.h"
#include "redblack.h"
#include "hancodeconvert.h"


////////////////////////120802용 JAY
void do_caculate_close(int do_close_data[], int i);
int close_data[30];
int no_close;
//////////////////////
FILE *fp;		//디버깅용 파일

//const char db_file_name1[] = "다듬어진명칭들.txt";
//const char db_file_name2[] = "모두2.0-3.txt";
const char db_file_name3[] = "db.txt";
const char mem_db_file_name[] = "memdb.bin";
const char text_inverted_file_name[] = "text_inverted_file.bin";
const char chosung_inverted_file_name[] = "chosung_inverted_file.bin";
#define	IGNORE_NON_HANGUL

/*
 * load_db_in_memory()에 의해서 채워지는 변수들
 */
char*			memory_db;						// 디비의 모든 라인을 읽어들일 메모리. 디비 파일의 크기를 보고 동적으로 할당하는 방식으로 바꿀 예정.
unsigned int*	position_in_memory;				// 디비의 각 줄이 memory[] 배열 내에서 어디에서 시작하는지 그 위치를 담을 배열. 디비 파일의 라인 수를 세어서 동적으로 할당하는 방식을 바꿀 예정.
unsigned char*	namepart_len;					// 디비 각 라인의 명칭부분 문자열 길이
unsigned char*	addrpart_len;					// 디비 각 라인의 주소부분 문자열 길이
unsigned int	db_line_count;					// 디비의 전체 라인 수가 저장됨.
unsigned int	MAX_DB_LINE;					// 디비의 총 라인 수 제한

/*
 * 초성 검색을 위한 2-3 역인덱스 생성에 필요한 변수들
 */
unsigned int cho3_count[MAX_3MOEUM_INDEX];	// 3연속초성들 각각의 개수. 인덱스는 3연속초성가 가질 수 있는 모든 경우의 수임 
unsigned int cho2_count[MAX_2MOEUM_INDEX];	// 2연속초성들 각각의 개수. 인덱스는 2연속초성가 가질 수 있는 모든 경우의 수임 
unsigned int cho3_start[MAX_3MOEUM_INDEX];	// cho3_inverted[]에서 각 트리플이 시작하는 인덱스값을 저장하는 배열
unsigned int cho2_start[MAX_2MOEUM_INDEX];	// cho2_inverted[]에서 각 튜플이 시작하는 인덱스값을 저장하는 배열
unsigned char *cho3_inverted;				// 역파일 배열. 각 줄에 등장한 모든 3연속초성과 2연속초성들의 줄번호가 기록된다.
unsigned char *cho2_inverted;
unsigned int MAX_INVFILE3_LEN;				// 3연속초성 역파일 배열의 바이트 수.
unsigned int MAX_INVFILE2_LEN;				// 2연속초성 역파일 배열의 바이트 수.
unsigned int MAX_FORMER_LATER_CNT;
unsigned int MIN_FORMER_LATER_CNT;
int *former_list;
int *latter_list;
int **cho_final;							// MAX_FINAL_COL을 동적으로 알아내는 버전으로 바꿈.
int cho_final_count[MAX_QUERY_LEN/2]={0};
int cho_max_final;

/*
 * make_rbtree()에서 채우는 사용하는 전역 변수들
 */
RBTree rbt1;
RBTree rbt2;
int max_index;

/*
 * create_inverted_file() 함수에 의해서 채워지는 변수들
 */
// 텍스트 검색용
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
int *mfd;	// merged냐 merged2 냐...소팅할 때 쓰기 때문에 글로벌로 뺐어야 했음.

// 텍스트 검색용
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

int no_of_filtered_data = 0;	//스코어링할 필터링 데이터의 총 개수(18000개 이하)

//전체 DB 중복테이블 
int Overlap_Count_perChar_tint[MAX_OVERLAP]; //중복이 있는 글자 t_int
int Overlap_Count_perChar_count[MAX_OVERLAP]; //최대중복횟수


int main() 
{	
	int query_len;
	int fFullyIncluded;
	char in_query[MAX_QUERY_LEN*2] = {" "};			// 입력된 쿼리 문자열이 저장될 배열
	short query_cho[MAX_JAEUM_CNT] = {0};			// 쿼리에서 초성만 추출하여 저장한 배열
	int query_alpha[MAX_MOEUM_CNT] = {0};			// 쿼리에 포함되어 있는 알파벳들이 저장되는 배열
	int query_count=0;								// 쿼리에 있는 글자들의 개수
	int query_alpha_count = 0;						// 쿼리에 있는 숫자를 제외한 알파벳 글자의 개수

	load_db_in_memory();				// 디비를 메모리에 올리고, 초성용 2,3연속초성 ID를 만든다.
	
	fp = fopen("output.txt", "w");	//디버깅용 파일

	make_rbtree();						// 완성형용을 위한 글자별 ID를 만든다.
	create_inverted_file_4text();		// 완성형용 역인덱스 생성
	create_inverted_file_4chosung();	// 초성용 역인덱스 생성

	

	for (;;) {
		init_variables_4text();		// 다시 검색 시 초기화 해야 하는 완성형용 중요 배열들을 초기화한다.
		init_variables_4chosung();	// 다시 검색 시 초기화 해야 하는 초성용 중요 배열들을 초기화한다.
		do {
			printf("\n검색어를 입력하세요(종료는 quit): ");
			gets(in_query);	// 공백같은 white char도 입력에 포함시키기 위해
			query_len = strlen((const char *)in_query);
			if (query_len > MAX_QUERY_LEN)
				printf("main(): 쿼리 길이가 %d를 넘었습니다. 다시 입력하세요.", MAX_QUERY_LEN);
		} while (query_len > MAX_QUERY_LEN);
		if (!strcmp(in_query, "quit"))
			break;

		printf("[검색 결과]\n");
		switch (CheckIfChosungIncluded(in_query)) {
		case -1:		// 완성형 검색만 하면 됨
			printf("CheckIfChosungIncluded()에서 -1 리턴: 완성형 글자만 포함되어 있음(유효쿼리: %s).\n", in_query);
			fFullyIncluded = search_inverted_file_4text(in_query);
			scoring_and_output_4text(fFullyIncluded);
			break;
		case 1:		// 초성 검색만 하면 됨.
			printf("CheckIfChosungIncluded()에서 1 리턴: 초성 글자만 포함되어 있음(유효쿼리: %s).\n", in_query);
			extract_chosung_from_query(in_query, query_cho, query_alpha, &query_count, &query_alpha_count);
			if (query_count < 2) {
				printf("main(): 쿼리 길이가 두 글자 미만이어서 초성 검색은 하지 않습니다.\n");
				continue;
			}
			search_using_inverted_file_4chosung(query_cho, query_count, query_alpha, query_alpha_count);
			//scoring_and_output_4chosung(query_cho, query_count, query_alpha, query_alpha_count);
			fFullyIncluded = -1;
			integrated_scoring_and_output(fFullyIncluded, in_query, cho_final[cho_max_final], cho_final_count[cho_max_final]);
			break;
		case 0:		// 완성형과 초성이 섞여 있는데 완성형 글자가 두 개 이상이어서 완성형 검색 결과에 대고 통합 스코어링 함.
			printf("CheckIfChosungIncluded()에서 0 리턴: 완성형 글자와 초성 섞였고, 완성형 두 개 이상 ==> 완성형 검색 결과에 통합 스코어링 함(유효쿼리: %s).\n", in_query);
			// 완성형 검색 실시
			fFullyIncluded = search_inverted_file_4text(in_query);
			//scoring_and_output_4text(fFullyIncluded);
			// 초성 검색 실시
			//extract_chosung_from_query(in_query, query_cho, query_alpha, &query_count, &query_alpha_count);
			//if (query_count < 2) {
			//	printf("main(): 쿼리 길이가 두 글자 미만이어서 초성 검색은 하지 않습니다.\n");
			//	continue;
			//}
			//search_using_inverted_file_4chosung(query_cho, query_count, query_alpha, query_alpha_count);
			//scoring_and_output_4chosung(query_cho, query_count);
			// 완성형 검색 우선 스코어링 실시
			integrated_scoring_and_output(fFullyIncluded, in_query, final[max_final], final_count[max_final]);
			break;
		case 2:		// 완성형과 초성이 섞여 있는데 완성형 글자가 두 개 미만이어서 초성 검색만 하고 그 결과에 대고 통합 스코어링 함
			printf("CheckIfChosungIncluded()에서 2 리턴: 완성형 글자와 초성 섞였고, 완성형은 딱 한 글자 ==> 초성 검색 결과에 통합 스코어링 함(유효쿼리: %s).\n", in_query);
			// 완성형 검색 실시
			//fFullyIncluded = search_inverted_file_4text(in_query);
			//scoring_and_output_4text(fFullyIncluded);
			// 초성 검색 실시
			extract_chosung_from_query(in_query, query_cho, query_alpha, &query_count, &query_alpha_count);
			if (query_count < 2) {
				printf("main(): 쿼리 길이가 두 글자 미만이어서 초성 검색은 하지 않습니다.\n");
				continue;
			}
			search_using_inverted_file_4chosung(query_cho, query_count, query_alpha, query_alpha_count);
			//scoring_and_output_4chosung(query_cho, query_count);
			// 완성형 검색 우선 스코어링 실시
			fFullyIncluded = -1;
			integrated_scoring_and_output(fFullyIncluded, in_query, cho_final[cho_max_final], cho_final_count[cho_max_final]);
			break;
		default:
			printf("main(): CheckIfChosungIncluded(in_query)에서 이상한 값이 넘어 왔습니다.\n");
			break;
		}
	}

	printf("cleaning...");
	cleanup();
	printf("done.\n");
	return 0;
}// end of main()

/*
 * 디비의 모든 라인을 읽으며 다음의 결과를 각 배열에 채운다.
 * 1. namepart_len[줄번호]		각 줄에서 '@' 이전의 명칭부분 문자열 길이를 저장
 * 2. addrpart_len[줄번호]		각 줄에서 '@' 이후의 주소부분 문자열 길이를 저장
 * 3. position_in_memory[줄번호]	각 줄 전체가 놓여져 있는 memory[] 내의 위치
 * 4. memory_db[필요한만큼]		디비의 모든 내용을 저장.
 * 5. cho3_count[3연속초성ID]	디비에 등장하는 각 3연속초성의 38진수 ID 별로 그 등장횟수를 저장.
 * 6. cho2_count2[2연속초성ID]	디비에 등장하는 각 2연속모음의 38진수 ID 별로 그 등장횟수를 저장.
 */
void
load_db_in_memory(void)
{
	unsigned int entry_count;			// 디비 줄번호를 세기 위해 사용됨
	//long p, prev;						// 디비 파일의 끝을 만났는지 알기 위해 사용
	bool naming;						// 명칭부분을 보고 있는지 주소 부분을 보고 있는지 구분하기 위한 구분자
	unsigned char t_char;				// 디비에서 한 글자씩 읽어 올 때 임시로 사용함
	char han[HANGUL_BYTE+1];			// 한글 처리 시 글자 하나를 담아 놓기 위한 공간. NULL 문자까지 포함하기 위해 +1.
	char c_han[HANGUL_BYTE+1];			// 완성형 글자를 조합형으로 바꿀 때 사용하기 위해 선언.
	int position_in_memdb;
	const char *dbfile = db_file_name3;
	struct stat buf;					// 디비 파일의 크기를 알아내기 위함
	long memory_db_size;
	char cu[20];
	size_t nRead;
	unsigned short johapcode;			// 완성형에서 조합형 코드로 변환된 값을 임시로 저장
	unsigned short cho_code;			// 분리된 초성 코드를 임시로 저장
	unsigned short cho[MAX_MOEUM_CNT];	// 명칭부분에 등장한 초성 코드들 배열. 매 줄을 처리할 때 매번 사용함.
	unsigned short cho_ad[MAX_MOEUM_CNT];// 주소부분에 등장한 초성 코드들 배열. 매 줄을 처리할 때 매번 사용함.
	int c_count = 0;					// 명칭부분의 초성 개수를 세기 위해 사용
	int c_ad_count = 0;					// 주소부분의 초성 개수를 세기 위해 사용
	int c_index;						// 3연속초성 또는 2연속초성을 38진법 ID로 변환할 때 임시로 사용. 즉, 3연속초성 또는 2연속초성의 ID를 담을 용도.
	
	// 이미 메모리 디비가 만들어져 있으면 새로 메모리 디비를 생성하지 않고 읽어들인다.
	FILE  *fp;
	if  ((fp = fopen(mem_db_file_name, "rb")) != NULL) {	// 메모리 디비가 이미 만들어져 있으면   
		printf("Loading the existing memory DB...");

		fread(&db_line_count, sizeof(unsigned int), 1, fp);	// db_line_count 읽어옴
		fread(&MAX_DB_LINE, sizeof(unsigned int), 1, fp);	// MAX_DB_LINE 읽어옴
		fread(&memory_db_size, sizeof(long), 1, fp);		// memory_db_size 읽어옴. 로컬 변수인데 글로벌로 빼야 할 듯...

		// 줄 개수 만큼 필요한 배열들을 할당한다.
		position_in_memory = (unsigned int *)malloc(sizeof(unsigned int)*MAX_DB_LINE);
		namepart_len = (unsigned char *)malloc(sizeof(unsigned char)*MAX_DB_LINE);
		addrpart_len = (unsigned char *)malloc(sizeof(unsigned char)*MAX_DB_LINE);
		if (position_in_memory == NULL || namepart_len == NULL || addrpart_len == NULL) {
			fprintf(stderr, "\t디비 라인 수 만큼을 필요로 하는 배열들을 할당하는 데에 실패했습니다.\n");
			exit(-3);
		}

		memory_db = (char *)malloc(memory_db_size);	// 디비 파일의 크기 만큼 memory_db[]를 할당한다.
		if (memory_db == NULL) {
			//fprintf(stderr, "\tmemory_db[]를 파일 크기 %s 바이트만큼 할당하는 데에 실패했습니다.\n", Int2Currency(memory_db_size, cu, false));
			fprintf(stderr, "failed to allocate memory_db");
			exit(-4);
		}
		//printf("\tmemory_db[]의 크기(파일 크기와 동일) = %s(%.3f MB).\n", Int2Currency(memory_db_size, cu, false), (double)memory_db_size/1024/1024);

		fread(position_in_memory, sizeof(unsigned int), MAX_DB_LINE, fp);	// position_in_memory_db[] 배열 읽어옴
		fread(namepart_len, sizeof(unsigned char), MAX_DB_LINE, fp);		// namepart_len[] 배열 읽어옴
		fread(addrpart_len, sizeof(unsigned char), MAX_DB_LINE, fp);		// addrpart_len[] 배열 읽어옴
		fread(memory_db, sizeof(char), memory_db_size, fp);					// memory_db[] 배열 읽어옴
		fread(cho3_count, sizeof(unsigned int), MAX_3MOEUM_INDEX, fp);		// 모든 3연속초성 ID들의 등장 회수 배열인 cho3_count[] 저장
		fread(cho2_count, sizeof(unsigned int), MAX_2MOEUM_INDEX, fp);		// 모든 2연속초성 ID들의 등장 회수 배열인 cho2_count[] 저장
		fclose(fp);	
	}
	else {	// 디비 원본을 읽어 메모리 디비를 생성한다.
		printf("Creating memory DB from raw DB ...\n");

		// 디비 파일을 오픈
		if  ((fp = fopen(dbfile, "rb")) == NULL) {
			fprintf(stderr, "file  open  error  to  read"); 
			exit(1); 
		} 

		// 디비 파일 내의 줄 개수를 알아낸다.
		MAX_DB_LINE = 0;
		for (;;) {
			nRead = fread(&t_char,  1,  1,  fp);
			if (nRead < 1)
				break;
			if (t_char == '\r')
				MAX_DB_LINE++;
		}
		MAX_DB_LINE++;			// 여분으로 한 줄 더 잡음.
		fseek(fp, 0, SEEK_SET);
		//printf("\t%s lines found in DB file %s\n", Int2Currency(MAX_DB_LINE-1, cu, false), dbfile);


		// 줄 개수 만큼 필요한 배열들을 할당한다.
		position_in_memory = (unsigned int *)malloc(sizeof(unsigned int)*MAX_DB_LINE);
		namepart_len = (unsigned char *)malloc(sizeof(unsigned char)*MAX_DB_LINE);
		addrpart_len = (unsigned char *)malloc(sizeof(unsigned char)*MAX_DB_LINE);
		if (position_in_memory == NULL || namepart_len == NULL || addrpart_len == NULL) {
			fprintf(stderr, "\t디비 라인 수 만큼을 필요로 하는 배열들을 할당하는 데에 실패했습니다.\n");
			exit(-1);
		}

		stat(dbfile, &buf);			// 디비 파일의 크기를 얻어와
		memory_db_size = buf.st_size;
		memory_db = (char *)malloc(memory_db_size);	// 디비 파일의 크기 만큼 memory_db[]를 할당한다.
		if (memory_db == NULL) {
			//fprintf(stderr, "\tmemory_db[]를 파일 크기 %s 바이트만큼 할당하는 데에 실패했습니다.\n", Int2Currency(memory_db_size, cu, false));
			exit(-1);
		}
		//printf("\tmemory_db[]의 크기(파일 크기와 동일) = %s(%.3f MB).\n", Int2Currency(memory_db_size, cu, false), (double)memory_db_size/1024/1024);

		entry_count = 0;
		position_in_memory[0] = 0;

		namepart_len[entry_count] = 0;
		addrpart_len[entry_count] = 0;

		naming = true;	// 이름 부분부터 시작한다고 가정

		for (;;) {
			nRead = fread(&t_char,  1,  1,  fp);
			if (nRead < 1 && feof(fp))
				break;
		
			if (t_char >= 128) {	// 한글이면
				han[0] = t_char;

				nRead = fread(&t_char,  1,  1,  fp);
				if (nRead < 1 && feof(fp))
					break;
			
				if (naming)
					namepart_len[entry_count] += 2;		// '@' 이전이면 namepart_len[줄번호] 1 증가
				else
					addrpart_len[entry_count] += 2;	// '@' 이후이면 addrpart_len[줄번호] 1 증가

				han[1] = t_char;
				han[2] = '\0';
				position_in_memdb = position_in_memory[entry_count] + namepart_len[entry_count] + addrpart_len[entry_count];
				memory_db[position_in_memdb - 2] = han[0];
				memory_db[position_in_memdb - 1] = han[1];

				// 초성 2,3 인덱스를 위해 추가됨
				HangulCodeConvert(KS, han, c_han);	// 완성형 한글코드 16비트를 조합형 16비트로 바꾼다.
				johapcode = ((unsigned char)c_han[0] << 8) | (unsigned char)c_han[1];
				cho_code = (johapcode >> 10) & 0x1f;		// 하위 10비트를 없애서 초성 코드를 추출한다.
				if (cho_code < 0x02 || cho_code > 0x14) {
					fprintf(stderr, "load_db_in_memory(): 줄번호 %d: 비정상 초성 글자 '%s' 발견! 무시합니다.\n", entry_count+1, han);
					continue;
				}
				if (naming) {	// 명칭 부분
					cho[c_count] = cho_code;
					c_count++;
				}
				else {			// 주소 부분
					cho_ad[c_ad_count] = cho_code;
					c_ad_count++;
				}
				// 초성 인덱스 처리를 위해 추가된 부분 끝
			}
			else if (t_char == '@' && naming) {		// 첫 '@'를 만남. 즉 명칭 부분의 끝을 만남. 같은 줄에 '@'가 또 나온다면 단순 아스키문자로 처리해야 함
				namepart_len[entry_count]++;			// 널 문자가 들어갈 자리를 마련함.
				position_in_memdb = position_in_memory[entry_count] + namepart_len[entry_count] + addrpart_len[entry_count];	// 다음 위치
				memory_db[position_in_memdb - 1] = NULL;	// 문자열 끝을 표시

				// 초성 2,3 인덱스를 위해 추가됨
				if (c_count == 1) {			// 명칭 부분에 있는 문자들의 초성 개수가 한 개밖에 없으면,
					c_index = cho[0]*38*38;
					if (c_index > MAX_3MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 3초성 인덱스가 MAX_3MOEUM_INDEX를 넘었습니다. 이는 초성코드가 이상한 것 입니다.\n");
						exit(-10);
					}
					cho3_count[c_index]++;
				
					c_index = cho[0]*38;
					if (c_index > MAX_2MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 2초성 인덱스가 MAX_2MOEUM_INDEX를 넘었습니다. 이는 초성코드가 이상한 것 입니다.\n");
						exit(-10);
					}
					cho2_count[c_index]++;
				}
				else if (c_count == 2) {	// 명칭 부분에 있는 문자들의 초성 개수가 두 개이면,
					c_index = cho[0]*38*38 + cho[1]*38;
					if (c_index > MAX_3MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 3초성 인덱스가 MAX_3MOEUM_INDEX를 넘었습니다. 이는 초성코드가 이상한 것 입니다.\n");
						exit(-10);
					}
					cho3_count[c_index]++;
				
					c_index = cho[0]*38 + cho[1];
					if (c_index > MAX_2MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 2초성인덱스가 MAX_2MOEUM_INDEX를 넘었습니다. 이는 초성코드가 이상한 것 입니다.\n");
						exit(-10);
					}
					cho2_count[c_index]++;
				}
				else {						// 명칭 부분에 있는 문자들의 초성 개수가 세 개이 이상면,
					int z;

					for (z = 0; z < c_count-2 ; z++) {	// 3연속초성을 묶어 38진수로 만든다.
						c_index = cho[z]*38*38 + cho[z+1]*38 + cho[z+2];
						if (c_index > MAX_3MOEUM_INDEX) {
							fprintf(stderr, "load_db_in_memory(): 3초성 인덱스가 MAX_3MOEUM_INDEX를 넘었습니다. 이는 초성코드가 이상한 것 입니다.\n");
							exit(-10);
						}
						cho3_count[c_index]++;
					}
					for (z = 0; z < c_count-1; z++) {	// 2연속초성을 묶어 38진수로 만든다.
						c_index = cho[z]*38 + cho[z+1];
						if (c_index > MAX_2MOEUM_INDEX) {
							fprintf(stderr, "load_db_in_memory(): 2초성 인덱스가 MAX_2MOEUM_INDEX를 넘었습니다. 이는 초성코드가 이상한 것 입니다.\n");
							exit(-10);
						}
						cho2_count[c_index]++;
					}
				}
				// 초성 인덱스 처리를 위해 추가된 부분 끝

				naming = false;
			}
			else if (t_char == '@' && !naming) {	// 주소부분에서 @를 또 만났음.명칭-주소부분 구분자로 사용하지 않고 디비에만 넣고 끝!
				addrpart_len[entry_count]++;
				position_in_memdb = position_in_memory[entry_count] + namepart_len[entry_count] + addrpart_len[entry_count];
				memory_db[position_in_memdb - 1] = t_char;
			}
			else if (t_char == '\r' && !naming) {	// 주소 부분을 보다가 한 줄이 다 끝났으면...
				addrpart_len[entry_count]++;	// NULL 들어갈 자리 확보
				position_in_memdb = position_in_memory[entry_count] + namepart_len[entry_count] + addrpart_len[entry_count];
				memory_db[position_in_memdb - 1] = NULL;	// 문자열 끝을 처리

				// 초성 2,3 인덱스를 위해 추가됨
				if (c_ad_count == 1) {
					c_index = cho_ad[0]*38*38;
					if (c_index > MAX_3MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 3초성 인덱스가 MAX_3MOEUM_INDEX를 넘었습니다. 이는 초성코드가 이상한 것 입니다.\n");
						exit(-10);
					}
					cho3_count[c_index]++;
				
					c_index = cho_ad[0]*38;
					if (c_index > MAX_2MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 2초성 인덱스가 MAX_2MOEUM_INDEX를 넘었습니다. 이는 초성코드가 이상한 것 입니다.\n");
						exit(-10);
					}
					cho2_count[c_index]++;
				}
				else if (c_ad_count == 2) {
					c_index = cho_ad[0]*38*38 + cho_ad[1]*38;
					if (c_index > MAX_3MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 3초성 인덱스가 MAX_3MOEUM_INDEX를 넘었습니다. 이는 초성코드가 이상한 것 입니다.\n");
						exit(-10);
					}
					cho3_count[c_index]++;
				
					c_index = cho_ad[0]*38 + cho_ad[1];
					if (c_index > MAX_2MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 2초성 인덱스가 MAX_2MOEUM_INDEX를 넘었습니다. 이는 초성코드가 이상한 것 입니다.\n");
						exit(-10);
					}
					cho2_count[c_index]++;
				}
				else {
					int z;

					for (z = 0; z < c_ad_count-2 ; z++) {
						c_index = cho_ad[z]*38*38 + cho_ad[z+1]*38 + cho_ad[z+2];
						if (c_index > MAX_3MOEUM_INDEX) {
							fprintf(stderr, "load_db_in_memory(): 3초성 인덱스가 MAX_3MOEUM_INDEX를 넘었습니다. 이는 초성코드가 이상한 것 입니다.\n");
							exit(-10);
						}
						cho3_count[c_index]++;
					}
					for (z = 0; z < c_ad_count-1 ; z++) {
						c_index = cho_ad[z]*38 + cho_ad[z+1];
						if (c_index > MAX_2MOEUM_INDEX) {
							fprintf(stderr, "load_db_in_memory(): 2초성 인덱스가 MAX_2MOEUM_INDEX를 넘었습니다. 이는 초성코드가 이상한 것 입니다.\n");
							exit(-10);
						}
						cho2_count[c_index]++;
					}
				}
				c_ad_count=0;	// 한 줄이 끝났으므로 주소부분과
				c_count=0;		// 명칭부분의 초성 개수 로컬 변수를 리셋한다.
				// 초성 인덱스 처리를 위해 추가된 부분 끝

				entry_count++;
				if (entry_count >= MAX_DB_LINE) {
					fprintf(stderr, "load_db_in_memory(): 디비 라인 수가 %d를 넘었습니다. 늘리고 다시 컴파일 하세요.\n", entry_count);
					exit(-1);
				}
				// 다음 줄의 정보를 담을 memory_db[] 내의 초기 위치 position_in_memory_db[entry_count]를 계산해둔다.
				position_in_memory[entry_count] = position_in_memory[entry_count-1] + namepart_len[entry_count-1] + addrpart_len[entry_count-1];
				namepart_len[entry_count] = addrpart_len[entry_count] = 0;
				naming = true;
			}
			else if (t_char == '\r' && naming) { // 한 줄이 다 끝났는데 여전히 명칭 부분을 하고 있었으면, 즉 이 줄에는 '@'가 없음
				namepart_len[entry_count]++;
				position_in_memdb = position_in_memory[entry_count] + namepart_len[entry_count] + addrpart_len[entry_count];
				memory_db[position_in_memdb - 1] = NULL;	// 문자열 끝을 표시

				// 초성 2,3 인덱스를 위해 추가됨
				if (c_count == 1) {			// 명칭 부분에 있는 문자들의 모음 개수가 한 개밖에 없으면,
					c_index = cho[0]*38*38;
					if (c_index > MAX_3MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 3초성 인덱스가 MAX_3MOEUM_INDEX를 넘었습니다. 이는 초성코드가 이상한 것 입니다.\n");
						exit(-10);
					}
					cho3_count[c_index]++;
				
					c_index = cho[0]*38;
					if (c_index > MAX_2MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 2초성 인덱스가 MAX_2MOEUM_INDEX를 넘었습니다. 이는 초성코드가 이상한 것 입니다.\n");
						exit(-10);
					}
					cho2_count[c_index]++;
				}
				else if (c_count == 2) {	// 명칭 부분에 있는 문자들의 모음 개수가 두 개이면,
					c_index = cho[0]*38*38 + cho[1]*38;
					if (c_index > MAX_3MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 3초성 인덱스가 MAX_3MOEUM_INDEX를 넘었습니다. 이는 초성코드가 이상한 것 입니다.\n");
						exit(-10);
					}
					cho3_count[c_index]++;
				
					c_index = cho[0]*38 + cho[1];
					if (c_index > MAX_2MOEUM_INDEX) {
						fprintf(stderr, "load_db_in_memory(): 2초성 인덱스가 MAX_2MOEUM_INDEX를 넘었습니다. 이는 초성코드가 이상한 것 입니다.\n");
						exit(-10);
					}
					cho2_count[c_index]++;
				}
				else {						// 명칭 부분에 있는 문자들의 모음 개수가 세 개이 이상면,
					int z;

					for (z = 0; z < c_count-2 ; z++) {	// 모음 세 개
						c_index = cho[z]*38*38 + cho[z+1]*38 + cho[z+2];
						if (c_index > MAX_3MOEUM_INDEX) {
							fprintf(stderr, "load_db_in_memory(): 3초성 인덱스가 MAX_3MOEUM_INDEX를 넘었습니다. 이는 초성코드가 이상한 것 입니다.\n");
							exit(-10);
						}
						cho3_count[c_index]++;
					}
					for (z = 0; z < c_count-1; z++) {	// 모음 두 개
						c_index = cho[z]*38 + cho[z+1];
						if (c_index > MAX_2MOEUM_INDEX) {
							fprintf(stderr, "load_db_in_memory(): 2초성 인덱스가 MAX_2MOEUM_INDEX를 넘었습니다. 이는 초성코드가 이상한 것 입니다.\n");
							exit(-10);
						}
						cho2_count[c_index]++;
					}
				}
				c_ad_count=0;	// 한 줄이 끝났으므로 주소부분과
				c_count=0;		// 명칭부분의 초성 개수 로컬 변수를 리셋한다.
				// 초성 인덱스 처리를 위해 추가된 부분 끝

				entry_count++;
				if (entry_count >= MAX_DB_LINE) {
					fprintf(stderr, "load_db_in_memory(): 디비 라인 수가 %d를 넘었습니다. 늘리고 다시 컴파일 하세요.\n", entry_count);
					exit(-1);
				}
				position_in_memory[entry_count] = position_in_memory[entry_count-1] + namepart_len[entry_count-1] + addrpart_len[entry_count-1];
				namepart_len[entry_count] = addrpart_len[entry_count] = 0;
			}
			else if (t_char == '\n')
				continue;
	#ifdef	IGNORE_NON_HANGUL	// 초성 또는 중성 추출이 필요 없는 경우는 '한글이외무시' 매크로를 define 해야 한다.
			else {			// 한글이 아니면 memory_db[]에는 넣고 모음 추출은 하지 않는다. 주소부분에서 @를 또 만난 경우도 여기로 떨어짐.
				if (naming)
					namepart_len[entry_count]++;
				else
					addrpart_len[entry_count]++;

				position_in_memdb = position_in_memory[entry_count] + namepart_len[entry_count] + addrpart_len[entry_count];
				memory_db[position_in_memdb - 1] = isalpha(t_char) ? toupper(t_char) : t_char;	// 알파벳인 경우 대문자로 바꿔서 메모리 디비에 저장함
			}
	#else	// 초성 또는 모음 추출이 필요한 경우 '한글이외무시' 매크로를 undef 해야 한다.
			else if (t_char >= '0' && t_char <= '9') {	// 숫자
				if (naming)
					namepart_len[entry_count]++;
				else
					addrpart_len[entry_count]++;

				position_in_memdb = position_in_memory[entry_count] + namepart_len[entry_count] + addrpart_len[entry_count];
				memory_db[position_in_memdb - 1] = t_char;

				if (naming)
					cho[c_count++] = t_char-'0'+0x15;		// '0': 초성코드21, '1': 22, ..., '9': 30
				else
					cho_ad[c_ad_count++] = t_char-'0'+0x15;
			}
			else { // 블랭크를 포함한 일반 아스키 문자
				if (naming)
					namepart_len[entry_count]++;
				else
					addrpart_len[entry_count]++;

				position_in_memdb = position_in_memory[entry_count] + namepart_len[entry_count] + addrpart_len[entry_count];
				memory_db[position_in_memdb - 1] = isalpha(t_char) ? toupper(t_char) : t_char;	// 알파벳인 경우 대문자로 바꿔서 메모리 디비에 저장함
	
				if (naming)
					cho[c_count++] = 0;			// 숫자를 제외한 일반 아스키문자는 초성 코드 생성 불가!!!
				else
					cho_ad[c_ad_count++] = 0;	// 그래서 일단 0을 채워둔다.
			}
	#endif
		} // end of for (;;)
		fclose(fp);
		db_line_count = entry_count;
		//printf("\t%d 라인, memory_db_index = %d out of (0 ~ %d).\ndone.\n", db_line_count, position_in_memdb, memory_db_size-1);
		
		printf("\tSaving memory DB ...");
		if  ((fp = fopen(mem_db_file_name, "wb")) == NULL) {	// 메모리 디비 생성. 생성에 실패했으면,   
			fprintf(stderr, "memory DB 파일 생성용 오픈 실패!");
			exit(-20);
		}
		fwrite(&db_line_count, sizeof(unsigned int), 1, fp);				// db_line_count 저장
		fwrite(&MAX_DB_LINE, sizeof(unsigned int), 1, fp);					// MAX_DB_LINE 저장
		fwrite(&memory_db_size, sizeof(long), 1, fp);						// memory_db_size 저장. 로컬 변수인데 글로벌로 빼야 할 듯...
		fwrite(position_in_memory, sizeof(unsigned int), MAX_DB_LINE, fp);	// position_in_memory_db[] 저장
		fwrite(namepart_len, sizeof(unsigned char), MAX_DB_LINE, fp);		// namepart_len[] 저장
		fwrite(addrpart_len, sizeof(unsigned char), MAX_DB_LINE, fp);		// addrpart_len[] 저장
		fwrite(memory_db, sizeof(char), memory_db_size, fp);				// memory_db[] 저장.
		fwrite(cho3_count, sizeof(unsigned int), MAX_3MOEUM_INDEX, fp);		// 모든 3연속초성 ID들의 등장 회수 배열인 cho3_count[] 저장
		fwrite(cho2_count, sizeof(unsigned int), MAX_2MOEUM_INDEX, fp);		// 모든 2연속초성 ID들의 등장 회수 배열인 cho2_count[] 저장
		fclose(fp);
		printf("Done.\n");
	}
	printf("Done.\n");
}

/*
 * 디비의 명칭 부분에 대해서만, 등장하는 모든 unique 글자들에 대해 ID를 만들기 위한 작업임.
 * 주소부분은 2,3 초성 인덱스 생성 시 포함되었음. 
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

	//중복글자 문제 해결부분
	int total_chrnum_indb=0;//db에 있는 총 글자 갯수 
	int Max_t_int =0;//t_int 최대값
	
	bool new_t_int = false;//이미 있는 t_int값인지 검사
	bool thatchar = false;//제대로된 글자여부 확인 //이 변수가 없을 경우 null값인 마지막 루프에서도 중복이 추가된다. 
	
	
	//한 레코드 중복
	int Overlap_Search_perLine[MAX_ONELINE][2];//한 레코드의 중복검사
	bool newchar_perLine = true; //한 레코드의 중복글자인지 확인
	int index_of_Overlap_Search_perLine;//인덱스
	
	//전체 DB 중복 테이블 변수들
	bool newoverlapchar = true; //전체 DB의 중복글자인지 확인
	int index_of_Overlap_Count_perChar=0;//인덱스

	
	for(int i=0; i<MAX_ONELINE; i++){//제거가능 하지만 for문의 초기화문이 for문 아래로 내려가면 있어야함 
		Overlap_Search_perLine[i][0]=0;
		Overlap_Search_perLine[i][1]=0;
	}
	for(int i=0; i<MAX_OVERLAP;i++){
		Overlap_Count_perChar_tint[i]=0;
		Overlap_Count_perChar_count[i]=0;
	}
	//중복글자 문제 해결부분끝

	unsigned char output_test[HANGUL_BYTE];//디버깅용 출력
	

	// 모든 줄의 명칭부분 문자들을 d라는 RB트리에 넣는다
	for (unsigned int i = 0; i < db_line_count; i++) {	// 디비의 모든 줄에 대해서 루프를 돈다.
		
		//중복글자 문제 해결부분
		index_of_Overlap_Search_perLine= 0;//한 레코드 중복검사 인덱스 초기화
			
		for(int i=0; i<MAX_ONELINE; i++){
			//한 레코드 중복검사 배열 초기화
			Overlap_Search_perLine[i][0]=0;
			Overlap_Search_perLine[i][1]=0;
		}
		//중복글자 문제 해결부분끝
				
		cur = position_in_memory[i];		// 이 줄의 내용이 저장된 memory_db[] 내의 시작 위치
		len = namepart_len[i];	// 이 줄에 포함된 이름 부분('@' 앞 부분)의 길이

		for (;;) {
			t_char = memory_db[cur++]; 
			len--;		// 더 봐야 할 이름 부분의 길이를 빼 나간다.
			

			//중복글자 문제 해결부분
			thatchar = false;
			//끝

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
				

				//중복글자 문제 해결부분
				thatchar = true;
				//끝

				rbt1.insert(t_int, 0);			// 레드-블랙트리에 문자 하나를 삽입한다. 근데 코드의 앞뒤 바이트가 뒤바뀌어 들어가는데 이건 이상한 것임.

				
			}
			else { // 블랭크를 포함한 일반 아스키 문자
				if (t_char != 0) {				// 이름부분이나 주소부분 문자열 맨 뒤에 있는 널 문자를 제외하기 위해.
					t_int = (int)t_char;
					rbt1.insert(t_int, 0);       // 레드-블랙트리에 삽입
					
					//중복글자 문제 해결부분
					thatchar = true;
					//끝
				}
			}

			
			//중복글자 문제 해결부분		
			//한 레코드 안의 중복글자 확인하는 부분
			if(thatchar == true){
				newchar_perLine = true;
				
				for(int i=0; i<index_of_Overlap_Search_perLine; i++){
					if(Overlap_Search_perLine[i][0]==t_int){//중복글자이면 기존것에 갯수증가시킴
						Overlap_Search_perLine[i][1]++;
						newchar_perLine = false;
						break;
					}
				}

				if(newchar_perLine == true){//중복글자가 아니면 새롭게 추가함
					Overlap_Search_perLine[index_of_Overlap_Search_perLine][0]=t_int;
					Overlap_Search_perLine[index_of_Overlap_Search_perLine][1]++;
					index_of_Overlap_Search_perLine++;
				}
							
			}
			//중복글자 문제 해결부분끝

			if (len <= 0)
				break;
		} // end of for (;;)
		

		//중복글자 문제 해결부분
		for(int i=0; i<MAX_ONELINE; i++){
			//전체 DB 중복 글자에 한 레코드에서 알아낸 글자들 포함하는 부분
			if(Overlap_Search_perLine[i][1]>1){//중복글자라면
				newoverlapchar = true;
				for(int t=0; t<index_of_Overlap_Count_perChar; t++)//이미 중복글자로 저장되어 있으면
					if(Overlap_Count_perChar_tint[t]==Overlap_Search_perLine[i][0]){
						newoverlapchar = false;
						if(Overlap_Count_perChar_count[t]<Overlap_Search_perLine[i][1])
							Overlap_Count_perChar_count[t]=Overlap_Search_perLine[i][1];
						break;
					}
				if(newoverlapchar == true){//새로운 중복글자이면
					Overlap_Count_perChar_tint[index_of_Overlap_Count_perChar]=Overlap_Search_perLine[i][0];
					Overlap_Count_perChar_count[index_of_Overlap_Count_perChar]=Overlap_Search_perLine[i][1];
					index_of_Overlap_Count_perChar++;
				}
			}
		}
		//중복글자 문제 해결부분끝

	} // end of for (unsigned int i=0; i < db_line_count; i++)
	//printf("\n");
	count = 0;

	
	//중복글자 문제 해결부분
	han[0] = 0;//배열 재초기화
	han[1] = 0;
	han[2] = 0;
	
	//출력
	printf("db에 있는 총 글자 갯수 total_chrnum_indb = %d\n", total_chrnum_indb);
	printf("db에 있는 총 가장큰 글자번호 Max_t_int = %d\n", Max_t_int);
	//printf("2000개의 배열 중 어디까지 찼는지 index_of_dbindex_int = %d\n", index_of_dbindex_int);

	q_sort(Overlap_Count_perChar_tint, Overlap_Count_perChar_count, 0, MAX_OVERLAP-1);//전체 DB의 중복글자 배열을 sort함
	
	int counting = 0;//전체 DB 중복글자 배열을 증가시킬때 사용//sorting했기 때문에 가능
	int overlap_num = 0;//중복횟수만큼 인덱스의 갯수를 늘리기 위해 사용
	//중복글자 문제 해결부분끝



	for (unsigned int i = 0; i < (unsigned int)0xFFFF; i++) {	// 모든 글자(한글 또는 아스키) 코드들에 대해서...완성형 코드의 제일 끝 코드가 0xFDFF(65,023)
		result = rbt1.search(i);    // 첫번째 레드-블랙트리에서 찾아봄
		if (result != NULL) {
			
			//중복글자 문제 해결부분
			while (Overlap_Count_perChar_tint[counting]<i){//전체 DB 중복글자 배열을 증가시키다가
				counting++;
			}
			if(Overlap_Count_perChar_tint[counting]==i){//중복글자라고 나오면
				overlap_num = Overlap_Count_perChar_count[counting];//얼만큼 인덱스를 추가해야하는지 저장
			}else{
				overlap_num = 1;
			}
			//중복글자 문제 해결부분끝


			for(int t = 0; t<overlap_num; t++){
				rbt2.insert(i, count);       // 두번째 레드-블랙트리에 삽입
				count++;
			}
		}
	}

	max_index = count;
	printf("레드 블랙 트리에서 얻어낸 max_index = %d\n", max_index);

}

//중복글자 문제 해결부분(quick sort)
//보통은 배열 1개만 인자로 갖지만 여기에서는 배열 2개를 인자로 갖는다.
void q_sort(int numbers_t_int[],int numbers_count[], int left, int right)
{
	int pivot, l_hold, r_hold, pivot_count;
	l_hold = left;
	r_hold = right;
	pivot = numbers_t_int[left];
	pivot_count = numbers_count[left];//두번째 배열

	while(left<right)
	{
		while((numbers_t_int[right] >= pivot) && (left < right))
			right--;
		if(left != right)
		{
			numbers_t_int[left] = numbers_t_int[right];
			numbers_count[left] = numbers_count[right];//두번째 배열
			left++;
		}
		while((numbers_t_int[left] <= pivot) && (left < right))
			left++;
		if(left != right)
		{
			numbers_t_int[right] = numbers_t_int[left];
			numbers_count[right] = numbers_count[left];//두번째 배열
			right--;
		}
	}
	numbers_t_int[left] = pivot;
	numbers_count[left] = pivot_count;//두번째 배열
	
	pivot = left;
	left = l_hold;
	right = r_hold;
	if(left<pivot)
		q_sort(numbers_t_int,numbers_count, left, pivot-1);
	if(right>pivot)
		q_sort(numbers_t_int,numbers_count, pivot+1, right);
}
//중복글자 문제 해결부분(quick sort)끝


/*
 * 디비의 명칭부분에 등장하는 모든 글자들에 대해서 텍스트 검색을 위한 역인덱스를 만든다.
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

	i_count = (int *)malloc(max_index*sizeof(int));		// 글로벌 배열. 검색 시 사용됨.
	i_start = (int *)malloc(max_index*sizeof(int));		// 글로벌 배열. 검색 시 사용됨.
	i_cur = (int *)malloc(max_index*sizeof(int));		// 로컬 배열. 검색 시 사용 안 됨.
	if (i_count == NULL || i_start == NULL || i_cur == NULL) {
		fprintf(stderr, "i_count[], i_start[], 또는 i_cur[] 배열에 정수 %d개씩 할당하는 데에 실패했습니다.\n", max_index);
		exit(-3);
	}

	FILE  *fp;
	if  ((fp = fopen(text_inverted_file_name, "rb")) != NULL) {	// 완성형용 역파일이 이미 만들어져 있으면
		int tmp_max_index;

		printf("Loading the existing text inverted file...");
		
		fread(&tmp_max_index, sizeof(int), 1, fp);							// max_index 변수 읽어옴
	
		if (tmp_max_index != max_index) {
			fprintf(stderr, "저장된 텍스트용 역파일에서 max_index 값이 잘못되었습니다. 깨졌나 봅니다.\n");
			exit(-4);
		}
		fread(&MAX_SEARCHED_COL, sizeof(unsigned int), 1, fp);					// MAX_SEARCHED_COL 변수 읽어옴
		fread(&MAX_BYTES_IN_INVERTED_FILE, sizeof(unsigned int), 1, fp);		// MAX_BYTES_IN_INVERTED_FILE 변수 읽어옴
		fread(i_count, sizeof(int), max_index, fp);								// i_count[] 배열 읽어옴
		fread(i_start, sizeof(int), max_index, fp);								// i_start[] 배열 읽어옴
		inverted = (unsigned char *)malloc(sizeof(unsigned char)*MAX_BYTES_IN_INVERTED_FILE);
		
		if (inverted == NULL) {
			fprintf(stderr, "inverted[] 배열 할당에 실패했습니다. 메모리가 부족합니다.\n");
			exit(-3);
		}
		fread(inverted, sizeof(unsigned char), MAX_BYTES_IN_INVERTED_FILE, fp);	// inverted[] 배열 읽어옴
		fclose(fp);	
		printf("done.\n");
			
	}
	else {
		for (i = 0; i < max_index; i++)
			i_count[i] = 0;

		//중복글자 문제 해결부분
		//모두 한 레코드안에 중복여부를 확인하기 위한 변수들
		int Overlap_Search_perLine[MAX_ONELINE][2];
		bool newchar_perLine = true;
		
		int overlapnum;//한 레코드에서 중복횟수

		bool create_inverted_switch = true;//역인덱스 생성시 한글과 한글이 아닌 글자의 작업방식이 다를때 구분해줌
		bool thatchar = false; //제대로된 글자여부 확인 //이 변수가 없을 경우 null값인 마지막 루프에서도 중복이 추가된다. 
		//중복글자 문제 해결부분끝

		for (unsigned int z = 0; z < db_line_count; z++) {
			cur = position_in_memory[z];		// 이 줄의 내용이 저장된 memory_db[] 내의 시작 위치
			len = namepart_len[z];	// 이 줄에 포함된 이름 부분('@' 앞 부분)의 길이

			//중복글자 문제 해결부분
			int index_of_Overlap_Search_perLine= 0;//한 레코드 중복글자 배열 초기화
			for(int i=0; i<MAX_ONELINE; i++){
				Overlap_Search_perLine[i][0]=0;
				Overlap_Search_perLine[i][1]=0;
			}
			//중복글자 문제 해결부분끝

			for (;;) {
				t_char = memory_db[cur++]; 
				len--;

				//중복글자 문제 해결부분
				thatchar = false;//초기화
				//끝

				if ((unsigned int)t_char >= 128) {
					han[0] = t_char;
				
					t_char = memory_db[cur++]; 
					len--;

					han[1] = t_char;
					han[2] = '\0';

					t_int = (int)(*(unsigned short*)han);

					//중복글자 문제 해결부분
					thatchar = true;
					//끝
				}
				else { // 블랭크를 포함한 일반 아스키 문자
					if (t_char != 0) {				// 이름부분이나 주소부분 문자열 맨 뒤에 있는 널 문자를 제외하기 위해.
						t_int = (int)t_char;

						//중복글자 문제 해결부분
						thatchar = true;
						//끝
					}
				}

				//중복글자 문제 해결부분
				if(thatchar == true){
					newchar_perLine = true;
					overlapnum = 0;//중복글자를 포함한 인덱스를 반영하기 위해 추가
					for(int i=0; i<index_of_Overlap_Search_perLine; i++){
						if(Overlap_Search_perLine[i][0]==t_int){//중복문자일 경우
							Overlap_Search_perLine[i][1]++;
							newchar_perLine = false;
							//중복글자를 포함한 인덱스를 반영하기 위해 추가
							overlapnum = Overlap_Search_perLine[i][1]-1;//-1추가 => 에러해결   
							break;
						}
					}

					if(newchar_perLine == true){//새로운 문자일 경우
						Overlap_Search_perLine[index_of_Overlap_Search_perLine][0]=t_int;
						Overlap_Search_perLine[index_of_Overlap_Search_perLine][1]++;
						index_of_Overlap_Search_perLine++;
					}
				//중복글자 문제 해결부분끝

					
					if (result = rbt2.search(t_int)) {	
							index = result->index;
							
							//중복글자 문제 해결부분
							index = index + overlapnum;//중복만큼 인덱스증가
							//끝

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
		for (i = 1; i < max_index; i++) {	// 디비에 등장한 모든 글자에 대해...
			i_start[i] = i_start[i-1] + i_count[i-1]*3;		// 세 바이트(24비트)를 사용한다는 것은 2^24 = 약 1600만 줄까지의 디비를 허용하겠다는 의미!!!
			if ((unsigned int)(i_start[i]+i_count[i]*3) > max_i_start)
				max_i_start = i_start[i] + i_count[i]*3;	// inverted[] 내에서 지금까지 필요하다고 판단된 공간의 끝+1 인덱스, 즉 크기
			if ((unsigned int)i_count[i] > max_i_count)
				max_i_count = i_count[i];		// 어떤 글자가 디비에 등장한 줄번호의 개수가 가장 많은 경우 줄번호 개수
		}
		MAX_SEARCHED_COL = max_i_count;
		printf("\tinverted[]의 크기(max_i_start) = %d.\n", max_i_start);
		printf("\t가장 여러 줄에서 등장한 글자의 등장 줄번호 개수(max_i_count) = %d.\n", MAX_SEARCHED_COL);

		MAX_BYTES_IN_INVERTED_FILE = max_i_start;
		inverted = (unsigned char *)malloc(sizeof(unsigned char)*MAX_BYTES_IN_INVERTED_FILE);
		if (inverted == NULL) {
			fprintf(stderr, "inverted[] 배열 할당에 실패했습니다. 이는 심각한 것 입니다.\n");
			exit(-3);
		}
					
		// 같은 글자가 여려 번 등장할 경우 각각에 대해 원본 디비의 어느 라인에서 등장했는지 알아야 한다. 이 때 (같은) 각 글자를 구분하기 위한 구분자 
		for (i = 0; i < max_index; i++)
			i_cur[i] = 0;

		printf("텍스트 Inverted[] 생성 시작...");
		
		
		// --------------------------------------------------- 역파일 생성부 ----------------------------
		for (unsigned int i = 0; i < db_line_count; i++) {
			cur = position_in_memory[i];		// 이 줄의 내용이 저장된 memory_db[] 내의 시작 위치
			len = namepart_len[i];	// 이 줄에 포함된 이름 부분('@' 앞 부분)의 길이
			
			
			//중복글자 문제 해결부분
			int index_of_Overlap_Search_perLine= 0;//한 레코드 중복글자 배열 초기화
			for(int t=0; t<MAX_ONELINE; t++){
				Overlap_Search_perLine[t][0]=0;
				Overlap_Search_perLine[t][1]=0;
			}
			//중복글자 문제 해결부분끝
			
			for (;;) {
				t_char = memory_db[cur++];
				len--;
				
				//중복글자 문제 해결부분
				thatchar = false;//초기화
				//끝
				
				if ((unsigned int)t_char >= 128) {
					han[0] = t_char;
				
					t_char = memory_db[cur++]; 
					len--;

					han[1] = t_char;
					han[2] = '\0';

					t_int = (int)(*(unsigned short*)han);

					//중복글자 문제 해결부분
					create_inverted_switch =true;
					thatchar = true;
					//끝

					//중복글자 문제 해결을 위해 주석처리한 부분
					/*if (result = rbt2.search(t_int)) {	
						index = result->index;  
						//fprintf(p_file0, "(%d)\n", index);
						// 이 글자가 여러 번 등장할 텐데 각각에 대해 원본 디비의 줄 번호를 inverted 배열에 넣어둔다.
						// 이제 그 글자의 i_start[] 내 인덱스(레드블랙트리에서 찾아보면 알 수 있음)를 알면 원본 디비의 몇 번 줄에 등장했는지 알 수 있음.
						// 즉, 어떤 글자가 있으면 그 글자를 레드블랙트리에서 검색하여 인덱스를 알아내고, 인덱스를 알면 몇번, 몇번, 몇번 줄에서 등장했음을 알 수 있다.
						// 그 줄번호 저장을 위해 3바이트를 쓴다. 그러면 대략 1600만줄 이상을 커버할 수 있다. 원본 디비의 줄 수가 2^24를 넘으면 3을 4로 바꿔줘야 함.
						inverted_index = i_start[index] + i_cur[index]*3;
						if (inverted_index > (unsigned long)MAX_BYTES_IN_INVERTED_FILE-3) {
							fprintf(stderr, "inverted[] 배열에서 오버플로우가 났습니다(에러코드 -4). inverted_index = %d\n", inverted_index);
							exit(-4);
						}
						inverted[inverted_index] = (i >> 16) & 255; 
						inverted[inverted_index + 1] = (i >> 8) & 255;
						inverted[inverted_index + 2] = i & 255;
						i_cur[index]++;		// 같은 글자가 여려 개 있는 경우 연이어 저장하기 위해 inverted 배열에서 바로 다음 위치로 간다.
					}*/
				}
				else { // 블랭크를 포함한 일반 아스키 문자
					if (t_char != 0) {				// 이름부분이나 주소부분 문자열 맨 뒤에 있는 널 문자를 제외하기 위해.
						t_int = (int)t_char;

						//중복글자 문제 해결부분
						create_inverted_switch =false;
						thatchar = true;
						//끝

						//중복글자 문제 해결을 위해 주석처리한 부분
						/*if (result = rbt2.search(t_int)) {
							index = result->index;  
							inverted_index = i_start[index] + i_cur[index]*3;
							if (inverted_index > MAX_BYTES_IN_INVERTED_FILE-3) {
								fprintf(stderr, "inverted[] 배열에서 오버플로우가 났습니다(에러코드 -5). inverted_index = %d\n", inverted_index);
								exit(-5);
							}
							inverted[inverted_index] = (i >> 16) & 255; 
							inverted[inverted_index + 1] = (i >> 8) & 255; 
							inverted[inverted_index + 2] = i & 255; 
							i_cur[index]++;
						}*/
					}
				}


				//중복글자 문제 해결부분
				if(thatchar == true){
					newchar_perLine = true;
					overlapnum = 0;//중복글자를 포함한 인덱스를 반영하기 위해 추가
					for(int t=0; t<index_of_Overlap_Search_perLine; t++){
						if(Overlap_Search_perLine[t][0]==t_int){//중복문자일 경우
							Overlap_Search_perLine[t][1]++;
							newchar_perLine = false;
							//중복글자를 포함한 인덱스를 반영하기 위해 추가
							overlapnum = Overlap_Search_perLine[t][1]-1;// -1추가 => 에러해결
							break;
						}
					}
					if(newchar_perLine == true){//새로운 문자일 경우
						Overlap_Search_perLine[index_of_Overlap_Search_perLine][0]=t_int;
						Overlap_Search_perLine[index_of_Overlap_Search_perLine][1]++;
						index_of_Overlap_Search_perLine++;
					}
				//중복글자 문제 해결부분끝
				
					if (result = rbt2.search(t_int)) {
						index = result->index; 

						//중복글자 문제 해결부분
						index = index + overlapnum;
						//끝

						inverted_index = i_start[index] + i_cur[index]*3;

						if(create_inverted_switch ==true){
							if (inverted_index > (unsigned long)MAX_BYTES_IN_INVERTED_FILE-3) {
								fprintf(stderr, "inverted[] 배열에서 오버플로우가 났습니다(에러코드 -4). inverted_index = %d\n", inverted_index);
								exit(-4);
							}else{}
						}
						else{
							if (inverted_index > MAX_BYTES_IN_INVERTED_FILE-3) {
									fprintf(stderr, "inverted[] 배열에서 오버플로우가 났습니다(에러코드 -5). inverted_index = %d\n", inverted_index);
									exit(-5);
							}else{}
						}
						
						inverted[inverted_index] = (i >> 16) & 255; 
						inverted[inverted_index + 1] = (i >> 8) & 255; 
						inverted[inverted_index + 2] = i & 255; 
						i_cur[index]++;
					}
				}

				if (len <= 0)		// 어떤 한 줄의 name 부분 처리가 끝났으면 다음 줄로 간다.
					break;
			} // end of for (;;)
		}// end of for (unsigned int i=0; i < entry_count; i++) {
		// -------------------------------------- 역파일생성부 끝 --------------------------------------
		printf("done.\n");
	
		free(i_cur);	// 다 썼으므로 반환해도 됨.
		
		fp = fopen(text_inverted_file_name, "wb");	// 완성형용 역파일을 파일에 저장하여 담부턴 그냥 읽어들일 수 있도록 한다
		if (fp == NULL) {
			fprintf(stderr, "텍스트용 역파일 저장을 위한 파일 생성 실패!\n");
			exit(-5);
		}

		printf("Saving the text inverted file...");

		fwrite(&max_index, sizeof(int), 1, fp);										// max_index 변수 저장
		fwrite(&MAX_SEARCHED_COL, sizeof(unsigned int), 1, fp);						// MAX_SEARCHED_COL 변수 저장
		fwrite(&MAX_BYTES_IN_INVERTED_FILE, sizeof(unsigned int), 1, fp);			// MAX_BYTES_IN_INVERTED_FILE 변수 저장
		fwrite(i_count, sizeof(int), max_index, fp);								// i_count[] 배열 저장
		fwrite(i_start, sizeof(int), max_index, fp);								// i_start[] 배열 저장
		fwrite(inverted, sizeof(unsigned char), MAX_BYTES_IN_INVERTED_FILE, fp);	// inverted[] 배열 저장
		fclose(fp);	
		printf("done.\n");
	}

	// 검색 시 사용할 searched[MAX_QUERY_LEN][MAX_SEARCHED_COL]를 미리 할당해둔다.
	searched = (int **)malloc(sizeof(int *)*MAX_QUERY_LEN);
	if (searched == NULL) {
		fprintf(stderr, "searched[] 배열 할당에 실패했습니다. 이는 심각한 것 입니다.\n");
		exit(-3);
	}
	for (i = 0; i < MAX_QUERY_LEN; i++) {
		searched[i] = (int *)malloc(sizeof(int)*MAX_SEARCHED_COL);
		if (searched[i] == NULL) {
			fprintf(stderr, "searched[][] 배열 할당에 실패했습니다. 이는 심각한 것 입니다.\n");
			exit(-3);
		}
	}

	// 검색 시 사용할 final[MAX_QUERY_LEN][MAX_SEARCHED_COL]를 미리 할당해둔다.
	final = (int **)malloc(sizeof(int *)*MAX_QUERY_LEN);
	if (final == NULL) {
		fprintf(stderr, "final[] 배열 할당에 실패했습니다. 이는 심각한 것 입니다.\n");
		exit(-3);
	}
	for (i = 0; i < MAX_QUERY_LEN; i++) {
		final[i] = (int *)malloc(sizeof(int)*MAX_SEARCHED_COL);
		if (final[i] == NULL) {
			fprintf(stderr, "final[][] 배열 할당에 실패했습니다. 이는 심각한 것 입니다.\n");
			exit(-3);
		}
	}
}

/*
 * 초성 2,3인덱스의 역인덱스 생성부
 * create_inverted_file_4chosung(void);
 */
void
create_inverted_file_4chosung(void)
{
	int m, k, w, i;						// 루프 인덱스 변수
	unsigned int z;						//        ;;
	int cur = 0;						// memory[] 배열 내에서 각 줄이 시작하는 위치를 저장할 때 사용.
	short cho[MAX_MOEUM_CNT];			// 명칭부분에 등장한 모음 코드들 배열. 매 줄을 처리할 때 매번 사용함.
	short cho_ad[MAX_MOEUM_CNT];		// 주소부분에 등장한 모음 코드들 배열. 매 줄을 처리할 때 매번 사용함.
	short c_count = 0;					// 명칭부분의 모음 개수를 세기 위해 사용
	short c_ad_count = 0;				// 주소부분의 모음 개수를 세기 위해 사용
	char han[HANGUL_BYTE+1];			// 한글 처리 시 글자 하나를 담아 놓기 위한 공간. NULL 문자까지 포함하기 위해 +1.
	char c_han[HANGUL_BYTE+1];			// 완성형 글자를 조합형으로 바꿀 때 사용하기 위해 선언.
	unsigned short johapcode;			// 코드 변환 시 임시로 사용
	unsigned short cho_code;			// 초성 분리 시 임시로 사용
	int c_index;						// 3연속초성 또는 2연속초성을 38진법 ID로 변환할 때 임시로 사용. 즉, 3연속초성 또는 2연속초성의 ID를 담을 용도.
	unsigned int inverted_index, inverted2_index;
	unsigned int max_cho3_start = 0, max_cho3_count = 0;
	unsigned int max_cho2_start = 0, max_cho2_count = 0;
	char currency[20], currency2[20];	// 숫자를 천 단위에 콤마를 넣기 위한 배열
	FILE* fp;
	//static int tmp1=0, tmp2=0, tmp3=0;
	unsigned int c_cur[MAX_3MOEUM_INDEX];	// 3연속초성 역파일에 줄번호를 기록할 때 동일 3연속초성이 여러 번 등장할 경우 그 저장 위치를 증가시킬 때 사용
	unsigned int c_cur2[MAX_2MOEUM_INDEX];	// 2연속초성 역파일에 줄번호를 기록할 때 동일 2연속초성이 여러 번 등장할 경우 그 저장 위치를 증가시킬 때 사용

	if  ((fp = fopen(chosung_inverted_file_name, "rb")) != NULL) {	// 초성용 역파일이 이미 만들어져 있으면
		printf("Loading the 초성용 inverted file...");

		fread(&MAX_FORMER_LATER_CNT, sizeof(unsigned int), 1, fp);		// MAX_FORMER_LATER_CNT 변수 읽음
		fread(&MIN_FORMER_LATER_CNT, sizeof(unsigned int), 1, fp);		// MIN_FORMER_LATER_CNT 변수 읽음
		fread(&MAX_INVFILE3_LEN, sizeof(unsigned int), 1, fp);			// MAX_INVFILE3_LEN 변수 읽음
		fread(&MAX_INVFILE2_LEN, sizeof(unsigned int), 1, fp);			// MAX_INVFILE2_LEN 변수 읽음
		fread(cho3_start, sizeof(unsigned int), MAX_3MOEUM_INDEX, fp);	// cho3_start[] 배열 읽음
		fread(cho2_start, sizeof(unsigned int), MAX_2MOEUM_INDEX, fp);	// cho2_start[] 배열 읽음
		cho3_inverted = (unsigned char *)malloc(MAX_INVFILE3_LEN);
		cho2_inverted = (unsigned char *)malloc(MAX_INVFILE2_LEN);
		if (cho3_inverted == NULL || cho2_inverted == NULL) {
			fprintf(stderr, "\tcho3_inverted[] 또는 cho2_inverted[] 배열 할당에 실패. 메모리 부족일 겁니다.\n");
			exit(-4);
		}
		fread(cho3_inverted, sizeof(unsigned char), MAX_INVFILE3_LEN, fp);	// cho3_inverted[] 배열 읽음
		fread(cho2_inverted, sizeof(unsigned char), MAX_INVFILE2_LEN, fp);	// cho2_inverted[] 배열 읽음
		fclose(fp);	
		printf("done.\n");
	}
	else {
		printf("초성 2,3 역인덱스 생성시작...\n");
		memset(c_cur, 0, sizeof(int)*MAX_3MOEUM_INDEX);		// 초기값은 반드시 0이어야 함.
		memset(c_cur2, 0, sizeof(int)*MAX_2MOEUM_INDEX);	// 초기값은 반드시 0이어야 함.
		cho3_start[0] = 0;
		cho2_start[0] = 0;

		for (m = 1; m < MAX_3MOEUM_INDEX; m++) {	// 모든 가상의 3연속초성 ID들에 대해
			cho3_start[m] = cho3_start[m-1] + cho3_count[m-1]*3;
			if ((cho3_start[m] + cho3_count[m]*3) > max_cho3_start)
				max_cho3_start = cho3_start[m] + cho3_count[m]*3;	// cho3_inverted[] 배열의 최대 길이를 알아낸다.
			if (cho3_count[m] > max_cho3_count)
				max_cho3_count = cho3_count[m];	// 가장 줄번호를 많이 가지고 있는 3연속초성의 줄번호 개수를 알아낸다.
		}
		for (m = 1; m < MAX_2MOEUM_INDEX; m++) {	// 모든 가상의 2연속초성 ID들에 대해
			cho2_start[m] = cho2_start[m-1] + cho2_count[m-1]*3;
			if ((cho2_start[m]+cho2_count[m]) > max_cho2_start)
				max_cho2_start = cho2_start[m]+cho2_count[m];	// cho2_inverted[] 배열의 최대 길이를 알아낸다.
			if (cho2_count[m] > max_cho2_count)
				max_cho2_count = cho2_count[m];	// 가장 줄번호를 많이 가지고 있는 2연속초성의 줄번호 개수를 알아낸다.
		}
		if (max_cho3_count > max_cho2_count) {
			MAX_FORMER_LATER_CNT = max_cho3_count;
			MIN_FORMER_LATER_CNT = max_cho2_count;
		}
		else {
			MAX_FORMER_LATER_CNT = max_cho2_count;
			MIN_FORMER_LATER_CNT = max_cho3_count;
		}

		//printf("\tcho3_inverted[]의 크기: %s\n\tcho2_inverted[]의 크기: %s\n", Int2Currency(max_cho3_start, currency, false), Int2Currency(max_cho2_start, currency2, false));
		//printf("\tcho3_count[]의 최대값 = %s\n\tcho2_count[]의 최대값 = %s\n", Int2Currency(max_cho3_count, currency, false), Int2Currency(max_cho2_count, currency2, false));
		//printf("\tMAX_FORMER_LATER_CNT = %s\n\tMIN_FORMER_LATER_CNT = %s\n", Int2Currency(MAX_FORMER_LATER_CNT, currency, false), Int2Currency(MIN_FORMER_LATER_CNT, currency2, false));

		MAX_INVFILE3_LEN = max_cho3_start;
		cho3_inverted = (unsigned char *)malloc(MAX_INVFILE3_LEN);
		MAX_INVFILE2_LEN = max_cho2_start;
		cho2_inverted = (unsigned char *)malloc(MAX_INVFILE2_LEN);
		if (cho3_inverted == NULL || cho2_inverted == NULL) {
			fprintf(stderr, "\tcho3_inverted[] 또는 cho2_inverted[] 배열 할당에 실패했습니다. 이는 심각한 문제입니다.\n");
			exit(-4);
		}

		for (z = 0; z < db_line_count; z++) {		// 모든 줄에 대해서
			cur = position_in_memory[z];	// memory[] 배열 내에서 이 줄의 시작 위치
			c_count = 0;
			c_ad_count = 0;

			for (k = 0; k < namepart_len[z];) {
				han[0] = memory_db[cur++];
				if ((unsigned short)han[0] >= 128) {
					han[1] = memory_db[cur++];
					han[2] = '\0';
					HangulCodeConvert(KS, han, c_han);		// 완성형 한글코드 16비트를 조합형 16비트로 바꾼다.
					johapcode = ((unsigned char)c_han[0] << 8) | (unsigned char)c_han[1];
					cho_code = (johapcode >> 10) & 0x1f;	// 하위 10비트를 없애서 초성 코드를 추출한다.
					cho[c_count++] = cho_code;
					k += 2;									// 한글이니까 2바이트 증가
				}
#ifdef	IGNORE_NON_HANGUL
				else
					k++;
#else
				else if (han[0] >= '0' && han[0] <= '9') {	// 숫자는 초성의 끝 코드 'ㅎ'(0x14)의 다음에 위치한다고 설정
					cho[c_count++] = han[0] - '0' + 0x15;
					k++;
				}
				else if (han[0] == 0)						// 메모리 디비에서 이름부분과 주소부분 끝에 널 문자를 더 넣었으므로 무시한다.
					k++;
				else {										// 숫자를 제외한 알파벳 등의 아스키 코드는 초성 빼내기 불가능! 초성 코드에 0을 넣어둔다.
					cho[c_count++] = 0;
					k++;
				}
#endif
				if (c_count >= MAX_MOEUM_CNT-1) {
					fprintf(stderr, "\tcho[] 배열에서 오버플로우가 발생했습니다.\n");
					exit(-3);
				}
			}
			// 디비 명칭 부분에 등장하는 초성들의 배열 완료

			for (w = 0; w < addrpart_len[z];) {
				han[0] = memory_db[cur++];
				if ((unsigned short)han[0] >= 128) {
					han[1] = memory_db[cur++];
					han[2] = '\0';
					HangulCodeConvert(KS, han, c_han);		// 완성형 한글코드 16비트를 조합형 16비트로 바꾼다.
					johapcode = ((unsigned char)c_han[0] << 8) | (unsigned char)c_han[1];
					cho_code = (johapcode >> 10) & 0x1f;	// 하위 10비트를 없애서 초성 코드를 추출한다.
					cho_ad[c_ad_count++] = cho_code;
					w += 2;									// 한글이니까 2바이트 증가
				}
#ifdef	IGNORE_NON_HANGUL
				else
					w++;
#else
				else if (han[0] >= '0' && han[0] <= '9') {	// 숫자
					cho_ad[c_ad_count++] = han[0] - '0' + 0x15;
					w++;
				}
				else if (han[0] == 0)						// 메모리 디비에서 이름부분과 주소부분 끝에 널 문자를 더 넣었으므로 무시한다.
					w++;
				else{
					cho_ad[c_ad_count++] = 0;				// 기타 아스키 코드는 초성 빼내기 불가능! 초성 코드에 0을 넣어둔다.
					w++;
				}
#endif
				if (c_ad_count >= MAX_MOEUM_CNT-1) {
					fprintf(stderr, "\tcho_ad[] 배열에서 오버플로우가 발생했습니다.\n");
					exit(-3);
				}
			}
			// 디비 주소 부분에 등장하는 초성들의 배열 완료
		
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////		
			if (c_count == 1) {	// 이 줄의 초성 개수가 1이면, 억지로라도 3연속초성 ID와 2연속초성 ID를 만들고
								// 거기에 해당하는 inverted[]에 줄번호 24비트를 넣는다.
				c_index = cho[0]*38*38;
				inverted_index = cho3_start[c_index] + c_cur[c_index]*3;
				if (inverted_index >= MAX_INVFILE3_LEN-2) {
					fprintf(stderr, "줄번호 %d: cho3_inverted[] 배열에서 오버플로우가 발생했습니다(%u). 늘리세요.\n", z+1, inverted_index);
					exit(-2);
				}
				cho3_inverted[inverted_index] = (z >> 16) & 255; 
				cho3_inverted[inverted_index + 1] = (z >> 8) & 255;
				cho3_inverted[inverted_index + 2] = z & 255; 
				c_cur[c_index]++;

				c_index = cho[0]*38;
				inverted2_index = cho2_start[c_index] + c_cur2[c_index]*3;
				if (inverted2_index >= MAX_INVFILE2_LEN-2) {
					fprintf(stderr, "줄번호 %d: cho2_inverted[] 배열에서 오버플로우가 발생했습니다(%u). 늘리세요.\n", z+1, inverted2_index);
					exit(-2);
				}
				cho2_inverted[inverted2_index] = (z >> 16) & 255; 
				cho2_inverted[inverted2_index + 1] = (z >> 8) & 255; 
				cho2_inverted[inverted2_index + 2] = z & 255;
				c_cur2[c_index]++;
			}
			else if (c_count == 2) {	// 이 줄의 연속 모음 개수가 2이면, 억지로라도 3연속모음 ID을 만들어 여기에 해당하는 inverted[]에 줄번호를 넣고
										// 2연속모음 ID는 당연히 잘 만들어 지니 만들고 거기에 해당하는 inverted[]에 줄번호 24비트를 넣는다.
				c_index = cho[0]*38*38 + cho[1]*38;
				inverted_index = cho3_start[c_index] + c_cur[c_index]*3;
				if (inverted_index >= MAX_INVFILE3_LEN-2) {
					fprintf(stderr, "줄번호 %d: cho3_inverted[] 배열에서 오버플로우가 발생했습니다(%u). 늘리세요.\n", z+1, inverted_index);
					exit(-2);
				}
				cho3_inverted[inverted_index] = (z >> 16) & 255; 
				cho3_inverted[inverted_index + 1] = (z >> 8) & 255; 
				cho3_inverted[inverted_index + 2] = z & 255; 
				c_cur[c_index]++;	

				c_index = cho[0]*38 + cho[1];
				inverted2_index = cho2_start[c_index] + c_cur2[c_index]*3;
				if (inverted2_index >= MAX_INVFILE2_LEN-2) {
					fprintf(stderr, "줄번호 %d: cho2_inverted[] 배열에서 오버플로우가 발생했습니다(%u). 늘리세요.\n", z+1, inverted2_index);
					exit(-2);
				}
				cho2_inverted[inverted2_index] = (z >> 16) & 255; 
				cho2_inverted[inverted2_index + 1] = (z >> 8) & 255; 
				cho2_inverted[inverted2_index + 2] = z & 255; 
				c_cur2[c_index]++;	
			}
			else if (c_count >= 3) {	// 이 줄의 연속 초성 개수가 세 개 이상이면, 모든 3연속 초성 조합과 모든 2연속 초성 조합들의 ID를 계산하고, inverted[] 내의 그 자리에
					// 줄번호 24비트를 넣는다.
				for (i = 0; i < c_count-2; i++) {	// 모든 3연속초성 조합 등장의 줄 번호를 inverted[] 배열에 넣는다.
					c_index = cho[i]*38*38 + cho[i+1]*38 + cho[i+2];
					inverted_index = cho3_start[c_index] + c_cur[c_index]*3;
					if (inverted_index >= MAX_INVFILE3_LEN-2) {
						fprintf(stderr, "줄번호 %d: cho3_inverted[] 배열에서 오버플로우가 발생했습니다(%u). 늘리세요.\n", z+1, inverted_index);
						exit(-2);
					}
					cho3_inverted[inverted_index] = (z >> 16) & 255; 
					cho3_inverted[inverted_index + 1] = (z >> 8) & 255; 
					cho3_inverted[inverted_index + 2] = z & 255; 
					c_cur[c_index]++;
				}
				for (i = 0; i < c_count-1 ; i++) {	// 모든 2연속초성 조합 등장의 줄 번호를 inverted[] 배열에 넣는다.
					c_index = cho[i]*38 + cho[i+1];
					inverted2_index = cho2_start[c_index] + c_cur2[c_index]*3;
					if (inverted2_index >= MAX_INVFILE2_LEN-2) {
						fprintf(stderr, "줄번호 %d: cho2_inverted[] 배열에서 오버플로우가 발생했습니다(%u). 늘리세요.\n", z+1, inverted2_index);
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
			// 주소 부분에 대해서도 똑같은 방식으로 역파일을 만든다.
			if (c_ad_count == 1) {
				c_index = cho_ad[0]*38*38;
				inverted_index = cho3_start[c_index] + c_cur[c_index]*3;
				if (inverted_index >= MAX_INVFILE3_LEN-2) {
					fprintf(stderr, "줄번호 %d: cho3_inverted[] 배열에서 오버플로우가 발생했습니다(%u). 늘리세요.\n", z+1, inverted_index);
					exit(-2);
				}
				cho3_inverted[inverted_index] = (z >> 16) & 255; 
				cho3_inverted[inverted_index + 1] = (z >> 8) & 255; 
				cho3_inverted[inverted_index + 2] = z & 255; 
				c_cur[c_index]++;

				c_index = cho_ad[0]*38;
				inverted2_index = cho2_start[c_index] + c_cur2[c_index]*3;
				if (inverted2_index >= MAX_INVFILE2_LEN-2) {
					fprintf(stderr, "줄번호 %d: cho2_inverted[] 배열에서 오버플로우가 발생했습니다(%u). 늘리세요.\n", z+1, inverted2_index);
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
					fprintf(stderr, "줄번호 %d: cho3_inverted[] 배열에서 오버플로우가 발생했습니다(%u). 늘리세요.\n", z+1, inverted_index);
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
					fprintf(stderr, "줄번호 %d: cho2_inverted[] 배열에서 오버플로우가 발생했습니다(%u). 늘리세요.\n", z+1, inverted2_index);
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
						fprintf(stderr, "줄번호 %d: cho3_inverted[] 배열에서 오버플로우가 발생했습니다(%u). 늘리세요.\n", z+1, inverted_index);
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
						fprintf(stderr, "줄번호 %d: cho2_inverted[] 배열에서 오버플로우가 발생했습니다(%u). 늘리세요.\n", z+1, inverted2_index);
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

		// 생성된 초성검색용 역인덱스와 관련 정보를 파일에 저장해둔다.
		fp = fopen(chosung_inverted_file_name, "wb");	// 완성형용 역파일을 파일에 저장하여 담부턴 그냥 읽어들일 수 있도록 한다
		if (fp == NULL) {
			fprintf(stderr, "초성용 역파일 저장을 위한 파일 생성 실패!\n");
			exit(-6);
		}

		printf("Saving the 초성용 inverted file...");

		fwrite(&MAX_FORMER_LATER_CNT, sizeof(unsigned int), 1, fp);		// MAX_FORMER_LATER_CNT 변수 저장
		fwrite(&MIN_FORMER_LATER_CNT, sizeof(unsigned int), 1, fp);		// MIN_FORMER_LATER_CNT 변수 저장
		fwrite(&MAX_INVFILE3_LEN, sizeof(unsigned int), 1, fp);			// MAX_INVFILE3_LEN 변수 저장
		fwrite(&MAX_INVFILE2_LEN, sizeof(unsigned int), 1, fp);			// MAX_INVFILE2_LEN 변수 저장
		fwrite(cho3_start, sizeof(unsigned int), MAX_3MOEUM_INDEX, fp);	// cho3_start[] 배열 저장
		fwrite(cho2_start, sizeof(unsigned int), MAX_2MOEUM_INDEX, fp);	// cho2_start[] 배열 저장
		fwrite(cho3_inverted, sizeof(unsigned char), MAX_INVFILE3_LEN, fp);	// cho3_inverted[] 배열 저장
		fwrite(cho2_inverted, sizeof(unsigned char), MAX_INVFILE2_LEN, fp);	// cho2_inverted[] 배열 저장
		fclose(fp);	
		printf("done.\n");
	}

	// 나중에 검색 루틴에서 사용할 줄번호 저장 배열을 지금 할당한다.
	former_list = (int *)malloc(sizeof(int)*MAX_FORMER_LATER_CNT);
	latter_list = (int *)malloc(sizeof(int)*MAX_FORMER_LATER_CNT);
	if (former_list == NULL || latter_list == NULL) {
		fprintf(stderr, "\tformer_list[] 또는 later_list[] 할당에 실패했습니다. 이는 심각한 것 입니다.\n");
		exit(-7);
	}

	// 초성 검색 시 사용할 cho_final[MAX_QUERY_LEN/2][MIN_FORMER_LATER_CNT]를 미리 할당해둔다.
	cho_final = (int **)malloc(sizeof(int *)*MAX_QUERY_LEN/2);
	if (cho_final == NULL) {
		fprintf(stderr, "cho_final[] 배열 할당에 실패했습니다. 이는 심각한 것 입니다.\n");
		exit(-3);
	}
	for (i = 0; i < MAX_QUERY_LEN/2; i++) {
		cho_final[i] = (int *)malloc(sizeof(int)*MIN_FORMER_LATER_CNT);
		if (cho_final[i] == NULL) {
			fprintf(stderr, "cho_final[%d][] 배열 할당에 실패했습니다. 이는 심각한 것 입니다.\n", i);
			exit(-3);
		}
	}
}

/*
 * 입력 받은 쿼리 문자열 in[]에서 초성들만 추출하여 cho_query[]에 채운다.
 * 쿼리에 숫자가 들어오면 초성 코드로 시뮬레이션하여 cho_query[]에 채워준다.
 * 그러나 숫자가 아닌 알파벳이 들어오면 별도로 모아 query_alpha[]에 채워둔다.
 * extract_chosung_from_query(in_query, query_cho, query_alpha, &query_count, &query_alpha_count);
 */
void
extract_chosung_from_query(const char *in, short *in_cho, int *in_alpha, int *qc, int *qac)
{
	int in_count = 0;					// 추출한 초성의 개수
	int alpha_count = 0;				// 추출한 알파벳의 개수
	char han[HANGUL_BYTE+1];			// 한글 처리 시 글자 하나를 담아 놓기 위한 공간. NULL 문자까지 포함하기 위해 +1.
	char c_in[HANGUL_BYTE+1];			// 완성형 글자를 조합형으로 바꿀 때 사용하기 위해 선언.
	int i;
	unsigned short johapcode;			// 코드 변환 시 임시로 사용
	unsigned short cho_code;			// 초성 분리 시 임시로 사용

	in_count=0;

	for (i = 0; in[i] != '\0'; i++) {
		if ((unsigned int) in[i] >= 128) {
			han[0] = in[i];
			han[1] = in[i+1];
			han[2]='\0';
			//printf("0x%4x ", (short)*(short *)han);
			HangulCodeConvert(KS, han, c_in);
			johapcode = ((unsigned char)c_in[0] << 8) | (unsigned char)c_in[1]; 
			cho_code = (johapcode >> 10) & 0x1f;	// 초성 추출
			if (cho_code < 0x02 || cho_code > 0x14) {
				fprintf(stderr, "extract_chosung_from_query(): 쿼리에서 비정상 초성 글자 '%s' 발견! 무시합니다.\n", han);
				i++;
				continue;
			}
			in_cho[in_count++] = cho_code;
			i++;						
		}
		// 모음 버전에서는 한글 이외의 쿼리는 처리 안 함
#ifndef IGNORE_NON_HANGUL
		else if (in[i] >= '0' && in[i] <= '9')		// 숫자는 초성 5비트코드의 남는 뒷 부분을 활용
			in_cho[in_count++] = in[i] - '0' + 0x15;
		else if ('a' <= in[i] && in[i] <= 'z') {	// 소문자 알파벳
			in_cho[in_count++] = 0;
			in_alpha[alpha_count++] = in[i]-32;		// 대문자로 바꾸어 별도 공간에 저장저장
		} else {
			in_cho[in_count++] = 0;
			in_alpha[alpha_count++] = in[i];		// 알파벳은 별도 공간에 저장
		}
#else
		else if ('a' <= in[i] && in[i] <= 'z')	// 소문자 알파벳
			in_alpha[alpha_count++] = in[i]-32;		// 대문자로 바꾸어 별도 공간에 저장저장
		else
			in_alpha[alpha_count++] = in[i];		// 알파벳은 별도 공간에 저장
#endif
	} // end of for (i = 0; in[i] != '\0'; i++)

	//printf("\n조합형으로 변환된 쿼리: %d개(", in_count);
	//for (i = 0; i < in_count; i++)
	//	printf("0x%x ", in_cho[i]);
	//printf(")\n");

	*qc = in_count;		// query_count로 넘겨줌
	*qac = alpha_count;	// query_alpha_count로 넘겨줌
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


	//중복글자 문제 해결부분
	int Overlap_Search_perLine[MAX_ONELINE][2];//모두 한 레코드안에 중복여부를 확인하기 위한 변수들
	bool newchar_perLine = true;
	int overlapnum;//한 레코드에서 중복횟수

	int index_of_Overlap_Search_perLine= 0;
	for(int i=0; i<MAX_ONELINE; i++){
		Overlap_Search_perLine[i][0]=0;
		Overlap_Search_perLine[i][1]=0;
	}
	
	bool test_overlap_t_int = false;//DB 최대 중복 초과여부
	int test_overlap_count = 0;//DB 최대 중복 저장
	//중복글자 문제 해결부분끝


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
			
			//중복글자 문제 해결을 위해 주석처리한 부분
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
		else { // 블랭크를 포함한 일반 아스키 문자
			if (isalpha(t_char))
				t_char = toupper(t_char);
				eo_string[eo_count][0] = t_char;
				eo_string[eo_count][1] = NULL;
				// eo_string[eo_count][2] = '\0';
				
				t_int = (int)t_char;

				//중복글자 문제 해결을 위해 주석처리한 부분
				/*
				if (result = rbt2.search(t_int)) {
					index = result->index;
					eo_index[eo_count++] = index;
				}
				else {
					index = -1;
					//printf("'%s': 디비에 없는 문자. 무시함!\n", eo_string[eo_count]);
					//eo_index[eo_count++] = index;
				}//*/
		}
		
		//중복글자 문제 해결부분
		newchar_perLine = true;
		overlapnum = 0;
		for(int i=0; i<index_of_Overlap_Search_perLine; i++){
			if(Overlap_Search_perLine[i][0]==t_int){//중복문자일 경우
				/*//왜 이렇게 되는건지는 모르겠는데 DB의 중복 이상의 쿼리를 넣어줘도 정상적으로 나옴
				//라가 중복 3개가 최대라면 '라라라라'라고 입력했을때 다음 인덱스를 침범할줄 알았는데 침범하지 않음
				//오히려 이부분을 써주면 라 중복 3개인 레코드들이 차수가 4가되는데 안 써주면 차수가 3으로 나옴
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
					overlapnum = Overlap_Search_perLine[i][1]-1;//-1추가 => 에러해결
					break;
				//}
			}
		}
		
		if(newchar_perLine == true){//새로운 문자일 경우
			Overlap_Search_perLine[index_of_Overlap_Search_perLine][0]=t_int;
			Overlap_Search_perLine[index_of_Overlap_Search_perLine][1]++;
			index_of_Overlap_Search_perLine++;
		}
		//중복글자 문제 해결부분끝
		
		if (result = rbt2.search(t_int)) {	
				index = result->index;
				
					//중복글자 문제 해결부분
					index = index + overlapnum;
					//끝
				eo_index[eo_count++] = index;
			}
			else {
				index = -1;
				//printf("'%s' ignored!\n", eo_string[eo_count]);
			}//

		if (len <= 0)
			break;
	} // end of for (;;)
	//printf("search_inverted_file_4text(): 쿼리에 있는 문자 개수 = %d\n", eo_count);

	// 질문
	for (i = 0; i < eo_count; i++) {	// 쿼리에 포함된 각 문자에 대해 루프를 돈다.
		index = eo_index[i];	// 쿼리에 포함된 각 문자들의 RB 트리 내 인덱스를 가져온다.
		
		searched_count[i] = 0;
		if (index == -1)			// 디비에 없는 글자가 쿼리에 들어오면
			continue;
		for (int j = 0; j < i_count[index]; j++) {	// 이 문자가 디비에서 등장한 횟수 만큼 반복
			if ((unsigned int)searched_count[i] >= MAX_SEARCHED_COL) {
				printf("searched[%d][]의 열 개수가 모자랍니다. 줄 번호 넣을 자리가 없습니다. 늘리세요.\n", i);
				exit(-1);
			}
			// inverted 배열에서 이 글자가 들어 있는 디비 줄 번호 24비트(3 바이트)를 가져온다.
			searched[i][searched_count[i]] = 0;
			 
			t_int = (int)inverted[i_start[index] + 3*j]; // 역파일에서 줄번호 세바이트 중 최상위 byte 읽어 옴
			searched[i][searched_count[i]] += t_int << 16;
			 
			t_int = (int)inverted[i_start[index] + 3*j + 1]; 
			searched[i][searched_count[i]] += t_int << 8;

			t_int = (int)inverted[i_start[index] + 3*j + 2];
			searched[i][searched_count[i]++] += t_int;
			// 예를 들면 searched[0][0] == 37, searched[0][1] == 90 이면,
			// '현대자동차'라는 쿼리에 대해, '현'이라는 글자가 나오는 첫 줄은 37이고, 두번째 줄은 90이라는 뜻임.
			// 또, searched[1][0] == 37이고, searched[1][1] = 900 이면, '대'라는 글자가 나오는 첫 줄은 37이고 두 번째 줄은 900 이라는 의미
		}
	}
	//printf("eo_string[][]의 크기 = %d\n", sizeof(eo_string));
	//memcpy((void *)eo_string_org, (const void *)eo_string, sizeof(eo_string));	// 등장횟수 오름차순으로 정렬하기 전에 쿼리 문자열 원본을 복사해둔다.
	//quickSort2(searched_count, 0, eo_count-1);
	//printf("등장 횟수 오름차순 정렬: ");
	//for (i = 0; i < eo_count; i++)
	//	printf("(%s, %d) ", eo_string[i], searched_count[i]);
	//printf("\n");

	// 쿼리 길이이에 따라 다음과 같이 처리한다.
	/*
	 * 쿼리 길이 <= 2 : 두 글자 모두 포함하는 줄 번호 찾고 끝.
	 * 쿼리 길이 >= 3 && <= 5 : 쿼리를 두 chunk로 나눠 처리.
	 * 쿼리 길이 >= 6 : 쿼리를 세 chunk로 나눠 처리.
	 */
	// 1st chunk 검색 시작: 쿼리의 첫 번째 글자와 두 번째 글자가 모두 등장하는 줄번호 들을 찾는다.
	max_final = 0;
	idx0 = 0;
	idx1 = 1;
	// search[0][...]과 search[1][...]에 들어 있는 줄 번호들은 모두 정렬되어 있어야 함.
	i = j = 0;
	if (searched_count[idx0] != 0 && searched_count[idx1] != 0) {	// 쿼리의 첫번째 글자와 두번째 글자가 등장하는 줄들의 개수가 0이 아니면,
 		for (;;) {
			if (searched[idx0][i] > searched[idx1][j])
				j++;
			else if (searched[idx0][i] < searched[idx1][j])
				i++;
			else {
				if ((unsigned int)final_count[max_final] >= MAX_SEARCHED_COL) {
					printf("final[%d][]의 열 개수가 모자랍니다. 줄 번호 넣을 자리가 없습니다. 늘리세요.\n", max_final);
					exit(-1);
				}
				final[max_final][final_count[max_final]++] = searched[idx0][i];
				i++;
				j++;
			}
			if (i == searched_count[idx0] || j == searched_count[idx1])	// 쿼리의 첫 번째 글자와 두 번째 글자 중 어느 것이라도 그 등장 줄 번호를 다 봤다면
				break;
		}
	}
	else {
		//fprintf(stderr, "디비에 없는 문자가 입력되었거나 쿼리의 길이가 두 글자 미만입니다. 검색할 수 없습니다.\n");
		return(-1);
	}
	//printf("search_inverted_file_4text(): 첫 두 글자 '%s', '%s' 모두 등장 줄 개수 = %d, max_final = %d.\n", eo_string[idx0], eo_string[idx1], final_count[max_final], max_final);

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
						printf("final[%d][]의 열 개수가 모자랍니다. 줄 번호 넣을 자리가 없습니다. 늘리세요.\n", max_final);
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
			//printf("1st chunk %d번째 글자 '%s'도 등장하는 줄 개수 = %d. max_final = %d\n", idx0+1, eo_string[idx0], final_count[max_final], max_final);
		}
		else
			//printf("1st chunk %d번째 글자 '%s'도 등장하는 줄 개수 = %d. max_final은 여전히 %d.\n", idx0+1, eo_string[idx0], final_count[max_final], max_final)
			;
	}
	first_max_final = max_final;	// 1st chunk 검색에서 쿼리의 첫번째 덩어리에 있는 문자들이 등장하는 줄 번호들을 갖고 있는 final[] 내의 위치를 기억해둠.

	if (eo_count < 3) {		// 쿼리 길이가 세글자보다 작으면 2nd, 3rd 청크에 대한 검색을 할 필요가 없음
		if (eo_count == (max_final+2))	// 두 글자짜리 완전 포함 줄번호들이 발견됨.
			return 0;
		else
			return -1;
	}

	// 2nd chunk 검색 시작: 쿼리의 두번째 청크가 (모두) 등장하는 줄번호 들을 찾는다.
	max_final++;
	if (eo_count == 3) {	// 쿼리 길이가 3이면 뒤 두 글자를 대상으로...
		idx0 = eo_count - 2;
		idx1 = eo_count - 1;
	}
	else if (eo_count >= 4) {	// 4 이상이면 두번째 청크 처음 글자와 그 다음 글자를 대상으로...
		idx0 = first_chunk_eo_count;
		idx1 = first_chunk_eo_count + 1;
	}
	// search[idx0][...]과 search[idx1][...]에 들어 있는 줄 번호들은 모두 정렬되어 있어야 함.
	i = j = 0;
	if (searched_count[idx0] != 0 && searched_count[idx1] != 0) {	// 두 번째 청크의 첫번째 글자와 두번째 글자가 등장하는 줄들의 개수가 0이 아니면,
 		for (;;) {
			if (searched[idx0][i] > searched[idx1][j])
				j++;
			else if (searched[idx0][i] < searched[idx1][j])
				i++;
			else {
				if ((unsigned int)final_count[max_final] >= MAX_SEARCHED_COL) {
					printf("final[%d][]의 열 개수가 모자랍니다. 줄 번호 넣을 자리가 없습니다. 늘리세요.\n", max_final);
					exit(-1);
				}
				final[max_final][final_count[max_final]++] = searched[idx0][i];
				i++;
				j++;
			}
			if (i == searched_count[idx0] || j == searched_count[idx1])	// 쿼리의 첫 번째 글자와 두 번째 글자 중 어느 것이라도 그 등장 줄 번호를 다 봤다면
				break;
		}
	}
	else {
		//fprintf(stderr, "디비에 없는 문자가 입력되었거나 쿼리의 길이가 두 글자 미만입니다. 검색할 수 없습니다.\n");
		return(-1);
	}
	//printf("2nd chunk 두 글자 '%s', '%s' 모두 등장 줄 개수 = %d, max_final = %d.\n", eo_string[idx0], eo_string[idx1], final_count[max_final], max_final);

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
						printf("final[%d][]의 열 개수가 모자랍니다. 줄 번호 넣을 자리가 없습니다. 늘리세요.\n", max_final);
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
			//printf("%d번째 글자 '%s'도 등장하는 줄 개수 = %d. max_final = %d\n", idx0+1, eo_string[idx0], final_count[max_final], max_final);
		}
		else
			//printf("%d번째 글자 '%s'도 등장하는 줄 개수 = %d. max_final은 여전히 %d.\n", idx0+1, eo_string[idx0], final_count[max_final], max_final)
			;
	}
	second_max_final = max_final;	// 2nd chunk 검색에서 2nd chunk 문자들이 등장하는 줄 번호들을 갖고 있는 final[] 내의 위치를 기억해둠.
	
	if (eo_count == 3) {	// 쿼리 길이가 3이면, 마지막 글자와 첫번째 글자가 모두 등장한 3rd chunk에 대해 검색해야 함.
		idx0 = 0;
		idx1 = 2;
	}
	else if (eo_count == 4) {	// 쿼리 길이가 4이면, 1st Chunk와 2nd Chunk에서 한글자씩 골라 3rd chunk로 삼아 검색해야 함.
		idx0 = 1;	// 0도 가능
		idx1 = 2;	// 3도 가능. 실행 시간 관계상 모든 조합을 다 해볼 수는 없음.
	}
	else if (eo_count == 5) {	// 쿼리 길이가 5이면, 1st Chunk와 2nd Chunk에서 한글자씩 골라 3rd chunk로 삼아 검색해야 함.
		idx0 = 2;	// 0이나 1도 가능
		idx1 = 3;	// 4도 가능. 실행 시간 관계상 모든 조합을 다 해볼 수는 없음.
	}
	else if (eo_count >= 6) {	// 쿼리 길이가 6 이상이면 아직 남아 있는 3rd chunk를 처리해야 함.
		idx0 = first_chunk_eo_count + second_chunk_eo_count;
		idx1 = idx0 + 1;
	}

	max_final++;
	// search[idx0][...]과 search[idx1][...]에 들어 있는 줄 번호들은 모두 정렬되어 있어야 함.
	i = j = 0;
	if (searched_count[idx0] != 0 && searched_count[idx1] != 0) {	// 3rd chunk의 첫번째 글자와 두번째 글자가 등장하는 줄들의 개수가 0이 아니면,
 		for (;;) {
			if (searched[idx0][i] > searched[idx1][j])
				j++;
			else if (searched[idx0][i] < searched[idx1][j])
				i++;
			else {
				if ((unsigned int)final_count[max_final] >= MAX_SEARCHED_COL) {
					printf("final[%d][]의 열 개수가 모자랍니다. 줄 번호 넣을 자리가 없습니다. 늘리세요.\n", max_final);
					exit(-1);
				}
				final[max_final][final_count[max_final]++] = searched[idx0][i];
				i++;
				j++;
			}
			if (i == searched_count[idx0] || j == searched_count[idx1])	// 쿼리의 첫 번째 글자와 두 번째 글자 중 어느 것이라도 그 등장 줄 번호를 다 봤다면
				break;
		}
	}
	else {
		//fprintf(stderr, "디비에 없는 문자가 입력되었거나 쿼리의 길이가 두 글자 미만입니다. 검색할 수 없습니다.\n");
		return(-1);
	}
	//printf("3rd chunk 두 글자 '%s', '%s' 모두 등장 줄 개수 = %d, max_final = %d.\n", eo_string[idx0], eo_string[idx1], final_count[max_final], max_final);

	if (eo_count >= 7) {	// 쿼리가 7글자 이상이면 3rd Chunk에 아직 안 본 글자가 있을 것이므로 이를 처리한다. //120706
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
							printf("final[%d][]의 열 개수가 모자랍니다. 줄 번호 넣을 자리가 없습니다. 늘리세요.\n", max_final);
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
				printf("%d번째 글자 '%s'도 등장하는 줄 개수 = %d. max_final = %d\n", idx0+1, eo_string[idx0], final_count[max_final], max_final);
			}
			else
				printf("%d번째 글자 '%s'도 등장하는 줄 개수 = %d. max_final은 여전히 %d.\n", idx0+1, eo_string[idx0], final_count[max_final], max_final);
		}
	} // end of if (eo_count >= 6)
	third_max_final = max_final;	// 3rd chunk 검색에서 3rd chunk 문자들이 등장하는 줄 번호들을 갖고 있는 final[] 내의 위치를 기억해둠.

	/*
	 * 세글자 이상의 쿼리이면 이제 첫 절반이 등장한 줄번호들의 나머지 절반(쿼리의 뒷 부분)에 대한 차수를 구해야 한다.
	 * final[l2r_max_final][...]에는 쿼리의 첫 절반 문자열들이 등장한 줄번호들이 들어 있다.
	 * 현재 (l2r_max_final+2)가 final[l2r_max_final][...]에 들어 있는 각 줄번호들의 차수이다.
	 * 만약 l2r_max_final이 2였다면 final[l2r_max_final][...]에 있는 줄번호들의 차수는 4이다. 즉, 쿼리 앞 네글자가 (모두) 등장한 줄 번호들이 구해져 있는 것이다.
	 * 
	 * 
	 */
	if (first_final_degree != NULL) free(first_final_degree);
	first_final_degree = (int *)calloc(final_count[first_max_final], sizeof(int));	// 쿼리 1st Chunk가 모두 등장한 줄번호들의 차수를 저장할 배열을 동적 할당. 나중에 free 시켜야 함.
	if (first_final_degree == NULL) {
		fprintf(stderr, "first_final_degree[] 배열 할당에 실패했습니다. 종료합니다.\n");
		exit(-13);
	}

	if (second_final_degree != NULL) free(second_final_degree);
	second_final_degree = (int *)calloc(final_count[second_max_final], sizeof(int));	// 쿼리 2nd Chunk가 모두 등장한 줄번호들의 차수를 저장할 배열을 동적 할당. 나중에 free 시켜야 함.
	if (second_final_degree == NULL) {
		fprintf(stderr, "second_final_degree[] 배열 할당에 실패했습니다. 종료합니다.\n");
		exit(-13);
	}
	//printf("first_max_final = %d, second_max_final = %d.\n", first_max_final, second_max_final);

	if (merged_final_degree != NULL) free(merged_final_degree);
	merged_final_degree = (int *)calloc(final_count[first_max_final]+final_count[second_max_final], sizeof(int));
	if (merged_final_degree == NULL) {
		fprintf(stderr, "merged_final_degree[] 배열 할당에 실패했습니다. 종료합니다.\n");
		exit(-13);
	}

	if (third_final_degree != NULL) free(third_final_degree);
	third_final_degree = (int *)calloc(final_count[third_max_final], sizeof(int));	// 쿼리 3rd Chunk가 모두 등장한 줄번호들의 차수를 저장할 배열을 동적 할당. 나중에 free 시켜야 함.
	if (third_final_degree == NULL) {
		fprintf(stderr, "third_final_degree[] 배열 할당에 실패했습니다. 종료합니다.\n");
		exit(-13);
	}
	//printf("third_max_final = %d.\n", third_max_final);

	if (merged2_final_degree != NULL) free(merged2_final_degree);
	merged2_final_degree = (int *)calloc(final_count[first_max_final]+final_count[second_max_final]+final_count[third_max_final], sizeof(int));
	if (merged2_final_degree == NULL) {
		fprintf(stderr, "merged2_final_degree[] 배열 할당에 실패했습니다. 종료합니다.\n");
		exit(-13);
	}
		
	for (i = 0; i < final_count[first_max_final]; i++)						// 1st chunk 문자들이 (모두)등장한 줄번호들의 차수를 초기화 한다.
		first_final_degree[i] = (first_max_final + 2);
	for (i = 0; i < final_count[second_max_final]; i++)						// 2nd chunk 문자들이 (모두)등장한 줄번호들의 차수를 초기화 한다.
		second_final_degree[i] = second_max_final - first_max_final + 1;
	if (eo_count >= 3 && eo_count <= 5) {
		for (i = 0; i < final_count[third_max_final]; i++)					// 3rd chunk 문자들이 (모두)등장한 줄번호들의 차수를 초기화 한다.
			third_final_degree[i] = 2;
	}
	else if (eo_count >= 6) {
		for (i = 0; i < final_count[third_max_final]; i++)					// 3rd chunk 문자들이 (모두)등장한 줄번호들의 차수를 초기화 한다.
			third_final_degree[i] = third_max_final - second_max_final + 1;
	}

	// 1st chunk와 그 이외의 문자들 사이의 차수를 계산한다.
	// 이전 3중루프 버전에 비해 지퍼 알고리즘을 사용하여 속도를 높인다.
	for (j = first_chunk_eo_count; j < eo_count; j++) {		// 쿼리 2nd chunk에 있는 모든 글자들 각각에 대해
		i = k = 0;
		if (final_count[first_max_final] != 0 && searched_count[j] != 0) {	
 			for (;;) {
				if (searched[j][k] > final[first_max_final][i]) i++;		// 줄번호 작은 쪽을 전진시킴
				else if (searched[j][k] < final[first_max_final][i]) k++;	// 줄번호 작은 쪽을 전진시킴
				else {														// 같은 줄번호가 발견됨. 차수를 증가시키고 둘 다 전진시킴.
					first_final_degree[i]++;
					i++; k++;
				}
				if (i == final_count[first_max_final] || k == searched_count[j])	// 어느 한 쪽이 끝을 만났으면 빠져나간다.
					break;
			} // end of for (;;)
		}
	}
	//printf("1st chunk와 그 이외의 문자들 사이의 차수 계산 완료.\n");

	// 2nd chunk와 1st chunk 문자들 사이의 차수를 계산한다.
	/*
	trouble_warning = 0;
	if (final_count[second_max_final] > TROUBLE_COUNT)
		trouble_warning = 1;
	if (eo_count == 3) s = 2;	// 쿼리 길이가 3이면 2:1 그리고 1:2 이렇게 자르기 때문에...
	else s = 1;
	//printf("final_count[second_max_final] = %d, then %d ~ %d.\n", final_count[second_max_final], first_chunk_eo_count-s, 0);
	for (i = 0; i < final_count[second_max_final]; i++) {		// 쿼리 2nd chunk 글자들이 (모두)등장한 각 줄번호들에 대해
		for (j = first_chunk_eo_count-s; j >= 0; j--) {			// 쿼리 1st chunk에 있는 모든 글자들 각각에 대해
			if (trouble_warning && (searched_count[j] > TROUBLE_COUNT))
				continue;
			for (k = 0; k < searched_count[j]; k++) {			// 쿼리 1st chunk에 있는 글자가 2nd chunk에도 등장한 모든 줄번호들을 찾는다.
				if (final[second_max_final][i] == searched[j][k]) {	// 줄번호가 같은 것 발견. 그 줄번호의 차수를 1 증가시킨다.
					second_final_degree[i]++;
					break;
				}
				if (final[second_max_final][i] < searched[j][k])	// 이 뒤는 더 볼 필요가 없으므로
					break;
			}
		}
	}
	*/
	/*
	 * 2nd chunk와 1st chunk 문자들 사이의 차수를 계산한다.
	 * 이전 3중루프 버전에 비해 지퍼 알고리즘을 사용하여 속도를 높인다.
	 */
	if (eo_count == 3) s = 2;	// 쿼리 길이가 3이면 2:1 그리고 1:2 이렇게 자르기 때문에...
	else s = 1;
	for (j = first_chunk_eo_count-s; j >= 0; j--) {			// 쿼리 1st chunk에 있는 모든 글자들 각각에 대해
		i = k = 0;
		if (final_count[second_max_final] != 0 && searched_count[j] != 0) {	
 			for (;;) {
				if (searched[j][k] > final[second_max_final][i]) i++;		// 줄번호 작은 쪽을 전진시킴
				else if (searched[j][k] < final[second_max_final][i]) k++;	// 줄번호 작은 쪽을 전진시킴
				else {														// 같은 줄번호가 발견됨. 차수를 증가시키고 둘 다 전진시킴.
					second_final_degree[i]++;
					i++; k++;
				}
				if (i == final_count[second_max_final] || k == searched_count[j])	// 어느 한 쪽이 끝을 만났으면 빠져나간다.
					break;
			} // end of for (;;)
		}
	}
	//printf("2nd chunk와 1st chunk 문자들 사이의 차수 계산 완료.\n");
	// 이제 3rd Chunk까지 처리하러 가자.
	if (eo_count == 3) {
		// 3rd Chunk와 나머지 문자들 간의 차수를 계산한다.
		j = 1;		// 3rd chunk에 속해 있지 않은 딱 한개의 문자의 인덱스.
		i = k = 0;
		if (final_count[third_max_final] != 0 && searched_count[j] != 0) {	
 			for (;;) {
				if (searched[j][k] > final[third_max_final][i]) i++;		// 줄번호 작은 쪽을 전진시킴
				else if (searched[j][k] < final[third_max_final][i]) k++;	// 줄번호 작은 쪽을 전진시킴
				else {														// 같은 줄번호가 발견됨. 차수를 증가시키고 둘 다 전진시킴.
					third_final_degree[i]++;
					i++; k++;
				}
				if (i == final_count[third_max_final] || k == searched_count[j])	// 어느 한 쪽이 끝을 만났으면 빠져나간다.
					break;
			} // end of for (;;)
		}
	}
	else if (eo_count == 4) {
		// 3rd Chunk와 나머지 문자들 간의 차수를 계산한다.
		for (j = 0; j < eo_count; j+=3) {			// 쿼리 3rd chunk에 속해 있지 않은 모든 글자들 각각에 대해(인덱스 0과 3에 있음)
			i = k = 0;
			if (final_count[third_max_final] != 0 && searched_count[j] != 0) {	
 				for (;;) {
					if (searched[j][k] > final[third_max_final][i]) i++;		// 줄번호 작은 쪽을 전진시킴
					else if (searched[j][k] < final[third_max_final][i]) k++;	// 줄번호 작은 쪽을 전진시킴
					else {														// 같은 줄번호가 발견됨. 차수를 증가시키고 둘 다 전진시킴.
						third_final_degree[i]++;
						i++; k++;
					}
					if (i == final_count[third_max_final] || k == searched_count[j])	// 어느 한 쪽이 끝을 만났으면 빠져나간다.
						break;
				} // end of for (;;)
			}
		}
	}
	else if (eo_count == 5) {
		// 3rd Chunk와 나머지 문자들 간의 차수를 계산한다.
		for (j = 0; j < eo_count; j++) {			// 쿼리 3rd chunk에 속해 있지 않은 모든 글자들 각각에 대해(인덱스 0, 1, 4에 있음)
			if (j == 2 || j == 3)
				continue;
			i = k = 0;
			if (final_count[third_max_final] != 0 && searched_count[j] != 0) {	
 				for (;;) {
					if (searched[j][k] > final[third_max_final][i]) i++;		// 줄번호 작은 쪽을 전진시킴
					else if (searched[j][k] < final[third_max_final][i]) k++;	// 줄번호 작은 쪽을 전진시킴
					else {														// 같은 줄번호가 발견됨. 차수를 증가시키고 둘 다 전진시킴.
						third_final_degree[i]++;
						i++; k++;
					}
					if (i == final_count[third_max_final] || k == searched_count[j])	// 어느 한 쪽이 끝을 만났으면 빠져나간다.
						break;
				} // end of for (;;)
			}
		}
	}
	else if (eo_count >= 6) {
		// 2nd chunk와 3rd chunk 문자들 사이의 차수를 계산한다.
		/*
		trouble_warning = 0;
		if (final_count[second_max_final] > TROUBLE_COUNT)
			trouble_warning = 1;
		for (i = 0; i < final_count[second_max_final]; i++) {							// 쿼리 2nd chunk 글자들이 (모두)등장한 각 줄번호들에 대해
			for (j = first_chunk_eo_count+second_chunk_eo_count; j < eo_count; j++) {	// 쿼리 3rd chunk에 있는 모든 글자들 각각에 대해
				if (trouble_warning && (searched_count[j] > TROUBLE_COUNT))
					continue;
				for (k = 0; k < searched_count[j]; k++) {								// 쿼리 3rd chunk에 있는 글자가 2nd chunk에도 등장한 모든 줄번호들을 찾는다.
					if (final[second_max_final][i] == searched[j][k]) {					// 줄번호가 같은 것 발견. 그 줄번호의 차수를 1 증가시킨다.
						second_final_degree[i]++;
						break;
					}
					if (final[second_max_final][i] < searched[j][k])	// 이 뒤는 더 볼 필요가 없으므로
						break;
				}
			}
		}
		*/
		/*
		 * 2nd chunk와 3rd chunk 문자들 사이의 차수를 계산한다.
		 * 이전 3중루프 버전에 비해 지퍼 알고리즘을 사용하여 속도를 높인다.
		 */
		for (j = first_chunk_eo_count+second_chunk_eo_count; j < eo_count; j++) {	// 쿼리 3rd chunk에 있는 모든 글자들 각각에 대해
			i = k = 0;
			if (final_count[second_max_final] != 0 && searched_count[j] != 0) {	
 				for (;;) {
					if (searched[j][k] > final[second_max_final][i]) i++;		// 줄번호 작은 쪽을 전진시킴
					else if (searched[j][k] < final[second_max_final][i]) k++;	// 줄번호 작은 쪽을 전진시킴
					else {														// 같은 줄번호가 발견됨. 차수를 증가시키고 둘 다 전진시킴.
						second_final_degree[i]++;
						i++; k++;
					}
					if (i == final_count[second_max_final] || k == searched_count[j])	// 어느 한 쪽이 끝을 만났으면 빠져나간다.
						break;
				} // end of for (;;)
			}
		}
		//printf("2nd chunk와 3rd chunk 문자들 사이의 차수 계산 완료.\n");

		// 3rd chunk와 그 이전 문자들 사이의 차수를 계산한다.
		/*
		trouble_warning = 0;
		if (final_count[third_max_final] > TROUBLE_COUNT)
			trouble_warning = 1;
		for (i = 0; i < final_count[third_max_final]; i++) {						// 쿼리 3rd chunk 글자들이 (모두)등장한 각 줄번호들에 대해
			for (j = first_chunk_eo_count+second_chunk_eo_count-1; j >= 0; j--) {	// 쿼리 앞 절반에 있는 모든 글자들 각각에 대해
				if (trouble_warning && (searched_count[j] > TROUBLE_COUNT))
					continue;
				for (k = 0; k < searched_count[j]; k++) {							// 쿼리 앞 절반에 있는 어떤 한 글자가 등장한 모든 줄번호들과 비교해보자.
					if (final[third_max_final][i] == searched[j][k]) {				// 줄번호가 같은 것 발견. 그 줄번호의 차수를 1 증가시킨다.
						third_final_degree[i]++;
						break;
					}
					if (final[third_max_final][i] < searched[j][k])	// 이 뒤는 더 볼 필요가 없으므로
						break;
				}
			}
		}
		*/
		/*
		 * 3rd chunk와 그 이전 문자들 사이의 차수를 계산한다.
		 * 이전 3중루프 버전에 비해 지퍼 알고리즘을 사용하여 속도를 높인다.
		 */
		for (j = first_chunk_eo_count+second_chunk_eo_count-1; j >= 0; j--) {	// 쿼리 앞 절반에 있는 모든 글자들 각각에 대해
			i = k = 0;
			if (final_count[third_max_final] != 0 && searched_count[j] != 0) {	
 				for (;;) {
					if (searched[j][k] > final[third_max_final][i]) i++;		// 줄번호 작은 쪽을 전진시킴
					else if (searched[j][k] < final[third_max_final][i]) k++;	// 줄번호 작은 쪽을 전진시킴
					else {														// 같은 줄번호가 발견됨. 차수를 증가시키고 둘 다 전진시킴.
						third_final_degree[i]++;
						i++; k++;
					}
					if (i == final_count[third_max_final] || k == searched_count[j])	// 어느 한 쪽이 끝을 만났으면 빠져나간다.
						break;
				} // end of for (;;)
			}
		}
	}
	//printf("3rd chunk와 그 이전 문자들 사이의 차수 계산 완료.\n");
	//printf("second_final_degree[] 생성 완료\n");
	//for (i = 0; i < final_count[r2l_max_final]; i++)
	//	printf("%d ", second_final_degree[i]);
	//printf("\n");
	// 차수기준 정렬 전에 first와 second를 합친다. 그러면서 겹치는 줄번호는 제거될 것이다.
	max_final++;
	i = j = 0;
	if (final_count[first_max_final] != 0 && final_count[second_max_final] != 0) {	// 쿼리의 첫번째 글자와 두번째 글자가 등장하는 줄들의 개수가 0이 아니면,
 		for (;;) {
			if ((unsigned int)final_count[max_final] >= MAX_SEARCHED_COL) {
				printf("final[%d][]의 열 개수가 모자랍니다. 줄 번호 넣을 자리가 없습니다. 늘리세요.\n", max_final);
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
			else {	// 줄번호가 같으면 차수가 높은 것을 선택
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
			// 남는 거 처리
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
		// 1st chunk와 2nd chunk 중 줄번호가 하나도 없는 경우 발견
		//printf("1st chunk와 2nd chunk 중 줄번호가 하나도 없는 덩어리가 있군요.\n");
		if (final_count[first_max_final] != 0) {		// 2nd chunk가 등장하는 줄번호가 없음. 1st chunk를 merged로 처리
			final_count[max_final] = final_count[first_max_final];
			memcpy(final[max_final], final[first_max_final], sizeof(int)*final_count[first_max_final]);
			memcpy(merged_final_degree, first_final_degree, sizeof(int)*final_count[first_max_final]);
			//printf("1st chunk를 merged로 복사하여 사용합니다.\n");
		}
		else if (final_count[second_max_final] != 0) {	// 1st chunk가 등장하는 줄번호가 없음. 2nd chunk를 merged로 처리
			final_count[max_final] = final_count[second_max_final];
			memcpy(final[max_final], final[second_max_final], sizeof(int)*final_count[second_max_final]);
			memcpy(merged_final_degree, second_final_degree, sizeof(int)*final_count[second_max_final]);
			//printf("2nd chunk를 merged로 복사하여 사용합니다.\n");
		}
		else {											// 1st & 2nd chunk가 모두 공집합임. merged는 비어있게 됨.
			// 아무것도 할 일 없음.
		}
	}
	//printf("degree 1차 merge 완료. max_final = %d, final_count[max_final] = %d.\n", max_final, final_count[max_final]);

	// merge를 한 번 더해야 함.
	merged_max_final = max_final;
	max_final++;
	i = j = 0;
	if (final_count[third_max_final] != 0 && final_count[merged_max_final] != 0) {	// 이제 first, second 합친 줄번호와 3rd chunk 줄번호들을 합친다.
 		for (;;) {
			if ((unsigned int)final_count[max_final] >= MAX_SEARCHED_COL) {
				fprintf(stderr, "final[%d][]의 열 개수가 모자랍니다. 줄 번호 넣을 자리가 없습니다. 늘리세요.\n", max_final);
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
			else {	// 줄번호가 같으면 차수가 높은 것을 선택
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
			// 남는 거 처리
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
		// 3rd chunk와 merged chunk 중 줄번호가 하나도 없는 경우 발견
		//printf("3rd chunk와 merged chunk 중 줄번호가 하나도 없는 덩어리가 있군요.\n");
		if (final_count[third_max_final] != 0) {		// merged chunk가 등장하는 줄번호가 없음. 3rd chunk를 merged2로 처리
			final_count[max_final] = final_count[third_max_final];
			memcpy(final[max_final], final[third_max_final], sizeof(int)*final_count[third_max_final]);
			memcpy(merged2_final_degree, third_final_degree, sizeof(int)*final_count[third_max_final]);
			//printf("3rd chunk를 merged2로 복사하여 사용합니다.\n");
		}
		else if (final_count[merged_max_final] != 0) {	// 3rd chunk가 등장하는 줄번호가 없음. merged chunk를 merged2로 처리
			final_count[max_final] = final_count[merged_max_final];
			memcpy(final[max_final], final[merged_max_final], sizeof(int)*final_count[merged_max_final]);
			memcpy(merged2_final_degree, merged_final_degree, sizeof(int)*final_count[merged_max_final]);
			//printf("merged chunk를 merged2로 복사하여 사용합니다.\n");
		}
		else {											// 1st & 2nd chunk가 모두 공집합임. merged는 비어있게 됨.
			// 아무것도 할 일 없음.
		}
	}
	//printf("degree merge 스텝 2 완료. max_final = %d, final_count[max_final] = %d.\n", max_final, final_count[max_final]);

	if (final_count[max_final] <= 0)
		return -1;

	// 차수 기준 내림차순으로 줄번호를 정렬한다.
	//quickSort3(merged2_final_degree, 0, final_count[max_final]-1);

	//////////////////////////////////////////////////////[JAY]120919에 바꿈
	/*
	i = 2;							// 최고 상위 두 개의 차수만 뽑아낸다.
	k = eo_count;					// 이론적 최고 차수는 쿼리 개수임.
	s = 0;
	e = final_count[max_final]-1;
	int prevj = -1;
	while (i && k) {
		j = partition4(merged2_final_degree, s, e, k);
		if (j >= s) {	// 차수 k 이상인 줄번호들이 있음.
			printf("차수 %d인 줄번호 %d개 partition 성공\n", k, j-prevj);
			prevj = j;
			i--;
			s = j+1;	// 차수가 k 미만인 부분에 대해서만 차수를 낮춰 다시 찾는다.
			if (s >= e)
				break;
		}
		k--;			// 차수를 하나 낮춤.
	};
	*/
	i = eo_count;					// 모든 차수를 뽑아낸다.
	k = eo_count;					// 이론적 최고 차수는 쿼리 개수임.
	s = 0;
	e = final_count[max_final]-1;
	int prevj = -1;

	//후보로 필터링할 최소차수(min_chasu) 저장하는 부분 - JAY
	int min_chasu = 0;
	if(eo_count<=4){
		min_chasu = 2;			//글자 최소 2개는 맞아야함
	}else{
		min_chasu = eo_count-2;	//글자 3개 틀린거까지는 봐줌
	}

	while (i && k) {
		j = partition4(merged2_final_degree, s, e, k);
		if (j >= s) {	// 차수 k 이상인 줄번호들이 있음.
			printf("차수 %d인 줄번호 %d개 partition 성공\n", k, j-prevj);
			if(k == min_chasu){
				if(j>18000){	//필터링된 데이터 개수가 18000이 넘으면 멈추므로, 전차수까지만 보도록 만든 부분 - JAY
					no_of_filtered_data = prevj;
					printf("2.필터링할 데이터개수 : %d, 최소차수 : %d\n", no_of_filtered_data, min_chasu);
				}else{
					no_of_filtered_data = j;
					printf("1.필터링할 데이터개수 : %d, 최소차수 : %d\n", no_of_filtered_data, min_chasu);
				}
			}
			prevj = j;
			i--;
			s = j+1;	// 차수가 k 미만인 부분에 대해서만 차수를 낮춰 다시 찾는다.
			if (s >= e){
				printf("\n[[%d, %d]]\n", s, final_count[max_final]-1);
				break;
			}
		}
		k--;			// 차수를 하나 낮춤.
	};
	////////////////////////////////////////////////////////
	printf("merged2_final_degree[] 내림차순 정렬 완료\n");
	if (merged2_final_degree[0] >= eo_count)
		return 1;
	else
		return 0;
}

/*
 * search_using_inverted_file_4chosung(query_cho, query_count, query_alpha, query_alpha_count);
 * 쿼리가 3글자 미만이면 문제가 발생하므로 쿼리 길이가 세글자 이상일 때에만 호출할 것
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
	char han[HANGUL_BYTE+1];			// 한글 처리 시 글자 하나를 담아 놓기 위한 공간. NULL 문자까지 포함하기 위해 +1.
	char c_han[HANGUL_BYTE+1];			// 완성형 글자를 조합형으로 바꿀 때 사용하기 위해 선언.
	int cur_final = 0;
	int cur_entry = 0;
	unsigned int a, b;

	if (in_count < 3) {
		former = in_eo[0]*38 + in_eo[1];
		cur = cho2_start[former];
		for (k = 0; k < cho2_count[former]; k++) {	// 이 첫 2연속초성이 등장하는 모든 줄번호에 대해
			cho_final[cur_final][cho_final_count[cur_final]++] = (cho2_inverted[cur]<<16) + (cho2_inverted[cur+1]<<8) + (cho2_inverted[cur+2]);	// 이 2연속초성의 등장 줄 번호들을 얻어온다.
			cur = cur + 3;			// 줄번호 하나가 3바이트(24비트)이니까
		}
	}
	else {
		former = in_eo[0]*38 + in_eo[1];											// 쿼리의 앞 첫 2연속초성ID를 계산한다.
		latter = in_eo[in_count-3]*38*38 + in_eo[in_count-2]*38 + in_eo[in_count-1];	// 쿼리의 뒤 첫 3연속초성ID를 계산한다.
		//printf("former: %d , latter : %d\n",former, latter);
		//printf("former_c: %d , latter_c : %d\n",cho2_count[former], cho3_count[latter]);
	
		cur = cho2_start[former];		// 쿼리의 첫 2연속초성의 등장 줄번호 첫 위치를 알아낸다.

		//printf("cur_final = %d\n", cur_final);
		for (k = 0; k < cho2_count[former]; k++) {	// 이 첫 2연속초성이 등장하는 모든 줄번호에 대해
			former_list[k] = (cho2_inverted[cur]<<16) + (cho2_inverted[cur+1]<<8) + (cho2_inverted[cur+2]);	// 이 2연속초성의 등장 줄 번호들을 얻어온다.
			cur = cur + 3;			// 줄번호 하나가 3바이트(24비트)이니까
			//printf("%d <= (%d)", former_list[k], cur-3);
			//printf("%d ",former_list[k]);
		}
		//printf("former_list[]에 %d개.\n", cho2_count[former]);

		cur = cho3_start[latter];		// 쿼리의 뒤 3연속초성의 등장 줄번호 첫 위치를 알아낸다.
		for (k = 0; k < cho3_count[latter]; k++) {	// 이 뒤 3연속초성이 등장하는 모든 줄번호에 대해
			a = 0; b = 0;

			b = (int)cho3_inverted[cur];
			a = b << 16;
			b = (int)cho3_inverted[cur+1];
			a += b << 8;
			b = (int)cho3_inverted[cur+2];
			a += b;
			latter_list[k] = a;		// 이 3연속초성의 등장 줄 번호들을 얻어온다.
			cur = cur + 3;
		//	fprintf(p_file2, "%d\n",latter_list[k]);
		}
		//printf("latter_list[]에 %d개.\n", cho3_count[latter]);

		//	앞의 튜플, 뒤의 트리플 지퍼
		i = 0;
		j = 0;	// 이 코드가 없어서 버그가 났었을 것임. 이종우가 이 줄을 추가함.
		if (cho2_count[former] != 0 && cho3_count[latter] != 0) {
			for (;;) {
				if (former_list[i] > latter_list[j])
					j++;
				else if (former_list[i] < latter_list[j])
					i++;
				else {	// 겹치는 줄번호 발견
					cho_final[cur_final][cho_final_count[cur_final]++] = former_list[i];
					i++;
					j++;
				}
				if (i == cho2_count[former] || j == cho3_count[latter])
					break;
			}
		}
		//printf("튜플, 트리플 count %d\n", cho_final_count[cur_final]);
		if (cho_final_count[cur_final] == 0) {	// 튜플, 트리플 겹치는 줄이 없으면
			if (cho3_count[latter] > 0) {
				cho_final_count[cur_final] = cho3_count[latter];
				for (a = 0; a < cho3_count[latter]; a++)
					cho_final[cur_final][a] = latter_list[a];
				//printf("==> 뒤 트리플 only로 전환: cho_final_count[%d] = %d.\n", cur_final, cho_final_count[cur_final]);
			}
			else if (cho2_count[former] > 0) {
				cho_final_count[cur_final] = cho2_count[former];
				for (a = 0; a < cho2_count[former]; a++)
					cho_final[cur_final][a] = former_list[a];
				//printf("==> 앞 튜플 only로 전환: cho_final_count[%d] = %d.\n", cur_final, cho_final_count[cur_final]);
			}
		}
		//for (int c = 0; c < cho_final_count[cur_final]; c++) {
		//	printf("%s\n", &memory_db[position_in_memory[cho_final[cur_final][c]]]);
		//}

		cur_final++;		// 이제 앞 첫 3연속초성과 뒤 첫 2연속초성을 모두 갖고 있는 줄번호들을 검사하러 가자. 즉, 첫 트리플, 튜플을 검사하러 가자.
		//printf("cur_final = %d\n", cur_final);

		former = in_eo[0]*38*38 + in_eo[1]*38 + in_eo[2];		// 쿼리의 앞 첫 3연속초성ID를 계산한다.
		latter = in_eo[in_count-2]*38 + in_eo[in_count-1];		// 쿼리의 뒤 첫 2연속초성ID를 계산한다.
		//printf("former: %d , latter : %d\n",former, latter);
		//printf("former_c: %d , latter_c : %d\n",cho3_count[former], cho2_count[latter]);

		cur = cho2_start[latter];		// 쿼리의 뒤 첫 2연속초성의 등장 줄번호들이 저장되어 있는 역파일 내의 첫 위치를 알아낸다.
		for (k = 0; k < cho2_count[latter]; k++) { // 이 뒤 첫 2연속초성이 등장하는 개수만큼 루프돈다.
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
		//printf("latter_list[]에 %d개.\n", cho2_count[latter]);

		cur = cho3_start[former];		// 쿼리의 앞 첫 3연속초성의 등장 줄번호들이 저장되어 있는 역파일 내의 첫 위치를 알아낸다.
		for (k = 0; k < cho3_count[former]; k++) {	// 이 앞 첫 3연속초성이 등장하는 개수만큼 루프돈다.
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
		//printf("former_list[]에 %d개.\n", cho3_count[former]);

		//	앞의 트리플, 뒤의 튜플 지퍼 ==> 정렬되어 있는 두 배열을 합친다.
		i = 0;
		j = 0;
		if (cho3_count[former] != 0 && cho2_count[latter] != 0) {
			for (;;) {
				if (former_list[i] > latter_list[j])
					j++;
				else if (former_list[i] < latter_list[j])
					i++;
				else {	// 겹치는 줄번호 발견
					cho_final[cur_final][cho_final_count[cur_final]++] = former_list[i];
					i++;
					j++;
				}
				if (i == cho3_count[former] || j == cho2_count[latter])
					break;
			}
		}
		//printf("트리플, 튜플 count %d\n", cho_final_count[cur_final]);
		//for (int c = 0; c < cho_final_count[cur_final]; c++) {
		//	printf("%s\n", &memory_db[position_in_memory[cho_final[cur_final][c]]]);
		//}
		if (cho_final_count[cur_final] == 0) {	// 트리플, 튜플 겹치는 줄이 없으면
			if (cho3_count[former] > 0) {
				cho_final_count[cur_final] = cho3_count[former];
				for (a = 0; a < cho3_count[former]; a++)
					cho_final[cur_final][a] = former_list[a];
				//printf("==> 앞 트리플 only로 전환: cho_final_count[%d] = %d.\n", cur_final, cho_final_count[cur_final]);
			}
			else if (cho2_count[latter] > 0) {
				cho_final_count[cur_final] = cho2_count[latter];
				for (a = 0; a < cho2_count[latter]; a++)
					cho_final[cur_final][a] = latter_list[a];
				//printf("==> 뒤 튜플 only로 전환: cho_final_count[%d] = %d.\n", cur_final, cho_final_count[cur_final]);
			}
		}

		cur_final++;	// cho_final[0][...]과 cho_final[1][...]은 채웠으므로, cho_final[2][...]를 채우러 간다. 이제 뭘로 채웠을까 궁금해진다.ㅋㅋ
		//printf("cur_final = %d\n", cur_final);
				
		//	투쓰리+쓰리투 머지 ==> 머지 시에도 지퍼 알고리즘을 이용한다. 정렬되어 있는 두 배열을 합치는 알고리즘.
		i = 0, j = 0;
		if (cho_final_count[cur_final-2] > 0 && cho_final_count[cur_final-1] > 0) {	// 지핑할 거리가 있어야...
			for (;;) {
				if (cho_final[cur_final-2][i] > cho_final[cur_final-1][j]) {
					cho_final[cur_final][cho_final_count[cur_final]++] = cho_final[cur_final-1][j];
					j++;
				}
				else if (cho_final[cur_final-2][i] < cho_final[cur_final-1][j]) {
					cho_final[cur_final][cho_final_count[cur_final]++] = cho_final[cur_final-2][i];
					i++;
				}
				else {	// 겹치는 줄은 한 번만 출력한다.
					cho_final[cur_final][cho_final_count[cur_final]++] = cho_final[cur_final-2][i];
					i++, j++;
				}
				if (i >= cho_final_count[cur_final-2] || j >= cho_final_count[cur_final-1])
					break;
			}
		}
		// 남은 줄들 머지
		if (i == cho_final_count[cur_final-2]) {
			for(;j < cho_final_count[cur_final-1]; j++)
				cho_final[cur_final][cho_final_count[cur_final]++] = cho_final[cur_final-1][j];
		}
		else if (j == cho_final_count[cur_final-1]) {
			for(;i < cho_final_count[cur_final-2]; i++)
				cho_final[cur_final][cho_final_count[cur_final]++] = cho_final[cur_final-2][i];
		}
		//printf("지퍼 후 cho_final_count[cur_final] = %d\n", cho_final_count[cur_final]);
	}

 	// 쿼리가 6글짜 초과인 경우에는 튜플...트리플, 또는 트리플...튜플 사이에 문자가 있을 것이므로 그것들을 처리하자.
	for (i = 0; i < in_count-6; i++) {
		int found_more;

		found_more = 0;
		johapcode = in_eo[i+3];	// 네번째, 다섯번째.... 초성을 가져옴.
		for(int j = 0; j < cho_final_count[cur_final]; j++) {  // cho_final_count[2]의 개수만큼. 즉, 투쓰리+쓰리투 모두 등장 라인 모두에 대해
			cur_entry = cho_final[cur_final][j];	// 줄번호 하나 가져옴

			for (int z = 0; z < namepart_len[cur_entry]; z++) {	// 쿼리의 "투쓰리+쓰리투"가 등장한 줄에서 명칭 부분에 대해서만
				if ((unsigned short)memory_db[position_in_memory[cur_entry]+z] >= 128) {	// 한글일때 -> 초성을 뽑아서 temp2에 넣자
					han[0] = memory_db[position_in_memory[cur_entry]+z]; 
					han[1] = memory_db[position_in_memory[cur_entry]+z+1];
					han[2]='\0';			//////////// 우이쒸! 다들 죽인다 그래라. 여기에 '\n'이 뭐냐? 미쳐 증말!!!
					HangulCodeConvert(KS, han, c_han);
					cho_code = ((unsigned char)c_han[0] << 8) | ((unsigned char)c_han[1]); 
					cho_code = (cho_code >> 10) & 0x1f;
					z++;		// 한글이므로 한 바이트 더 전진
				}
#ifndef	IGNORE_NON_HANGUL
				else if ((unsigned short)memory_db[position_in_memory[cur_entry]+z] >= '0' && (unsigned short)memory_db[position_in_memory[cur_entry]+z] <= '9')	// 숫자
					cho_code = (unsigned short)memory_db[position_in_memory[cur_entry]+z] - '0' + 0x15;
				else if (memory_db[position_in_memory[cur_entry]+z] == 0)	// 맨 뒤 널 문자이면 무시.
					continue;
				else 		// 알파벳
					cho_code = 0;
#else
				else if (memory_db[position_in_memory[cur_entry]+z] == 0)	// 맨 뒤 널 문자이면 무시.
					continue;
				else
					cho_code = memory_db[position_in_memory[cur_entry]+z];
#endif
				if (johapcode == cho_code) {	// 초성이 같으면 cho_final[]에 cur_entry를 추가
					cho_final[cur_final+1][cho_final_count[cur_final+1]++] = cur_entry;
					found_more = 1;
					break;
				}
			} // end of for (int z = 0; z < namepart_len[cur_entry]; z++)
		} // end of for(int j = 0; j < cho_final_count[cur_final]; j++)
		if (found_more)
			cur_final++;
		//printf("cur_final(7글자이상) = %d\n", cur_final);
		//printf("cho_final_count[cur_final] = %d\n", cho_final_count[cur_final]);
	} // end of for (i = 0; i < in_count-6; i++)

	// 쿼리에 알파벳이 포함되어 있었으면 이 알파벳까지 포함하고 있는 줄번호들을 별도로 저정한다.
	// cho_final[cur_final][...]은 cur_final이 증가됨에 따라 더 쿼리를 더 완전하게 포함하고 있는 줄 번호들이 들어간다. 
	for (int xx = 0; xx < in_alpha_count; xx++) {  // 쿼리 문자개수인 in_count 중 알파벳의 개수인 query_alpha_count 만큼
		int found_more;

		found_more = 0;
		johapcode = in_alpha[xx];		// 쿼리에 포함되어 있던 알파벳을 가져온다.
		//printf("cur_final %d, cho_final_count[cur_final] %d\n",cur_final, cho_final_count[cur_final]);
		
		for (int ii = 0; ii < cho_final_count[cur_final]; ii++) {
			cur_entry = cho_final[cur_final][ii];						
			
			for (int z = 0; z < namepart_len[cur_entry]; z++) { //cur_entry에서 name만큼만
				// 소문자면
				if((unsigned short)memory_db[position_in_memory[cur_entry]+z] >= 'a' && (unsigned short)memory_db[position_in_memory[cur_entry]+z] <= 'z')
					cho_code = (unsigned short)memory_db[position_in_memory[cur_entry]+z] - 32;	// 대문자로 바꾸어 비교함
				else if (memory_db[position_in_memory[cur_entry]+z] == 0)	// 맨 뒤 널 문자이면 무시.
					continue;
				else
					cho_code = (unsigned short)memory_db[position_in_memory[cur_entry]+z];

				if (johapcode == cho_code) {	// 알파벳이 일치하면
					cho_final[cur_final+1][cho_final_count[cur_final+1]++] = cur_entry;
					found_more = 1;
					break;
				}
			} // end of for(int z = 0; z < namepart_len[cur_entry]; z++)
		} // end of for (int ii = 0; ii < cho_final_count[cur_final]; ii++)
		if (found_more)
			cur_final++;
		//printf("cur_final(알파벳처리중) = %d\n", cur_final);
	}

	cho_max_final = cur_final;
	//printf("cho_max_final = %d, cho_final_count[cho_cur_final] = %d\n",cho_max_final, cho_final_count[cho_max_final]);
}

void
scoring_and_output_4text(int fFullyIncluded)
{
	int i, cur, k, z;
	short *score;	// 각 검색 결과가 쿼리와 얼마나 같은지를 나타내는 점수 배열
	int print_cnt;

	mfd = merged2_final_degree;

	if (fFullyIncluded < 0)
		return;
	else if (fFullyIncluded) {
		printf("--------------- 명칭부분 완전 포함 결과 ---------------\n");
		printf(" 줄번호 차수 검색결과(스코어)\n");
		/********************************************************************************JAY 전체차수보기용..
		for (i = 0; mfd[i] >= mfd[0]; i++);		// 완전포함 차수의 개수를 얻는다.
		k = i;
		printf("최고차수 개수: %d.\n", k);

		for (; mfd[i] >= mfd[k]; i++);			// 그 다음 차수의 개수를 얻는다.
		z = i;
		printf("2.[[[[%d, %d]]]]", k, z);
		printf("다음 차수 개수: %d.\n", z-k);
		*/
		//////////////
		/*
		for (i = 0; mfd[i] >= mfd[0]; i++);		// 완전포함 차수의 개수를 얻는다.
		k = i;
		for (i = k; i < final_count[max_final]; i++) {
			if(i>18000){
				break;
			}
		}
		z = i;	// 출력할 검색 결과의 개수를 알아낸다.
		printf("[[[**2**]]]", k, z);
		printf("다음 차수 개수: %d.\n", z-k);
		*/
		//////////////
		k = 0;
		i = -1;
		do{
			
			//최대 18,000까지 불러오기
			if(i>18000){
				printf("로드될 데이터가 18000개가 넘습니다.\n");
				break;
			}
			/*
			// 파티셔닝 안된 차수까지 전부 불러오기
			
			if(mfd[i]<0){
				printf("minus[%d]\n", i);
				break;
			}
			*/

			i++;

			if(mfd[i] < mfd[k]){
				printf("개수: %d[%d-%d].\n", i-k, mfd[i], mfd[k]);
				//파티셔닝 된 차수까지만 불러오기
				/*
				if(i > final_count[max_final]-1){
					printf("마지막차수[%d]\n", i);
					break;
				}
				*/
				//필터링할 최소차수(min_chasu)까지만 불러오기
				if(i > no_of_filtered_data-1){
					printf("마지막차수[%d]\n", i);
					break;
				}
				k = i;
			}
		}while(1);
		z = i;
		printf("총로드개수: %d.\n", z);

		score = (short *)malloc(sizeof(short)*z);
		if (!score ) {
			fprintf(stderr, "scoring_and_output_4text(): score[] 배열 할당 실패!\n");
			return;
		}

/////////////////JAY
		/*
		for (i = 0; i < k; i++) {			// 완전포함 차수에 대해서.
			cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
			score[i] = CalSimilarity((const unsigned char *)&memory_db[cur], i);	// 검색 결과와 쿼리 간의 일치도를 계산한다.
		}
		quickSort5(score, 0, k-1);	// 감점 스코어 기준 오름차순 정렬
		for (i = k; i < z; i++) {			// 그 다음 차수에 대해서.
			cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
			score[i] = CalSimilarity((const unsigned char *)&memory_db[cur], i);	// 검색 결과와 쿼리 간의 일치도를 계산한다.
		}
		quickSort5(score, k, z-1);	// 감점 스코어 기준 오름차순 정렬
		*/
/////////////////JAY

		//차수는 따지지 않는다고 봤을때
		for (i = 0; i < z; i++) {	//차수 따지지않음.
			cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
			score[i] = CalSimilarity((const unsigned char *)&memory_db[cur], i);	// 검색 결과와 쿼리 간의 일치도를 계산한다.
		}
		quickSort5(score, 0, z-1);	// 감점 스코어 기준 오름차순 정렬
		
/////////////////JAY

		print_cnt = 0;
		for (i = 0; i < z; i++) {
			if (score[i] <= MAX_MINUS_POINT)	// 점수가 MAX_NINUS_POINT 이하인 줄이 몇 개인지 센다.
				print_cnt++;
		}
		if (print_cnt >= MAX_OUT_LINES) {		// 점수가 MAX_NINUS_POINT 이하인 줄로만 MAX_OUT_LINES 개를 출력할 수 있으면
			int extra_print_cnt = 0;

			for (i = 0; i < z; i++) {
				if (score[i] <= MAX_MINUS_POINT) {
					printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
					cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
					printf("%s(%d)\n", &memory_db[cur], score[i]);

					extra_print_cnt++;
					if (extra_print_cnt >= MAX_OUT_LINES)
						break;
				}
			}
		}
		else {	// 점수가 MAX_NINUS_POINT 이하인 줄로만 MAX_OUT_LINES 개를 출력하기에는 부족함.
			int extra_print_cnt = 0;

			for (i = 0; i < z; i++) {
				if (score[i] <= MAX_MINUS_POINT) {
					printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
					cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
					printf("%s(%d)\n", &memory_db[cur], score[i]);
				}
			}

			for (i = 0; i < z; i++) {
				if (score[i] > MAX_MINUS_POINT) {
					printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
					cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
					printf("%s(%d)\n", &memory_db[cur], score[i]);

					extra_print_cnt++;
					if (extra_print_cnt >= (MAX_OUT_LINES - print_cnt))
						break;
				}
			}
		}
		printf("------------- 명칭부분 완전 포함 결과 끝(%d개) -------------\n", z);
		free(score);
	}
	else {
		// 여기로 왔더라도 쿼리 길이가 3 미만이면 완전 포함일 것임
		if (eo_count >= 3) {
			printf("----- 명칭부분 부분 포함 결과 -----\n");
			printf(" 줄번호 차수 검색결과(스코어)\n");
		}
		else {
			printf("---------- 명칭부분 완전 포함 결과 ----------\n");
			printf(" 줄번호 검색결과(스코어)\n");
		}
		/*
		for (i = 0; i < final_count[max_final]; i++) {
			if (eo_count >= 3) {	// 쿼리 길이가 세 글자 이상일 때에만 merged_final_degree[]를 만들기 때문
				if (mfd[i] < mfd[0])		// 최고 차수와 그 다음 차수까지만 보자.
					break;
			}
		}
		k = i;	// 출력할 검색 결과의 개수를 알아낸다.
		printf("최고차수 개수: %d.\n", k);
		for (i = k; i < final_count[max_final]; i++) {
			//*******************************************************************************************JAY : 모든차수를 다보기 위해 일단 지워봤음.120828
			if (eo_count >= 3) {	// 쿼리 길이가 세 글자 이상일 때에만 merged_final_degree[]를 만들기 때문
				if (mfd[i] < mfd[k])		// 최고 차수와 그 다음 차수까지만 보자.
					break;
			}
			
			if(i>18000){
				break;
			}
		}
		*/
		/////////////////////////////[JAY]120919
		if (eo_count >= 3){		// 쿼리 길이가 세 글자 이상일 때에만 merged_final_degree[]를 만들기 때문
			k = 0;
			i = -1;
			do{
				
				//최대 18,000까지 불러오기
				if(i>18000){
					printf("18000[%d]\n", i);
					break;
				}
				/*
				// 파티셔닝 안된 차수까지 전부 불러오기
			
				if(mfd[i]<0){
					printf("minus[%d]\n", i);
					break;
				}
				*/

				i++;

				if(mfd[i] < mfd[k]){
					printf("개수: %d[%d-%d].\n", i-k, mfd[i], mfd[k]);
					//파티셔닝 된 차수까지만 불러오기
					/*
					if(i > final_count[max_final]-1){
						printf("마지막차수[%d]\n", i);
						break;
					}
					*/
					//필터링할 최소차수(min_chasu)까지만 불러오기
					if(i > no_of_filtered_data-1){
						printf("마지막차수[%d]\n", i);
						break;
					}
					k = i;
				}
			}while(1);
		}
		else{
			for (i = 0; i < final_count[max_final]; i++) {
				//최대 18,000까지 불러오기
				if(i>18000){
					printf("로드될 데이터가 18000개가 넘습니다.\n");
					break;
				}
			}
		}

		z = i;
		printf("총로드개수: %d.\n", z);

		printf("[**1**]\n");
		/////////////////////////////
		score = (short *)malloc(sizeof(short)*z);
		if (!score ) {
			fprintf(stderr, "scoring_and_output_4text(): score[] 배열 할당 실패!\n");
			return;
		}
		/////////////////JAY
		/*
		for (i = 0; i < k; i++) {			// 완전포함 차수에 대해서.
			cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
			score[i] = CalSimilarity((const unsigned char *)&memory_db[cur], i);	// 검색 결과와 쿼리 간의 일치도를 계산한다.
		}
		quickSort5(score, 0, k-1);	// 감점 스코어 기준 오름차순 정렬
		for (i = k; i < z; i++) {			// 그 다음 차수에 대해서.
			cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
			score[i] = CalSimilarity((const unsigned char *)&memory_db[cur], i);	// 검색 결과와 쿼리 간의 일치도를 계산한다.
		}
		quickSort5(score, k, z-1);	// 감점 스코어 기준 오름차순 정렬
		*/
/////////////////JAY

		//차수는 따지지 않는다고 봤을때
		for (i = 0; i < z; i++) {	//차수 따지지않음.
			cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
			score[i] = CalSimilarity((const unsigned char *)&memory_db[cur], i);	// 검색 결과와 쿼리 간의 일치도를 계산한다.
		}
		quickSort5(score, 0, z-1);	// 감점 스코어 기준 오름차순 정렬
		
/////////////////JAY

		print_cnt = 0;
		for (i = 0; i < z; i++) {
			if (score[i] <= MAX_MINUS_POINT)	// 점수가 MAX_NINUS_POINT 이하인 줄이 몇 개인지 센다.
				print_cnt++;
		}

		if (print_cnt >= MAX_OUT_LINES) {		// 점수가 MAX_NINUS_POINT 이하인 줄로만 MAX_OUT_LINES 개를 출력할 수 있으면
			int extra_print_cnt = 0;

			for (i = 0; i < z; i++) {
				if (score[i] <= MAX_MINUS_POINT) {
					if (eo_count >= 3) {	// 쿼리 길이가 세 글자 이상일 때에만 merged_final_degree[]를 만들기 때문
						printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
					}
					else {					// 쿼리 길이가 두 글자일 때에는 차수 계산이 필요 없으므로 mfd 배열을 만들지 않는다.
						printf("%7d   ", final[max_final][i]+1);
					}
					cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
					printf("%s(%d)\n", &memory_db[cur], score[i]);

					extra_print_cnt++;
					if (extra_print_cnt >= MAX_OUT_LINES)
						break;
				}
			}
		}
		else {	// 점수가 MAX_NINUS_POINT 이하인 줄로만 MAX_OUT_LINES 개를 출력하기에는 부족함.
			int extra_print_cnt = 0;

			for (i = 0; i < z; i++) {
				if (score[i] <= MAX_MINUS_POINT) {
					if (eo_count >= 3) {	// 쿼리 길이가 세 글자 이상일 때에만 merged_final_degree[]를 만들기 때문
						printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
					}
					else {					// 쿼리 길이가 두 글자일 때에는 차수 계산이 필요 없으므로 mfd 배열을 만들지 않는다.
						printf("%7d   ", final[max_final][i]+1);
					}
					cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
					printf("%s(%d)\n", &memory_db[cur], score[i]);
				}
			}

			for (i = 0; i < z; i++) {
				if (score[i] > MAX_MINUS_POINT) {
					if (eo_count >= 3) {	// 쿼리 길이가 세 글자 이상일 때에만 merged_final_degree[]를 만들기 때문
						printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
					}
					else {					// 쿼리 길이가 두 글자일 때에는 차수 계산이 필요 없으므로 mfd 배열을 만들지 않는다.
						printf("%7d   ", final[max_final][i]+1);
					}
					cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
					printf("%s(%d)\n", &memory_db[cur], score[i]);

					extra_print_cnt++;
					if (extra_print_cnt >= (MAX_OUT_LINES - print_cnt))
						break;
				}
			}
		}

		if (eo_count >= 3)
			printf("------------- (쿼리길이 >= 3) 명칭부분 부분 포함 결과 끝(%d개) -------------\n", z);
		else
			printf("------------- (쿼리길이 < 3) 명칭부분 완전 포함 결과 끝(%d개) -------------\n", z);

		free(score);
	}
	/*
	if (!fFullyIncluded) {	// 역파일만으로는 쿼리를 완전 포함하고 있는 라인을 찾을 수 없을 때, 주소까지로 확장 검색
		// 현재 쿼리의 앞 두 문자가 동시에 등장하는 줄번호는 구해진 상태. 
		for (i = 1; i < MAX_QUERY_LEN; i++)
			final_count[i] = 0; // 매우 중요
		
		max_final = 0;
		for (j = 2; j < eo_count; j++) {
			unsigned char former, latter;

			former = eo_string[j][0];
			latter = eo_string[j][1];

			for (i = 0; i < final_count[j-2]; i++) {
				cur = position_in_memory[final[j-2][i]];
				len = namepart_len[final[j-2][i]] + addrpart_len[final[j-2][i]];
														// address 라는 부분만 빼고 앞 블럭과 완전 같음.
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
					else { // 블랭크를 포함한 일반 아스키 문자
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

		// 이종우가 추가한 부분!
		// 이유: 이름 부분만으로 완전 포함된 줄이 없을 때
		printf("--------------- 주소부분까지 확장 검색 결과(%d 개) ---------------\n", final_count[max_final]);
		for (i = 0; i < final_count[max_final]; i++) {	// 쿼리 문자열이 디비의 어느 줄에 전부 등장하는 모든 줄에 대해서...
			printf("줄번호 %d: ", final[max_final][i]+1);
			cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
			len = namepart_len[final[max_final][i]] + addrpart_len[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 이름 부분 + 주소 부분 길이
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
				else { // 블랭크를 포함한 일반 아스키 문자
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
 * 통합 스코어링 함수를 새로 만들었으므로 필요 없어 코멘트 처리함.
 */
/*
void
scoring_and_output_4chosung(short *query_cho, int query_count, int *in_alpha, int in_alpha_count)
{
	int score=0;
	short c_count = 0;					// 명칭부분의 초성 개수를 세기 위해 사용
	short c_alpha_count = 0;
	//short c_ad_count = 0;				// 주소부분의 초성 개수를 세기 위해 사용
	//short c_ad_alpha_count = 0;
	int cur_entry = 0;
	int cur;
	char han[HANGUL_BYTE+1];			// 한글 처리 시 글자 하나를 담아 놓기 위한 공간. NULL 문자까지 포함하기 위해 +1.
	char c_han[HANGUL_BYTE+1];			// 완성형 글자를 조합형으로 바꿀 때 사용하기 위해 선언.
	unsigned short johapcode;
	int i;
	short namepart_cho[MAX_JAEUM_CNT]={0};
	char namepart_alpha[MAX_JAEUM_CNT] = {0};
	//short addrpart_cho[MAX_JAEUM_CNT]={0};
	//char addrpart_alpha[MAX_JAEUM_CNT] = {0};
	bool used[60]={false};
	int *cho_score;

	// 스코어 산출 시 사용할 cho_score[](cho_final_count[cho_max_final] 개)를 미리 할당해둔다.
	cho_score = (int *)malloc(sizeof(int)*cho_final_count[cho_max_final]);
	if (cho_score == NULL) {
		fprintf(stderr, "cho_score[] 배열 할당 실패. 메모리가 부족해서 일겁니다.\n");
		exit(-10);
	}
	//memset(cho_score, 0, sizeof(int)*cho_final_count[cho_max_final]);	// 미리 클리어해둔다.

	//***************************** score 점수*************************************************
	for (int f = 0; f < cho_final_count[cho_max_final]; f++) { // cho_final_count[cho_max_final]의 갯수만큼
		score = 0;
		c_count = 0;		// 명칭 부분 길이를 세는 용도
		//c_ad_count = 0;		// 주소 부분 길이를 세는 용도
		// cur = 0;
		cur_entry = cho_final[cho_max_final][f];	// 현재 라인넘버를 cur_entry에 넣어줌
		cur = position_in_memory[cur_entry];		// 현재 라인이 들어 있는 memory_db[] 배열 내의 위치
		
		for (int w = 0; w < 30; w++)
			used[w] = false;
		
		// 초성검색된 줄번호 cur_entry에 해당하는 명칭부분 문자열에서 초성 뽑아주는 for문
		int q;
		for (q = 0; q < namepart_len[cur_entry]; q++) { // 그 줄의 명칭 부분 길이 만큼
			if ((unsigned short)memory_db[cur] >= 128) {	// 한글이면
				han[0] = memory_db[cur++]; 
				han[1] = memory_db[cur++];
				han[2] = '\0';
				HangulCodeConvert(KS, han, c_han);
				johapcode = ((unsigned char)c_han[0] << 8) | ((unsigned char)c_han[1]); 
				namepart_cho[c_count] = (johapcode >> 10) & 0x1f;		// 초성을 추출한다.
				c_count++;
				q++;	// 한글이니까 한 바이트 더 전진해준다.
			}
			else	// 숫자는 아닌 아스키코드
				if (memory_db[cur] != 0)		// 문자열 맨 뒤의 널 문자는 쿼리에 포함되지 않게 해야 함.
					namepart_alpha[c_alpha_count++] = memory_db[cur++];
		}
		
		// 초성검색된 줄번호 cur_entry에 해당하는 주소부분 문자열에서 초성 뽑아주는 for문
		// 주소부분 문자열을 쿼리와 비교하는 것은 일단은 제외한다. 완성형에서도 그건 안 하고 있음.
		// 주소 부분에 대한 검색은 일단 안 하므로 코멘트 처리 했음.
		//for (q = 0; q < addrpart_len[cur_entry]; q++) {	// 그 라인의 주소부분 길이 만큼
		//	if ((unsigned short)memory_db[cur] >= 128) {	// 한글이면
		//		han[0] = memory_db[cur++]; 
		//		han[1] = memory_db[cur++];
		//		han[2] = '\0';
		//		HangulCodeConvert(KS, han, c_han);
		//		johapcode = ((unsigned char)c_han[0] << 8) | ((unsigned char)c_han[1]); 
		//		addrpart_cho[c_ad_count] = (johapcode >> 10) & 0x1f;	// 초성 추출
		//		c_ad_count++;
		//		q++;	// 한글이니까 한 바이트 더 전진한다.
		//	}
		//	else
		//		if (memory_db[cur] != 0)		// 문자열 맨 뒤의 널 문자는 쿼리에 포함되지 않게 해야 함.
		//			addrpart_alpha[c_ad_alpha_count++] = memory_db[cur++];
		//}

		for (i = 0; i < query_count; i++) {	// 쿼리로 들어온 글자의 갯수만큼
			for (int b = 0; b < c_count; b++) {	// 이 쿼리를 포함하고 있다고 검색된 줄의 명칭 부분 초성 개수만큼
				if (query_cho[i] == namepart_cho[b]) {	// 이 줄에서 쿼리와 초성이 같은 위치부터 비교 시작
					for (int u = 1; u < c_count-b && u < query_count-i; u++) {
						if (query_cho[i+u] == namepart_cho[b+u]) {		// 다음 초성도 같으면
							used[b+u] = true;
							score += 10;
						}
						else
							break;
					} // end of for (int u = 1; u < c_count-b && u < in_count-i; u++)
				}
			} // end of for (int b = 0; b < c_count; b++)

			// 주소 부분에 대한 스코어링은 일단 제외하기로 해서 코멘트 처리 했음.
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
		
		for (int l = 0; l < c_count; l++)	// 명칭 부분 문자 개수 만큼
			if (used[l] == false)	// 쿼리에 포함 안 된 초성 하나당 10점씩 감점
				score -= 10;

		cho_score[f] = score;	// 이 줄의 최종 점수를 기록함.
	}

	printf("quickSort(cho_score, 0, %d)...", cho_final_count[cho_max_final]-1);
	quickSort6(cho_score, 0, cho_final_count[cho_max_final]-1);	// cho_score[]를 내림차순 정렬하면서 cho_final[cho_max_final][...]도 같이 바꿔준다.
	printf("done.\n");

	// 모음 검색 결과 출력해주는 부분
	for (i = 0; i < cho_final_count[cho_max_final]; i++) {
		int cur;
		int len;

		cur = position_in_memory[cho_final[cho_max_final][i]];
		len = namepart_len[cho_final[cho_max_final][i]];
		
		printf("%7d %5d점 %s", cho_final[cho_max_final][i]+1, cho_score[i], &memory_db[cur]);

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
 * 두 검색 결과를 합쳐서 스코어링 한다.
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

	strCnt = StrToEoArray((const unsigned char *)in_query, strArray);	// 쿼리를 온전한 완성형 문자는 그대로, 완성형 초성코드는 조합형으로 바꾼다.
	//for (int i = 0; i < strCnt; i++) {
	//	if (strArray[i][2] == '\0')			// 완성형 완전한 문자
	//		printf("%s ", strArray[i]);
	//	else if (strArray[i][2] == '\1')	// 초성 조합형 코드 문자
	//		printf("0x%02x ", strArray[i][0]);
	//}
	printf("\n");

	if (fFullyIncluded >= 0) {	// 완성형 검색 결과를 대상으로 통합 스코어링을 실시함.
		mfd = merged2_final_degree;
		if (fFullyIncluded) {	// 완전포함
			for (i = 0; mfd[i] >= mfd[0]; i++);		// 완전포함 차수의 개수를 얻는다.
			k = i;
			//printf("최고 차수 개수: %d.\n", k);
			for (i = k; mfd[i] >= mfd[k]; i++);	// 다음 차수의 개수를 얻는다.
			z = i;
			//printf("다음 차수 개수: %d.\n", z-k);
			score = (short *)malloc(sizeof(short)*z);
			if (!score ) {
				fprintf(stderr, "integrated_scoring_and_output(): score[] 배열 할당 실패!\n");
				return;
			}
			for (i = 0; i < k; i++) {			// 완전포함 차수에 대해서.
				cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
				score[i] = CalMixedSimilarity(strArray, strCnt, (const unsigned char *)&memory_db[cur]);	// 검색 결과와 쿼리 간의 일치도를 계산한다.
			}
			quickSort5(score, 0, k-1);	// 감점 스코어 기준 오름차순 정렬
			for (i = k; i < z; i++) {			// 다음 차수에 대해서.
				cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
				score[i] = CalMixedSimilarity(strArray, strCnt, (const unsigned char *)&memory_db[cur]);	// 검색 결과와 쿼리 간의 일치도를 계산한다.
			}
			quickSort5(score, k, z-1);	// 감점 스코어 기준 오름차순 정렬
			printf("--------------- 텍스트 검색 기반 통합 스코어링 결과 ---------------\n");
			printf(" 줄번호 차수 검색결과(스코어)\n");

			print_cnt = 0;
			for (i = 0; i < z; i++) {
				if (score[i] <= MAX_MINUS_POINT)	// 점수가 MAX_NINUS_POINT 이하인 줄이 몇 개인지 센다.
					print_cnt++;
			}
			if (print_cnt >= MAX_OUT_LINES) {		// 점수가 MAX_NINUS_POINT 이하인 줄로만 MAX_OUT_LINES 개를 출력할 수 있으면
				int extra_print_cnt = 0;

				for (i = 0; i < z; i++) {			// 완전포함 차수까지만 보자.
					if (score[i] <= MAX_MINUS_POINT) {
						printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
						cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
						printf("%s(%d)\n", &memory_db[cur], score[i]);

						extra_print_cnt++;
						if (extra_print_cnt >= MAX_OUT_LINES)
							break;
					}
				}
			}
			else {	// 점수가 MAX_MINUS_POINT 이하인 줄로만 MAX_OUT_LINES 개를 출력하기에는 부족함.
				int extra_print_cnt = 0;

				for (i = 0; i < z; i++) {		// 일단 벌점이 MAX_MINUS_POINT 이하인 줄들을 전부 출력한다.
					if (score[i] <= MAX_MINUS_POINT) {
						printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
						cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
						printf("%s(%d)\n", &memory_db[cur], score[i]);
					}
				}

				for (i = 0; i < z; i++) {
					if (score[i] > MAX_MINUS_POINT) {	// 벌점이 MAX_MINUS_POINT 초과인 줄들을 출력하되 MAX_OUT_LINES를 넘지 않게 한다.
						printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
						cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
						printf("%s(%d)\n", &memory_db[cur], score[i]);

						extra_print_cnt++;
						if (extra_print_cnt >= (MAX_OUT_LINES - print_cnt))
							break;
					}
				}
			}
			printf("------------- 텍스트 검색(명칭부분 완전 포함) 기반 통합 스코어링 결과(%d개) -------------\n", z);
			free(score);
		}
		else {					// 부분포함
			for (i = 0; i < final_count[max_final]; i++) {
				if (eo_count >= 3) {	// 쿼리 길이가 세 글자 이상일 때에만 merged_final_degree[]를 만들기 때문
					if (mfd[i] < mfd[0])	// 최고차수까지만 보자.
						break;
				}
			}
			k = i;	// 출력할 검색 결과의 개수를 알아낸다.
			//printf("최고 차수 개수: %d.\n", k);
			for (i = k; i < final_count[max_final]; i++) {
				if (eo_count >= 3) {	// 쿼리 길이가 세 글자 이상일 때에만 merged_final_degree[]를 만들기 때문
					if (mfd[i] < mfd[k])	// 다음 차수까지 보자.
						break;
				}
			}
			z = i;	// 출력할 검색 결과의 개수를 알아낸다.
			//printf("다음 차수 개수: %d.\n", z-k);
			score = (short *)malloc(sizeof(short)*z);
			if (!score ) {
				fprintf(stderr, "integrated_scoring_and_output(): score[] 배열 할당 실패!\n");
				return;
			}
			for (i = 0; i < k; i++) {			// 완전포함 차수에 대해서.
				cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
				score[i] = CalMixedSimilarity(strArray, strCnt, (const unsigned char *)&memory_db[cur]);	// 검색 결과와 쿼리 간의 일치도를 계산한다.
			}
			quickSort5(score, 0, k-1);	// 감점 스코어 기준 오름차순 정렬
			for (i = k; i < z; i++) {			// 다음 차수에 대해서.
				cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
				score[i] = CalMixedSimilarity(strArray, strCnt, (const unsigned char *)&memory_db[cur]);	// 검색 결과와 쿼리 간의 일치도를 계산한다.
			}
			quickSort5(score, k, z-1);	// 감점 스코어 기준 오름차순 정렬
			printf("--------------- 텍스트 검색 기반 통합 스코어링 결과 ---------------\n");
			if (eo_count >= 3)
				printf(" 줄번호 차수 검색결과(스코어)\n");
			else
				printf(" 줄번호 검색결과(스코어)\n");

			print_cnt = 0;
			for (i = 0; i < z; i++) {
				if (score[i] <= MAX_MINUS_POINT)	// 점수가 MAX_NINUS_POINT 이하인 줄이 몇 개인지 센다.
					print_cnt++;
			}

			if (print_cnt >= MAX_OUT_LINES) {		// 점수가 MAX_NINUS_POINT 이하인 줄로만 MAX_OUT_LINES 개를 출력할 수 있으면
				int extra_print_cnt = 0;

				for (i = 0; i < z; i++) {
					if (score[i] <= MAX_MINUS_POINT) {
						if (eo_count >= 3) {	// 쿼리 길이가 세 글자 이상일 때에만 merged_final_degree[]를 만들기 때문
							printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
						}
						else {					// 쿼리 길이가 두 글자일 때에는 차수 계산이 필요 없으므로 mfd 배열을 만들지 않는다.
							printf("%7d   ", final[max_final][i]+1);
						}
						cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
						printf("%s(%d)\n", &memory_db[cur], score[i]);

						extra_print_cnt++;
						if (extra_print_cnt >= MAX_OUT_LINES)
							break;
					}
				}
			}
			else {	// 점수가 MAX_MINUS_POINT 이하인 줄로만 MAX_OUT_LINES 개를 출력하기에는 부족함.
				int extra_print_cnt = 0;

				for (i = 0; i < z; i++) {		// 일단 점수가 MAX_MINUS_POINT 이하인 줄들은 다 출력하고,
					if (score[i] <= MAX_MINUS_POINT) {
						if (eo_count >= 3) {	// 쿼리 길이가 세 글자 이상일 때에만 merged_final_degree[]를 만들기 때문
							printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
						}
						else {					// 쿼리 길이가 두 글자일 때에는 차수 계산이 필요 없으므로 mfd 배열을 만들지 않는다.
							printf("%7d   ", final[max_final][i]+1);
						}
						cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
						printf("%s(%d)\n", &memory_db[cur], score[i]);
					}
				}

				for (i = 0; i < z; i++) {		// 그래도 MAX_OUT_LINES를 다 채우지 못했을 것이므로 보충한다.
					if (score[i] > MAX_MINUS_POINT) {
						if (eo_count >= 3) {	// 쿼리 길이가 세 글자 이상일 때에만 merged_final_degree[]를 만들기 때문
							printf("%7d  %d   ", final[max_final][i]+1, mfd[i]);
						}
						else {					// 쿼리 길이가 두 글자일 때에는 차수 계산이 필요 없으므로 mfd 배열을 만들지 않는다.
							printf("%7d   ", final[max_final][i]+1);
						}
						cur = position_in_memory[final[max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
						printf("%s(%d)\n", &memory_db[cur], score[i]);

						extra_print_cnt++;
						if (extra_print_cnt >= (MAX_OUT_LINES - print_cnt))
							break;
					}
				}
			}
			if (eo_count >= 3)
				printf("------------- 텍스트 검색(쿼리 길이 3 이상. 명칭부분 부분 포함) 기반 통합 스코어링 결과(%d개) -------------\n", z);
			else
				printf("------------- 텍스트 검색(쿼리 길이 3 미만. 명칭부분 완전 포함) 기반 통합 스코어링 결과(%d개) -------------\n", z);
			free(score);
		}
	}
	else {						// 초성 검색 결과를 대상으로 통합 스코어링을 실시함.
		k = cho_final_count[cho_max_final];
		score = (short *)malloc(sizeof(short)*k);
		if (!score ) {
			fprintf(stderr, "integrated_scoring_and_output(): score[] 배열 할당 실패!\n");
			return;
		}
		for (i = 0; i < k; i++) {			// 검색된 모든 결과에 대해서.
			cur = position_in_memory[cho_final[cho_max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
			score[i] = CalMixedSimilarity(strArray, strCnt, (const unsigned char *)&memory_db[cur]);	// 검색 결과와 쿼리 간의 일치도를 계산한다.
		}
		quickSort7(score, 0, k-1);	// 감점 스코어 기준 오름차순 정렬
		printf("--------------- 초성 검색 기반 통합 스코어링 결과 ---------------\n");
		printf(" 줄번호  검색결과(스코어)\n");

		if (k > MAX_OUT_LINES)
			k = MAX_OUT_LINES;
		for (i = 0; i < k; i++) {
			printf("%7d  ", cho_final[cho_max_final][i]+1);
			cur = position_in_memory[cho_final[cho_max_final][i]];	// 그 줄번호에 해당하는 줄의 디비 내용이 저장되어 있는 위치
			printf("%s(%d)\n", &memory_db[cur], score[i]);
		}
		printf("------------- 초성 검색 기반 통합 스코어링 결과(%d개) -------------\n", cho_final_count[cho_max_final]);
		free(score);
	}
}

/*
 * 쿼리 문자열과 검색된 문자열 간의 유사도를 계산한다. 완성형 검색에서만 사용해야 함.
 * 리턴 값: 감점. 0: 일치, 클 수록 불일치도가 높은 것임.
 * 입력 str: 검색된 문자열. 쿼리 문자열은 ep_string[][]에 이미 있음
 */
//CalMixedSimilarity는 초성만 혹은 초성이 포함되었을경우에 쓰는 점수화 함수이고
//CalSimilarity는 완성형일 경우만 쓰는 점수화 함수 
int
CalSimilarity(const unsigned char *str, int no_str)
{
	///*
	//시간측정용
	//LARGE_INTEGER liCounter1, liCounter2, liFrequency;
	//QueryPerformanceFrequency(&liFrequency); // 주파수(1초당 증가되는 카운트수)를 구한다.

	int strCount;
	unsigned char strArray[MAX_QUERY_LEN][HANGUL_BYTE+1];
	strCount = StrToStrArray(str, strArray);

///////////////이하 JAY
	int i, j;
	int query_same[MAX_QUERY_LEN][5];
	int found_match_no;

	//한 배열로 넣기 변수
	int query_same_array[30];
	for(i=0; i<30; i++){
		query_same_array[i] = -5;
	}
	int query_same_array_isChanged[30];
	int array_match_no;

	//KEB 수식화 
	//JAY가 5로 한걸 5->10으로 늘려줌 그리고 MAX_BLOCK으로 상수화시킴
	//예제 질의어 브라운샤브샤브에서 오브플러우발생해서(블록이 24222개로 5개이상인 데이터가 있음)
	//중복단어가 많아질수록 이 배열의 크기 주의
	//중복단어: 중복글자아님 ex) 바로바로, 샤브샤브 등등
	int close_list[MAX_BLOCK];
	int estmLevel =0;//추정단계를 세분화 시키기 위한 변수
	//수식화끝
	

	//완성형을 조합형 코드로 변환하기 위한 과정의 변수
	unsigned short johapcode;
	unsigned int cho_code_q, jung_code_q, jong_code_q;
	unsigned int cho_code_d, jung_code_d, jong_code_d;
	char han_query[HANGUL_BYTE+1];				// 한글 처리 시 글자 하나를 담아 놓기 위한 공간. NULL 문자까지 포함하기 위해 +1.
	char han_data[HANGUL_BYTE+1];
	char c_han_query[HANGUL_BYTE+1];			// 완성형 글자를 조합형으로 바꿀 때 사용하기 위해 선언.
	char c_han_data[HANGUL_BYTE+1];

	//복원값 관련 변수
	int isChangedValue[30];						//복원된 값인지 여부저장		//30에는, 데이터의 최대개수를 저장한다.
	for(i=0; i<20; i++){
		isChangedValue[i] = 0;
	}
	char ChangedValue[30][3];					//복원된 값 값 저장			// Q:클 D:크 일때, '클'저장된다.
	

	
	//디버깅용 파일
	fprintf(fp, "%d.\t",no_str);
	for(i=0;i<strCount;i++){
		fprintf(fp, "%s",strArray[i]);
	}
	fprintf(fp, "\n");
	

	//이하, 매치되는 글자 위치 배열 작성 파트
	array_match_no = 1;

	for(i=0; i<eo_count; i++){
		found_match_no = 1;
		query_same_array[0] = -1;

		for(j=0; j<strCount; j++){
			// 1) 각 글자가 같을 경우
			if (!strcmp((const char *)eo_string[i], (const char *)strArray[j])){
				query_same[i][found_match_no]=j;	//쿼리의 i번째 글자와 매치되는 데이터 글자의 위치를 넣는다.
				found_match_no++;

				//한배열 안에 넣기
				query_same_array[array_match_no] = j;
				query_same_array_isChanged[array_match_no] = 0;	//0은 false
				array_match_no++;
			}
			// 2) 각 글자가 다를 경우
			else{
				if ((unsigned short)eo_string[i][0] >= 128 && (unsigned short)strArray[j][0] >= 128) {	// 쿼리와 데이터가 한글이면
					
					//이하, 데이터의 초/중/종성 추출
					han_data[0] = strArray[j][0]; 
					han_data[1] = strArray[j][1];
					han_data[2] = '\0';
					HangulCodeConvert(KS, han_data, c_han_data);
					johapcode = ((unsigned char)c_han_data[0] << 8) | ((unsigned char)c_han_data[1]); 
					cho_code_d = (johapcode >> 10) & 0x1f;		// 데이터의 초성 추출
					jung_code_d = (johapcode >> 5) & 0x1f;		// 데이터의 중성 추출
					jong_code_d = johapcode & 0x1f;				// 데이터의 종성 추출

					//이하, 쿼리의 초/중/종성 추출
					han_query[0] = eo_string[i][0]; 
					han_query[1] = eo_string[i][1];
					han_query[2] = '\0';
					HangulCodeConvert(KS, han_query, c_han_query);
					johapcode = ((unsigned char)c_han_query[0] << 8) | ((unsigned char)c_han_query[1]); 
					cho_code_q = (johapcode >> 10) & 0x1f;		// 쿼리의 초성 추출
					jung_code_q = (johapcode >> 5) & 0x1f;		// 쿼리의 중성 추출
					jong_code_q = johapcode & 0x1f;				// 쿼리의 종성 추출
					
					int cho_same = 0;	//초성이 같거나 ㅋ/ㄲ, ㅌ/ㄸ, ㅍ/ㅃ, ㅅ/ㅆ, ㅊ/ㅉ 일경우
					if(cho_code_q == cho_code_d
						|| ((cho_code_q == 3 && cho_code_d == 17) || (cho_code_q == 17 && cho_code_d == 3))		//ㅋ과 ㄲ
						|| ((cho_code_q == 6 && cho_code_d == 18) || (cho_code_q == 18 && cho_code_d == 6))		//ㅌ과 ㄸ
						|| ((cho_code_q == 10 && cho_code_d == 19) || (cho_code_q == 19 && cho_code_d == 10))	//ㅍ과 ㅃ
						|| ((cho_code_q == 16 && cho_code_d == 15) || (cho_code_q == 15 && cho_code_d == 16))	//ㅅ과 ㅆ
						|| ((cho_code_q == 11 && cho_code_d == 12) || (cho_code_q == 12 && cho_code_d == 11))	//ㅊ과 ㅉ
						){
							cho_same = 1;
					}

					//초성이 같거나 ㅋ/ㄲ, ㅌ/ㄸ, ㅍ/ㅃ, ㅅ/ㅆ, ㅊ/ㅉ 일경우
					if(cho_same == 1){
						//  (1) 중성이 같을 때
						if( jung_code_q == jung_code_d ){
							//종성도 같을 때
							if( jung_code_q == jung_code_d && jong_code_q == jong_code_d ){
								query_same[i][found_match_no]=j;	//쿼리의 i번째 글자와 매치되는 데이터 글자의 위치를 넣는다.
								found_match_no++;

								//한배열 안에 넣기
								query_same_array[array_match_no] = j;
								query_same_array_isChanged[array_match_no] = 1;	//1은 true (즉, 같은 글자가 아니라, 유사한 글자이다.)
								array_match_no++;
								//복원된 값이라는 것을 표시
								isChangedValue[j] = 1;
								//복원된 값 저장
								sprintf(ChangedValue[j], "%s", eo_string[i]);
							}
							//둘중 하나는 종성이 없고, 하나는 종성이 ㄹ일때
							if((jong_code_q == 9 && jong_code_d == 1) || ( jong_code_q == 1 && jong_code_d == 9)){ //1은 종성이 없음을, 9는 'ㄹ'종성을 의미
								query_same[i][found_match_no]=j;	//쿼리의 i번째 글자와 매치되는 데이터 글자의 위치를 넣는다.
								found_match_no++;

								//한배열 안에 넣기
								query_same_array[array_match_no] = j;
								query_same_array_isChanged[array_match_no] = 1;	//1은 true (즉, 같은 글자가 아니라, 유사한 글자이다.)
								array_match_no++;
								//복원된 값이라는 것을 표시
								isChangedValue[j] = 1;
								//복원된 값 저장
								sprintf(ChangedValue[j], "%s", eo_string[i]);
							}
							//수식화 추가부분(선농탕/설렁탕을 보정하기 위해)
							//종성이 ㄴ/ㄹ 혹은 ㄹ/ㄴ일때 
							else if((jong_code_q == 5 && jong_code_d == 9) || ( jong_code_q == 9 && jong_code_d == 5)){
								query_same[i][found_match_no]=j;	//쿼리의 i번째 글자와 매치되는 데이터 글자의 위치를 넣는다.
								found_match_no++;

								//한배열 안에 넣기
								query_same_array[array_match_no] = j;
								query_same_array_isChanged[array_match_no] = 1;	//1은 true (즉, 같은 글자가 아니라, 유사한 글자이다.)
								array_match_no++;
								//복원된 값이라는 것을 표시
								isChangedValue[j] = 1;
								//복원된 값 저장
								sprintf(ChangedValue[j], "%s", eo_string[i]);

								//추정단계 세분화
								estmLevel ++;//더 연관성이 없는 추정임을 나타남. 측정단계에서 감점하는데 사용
							}
							//종성없음 vs 종성 ㅅ
							else if((jong_code_q == 21 && jong_code_d == 1) || ( jong_code_q == 1 && jong_code_d == 21)){
								query_same[i][found_match_no]=j;	//쿼리의 i번째 글자와 매치되는 데이터 글자의 위치를 넣는다.
								found_match_no++;

								//한배열 안에 넣기
								query_same_array[array_match_no] = j;
								query_same_array_isChanged[array_match_no] = 1;	//1은 true (즉, 같은 글자가 아니라, 유사한 글자이다.)
								array_match_no++;
								//복원된 값이라는 것을 표시
								isChangedValue[j] = 1;
								//복원된 값 저장
								sprintf(ChangedValue[j], "%s", eo_string[i]);
							}
						}
						//  (2) 중성이 다를 때
						else{
							// (3) 종성이 같을 때
							if(jong_code_q == jong_code_d){ 
								//  초성이 ㅈ(14)/ㅉ(15)/ㅊ(16)일때,
								if( (cho_code_q == 14) || (cho_code_q == 15) || (cho_code_q == 16) ){
									if(((jung_code_q == 13 && jung_code_d == 19) || ( jung_code_q == 19 && jung_code_d == 13))		//13=ㅗ, 19=ㅛ
										||((jung_code_q == 3 && jung_code_d == 5) || ( jung_code_q == 5 && jung_code_d == 3))		//3=ㅏ, 5=ㅑ
										||((jung_code_q == 20 && jung_code_d == 26) || ( jung_code_q == 26 && jung_code_d == 20))	//20=ㅜ, 26=ㅠ
										||((jung_code_q == 7 && jung_code_d == 11) || ( jung_code_q == 11 && jung_code_d == 7))){	//7=ㅓ, 11=ㅕ	
											query_same[i][found_match_no]=j;	//쿼리의 i번째 글자와 매치되는 데이터 글자의 위치를 넣는다.
											found_match_no++;

											//한배열 안에 넣기
											query_same_array[array_match_no] = j;
											query_same_array_isChanged[array_match_no] = 1;	//1은 true (즉, 같은 글자가 아니라, 유사한 글자이다.)
											array_match_no++;
											//복원된 값이라는 것을 표시
											isChangedValue[j] = 1;
											//복원된 값 저장
											sprintf(ChangedValue[j], "%s", eo_string[i]);
									}
								}
								//둘중 하나는 중성이 ㅔ이고, 하나는 ㅐ일때
								if((jung_code_q == 4 && jung_code_d == 10) || ( jung_code_q == 10 && jung_code_d == 4)){	//4=ㅐ, 10=ㅔ
									query_same[i][found_match_no]=j;	//쿼리의 i번째 글자와 매치되는 데이터 글자의 위치를 넣는다.
									found_match_no++;

									//한배열 안에 넣기
									query_same_array[array_match_no] = j;
									query_same_array_isChanged[array_match_no] = 1;	//1은 true (즉, 같은 글자가 아니라, 유사한 글자이다.)
									array_match_no++;
									//복원된 값이라는 것을 표시
									isChangedValue[j] = 1;
									//복원된 값 저장
									sprintf(ChangedValue[j], "%s", eo_string[i]);
								}
								//둘중 하나는 중성이 ㅑ이고, 하나는 ㅛ일때
								else if((jung_code_q == 5 && jung_code_d == 19) || ( jung_code_q == 19 && jung_code_d == 5)){	//5=ㅑ, 19=ㅛ
									query_same[i][found_match_no]=j;	//쿼리의 i번째 글자와 매치되는 데이터 글자의 위치를 넣는다.
									found_match_no++;

									//한배열 안에 넣기
									query_same_array[array_match_no] = j;
									query_same_array_isChanged[array_match_no] = 1;	//1은 true (즉, 같은 글자가 아니라, 유사한 글자이다.)
									array_match_no++;
									//복원된 값이라는 것을 표시
									isChangedValue[j] = 1;
									//복원된 값 저장
									sprintf(ChangedValue[j], "%s", eo_string[i]);
								}
								//둘중 하나는 중성이 ㅏ이고, 하나는 ㅓ일때
								else if(jung_code_q == 3 && jung_code_d == 7 || ( jung_code_q == 7 && jung_code_d == 3)){ //3=ㅏ, 7=ㅓ
									query_same[i][found_match_no]=j;	//쿼리의 i번째 글자와 매치되는 데이터 글자의 위치를 넣는다.
									found_match_no++;

									//한배열 안에 넣기
									query_same_array[array_match_no] = j;
									query_same_array_isChanged[array_match_no] = 1;	//1은 true (즉, 같은 글자가 아니라, 유사한 글자이다.)
									array_match_no++;
									//복원된 값이라는 것을 표시
									isChangedValue[j] = 1;
									//복원된 값 저장
									sprintf(ChangedValue[j], "%s", eo_string[i]);
								}
								//수식화 추가부분(선농탕/설렁탕을 보정하기 위해)
								//그 중 종성이 ㄴ이고,       //수식화추가 혹은 종성이 ㅇ이고
								//둘중 하나는 중성이 ㅓ이고, 하나는 ㅗ일때
								//else if( (jong_code_q == 5 && jong_code_d == 5)|| (jong_code_q == 23 && jong_code_d == 23) ){ //5=ㄴ      ////수식화 추가 그중 종성이 ㅇ인것도 포함
								else if( (jong_code_q == 5 && jong_code_d == 5)){ //5=ㄴ  
									if( ( jung_code_q == 7 && jung_code_d == 13 ) || ( jung_code_q == 13 && jung_code_d == 7) ){ //7=ㅓ, 13=ㅗ
										query_same[i][found_match_no]=j;	//쿼리의 i번째 글자와 매치되는 데이터 글자의 위치를 넣는다.
										found_match_no++;

										//한배열 안에 넣기
										query_same_array[array_match_no] = j;
										query_same_array_isChanged[array_match_no] = 1;	//1은 true (즉, 같은 글자가 아니라, 유사한 글자이다.)
										array_match_no++;
										//복원된 값이라는 것을 표시
										isChangedValue[j] = 1;
										//복원된 값 저장
										sprintf(ChangedValue[j], "%s", eo_string[i]);
										
										//추정단계 세분화
										estmLevel ++;//더 연관성이 없는 추정임을 나타남. 측정단계에서 감점하는데 사용
									}
								}
							}
						}
					}
					else{
						//  (1-2) 초성 하나가 ㅎ(20)/나머지가 ㅍ(19)일때,
						if(cho_code_q == 20 && cho_code_d == 19){		//쿼리초성 ㅎ(20), 데이터초성 ㅍ(19)
							if((jung_code_q == 22 && (jung_code_d == 4 || jung_code_d == 10))	//22=ㅞ, 4=ㅐ 10=ㅔ
								|| (jung_code_q == 14 && jung_code_d == 3)						//14=ㅘ, 3=ㅏ
								|| (jung_code_q == 20 && jung_code_d == 27)						//20=ㅜ, 27=ㅡ
								|| (jung_code_q == 23 && jung_code_d == 29)						//23=ㅟ, 29=l
								){
									query_same[i][found_match_no]=j;	//쿼리의 i번째 글자와 매치되는 데이터 글자의 위치를 넣는다.
									found_match_no++;

									//한배열 안에 넣기
									query_same_array[array_match_no] = j;
									query_same_array_isChanged[array_match_no] = 1;	//1은 true (즉, 같은 글자가 아니라, 유사한 글자이다.)
									array_match_no++;
									//복원된 값이라는 것을 표시
									isChangedValue[j] = 1;
									//복원된 값 저장
									sprintf(ChangedValue[j], "%s", eo_string[i]);
							}
						}else if(cho_code_q == 19 && cho_code_d == 20){	//쿼리초성 ㅍ(19), 데이터초성 ㅎ(20)
							if(((jung_code_q == 4 || jung_code_q == 10) && jung_code_d == 22)	//22=ㅞ, 4=ㅐ 10=ㅔ
								|| (jung_code_q == 3 && jung_code_d == 14)						//14=ㅘ, 3=ㅏ
								|| (jung_code_q == 27 && jung_code_d == 20)						//20=ㅜ, 27=ㅡ
								|| (jung_code_q == 29 && jung_code_d == 23)						//23=ㅟ, 29=l
								){
									query_same[i][found_match_no]=j;	//쿼리의 i번째 글자와 매치되는 데이터 글자의 위치를 넣는다.
									found_match_no++;

									//한배열 안에 넣기
									query_same_array[array_match_no] = j;
									query_same_array_isChanged[array_match_no] = 1;	//1은 true (즉, 같은 글자가 아니라, 유사한 글자이다.)
									array_match_no++;
									//복원된 값이라는 것을 표시
									isChangedValue[j] = 1;
									//복원된 값 저장
									sprintf(ChangedValue[j], "%s", eo_string[i]);
							}
						}
						
						//수식화 추가부분(선농탕/설렁탕을 보정하기 위해)
						if((cho_code_q == 4 && cho_code_d == 7) || (cho_code_q == 7 && cho_code_d == 4)){//초성 ㄴ/ㄹ or  ㄹ/ㄴ
							if((jong_code_q == 23 && jong_code_d == 23)){ //종성이 둘다 ㅇ
								if( ( jung_code_q == 7 && jung_code_d == 13 ) || ( jung_code_q == 13 && jung_code_d == 7) ){// 중성 ㅗ/ㅓ or ㅓ/ㅗ
									query_same[i][found_match_no]=j;	//쿼리의 i번째 글자와 매치되는 데이터 글자의 위치를 넣는다.
									found_match_no++;

									//한배열 안에 넣기
									query_same_array[array_match_no] = j;
									query_same_array_isChanged[array_match_no] = 1;	//1은 true (즉, 같은 글자가 아니라, 유사한 글자이다.)
									array_match_no++;
									//복원된 값이라는 것을 표시
									isChangedValue[j] = 1;
									//복원된 값 저장
									sprintf(ChangedValue[j], "%s", eo_string[i]);
								}
							}
						}
					}
					cho_same = 0;	//초기화
				}
			}	

			//데이터의 마지막 글자를 비교할 때
			if(j==(strCount-1)){
				if(found_match_no == 1){
					query_same[i][0] = 1;	//매치되는 글자가 없어도, '한배열'에 넣을때 자리를 1개 차지하므로, 1을 입력한다.
					query_same[i][1] = -1;
					//한배열 안에 넣기
					query_same_array[array_match_no] = -1;
					array_match_no++;
				}else{
					query_same[i][0]=found_match_no-1;	//query_same[i][0] 에는 쿼리의 i번째 글자와 매치되는 데이터 글자의 갯수가 저장된다.
				}
			}
		}
	}
	
	//한 배열 안에 넣을때 최대값
	int max_array = array_match_no;
	if(max_array > 30){
		printf("\n\n배열의 최대값이 30이므로, 더이상 계산할 수 없습니다.\n\n");
		return 0;
	}

	/////////// <<<< 점수 측정 시작 >>>> ///////////
	int simple_method_score = 0;	// [심플] 방법 총점수

	int pure_nf = 0;//쿼리에 없는 글자수


	//수식화부분
	//아래부터는 JAY가 한 부분을 일부 가져왔고 그 흐름을 따르지만
	//많이 수정됨
	
	//(1)질의어/
	int Qmatch =0;//질의어 글자 중 데이터에 있는 질의어 글자의 갯수
	int Qrest =0;//질의어에 중복글자가 있을 경우 보정
	//Q:뽀로로테마    D1:아로마테크빌딩(Qmatch=4) > D2:뽀로로어린이집(Qmatch=3)
	//문자열유사도에서는 추정단계 때문에 집합기반 연산에서 쓴 중복글자 아이디를 사용하지 못했기 때문에 생긴 문제
		//데이터에 중복글자가 있을 경우
		//Q:뽀로테마      D1:뽀로로어린이집 일 경우엔 현재 소스상태에서는 문제 없음
	int Qestm = 0;//추정된 글자수
	
	//(2)데이터길이
	int Dlen =0;//데이터의 길이

	//(3)외자
	int Qalone =0;//외자갯수
	int isNotSeq =0;//외자순서

	//(4)다자블록
	int BLcount=0;//블록갯수
	int BLorder=0;//블록순서
	int FstBL =0;//첫블록여부

	
	for(i=0;i<eo_count; i++){
		if(query_same[i][1] == -1){
			pure_nf++;
		}
	}

	
	//수식화 : 일단 이걸 지우면 어떤 문제가 생길지 몰라서 두고 안에 simple계산부분만 주석처리
	if((eo_count >4) && (pure_nf >= (eo_count+2))){//eo_count-3이였는데 수정 -> 이것때문에 Q:뽀로로테마 D:뽀로로어린이집 이 밀렸음
		//simple_method_score +=2000;
	}else if((eo_count <=4) && (pure_nf >2 )){
		//simple_method_score +=2000;
	}else{
	

		//디버깅용 파일
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
		//(4)다자블록
		//1. 블록구하기
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

		//이하, 120815
		int frag_start[10], frag_end[10];	//fragmentation 비교 위한 인자값
		int iii = 0;
		/////////////

		//이하, 뭉치(block) 찾는 부분 (iii는 block의 갯수)
		int m = 0;
		
		for(i=0; i<max_array+2; i++){ 
			if(close_data[i]>0){
				no_close = 0;
				do_caculate_close(close_data, i);
				close_list[++m] = no_close+1;		//close_list[0] : 연속매칭 뭉치가 총 몇개인지 기입한다.
													//          [1~5] : 하나의 연속매칭 뭉치가 총 몇글자가 연속되는지 기입한다.
				
				//이하, fragmentation정보 입력
				frag_start[iii] = query_same_array[i];
				frag_end[iii] = query_same_array[i] + no_close;
				iii++;
			}
		}
		
		close_list[0] = m;
		
		


		//2. 겹치는 블록 제거
		int mostbigblock =0;//가장 큰 크기의 블록
		int tmp_close_list[MAX_BLOCK];

		int fn, fn2, fn3;//for루프 int변수들
		
		//tmp_close_list배열 초기화 [0]:블록갯수 [1~]:각블록크기
		for(fn =0; fn<MAX_BLOCK; fn++)
			tmp_close_list[fn]=0;
		for(fn =0; fn<m; fn++)
			tmp_close_list[fn+1]=close_list[fn+1];

		//tmp_strArray배열 초기화   
		int tmp_strArray[30];
		for(fn =0; fn<30; fn++)
			tmp_strArray[fn]=-1;

		//블록간 겹침을 확인하기 위한 변수들
		int blstart =0;
		bool isdubble = false; 

		//디버깅용 삭제블록
		int delBL[5];
		for(fn=0; fn<5; fn++)
			delBL[fn]=0;
		int index_delBL=0;


		int delblock =0;
		for(fn =0; fn<m; fn++){
			mostbigblock=0;

			//가장 큰 크기의 블록부터 고려
			for(fn2 =0; fn2<m; fn2++){
				if(mostbigblock < tmp_close_list[fn2+1]){
					mostbigblock = tmp_close_list[fn2+1];
					delblock = fn2+1;
				}
			}
			
			//겹칩여부 확인
			blstart= frag_start[delblock-1];
			isdubble = false; 
		
			for(fn2 =0; fn2<mostbigblock; fn2++){
				//고려하고 있는 블록이 이전에 고려한 블록과 겹침 여부 확인
				//한글자라도 겹치면 isdubble이 true가 되어 삭제대상블록이됨
				//참고 : 가장 큰 첫 블록일 경우에는 if문안으로 안들어감
				if(tmp_strArray[blstart]==1)
					isdubble = true;
				blstart++;
			}

			//겹침여부에 따른 삭제대상추출
			blstart= frag_start[delblock-1];
			
			if(isdubble == true){//삭제대상 블록일 경우
				//디버깅용 삭제블록
				delBL[index_delBL] = delblock-1;
				index_delBL++;
			}
			else{//삭제 대상 블록이 아닐 경우
				for(fn3 =0; fn3<mostbigblock; fn3++){
					tmp_strArray[blstart]=1;
					blstart++;
				}
			}

			//작업끝난 블록 tmp_close_list에서 삭제
			tmp_close_list[delblock]=0;
		}


		//블록 삭제
		int delcount = index_delBL;
		
		if(delcount>0){//0이상이다 == 삭제 블록이 있다
			int tmp_frag_start[10];
			int tmp_frag_end[10];
			
			//변수초기화
			for(fn=0; fn<MAX_BLOCK; fn++){
				tmp_frag_start[fn]=0;
				tmp_frag_end[fn]=0;
				tmp_close_list[fn]=0;
			}
			index_delBL=0;

			//삭제대상 블록 삭제표시하기
			for(fn=0;fn<delcount;fn++){
				frag_start[delBL[index_delBL]]=-1;
				frag_end[delBL[index_delBL]]=-1;
				close_list[delBL[index_delBL]+1]=-1;
				index_delBL++;
			}

			//tmp_배열들에 값을 저장시킴
			//삭제된 블록들이 배열 중간에 있을 경우 중간이 비게 되니까 비는곳없애기 위함
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

			//모든 블록 변수들 블록삭제 적용
			iii= iii-delcount;
			m = m-delcount;
			close_list[0] = m;

			for(fn=0;fn<m;fn++){
				frag_start[fn]=tmp_frag_start[fn];
				frag_end[fn]=tmp_frag_end[fn];
				close_list[fn+1]=tmp_close_list[fn+1];
			}

		}

		//블록갯수 결과
		BLcount = close_list[0];



		//3. 블록순서 구하기
		if(close_list[0]>1){
			int bl_ct = close_list[0]-1;

			for(int ti=0; ti<bl_ct;ti++)
				if(frag_start[ti]>frag_start[ti+1])
					//순서대로 안된 경우, 블록순서가 뒤바뀌어있는 경우
					BLorder++;
		}

		//(4)다자블록 끝
		/////////////////////////////





		////////////////////
		//(3)외자 


		//각 블록이 쿼리에 포함되었는지 혹은 Not_Found인지 판단 + 낱자인 글자도 고려하는 부분
		// 이하, [심플 : 1] NOT_FOUND + [심플 : 2] 복원값 계산용 과정
		int isMatching = -1;
		int start = 0;
		int isInQuery[MAX_QUERY_LEN];
		for(i=0; i<MAX_QUERY_LEN; i++){
			isInQuery[i] = 0;
		}
		int isAlreadyMatch[30];		//낱자도 NOT_FOUND에 추가하는 변수		//30은 임의의 수. 데이터 글자의 최대개수만큼 필요하다.
		for(i=0; i<30; i++){
			isAlreadyMatch[i] = 0;
		}
		int t;
		int changed_value = 0;
		int isSame = 0;
		int no_changed_value = 0;	//보유하고 있는 복원값의 개수
		int tmp_no_changed_value = 0;

		for(k=0; k<eo_count; k++){
			for(i=0; i<iii; i++){
				//이하, block의 첫번째 글자 비교
				//원본 글자와 같을 때
				if( !strcmp((const char *)eo_string[k], (const char *)strArray[frag_start[i]]) ){
					start= k+1;
					isSame = 1;
				}
				//복원된 글자와 같을 때
				else if(isChangedValue[frag_start[i]] == 1){
					if( !strcmp((const char *)eo_string[k], (const char *)ChangedValue[frag_start[i]]) ){	
						start= k+1;
						isSame = 1;
						tmp_no_changed_value++;
					}
				}

				//이하, block의 나머지 글자 비교
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

		//낱자도 NOT_FOUND에 추가하는 부분
		for(i=0; i<eo_count; i++){
			if(isInQuery[i]==0){
				for(j=0; j<strCount; j++){
					if(isAlreadyMatch[j]==0){
						if( !strcmp((const char *)eo_string[i], (const char *)strArray[j]) ){
							isInQuery[i] = 2;
							//수식화추가
							isAlreadyMatch[j]=2;
						}else if(isChangedValue[j] == 1){
							if(!strcmp((const char *)eo_string[i], (const char *)ChangedValue[j]) ){	
								isInQuery[i] = 2;
								no_changed_value++;
								//수식화추가
								isAlreadyMatch[j]=2;
							}
						}
					}
				}
			}
		}
		
		//데이터 글자 중 질의어에 있는 데이터 글자의 갯수 구하기
		//중복글자 문제 해결을 위해 추가 Q:뽀로로테마 D:아로마테크빌딩
		for(int tj=0; tj<30;tj++){
			if(isAlreadyMatch[tj]!=0)
				Qrest++;
		}


		////////////////////////////////[외자위치 확인하는 부분//120919]
		int front_isSequential = 0;
		int rear_isSequential = 0;
		int isNotSequential = eo_count;		//데이터상에 있는 쿼리글자의 위치가 순서대로 있는지 여부를 확인하여 > 총 개수 저장
	
		int isSequential[MAX_QUERY_LEN];	//위치가 순서대로 매치되었는지 여부 확인용.
		for(i=0; i<MAX_QUERY_LEN; i++){
			isSequential[i] = 0;
		}
	
		for(i=0; i<eo_count; i++){
			if(isInQuery[i] == 2){	//외자글자일때만 위치 확인함
				for(j=1; j<=query_same[i][0]; j++){
					//앞글자와 비교
					if(i==0){	//첫글자일 경우, 마지막글자를 본다.
						if(front_isSequential==0){
							if(query_same[eo_count-1][1] == -1){			//마지막 글자가 데이터에 없는 글자일 때
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
							if((query_same[i-1][1] == -1) && (i>1)){			//이전 글자가 데이터에 없는 글자일 때
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
				
					//뒷글자와 비교
					if(i==eo_count-1){//마지막글자일 경우, 첫글자와 비교한다.
						if(rear_isSequential==0){
							if(query_same[0][1] == -1){	//첫글자가 데이터에 없는 글자일 때
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
							if((query_same[i+1][1] == -1) && (i<eo_count-2)){	//이후 글자가 데이터에 없는 글자일 때
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
			}else if(isInQuery[i]==1){	//block인 경우 
				isNotSequential--;
				isSequential[i] = 1;
			}else if(isInQuery[i]==0){	//글자가 없는 경우
				isNotSequential--;
			}
		}
		fprintf(fp, "\t[순서틀린거]%d", isNotSequential);
		fprintf(fp, "\n");
		for(i=0; i<eo_count; i++){
			fprintf(fp, "[%d]", isSequential[i]);
		}
		//디버깅용 파일//////////////////////////////////
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
		//(3) 외자 끝
		/////////////////////////////



		///////////////////////
		//(4)다자블록
		//FstBL 첫블록여부확인
		int isInBlock = 0;
		for(i=0; i<iii; i++){//데이터 첫글자 확인
			if(frag_start[i]==0){
				isInBlock = 1;
			}
		}
		if(isInQuery[0] == 1){//질의어 첫글자 확인
			if(isInBlock == 1){
				isInBlock = 2;	//쿼리/데이터 첫글자 모두 block에 포함
			}else{
				isInBlock = 1;	//쿼리or데이터 중 하나의 첫글자가 block에 포함
			}
		}

		//외자 첫글자 확인 추가???
		//isAlready로 확인후 isInQuery에서 다시 확인
		//isInAlone추가, Fstalone추가

		if(isInBlock == 1){
			FstBL = 25;
		}else if(isInBlock ==2){
			FstBL = 50;
		}
		//(4)다자블록 끝
		////////////////////////


		/////////////////////
		//(1)질의어
		
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
		//Qmatch<Qrest일 경우는 어차피 Qmatch값을 이용하는거니까 고려하지 않아도됨
		
		Qestm = no_changed_value+estmLevel;

		//(1)질의어 끝
		//////////////////////

		
		//////////////////////
		//(2)데이터길이
		if(strCount>eo_count)
			Dlen = strCount-eo_count;
		else
			Dlen = eo_count-strCount;
		//(2)데이터길이끝
		//////////////////////
		
	}


		//최종점수 출력 //디버깅용 파일
		
		simple_method_score= 0;
		
		/* 10의4승
		simple_method_score = simple_method_score - Qmatch*2*10000/eo_count + (2*10000/eo_count)*(Qestm/eo_count) 
			+ 1000*Dlen/eo_count
			+ Qalone*200 + BLcount*100 +BLorder*100  +isNotSeq*2000 
			-FstBL*100;//*/
		///*10의 2승
		simple_method_score = simple_method_score - Qmatch*2*100/eo_count + (2*100/eo_count)*(Qestm/eo_count) 
			+ 10*Dlen/eo_count
			+ Qalone*2 + BLcount +BLorder  +isNotSeq*20 
			-FstBL;//*/

		//고은혜봐라
		/*(1) 질의어
		simple_method_score = simple_method_score - Qmatch*2*100/eo_count + (2*100/eo_count)*(Qestm/eo_count);//*/

		/*(2) 데이터길이
		simple_method_score = -200;
		simple_method_score = simple_method_score + 10*Dlen/eo_count;//*/

		/*(3) 외자
		simple_method_score = -200;
		simple_method_score = simple_method_score + Qalone*2 +isNotSeq*20;//*/

		/*(4) 블록
		simple_method_score = -200;
		simple_method_score = simple_method_score + BLcount +BLorder-FstBL;//*/

	
		fprintf(fp, "\n>> %d\n",simple_method_score);

		return simple_method_score;

}

/*
 * 데이터 문자열의 어느위치서부터 얼마나 매칭되는지 계산하는 함수
 * 입력 do_close_data[]: 다익스트라 알고리즘의 배열을 이용하여 1024의 값을 가진(연속매칭되는) 배열값만 추출해낸 배열
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
 * 쿼리 문자열과 검색된 문자열 간의 유사도를 계산한다. 
 * 리턴 값: 감점. 0: 일치, 클 수록 불일치도가 높은 것임.
 * 입력 str: 검색된 문자열, query_Array: 쿼리 문자열(온전한 완성형 문자와 초성 문자가 섞여 있을 수 있음)
 *      query_count: 쿼리의 어절 개수
 */
int
CalMixedSimilarity(const unsigned char query_Array[MAX_QUERY_LEN][HANGUL_BYTE+1], int query_count, const unsigned char *str)
{

	int qi = 0, ri = 0, strCount;
	unsigned char strArray[MAX_QUERY_LEN][HANGUL_BYTE+1];
	char han[HANGUL_BYTE+1];			// 한글 처리 시 글자 하나를 담아 놓기 위한 공간. NULL 문자까지 포함하기 위해 +1.
	char c_han[HANGUL_BYTE+1];			// 완성형 글자를 조합형으로 바꿀 때 사용하기 위해 선언.
	unsigned short johapcode;
	int minus_point = 0;
	int in_one_eo, fPartiallyMatched;
	int keep_matching = 0;
	int isSame, fBothWanCode;
	unsigned char cho_code;

	strCount = StrToStrArray(str, strArray);	// 검색된 결과 문자열일테니 이를 어절 단위로 분리한다.
	minus_point = (query_count > strCount) ? (query_count - strCount) : (strCount - query_count);	// 길이 차이만큼 감점한다.
	in_one_eo = 0;
	fPartiallyMatched = 0;

	while (1) {
		if (query_Array[qi][2] == '\1') {	// 조합형 코드면 초성만 있는 것임
			if ((unsigned short)strArray[ri][0] >= 128) {	// 한글이면
				han[0] = strArray[ri][0]; 
				han[1] = strArray[ri][1];
				han[2] = '\0';
				HangulCodeConvert(KS, han, c_han);
				johapcode = ((unsigned char)c_han[0] << 8) | ((unsigned char)c_han[1]); 
				cho_code = (johapcode >> 10) & 0x1f;		// 초성을 추출한다.
				isSame = query_Array[qi][0] == cho_code;
			}
			else // 쿼리에는 초성 코드와 비교하는 검색 결과 문자가 한글이 아님. 무조건 다를 것임.
				isSame = 0;
			fBothWanCode = 0;
		}
		else {								// (query_Array[qi][2] == '\0'), 즉 온전한 완성형 코드면
			isSame = !strcmp((const char *)query_Array[qi], (const char *)strArray[ri]);
			fBothWanCode = 1;	// 쿼리와 결과문자열 에 있는 이 문자가 둘 다 완성형 코드임.
		}

		if (isSame) {			// 같은 글자이면
			qi++; ri++;			// 감점 없이 둘 다 전진한다.
			if (keep_matching)	// 연속해서 매칭된 글자가 등장하고 있으면 우대한다.
				keep_matching *= 2;
			else
				keep_matching = 1;	// '연속 매칭 여부를 나타내는 플래그'를 on.
			minus_point -= keep_matching;
			in_one_eo = 0;	// 'query_Array[qi] 글자가 strArray[ri] 내에서 아직 발견 안 됨'을 의미하는 플래그를 리셋함.
			fPartiallyMatched = 0;
		}
		else {	// 다른 글자면 감점하고, 결과 인덱스만 전진한다.
			keep_matching = 0;	// '연속 매칭 여부를 나타내는 플래그'를 off.
			minus_point += 6;
			if (fBothWanCode && is_partially_matched((const char *)query_Array[qi], (const char *)strArray[ri])) {
				fPartiallyMatched = 1;
				minus_point -= 6;
			}
			ri++;
			if (++in_one_eo >= strCount) {	// query_Array[qi] 글자가 결국 strArray[ri] 내에서 발견 안 됨.
				if (!fPartiallyMatched)
					minus_point += 20;
				qi++;
				in_one_eo = 0;
				fPartiallyMatched = 0;
			}
		}
		if (ri >= strCount) { 	// 결과 인덱스를 전진했는데 끝문자를 넘어갔으면 wrap around 한다.
			ri = 0;
			keep_matching = 0;	// wrap around 되었으니까 연속 매칭은 끝난 것임
		}
		if (qi >= query_count)	// 쿼리 인덱스를 전진했는데 쿼리 끝문자를 넘어갔으면 종료한다.
			break;
	}

	return minus_point;

}

/*
 * 쿼리 문자열에서 어절 단위로 분리하여 리턴한다. 완성형 검색에서만 사용함.
 * 초성이 포함된 문자열에 대해서는 사용하지 말 것! 구분 못 함.
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
		if ((unsigned int)t_char >= 128) {	// 한글
			strArray[str_count][0] = t_char;
				
			t_char = str[cur++];
			len--;

			strArray[str_count][1] = t_char;
			strArray[str_count++][2] = '\0';
		}
		else { // 블랭크를 포함한 일반 아스키 문자
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
 * 쿼리 문자열을 어절 단위로 분리하여 리턴한다.
 * 완성형과 초성이 포함되어 있는 쿼리 문자열이어도 작동함.
 * 쿼리 문자열에 대해서만 호출해야 함. 쿼리에 초성이 포함되어 있으면
 * 조합형 초성 코드로 변환하여 저장함.
 * 입력: str: 쿼리 문자열 원본. 출력: strArray: 각 어절을 별도로 담는다.
 */
int
StrToEoArray(const unsigned char *str, unsigned char strArray[MAX_QUERY_LEN][HANGUL_BYTE+1])
{
	int i, j, str_count;
	union code {
		unsigned short fullcode;
		unsigned char halfcode[2];
	} code;
	char han[HANGUL_BYTE+1];			// 한글 처리 시 글자 하나를 담아 놓기 위한 공간. NULL 문자까지 포함하기 위해 +1.
	char c_han[HANGUL_BYTE+1];			// 완성형 글자를 조합형으로 바꿀 때 사용하기 위해 선언.
	unsigned short johapcode;

	str_count = 0;

	for (i = 0; str[i] != '\0'; i++) {
		if ((unsigned int)str[i] >= 128) {		// 한글
			code.halfcode[1] = str[i];
			code.halfcode[0] = str[i+1];
			for (j = 0; j < 19; j++) {		// 들어온 코드가 완성형 초성 코드에 있는 것인지 검사
				if ((unsigned int)code.fullcode == Johap[j])
					break;
			}
			if (j < 19)	{	// 중간에 빠져 나온 것이므로 완성형코드로 초성이 입력된 것임. 조합형으로 바꿔서 strArray에 넣는다.
				han[0] = str[i];
				han[1] = str[i+1];
				han[2] = '\0';
				HangulCodeConvert(KS, han, c_han);
				johapcode = ((unsigned char)c_han[0] << 8) | ((unsigned char)c_han[1]); 
				strArray[str_count][0] = (johapcode >> 10) & 0x1f;	// 초성을 추출한다.
				strArray[str_count][1] = '\0';						// filler
				strArray[str_count++][2] = '\1';					// 조합형 초성 코드라는 표시
			}
			else {			// 완성형 초성 코드가 아니므로 완전한 완성형 글자임(디비에 없는 글자일 가능성은 0%. 이미 걸렀기 때문)
				strArray[str_count][0] = code.halfcode[1];
				strArray[str_count][1] = code.halfcode[0];
				strArray[str_count++][2] = '\0';
			}
			i++;	// 한글이니까 한 바이트 더 전진
		}
		else {	// 일반 아스키 코드 문자
			if (isalpha(str[i]))	// 알파벳이면 대문자로 바꿔 넣고
				strArray[str_count][0] = toupper(str[i]);
			else					// 아니면 그대로 넣는다.
				strArray[str_count][0] = str[i];
			strArray[str_count][1] = '\0';		// filler
			strArray[str_count++][2] = '\0';	// 조합형 초성이 아니라는 표시
		}
	} // end of for (i = 0; query_str[i] != '\0'; i++)

	return str_count;
}

// 입력: 쿼리 문자열
// 출력: -1: 모두 완성형 글자만 있는 경우
//        1: 모두 초성 글자만 있는 경우
//		  0: 완성형과 초성이 섞였는데 완성형 글자가 두 글자 이상이어서 완성형 검색을 먼저 한 후 초성 검색을 하라.
//		  2: 완성형과 초성이 섞였는데 완성형 글자가 두 글자 미만이어서 초성 검색을 먼저 한 후 완성형 검색을 하라.
//		 쿼리에 같은 완성형 글자가 여러 개 포함된 경우 맨 처음 글자를 제외하고 모두 초성으로 바뀐다.
int
CheckIfChosungIncluded(char *query_str)
{
	int wansung_code_count = 0;			// 추출한 완성형 글자 개수
	int chosung_code_count = 0;			// 추출한 초성 글자 개수
	union code {
		unsigned short fullcode;
		unsigned char halfcode[2];
	} code;
	register int i, j, k;
	int t_int, t_int_prev;
	Node *result;
	char han[HANGUL_BYTE+1];			// 한글 처리 시 글자 하나를 담아 놓기 위한 공간. NULL 문자까지 포함하기 위해 +1.
	char c_han[HANGUL_BYTE+1];			// 완성형 글자를 조합형으로 바꿀 때 사용하기 위해 선언.
	unsigned short johapcode;
	unsigned short cho_code;

	for (i = 0; query_str[i] != '\0'; i++) {
		if ((unsigned int) query_str[i] >= 128) {
			code.halfcode[1] = query_str[i];
			code.halfcode[0] = query_str[i+1];
			//printf("0x%x ", code.fullcode);
			for (j = 0; j < 19; j++) {		// 들어온 코드가 완성형 초성 코드에 있는 것인지 검사
				if ((unsigned int)code.fullcode == Johap[j])
					break;
			}
			if (j < 19)	// 중간에 빠져 나온 것이므로 초성이 입력된 것임.
				chosung_code_count++;
			else {		// 초성 코드에 없었으므로 완성형 글자가 입력되었거나 디비에 없는 글자가 입력된 것임.
				t_int = (int)(*(unsigned short*)&query_str[i]);
				if (result = rbt2.search(t_int)) {	// 온전한 완성형 글자임

					//중복글자문제 해결부분
					//////////////////////////////////JAY
					// 이전엔 여기서
					// wansung_code_count++;
					// 하고 끝냈는데, 이제는 그러면 안 되고,
					// 자신하고 같은 글자가 앞 부분에 있는지 확인하고 있으면 자신을 초성 코드로 바꾼다.
					/*
					for (k = 0; k < i; k++) {	// 쿼리에 이 글자의 앞부분에 자기와 같은 글자가 있는지 검사한다.
						if ((unsigned int)query_str[k] >= 128) {
							t_int_prev = (int)(*(unsigned short*)&query_str[k]);
							if (t_int == t_int_prev) {	// 같은 글자이면 쿼리 글자를 초성으로 바꾼다.
								han[0] = query_str[i];
								han[1] = query_str[i+1];
								han[2] = '\0';
								HangulCodeConvert(KS, han, c_han);
								johapcode = ((unsigned char)c_han[0] << 8) | ((unsigned char)c_han[1]); 
								cho_code = (johapcode >> 10) & 0x1f;	// 초성 추출
								if (cho_code < 0x02 || cho_code > 0x14) {
									fprintf(stderr, "CheckIfChosungIncluded(): 쿼리에서 비정상 초성 글자 '%s' 발견! 무시합니다.\n", han);
									k++;
									continue;
								}
								//(*(unsigned short*)&query_str[i]) = (unsigned short)Johap[cho_code-2];	// 초성만으로 이루어진 완성형 코드를 얻어 쿼리를 대체한다.
								query_str[i] = (unsigned char)(Johap[cho_code-2] >> 8);	// 초성만으로 이루어진 완성형 코드를 얻어 쿼리를 대체한다.
								query_str[i+1] = (unsigned char)(Johap[cho_code-2] & 0x00ff);	// 초성만으로 이루어진 완성형 코드를 얻어 쿼리를 대체한다.
								break;
							}
							k++;
						}
					}
					if (k >= i)	// 같은 글자가 없으면 완성형 글자수를 하나 증가시킨다.
						wansung_code_count++;
					else		// 완성형 글자가 중복으로 인해 초성 완성형 코드로 바뀌었으면 초성 글자수 하나를 증가시킨다.
						chosung_code_count++;
					*/
					//////////////////////////////////JAY

					wansung_code_count++;	//같은 글자가 여러개나올때, CalMixedSimilarity가 되므로, JAY가 할땐 그냥 완성형으로 처리한다.

				}
			}
			i++;	// 한글이니까 한 바이트 더 전진
		}
		else
			wansung_code_count++;
	} // end of for (i = 0; query_str[i] != '\0'; i++)
	//printf("\n");
	if (chosung_code_count == 0)	// 모두 완성형 글자만 들어온 경우
		return -1;
	if (wansung_code_count == 0)	// 모두 초성 글자만 들어온 경우
		return 1;
	// 여기까지 오면, 쿼리가 널이거나(이건 논외로 함) 쿼리에 완성형, 초성이 섞여 있음. 
	if (wansung_code_count >= 2)	// 섞였는데 완성형 글자가 2개 이상이면 완성형 검색 후 그 결과와 초성 검색 결과를 합친 줄번호들에 대고 스코어링을 하라.
		return 0;
	else							// 섞였는데 완성형 글자가 2개 미만이면 초성 검색 후 그 결과에 대고 스코어링을 하라.
		return 2;

	return 3;
}

// 입력: qs: 쿼리에 있는 어떤 글자(완성형), rs:검색 결과로 나온 어떤 줄에 있는 한 글자(완성형)
// 결과값: 두 글자가 같지는 않은데 초성, 중성, 종성 중 두 개 이상이 같으면 1을 리턴한다.
//         초성과 중성만 있는 경우는 중성(모음)이 같으면 1을 리턴한다.
int is_partially_matched(const char *qs, const char *rs)
{
	char c_han[HANGUL_BYTE+1];			// 완성형 글자를 조합형으로 바꿀 때 사용하기 위해 선언.
	unsigned short johapcode;
	unsigned short qs_cho, qs_moeum, qs_jong;
	unsigned short rs_cho, rs_moeum, rs_jong;
	int point = 0;

	if ((unsigned short)qs[0] < 128 || (unsigned short)rs[0] < 128)		// 둘 중 하나라도 한글이 아니면 0을 리턴.
		return 0;

	// qs에서 초성, 중성, 종성을 추출한다.
	HangulCodeConvert(KS, qs, c_han);
	johapcode = ((unsigned char)c_han[0] << 8) | ((unsigned char)c_han[1]); 
	qs_cho = (johapcode >> 10) & 0x001f;	// 초성을 추출한다.
	qs_moeum = (johapcode >> 5) & 0x001f;	// 중성을 추출한다.
	qs_jong = johapcode & 0x001f;			// 중성을 추출한다.

	// rs에서 초성, 중성, 종성을 추출한다.
	HangulCodeConvert(KS, rs, c_han);
	johapcode = ((unsigned char)c_han[0] << 8) | ((unsigned char)c_han[1]); 
	rs_cho = (johapcode >> 10) & 0x001f;	// 초성을 추출한다.
	rs_moeum = (johapcode >> 5) & 0x001f;	// 중성을 추출한다.
	rs_jong = johapcode & 0x001f;			// 중성을 추출한다.

	if (qs_cho && rs_cho && qs_cho == rs_cho) point++;
	if (qs_moeum && rs_moeum && qs_moeum == rs_moeum) point++;
	if (qs_jong && rs_jong && qs_jong == rs_jong) point++;
	if (point == 1) {	// 초중종 중 하나만 같다.
		if (!(qs_jong || rs_jong))	// 종성이 둘 다 없는 글자였다면  partially matched!
			return 1;
		else
			return 0;
	}
	else if (point == 2)	// 초중종 중 두 개가 같다면 partially matched!
		return 1;
	else					// 초중종 중 같은 것이 없다.
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
	// 완성형 검색에 사용된 글로벌 메모리 영역들을 반환한다.
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

	// 초성 검색에 사용된 글로벌 메모리 영역들을 반환한다.
	free(cho3_inverted);
	free(cho2_inverted);
	free(former_list);
	free(latter_list);
	for (int i = 0; i < MAX_QUERY_LEN/2; i++) {
		free(cho_final[i]);
	}
	free(cho_final);
}
