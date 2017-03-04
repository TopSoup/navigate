#include <AEEStdLib.h>

// #include "ptt-config.h"
// #include "datatype.h"
#include "navigatepch.h"
#include "configmgr.h"

#include "minIni.h"
//#include "log.h"

// configmgr.c

typedef struct {
	//void*			iLog;
	IFileMgr*		pFileMgr;
	char			sBuffer[INI_BUFFERSIZE];
	AECHAR			wsBuffer[INI_BUFFERSIZE];
} ConfigMgrInstance;



void* confmgr_createinstance(IShell* shell,void* iLog)
{
	ConfigMgrInstance* confmgr;
	if( NULL == shell )
		return NULL;

	DBGPRINTF("confmgr_createinstance");
	confmgr = (ConfigMgrInstance*) MALLOC(sizeof(ConfigMgrInstance));
	MEMSET(confmgr,0,sizeof(ConfigMgrInstance));
	//confmgr->iLog = iLog;

	if( AEE_SUCCESS != ISHELL_CreateInstance(shell, AEECLSID_FILEMGR, (void **)&confmgr->pFileMgr) ) {
		//LOG_WRITE(confmgr->iLog,"ISHELL_CreateInstance for AEECLSID_FILEMGR failed!");
		FREE(confmgr);
		return NULL;
	}
	
	// if( IFILEMGR_Test(confmgr->pFileMgr,SLPTT_INI_PATH) != AEE_SUCCESS ) {
	// 	IFILEMGR_MkDir(confmgr->pFileMgr,SLPTT_INI_PATH);
	// }

	if( IFILEMGR_Test(confmgr->pFileMgr,NAVIGATE_CUSTOM_INI) != SUCCESS ) {
		IFile* fp;
		fp = IFILEMGR_OpenFile(confmgr->pFileMgr,NAVIGATE_CUSTOM_INI,_OFM_CREATE);
		if( NULL == fp ) {
			return -1;
		}
		IFILE_Release(fp);
	}

	DBGPRINTF("confmgr_createinstance OVER");
	
	return confmgr;
}

void confmgr_release(void* inst)
{
	ConfigMgrInstance* confmgr;
	if(NULL != inst)
	{
		confmgr = (ConfigMgrInstance*)inst;
		if( confmgr->pFileMgr ) {
			IFILEMGR_Release(confmgr->pFileMgr);
		}
		FREE(inst);
	}
	inst = NULL;
}

const char* confmgr_gets(void* inst,const char* session,const char* key,const char* short_session,const char* short_key,const char* defval) {
	ConfigMgrInstance* c;

	if( NULL == inst || session == inst || NULL == key ) {
		return NULL;
	}

	c = (ConfigMgrInstance*)inst;
	if( NULL == short_session || NULL == short_key || ini_gets(short_session,short_key,(char*)c->sBuffer,sizeof(c->sBuffer),NAVIGATE_CUSTOM_INI,c->pFileMgr) <= 0 ) {
		if( ini_gets(session,key,(char*)c->sBuffer,sizeof(c->sBuffer),NAVIGATE_CUSTOM_INI,c->pFileMgr) <= 0 ) {
			if( ini_gets(session,key,(char*)c->sBuffer,sizeof(c->sBuffer),NAVIGATE_DEFAULT_INI,c->pFileMgr) <= 0 ) {
				//LOG_WRITE(c->iLog,"[%s] %s does not exists",session,key);
				return defval;
			}
		}
	}
	//LOG_WRITE(c->iLog,"[%s] %s=%s",session,key,c->sBuffer);
	return c->sBuffer;
}

const AECHAR* confmgr_getws(void* inst,const char* session,const char* key,const char* short_session,const char* short_key,const AECHAR* defval) {
	ConfigMgrInstance* c;

	if( NULL == inst || session == inst || NULL == key ) {
		return NULL;
	}

	c = (ConfigMgrInstance*)inst;
	if( NULL == short_session || NULL == short_key || ini_gets(short_session,short_key,(char*)c->sBuffer,sizeof(c->sBuffer),NAVIGATE_CUSTOM_INI,c->pFileMgr) <= 0 ) {
		if( ini_gets(session,key,(char*)c->sBuffer,sizeof(c->sBuffer),NAVIGATE_CUSTOM_INI,c->pFileMgr) <= 0 ) {
			if( ini_gets(session,key,(char*)c->sBuffer,sizeof(c->sBuffer),NAVIGATE_DEFAULT_INI,c->pFileMgr) <= 0 ) {
				//LOG_WRITE(c->iLog,"[%s] %s does not exists",session,key);
				return defval;
			}
		}
	}
	//LOG_WRITE(c->iLog,"[%s] %s=%s",session,key,c->sBuffer);
	UTF8TOWSTR((const byte*)c->sBuffer,STRLEN(c->sBuffer),c->wsBuffer,sizeof(c->wsBuffer));
	return c->wsBuffer;
}

int confmgr_getb(void* inst,const char* session,const char* key,const char* short_session,const char* short_key,int defval) {
	int val;
	ConfigMgrInstance* c;

	if( NULL == inst || session == inst || NULL == key ) {
		return defval;
	}

	c = (ConfigMgrInstance*)inst;
	if( NULL == short_session || NULL == short_key || ini_getbool(short_session,short_key,&val,NAVIGATE_CUSTOM_INI,c->pFileMgr) <= 0 ) {
		if( ini_getbool(session,key,&val,NAVIGATE_CUSTOM_INI,c->pFileMgr) <= 0 ) {	
			if( ini_getbool(session,key,&val,NAVIGATE_DEFAULT_INI,c->pFileMgr) <= 0 ) {
				//LOG_WRITE(c->iLog,"[%s] %s does not exists",session,key);
				val = defval;
			}
		}
	}
	//LOG_WRITE(c->iLog,"[%s] %s=%s",session,key,(val ? "TRUE" : "FALSE"));
	return val ? TRUE : FALSE;
}


