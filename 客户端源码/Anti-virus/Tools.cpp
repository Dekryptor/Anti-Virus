#include "pch.h"
#include "Tools.h"
#include <Psapi.h>
#include <tchar.h>
#include "Md5.h"
#include "Data.h"
#include <fstream>
using std::fstream;
using std::ios;


//*****************************************************************************************
// ��������: GetAllProcesses
// ����˵��: �������̲��ҷŵ�������
// ��    ��: lracker
// ʱ    ��: 2019/10/14
// ��    ��: _Out_ vector<PROCESSENTRY32> & ProcessVector
// �� �� ֵ: BOOL
//*****************************************************************************************
BOOL GetAllProcesses(_Out_ vector<PROCESSINFO>& ProcessVector)
{
	// �����������
	ProcessVector.clear();
	// ��ȡ���̿���
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	// ��ʼ���ṹ��
	PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
	// ��ʼ������һ��
	if (!Process32First(hProcessSnap, &pe))
	{
		CloseHandle(hProcessSnap);
		return FALSE;
	}	
	do 
	{
		// ��ȡ���̵��ڴ�ռ��
		PROCESSINFO pInfo = *(PROCESSINFO*)&pe;
		// �򿪽���
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe.th32ProcessID);
		// ������̴�ʧ�ܣ�����̵��ڴ�ռ����Ϊ0
		if (!hProcess)
			pInfo.dwMemoryUsage = 0;
		else
		{
			PROCESS_MEMORY_COUNTERS pmc = { sizeof(PROCESS_MEMORY_COUNTERS) };
			GetProcessMemoryInfo(hProcess, &pmc, sizeof(PROCESS_MEMORY_COUNTERS));
			pInfo.dwMemoryUsage = pmc.WorkingSetSize;  //��ȡ�ڴ�ռ��
		}
		ProcessVector.push_back(pInfo);
	} while (Process32Next(hProcessSnap,&pe));
	return TRUE;
}

//*****************************************************************************************
// ��������: CmpStatus
// ����˵��: ���ݷ��������ҵ�����״̬
// ��    ��: lracker
// ʱ    ��: 2019/10/15
// ��    ��: CString Service
// �� �� ֵ: BOOL
//*****************************************************************************************
DWORD64 CmpStatus(ENUM_SERVICE_STATUS_PROCESS OldServer, vector<ENUM_SERVICE_STATUS_PROCESS> m_NewServiceVector)
{
	for (auto& i : m_NewServiceVector)
	{
		// ���µı������ҵ��˸÷���
		if (!_tcscmp(OldServer.lpServiceName, i.lpServiceName))
		{
			if (OldServer.ServiceStatusProcess.dwCurrentState != i.ServiceStatusProcess.dwCurrentState)
				return i.ServiceStatusProcess.dwCurrentState;
			else
				return 0;
		}
	}
	return 0;
}

//*****************************************************************************************
// ��������: CmpSerIndex
// ����˵��: �ȽϷ����б����PID
// ��    ��: lracker
// ʱ    ��: 2019/10/15
// ��    ��: _In_ vector<ENUM_SERVICE_STATUS_PROCESS> ProcessVector
// ��    ��: DWORD dwPid
// �� �� ֵ: BOOL
//*****************************************************************************************
BOOL CmpSerName(_In_ vector<ENUM_SERVICE_STATUS_PROCESS> ProcessVector, CString ServiceName)
{
	for (auto& i : ProcessVector)
	{
		// ��������ȥ��
		if (i.lpServiceName == ServiceName)
			return TRUE;
	}
	return FALSE;
}

//*****************************************************************************************
// ��������: CmpIndex
// ����˵��: �����б������PID�Ƿ����
// ��    ��: lracker
// ʱ    ��: 2019/10/14
// ��    ��: _In_ vector<PROCESSINFO> ProcessVector
// ��    ��: DWORD dwPid
// �� �� ֵ: BOOL
//*****************************************************************************************

BOOL CmpProcIndex(_In_ vector<PROCESSINFO> ProcessVector, DWORD64 dwPid)
{
	for (auto& i : ProcessVector)
	{
		if (i.th32ProcessID == dwPid)
			return TRUE;
	}
	return FALSE;
}

