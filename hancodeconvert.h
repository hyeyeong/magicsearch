///////////////////////////////////////////////////////////////////
// ����ڰ� ȣ���ϴ� �Լ� 
//	nFlag == 1 : �ϼ��� -> ������
//	nFlag == 0 : ������ -> �ϼ���
void HangulCodeConvert(int nFlag, const char* strSrc, char* strDest);

// �ϼ����� ���������� ��ȯ�ϰ� ���� ��, ��� �� 
//	char buf[16];
//	HangulCodeConvert(1, "�ѱ��ڵ庯ȯ", buf);




///////////////////////////////////////////////////////////////////
// ���������� ȣ���ϴ� �Լ� 
unsigned int 	Binary(unsigned int val, unsigned int range);
void 		TG2KS(unsigned char *c1, unsigned char *c2);
void 		KS2TG(unsigned char *c1, unsigned char *c2);
///////////////////////////////////////////////////////////////////
// ���̺� 

//typedef const char* LPCTSTR;

#define MAX_KSCODE      2350   	
#define SINGLENUM       51           
#define KS              1
#define TG              0
#define SPACE           0x20

extern unsigned int Johap[75];