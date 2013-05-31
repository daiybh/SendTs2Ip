#pragma once

#include "Socket.h"
#include <WS2tcpip.h>

using namespace std;

class Udp : public Socket
{
public:

	Udp(UINT mtu = 1500);
	virtual ~Udp();

	virtual BOOL Open(string bindIp = "", int bindPort = 0);

	virtual BOOL Connect(string connectIp, int connectPort);

	virtual int  Read(BYTE* pBuffer, UINT16 bufferSize, UINT nTimeOut = 500000);

	virtual int  Write(PBYTE pBuffer, UINT16 bufferSize, UINT nTimeOut = 500000);

protected:

	BOOL SetMulticast(PCSTR textIP);

	BOOL	m_isConnect;
};