//*****************************************************************************************
// ��������: GetAllThread
// ����˵��: ����PID��ȡ�ý��̵������߳�
// ��    ��: lracker
// ʱ    ��: 2019/10/14
// ��    ��: _Out_ vector<THREADENTRY32> & ThreadVector
// �� �� ֵ: BOOL
//*****************************************************************************************
BOOL GetAllThread(_Out_ vector<THREADENTRY32>& ThreadVector, DWORD64 dwPid)
{
	// ����߳�����
	ThreadVector.clear();
	// �����߳̿���
	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	// ��ʼ���ṹ��
	THREADENTRY32 ThreadInfo = { sizeof(THREADENTRY32) };
	// ��ʼ������һ��
	if (!Thread32First(hThreadSnap, &ThreadInfo))
	{
		CloseHandle(hThreadSnap);
		return FALSE;
	}
	do 
	{
		// ������̵߳�ӵ���ߺͽ��̵�PIDһ���Ļ��������������
		if (ThreadInfo.th32OwnerProcessID == dwPid)
			ThreadVector.push_back(ThreadInfo);
	} while (Thread32Next(hThreadSnap, &ThreadInfo));
	CloseHandle(hThreadSnap);
	return TRUE;
}

//*****************************************************************************************
// ��������: GetAllModule
// ����˵��: ��ȡ���̵�����ģ��
// ��    ��: lracker
// ʱ    ��: 2019/10/14
// ��    ��: _Out_ vector<ModuleInfo> & ModuleVector
// ��    ��: DWORD dwPid
// �� �� ֵ: BOOL
//*****************************************************************************************
BOOL GetAllModule(_Out_ vector<ModuleInfo>& ModuleVector, DWORD64 dwPid)
{
	// ���ģ������
	ModuleVector.clear();
	// ��ȡ���̾��
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);
	// ���Ȼ�ȡ���õ��Ļ������Ĵ�С
	DWORD dwBuffSize = 0;
	EnumProcessModulesEx(hProcess, NULL, 0, &dwBuffSize, LIST_MODULES_ALL);
	// ���붯̬�ڴ�ռ�
	HMODULE* pModuleHandleArr = (HMODULE*)new char[dwBuffSize];
	// �ٴε��ú������ܹ���ȡģ����
	EnumProcessModulesEx(hProcess, pModuleHandleArr, dwBuffSize, &dwBuffSize, LIST_MODULES_ALL);
	// ����dwBuffSize���ֽ���������Ҫ����ÿ��������ֽ����ſ��Եõ�ģ�����ĸ���
	for (int i = 0; i < dwBuffSize / sizeof(HMODULE); ++i)
	{
		ModuleInfo stcModuleInfo = { };
		// ���ݽ��̾����ģ��������ȡģ����Ϣ�ĺ���
		GetModuleInformation(hProcess, pModuleHandleArr[i], &stcModuleInfo,sizeof(MODULEINFO));
		// ���ݽ��̾����ģ��������ȡģ���·��(����ģ����)
		GetModuleFileNameEx(hProcess, pModuleHandleArr[i], stcModuleInfo.szModuleFile, MAX_PATH);
		GetModuleFileNameEx(hProcess, pModuleHandleArr[i], stcModuleInfo.szModuleName, MAX_PATH);
		// ͨ���ļ�·��������ȡ�ļ���
		PathStripPath(stcModuleInfo.szModuleName);
		ModuleVector.push_back(stcModuleInfo);
	}
	CloseHandle(hProcess);
	return TRUE;
}

//*****************************************************************************************
// ��������: GetCPU
// ����˵��: ��ȡCPUʹ����
// ��    ��: lracker
// ʱ    ��: 2019/10/14
// �� �� ֵ: int
//*****************************************************************************************
int GetCPU()
{
	// ��һ�λ�ȡ������ʱ��
	FILETIME idleTime, kernelTime, userTime;
	GetSystemTimes(&idleTime, &kernelTime, &userTime);
	// �����ں˶��� 
	HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	// �ȴ�1000����
	WaitForSingleObject(hEvent, 1000);
	// �ڶ��λ�ȡ������ʱ��
	FILETIME preidleTime, prekernelTime, preuserTime;
	GetSystemTimes(&preidleTime, &prekernelTime, &preuserTime);
	// ת���������е�ʱ�䣬����ȡ��ʱ�������ת��
	double ChangeidleTime1 = (double)idleTime.dwHighDateTime * 4.294967296E9 + (double)idleTime.dwLowDateTime;
	double ChangekernelTime1 = (double)kernelTime.dwHighDateTime * 4.294967296E9 + (double)kernelTime.dwLowDateTime;
	double ChangeuserTime1 = (double)userTime.dwHighDateTime * 4.294967296E9 + (double)userTime.dwLowDateTime;
	double ChangeidleTime2 = (double)preidleTime.dwHighDateTime * 4.294967296E9 + (double)preidleTime.dwLowDateTime;
	double ChangekernelTime2 = (double)prekernelTime.dwHighDateTime * 4.294967296E9 + (double)prekernelTime.dwLowDateTime;
	double ChangeuserTime2 = (double)preuserTime.dwHighDateTime * 4.294967296E9 + (double)preuserTime.dwLowDateTime;
	return (int)(100.0 - (ChangeidleTime2 - ChangeidleTime1) / (ChangekernelTime2 - ChangekernelTime1 + ChangeuserTime2 - ChangeuserTime1) * 100.0);
}

