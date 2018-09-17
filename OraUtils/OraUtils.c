
/* Result Sets Interface */
#ifndef SQL_CRSR
#  define SQL_CRSR
  struct sql_cursor
  {
    unsigned int curocn;
    void *ptr1;
    void *ptr2;
    unsigned int magic;
  };
  typedef struct sql_cursor sql_cursor;
  typedef struct sql_cursor SQL_CURSOR;
#endif /* SQL_CRSR */

/* Thread Safety */
typedef void * sql_context;
typedef void * SQL_CONTEXT;

/* Object support */
struct sqltvn
{
  unsigned char *tvnvsn; 
  unsigned short tvnvsnl; 
  unsigned char *tvnnm;
  unsigned short tvnnml; 
  unsigned char *tvnsnm;
  unsigned short tvnsnml;
};
typedef struct sqltvn sqltvn;

struct sqladts
{
  unsigned int adtvsn; 
  unsigned short adtmode; 
  unsigned short adtnum;  
  sqltvn adttvn[1];       
};
typedef struct sqladts sqladts;

static struct sqladts sqladt = {
  1,1,0,
};

/* Binding to PL/SQL Records */
struct sqltdss
{
  unsigned int tdsvsn; 
  unsigned short tdsnum; 
  unsigned char *tdsval[1]; 
};
typedef struct sqltdss sqltdss;
static struct sqltdss sqltds =
{
  1,
  0,
};

/* File name & Package Name */
struct sqlcxp
{
  unsigned short fillen;
           char  filnam[12];
};
static const struct sqlcxp sqlfpn =
{
    11,
    "OraUtils.pc"
};


static unsigned int sqlctx = 160403;


static struct sqlexd {
   unsigned int   sqlvsn;
   unsigned int   arrsiz;
   unsigned int   iters;
   unsigned int   offset;
   unsigned short selerr;
   unsigned short sqlety;
   unsigned int   occurs;
      const short *cud;
   unsigned char  *sqlest;
      const char  *stmt;
   sqladts *sqladtp;
   sqltdss *sqltdsp;
            void  **sqphsv;
   unsigned int   *sqphsl;
            int   *sqphss;
            void  **sqpind;
            int   *sqpins;
   unsigned int   *sqparm;
   unsigned int   **sqparc;
   unsigned short  *sqpadto;
   unsigned short  *sqptdso;
   unsigned int   sqlcmax;
   unsigned int   sqlcmin;
   unsigned int   sqlcincr;
   unsigned int   sqlctimeout;
   unsigned int   sqlcnowait;
              int   sqfoff;
   unsigned int   sqcmod;
   unsigned int   sqfmod;
   unsigned int   sqlpfmem;
            void  *sqhstv[6];
   unsigned int   sqhstl[6];
            int   sqhsts[6];
            void  *sqindv[6];
            int   sqinds[6];
   unsigned int   sqharm[6];
   unsigned int   *sqharc[6];
   unsigned short  sqadto[6];
   unsigned short  sqtdso[6];
} sqlstm = {13,6};

/* SQLLIB Prototypes */
extern void sqlcxt (void **, unsigned int *,
                    struct sqlexd *, const struct sqlcxp *);
extern void sqlcx2t(void **, unsigned int *,
                    struct sqlexd *, const struct sqlcxp *);
extern void sqlbuft(void **, char *);
extern void sqlgs2t(void **, char *);
extern void sqlorat(void **, unsigned int *, void *);

/* Forms Interface */
static const int IAPSUCC = 0;
static const int IAPFAIL = 1403;
static const int IAPFTL  = 535;
extern void sqliem(unsigned char *, signed int *);

typedef struct { unsigned short len; unsigned char arr[1]; } VARCHAR;
typedef struct { unsigned short len; unsigned char arr[1]; } varchar;

/* cud (compilation unit data) array */
static const short sqlcud0[] =
{13,4138,178,8,0,
5,0,0,0,0,0,58,64,0,0,1,1,0,1,0,3,109,0,0,
24,0,0,0,0,0,27,66,0,0,4,4,0,1,0,1,97,0,0,1,97,0,0,1,97,0,0,1,10,0,0,
55,0,0,2,0,0,32,86,0,0,0,0,0,1,0,
70,0,0,3,0,0,30,88,0,0,0,0,0,1,0,
85,0,0,4,0,0,29,97,0,0,0,0,0,1,0,
100,0,0,5,0,0,17,114,0,0,1,1,0,1,0,1,97,0,0,
119,0,0,5,0,0,45,116,0,0,0,0,0,1,0,
134,0,0,5,0,0,13,118,0,0,6,0,0,1,0,2,97,0,0,2,4,0,0,2,4,0,0,2,4,0,0,2,4,0,0,2,
4,0,0,
173,0,0,5,0,0,13,135,0,0,6,0,0,1,0,2,97,0,0,2,4,0,0,2,4,0,0,2,4,0,0,2,4,0,0,2,
4,0,0,
212,0,0,5,0,0,15,148,0,0,0,0,0,1,0,
227,0,0,0,0,0,90,207,0,64,0,0,5,105,110,77,83,69,1,0,
247,0,0,0,0,0,93,208,0,64,0,0,5,105,110,77,83,69,1,0,
267,0,0,0,0,0,93,209,0,64,1,1,5,105,110,77,83,69,1,0,3329,3,0,0,
291,0,0,0,0,0,93,210,0,64,1,1,5,105,110,77,83,69,1,0,3329,3,0,0,
315,0,0,0,0,0,93,211,0,64,1,1,5,105,110,77,83,69,1,0,3329,3,0,0,
339,0,0,0,0,0,93,212,0,64,1,1,5,105,110,77,83,69,1,0,3329,4,0,0,
363,0,0,0,0,0,93,213,0,64,1,1,5,105,110,77,83,69,1,0,3329,4,0,0,
387,0,0,6,0,0,17,214,0,0,1,1,0,1,0,1,97,0,0,
406,0,0,6,0,0,23,215,0,64,0,0,5,105,110,77,83,69,1,0,
426,0,0,0,0,0,91,224,0,64,0,0,5,105,110,77,83,69,1,0,
446,0,0,0,0,0,90,288,0,64,0,0,6,105,110,67,73,78,78,1,0,
467,0,0,0,0,0,93,289,0,64,0,0,6,105,110,67,73,78,78,1,0,
488,0,0,0,0,0,93,290,0,64,1,1,6,105,110,67,73,78,78,1,0,3329,3,0,0,
513,0,0,0,0,0,93,291,0,64,1,1,6,105,110,67,73,78,78,1,0,3329,3,0,0,
538,0,0,0,0,0,93,292,0,64,1,1,6,105,110,67,73,78,78,1,0,3329,3,0,0,
563,0,0,0,0,0,93,293,0,64,1,1,6,105,110,67,73,78,78,1,0,3329,3,0,0,
588,0,0,0,0,0,93,294,0,64,1,1,6,105,110,67,73,78,78,1,0,3329,4,0,0,
613,0,0,6,0,0,17,295,0,0,1,1,0,1,0,1,97,0,0,
632,0,0,6,0,0,23,296,0,64,0,0,6,105,110,67,73,78,78,1,0,
653,0,0,0,0,0,91,304,0,64,0,0,6,105,110,67,73,78,78,1,0,
674,0,0,7,0,0,24,333,0,0,1,1,0,1,0,1,97,0,0,
693,0,0,0,0,0,90,415,0,64,0,0,5,105,110,82,117,110,1,0,
713,0,0,0,0,0,93,416,0,64,0,0,5,105,110,82,117,110,1,0,
733,0,0,0,0,0,93,417,0,64,1,1,5,105,110,82,117,110,1,0,3329,3,0,0,
757,0,0,0,0,0,93,418,0,64,1,1,5,105,110,82,117,110,1,0,3329,3,0,0,
781,0,0,0,0,0,93,419,0,64,1,1,5,105,110,82,117,110,1,0,3329,3,0,0,
805,0,0,0,0,0,93,420,0,64,1,1,5,105,110,82,117,110,1,0,3329,3,0,0,
829,0,0,0,0,0,93,421,0,64,1,1,5,105,110,82,117,110,1,0,3329,3,0,0,
853,0,0,0,0,0,93,422,0,64,1,1,5,105,110,82,117,110,1,0,3329,3,0,0,
877,0,0,0,0,0,93,423,0,64,1,1,5,105,110,82,117,110,1,0,3329,3,0,0,
901,0,0,0,0,0,93,424,0,64,1,1,5,105,110,82,117,110,1,0,3329,4,0,0,
925,0,0,0,0,0,93,425,0,64,1,1,5,105,110,82,117,110,1,0,3329,4,0,0,
949,0,0,0,0,0,93,426,0,64,1,1,5,105,110,82,117,110,1,0,3329,4,0,0,
973,0,0,6,0,0,17,427,0,0,1,1,0,1,0,1,97,0,0,
992,0,0,6,0,0,23,428,0,64,0,0,5,105,110,82,117,110,1,0,
1012,0,0,0,0,0,91,437,0,64,0,0,5,105,110,82,117,110,1,0,
1032,0,0,8,84,0,4,478,0,0,3,2,0,1,0,2,3,0,0,1,3,0,0,1,3,0,0,
1059,0,0,9,0,0,17,488,0,0,1,1,0,1,0,1,97,0,0,
1078,0,0,9,0,0,45,490,0,0,0,0,0,1,0,
1093,0,0,9,0,0,13,492,0,0,2,0,0,1,0,2,3,0,0,2,4,0,0,
1116,0,0,9,0,0,15,503,0,0,0,0,0,1,0,
};


