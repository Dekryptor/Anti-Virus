#pragma once
#include <WinSock2.h>
#include <stdint.h>

enum DataType
{
	ERR = -2,
	SYS = -1,
	UPDATE = 0,
	UPLOAD = 1
};


/*�ͻ��˷���ȥ����Ϣ�ṹ��*/
typedef struct _SENDDATAPACK
{
	DataType Type;  //���ݵ�����
	int nSize;		//���ݵĴ�С
}SENDDATAPACK;

#pragma pack(push,1)   //�ýṹ�尴��һ���ֽڶ���
/*����������������Ϣ�ṹ��*/
typedef struct _RECVDATASTRUCT
{
	uint32_t Type;  // ���ݵ�����
	uint32_t nSize;		// ���ݵĴ�С
	char data[1];   // ���ݣ�����Ϊ1��������渲�ǵ���ʱ�����ͨ��.data�ٿ�����
}RECVDATASTRUCT;
#pragma pack(pop)

class CSocket
{
	SOCKET m_Server;
public:
	CSocket();
	void connect();
	void Send(DataType Type, const char* data = NULL);
	RECVDATASTRUCT* Recv();  // �����ݽ���ٷ�װ����
	void Close();

};