//*****************************************************************************************
// ��������: GetUsedMem
// ����˵��: ��ȡ�Ѿ�ʹ�õ��ڴ�
// ��    ��: lracker
// ʱ    ��: 2019/10/14
// �� �� ֵ: DWORDLONG
//*****************************************************************************************
DWORDLONG GetUsedMem()
{
	// ��ȡ��ǰ���ڴ�״̬
	MEMORYSTATUSEX stcMemStatusEx = { 0 };
	stcMemStatusEx.dwLength = sizeof(stcMemStatusEx);
	GlobalMemoryStatusEx(&stcMemStatusEx);
	return stcMemStatusEx.ullTotalPhys - stcMemStatusEx.ullAvailPhys;
}

//*****************************************************************************************
// ��������: IEClean
// ����˵��: ����IE�����������
// ��    ��: lracker
// ʱ    ��: 2019/10/14
// ��    ��: TCHAR * IEPath1
// ��    ��: vector<CString> & m_IERubbishPath
// �� �� ֵ: INT
//*****************************************************************************************
INT IEClean(TCHAR* IEPath1, vector<CString>& m_IERubbishPath)
{
	CString temp = _T("\\*.*");
	CString IEPath = IEPath1 + temp;
	CFileFind tempFind;
	BOOL IsFinded = tempFind.FindFile(IEPath);
	INT nCount = 0;
	while (IsFinded)
	{
		IsFinded = tempFind.FindNextFileW();
		if (!tempFind.IsDots())
		{
			TCHAR sFoundFileName[200] = { 0 };
			_tcscpy_s(sFoundFileName, tempFind.GetFileName().GetBuffer(200));
			if (tempFind.IsDirectory())
			{
				TCHAR sTempDir[200] = { 0 };
				wsprintf(sTempDir, _T("%s\\%s"), IEPath1, sFoundFileName);
				nCount += IEClean(sTempDir, m_IERubbishPath); //ɾ���ļ����µ��ļ�
				RemoveDirectory(sTempDir); //�Ƴ����ļ�
			}
			else

			{
				TCHAR sTempFileName[200] = { 0 };
				wsprintf(sTempFileName, _T("%s\\%s"), IEPath1, sFoundFileName);
				m_IERubbishPath.push_back(sTempFileName);
				nCount++;
			}
		}
	}
	tempFind.Close();
	return nCount;
}

//*****************************************************************************************
// ��������: GetVsRubbish
// ����˵��: ���ݺ�׺��ȡ����
// ��    ��: lracker
// ʱ    ��: 2019/10/14
// ��    ��: CString dir
// ��    ��: std::vector<CString> & paths
// �� �� ֵ: bool
//*****************************************************************************************
BOOL GetRubbish(CString dir, vector<CString>& paths, vector<CString>& extName)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hListFile = FindFirstFile(dir + "\\*", &FindFileData);
	if (hListFile == INVALID_HANDLE_VALUE)
		return false;
	do
	{
		if (!lstrcmp(FindFileData.cFileName, L".") || !lstrcmp(FindFileData.cFileName, L".."))
			continue;
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)  //����Ǹ�Ŀ¼�Ļ�
			GetRubbish(dir + L"\\" + FindFileData.cFileName, paths, extName);
		else
		{
			// �ж��Ƿ���vs�����ļ��ĺ�׺
			for (auto& i : extName)
			{
				TCHAR* pExtName = FindFileData.cFileName + wcslen(FindFileData.cFileName);
				while (pExtName != FindFileData.cFileName & *pExtName != '.')
					pExtName--;
				if (!_wcsicmp(pExtName, i))//�����������׺�Ļ����Ǿ�ѹ��
					paths.push_back(dir + L"\\" + FindFileData.cFileName);
			}
		}
	} while (FindNextFile(hListFile, &FindFileData));
}

