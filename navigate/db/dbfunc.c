/******************************************************************************
FILE:  TS_DBFunctions.c

SERVICES:  Implements backend of Expense Tracker that interacts with the expense
   tracker database.

GENERAL DESCRIPTION:

PUBLIC CLASSES AND STATIC FUNCTIONS:
	List the class(es) and static functions included in this file

INITIALIZATION & SEQUENCING REQUIREMENTS:

	See Exported Routines

       Copyright ?2002 QUALCOMM Incorporated.
               All Rights Reserved.
            QUALCOMM Proprietary/GTDR
******************************************************************************/
#include "AEEControls_res.h"

#include "dbfunc.h"
#include "navigate.brh"
#include "nmdef.h"

//ITEM Ê¹ÓÃÍ¼±ê
#define USE_LIST_ICON

static boolean TS_ParseExpenseRecord( IDBRecord* pRecord, AECHAR* psDesc, AECHAR* psLat, AECHAR* psLon );

//*****************************************************************************
// DATABASE FUNCTIONS
//*****************************************************************************

/*===========================================================================

FUNCTION: TS_ParseExpenseRecord

DESCRIPTION:
  This function extracts each of the values from an expense record in the 
  database and returns the values.  If any of the output argument's pointer values
  is NULL, the datum for that argument is not returned.

PARAMETERS:
	pRecord [in] - Pointer to a valid database record.
  
  pnDateTime [out] - Pointer to a dword that will be assigned the number of 
    seconds since 00:00:00.  NULL value is ok.

  pcType [out] - Pointer to a char that will be assigned the transaction type
    of the expense record entry.  NULL value is ok.

  psDesc [out] - Pointer to a wide-string character buffer that will be assigned
    the description of the tracsaction.  NULL value is ok.

  pnAmount [out] - Pointer to a dword that will be assigned the amount of the
    transaction.  NULL value is ok.

DEPENDENCIES:
  Assumes pRecord argument is valid.

RETURN VALUE:
  None

SIDE EFFECTS:
  Resets the Record index.

===========================================================================*/

static boolean TS_ParseExpenseRecord( IDBRecord* pRecord, AECHAR* psDesc, AECHAR* psLat, AECHAR* psLon )
{
  uint16 nFieldLen;
  AEEDBFieldName rFieldName;
  AEEDBFieldType rFieldType;
  AECHAR* psStringPtr;

  IDBRECORD_Reset( pRecord );  // Set the record index to -1
  
  // Get the first field

  rFieldType = IDBRECORD_NextField( pRecord, &rFieldName, &nFieldLen );

  if( psDesc )
  {
    psStringPtr = IDBRECORD_GetFieldString( pRecord );
    WSTRCPY( psDesc, psStringPtr );

    // psStringPtr is not freed by us because the pointer is owned by the
    // IDBRecord (See API Reference for IDBRECORD_GetFieldString ).
  }

  // Get next field

  rFieldType = IDBRECORD_NextField( pRecord, &rFieldName, &nFieldLen );
  
  if( psLat )
  {
    psStringPtr = IDBRECORD_GetFieldString( pRecord );
    WSTRCPY( psLat, psStringPtr );

    // psStringPtr is not freed by us because the pointer is owned by the
    // IDBRecord (See API Reference for IDBRECORD_GetFieldString ).
  }

  // Get next field

  rFieldType = IDBRECORD_NextField( pRecord, &rFieldName, &nFieldLen );

  if( psLon )
  {
    psStringPtr = IDBRECORD_GetFieldString( pRecord );
    WSTRCPY( psLon, psStringPtr );

    // psStringPtr is not freed by us because the pointer is owned by the
    // IDBRecord (See API Reference for IDBRECORD_GetFieldString ).
  }

  return TRUE;
}

/*===========================================================================

FUNCTION: TS_InitExpenseFieldStruct

DESCRIPTION:
  This function fills in the values of a AEEDBField structure array.  Once
  filled in, the array can be used later to write an entry to the database.

PARAMETERS:
	pFieldArray [out] - Pointer to a valid database record.
  
  pnDateTime [in] - Pointer to a dword that contains the number of seconds
    since 00:00:00.

  pcType [in] - Pointer to a char that contains the trasaction type.

  psDesc [in] - Pointer to a wide-string character buffer that contains
    the description of the tracsaction.

  pnAmount [in] - Pointer to a dword that contains the amount of the
    transaction.

DEPENDENCIES:
  Assumes all pointer values in the arguments to be valid.

RETURN VALUE:
  None

SIDE EFFECTS:
  None

===========================================================================*/

