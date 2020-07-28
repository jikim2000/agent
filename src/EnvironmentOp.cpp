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

// EnvironmentOp.cpp: implementation of the CEnvironmentOp class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "com_struct.h"
#include "EnvironmentOp.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEnvironmentOp*	t_EnvInfoOp	= NULL;

CEnvironmentOp::CEnvironmentOp() 
{
	m_nGlobalID				= 1;
	m_hMainDlg				= NULL;
	m_nMainContinue			= 1;
	m_nLgnSvrInfoIdx		= 0;
	m_nMgrSvrInfoIdx		= 0;
	m_nUdtChkSvrInfoIdx		= 0;
	m_nUdtDataSvrInfoIdx	= 0;
	m_nSysBootMode			= 0;
	m_nModuleUpdateMode		= 0;


	m_nSysOffMode			= 0;
	m_nSysOffFlag			= 0;
	m_nSysOffReason			= 0;
	m_nSysOffTime			= 0;


	m_nMgrSvrAuthStatus		= 0;
	m_nApplyPolicyStatus	= STATUS_USED_MODE_OFF;
}
//--------------------------------------------------------------------

CEnvironmentOp::~CEnvironmentOp()
{

}
//--------------------------------------------------------------------

INT32		CEnvironmentOp::InitEnvOp()
{
	SetSysBootChkMode(1);
	if(IsSysBootTime())
	{
		t_EnvInfo->SetReg_LastOffInfo(ASI_BOOT_TYPE_NORMAL);
		t_EnvInfo->SetReg_TimeStamp(0);
	}

	if (t_EnvInfo->GetReg_EnableServiceOpt() != 1)
		SetServiceRecoveryOption();

	return 0;
}
//--------------------------------------------------------------------

String			CEnvironmentOp::GetComputerName()
{
	CSystemInfo tSysInfo;
	return tSysInfo.GetCompName();
}
//---------------------------------------------------------------------------

String			CEnvironmentOp::GetUserUniCode()
{
	String strUserUnicode = "";

	PDB_ENV_SORG_LINK pdesl = t_ManageEnvSOrgLink->FirstItem();
	if(!pdesl || pdesl->tDPH.nUsedMode != STATUS_USED_MODE_ON)		return strUserUnicode;

	switch(pdesl->nUserCatchType)
	{
		case SS_ENV_SORG_LINK_USER_CATCH_TYPE_COM_NAME:
			strUserUnicode = GetComputerName();		break;
		case SS_ENV_SORG_LINK_USER_CATCH_TYPE_REGISTRY:
		{
			String strCompInfo = pdesl->strUserCatchValue;
			HKEY nKey = HKEY_LOCAL_MACHINE;
			String strCompPath, strCompValue, strCompData, strRegCompData;
			UINT32 nRegCompData = 0;
			{

				CTokenString Token(strCompInfo.c_str(), strCompInfo.length(), '/');
				String strKey = Token.NextToken();

				if(!_stricmp(strKey.c_str(), "HKLM"))
					nKey = HKEY_LOCAL_MACHINE;
				else if(!_stricmp(strKey.c_str(),"HKCR"))
					nKey = HKEY_CLASSES_ROOT;
				else if(!_stricmp(strKey.c_str(),"HKCU"))
					nKey = HKEY_CURRENT_USER;
				else if(!_stricmp(strKey.c_str(),"HKU"))
					nKey = HKEY_USERS;

				strCompInfo = Token.RemainValue();
			}
			{
				CTokenString Token(strCompInfo.c_str(), strCompInfo.length(), '/', 1);
				strCompData = Token.NextToken();
				strCompValue = Token.NextToken();
				strCompPath = Token.RemainValue();		
			}
			
			CRegUtil tRegUtil;
			CHAR szRegBuf[CHAR_MAX_SIZE] = {0, };
			if(tRegUtil.RegReadString(nKey, strCompPath.c_str(), strCompValue.c_str(), szRegBuf))
			{					
				break;
			}

			strUserUnicode = szRegBuf;
			break;
		}
		case SS_ENV_SORG_LINK_USER_CATCH_TYPE_MANAGER:
		{			
			PDB_LOCAL_ENV_AUTH pdlea = t_ManageLocalEnvAuth->FirstItem();
			if(pdlea)
			{
				strUserUnicode = pdlea->strUniCode;
			}
			break;
		}
		case SS_ENV_SORG_LINK_USER_CATCH_TYPE_USER_NAME:
		{
			strUserUnicode = t_ManageWinSession.GetLogonUserName();
			break;
		}
	}

	return strUserUnicode;
}
//---------------------------------------------------------------------------