//*****************************************************************************************
// ��������: GetALLService
// ����˵��: ��ȡ���з���
// ��    ��: lracker
// ʱ    ��: 2019/10/15
// ��    ��: _Out_ vector<ENUM_SERVICE_STATUS_PROCESSA> & ServiceVector
// �� �� ֵ: BOOL
//*****************************************************************************************
BOOL GetALLService(_Out_ vector<ENUM_SERVICE_STATUS_PROCESS>& ServiceVector)
{
	// �����Ҫ��Ȩ��
	// ��Դ�̼����������ƹ�����
	SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCM)
		return FALSE;
	// ��һ�ε��ã���ȡ��Ҫ���ڴ��С
	DWORD dwServiceNum = 0;
	DWORD dwSize = 0;
	EnumServicesStatusEx(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL, NULL, 0, &dwSize, &dwServiceNum, NULL, NULL);
	if (!dwSize)
		return FALSE;
	// ������Ҫ���ڴ棬�ڶ��ε���
	LPENUM_SERVICE_STATUS_PROCESS pEnumService = (LPENUM_SERVICE_STATUS_PROCESS)new char[dwSize];
	// �ڶ���ö��
	bool bStatus = FALSE;
	bStatus = EnumServicesStatusEx(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL, (PBYTE)pEnumService, dwSize, &dwSize, &dwServiceNum, NULL, NULL);
	for (DWORD64 i = 0; i < dwServiceNum; i++)
	{
		// ��ȡ������Ϣ
		ServiceVector.push_back(pEnumService[i]);
	}
	return TRUE;
}

//*****************************************************************************************
// ��������: EnableDebugPrivilege
// ����˵��: ���Ե�ʱ�����Ȩ��
// ��    ��: lracker
// ʱ    ��: 2019/10/15
// ��    ��: BOOL fEnable
// �� �� ֵ: BOOL
//*****************************************************************************************
BOOL EnableDebugPrivilege(BOOL fEnable) //����Ϊ����Ȩ��
{
	BOOL fOk = FALSE;
	HANDLE hToken;
	// ���޸�Ȩ�޵ķ�ʽ���򿪽��̵�����
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
	{
		// ����Ȩ�޽ṹ��
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		// ���LUID
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;
		// ����Ȩ��
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL); // �޸�Ȩ��
		fOk = (GetLastError() == ERROR_SUCCESS);
		
		CloseHandle(hToken);
	}
	return fOk;
}

//*****************************************************************************************
// ��������: RVATOFOA
// ����˵��: RVAת��ΪFOA
// ��    ��: lracker
// ʱ    ��: 2019/10/16
// ��    ��: DWORD dwRVA
// �� �� ֵ: DWORD
//*****************************************************************************************
DWORD RVATOFOA(DWORD dwRVA)
{
	HANDLE hFileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, _T("PE"));
	LPVOID pFileBuff = MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
	PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pFileBuff;
	PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew + (DWORD64)pDos);
	PIMAGE_FILE_HEADER pFile = (PIMAGE_FILE_HEADER)&pNt->FileHeader;
	DWORD dwNum = pFile->NumberOfSections;
	PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNt);
	DWORD dwOffset = 0;
	for (int i = 0; i < dwNum; i++)
	{
		// RVA���ڸ��������ڵ�RVA&&<=�������ļ��еĴ�С+���ε�RVA
		if (dwRVA >= pSectionHeader[i].VirtualAddress&& dwRVA <= pSectionHeader[i].VirtualAddress + pSectionHeader[i].SizeOfRawData)
		{
			 dwOffset = dwRVA - pSectionHeader[i].VirtualAddress + pSectionHeader[i].PointerToRawData;
			 break;
		}
	}
	CloseHandle(hFileMap);
	return dwOffset;
}

//*****************************************************************************************
// ��������: VirusScan
// ����˵��: ɨ����ļ��Ƿ��ж���������ļ���md5�ͱ��صĲ��������md5���бȽ�
// ��    ��: lracker
// ʱ    ��: 2019/10/17
// ��    ��: char * FilePath
// �� �� ֵ: BOOL
//*****************************************************************************************
BOOL VirusScan(char* FilePath, char* Library)
{
	// ��ȡ�ļ� MD5
	char* FileMD5 = md5FileValue(FilePath);
	// ��ʼ��Library��������
	char Librarytmp[33] = {};
	memcpy_s(Librarytmp, 32, Library, 32);
	// �����߱ȽϱȽϣ��������ȵĻ�
	while (strcmp(Librarytmp,g_End))
	{
		// �����ȵĻ�
		if (!strcmp(FileMD5, Librarytmp))
		{
			// �ҵ��ͷ�����
			return TRUE;
		}
		// ������һ���Ƚϵ�md5
		while (*Library != 0x0a)
			Library++;
		// ���� \n
		Library++;
		// ��������
		while (*Library != 0x0a)
			Library++;
		// ����\n
		Library++;
		memcpy_s(Librarytmp, 32, Library, 32);
	}
	// �Ҳ������ؼ�
	return FALSE;
}

