#pragma once

#include <winsock2.h>
#include <string>
#include <vector>
using namespace std;

// select mode 
static const int SELECT_MODE_READY = 0x001;
static const int SELECT_MODE_WRITE = 0x002;

// select return codes 
static const int SELECT_STATE_READY =  0;
static const int SELECT_STATE_ERROR =  1;
static const int SELECT_STATE_ABORTED = 2;
static const int SELECT_STATE_TIMEOUT = 3;

class Socket
{
public:
	Socket(UINT mtu = 1500);
	virtual ~Socket();

	virtual void Close();

	virtual int Write(PBYTE pBuffer, int writeSize, UINT nTimeOut = 500000);  // 0.5sec
	virtual int Read(BYTE* pBuffer, int readSize, UINT nTimeOut = 500000); // 0.5sec

	virtual SOCKADDR_IN GetBindAddr();
	virtual SOCKADDR_IN GetConnectAddr();

	virtual	UINT GetMTU();

	static BOOL GetLocalIPList(vector<tstring>& vIPList);
	//static BOOL GetAdapterSpeed(vector<int>& vList);

protected:
	void	ReportError();
	int Select(int iMode, int nTimeoutUsec);

	BOOL	m_isOpen;

	SOCKET		m_Socket;
	SOCKADDR_IN m_BindAddr;
	SOCKADDR_IN m_ConnectAddr;

	UINT	m_Mtu;
};