INT32			CEnvironmentOp::SetServiceRecoveryOption(INT32 nResetCount, INT32 nActionMilisecond)
{
	return 0;
}
//---------------------------------------------------------------------------

INT32			CEnvironmentOp::SetSysBootChkMode(UINT32 nChkType, UINT32 nBootTime)
{
	if(!nBootTime)
	{
		nBootTime = (GetCurrentDateTimeInt() - (GetTickCount() / 1000));
	}

	if(!nChkType)	
	{
		m_nSysBootMode = 0;
		return 0;
	}

	if(t_EnvInfo->GetReg_DbgEnv_SysBoot() == 0)
	{
		WriteLogN("system boot check info : [%d][%d]:[%d]", GetCurrentDateTimeInt(), nBootTime, GetCurrentDateTimeInt() - nBootTime);
		if(abs(INT32(GetCurrentDateTimeInt() - nBootTime)) > TIMER_INTERVAL_TIME_MIN*5)
		{
			m_nSysBootMode = 0;
			return 0;
		}
	}
	else
	{
		nBootTime = GetCurrentDateTimeInt();
	}

	t_EnvInfo->SetReg_BootChkTime(nBootTime);
	m_nSysBootMode = 1;
	return 0;	
}
//---------------------------------------------------------------------------

INT32			CEnvironmentOp::SetSysBootMode(UINT32 nSysBootMode)
{
	m_nSysBootMode = nSysBootMode;
	if(m_nSysBootMode)
	{
		t_EnvInfo->SetReg_BootChkTime(GetCurrentDateTimeInt());
		t_EnvInfo->SetReg_LastOffInfo(ASI_BOOT_TYPE_NORMAL);
	}
	return m_nSysBootMode;
}
//---------------------------------------------------------------------------

INT32			CEnvironmentOp::SetSysOffMode(UINT32 nLogOffMode, UINT32 nFlag, DWORD dwReason)
{
	m_nSysOffMode	= nLogOffMode;
	m_nSysOffFlag	= nFlag;
	m_nSysOffReason	= dwReason;

	if(m_nSysOffMode)		m_nSysOffTime = GetCurrentDateTimeInt();
	return 0;
}
//---------------------------------------------------------------------------

void			CEnvironmentOp::SetMgrSvrInfo(String strSvrInfoList)
{
	m_tMgrSvrInfoMap.clear();
	m_nMgrSvrInfoIdx = 0;

	INT32 nSvrIdx = 0;
	CTokenString Token(strSvrInfoList.c_str(), strSvrInfoList.length(), ';');
	while(Token.IsNextToken())
	{
		String strSvrInfo = Token.NextToken();
		CTokenString TokenSub(strSvrInfo.c_str(), strSvrInfo.length(), ':');

		MEM_SVR_INFO tMSI;
		tMSI.strSvrInfo = TokenSub.NextToken();
		tMSI.nSvrPort = TokenSub.NextToken_UInt();

		WriteLogN("set mgr svr info list : [%d][%s][%d]", nSvrIdx, tMSI.strSvrInfo.c_str(), tMSI.nSvrPort);
		m_tMgrSvrInfoMap[nSvrIdx++] = tMSI;
	}
	
	return;
}
//---------------------------------------------------------------------------

void			CEnvironmentOp::SetUdtChkSvrInfo(String strSvrInfoList)
{
	m_tUdtChkSvrInfoMap.clear();
	m_nUdtChkSvrInfoIdx = 0;

	INT32 nSvrIdx = 0;
	CTokenString Token(strSvrInfoList.c_str(), strSvrInfoList.length(), ';');
	while(Token.IsNextToken())
	{
		String strSvrInfo = Token.NextToken();
		CTokenString TokenSub(strSvrInfo.c_str(), strSvrInfo.length(), ':');

		MEM_SVR_INFO tMSI;
		tMSI.strSvrInfo = TokenSub.NextToken();
		tMSI.nSvrPort = TokenSub.NextToken_UInt();

		WriteLogN("set udt info svr info list : [%d][%s][%d]", nSvrIdx, tMSI.strSvrInfo.c_str(), tMSI.nSvrPort);
		m_tUdtChkSvrInfoMap[nSvrIdx++] = tMSI;
	}
	return;
}
//---------------------------------------------------------------------------

