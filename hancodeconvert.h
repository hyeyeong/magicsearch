///////////////////////////////////////////////////////////////////
// 사용자가 호출하는 함수 
//	nFlag == 1 : 완성형 -> 조합형
//	nFlag == 0 : 조합형 -> 완성형
void HangulCodeConvert(int nFlag, const char* strSrc, char* strDest);

// 완성형을 조합형으로 변환하고 싶을 때, 사용 예 
//	char buf[16];
//	HangulCodeConvert(1, "한글코드변환", buf);




///////////////////////////////////////////////////////////////////
// 내부적으로 호출하는 함수 
unsigned int 	Binary(unsigned int val, unsigned int range);
void 		TG2KS(unsigned char *c1, unsigned char *c2);
void 		KS2TG(unsigned char *c1, unsigned char *c2);
///////////////////////////////////////////////////////////////////
// 테이블 

//typedef const char* LPCTSTR;

#define MAX_KSCODE      2350   	
#define SINGLENUM       51           
#define KS              1
#define TG              0
#define SPACE           0x20

extern unsigned int Johap[75];