static void TS_InitExpenseFieldStruct( AEEDBField* pFieldArray, AECHAR* psDesc, AECHAR* psLat, AECHAR* psLon )
{
  // Fill in the description field

  pFieldArray[ DB_RECORD_FIELD_DESC ].fType = AEEDB_FT_STRING;
  pFieldArray[ DB_RECORD_FIELD_DESC ].fName = AEEDBFIELD_TEXT;
  pFieldArray[ DB_RECORD_FIELD_DESC ].wDataLen = WSTRSIZE( psDesc );
  pFieldArray[ DB_RECORD_FIELD_DESC ].pBuffer = psDesc;

  // Fill in the description field

  pFieldArray[ DB_RECORD_FIELD_LAT ].fType = AEEDB_FT_STRING;
  pFieldArray[ DB_RECORD_FIELD_LAT ].fName = AEEDBFIELD_TEXT;
  pFieldArray[ DB_RECORD_FIELD_LAT ].wDataLen = WSTRSIZE( psLat );
  pFieldArray[ DB_RECORD_FIELD_LAT ].pBuffer = psLat;

  // Fill in the description field

  pFieldArray[ DB_RECORD_FIELD_LON ].fType = AEEDB_FT_STRING;
  pFieldArray[ DB_RECORD_FIELD_LON ].fName = AEEDBFIELD_TEXT;
  pFieldArray[ DB_RECORD_FIELD_LON ].wDataLen = WSTRSIZE( psLon );
  pFieldArray[ DB_RECORD_FIELD_LON ].pBuffer = psLon;
}

/*===========================================================================

FUNCTION: TS_AddExpenseItem

DESCRIPTION:
  This function adds a single expense transaction to the database.

PARAMETERS:
	pApp [in] - Pointer to the CTopSoupApp structure. This structure contains 
    information specific to this applet. 
  
  nDateTime [in] - A dword that contains the number of seconds
    since 00:00:00.

  cType [in] - A char that contains the transaction type.

  psDesc [in] - Pointer to a wide-string character buffer that contains
    the description of the tracsaction.

  nAmount [in] - Pointer to a dword that contains the amount of the
    transaction.

DEPENDENCIES:
  Assumes database has been successfully opened or created.

RETURN VALUE:
  TRUE - If record successfully added to the database.
  FALSE - If record could not be added.

SIDE EFFECTS:
  None

===========================================================================*/

boolean TS_AddExpenseItem( CTopSoupApp* pApp, AECHAR* psDesc, AECHAR* psLat, AECHAR* psLon )
{
  IDBRecord* pRecord;
  AEEDBField pFieldArray[ NUM_DB_RECORD_FIELDS ];
  
  // Fill-in the AEEDBField field array

  TS_InitExpenseFieldStruct( pFieldArray, psDesc, psLat, psLon );

  // Create a new record based on the AEEDBField field array values

  pRecord = IDATABASE_CreateRecord( pApp->m_pDatabase, pFieldArray, NUM_DB_RECORD_FIELDS );

  // Return false if the record could not be created

  if( !pRecord )
    return FALSE;

  // Release the record

  IDBRECORD_Release( pRecord );

  return TRUE;
}

/*===========================================================================

FUNCTION: TS_UpdateExpenseItem

DESCRIPTION:
  This function updates the data in a single existing expense transaction in
  the database.

PARAMETERS:
	pApp [in] - Pointer to the CTopSoupApp structure. This structure contains 
    information specific to this applet. 
  
  nRecordID [in] - ID of the database record to be updated.

  nDateTime [in] - A dword that contains the number of seconds
    since 00:00:00.

  cType [in] - A char that contains the transaction type.

  psDesc [in] - Pointer to a wide-string character buffer that contains
    the description of the tracsaction.

  nAmount [in] - Pointer to a dword that contains the amount of the
    transaction.

DEPENDENCIES:
  Assumes database has been successfully opened or created.
  Assumes nRecordID is valid.

RETURN VALUE:
  TRUE - If record successfully updated.
  FALSE - If record could not be updated.

SIDE EFFECTS:
  None

===========================================================================*/

boolean TS_UpdateExpenseItem( CTopSoupApp* pApp, uint16 nRecordID, AECHAR* psDesc, AECHAR* psLat, AECHAR* psLon )
{
  IDBRecord* pRecord;
  AEEDBField pFieldArray[ NUM_DB_RECORD_FIELDS ];

  // Get the record using the ID

  if(( pRecord = IDATABASE_GetRecordByID( pApp->m_pDatabase, nRecordID )) == NULL)
    return FALSE;

  // Update the fields in the record's AEEDBField structure array

  TS_InitExpenseFieldStruct( pFieldArray, psDesc, psLat, psLon );

  // Update the record

  IDBRECORD_Update( pRecord, (AEEDBField*) pFieldArray, NUM_DB_RECORD_FIELDS );

  // Release the record

  IDBRECORD_Release( pRecord );

  return TRUE;
}