void			CEnvironmentOp::SetUdtDataSvrInfo(String strSvrInfoList)
{
	m_tUdtDataSvrInfoMap.clear();
	m_nUdtDataSvrInfoIdx = 0;

	INT32 nSvrIdx = 0;
	CTokenString Token(strSvrInfoList.c_str(), strSvrInfoList.length(), ';');
	while(Token.IsNextToken())
	{
		String strSvrInfo = Token.NextToken();
		CTokenString TokenSub(strSvrInfo.c_str(), strSvrInfo.length(), ':');

		MEM_SVR_INFO tMSI;
		tMSI.strSvrInfo = TokenSub.NextToken();
		tMSI.nSvrPort = TokenSub.NextToken_UInt();

		WriteLogN("set udt data svr info list : [%d][%s][%d]", nSvrIdx, tMSI.strSvrInfo.c_str(), tMSI.nSvrPort);
		m_tUdtDataSvrInfoMap[nSvrIdx++] = tMSI;
	}
	return;
}
//---------------------------------------------------------------------------

String			CEnvironmentOp::GetUdtDataSvrInfo()
{
	String strRtn;
	
	TMapMemSvrInfoItor begin, end;
	begin = m_tUdtDataSvrInfoMap.begin();	end = m_tUdtDataSvrInfoMap.end();
	for(begin; begin != end; begin++)
	{
		String strSvrInfo;
		strSvrInfo = SPrintf("%s:%u;", begin->second.strSvrInfo.c_str(), begin->second.nSvrPort);			
		strRtn += strSvrInfo;
	}
	return strRtn;
}
//---------------------------------------------------------------------------

void			CEnvironmentOp::SetAgtUpFileInfo(TMapMemAgtUpFileInfo& tMemAgtUpFileInfoMap)
{
	m_tMemAgtUpFileInfoMap.clear();

	m_tMemAgtUpFileInfoMap = tMemAgtUpFileInfoMap;
	WriteLogN("set agt up file info list : [%d]", m_tMemAgtUpFileInfoMap.size());
	return;
}
//---------------------------------------------------------------------------

ASI_FDL_INFO	CEnvironmentOp::GetAgtUpFileInfo()
{
	ASI_FDL_INFO tAFI;

	tAFI.nID = GetGlobalID();
	tAFI.nItemType = ASIFDL_DL_FILE_TYPE_UPDATE;
	tAFI.nItemID = 1;	
	//tAFI.nDLSvrUsedFlag	= (ASIFDL_DL_SVR_TYPE_LOCAL | ASIFDL_DL_SVR_TYPE_PTOP | ASIFDL_DL_SVR_TYPE_SITE | ASIFDL_DL_SVR_TYPE_PUBLIC);
	tAFI.nDLSvrUsedFlag	= (ASIFDL_DL_SVR_TYPE_LOCAL | ASIFDL_DL_SVR_TYPE_SITE | ASIFDL_DL_SVR_TYPE_PUBLIC);

	strncpy(tAFI.szDLPath, STR_DEPLOY_FILE_HOME_AGT_UP, MAX_PATH-1);
	
	{
		TMapMemAgtUpFileInfoRItor begin, end;
		begin = m_tMemAgtUpFileInfoMap.rbegin();	end = m_tMemAgtUpFileInfoMap.rend();
		for(begin; begin != end; begin++)
		{
			if(begin->second.nFileType == SS_PO_UPDATE_SUPPORT_FILE_TYPE_NORMAL || begin->second.nFileType == 0)
			{
				strncpy(tAFI.szFileName, begin->second.strFileName.c_str(), MAX_PATH-1);
				strncpy(tAFI.szFileHash, begin->second.strFileHash.c_str(), MAX_PATH-1);
				break;
			}
		}
	}

	return tAFI;
}
//---------------------------------------------------------------------------

