#ifndef MININI_H
#define MININI_H


#define mTCHAR							char
#define INI_REAL                      double

#if !defined INI_BUFFERSIZE
  #define INI_BUFFERSIZE  512
#endif

#if defined __cplusplus
  extern "C" {
#endif

int   ini_getbool(const mTCHAR *Section, const mTCHAR *Key, int* val, const mTCHAR *Filename,void* FileMgr);
int  ini_getl(const mTCHAR *Section, const mTCHAR *Key, long* val, const mTCHAR *Filename,void* FileMgr);
int   ini_gets(const mTCHAR *Section, const mTCHAR *Key, mTCHAR *Buffer, int BufferSize, const mTCHAR *Filename,void* FileMgr);
int   ini_getsection(int idx, mTCHAR *Buffer, int BufferSize, const mTCHAR *Filename,void* FileMgr);
int   ini_getkey(const mTCHAR *Section, int idx, mTCHAR *Buffer, int BufferSize, const mTCHAR *Filename,void* FileMgr);


INI_REAL ini_getf(const mTCHAR *Section, const mTCHAR *Key, INI_REAL* val, const mTCHAR *Filename,void* FileMgr);


int   ini_putl(const mTCHAR *Section, const mTCHAR *Key, long Value, const mTCHAR *Filename,void* FileMgr);
int   ini_puts(const mTCHAR *Section, const mTCHAR *Key, const mTCHAR *Value, const mTCHAR *Filename,void* FileMgr);
int   ini_putf(const mTCHAR *Section, const mTCHAR *Key, INI_REAL Value, const mTCHAR *Filename,void* FileMgr);



typedef int (*INI_CALLBACK)(const mTCHAR *Section, const mTCHAR *Key, const mTCHAR *Value, void *pUser);
int  ini_browse(INI_CALLBACK Callback,const mTCHAR *Filename,void *FileMgr,void* pUser);


#if defined __cplusplus
  }
#endif

#endif /* MININI_H */
