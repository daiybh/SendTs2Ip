// SendTs2Ip.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SourceTsFile.h"
#include "Rtp.h"

Udp* g_udp;
Rtp* g_rtp;
Source_TsFile* g_sourct;

void GetTsPacketData(void* p_user_data)
{
	BYTE*	p_data;
	DWORD	data_size;
	
	if (!g_sourct->GetTsPacketData(p_data, data_size))
		return;
	
	if (g_udp){
		g_udp->Write(p_data, (UINT16)data_size);
	}
	else{
		g_rtp->Write(p_data, (UINT16)data_size, 0x21, GetTickCount());
	}
}

BOOL ReadStdIn(int& i)
{
	int c;
	string text;

	while(TRUE)
	{
		c = getchar();
		if ( c == '\n' || c == EOF)
		{
			if (text.length() == 0)
				return FALSE;

			break;
		}
		text += (char)c;
	}	

	i = atoi( text.c_str());
	return TRUE;
}

UINT ReadStdIn(string_t& text)
{
	int i,c;
	string s_read;

	i = 0;
	text.clear();

	while(TRUE)
	{
		c = getchar();
		if ( c == '\n' || c == EOF)
			break;
		s_read += (char)c;
		i++;
	}
	text = a2t(s_read);
	return i;
}
void GetParam(vector<tstring> vIPList,string_t &ts_file_path,tstring &bind_ip,INT &bind_port,tstring &target_ip,INT &target_port,INT &mtu,BOOL &is_rtp)
{
	cout << "请输入TS流文件地址：" << endl;
	ReadStdIn(ts_file_path);

	string_t msg, c;
	INT i;
	msg = _T("\nSelect Bind IP Address (");
	for ( i = 0; i < vIPList.size(); i++) 
	{
		st_sprintf(&c, _T("%d"), i);
		msg += c;
	}
	msg += _T(", default index: 0) :");
	cout << t2a(msg) << endl;

	for ( i = 0; i < vIPList.size(); i++) 
	{
		cout << i << " " << t2a(vIPList[i]) << endl;
	}
	if (ReadStdIn(i) == FALSE || i > vIPList.size())
		i = 0;
	bind_ip = vIPList[i];

	cout << "\nInput Bind Port (default 0) :" << endl;
	if (ReadStdIn(bind_port) == FALSE)
		bind_port = 0;

	cout << "\nInput Target IP Address (dafault 239.0.0.1):" << endl;
	if (ReadStdIn(msg) == 0)
		target_ip = _T("239.0.0.1");
	else
		target_ip = (msg);

	cout << "\nInput Target Port (dafault 1234):" << endl;
	if(ReadStdIn(target_port) == FALSE)
		target_port = 1234;

	cout << "\nInput MTU (dafault 1482):" << endl;
	if(ReadStdIn(mtu) == FALSE)
		mtu = 1482;

	is_rtp = 1;
	cout << "\nInput Protocol, 0 UDP 1 RTP (dafault 1 RTP):" << endl;
	if(ReadStdIn(is_rtp) == FALSE)
		is_rtp = 1;
}
void getParamValue(TCHAR*src,TCHAR* &pstrParamName,TCHAR* &pstrValue)
{
	//ts=c:\1.ts
	TCHAR * ppos = _tcschr(src,'=');
	if(ppos==NULL)
		return;
	pstrParamName = src;
	memset(ppos,0,1);
	pstrValue = ppos+1;	
}
void ShouUsage()
{
	cout << "*******************************\n" << endl;
	//cout << "喜之狼の裤子\n70565912@qq.com\n" << endl;
	cout << "SendTs2Ip.exe v1.0\n" << endl;
	cout << "*******************************\n" << endl;
	cout << "SendTs2Ip.exe tsFilePath bindIP[0] target_ip target_port is_rtp\n"<<endl;
	cout << "SendTs2Ip.exe ts=c:\\1.ts bindIP=192.168.1.2 target_ip=202.2.2.3 target_port=5555 is_rtp=0\n"<<endl;
	cout << "*******************************\n\n" << endl;
}
int _tmain(int argc, _TCHAR* argv[])
//int main(int argc, char* argv[])
{
	tstring	ts_file_path;
	tstring		bind_ip;
	INT			bind_port;
	tstring		target_ip;
	INT			target_port;
	INT			mtu;
	BOOL		is_rtp;
	INT i;
	vector<tstring> vIPList;
	if ( Socket::GetLocalIPList(vIPList) == FALSE)
	{
		printf("No ip address!\n");
		return 0;
	}

	//HANDLE stdHandle = 	GetStdHandle(STD_OUTPUT_HANDLE);

	vector<tstring> vTsFile;

	if(argc <5)
		ShouUsage();
	ts_file_path=_T("d:\\cctv_HD.mpg");
	if(argc>1)
		ts_file_path=argv[1];
	if(argc>5)
	{
		//先判断是那种参数模式
		int nMod = 0;
		//如果是 = 模式	

		if(_tcschr(argv[1],'=')!=NULL)
			nMod = 1;

		if(nMod==1)
		{
			TCHAR tempBuffer[100];
			for (int i=1;i<argc;i++)
			{
				_tcscpy(tempBuffer,argv[i]);
				_tcslwr(tempBuffer);
				TCHAR *pstrParamName=NULL;
				TCHAR *pstrValue=NULL;
				getParamValue(tempBuffer,pstrParamName,pstrValue);
				if(pstrValue==NULL || pstrParamName==NULL)
					continue;

				if(_tcscmp(pstrParamName,_T("bindip"))==0)
				{
					bind_ip =pstrValue;
				}
				else if(_tcscmp(pstrParamName,_T("target_ip"))==0)
				{
					target_ip = pstrValue;
				}
				else if(_tcscmp(pstrParamName,_T("target_port"))==0)
				{
					target_port = _ttoi(pstrValue);
				}
				else if(_tcscmp(pstrParamName,_T("is_rtp"))==0)
				{

					is_rtp = _ttoi(pstrValue);
				}
				else if(_tcscmp(pstrParamName,_T("ts"))==0)
				{
					ts_file_path=pstrValue;
				}
				else if(_tcscmp(pstrParamName,_T("showmsg"))==0)
				{
					setShowMsg(_ttoi(pstrValue));
				}
			}
		}
		else if(nMod==0)
		{
			//否则
			int nip = _ttoi(argv[2]);
			bind_ip = vIPList[nip];
			target_ip = argv[3];
			target_port = _ttoi(argv[4]);
			is_rtp = _ttoi(argv[5]);

		}
	}
	else
	{
		bind_ip = vIPList[0];
		bind_port = 0;
		target_ip=vIPList[0];
		target_port=5555;
		mtu = 1482;
		is_rtp = 1;
		i=0;
		cout << "\nInput run mode (default 0) :" << endl;
		cout << "\n\t=0:port 5555"<<endl;
		cout << "\n\t>0:port 5555+"<<endl;
		cout << "\n\t-1:Custom"   <<endl;
		if(ReadStdIn(i)&&i==-1)
			GetParam(vIPList,ts_file_path,bind_ip,bind_port,target_ip,target_port,mtu,is_rtp);
		else
		{
			target_port+=i;
		}
	}
	cout<<"------------------------------------"<<endl;
	cout<<"bindIP:"<<t2a(bind_ip)<<":"<<bind_port<<" ";
	cout<<"targetIP:"<<t2a(target_ip)<<":"<<target_port<<endl;
	cout<<"rtp:"<<is_rtp<<" ";
	cout<<"tsfilePath:"<<t2a(ts_file_path)<<endl;
	g_udp = NULL;
	g_rtp = NULL;

	{
		TCHAR szOldTitle[MAX_PATH];
		TCHAR szNewTitle[MAX_PATH];

		if(GetConsoleTitle(szOldTitle,MAX_PATH))
		{
			_stprintf(szNewTitle,_T("SendTs2IP:%s_%d_%s"),target_ip.c_str(),target_port,ts_file_path.c_str());
			SetConsoleTitle(szNewTitle);
		}
	}
	if (is_rtp)	{
		g_rtp = new Rtp(mtu);
		g_rtp->Open(t2a(bind_ip), bind_port);
		g_rtp->Connect(t2a(target_ip), target_port);
	}
	else {
		g_udp = new Udp(mtu);
		g_udp->Open(t2a(bind_ip), bind_port);
		g_udp->Connect(t2a(target_ip), target_port);
	}
	
	g_sourct = new Source_TsFile();
	g_sourct->SetGetTsPacketDataCB(GetTsPacketData, NULL, TS_PACKET_SIZE_MIN*7);
	do
	{
		if (g_sourct->Open(ts_file_path.c_str()) == FALSE)
		{
			printf("Open ts file error :%s", t2a(ts_file_path).c_str());
			break;
		}
		if(getShowMsg())
		{
			cout << "\nIs working.\nIf u want exit, Input any key :" << endl;			
		}
		cin >> i;

	}while(0);

	g_sourct->Close();

	if (g_udp)
		delete g_udp;
	if (g_rtp)
		delete g_rtp;
	delete g_sourct;

	return 0;
}