SOCK_INIT_DATA	CEnvironmentOp::GetCltLgnSID(INT32 nSSLMode)
{
	SOCK_INIT_DATA tSID;
	CHostToAddrUtil tHTAUtil;
	TMapMemSvrInfo tMemSvrInfoMap;
	MEM_SVR_INFO tMSI;

	String strSvrInfoList = t_EnvInfo->m_strLgnSvrInfoList;
	PDB_PO_SVR_INFO_LGN pPolicy = (PDB_PO_SVR_INFO_LGN)t_DeployPolicyUtil->GetCurPoPtr(SS_POLICY_TYPE_SVR_INFO_LGN);
	if(pPolicy && pPolicy->strSvrInfoList.length() > 7 && pPolicy->tDPH.nUsedMode == STATUS_USED_MODE_ON)
	{
		switch(t_EnvInfo->m_nLgnSvrInfoListMode)
		{
			case 0:		strSvrInfoList = pPolicy->strSvrInfoList;		break;
			case 1:		t_EnvInfo->SetReg_LgnSvrInfoListMode(0);		break;
			case 2:		break;
			default:	strSvrInfoList = pPolicy->strSvrInfoList;		break;
		}
	}

	{
		INT32 nSvrIdx = 0;
		CTokenString Token(strSvrInfoList.c_str(), strSvrInfoList.length(), ';');
		while(Token.IsNextToken())
		{
			String strLgnSvrInfo = Token.NextToken();
			CTokenString TokenInfo(strLgnSvrInfo.c_str(), strLgnSvrInfo.length(), ':');
			
			tMSI.strSvrInfo		= TokenInfo.NextToken();
			tMSI.nSvrPort		= TokenInfo.NextToken_UInt();

			tMemSvrInfoMap[nSvrIdx++] = tMSI;
		}

		if(tMemSvrInfoMap.size() == 0)
		{
			WriteLogE("invalid lgn svr info list : [%s]", strSvrInfoList);
			return tSID;
		}

		TMapMemSvrInfoItor find = tMemSvrInfoMap.find(m_nLgnSvrInfoIdx % tMemSvrInfoMap.size());
		if(find == tMemSvrInfoMap.end())
		{
			WriteLogE("invalid lgn svr info list number : [%s][%d]/[%d]", strSvrInfoList, tMemSvrInfoMap.size(), m_nLgnSvrInfoIdx);
			return tSID;
		}

		tMSI = find->second;
	}

	switch(nSSLMode)
	{
		case 0:
		{
			tSID.nPktHdrSize		= PKT_DATA_HDR_SHA256_SIZE;
			tSID.nPktEncType		= ASI_PKT_ENC_TYPE_SEED_SHA2;
			tSID.nRunTypeClt		= ASI_CLT_RUN_TYPE_TCP_EVENT;
			tSID.nOpTypeClt			= ASI_CLT_TCP_EVENT_OP_TYPE_DEFAULT;
			tSID.nSessionKey		= DEFAULT_SESSION_LCN_KEY;

			strncpy(tSID.szLogFileName, "/log/nanny_agt_lgn_clt_socket_log_", MAX_PATH);
			break;
		}
		case 1:
		{
			tSID.nPktHdrSize		= PKT_DATA_HDR_SSL_SIZE;
			tSID.nPktEncType		= ASI_PKT_ENC_TYPE_SSL;
			tSID.nRunTypeClt		= ASI_CLT_RUN_TYPE_TCP_EVENT;
			tSID.nOpTypeClt			= ASI_CLT_TCP_EVENT_OP_TYPE_DEFAULT;
			tSID.nSessionKey		= DEFAULT_SESSION_LCN_KEY;

			strncpy(tSID.szLogFileName, "/log/nanny_agt_lgn_clt_ssl_socket_log_", MAX_PATH);
			break;
		}
	}

	tHTAUtil.ConvertHostToAddr(tMSI.strSvrInfo.c_str(), tSID.szOpenAddress, MAX_PATH);
	tSID.nOpenPort			= tMSI.nSvrPort;

	tSID.nOpTypeClt			= ASI_CLT_TCP_EVENT_OP_TYPE_DEFAULT;
	tSID.nRemainSockEvt		= 1;
	tSID.nOnKeepAlive		= 1;	
	tSID.nPktPoolFlag		= (ASI_PKT_POOL_FLAG_RECV | ASI_PKT_POOL_FLAG_SEND);
	tSID.nRemainDebugLog	= t_EnvInfo->m_nRemainSockLog;

	strncpy(tSID.szLogFilePath, t_EnvInfo->m_strRootPath.c_str(), MAX_PATH);
	return tSID;
}
//---------------------------------------------------------------------------