#include "OraUtils.h"

#include <sqlca.h>


//-- local copies
void Trim(char* str) {
	int l = 0;
	int i;
	int r = (int)strlen(str);
	char ret[MAX_PATH];
	while (isspace(str[l])>0) l++;
	while (isspace(str[r-1])>0) r--;
	for (i = 0; i<(r-l); i++) ret[i] = str[l+i];
	ret[r-l] = '\0';
	strcpy(str, ret);
}
void out(tDbg* dbg_, int msgtype, const char* callerFunc_, char* msgMask_, ...) {
	if (msgtype==DBG_MSG_INFO&&!dbg_->verbose) return;

	char indent[16]=""; for (int t=0; t<dbg_->stackLevel; t++) strcat_s(indent, 16, "\t");
	char timestamp[50]; gettimestamp(timestamp, 50);
	char tmpmsg[DBG_MSG_MAXLEN];

	va_list va_args;
	va_start(va_args, msgMask_);
	vsprintf_s(tmpmsg, DBG_MSG_MAXLEN, msgMask_, va_args);
	sprintf_s(dbg_->msg, DBG_MSG_MAXLEN, "%s%s\n", indent, tmpmsg);
	strcat_s(dbg_->stack, DBG_STACK_MAXLEN, dbg_->msg);
	va_end(va_args);

	if (dbg_->dbgtoscreen) printf("%s", dbg_->msg);
	if (dbg_->dbgtofile && dbg_->outfile!=nullptr) fprintf(dbg_->outfile, "%s", dbg_->msg);

}

void gettimestamp(char* timeS, size_t timeSsize) {
	time_t ltime; // calendar time 
	HANDLE TimeMutex;
	struct tm ltm;

	TimeMutex=CreateMutex(NULL, TRUE, NULL);
	WaitForSingleObject(TimeMutex, INFINITE);
	ltime=time(NULL); // get current cal time 

	localtime_s(&ltm, &ltime);
	sprintf_s(timeS, 18, "%d%02d%02d-%02d:%02d:%02d", 1900+ltm.tm_year, ltm.tm_mon, ltm.tm_mday, ltm.tm_hour, ltm.tm_min, ltm.tm_sec);
	ReleaseMutex(TimeMutex);
}

