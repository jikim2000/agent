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


#ifndef ManagePoPmDmUnitH
#define ManagePoPmDmUnitH
//---------------------------------------------------------------------------

class CManagePoPmDmUnit : public CManagePoBase<DB_PO_PM_DM_UNIT>
{
public:
	INT32					LoadDBMS();

public:	
	PDB_PO_PM_DM_UNIT		GetDefaultData();


public:
	INT32					GetHash(UINT32 nID, String& strOrgValue);

public:
	INT32					AddPoPmDmUnit(DB_PO_PM_DM_UNIT&	data);
    INT32					EditPoPmDmUnit(DB_PO_PM_DM_UNIT&	data);
	INT32					DelPoPmDmUnit(UINT32 nID);
	INT32					ApplyPoPmDmUnit(DB_PO_PM_DM_UNIT&	data);

public:
	String					GetName(UINT32 nID);    

public:
	INT32					SetPkt(MemToken& SendToken);
	INT32					SetPkt(UINT32 nID, MemToken& SendToken);
	INT32					SetPkt(PDB_PO_PM_DM_UNIT pdata, MemToken& SendToken);
	INT32					GetPkt(MemToken& RecvToken, DB_PO_PM_DM_UNIT& data);

public:
	CManagePoPmDmUnit();
    ~CManagePoPmDmUnit();

};

extern CManagePoPmDmUnit*	t_ManagePoPmDmUnit;

#endif
