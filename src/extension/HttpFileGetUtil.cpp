/*
 * Copyright (C) 2020-2025 ASHINi corp. 
 * 
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 * 
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Lesser General Public License for more details. 
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA 
 * 
 */


#include "stdafx.h"
#include "com_struct.h"
#include "HttpFileGetUtil.h"

CHttpFileGetUtil::CHttpFileGetUtil()
{
	m_nContinue = 1;
}

CHttpFileGetUtil::~CHttpFileGetUtil()
{
}

void  CHttpFileGetUtil::EnableHttpUtil(INT32 nEnable)
{
    m_nContinue = nEnable;
}

INT32	CHttpFileGetUtil::SendData(INT32 nSock, char *pMessage)
{
	INT32 nSendedSize = 0;
	INT32 nSendSize = 0;
	INT32 nOnceSendSize = 0;
	INT32 nRetVal = 0;
	time_t	nStartTime = time(NULL);
	time_t	nEndTime;
	INT32 nBufSize = 0;

	if(nSock == -1 || pMessage == NULL)
		return -1;

	nBufSize = strlen(pMessage);
	if(nBufSize < 1)
		return -2;
	
	nRetVal = 0;
	while(nSendedSize < nBufSize && nRetVal == 0)
	{
		nOnceSendSize = nBufSize - nSendedSize;
		nSendSize = send(nSock, &pMessage[nSendedSize], nOnceSendSize, 0);
		if(nSendSize == -1)
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK)
			{
				nEndTime = time(NULL);
				if(difftime(nEndTime, nStartTime) < 5)
				{					
					Sleep(10);
					continue;
				}
				else
				{
					nRetVal = -3;
					break;
				}
			}
			else
			{
				nRetVal = -4;
				break;
			}
		}
		else if(nSendSize == 0)
		{
			nRetVal = -5;
			break;
		}
		else
		{			
			nSendedSize += nSendSize;
			nStartTime = time(NULL);			
		}
	}

	return nRetVal;
}


