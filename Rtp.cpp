#include "stdafx.h"
#include "Rtp.h"

static const UINT16 RTP_HEADER_SIZE = 12;
static const UINT16 RTP_H263_HEADER_SIZE = 2;

Rtp::Rtp(UINT mtu) :Udp(mtu)
{
	m_SequenceNumber = 0;
}

Rtp::~Rtp()
{
}

int Rtp::Read(BYTE* pBuffer, UINT16 bufferSize, INT8* pPayloadType, UINT16* pSequenceNumber, INT32* pTimeStamp, INT32* pSsrc, UINT nTimeOut)
{
	int iRead;

	UINT8 payloadType;
	UINT16 sequenceNumber;
	INT32 timeStamp;
	INT32 ssrc;

	m_RtpPacket.ExtendBuffer(bufferSize+RTP_HEADER_SIZE);
	m_RtpPacket.ClearData();

	iRead = Udp::Read(m_RtpPacket.m_pBuffer, m_RtpPacket.m_nBufferSize, nTimeOut);
	if (iRead < 12)
		return -1;

	if (!ParseRtpHeader(m_RtpPacket.m_pBuffer, &payloadType, &sequenceNumber, &timeStamp, &ssrc))
		return -1;

	m_RtpPacket.m_nDataSize = iRead;
	
	memcpy(pBuffer, m_RtpPacket.m_pBuffer + RTP_HEADER_SIZE, iRead - RTP_HEADER_SIZE);

	if (pPayloadType)
		*pPayloadType = payloadType;
	if (pSequenceNumber)
		*pSequenceNumber = sequenceNumber;
	if (pTimeStamp)
		*pTimeStamp = timeStamp;
	if (pSsrc)
		*pSsrc = ssrc;

	return iRead - RTP_HEADER_SIZE;
}

int Rtp::Write(PBYTE pBuffer, UINT16 bufferSize, INT8 payloadType, INT32 timeStamp, INT32 ssrc, BOOL marker, UINT nTimeOut)
{
	int iWrite;

	CreateRtpPacket(pBuffer, bufferSize, payloadType, timeStamp, ssrc, marker);

	iWrite = Udp::Write(m_RtpPacket.m_pBuffer, m_RtpPacket.m_nDataSize, nTimeOut);
	if (iWrite < RTP_HEADER_SIZE)
		return -1;

	return iWrite - RTP_HEADER_SIZE;
}

void Rtp::CreateRtpPacket(PBYTE pData, UINT16 dataSize, UINT8 nPayloadType, INT32 timeStamp, INT32 nSSRC, BOOL marker)
{
	m_RtpPacket.ExtendBuffer(RTP_HEADER_SIZE + dataSize);
	m_RtpPacket.ClearData();

	m_RtpPacket.m_pBuffer[0] = 0x80;
	m_RtpPacket.m_pBuffer[1] = (marker?0x80:0x00)|nPayloadType;

	m_RtpPacket.m_pBuffer[2] = ( m_SequenceNumber >> 8 )&0xff;
	m_RtpPacket.m_pBuffer[3] = m_SequenceNumber&0xff;

	m_RtpPacket.m_pBuffer[4] = (BYTE)( timeStamp >> 24 )&0xff;
	m_RtpPacket.m_pBuffer[5] = (BYTE)( timeStamp >> 16 )&0xff;
	m_RtpPacket.m_pBuffer[6] = (BYTE)( timeStamp >>  8 )&0xff;
	m_RtpPacket.m_pBuffer[7] = (BYTE)timeStamp&0xff;

	m_RtpPacket.m_pBuffer[ 8] = ( nSSRC >> 24 )&0xff;
	m_RtpPacket.m_pBuffer[ 9] = ( nSSRC >> 16 )&0xff;
	m_RtpPacket.m_pBuffer[10] = ( nSSRC >>  8 )&0xff;
	m_RtpPacket.m_pBuffer[11] = nSSRC&0xff;

	m_SequenceNumber++;

	m_RtpPacket.m_nDataSize = RTP_HEADER_SIZE;

	m_RtpPacket.AppendData(pData, dataSize);
}

BOOL Rtp::ParseRtpHeader(PBYTE pRtpHeader, UINT8* pPayloadType, UINT16* pSequenceNumber, INT32* pTimeStamp, INT32* pSsrc)
{
	if (pRtpHeader[0] != 0x80)
		return FALSE;

	*pPayloadType	= pRtpHeader[1]; 
	*pSequenceNumber = ((WORD)pRtpHeader[2]) << 8 | pRtpHeader[3];
	*pTimeStamp = (INT32)pRtpHeader[4] << 24 | (INT32)pRtpHeader[5] << 16 | (INT32)pRtpHeader[6] << 8 | pRtpHeader[7];
	*pSsrc = (INT32)pRtpHeader[8] << 24 | (INT32)pRtpHeader[9] << 16 | (INT32)pRtpHeader[10] << 8 | pRtpHeader[11];

	return TRUE;
}