int32 confmgr_getl(void* inst,const char* session,const char* key,const char* short_session,const char* short_key,int32 defval) {
	long val;
	ConfigMgrInstance* c;

	if( NULL == inst || session == inst || NULL == key ) {
		return defval;
	}

	c = (ConfigMgrInstance*)inst;
	if( NULL == short_session || NULL == short_key || ini_getl(short_session,short_key,&val,NAVIGATE_CUSTOM_INI,c->pFileMgr) <= 0 ) {
		if( ini_getl(session,key,&val,NAVIGATE_CUSTOM_INI,c->pFileMgr) <= 0 ) {	
			if( ini_getl(session,key,&val,NAVIGATE_DEFAULT_INI,c->pFileMgr) <= 0 ) {
				//LOG_WRITE(c->iLog,"[%s] %s does not exists",session,key);
				val = defval;
			}
		}
	}

	//LOG_WRITE(c->iLog,"[%s] %s=%d",session,key,val);
	return val;
}

int confmgr_puts(void* inst,const char* session,const char* key,const char* val) {
	ConfigMgrInstance* c;

	if( NULL == inst || session == inst || NULL == key ) {
		return -1;
	}

	c = (ConfigMgrInstance*)inst;

	return ini_puts(session,key,val,NAVIGATE_CUSTOM_INI,c->pFileMgr);
}

int confmgr_putws(void* inst,const char* session,const char* key,const AECHAR* val) {
	ConfigMgrInstance* c;

	if( NULL == inst || session == inst || NULL == key ) {
		return -1;
	}

	c = (ConfigMgrInstance*)inst;
	if( val ) {
		WSTRTOUTF8(val,WSTRLEN(val),(byte*)c->sBuffer,sizeof(c->sBuffer));
		return ini_puts(session,key,c->sBuffer,NAVIGATE_CUSTOM_INI,c->pFileMgr);
	} else {
		return ini_puts(session,key,NULL,NAVIGATE_CUSTOM_INI,c->pFileMgr);
	}
}

int confmgr_putb(void* inst,const char* session,const char* key,int val) {
	ConfigMgrInstance* c;

	if( NULL == inst || session == inst || NULL == key ) {
		return -1;
	}

	c = (ConfigMgrInstance*)inst;

	return ini_putl(session,key,(val ? 1 : 0),NAVIGATE_CUSTOM_INI,c->pFileMgr);
}

int confmgr_putl(void* inst,const char* session,const char* key,int32 val) {
	ConfigMgrInstance* c;

	if( NULL == inst || session == inst || NULL == key ) {
		return -1;
	}

	c = (ConfigMgrInstance*)inst;

	return ini_putl(session,key,val,NAVIGATE_CUSTOM_INI,c->pFileMgr);
}

static int confmgr_ini_parser(const mTCHAR *Section, const mTCHAR *Key, const mTCHAR *Value, void *pUser) {
	ConfigMgrInstance* c = (ConfigMgrInstance*)pUser;

	//LOG_WRITE(c->iLog,"parse: session=%s key=%s value=%s",Section,Key,Value);
	confmgr_puts((void*)pUser,Section,Key,Value);
	return 1;
}

// @Return
//	0: δ����
//	1:	�Ѵ���
//  -1:	���󣨲����ܵĺ���,����ʽ������
int confmgr_on_sms(void* inst,const char* phone,const char* smstext) {
	IFile* fp;
	int len;
	const char* accept_nums;
	ConfigMgrInstance* c;

	if( NULL == inst || NULL == phone || NULL == smstext)
		return -1;

	c = (ConfigMgrInstance*)inst;
	accept_nums = confmgr_gets(c,"sms","center",NULL,NULL,"10659031107260,18931880692,18903110989");

	if( NULL == STRSTR(accept_nums,phone) ) {
		//LOG_WRITE(c->iLog,"sms sender is not in center list");
		return -1;
	}
	
	len = STRLEN(smstext);
	if( len <= 0 ) {
		//LOG_WRITE(c->iLog,"empty sms text");
		return -1;
	}

	if( IFILEMGR_Test(c->pFileMgr,NAVIGATE_CUSTOM_INI) == SUCCESS ) {
		IFILEMGR_Remove(c->pFileMgr,NAVIGATE_CUSTOM_INI);
	}
	
	fp = IFILEMGR_OpenFile(c->pFileMgr,NAVIGATE_CUSTOM_INI,_OFM_CREATE);
	if( NULL == fp ) {
		//LOG_WRITE(c->iLog,"Can not create temp file: %s for error: %x",NAVIGATE_CUSTOM_INI,IFILEMGR_GetLastError(c->pFileMgr));
		return -1;
	} 
	IFILE_Write(fp,smstext,len);
	IFILE_Release(fp);
	
	ini_browse(confmgr_ini_parser,NAVIGATE_CUSTOM_INI,c->pFileMgr,c);
	IFILEMGR_Remove(c->pFileMgr,NAVIGATE_CUSTOM_INI);
	return 1;
}