/*===========================================================================

FUNCTION: TS_DeleteExpenseItem

DESCRIPTION:
  This function deletes a database record based on the ID passed in the
  second argument.

PARAMETERS:
	pApp [in] - Pointer to the CTopSoupApp structure. This structure contains 
    information specific to this applet. 
  
  nRecordID [in] - ID of the database record to be deleted.

DEPENDENCIES:
  Assumes database has been successfully opened or created.
  Assumes nRecordID is valid.

RETURN VALUE:
  TRUE - If record successfully deleted.
  FALSE - If record could not be deleted.

SIDE EFFECTS:
  None

===========================================================================*/

boolean TS_DeleteExpenseItem( CTopSoupApp* pApp, uint16 nRecordID )
{
  IDBRecord* pRecord;

  // Get the record using the ID

  if(( pRecord = IDATABASE_GetRecordByID( pApp->m_pDatabase, nRecordID )) == NULL)
    return FALSE;
  
  // Remove the record

  if( IDBRECORD_Remove( pRecord ) != SUCCESS )
  {
    // The record could not be removed so release the record and return FALSE

    IDBRECORD_Release( pRecord );
    return FALSE;
  }

  // Record removed (and released) successfully

  return TRUE;
}

/*===========================================================================

FUNCTION: TS_GetExpenseItem

DESCRIPTION:
  This function gets a single record based on it's ID and returns the individual
  record values.

PARAMETERS:
	pApp [in] - Pointer to the CTopSoupApp structure. This structure contains 
    information specific to this applet. 
  
  nRecordID [in] - ID of the database record to be accessed.

  nDateTime [out] - Pointer to a dword that is assigned the number of 
    seconds since 00:00:00.  NULL value is ok.

  psDesc [out] - Pointer to a wide-string character buffer that is assigned
    the description of the tracsaction.  NULL value is ok.

  cType [out] - Pointer to a char that is assigend the transaction type.
    NULL value is ok.

  nAmount [out] - Pointer to a dword that is assigned the amount of the
    transaction.  NULL value is ok.

DEPENDENCIES:
  Assumes database has been successfully opened or created.


RETURN VALUE:
  TRUE - If record successfully accessed.
  FALSE - If record could not be accessed.

SIDE EFFECTS:
  None

===========================================================================*/

boolean TS_GetExpenseItem( CTopSoupApp* pApp, uint16 nRecordID, AECHAR* psDesc, AECHAR* psLat, AECHAR* psLon )
{
  IDBRecord* pRecord;
  
  // Get the record using the ID

  if(( pRecord = IDATABASE_GetRecordByID( pApp->m_pDatabase, nRecordID )) == NULL)
    return FALSE;

  // Extract each of the data values from the record and assogn to the output 
  // arguments

  TS_ParseExpenseRecord( pRecord, psDesc, psLat, psLon );

  // Release the record

  IDBRECORD_Release( pRecord );

  return TRUE;
}

/*===========================================================================

FUNCTION: TS_GetExpenseList

DESCRIPTION:
  This function searches the database for all entries which match the 
  transaction type, start and end date argument values and creates 
  a menu item entry in the pMenu argument containing the transaction
  type icon, formated amount string and date/time (in seconds) in the
  menu item's user data.

  The value of each menu item is added to generate the total amount 
  of all the transactions in the menu object.

  The ID of each menu item is set to the ID of the record in the database
  and is therefore guaranteed to be unique.

PARAMETERS:
	pApp [in] - Pointer to the CTopSoupApp structure. This structure contains 
    information specific to this applet. 
  
  pMenu [out] - Pointer to the IMenuCtl to which menu items are added.

  pnTotal [out] - Pointer to a four byte integer to which the total is written.

  nStartDate [in] - The earliest date, in seconds, of a listed transaction.

  nEndDate [in] - The latest date, in seconds, of a listed transaction.

  cType [in] - The type of transaction to be listed.

DEPENDENCIES:
  Assumes database has been successfully opened or created.
  Assumes the pMenu argument is a valid IMenuCtl pointer.
  Assumes the pnTotal arument is a valid pointer.

RETURN VALUE:
  TRUE - If record successfully accessed.
  FALSE - If record could not be accessed.

SIDE EFFECTS:
  None

===========================================================================*/