SOCK_INIT_DATA	CEnvironmentOp::GetCltMgrSID(INT32 nSSLMode)
{
	SOCK_INIT_DATA tSID;
	CHostToAddrUtil tHTAUtil;
	MEM_SVR_INFO tMSI;

	{
		TMapMemSvrInfoItor find = m_tMgrSvrInfoMap.find(m_nMgrSvrInfoIdx % m_tMgrSvrInfoMap.size());
		if(find == m_tMgrSvrInfoMap.end())
		{
			WriteLogE("invalid mgr svr info list number : [%s][%d]/[%d]", m_tMgrSvrInfoMap.size(), m_nMgrSvrInfoIdx);
			return tSID;
		}

		tMSI = find->second;
	}

	switch(nSSLMode)
	{
		case 0:
		{
			tSID.nPktHdrSize		= PKT_DATA_HDR_SHA256_SIZE;
			tSID.nPktEncType		= ASI_PKT_ENC_TYPE_SEED_SHA2;
			tSID.nRunTypeSvr		= ASI_SVR_RUN_TYPE_TCP_IOCP;
			tSID.nOpTypeSvr			= ASI_SVR_TCP_IOCP_OP_TYPE_DEFAULT;
			tSID.nSessionKey		= DEFAULT_SESSION_AGENT_KEY;

			strncpy(tSID.szLogFileName, "/log/nanny_agt_mgr_clt_socket_log_", MAX_PATH-1);
			break;
		}
		case 1:
		{
			tSID.nPktHdrSize		= PKT_DATA_HDR_SSL_SIZE;
			tSID.nPktEncType		= ASI_PKT_ENC_TYPE_SSL;
			tSID.nRunTypeSvr		= ASI_SVR_RUN_TYPE_TCP_EVENT;

			tSID.nSessionKey		= DEFAULT_SESSION_AGENT_KEY;

			strncpy(tSID.szPemFileCertPath, t_EnvInfo->m_strPemCertFileName.c_str(), MAX_PATH-1);
			strncpy(tSID.szPemFileKeyPath, t_EnvInfo->m_strPemKeyFileName.c_str(), MAX_PATH-1);
			strncpy(tSID.szLogFileName, "/log/nanny_agt_mgr_clt_ssl_socket_log_", MAX_PATH-1);
			break;
		}
	}
	tHTAUtil.ConvertHostToAddr(tMSI.strSvrInfo.c_str(), tSID.szOpenAddress, MAX_PATH);
	tSID.nOpenPort			= tMSI.nSvrPort;

	tSID.nOpTypeClt			= ASI_CLT_TCP_EVENT_OP_TYPE_DEFAULT;
	tSID.nRemainSockEvt		= 1;
	tSID.nOnKeepAlive		= 1;	
	tSID.nPktPoolFlag		= (ASI_PKT_POOL_FLAG_RECV | ASI_PKT_POOL_FLAG_SEND);
	tSID.nRemainDebugLog	= t_EnvInfo->m_nRemainSockLog;

	strncpy(tSID.szLogFilePath, t_EnvInfo->m_strRootPath.c_str(), MAX_PATH-1);

	return tSID;
}
//---------------------------------------------------------------------------