INT32 CHttpFileGetUtil::GetFile_Address(LPSTR lpAddress, LPSTR lpFile, LPSTR lpSaveedFileName, INT32 nPort)
{
	INT32 nSock = -1;
	INT32 nRetVal = 0;
	sockaddr_in sin;
	char acSendMsg[CHAR_MAX_SIZE] = {0,};

	WriteLog("\n\n---------------- Get File Recv Operation Start !! ---------------------------------------");
	WriteLog("start date : %s, server [%s:%d], file [%s]", GetCurrentDateTime(1).c_str(), lpAddress, nPort, lpFile);

	nSock = socket (AF_INET, SOCK_STREAM, 0);
    if (nSock == -1)
	{
		WriteLog("fail to create socket : [%d]", errno);
		return -100;
	}
    sin.sin_family = AF_INET;
    sin.sin_port = htons( (unsigned short)nPort);	
    sin.sin_addr.s_addr = inet_addr(lpAddress) ;    
	
    if( connect (nSock,(const struct sockaddr *)&sin, sizeof(sockaddr_in) ) == -1 ) 
	{
		WriteLog("fail to connect server [%s:%d]: [%d]", lpAddress, nPort, errno);
		CLOSE_SOCK(nSock);
		return -101;
    }

	struct linger lg;	lg.l_onoff = 1;	lg.l_linger = 10000;
	setsockopt(nSock, SOL_SOCKET, SO_LINGER, (LPCTSTR)&lg, sizeof(lg));

	snprintf(acSendMsg, CHAR_MAX_SIZE-1, "GET %s HTTP/1.1\r\n", lpFile);
	nRetVal = SendData(nSock, acSendMsg);
	if(nRetVal != 0)
	{
		WriteLog("fail to send data -102 [%d] : [%d]", nRetVal, errno);
		CLOSE_SOCK(nSock);
		return -102;
	}

	snprintf(acSendMsg, CHAR_MAX_SIZE-1, "Accept: */*\r\n");
	nRetVal = SendData(nSock, acSendMsg);
	if(nRetVal != 0)
	{
		WriteLog("fail to send data -103 [%d] : [%d]", nRetVal, errno);
		CLOSE_SOCK(nSock);
		return -103;
	}

	snprintf(acSendMsg, CHAR_MAX_SIZE-1, "Accept-Language: ko\r\n");
	nRetVal = SendData(nSock, acSendMsg);
	if(nRetVal != 0)
	{
		WriteLog("fail to send data -104 [%d] : [%d]", nRetVal, errno);
		CLOSE_SOCK(nSock);
		return -104;
	}

	snprintf(acSendMsg, CHAR_MAX_SIZE-1, "Accept-Encoding: gzip, deflate\r\n");
	nRetVal = SendData(nSock, acSendMsg);
	if(nRetVal != 0)
	{
		WriteLog("fail to send data -105 [%d] : [%d]", nRetVal, errno);
		CLOSE_SOCK(nSock);
		return -105;
	}

	snprintf(acSendMsg, CHAR_MAX_SIZE-1, "User-Agent: Mozilla/4.0\r\n");
	nRetVal = SendData(nSock, acSendMsg);
	if(nRetVal != 0)
	{
		WriteLog("fail to send data -106 [%d] : [%d]", nRetVal, errno);
		CLOSE_SOCK(nSock);
		return -106;
	}

	snprintf(acSendMsg, CHAR_MAX_SIZE-1, "Host: %s\r\n", lpAddress);
	nRetVal = SendData(nSock, acSendMsg);
	if(nRetVal != 0)
	{
		WriteLog("fail to send data -107 [%d] : [%d]", nRetVal, errno);
		CLOSE_SOCK(nSock);
		return -107;
	}
	snprintf(acSendMsg, CHAR_MAX_SIZE-1, "Connection: Keep-Alive\r\n\r\n\r\n\r\n");
	nRetVal = SendData(nSock, acSendMsg);
	if(nRetVal != 0)
	{
		WriteLog("fail to send data -108 [%d] : [%d]", nRetVal, errno);
		CLOSE_SOCK(nSock);
		return -108;
	}

	char c1[2] = {0, };
	int l,line_length = 0;
	bool loop = true;
	String strMessage;
	INT32 nLoopCnt = 100;

	INT32 nRecvHMsgIdx = 0;
	CHAR szRecvHMsg[CHAR_MAX_SIZE] = {0, };

	while(loop && nLoopCnt) 
	{
		l = recv(nSock, c1, 1, 0);

		if(l == SOCKET_ERROR) 
		{
			if(errno == EAGAIN || errno == EWOULDBLOCK)
			{
				Sleep(10);
				nLoopCnt--;
			}
			else
				loop = false;
		}
		else if(l == 0)
		{
			WriteLog("close server socket -898 [%d]", errno);
			CLOSE_SOCK(nSock);
			return -890;		
		}

		if(c1[0]=='\n')
		{
			if(line_length == 0) 
				loop = false;			
			line_length = 0;
		}
		else if(c1[0]!='\r') 
			line_length++;		

		szRecvHMsg[nRecvHMsgIdx] = c1[0];
		nRecvHMsgIdx += 1;

		if(nRecvHMsgIdx == 1000)
		{
			strMessage += String(szRecvHMsg);
			memset(szRecvHMsg, 0, CHAR_MAX_SIZE);
			nRecvHMsgIdx = 0;
		}
	}
	WriteLog("recv hdr : [%s]", szRecvHMsg);

	strMessage += String(szRecvHMsg);

	DWORD dwWriteTotal = 0;
	DWORD dwRecvTotal = 0;
	DWORD dwRecvedTotal = 0;
    INT32 dwRecvLen;
    DWORD dwRecvOnceLen;
	INT32 nHeaderRtn;

    if(is_file(lpSaveedFileName) == 0)
        unlink(lpSaveedFileName);

	if(( nHeaderRtn = GetHeaderResult(strMessage)) == 200)
	{
		dwRecvTotal = GetRecvFileSize(strMessage);
		DWORD dwWaitTime = 0;
		char *pszData = NULL;
		pszData = (char *)malloc(RECV_FILE_MAX_SIZE);
		if(pszData == NULL)
		{
			WriteLog("fail to malloc memory [%d]", errno);
			CLOSE_SOCK(nSock);
			return -555;
		}
		
		FILE *fp = fopen(lpSaveedFileName, "w+");
		if(fp == NULL)
		{
			WriteLog("fail to open %s : [%d]", lpSaveedFileName, errno);
			safe_free(pszData);
			CLOSE_SOCK(nSock);
			return -666;
		}

		while(dwRecvTotal > 0 && dwRecvTotal > dwRecvedTotal && m_nContinue)
        {
            if(nSock == -1)
            {
				safe_free(pszData);
                fclose(fp);
                WriteLog("invalid socket handle -888");
				CLOSE_SOCK(nSock);
                return -888;
            }

            if(dwRecvTotal - dwRecvedTotal > RECV_FILE_MAX_SIZE)
                dwRecvOnceLen = RECV_FILE_MAX_SIZE;
            else
                dwRecvOnceLen = dwRecvTotal - dwRecvedTotal;

            memset(pszData, 0, RECV_FILE_MAX_SIZE);
			dwRecvLen = recv(nSock, pszData, dwRecvOnceLen, 0);
            if(dwRecvLen == SOCKET_ERROR)
            {
                if((errno == EAGAIN || errno == EWOULDBLOCK) && dwWaitTime != 3000)
				{
					Sleep(10);
					dwWaitTime++;
				}
				else
				{
					WriteLog("fail to recv %s : [%d]", lpSaveedFileName, errno);
					safe_free(pszData);
                    fclose(fp);
                    CLOSE_SOCK(nSock);
                    return -889;
                }
            }
			else if(dwRecvLen == 0)
			{
				WriteLog("fail to recv %s : server socket close", lpSaveedFileName);
				safe_free(pszData);
				fclose(fp);
				CLOSE_SOCK(nSock);
				return -890;	
			}
			else
			{
			    dwRecvedTotal += dwRecvLen;
                DWORD dwWriteLen = 0;
				dwWriteLen = (DWORD)fwrite(pszData, 1, (DWORD)dwRecvLen, fp);
				dwWriteTotal += dwWriteLen;
				dwWaitTime = 0;                
			}
        }

		safe_free(pszData);
		fclose(fp);
        CLOSE_SOCK(nSock);

        if(dwRecvTotal == 0)
        {
			WriteLog("fail to recv %s : recv size is zero", lpSaveedFileName);
            unlink(lpSaveedFileName);
        }

        if(m_nContinue== 0)
        {
			WriteLog("fail to recv %s -988 : operation stop by user", lpSaveedFileName);
            return -988;
        }
		WriteLog("success to recv %s size [%d]", lpSaveedFileName, dwWriteTotal);
	} 
	else 
	{
		WriteLog("fail to recv %s -109 : return header value [%d]", lpSaveedFileName, nHeaderRtn);
		CLOSE_SOCK(nSock);
		return -109;
	}
	
	WriteLog("end date : %s", GetCurrentDateTime(1).c_str());
	WriteLog("**************** Get File Operation Success !! ***************************************\n\n");
	
	return 0;
}

