/******************************************************************************
FILE:  DBFunc.h

SERVICES:  Header File

GENERAL DESCRIPTION:

PUBLIC CLASSES AND STATIC FUNCTIONS:
	List the class(es) and static functions included in this file

INITIALIZATION & SEQUENCING REQUIREMENTS:

	See Exported Routines

       Copyright ?2002 QUALCOMM Incorporated.
               All Rights Reserved.
            QUALCOMM Proprietary/GTDR
******************************************************************************/
#ifndef DBFUNC_H
#define DBFUNC_H

#include "navigate.h"

//*****************************************************************************
// ENUMERATED TYPE DEFINITIONS
//*****************************************************************************

// Enumerate the different field types that will be stored in the database for each 
// transaction

typedef enum
{
  DB_RECORD_FIELD_DESC,	//目的地[AECHAR]
  DB_RECORD_FIELD_LAT,	//经度[AECHAR]
  DB_RECORD_FIELD_LON,	//纬度[AECHAR]
  NUM_DB_RECORD_FIELDS	
} EDBRecordFieldType;


//*****************************************************************************
// FUNCTION DECLARATIONS
//*****************************************************************************

extern boolean TS_AddExpenseItem( CTopSoupApp* pApp, AECHAR* psDesc, AECHAR* psLat, AECHAR* psLon );
extern boolean TS_UpdateExpenseItem( CTopSoupApp* pApp, uint16 nRecordID, AECHAR* psDesc, AECHAR* psLat, AECHAR* psLon );
extern boolean TS_DeleteExpenseItem( CTopSoupApp* pApp, uint16 nRecordID );
extern boolean TS_GetExpenseItem( CTopSoupApp* pApp, uint16 nRecordID, AECHAR* psDesc, AECHAR* psLat, AECHAR* psLon );
extern boolean TS_GetExpenseList( CTopSoupApp * pApp, IMenuCtl* pMenu, uint32* pnTotal);
extern void    TS_EmptyExpenseDatabase( CTopSoupApp* pApp );

//FIXME
extern int TS_AddExpenseItemOnce( CTopSoupApp* pApp, AECHAR* psDesc, AECHAR* psLat, AECHAR* psLon );
#endif