SOCK_INIT_DATA	CEnvironmentOp::GetCltUdtSID(INT32 nSSLMode)
{
	SOCK_INIT_DATA tSID;
	CHostToAddrUtil tHTAUtil;
	MEM_SVR_INFO tMSI;

	{
		TMapMemSvrInfoItor find = m_tUdtChkSvrInfoMap.find(m_nUdtChkSvrInfoIdx % m_tUdtChkSvrInfoMap.size());
		if(find == m_tUdtChkSvrInfoMap.end())
		{
			WriteLogE("invalid udt info svr info list number : [%s][%d]/[%d]", m_tUdtChkSvrInfoMap.size(), m_nUdtChkSvrInfoIdx);
			return tSID;
		}

		tMSI = find->second;
	}

	switch(nSSLMode)
	{
		case 0:
		{
			tSID.nPktHdrSize		= PKT_DATA_HDR_SHA256_SIZE;
			tSID.nPktEncType		= ASI_PKT_ENC_TYPE_SEED_SHA2;
			tSID.nRunTypeSvr		= ASI_SVR_RUN_TYPE_TCP_IOCP;
			tSID.nOpTypeSvr			= ASI_SVR_TCP_IOCP_OP_TYPE_DEFAULT;
			tSID.nSessionKey		= DEFAULT_SESSION_AGENT_KEY;

			strncpy(tSID.szLogFileName, "/log/nanny_agt_udt_info_clt_socket_log_", MAX_PATH-1);
			break;
		}
		case 1:
		{
			tSID.nPktHdrSize		= PKT_DATA_HDR_SSL_SIZE;
			tSID.nPktEncType		= ASI_PKT_ENC_TYPE_SSL;
			tSID.nRunTypeSvr		= ASI_SVR_RUN_TYPE_TCP_EVENT;

			tSID.nSessionKey		= DEFAULT_SESSION_AGENT_KEY;

			strncpy(tSID.szPemFileCertPath, t_EnvInfo->m_strPemCertFileName.c_str(), MAX_PATH);
			strncpy(tSID.szPemFileKeyPath, t_EnvInfo->m_strPemKeyFileName.c_str(), MAX_PATH);
			strncpy(tSID.szLogFileName, "/log/nanny_agt_udt_info_clt_ssl_socket_log_", MAX_PATH);
			break;
		}
	}
	tHTAUtil.ConvertHostToAddr(tMSI.strSvrInfo.c_str(), tSID.szOpenAddress, MAX_PATH);
	tSID.nOpenPort			= tMSI.nSvrPort;

	tSID.nOpTypeClt			= ASI_CLT_TCP_EVENT_OP_TYPE_DEFAULT;
	tSID.nRemainSockEvt		= 1;
	tSID.nOnKeepAlive		= 1;	
	tSID.nPktPoolFlag		= (ASI_PKT_POOL_FLAG_RECV | ASI_PKT_POOL_FLAG_SEND);
	tSID.nRemainDebugLog	= t_EnvInfo->m_nRemainSockLog;

	strncpy(tSID.szLogFilePath, t_EnvInfo->m_strRootPath.c_str(), MAX_PATH);
	return tSID;
}
//---------------------------------------------------------------------------

void	CEnvironmentOp::NextCltSvrIdx(UINT32 nType)
{
	switch(nType)
	{
		case SS_ENV_SVR_INFO_SVR_TYPE_LGN:			m_nLgnSvrInfoIdx += 1;		break;
		case SS_ENV_SVR_INFO_SVR_TYPE_MGR:			m_nMgrSvrInfoIdx += 1;		break;
		case SS_ENV_SVR_INFO_SVR_TYPE_UDT_CHK:		m_nUdtChkSvrInfoIdx += 1;	break;
		case SS_ENV_SVR_INFO_SVR_TYPE_UDT_DATA:		m_nUdtDataSvrInfoIdx += 1;	break;
	}
	return;
}
//---------------------------------------------------------------------------