boolean TS_GetExpenseList( CTopSoupApp * pApp, IMenuCtl* pMenu, uint32* pnTotal)
{
   IDBRecord* pRecord;
   AECHAR psDesc[MAX_DESC_SIZE + 2];
   AECHAR psItemBuf[ MAX_RES_STRING_BUF_SIZE ];
   CtlAddItem ai;
   uint32 total = 0;
   
   // Initialize
   ai.wText = 0;
   ai.pImage = NULL;
#ifdef USE_LIST_ICON
   ai.pszResImage = ai.pszResText = NAVIGATE_RES_FILE;
#else
   ai.pszResImage = ai.pszResText = NULL;
#endif
   ai.wFont = AEE_FONT_LARGE;
   ai.dwData = 0;

  // Reset the database's record index

  IDATABASE_Reset( pApp->m_pDatabase );

  // Clear any items from the IMenuCtl

  IMENUCTL_Reset( pMenu );

  // Initialize the total to zero

  *pnTotal = 0;

  // Get each record in the database, one at a time
  while( (pRecord = IDATABASE_GetNextRecord( pApp->m_pDatabase )) != NULL )
  {
    // Parse the description from the current record
    TS_ParseExpenseRecord( pRecord, psDesc, NULL, NULL );

	// Check Valid
    if( WSTRLEN(psDesc) > 0)
    {
	  total ++ ;

	  // Create the menu entry.
	  WSPRINTF(psItemBuf, MAX_RES_STRING_BUF_SIZE, L"%02d. %s", total, psDesc);

#ifdef USE_LIST_ICON
	  ai.wImage = IDB_LOCATION;
#else
	  ai.wImage = 0;
#endif
	  ai.pText = psItemBuf;
      ai.wItemID = EXPENSE_LIST_ID + IDBRECORD_GetID(pRecord);
      IMENUCTL_AddItemEx(pMenu, &ai);
    }
    // Release the current record

    IDBRECORD_Release( pRecord );
  }
  
  *pnTotal = total;
  
  return TRUE;
}

/*===========================================================================

FUNCTION: TS_EmptyExpenseDatabase

DESCRIPTION:
  This function removes all the record from the database.  The database remains
  open.

PARAMETERS:
	pApp [in] - Pointer to the CTopSoupApp structure. This structure contains 
    information specific to this applet. 
  
DEPENDENCIES:
  Assumes database has been successfully opened or created.

RETURN VALUE:
  None

SIDE EFFECTS:
  None

===========================================================================*/

void TS_EmptyExpenseDatabase( CTopSoupApp* pApp )
{
  IDBRecord* pRecord;

  // Reset the database record index

  IDATABASE_Reset( pApp->m_pDatabase );

  // Get each record from the database and remove it

  while((pRecord = IDATABASE_GetNextRecord( pApp->m_pDatabase )) != NULL)
  {
    IDBRECORD_Remove( pRecord );
  }
}


int TS_AddExpenseItemOnce( CTopSoupApp* pApp, AECHAR* psDesc, AECHAR* psLat, AECHAR* psLon )
{
	IDBRecord* pRecord;
	AEEDBField pFieldArray[ NUM_DB_RECORD_FIELDS ];
    boolean bExist =FALSE;

	{
		IDBRecord* pRecord;
		AECHAR desc[MAX_DESC_SIZE + 2],lat[MAX_DESC_SIZE+2], lon[MAX_DESC_SIZE+2];
		uint32 total = 0;

		// Reset the database's record index
		IDATABASE_Reset( pApp->m_pDatabase );

		// Get each record in the database, one at a time
		while( (pRecord = IDATABASE_GetNextRecord( pApp->m_pDatabase )) != NULL )
		{
			MEMSET(desc,0,sizeof(desc));
			MEMSET(lat,0,sizeof(lat));
			MEMSET(lon,0,sizeof(lon));

			// Parse the description from the current record
			TS_ParseExpenseRecord( pRecord, desc,lat, lon );

			// Check exist
			if( 0 == WSTRICMP(psDesc,desc) && 0 == WSTRICMP(psLat,lat) && 0 == WSTRICMP(psLon,lon)) {
				bExist = TRUE;
				break;
			}
			
			// Release the current record
			IDBRECORD_Release( pRecord );
		}
	}

	//check if data exist
	if( TRUE == bExist )
		return 1;

	// Fill-in the AEEDBField field array

	TS_InitExpenseFieldStruct( pFieldArray, psDesc, psLat, psLon );

	// Create a new record based on the AEEDBField field array values

	pRecord = IDATABASE_CreateRecord( pApp->m_pDatabase, pFieldArray, NUM_DB_RECORD_FIELDS );

	// Return false if the record could not be created

	if( !pRecord )
		return -1;

	// Release the record

	IDBRECORD_Release( pRecord );

	return 0;
}