INT32 CHttpFileGetUtil::GetFile_Host(LPSTR lpHostName, LPSTR lpFile, LPSTR lpSaveedFileName, INT32 nPort)
{
    CHAR lpAddress[MAX_PATH] = {0, };
	
	struct hostent * host_addr = gethostbyname(lpHostName);
    if(host_addr) 
	{
		struct in_addr ia;
		ia.s_addr = *((int*)*host_addr->h_addr_list);
		
		snprintf(lpAddress, MAX_PATH-1, "%s", inet_ntoa(ia));
		
		if(_stricmp(lpHostName, lpAddress))
		{
			WriteLog("host name to addr : [%s][%s]", lpHostName, lpAddress);
		}
    }
	else
	{
		WriteLog("Unable to locate host : %d", -103);
		strncpy(lpAddress, lpHostName, MAX_PATH-1);
	}
	    
	return GetFile_Address(lpAddress, lpFile, lpSaveedFileName, nPort);
}

void CHttpFileGetUtil::WriteLog(char *fmt,...)
{
	FILE *fp = NULL;
	va_list args;
	char acLogFile[MAX_PATH] = {0, };

	if(t_EnvInfo == NULL)
	{
		return;
	}
	snprintf(acLogFile, MAX_PATH-1,  "%s/dm_http_download_%s.log", t_EnvInfo->m_strLogPath.c_str(), GetCurrentDateTime().c_str());
	
	if((fp =fopen(acLogFile, "a")) !=NULL)
	{		
		va_start(args,fmt);
		
		vfprintf(fp, fmt, args);
		fprintf(fp, "\n");
		fclose(fp);
		
		va_end(args);
	}
}