SOCK_INIT_DATA	CEnvironmentOp::GetSvrLinkSID(INT32 nSSLMode)
{
	SOCK_INIT_DATA tSID;

	switch(nSSLMode)
	{
		case 0:
		{
			tSID.nPktHdrSize		= PKT_DATA_HDR_SHA256_SIZE;
			tSID.nPktEncType		= ASI_PKT_ENC_TYPE_SEED_SHA2;
			tSID.nRunTypeSvr		= ASI_SVR_RUN_TYPE_TCP_IOCP;
			tSID.nOpTypeSvr			= ASI_SVR_TCP_IOCP_OP_TYPE_DEFAULT;
			tSID.nSessionKey		= DEFAULT_SESSION_AGENT_KEY;

			strncpy(tSID.szLogFileName, "/log/nanny_agt_link_socket_log_", MAX_PATH);
			break;
		}
		case 1:
		{
			tSID.nPktHdrSize		= PKT_DATA_HDR_SSL_SIZE;
			tSID.nPktEncType		= ASI_PKT_ENC_TYPE_SSL;
			tSID.nRunTypeSvr		= ASI_SVR_RUN_TYPE_TCP_EVENT;

			tSID.nSessionKey		= DEFAULT_SESSION_AGENT_KEY;

			strncpy(tSID.szPemFileCertPath, t_EnvInfo->m_strPemCertFileName.c_str(), MAX_PATH);
			strncpy(tSID.szPemFileKeyPath, t_EnvInfo->m_strPemKeyFileName.c_str(), MAX_PATH);
			strncpy(tSID.szLogFileName, "/log/nanny_agt_link_ssl_socket_log_", MAX_PATH);
			break;
		}
	}
	strncpy(tSID.szOpenAddress, ANY_OPEN_ADDRESS, MAX_PATH);
	tSID.nOpenPort			= t_EnvInfo->m_nIFPort;

	tSID.nPktTransMode		= ASI_SVR_PKT_TRANS_START_TYPE_AUTO_START;
	tSID.nRemainSockEvt		= 1;
	tSID.nOnKeepAlive		= 1;	
	tSID.nPktPoolFlag		= (ASI_PKT_POOL_FLAG_RECV | ASI_PKT_POOL_FLAG_SEND);
	tSID.nMaxComPortNum		= 1;
	tSID.nRemainDebugLog	= t_EnvInfo->m_nRemainSockLog;

	strncpy(tSID.szLogFilePath, t_EnvInfo->m_strRootPath.c_str(), MAX_PATH);

	return tSID;
}
//---------------------------------------------------------------------------

void	CEnvironmentOp::GetDLFileInitData(ASI_FDL_INIT& tAFI)
{
	memset(&tAFI, 0, sizeof(ASI_FDL_INIT));
	tAFI.hOwner				= m_hMainDlg;
	tAFI.nNotifyMsgID		= WM_GLOBAL_FDL_NOTIFY;
	tAFI.pnMainContinue		= &m_nMainContinue;

	tAFI.nDLSvrUsedFlag		= ASIFDL_DL_SVR_TYPE_ALL;
	snprintf(tAFI.szPtoPDLLPath, MAX_PATH-1, "%s/asi_ptp.so", t_EnvInfo->m_strDLLPath.c_str());

	snprintf(tAFI.szHomePath, MAX_PATH-1, "%s/%s", t_EnvInfo->m_strRootPath.c_str(), STR_DEPLOY_FILE_LOCAL_DOWN);
	snprintf(tAFI.szTempPath, MAX_PATH-1, "%s/%s", t_EnvInfo->m_strRootPath.c_str(), STR_DEPLOY_FILE_LOCAL_TEMP);

	if(DirectoryExists(tAFI.szHomePath) == FALSE)
		ForceDirectories(tAFI.szHomePath);
	if(DirectoryExists(tAFI.szTempPath) == FALSE)
		ForceDirectories(tAFI.szTempPath);

	strncpy(tAFI.szLogPath, t_EnvInfo->m_strLogPath.c_str(), MAX_PATH-1);
	strncpy(tAFI.szLogFile, "/nanny_agt_fdl_sys_", MAX_PATH-1);
	
	snprintf(tAFI.szDLPtoPSvrInfo, MAX_PATH-1, ":%u;", t_EnvInfo->m_nDLPTPPort);
	snprintf(tAFI.szDLPubSvrInfo, MAX_PATH-1, "%s:%u;", STR_DEPLOY_DOMAIN, CON_PORT_TYPE_DEPLOY_FILE);

	//sprintf_ext(MAX_PATH, tAFI.szDLSiteSvrInfo, "%s", GetUdtDataSvrInfo());
}
//---------------------------------------------------------------------------

