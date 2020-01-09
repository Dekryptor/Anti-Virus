#include "pch.h"
#include "CSocket.h"
#include <ws2tcpip.h>

CSocket::CSocket()
{

}

void CSocket::connect()
{
	// ��ʼ���׽��ֻ���
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	// �����׽���
	m_Server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// ���÷�������ַ�Ͷ˿�
	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	// �˿�Ϊ6666
	addr.sin_port = htons(6666);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	// ���ӷ�����
	::connect(m_Server, (sockaddr*)&addr, sizeof(addr));
}

void CSocket::Send(DataType Type, const char* data)
{
	int nSize;
	if (data == NULL)
		nSize = 0;
	else 
		nSize = strlen(data);
	SENDDATAPACK SendDataPack = { Type,nSize };
	// �ȷ��Ͱ˸��ֽڵĽṹ���������
	::send(m_Server, (const char*)&SendDataPack, sizeof(SENDDATAPACK), 0);
	// �ٷ������ݹ�ȥ
	if(nSize)
		::send(m_Server, data, nSize, 0);
}

RECVDATASTRUCT* CSocket::Recv()
{
	RECVDATASTRUCT head = { 0 };
	if (::recv(m_Server, (char*)&head, sizeof(head) - 1, 0) != sizeof(head) - 1)
		return NULL;
	RECVDATASTRUCT* pBuff = (RECVDATASTRUCT*)malloc(head.nSize + sizeof(head));
	memset(pBuff, 0, head.nSize + sizeof(head));
	memcpy_s(pBuff, sizeof(head) - 1, &head, sizeof(head) - 1);  // ��headͷ�����ƽ�ȥ
	if (::recv(m_Server, pBuff->data, head.nSize, 0) != head.nSize)  // �����ݲ���Ҳ����
	{
		free(pBuff);
		pBuff = NULL;
		return NULL;
	}
	return pBuff;
}

void CSocket::Close()
{
	shutdown(m_Server, SD_BOTH);
}