String CHttpFileGetUtil::GetCurrentDateTime(INT32 nDateTime)
{
	String strResult;
    time_t tTime = 0;
    struct tm *pTime = NULL;
	char acDate[16] = {0,} ;
    char acTime[16] = {0,} ;

	tTime = time(NULL);
	pTime = localtime(&tTime);
	if(pTime)
	{
		snprintf(acDate, 15, "%.4d_%.2d_%.2d", pTime->tm_year + 1900, pTime->tm_mon + 1, pTime->tm_mday);
		snprintf(acTime, 15, "%.2d:%.2d:%.2d", pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
	}
    
    if(nDateTime)
        strResult = SPrintf("%s %s", acDate, acTime);
    else
        strResult = SPrintf("%s", acDate);

    return strResult;
}

INT32 CHttpFileGetUtil::GetRecvFileSize(string strHeader)
{	
	INT32 nPos = 0;
    INT32 nRtn = -1;
	CHAR *pStr = NULL;
	CHAR *pStart = NULL;

	if(strHeader.length() == 0)
	{
		return -1;
	}

	String strHeaderValue = _strlwr((char *)strHeader.c_str());

	pStart = (char *)strHeaderValue.c_str();
	pStr = strstr(pStart, "content-length:");
	if(pStr == NULL)
		nPos = -1;
	else
		nPos = (INT32)(pStr - pStart);

    strHeader = strHeaderValue;

    if(nPos > -1)
    {
        nPos += 15;
        const char* lpFind = strHeader.c_str();
		CHAR lpNum[CHAR_MAX_SIZE] = {0, };
		
		int nIndex = nPos;
		int nSaveIndex = 0;
		while(lpFind[nIndex] != '\r')
		{
			if(lpFind[nIndex] != ' ')
			{
				lpNum[nSaveIndex] = lpFind[nIndex];
				nSaveIndex += 1;
			}
			nIndex += 1;
		}

		if(lpNum[0] != 0)
			nRtn = atoi(lpNum);
	}
	return nRtn;
}

int CHttpFileGetUtil::GetHeaderResult(String strHeader)
{
	int nPos = 0;
	int nRtn = -1;
	char *pStr = NULL;
	char *pStart = NULL;
	int nFind	= 0;
	int nIndex = 0;
	int nSaveIndex = 0;
	const char* pFind = NULL;
	char lpNum[CHAR_MAX_SIZE] = {0, };
	int nLength = strHeader.length();


	if(nLength < 1)
	{
		return -1;
	}
	String strHeaderValue = _strlwr((char *)strHeader.c_str());

	pStart = (char *)strHeaderValue.c_str();

	pStr = strstr((char *)strHeaderValue.c_str(), "http/");
	if(pStr == NULL)
	{
		return -2;
	}

	nPos = (int)(pStr - pStart);
	if(nPos > nLength - 5)
	{
		return -3;
	}

	strHeader = strHeaderValue;

	nPos += 5;
	pFind = strHeader.c_str();

	nIndex = nPos;

	while(pFind[nIndex] != '\r')
	{
		if(pFind[nIndex] != ' ')
		{
			if(nFind == 1)
			{
				lpNum[nSaveIndex] = pFind[nIndex];
				nSaveIndex++;
				nIndex++;
			}
			else
			{
				nIndex++;
			}
		}
		else
		{
			nIndex++;
			nFind++;
		}
		if(nIndex > nLength-1)
			break;
	}

	if(lpNum[0] != 0)
		nRtn = atoi(lpNum);
	return nRtn;
}