//*****************************************************************************************
// ��������: EnumFold
// ����˵��: �����ļ������е��ļ�������ѹ������
// ��    ��: lracker
// ʱ    ��: 2019/10/17
// ��    ��: CString FoldPath
// ��    ��: vector<CString> & FileVector
// �� �� ֵ: ULARGE_INTEGER
//*****************************************************************************************
ULARGE_INTEGER EnumFold(CString FoldPath, vector<CString>& FileVector)
{
	// ��ȡ��һ���ļ�����Ϣ
	// �ļ��ܴ�С
	ULARGE_INTEGER qwFileTotalSize = {};
	WIN32_FIND_DATA w32FindData = {};
	CString FoldPath2 = FoldPath + _T("\\*");
	HANDLE hFindFile = FindFirstFile(FoldPath2, &w32FindData);
	// ѭ��������ȡ��ǰĿ¼�е��ļ���Ϣ
	do 
	{
		// ȥ����������Ŀ¼
		if ((!_tcscmp(w32FindData.cFileName, _T("."))) || (!_tcscmp(w32FindData.cFileName, _T(".."))))
			continue;
		// ������ļ��еĻ����Ǿ͵ݹ������ȥ
		if (w32FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			CString SubFoldPath = FoldPath + _T("\\") + w32FindData.cFileName;
			qwFileTotalSize = EnumFold(SubFoldPath, FileVector);
		}
		// ������ļ��Ļ�����ô��ѹ��������
		else
		{
			CString FilePath = FoldPath + _T("\\") + w32FindData.cFileName;
			FileVector.push_back(FilePath);
		}
	} while (FindNextFile(hFindFile, &w32FindData));
	return qwFileTotalSize;
}

//*****************************************************************************************
// ��������: ReadWhiteList
// ����˵��: ��ȡ������
// ��    ��: lracker
// ʱ    ��: 2019/10/17
// �� �� ֵ: void
//*****************************************************************************************
void ReadWhiteList()
{
	char buffer[256];

	fstream outFile;
	outFile.open(g_WhiteList, ios::in);

	while (!outFile.eof())
	{
		outFile.getline(buffer, 256, '\n');//getline(char *,int,char) ��ʾ�����ַ��ﵽ256�����������оͽ���
		USES_CONVERSION;
		g_ProcessVector.push_back(A2W(buffer));
	}
	outFile.close();
}

//*****************************************************************************************
// ��������: GetPid
// ����˵��: ͨ������������ȡ����PID
// ��    ��: lracker
// ʱ    ��: 2019/10/18
// ��    ��: CString ProcessName
// �� �� ֵ: DWORD
//*****************************************************************************************
DWORD GetPid(CString ProcessName)
{
	// ��ȡ���̿���
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	// ��ʼ���ṹ��
	PROCESSENTRY32 pe = { sizeof(PROCESSENTRY32) };
	// ��ʼ������һ��
	if (!Process32First(hProcessSnap, &pe))
	{
		CloseHandle(hProcessSnap);
		return FALSE;
	}
	do
	{
		if (pe.szExeFile == ProcessName)
			return pe.th32ProcessID;
	} while (Process32Next(hProcessSnap, &pe));
	return 0;
}

//*****************************************************************************************
// ��������: Inject
// ����˵��: Զ��ע��
// ��    ��: lracker
// ʱ    ��: 2019/10/18
// ��    ��: DWORD dwPid
// �� �� ֵ: BOOL
//*****************************************************************************************
BOOL Inject(DWORD dwPid)
{
	// �򿪽��̾��
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
	// �ڽ��̿ռ�������ռ�
	LPVOID lpPathAddr = VirtualAllocEx(hProcess, 0, wcslen(DLLPATH) * 2 + 2, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	// ��Ŀ�����д��DLL
	DWORD64 dwWriteSize = 0;
	WriteProcessMemory(hProcess, lpPathAddr, DLLPATH, wcslen(DLLPATH) * 2 + 2, &dwWriteSize);
	// ��Ŀ������д����߳�
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, NULL, (PTHREAD_START_ROUTINE)LoadLibraryW, lpPathAddr, NULL, NULL);
	// �ȴ��߳̽���
	WaitForSingleObject(hThread, -1);
	// ������
	VirtualFreeEx(hProcess, lpPathAddr, 0, MEM_RELEASE);
	CloseHandle(hThread);
	CloseHandle(hProcess);
	return 0;
}
