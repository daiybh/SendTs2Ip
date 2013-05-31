// Source_TsFile.cpp: implementation of the Source_TsFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SourceTsFile.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Source_TsFile::Source_TsFile()
{
	m_hStopCapThreadEvent = NULL;
	m_hCapThread = NULL;
	m_hFile = NULL;

	m_pGetDataCB = NULL;
	m_pCBUserData = NULL;
	m_PacketGetSize = 0;
}

Source_TsFile::~Source_TsFile()
{
	Close();
}

BOOL Source_TsFile::SetGetTsPacketDataCB( GetTsPacketDataCB pCB, void* p_user_data, DWORD packet_get_size )
{
	if (m_hCapThread)
		return FALSE;

	m_pGetDataCB = pCB;
	m_pCBUserData = p_user_data;
	m_PacketGetSize = packet_get_size;

	return TRUE;
}

BOOL Source_TsFile::Open(LPCTSTR ts_file_path)
{
	if(m_hCapThread != NULL)
		return TRUE;

	if (m_pGetDataCB == NULL || m_PacketGetSize == 0)
		return FALSE;

	//FILE * fFile = fopen(t2a(ts_file_path).c_str(),"rb+");
	m_hFile = CreateFile(ts_file_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if(m_hFile == INVALID_HANDLE_VALUE)
	{
		int nLastError = GetLastError();
		return FALSE;
	}

	m_TsPacketData.AllocateBuffer(m_PacketGetSize);
	m_Buffer.AllocateBuffer(1024*1024*2);
	m_BufferSync.AllocateBuffer(1024*1024*2+TS_PACKET_SIZE_MAX*3);
	memset(m_BufferSync.m_pBuffer, 0, m_BufferSync.m_nBufferSize);

	m_hStopCapThreadEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hCapThread = (HANDLE)_beginthreadex(NULL, 0, ReadTSThread, this, 0, NULL);

	return TRUE;
}

BOOL Source_TsFile::Close()
{
	if(m_hCapThread == NULL)
		return TRUE;

	printf("DataSourceFile	:Close file source.\n");
	
	SetEvent(m_hStopCapThreadEvent);
	WaitForSingleObject(m_hCapThread, INFINITE);

	CloseHandle(m_hCapThread);
	m_hCapThread = NULL;

	CloseHandle(m_hStopCapThreadEvent);
	m_hCapThread = NULL;

	CloseHandle(m_hFile);
	m_hFile = NULL;

	m_Buffer.FreeBuffer();
	m_BufferSync.FreeBuffer();
	m_TsPacketData.FreeBuffer();

	return TRUE;
}

UINT Source_TsFile::ReadTSThread(LPVOID param)
{
	Source_TsFile* pThis = (Source_TsFile*)param;
	return pThis->ReadTS();
}

DWORD Source_TsFile::ReadTS()
{
	DWORD	thread_waite;

	INT64	start_pcr = 0;
	int		start_pcr_pid = 0;
	INT64	current_pcr = 0;

	INT64	pcr = 0;
	int		pcr_pid = 0;

	DWORD	start_time = 0;
	DWORD	current_time = 0;

	Buffer*	p_ts_packet = NULL;
	
	while(TRUE)
	{
		thread_waite = WaitForSingleObject(m_hStopCapThreadEvent, 0);
		if(thread_waite == WAIT_OBJECT_0)
			return 1;

		// 使用PCR同当前时间进行对比 控制读取速度
		current_time = GetTickCount();
		if (current_time - start_time <  (current_pcr - start_pcr) / PCR_BASS)
		{
			Sleep(10);
			continue;
		}

		// 读取buffer 如果读取失败,等待后继续再试
		if (m_BufferSync.m_nDataSize < TS_PACKET_SIZE_MAX*3)
		{
			if (ReadBuffer() == FALSE)
			{
				Sleep(10);
				continue;
			}
		}
		
		// 同步
		if (GotoSync() == FALSE)
		{
			Sleep(10);
			continue;
		}

		// 获取PCR 控制读取速度
		if (GetPcr(m_BufferSync.m_pData, &pcr, &pcr_pid) == TRUE)
		{
			// 设置起始pcr值和起始时间
			if (start_pcr == 0)
			{
				start_pcr = pcr;
				start_pcr_pid = pcr_pid;
				start_time = current_time;
			}
			
			if (pcr_pid == start_pcr_pid)
			{
				// 重置起始pcr值和起始时间
				if ( (pcr - current_pcr) / PCR_BASS > 100 || (pcr - current_pcr) < 0)
				{	
					start_pcr = pcr;
					start_time = current_time;
					printf("Reset pcr and time.\n");
				}
				current_pcr = pcr;
			}
		}

		// 加入分包缓冲
		p_ts_packet = m_vTsPacket.GetEmptyBuffer();
		if (p_ts_packet == NULL)
		{
			if (m_vTsPacket.GetFullBufferSize() < TS_PACKET_BUFFER_SIZE_MAX)
			{
				p_ts_packet = new Buffer();
			}
			else
			{
				p_ts_packet = m_vTsPacket.GetFullBuffer();
				printf("TS packet is too more,droped!\n");
			}
		}
		p_ts_packet->FillData(m_BufferSync.m_pData, m_nTsPacketSize);
		m_vTsPacket.AddFullBuffer(p_ts_packet);

		m_BufferSync.m_pData += m_nTsPacketSize;
		m_BufferSync.m_nDataSize -= m_nTsPacketSize;

		// 数据回调
		if (m_pGetDataCB && m_vTsPacket.GetDataSize() >= m_PacketGetSize)
			m_pGetDataCB(m_pCBUserData);
	}

	return 0;
}

BOOL Source_TsFile::GetPcr(const PBYTE p_ts_packet, INT64* p_pcr, INT* p_pcr_pid)
{
	if (( p_ts_packet[0] == 0x47 ) &&
		( p_ts_packet[3]&0x20 ) && 
		( p_ts_packet[5]&0x10 ) &&
		( p_ts_packet[4] >= 7 ) )			
	{
		*p_pcr_pid = ((INT)p_ts_packet[1] & 0x1F) << 8 | p_ts_packet[2];

		*p_pcr =	( (INT64)p_ts_packet[6] << 25 ) |
					( (INT64)p_ts_packet[7] << 17 ) |
					( (INT64)p_ts_packet[8] << 9  ) |
					( (INT64)p_ts_packet[9] << 1  ) |
					( (INT64)p_ts_packet[10] >> 7 );
		return TRUE;
	}

	return FALSE;
}

BOOL Source_TsFile::GotoSync()
{
	while(m_BufferSync.m_nDataSize >= TS_PACKET_SIZE_MAX * 3)
	{
		if (m_BufferSync.m_pData[0] == 0x47 &&
			m_BufferSync.m_pData[TS_PACKET_188] == 0x47 &&
			m_BufferSync.m_pData[TS_PACKET_188*2] == 0x47)
		{
			m_nTsPacketSize = TS_PACKET_188;
			return TRUE;
		}

		if (m_BufferSync.m_pData[0] == 0x47 &&
			m_BufferSync.m_pData[TS_PACKET_196] == 0x47 &&
			m_BufferSync.m_pData[TS_PACKET_196*2] == 0x47)
		{
			m_nTsPacketSize = TS_PACKET_196;
			return TRUE;
		}

		if (m_BufferSync.m_pData[0] == 0x47 &&
			m_BufferSync.m_pData[TS_PACKET_204] == 0x47 &&
			m_BufferSync.m_pData[TS_PACKET_204*2] == 0x47)
		{
			m_nTsPacketSize = TS_PACKET_204;
			return TRUE;
		}

		m_BufferSync.m_pData++;
		m_BufferSync.m_nDataSize--;
	}

	return FALSE;
}

BOOL Source_TsFile::ReadBuffer()
{
	DWORD nReadSize;
	if (ReadFile(m_hFile, m_Buffer.m_pBuffer, m_Buffer.m_nBufferSize, &nReadSize, 0) == FALSE)
		return FALSE;

	m_BufferSync.AppendData(m_Buffer.m_pBuffer, nReadSize);

	if (nReadSize != m_Buffer.m_nBufferSize)
		SetFilePointer(m_hFile, 0, NULL, FILE_BEGIN);

	if (m_BufferSync.m_nDataSize < TS_PACKET_SIZE_MAX*3)
		return FALSE;

	return TRUE;
}

BOOL Source_TsFile::GetTsPacketData( BYTE*& p_data, DWORD& data_size )
{
	if (!m_hCapThread)
		return FALSE;

	if (m_vTsPacket.GetDataSize() < m_PacketGetSize)
		return FALSE;

	m_TsPacketData.ClearData();

	Buffer* p_buffer;
	while(m_TsPacketData.m_nDataSize < m_PacketGetSize)
	{
		p_buffer = m_vTsPacket.GetFullBuffer();
		m_TsPacketData.AppendData(p_buffer->m_pData, p_buffer->m_nDataSize);
		m_vTsPacket.AddEmptyBuffer(p_buffer);
	}

	p_data = m_TsPacketData.m_pData;
	data_size = m_TsPacketData.m_nDataSize;

	return TRUE;
}