//=== DB common functions
EXPORT bool OraConnect(tDbg* dbg, tDBConnection* DBConnInfo) {

	/* EXEC SQL BEGIN DECLARE SECTION; */ 

	char* username = DBConnInfo->DBUser;
	char* password = DBConnInfo->DBPassword;
	char* dbstring = DBConnInfo->DBConnString;
	sql_context vCtx = NULL;
	/* EXEC SQL END DECLARE SECTION; */ 


	char* vPath = getenv("PATH");
	char* vOH = getenv("ORACLE_HOME");

	/* EXEC SQL CONTEXT ALLOCATE : vCtx; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 1;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )5;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlstm.sqhstv[0] = (         void  *)&vCtx;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(void *);
 sqlstm.sqhsts[0] = (         int  )0;
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt((void **)0, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL CONTEXT USE : vCtx; */ 

	/* EXEC SQL CONNECT : username IDENTIFIED BY : password USING : dbstring; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 4;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.iters = (unsigned int  )10;
 sqlstm.offset = (unsigned int  )24;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlstm.sqhstv[0] = (         void  *)username;
 sqlstm.sqhstl[0] = (unsigned int  )0;
 sqlstm.sqhsts[0] = (         int  )0;
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqhstv[1] = (         void  *)password;
 sqlstm.sqhstl[1] = (unsigned int  )0;
 sqlstm.sqhsts[1] = (         int  )0;
 sqlstm.sqindv[1] = (         void  *)0;
 sqlstm.sqinds[1] = (         int  )0;
 sqlstm.sqharm[1] = (unsigned int  )0;
 sqlstm.sqadto[1] = (unsigned short )0;
 sqlstm.sqtdso[1] = (unsigned short )0;
 sqlstm.sqhstv[2] = (         void  *)dbstring;
 sqlstm.sqhstl[2] = (unsigned int  )0;
 sqlstm.sqhsts[2] = (         int  )0;
 sqlstm.sqindv[2] = (         void  *)0;
 sqlstm.sqinds[2] = (         int  )0;
 sqlstm.sqharm[2] = (unsigned int  )0;
 sqlstm.sqadto[2] = (unsigned short )0;
 sqlstm.sqtdso[2] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlstm.sqlcmax = (unsigned int )100;
 sqlstm.sqlcmin = (unsigned int )2;
 sqlstm.sqlcincr = (unsigned int )1;
 sqlstm.sqlctimeout = (unsigned int )0;
 sqlstm.sqlcnowait = (unsigned int )0;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}



	if (sqlca.sqlcode==0) {
		DBConnInfo->DBCtx = vCtx;
		info("OraConnect() - Connected to ORACLE as user: %s ; DBConnInfo->DBCtx=%p\n", username, DBConnInfo->DBCtx);
	} else {
		info("PATH=%s", vPath);
		info("ORACLE_HOME=%s", vOH);
		info("DBUser=%s", DBConnInfo->DBUser);
		err("%s failed. user = %s\n Error %s", __func__, DBConnInfo->DBUser, sqlca.sqlerrm.sqlerrmc);
	}
	return(sqlca.sqlcode==0);
}
EXPORT void OraDisconnect(tDBConnection* DBConnInfo, int Commit) {
	/* EXEC SQL BEGIN DECLARE SECTION; */ 

	sql_context vCtx = DBConnInfo->DBCtx;
	/* EXEC SQL END   DECLARE SECTION; */ 


	/* EXEC SQL CONTEXT USE : vCtx; */ 

	if (Commit==0) {
		/* EXEC SQL ROLLBACK RELEASE; */ 

{
  struct sqlexd sqlstm;
  sqlstm.sqlvsn = 13;
  sqlstm.arrsiz = 4;
  sqlstm.sqladtp = &sqladt;
  sqlstm.sqltdsp = &sqltds;
  sqlstm.iters = (unsigned int  )1;
  sqlstm.offset = (unsigned int  )55;
  sqlstm.cud = sqlcud0;
  sqlstm.sqlest = (unsigned char  *)&sqlca;
  sqlstm.sqlety = (unsigned short)4352;
  sqlstm.occurs = (unsigned int  )0;
  sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	} else {
		/* EXEC SQL COMMIT WORK RELEASE; */ 

{
  struct sqlexd sqlstm;
  sqlstm.sqlvsn = 13;
  sqlstm.arrsiz = 4;
  sqlstm.sqladtp = &sqladt;
  sqlstm.sqltdsp = &sqltds;
  sqlstm.iters = (unsigned int  )1;
  sqlstm.offset = (unsigned int  )70;
  sqlstm.cud = sqlcud0;
  sqlstm.sqlest = (unsigned char  *)&sqlca;
  sqlstm.sqlety = (unsigned short)4352;
  sqlstm.occurs = (unsigned int  )0;
  sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	}
}
EXPORT void OraCommit(tDBConnection* DBConnInfo) {
	/* EXEC SQL BEGIN DECLARE SECTION; */ 

	sql_context vCtx = DBConnInfo->DBCtx;
	/* EXEC SQL END   DECLARE SECTION; */ 


	/* EXEC SQL CONTEXT USE : vCtx; */ 

	/* EXEC SQL COMMIT; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 4;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )85;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


}

//=== Retrieval functions
EXPORT bool Ora_GetFlatOHLCV(tDbg* dbg, tDBConnection* db, char* pSymbol, char* pTF, char* pDate0, int pRecCount, char** oBarTime, float* oBarData, char* oBaseTime, float* oBaseBar) {
	//-- 'Flat' bar means returning OHLC(V) values for each record as an array 
	/* EXEC SQL BEGIN DECLARE SECTION; */ 

	sql_context vCtx = db->DBCtx;
	int retval = 0;
	int i;
	char stmt[1000];
	char vNDT[12+1];
	float vOpen, vHigh, vLow, vClose, vVolume;
	/* EXEC SQL END   DECLARE SECTION; */ 

	sprintf(stmt, "select to_char(newdatetime,'YYYYMMDDHH24MI'), open, high, low, close, nvl(volume,0) from %s_%s where NewDateTime<=to_date('%s','YYYYMMDDHH24MI') order by 1 desc", pSymbol, pTF, pDate0);

	/* EXEC SQL CONTEXT USE : vCtx; */ 

	/* EXEC SQL PREPARE sOHLCV FROM : stmt; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 4;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )100;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlstm.sqhstv[0] = (         void  *)stmt;
 sqlstm.sqhstl[0] = (unsigned int  )1000;
 sqlstm.sqhsts[0] = (         int  )0;
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL DECLARE cOHLCV CURSOR FOR sOHLCV; */ 

	/* EXEC SQL OPEN cOHLCV; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 4;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )119;
 sqlstm.selerr = (unsigned short)1;
 sqlstm.sqlpfmem = (unsigned int  )0;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlstm.sqcmod = (unsigned int )0;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	for (i = pRecCount-1; i>=0; i--) {
		/* EXEC SQL FETCH cOHLCV into : vNDT, : vOpen, : vHigh, : vLow, : vClose, : vVolume; */ 

{
  struct sqlexd sqlstm;
  sqlstm.sqlvsn = 13;
  sqlstm.arrsiz = 6;
  sqlstm.sqladtp = &sqladt;
  sqlstm.sqltdsp = &sqltds;
  sqlstm.iters = (unsigned int  )1;
  sqlstm.offset = (unsigned int  )134;
  sqlstm.selerr = (unsigned short)1;
  sqlstm.sqlpfmem = (unsigned int  )0;
  sqlstm.cud = sqlcud0;
  sqlstm.sqlest = (unsigned char  *)&sqlca;
  sqlstm.sqlety = (unsigned short)4352;
  sqlstm.occurs = (unsigned int  )0;
  sqlstm.sqfoff = (           int )0;
  sqlstm.sqfmod = (unsigned int )2;
  sqlstm.sqhstv[0] = (         void  *)vNDT;
  sqlstm.sqhstl[0] = (unsigned int  )13;
  sqlstm.sqhsts[0] = (         int  )0;
  sqlstm.sqindv[0] = (         void  *)0;
  sqlstm.sqinds[0] = (         int  )0;
  sqlstm.sqharm[0] = (unsigned int  )0;
  sqlstm.sqadto[0] = (unsigned short )0;
  sqlstm.sqtdso[0] = (unsigned short )0;
  sqlstm.sqhstv[1] = (         void  *)&vOpen;
  sqlstm.sqhstl[1] = (unsigned int  )sizeof(float);
  sqlstm.sqhsts[1] = (         int  )0;
  sqlstm.sqindv[1] = (         void  *)0;
  sqlstm.sqinds[1] = (         int  )0;
  sqlstm.sqharm[1] = (unsigned int  )0;
  sqlstm.sqadto[1] = (unsigned short )0;
  sqlstm.sqtdso[1] = (unsigned short )0;
  sqlstm.sqhstv[2] = (         void  *)&vHigh;
  sqlstm.sqhstl[2] = (unsigned int  )sizeof(float);
  sqlstm.sqhsts[2] = (         int  )0;
  sqlstm.sqindv[2] = (         void  *)0;
  sqlstm.sqinds[2] = (         int  )0;
  sqlstm.sqharm[2] = (unsigned int  )0;
  sqlstm.sqadto[2] = (unsigned short )0;
  sqlstm.sqtdso[2] = (unsigned short )0;
  sqlstm.sqhstv[3] = (         void  *)&vLow;
  sqlstm.sqhstl[3] = (unsigned int  )sizeof(float);
  sqlstm.sqhsts[3] = (         int  )0;
  sqlstm.sqindv[3] = (         void  *)0;
  sqlstm.sqinds[3] = (         int  )0;
  sqlstm.sqharm[3] = (unsigned int  )0;
  sqlstm.sqadto[3] = (unsigned short )0;
  sqlstm.sqtdso[3] = (unsigned short )0;
  sqlstm.sqhstv[4] = (         void  *)&vClose;
  sqlstm.sqhstl[4] = (unsigned int  )sizeof(float);
  sqlstm.sqhsts[4] = (         int  )0;
  sqlstm.sqindv[4] = (         void  *)0;
  sqlstm.sqinds[4] = (         int  )0;
  sqlstm.sqharm[4] = (unsigned int  )0;
  sqlstm.sqadto[4] = (unsigned short )0;
  sqlstm.sqtdso[4] = (unsigned short )0;
  sqlstm.sqhstv[5] = (         void  *)&vVolume;
  sqlstm.sqhstl[5] = (unsigned int  )sizeof(float);
  sqlstm.sqhsts[5] = (         int  )0;
  sqlstm.sqindv[5] = (         void  *)0;
  sqlstm.sqinds[5] = (         int  )0;
  sqlstm.sqharm[5] = (unsigned int  )0;
  sqlstm.sqadto[5] = (unsigned short )0;
  sqlstm.sqtdso[5] = (unsigned short )0;
  sqlstm.sqphsv = sqlstm.sqhstv;
  sqlstm.sqphsl = sqlstm.sqhstl;
  sqlstm.sqphss = sqlstm.sqhsts;
  sqlstm.sqpind = sqlstm.sqindv;
  sqlstm.sqpins = sqlstm.sqinds;
  sqlstm.sqparm = sqlstm.sqharm;
  sqlstm.sqparc = sqlstm.sqharc;
  sqlstm.sqpadto = sqlstm.sqadto;
  sqlstm.sqptdso = sqlstm.sqtdso;
  sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


		if (sqlca.sqlcode==0) {
			strcpy(oBarTime[i], vNDT);
			oBarData[5*i+0] = vOpen;
			oBarData[5*i+1] = vHigh;
			oBarData[5*i+2] = vLow;
			oBarData[5*i+3] = vClose;
			oBarData[5*i+4] = vVolume;
		} else if (sqlca.sqlcode==1403) {
			break;
		} else {
			err("%s failed. stmt = %s\n Error %s", __func__, stmt, sqlca.sqlerrm.sqlerrmc);
			retval = sqlca.sqlcode;
			break;
		}
	}
	//-- one more fetch to get baseBar
	/* EXEC SQL FETCH cOHLCV into : vNDT, : vOpen, : vHigh, : vLow, : vClose, : vVolume; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )173;
 sqlstm.selerr = (unsigned short)1;
 sqlstm.sqlpfmem = (unsigned int  )0;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlstm.sqfoff = (           int )0;
 sqlstm.sqfmod = (unsigned int )2;
 sqlstm.sqhstv[0] = (         void  *)vNDT;
 sqlstm.sqhstl[0] = (unsigned int  )13;
 sqlstm.sqhsts[0] = (         int  )0;
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqhstv[1] = (         void  *)&vOpen;
 sqlstm.sqhstl[1] = (unsigned int  )sizeof(float);
 sqlstm.sqhsts[1] = (         int  )0;
 sqlstm.sqindv[1] = (         void  *)0;
 sqlstm.sqinds[1] = (         int  )0;
 sqlstm.sqharm[1] = (unsigned int  )0;
 sqlstm.sqadto[1] = (unsigned short )0;
 sqlstm.sqtdso[1] = (unsigned short )0;
 sqlstm.sqhstv[2] = (         void  *)&vHigh;
 sqlstm.sqhstl[2] = (unsigned int  )sizeof(float);
 sqlstm.sqhsts[2] = (         int  )0;
 sqlstm.sqindv[2] = (         void  *)0;
 sqlstm.sqinds[2] = (         int  )0;
 sqlstm.sqharm[2] = (unsigned int  )0;
 sqlstm.sqadto[2] = (unsigned short )0;
 sqlstm.sqtdso[2] = (unsigned short )0;
 sqlstm.sqhstv[3] = (         void  *)&vLow;
 sqlstm.sqhstl[3] = (unsigned int  )sizeof(float);
 sqlstm.sqhsts[3] = (         int  )0;
 sqlstm.sqindv[3] = (         void  *)0;
 sqlstm.sqinds[3] = (         int  )0;
 sqlstm.sqharm[3] = (unsigned int  )0;
 sqlstm.sqadto[3] = (unsigned short )0;
 sqlstm.sqtdso[3] = (unsigned short )0;
 sqlstm.sqhstv[4] = (         void  *)&vClose;
 sqlstm.sqhstl[4] = (unsigned int  )sizeof(float);
 sqlstm.sqhsts[4] = (         int  )0;
 sqlstm.sqindv[4] = (         void  *)0;
 sqlstm.sqinds[4] = (         int  )0;
 sqlstm.sqharm[4] = (unsigned int  )0;
 sqlstm.sqadto[4] = (unsigned short )0;
 sqlstm.sqtdso[4] = (unsigned short )0;
 sqlstm.sqhstv[5] = (         void  *)&vVolume;
 sqlstm.sqhstl[5] = (unsigned int  )sizeof(float);
 sqlstm.sqhsts[5] = (         int  )0;
 sqlstm.sqindv[5] = (         void  *)0;
 sqlstm.sqinds[5] = (         int  )0;
 sqlstm.sqharm[5] = (unsigned int  )0;
 sqlstm.sqadto[5] = (unsigned short )0;
 sqlstm.sqtdso[5] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	if (sqlca.sqlcode==0) {
		strcpy(oBaseTime, vNDT);
		oBaseBar[0] = vOpen;
		oBaseBar[1] = vHigh;
		oBaseBar[2] = vLow;
		oBaseBar[3] = vClose;
		oBaseBar[4] = vVolume;
	} else if (sqlca.sqlcode!=1403) {
		err("%s failed. stmt = %s\n Error %s", __func__, stmt, sqlca.sqlerrm.sqlerrmc);
		retval = sqlca.sqlcode;
	}

	/* EXEC SQL CLOSE cOHLCV; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )212;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}



	return (retval==0);
}

//=== Logging functions
EXPORT bool Ora_LogSaveMSE(tDbg* dbg, tDBConnection* db, int pid, int tid, int mseCnt, float* mseT, float* mseV) {
	/* EXEC SQL BEGIN DECLARE SECTION; */ 

	int i;
	int vInsertCount;
	sql_context vCtx = db->DBCtx;
	char stmt[1000];
	//--
	int* vProcessId;
	int* vThreadId;
	int* vEpoch;
	double* vMSE_T;
	double* vMSE_V;
	//--
	int dbl_type = 22;	// Oracle SQLT_BDOUBLE
	int int_type = 3;	// 
	unsigned int dbl_len = sizeof(double);
	unsigned int vchar_type = 96;
	unsigned int vchar_len = 12+1;

	/* EXEC SQL END   DECLARE SECTION; */ 


	//-- Connects to DB only once
	if (vCtx==NULL) {
		if (!OraConnect(dbg, db)) {
			err("%s() could not connect to Log Database...\n", __func__);
			return false;
		}
		vCtx = db->DBCtx;
	}
	err("%s() CheckPoint 2 - LogDB->DBCtx=%p , vCtx=%p\n", __func__, db->DBCtx, vCtx);

	vInsertCount=mseCnt;
	vProcessId=(int*)malloc(vInsertCount*sizeof(int));
	vThreadId=(int*)malloc(vInsertCount*sizeof(int));
	vEpoch = (int*)malloc(vInsertCount*sizeof(int));
	vMSE_T = (double*)malloc(vInsertCount*sizeof(double));
	vMSE_V = (double*)malloc(vInsertCount*sizeof(double));

	for (i = 0; i < vInsertCount; i++) {
		vProcessId[i]=pid;
		vThreadId[i]=tid;
		vEpoch[i] = i;
		vMSE_T[i] = mseT[i];
		vMSE_V[i] = mseV[i];
	}

	//-- Then, Build the Insert statement
	sprintf(&stmt[0], "insert into TrainLog(ProcessId, ThreadId, Epoch, MSE_T, MSE_V) values(:P01, :P02, :P03, :P04, :P05)");
	err("%s() CheckPoint 4 - stmt='%s' ; pid[0]=%d ; tid[0]=%d\n", __func__, stmt, pid, tid);

	/* EXEC SQL CONTEXT USE :vCtx; */ 

	//EXEC SQL ALTER SESSION SET EVENTS '10046 trace name CONTEXT forever, level 4';
	//EXEC SQL ALTER SESSION SET SQL_TRACE = TRUE;
	/* EXEC SQL FOR :vInsertCount ALLOCATE DESCRIPTOR 'inMSE'; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )227;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )100;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL SET DESCRIPTOR 'inMSE' COUNT = 5; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )247;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )5;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inMSE' VALUE 1 DATA = :vProcessId; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )267;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )1;
 sqlstm.sqhstv[0] = (         void  *)vProcessId;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(int);
 sqlstm.sqhsts[0] = (         int  )sizeof(int);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inMSE' VALUE 2 DATA = :vThreadId; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )291;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )2;
 sqlstm.sqhstv[0] = (         void  *)vThreadId;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(int);
 sqlstm.sqhsts[0] = (         int  )sizeof(int);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inMSE' VALUE 3 DATA = :vEpoch; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )315;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )3;
 sqlstm.sqhstv[0] = (         void  *)vEpoch;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(int);
 sqlstm.sqhsts[0] = (         int  )sizeof(int);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inMSE' VALUE 4 DATA = :vMSE_T; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )339;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )4;
 sqlstm.sqhstv[0] = (         void  *)vMSE_T;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(double);
 sqlstm.sqhsts[0] = (         int  )sizeof(double);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inMSE' VALUE 5 DATA = :vMSE_V; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )363;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )5;
 sqlstm.sqhstv[0] = (         void  *)vMSE_V;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(double);
 sqlstm.sqhsts[0] = (         int  )sizeof(double);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL PREPARE DynIns FROM :stmt; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )387;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlstm.sqhstv[0] = (         void  *)stmt;
 sqlstm.sqhstl[0] = (unsigned int  )1000;
 sqlstm.sqhsts[0] = (         int  )0;
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount EXECUTE DynIns USING DESCRIPTOR 'inMSE'; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )406;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	//EXEC SQL ALTER SESSION SET SQL_TRACE = FALSE;

	//pInsertCount[vNetLevel] = sqlca.sqlerrd[2];
	err("BulkMSEInsert() inserted %d rows.\n", sqlca.sqlerrd[2]);
	if (sqlca.sqlcode!=0) {
		if (sqlca.sqlcode!=1) err("%s failed. stmt = %s\n Error %s\n", __func__, stmt, sqlca.sqlerrm.sqlerrmc);
		return false;
	}
	/* EXEC SQL DEALLOCATE DESCRIPTOR 'inMSE'; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )426;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}



	//-- free()s
	free(vProcessId);
	free(vThreadId);
	free(vEpoch);
	free(vMSE_T);
	free(vMSE_V);

	return (sqlca.sqlcode==0);
}
EXPORT bool Ora_LogSaveW(tDbg* dbg, tDBConnection* db, int pid, int tid, int epoch, int Wcnt, numtype* W) {
	/* EXEC SQL BEGIN DECLARE SECTION; */ 

	int i;
	int vInsertCount;
	sql_context vCtx = db->DBCtx;
	char stmt[1000];
	//--
	int* vProcessId;
	int* vThreadId;
	int* vEpoch;
	int* vWid;
	double* vW;
	//--
	int dbl_type = 22;	// Oracle SQLT_BDOUBLE
	int int_type = 3;	// 
	unsigned int dbl_len = sizeof(double);
	unsigned int vchar_type = 96;
	unsigned int vchar_len = 12+1;

	/* EXEC SQL END   DECLARE SECTION; */ 


	//-- Connects to DB only once
	if (vCtx==NULL) {
		if (!OraConnect(dbg, db)) {
			err("%s() could not connect to Log Database...\n", __func__);
			return false;
		}
		vCtx = db->DBCtx;
	}
	err("%s() CheckPoint 2 - LogDB->DBCtx=%p , vCtx=%p\n", __func__, db->DBCtx, vCtx);

	vInsertCount=Wcnt;
	vProcessId=(int*)malloc(vInsertCount*sizeof(int));
	vThreadId=(int*)malloc(vInsertCount*sizeof(int));
	vEpoch = (int*)malloc(vInsertCount*sizeof(int));
	vWid = (int*)malloc(vInsertCount*sizeof(int));
	vW = (double*)malloc(vInsertCount*sizeof(double));

	for (i = 0; i < vInsertCount; i++) {
		vProcessId[i]=pid;
		vThreadId[i]=tid;
		vEpoch[i] = epoch;
		vWid[i]=i;
		vW[i]=W[i];
	}

	//-- Then, Build the Insert statement
	sprintf(&stmt[0], "insert into CoreImage_NN (ProcessId, ThreadId, Epoch, WId, W) values(:P01, :P02, :P03, :P04, :P05)");
	err("%s() CheckPoint 4 - stmt='%s' ; pid[0]=%d ; tid[0]=%d\n", __func__, stmt, pid, tid);

	/* EXEC SQL CONTEXT USE :vCtx; */ 

	//EXEC SQL ALTER SESSION SET EVENTS '10046 trace name CONTEXT forever, level 4';
	//EXEC SQL ALTER SESSION SET SQL_TRACE = TRUE;
	/* EXEC SQL FOR :vInsertCount ALLOCATE DESCRIPTOR 'inCINN'; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )446;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )100;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL SET DESCRIPTOR 'inCINN' COUNT = 5; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )467;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )5;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inCINN' VALUE 1 DATA = :vProcessId; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )488;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )1;
 sqlstm.sqhstv[0] = (         void  *)vProcessId;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(int);
 sqlstm.sqhsts[0] = (         int  )sizeof(int);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inCINN' VALUE 2 DATA = :vThreadId; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )513;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )2;
 sqlstm.sqhstv[0] = (         void  *)vThreadId;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(int);
 sqlstm.sqhsts[0] = (         int  )sizeof(int);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inCINN' VALUE 3 DATA = :vEpoch; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )538;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )3;
 sqlstm.sqhstv[0] = (         void  *)vEpoch;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(int);
 sqlstm.sqhsts[0] = (         int  )sizeof(int);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inCINN' VALUE 4 DATA = :vWid; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )563;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )4;
 sqlstm.sqhstv[0] = (         void  *)vWid;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(int);
 sqlstm.sqhsts[0] = (         int  )sizeof(int);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inCINN' VALUE 5 DATA = :vW; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )588;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )5;
 sqlstm.sqhstv[0] = (         void  *)vW;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(double);
 sqlstm.sqhsts[0] = (         int  )sizeof(double);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL PREPARE DynIns FROM :stmt; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )613;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlstm.sqhstv[0] = (         void  *)stmt;
 sqlstm.sqhstl[0] = (unsigned int  )1000;
 sqlstm.sqhsts[0] = (         int  )0;
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount EXECUTE DynIns USING DESCRIPTOR 'inCINN'; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )632;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	//EXEC SQL ALTER SESSION SET SQL_TRACE = FALSE;

	err("%s() inserted %d rows.\n", __func__, sqlca.sqlerrd[2]);
	if (sqlca.sqlcode!=0) {
		if (sqlca.sqlcode!=1) err("%s failed. stmt = %s\n Error %s", __func__, stmt, sqlca.sqlerrm.sqlerrmc);
		return (sqlca.sqlcode==0);
	}
	/* EXEC SQL DEALLOCATE DESCRIPTOR 'inCINN'; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )653;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}



	//-- free()s
	free(vProcessId);
	free(vThreadId);
	free(vEpoch);
	free(vWid);
	free(vW);

	return (sqlca.sqlcode==0);
}
EXPORT bool Ora_LogSaveClient(tDbg* dbg, tDBConnection* db, int pid, char* clientName, DWORD startTime, DWORD duration, int simulLen, char* simulStart, bool doTrain, bool doTrainRun, bool doTestRun) {
	/* EXEC SQL BEGIN DECLARE SECTION; */ 

	sql_context vCtx = db->DBCtx;
	char stmt[1000];
	/* EXEC SQL END DECLARE SECTION; */ 


	//-- Connects to DB only once
	if (vCtx==NULL) {
		if (!OraConnect(dbg, db)) {
			err("%s() could not connect to Log Database...\n", __func__);
			return false;
		}
		vCtx = db->DBCtx;
	}
	//-- Builds Insert statement
	sprintf(&stmt[0], "insert into ClientInfo(ProcessId, ClientName, ClientStart, SimulationLen, Duration, SimulationStart, DoTraining, DoTrainRun, DoTestRun) values(%d, '%s', sysdate, %d, %ld, to_date('%s','YYYYMMDDHH24MI'), %d, %d, %d)", pid, clientName, simulLen, (DWORD)(duration/1000), simulStart, (doTrain)?1:0, (doTrainRun)?1:0, (doTestRun)?1:0);
	//-- Executes Insert statement
	/* EXEC SQL CONTEXT USE : vCtx; */ 

	/* EXEC SQL EXECUTE IMMEDIATE : stmt; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )674;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlstm.sqhstv[0] = (         void  *)stmt;
 sqlstm.sqhstl[0] = (unsigned int  )1000;
 sqlstm.sqhsts[0] = (         int  )0;
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	if (sqlca.sqlcode!=0) {
		err("%s failed. stmt = %s\n Error %s", __func__, stmt, sqlca.sqlerrm.sqlerrmc);
		return false;
	}
	return true;
}
EXPORT bool Ora_LogSaveRun(tDbg* dbg, tDBConnection* db, int pid, int tid, int setid, int npid, int ntid, int barCnt, int featuresCnt, int* feature, numtype* prediction, numtype* actual) {
	/* EXEC SQL BEGIN DECLARE SECTION; */ 

	int i, b, f;
	int vInsertCount;
	int vFeaturesCnt;
	sql_context vCtx = db->DBCtx;
	char stmt[1000];
	//--
	int* vProcessId;
	int* vThreadId;
	int* vSetId;
	int* vNetProcessId;
	int* vNetThreadId;
	int* vStep;
	int* vFeatureId;
	double* vPredictionTRS;
	double* vActualTRS;
	double* vErrorTRS;
	//--
	int dbl_type = 22;	// Oracle SQLT_BDOUBLE
	int int_type = 3;	// 
	unsigned int dbl_len = sizeof(double);
	unsigned int vchar_type = 96;
	unsigned int vchar_len = 12+1;

	/* EXEC SQL END   DECLARE SECTION; */ 


	//-- Connects to DB only once
	if (vCtx==NULL) {
		if (!OraConnect(dbg, db)) {
			err("%s() could not connect to Log Database...\n", __func__);
			return false;
		}
		vCtx = db->DBCtx;
	}
	err("%s() CheckPoint 2 - LogDB->DBCtx=%p , vCtx=%p\n", __func__, db->DBCtx, vCtx);

	vFeaturesCnt=featuresCnt;
	vInsertCount=barCnt*featuresCnt;
	vProcessId=(int*)malloc(vInsertCount*sizeof(int));
	vThreadId=(int*)malloc(vInsertCount*sizeof(int));
	vSetId=(int*)malloc(vInsertCount*sizeof(int));
	vNetProcessId=(int*)malloc(vInsertCount*sizeof(int));
	vNetThreadId=(int*)malloc(vInsertCount*sizeof(int));
	vStep = (int*)malloc(vInsertCount*sizeof(int));
	vFeatureId = (int*)malloc(vInsertCount*sizeof(int));
	vPredictionTRS=(double*)malloc(vInsertCount*sizeof(double));
	vActualTRS=(double*)malloc(vInsertCount*sizeof(double));
	vErrorTRS=(double*)malloc(vInsertCount*sizeof(double));

	i=0;
	for (b=0; b<barCnt; b++) {
		for (f=0; f<featuresCnt; f++) {
			vProcessId[i]=pid;
			vThreadId[i]=tid;
			vSetId[i]=setid;
			vNetProcessId[i]=npid;
			vNetThreadId[i]=ntid;
			vStep[i] = b;
			vFeatureId[i]=feature[f];
			vPredictionTRS[i]=prediction[i];
			vActualTRS[i]=actual[i];
			vErrorTRS[i]=fabs(actual[i]-prediction[i]);

			i++;
		}
	}

	//-- Then, Build the Insert statement
	sprintf(&stmt[0], "insert into RunLog (ProcessId, ThreadId, SetId, NetProcessId, NetThreadId, Pos, FeatureId, PredictedTRS, ActualTRS, ErrorTRS) values(:P01, :P02, :P03, :P04, :P05, :P06, :P07, :P08, :P09, :P10)");
	err("%s() CheckPoint 4 - stmt='%s' ; pid[0]=%d ; tid[0]=%d\n", __func__, stmt, pid, tid);

	/* EXEC SQL CONTEXT USE :vCtx; */ 

	//EXEC SQL ALTER SESSION SET EVENTS '10046 trace name CONTEXT forever, level 4';
	//EXEC SQL ALTER SESSION SET SQL_TRACE = TRUE;
	/* EXEC SQL FOR :vInsertCount ALLOCATE DESCRIPTOR 'inRun'; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )693;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )100;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL SET DESCRIPTOR 'inRun' COUNT = 10; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )713;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )10;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inRun' VALUE 1 DATA = :vProcessId; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )733;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )1;
 sqlstm.sqhstv[0] = (         void  *)vProcessId;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(int);
 sqlstm.sqhsts[0] = (         int  )sizeof(int);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inRun' VALUE 2 DATA = :vThreadId; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )757;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )2;
 sqlstm.sqhstv[0] = (         void  *)vThreadId;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(int);
 sqlstm.sqhsts[0] = (         int  )sizeof(int);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inRun' VALUE 3 DATA = :vSetId; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )781;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )3;
 sqlstm.sqhstv[0] = (         void  *)vSetId;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(int);
 sqlstm.sqhsts[0] = (         int  )sizeof(int);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inRun' VALUE 4 DATA = :vNetProcessId; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )805;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )4;
 sqlstm.sqhstv[0] = (         void  *)vNetProcessId;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(int);
 sqlstm.sqhsts[0] = (         int  )sizeof(int);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inRun' VALUE 5 DATA = :vNetThreadId; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )829;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )5;
 sqlstm.sqhstv[0] = (         void  *)vNetThreadId;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(int);
 sqlstm.sqhsts[0] = (         int  )sizeof(int);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inRun' VALUE 6 DATA = :vStep; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )853;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )6;
 sqlstm.sqhstv[0] = (         void  *)vStep;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(int);
 sqlstm.sqhsts[0] = (         int  )sizeof(int);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inRun' VALUE 7 DATA = :vFeatureId; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )877;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )7;
 sqlstm.sqhstv[0] = (         void  *)vFeatureId;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(int);
 sqlstm.sqhsts[0] = (         int  )sizeof(int);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inRun' VALUE 8 DATA = :vPredictionTRS; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )901;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )8;
 sqlstm.sqhstv[0] = (         void  *)vPredictionTRS;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(double);
 sqlstm.sqhsts[0] = (         int  )sizeof(double);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inRun' VALUE 9 DATA = :vActualTRS; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )925;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )9;
 sqlstm.sqhstv[0] = (         void  *)vActualTRS;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(double);
 sqlstm.sqhsts[0] = (         int  )sizeof(double);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount SET DESCRIPTOR 'inRun' VALUE 10 DATA = :vErrorTRS; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )949;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )10;
 sqlstm.sqhstv[0] = (         void  *)vErrorTRS;
 sqlstm.sqhstl[0] = (unsigned int  )sizeof(double);
 sqlstm.sqhsts[0] = (         int  )sizeof(double);
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL PREPARE DynIns FROM :stmt; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )973;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlstm.sqhstv[0] = (         void  *)stmt;
 sqlstm.sqhstl[0] = (unsigned int  )1000;
 sqlstm.sqhsts[0] = (         int  )0;
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL FOR :vInsertCount EXECUTE DynIns USING DESCRIPTOR 'inRun'; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )vInsertCount;
 sqlstm.offset = (unsigned int  )992;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	//EXEC SQL ALTER SESSION SET SQL_TRACE = FALSE;

	//pInsertCount[vNetLevel] = sqlca.sqlerrd[2];
	err("%s() inserted %d rows.\n", __func__, sqlca.sqlerrd[2]);
	if (sqlca.sqlcode!=0) {
		if (sqlca.sqlcode!=1) err("%s failed. stmt = %s\n Error %s", __func__, stmt, sqlca.sqlerrm.sqlerrmc);
		return false;
	}
	/* EXEC SQL DEALLOCATE DESCRIPTOR 'inRun'; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )1012;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}



	//-- free()s
	free(vProcessId);
	free(vThreadId);
	free(vSetId);
	free(vNetProcessId);
	free(vNetThreadId);
	free(vStep);
	free(vFeatureId);
	free(vPredictionTRS);
	free(vActualTRS);
	free(vErrorTRS);

	return (sqlca.sqlcode==0);
}

EXPORT bool Ora_LogLoadW(tDbg* dbg, tDBConnection* db, int pid, int tid, int epoch, int Wcnt, numtype* oW) {
	/* EXEC SQL BEGIN DECLARE SECTION; */ 

	int i;
	int retval=0;
	sql_context vCtx = db->DBCtx;
	char stmt[1000];
	//-- Network Weights variables
	int vPid=pid; int vTid=tid;	int vEpoch=epoch; 
	int vWid; double vW;
	/* EXEC SQL END DECLARE SECTION; */ 


	//-- Connects to DB only once
	if (vCtx==NULL) {
		if (!OraConnect(dbg, db)) {
			err("%s() could not connect to Log Database...\n", __func__);
			return false;
		}
		vCtx = db->DBCtx;
	}

	//-- Load Network Weights from CoreImage_NN
	/* EXEC SQL CONTEXT USE : vCtx; */ 


	if (vEpoch==-1) {
		/* EXEC SQL select max(epoch) into :vEpoch from CoreImage_NN where ProcessId= :vPid and ThreadId= :vTid; */ 

{
  struct sqlexd sqlstm;
  sqlstm.sqlvsn = 13;
  sqlstm.arrsiz = 6;
  sqlstm.sqladtp = &sqladt;
  sqlstm.sqltdsp = &sqltds;
  sqlstm.stmt = "select max(epoch) into :b0  from CoreImage_NN where (Proces\
sId=:b1 and ThreadId=:b2)";
  sqlstm.iters = (unsigned int  )1;
  sqlstm.offset = (unsigned int  )1032;
  sqlstm.selerr = (unsigned short)1;
  sqlstm.sqlpfmem = (unsigned int  )0;
  sqlstm.cud = sqlcud0;
  sqlstm.sqlest = (unsigned char  *)&sqlca;
  sqlstm.sqlety = (unsigned short)4352;
  sqlstm.occurs = (unsigned int  )0;
  sqlstm.sqhstv[0] = (         void  *)&vEpoch;
  sqlstm.sqhstl[0] = (unsigned int  )sizeof(int);
  sqlstm.sqhsts[0] = (         int  )0;
  sqlstm.sqindv[0] = (         void  *)0;
  sqlstm.sqinds[0] = (         int  )0;
  sqlstm.sqharm[0] = (unsigned int  )0;
  sqlstm.sqadto[0] = (unsigned short )0;
  sqlstm.sqtdso[0] = (unsigned short )0;
  sqlstm.sqhstv[1] = (         void  *)&vPid;
  sqlstm.sqhstl[1] = (unsigned int  )sizeof(int);
  sqlstm.sqhsts[1] = (         int  )0;
  sqlstm.sqindv[1] = (         void  *)0;
  sqlstm.sqinds[1] = (         int  )0;
  sqlstm.sqharm[1] = (unsigned int  )0;
  sqlstm.sqadto[1] = (unsigned short )0;
  sqlstm.sqtdso[1] = (unsigned short )0;
  sqlstm.sqhstv[2] = (         void  *)&vTid;
  sqlstm.sqhstl[2] = (unsigned int  )sizeof(int);
  sqlstm.sqhsts[2] = (         int  )0;
  sqlstm.sqindv[2] = (         void  *)0;
  sqlstm.sqinds[2] = (         int  )0;
  sqlstm.sqharm[2] = (unsigned int  )0;
  sqlstm.sqadto[2] = (unsigned short )0;
  sqlstm.sqtdso[2] = (unsigned short )0;
  sqlstm.sqphsv = sqlstm.sqhstv;
  sqlstm.sqphsl = sqlstm.sqhstl;
  sqlstm.sqphss = sqlstm.sqhsts;
  sqlstm.sqpind = sqlstm.sqindv;
  sqlstm.sqpins = sqlstm.sqinds;
  sqlstm.sqparm = sqlstm.sqharm;
  sqlstm.sqparc = sqlstm.sqharc;
  sqlstm.sqpadto = sqlstm.sqadto;
  sqlstm.sqptdso = sqlstm.sqtdso;
  sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	}
	if (vEpoch==-1) {
		err("%s failed. Could not find max(epoch) for ProcessId=%d, ThreadId=%d\n", __func__, vPid, vTid);
		return false;
	}

	sprintf(&stmt[0], "select WId, W from CoreImage_NN where ProcessId=%d and ThreadId=%d and Epoch=%d order by 1,2", pid, tid, vEpoch);
	
	err("%s() CheckPoint 3 - stmt=%s\n", __func__, stmt);
	/* EXEC SQL PREPARE sLoadW FROM :stmt; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )1059;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlstm.sqhstv[0] = (         void  *)stmt;
 sqlstm.sqhstl[0] = (unsigned int  )1000;
 sqlstm.sqhsts[0] = (         int  )0;
 sqlstm.sqindv[0] = (         void  *)0;
 sqlstm.sqinds[0] = (         int  )0;
 sqlstm.sqharm[0] = (unsigned int  )0;
 sqlstm.sqadto[0] = (unsigned short )0;
 sqlstm.sqtdso[0] = (unsigned short )0;
 sqlstm.sqphsv = sqlstm.sqhstv;
 sqlstm.sqphsl = sqlstm.sqhstl;
 sqlstm.sqphss = sqlstm.sqhsts;
 sqlstm.sqpind = sqlstm.sqindv;
 sqlstm.sqpins = sqlstm.sqinds;
 sqlstm.sqparm = sqlstm.sqharm;
 sqlstm.sqparc = sqlstm.sqharc;
 sqlstm.sqpadto = sqlstm.sqadto;
 sqlstm.sqptdso = sqlstm.sqtdso;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	/* EXEC SQL DECLARE cLoadW CURSOR FOR sLoadW; */ 

	/* EXEC SQL OPEN cLoadW; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.stmt = "";
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )1078;
 sqlstm.selerr = (unsigned short)1;
 sqlstm.sqlpfmem = (unsigned int  )0;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlstm.sqcmod = (unsigned int )0;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


	for(i=0; i<Wcnt; i++) {
		/* EXEC SQL FETCH cLoadW INTO vWid, vW; */ 

{
  struct sqlexd sqlstm;
  sqlstm.sqlvsn = 13;
  sqlstm.arrsiz = 6;
  sqlstm.sqladtp = &sqladt;
  sqlstm.sqltdsp = &sqltds;
  sqlstm.iters = (unsigned int  )1;
  sqlstm.offset = (unsigned int  )1093;
  sqlstm.selerr = (unsigned short)1;
  sqlstm.sqlpfmem = (unsigned int  )0;
  sqlstm.cud = sqlcud0;
  sqlstm.sqlest = (unsigned char  *)&sqlca;
  sqlstm.sqlety = (unsigned short)4352;
  sqlstm.occurs = (unsigned int  )0;
  sqlstm.sqfoff = (           int )0;
  sqlstm.sqfmod = (unsigned int )2;
  sqlstm.sqhstv[0] = (         void  *)&vWid;
  sqlstm.sqhstl[0] = (unsigned int  )sizeof(int);
  sqlstm.sqhsts[0] = (         int  )0;
  sqlstm.sqindv[0] = (         void  *)0;
  sqlstm.sqinds[0] = (         int  )0;
  sqlstm.sqharm[0] = (unsigned int  )0;
  sqlstm.sqadto[0] = (unsigned short )0;
  sqlstm.sqtdso[0] = (unsigned short )0;
  sqlstm.sqhstv[1] = (         void  *)&vW;
  sqlstm.sqhstl[1] = (unsigned int  )sizeof(double);
  sqlstm.sqhsts[1] = (         int  )0;
  sqlstm.sqindv[1] = (         void  *)0;
  sqlstm.sqinds[1] = (         int  )0;
  sqlstm.sqharm[1] = (unsigned int  )0;
  sqlstm.sqadto[1] = (unsigned short )0;
  sqlstm.sqtdso[1] = (unsigned short )0;
  sqlstm.sqphsv = sqlstm.sqhstv;
  sqlstm.sqphsl = sqlstm.sqhstl;
  sqlstm.sqphss = sqlstm.sqhsts;
  sqlstm.sqpind = sqlstm.sqindv;
  sqlstm.sqpins = sqlstm.sqinds;
  sqlstm.sqparm = sqlstm.sqharm;
  sqlstm.sqparc = sqlstm.sqharc;
  sqlstm.sqpadto = sqlstm.sqadto;
  sqlstm.sqptdso = sqlstm.sqtdso;
  sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}


		if (sqlca.sqlcode==0) {
			oW[i] = (numtype)vW;
		} else if (sqlca.sqlcode==1403) {
			break;
		} else {
			err("%s failed. stmt = %s\n Error %s\n", __func__, stmt, sqlca.sqlerrm.sqlerrmc);
			retval = sqlca.sqlcode;
			break;
		}
	}
	/* EXEC SQL CLOSE cLoadW; */ 

{
 struct sqlexd sqlstm;
 sqlstm.sqlvsn = 13;
 sqlstm.arrsiz = 6;
 sqlstm.sqladtp = &sqladt;
 sqlstm.sqltdsp = &sqltds;
 sqlstm.iters = (unsigned int  )1;
 sqlstm.offset = (unsigned int  )1116;
 sqlstm.cud = sqlcud0;
 sqlstm.sqlest = (unsigned char  *)&sqlca;
 sqlstm.sqlety = (unsigned short)4352;
 sqlstm.occurs = (unsigned int  )0;
 sqlcxt(&vCtx, &sqlctx, &sqlstm, &sqlfpn);
}



	return (retval==0);
}