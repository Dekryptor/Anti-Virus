#pragma once
#include <TlHelp32.h>
#include <vector>
#include <tchar.h>
#include <Psapi.h>
#include <winsvc.h>
using std::vector;

// ���̽ṹ�壬����������ڴ��ʹ����
struct PROCESSINFO :PROCESSENTRY32 
{
	DWORD64 dwMemoryUsage;  //�ڴ�ʹ����
};
struct ModuleInfo :MODULEINFO
{
	TCHAR szModuleFile[MAX_PATH];
	TCHAR szModuleName[MAX_PATH];
};
typedef struct _SOFTINFO
{
	WCHAR szSoftName[50];	// �������
	WCHAR szSoftVer[50];	// ����汾��
	WCHAR szSoftData[20];	// �����װ����
	WCHAR szSoftSize[MAX_PATH];	// �����С
	WCHAR strSoftInsPath[MAX_PATH];	// �����װ·��
	WCHAR strSoftUniPath[MAX_PATH];	// �����װ·��
	WCHAR strSoftVenRel[50];	// �����������
	WCHAR strSoftIco[MAX_PATH];	// ���ͼ��·��
}SOFTINFO, * PSOFTINFO;

DWORD64 CmpStatus(ENUM_SERVICE_STATUS_PROCESS OldServer, vector<ENUM_SERVICE_STATUS_PROCESS> m_NewServiceVector);
BOOL CmpSerName(_In_ vector<ENUM_SERVICE_STATUS_PROCESS> ProcessVector, CString ServiceName);
BOOL CmpProcIndex(_In_ vector<PROCESSINFO> ProcessVector, DWORD64 dwPid);
BOOL GetAllProcesses(_Out_ vector<PROCESSINFO>& ProcessVector);
BOOL GetAllThread(_Out_ vector<THREADENTRY32>& ThreadVector, DWORD64 dwPid);
BOOL GetAllModule(_Out_ vector<ModuleInfo>& ModuleVector, DWORD64 dwPid);
int GetCPU();
DWORDLONG GetUsedMem();
INT IEClean(TCHAR* IEPath1, vector<CString>& m_IERubbishPath);
BOOL GetRubbish(CString dir, vector<CString>& paths, vector<CString>& extName);
BOOL GetALLService(_Out_ vector<ENUM_SERVICE_STATUS_PROCESS>& ServiceVector);
BOOL EnableDebugPrivilege(BOOL fEnable); //����Ϊ����Ȩ��
DWORD RVATOFOA(DWORD dwRVA);	// RVAת��ΪFOA
BOOL VirusScan(char* FilePath, char* Library);		// ���ļ��Ͳ����������������бȽ�
ULARGE_INTEGER EnumFold(CString FoldPath, vector<CString>& FileVector);
void ReadWhiteList();
DWORD GetPid(CString ProcessName);
BOOL Inject(DWORD dwPid);