void		CEnvironmentOp::GetTSInitData(ASI_TS_INIT& tATI)
{
	memset(&tATI, 0, sizeof(ASI_TS_INIT));

	strncpy(tATI.szLogFilePath, t_EnvInfo->m_strLogPath.c_str(), MAX_PATH-1);
	strncpy(tATI.szLogFileName, "/nanny_agt_ts_sys_", MAX_PATH-1);
}
//---------------------------------------------------------------------------


void			CEnvironmentOp::GetDMInitData(ASIDM_INIT& tAI)
{
	memset(&tAI, 0, sizeof(ASIDM_INIT));
	tAI.nRemainLog		= 1;
	strncpy(tAI.szLogFileName, "/nanny_agt_dm_log_", MAX_PATH-1);
	strncpy(tAI.szLogFilePath, t_EnvInfo->m_strLogPath.c_str(), MAX_PATH-1);
}
//---------------------------------------------------------------------------

void			CEnvironmentOp::GetFFInitData(ASI_FF_INIT_INFO& tAFII)
{
	memset(&tAFII, 0, sizeof(ASI_FF_INIT_INFO));
	tAFII.nRemainLog		= 1;
	tAFII.nFinderThreadMaxNum = 0;
	tAFII.nAutoSearchDirLevel = 3;
	strncpy(tAFII.szLogFile, "/nanny_agt_ff_log_", MAX_PATH-1);
	strncpy(tAFII.szLogPath, t_EnvInfo->m_strLogPath.c_str(), MAX_PATH-1);
	snprintf(tAFII.szDocFileFmtDLLPath, MAX_PATH-1, "%s/dll/asi_dff.so", t_EnvInfo->m_strRootPath.c_str());
	snprintf(tAFII.szFileInfoDLLPath, MAX_PATH-1, "%s/dll/asi_fi.so", t_EnvInfo->m_strRootPath.c_str());
}
//---------------------------------------------------------------------------

void			CEnvironmentOp::GetFFPtnInitData(ASI_FF_INIT_INFO& tAFII)
{
	memset(&tAFII, 0, sizeof(ASI_FF_INIT_INFO));
	tAFII.nRemainLog		= 0;
	tAFII.nFinderThreadMaxNum = 0;
	tAFII.nAutoSearchDirLevel = 3;
	strncpy(tAFII.szLogFile, "/nanny_agt_ff_ptn_log_", MAX_PATH-1);
	strncpy(tAFII.szLogPath, t_EnvInfo->m_strLogPath.c_str(), MAX_PATH-1);
	snprintf(tAFII.szDocFileFmtDLLPath, MAX_PATH-1, "%s/dll/asi_dff.so", t_EnvInfo->m_strRootPath.c_str());
	snprintf(tAFII.szFileInfoDLLPath, MAX_PATH-1, "%s/dll/asi_fi.so", t_EnvInfo->m_strRootPath.c_str());
}
//---------------------------------------------------------------------------

void			CEnvironmentOp::GetASInitData(ASISI_INIT& tASI)
{
	tASI.nRemainLog		= 1;
	strncpy(tASI.szLogFilePath, t_EnvInfo->m_strLogPath.c_str(), MAX_PATH-1);
	strncpy(tASI.szLogFileName, "/nanny_agt_sw_log_", MAX_PATH-1);
}
//---------------------------------------------------------------------------

void			CEnvironmentOp::SetIni_NotifyInfo_TimeStamp(String strSection, String strKey, UINT32 nTimeStamp)
{
	String strPath = t_EnvInfo->m_strRootPath + "/" + STR_NOTIFY_INI_FILE;
	CBIniFile IniFile(strPath.c_str());
	IniFile.WriteProfileInt(strSection.c_str(), strKey.c_str(), nTimeStamp);
}
//---------------------------------------------------------------------------

INT32			CEnvironmentOp::GetIni_NotifyInfo_TimeStamp(String strSection, String strKey)
{
	String strPath = t_EnvInfo->m_strRootPath + "/" + STR_NOTIFY_INI_FILE;
	CBIniFile IniFile(strPath.c_str());
	INT32 nTimeStamp = 0;
	CHAR szIniBuf[256]={0,};
	IniFile.GetProfileString(strSection.c_str(), strKey.c_str(), 0, szIniBuf, sizeof(szIniBuf));
	nTimeStamp = atoi(szIniBuf);

	return nTimeStamp;
}


