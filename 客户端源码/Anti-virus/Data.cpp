#include "pch.h"
#include "Data.h"
int g_nMachine = 0;
CString g_PEPath;
const TCHAR* RES[20] = {
	L"���",
	L"λͼ",
	L"ͼ��",
	L"�˵�",
	L"�Ի���",
	L"�ַ����б�",
	L"����Ŀ¼",
	L"����",
	L"��ݼ�",
	L"�Ǹ�ʽ����Դ",
	L"��Ϣ�б�",
	L"���ָ������",
	L"NULL",
	L"ͼ����",
	L"NULL",
	L"�汾��Ϣ",
};

TCHAR g_VirusLibrary[] = _T("File\\VirusLibrary.txt");
char g_WhiteList[] = "File\\WhiteList.txt";
char* g_End = "--------------------------------";
vector<CString> g_ProcessVector;
extern DWORD g_OldTaskPid = 0;