#pragma once
#define mymalloc malloc

#include <wininet.h>
#include <atlcoll.h>
#include "ml_api.h"
#include "rv.h"
#include "CSizer.h"
#include "CLinkHandle.h"
#include "CSubclasser2.h"
#include "Base64_RC4.h"
#include "CReg.h"
#include "CLogger.h"
#ifdef CPPDEBUG
#include "StackWalker.h"
#include "eh.h"
#include "resource.h"
#endif
#include "CBaseServerManager.h"

#define PING_PORT		9930
#define PING_PERIOD		30
#define PING_VER		0x03

#define AUTOLICENSE_PREFIX_LENGTH	11
#define AUTOLICENSE_PREFIX "autolicense"
#define MIN_PASSWORD_LENGTH	6

extern CSubclasser2* g_sc;
extern HANDLE g_hLic;
extern char g_projectName[ 100 ];//api.cpp
extern CAtlArray<TCHAR*> g_strings;
extern CLogger* g_logger;

#ifdef CPPDEBUG
extern CSimpleArray<const char*> g_incompleteTries;


#define TRY		_se_translator_function seOld = _set_se_translator( exceptionPreFilter );\
				g_incompleteTries.Add( __FUNCTION__ ); \
				try \
				{ 
#define CATCH	}catch( CMyException *e ){ exceptionFilter( __FUNCTION__, e ); } \
				_set_se_translator( seOld );\
				int ii = g_incompleteTries.Find( __FUNCTION__ );\
				if( ii == -1 ) dbg( "!! ii %s", __FUNCTION__ );\
				g_incompleteTries.RemoveAt( ii );
#else
#define TRY	
#define CATCH 
#endif


#ifdef UNICODE
#define TVAR( var ) t##var
#define U2A( var ) CW2A var( t##var )
#else
#define TVAR( var ) var
#define U2A( unused1, unused2 ) 
#endif

#ifndef UNICODE
	typedef unsigned char UTCHAR;
#else
	typedef wchar_t UTCHAR; 
#endif

class CMyException
{
	public:
		unsigned int code;
		EXCEPTION_POINTERS *ep;
	public:
		CMyException( unsigned int c, EXCEPTION_POINTERS* p ){ code = c; ep = p; }
};

void exceptionPreFilter( unsigned int u, EXCEPTION_POINTERS* p );
void exceptionFilter( const char* func, CMyException *e );

#ifdef CPPDEBUG
void bin_to_strhex( const BYTE*bin, unsigned int binsz, char **result);
#endif

#ifdef CPPDEBUG
void dbg( char* _fmt, ... );
void dbg( wchar_t* _fmt, ... );
#else
#define dbg
#endif

#ifdef CPPDEBUG
#include <io.h> 
class MyStackWalker : public StackWalker
{
private:
	int m_h;
public:
  MyStackWalker( int h ) : StackWalker() { m_h = h; }
  MyStackWalker(DWORD dwProcessId, HANDLE hProcess) : StackWalker(dwProcessId, hProcess) {}
  virtual void OnOutput( LPCSTR szText )
  { 
		_write( m_h, szText, strlen( szText ) ); /*StackWalker::OnOutput(szText);*/ 
		_write( m_h, "\n", 1 );
	}
};
#endif

#define _WM_REMOVE_ICON			( WM_USER + 200 )
#define _WM_ADD_ICON			( WM_USER + 201 )

// deinit reasons
#define DEINIT_REASON_NONE			0	//Script finished its execution independently.
#define DEINIT_REASON_REMOVE		1	//Expert removed from chart.
#define DEINIT_REASON_RECOMPILE		2	//Expert recompiled.
#define DEINIT_REASON_CHARTCHANGE	3	//symbol or timeframe changed on the chart.
#define DEINIT_REASON_CHARTCLOSE	4	//Chart closed.
#define DEINIT_REASON_PARAMETERS	5	//Inputs parameters was changed by user.
#define DEINIT_REASON_ACCOUNT		6	//Other account activated.

#define COMPILED_IMAGE_BASE		0x10000000

#define ECS(h) WaitForSingleObject( h, INFINITE )
#define LCS(h) ReleaseMutex( h )

#define GEN_EXT_ID_LENGTH		20
#define GEN_LIC_ID_LENGTH		32
#define GEN_FSAUTH_LENGTH		32
#define GEN_LICENSE_KEY_LENGTH	128

#define REGISTER_CODE_LENGTH	4

class CAccounts;

#if defined( GEN_LT_REMOTE )
#	ifndef _DEBUG
#		define MAX_AUTH_DURATION		30	//seconds
#	else
#		define MAX_AUTH_DURATION		30
#	endif
#else
#	define MAX_AUTH_DURATION		30	//seconds
#endif

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)


#define DECR( str, key )			CRC4 rc4;\
									DECR2( str, key );
#define DECR2( str, key )			ENCR( str, key )
#define ENCR( str, key )			rc4.Encrypt( str, sizeof( str ), key, sizeof( key ) )

enum LicenseType
{
	LT_OPEN,
	LT_LOCAL_ACCOUNT_NO,
	LT_LOCAL_LOGIN,
	LT_LOCAL_CID,
	LT_LOCAL_RECEIPT,
	LT_REMOTE_OPEN,
	LT_REMOTE_ACCOUNT_NO,
	LT_REMOTE_LOGIN,
	LT_REMOTE_CID,
	LT_REMOTE_RECEIPT,
	LT_REMOTE_AUTO
};

class CBaseAuth
{
	public:
		char		server[ 100 ];
		char		baseUri[ 100 ];
		int			port;

	public:
		CBaseAuth( const char* server, int port, const char* baseUri )
		{
			lstrcpyA( this->server, server );
			this->port = port;
			lstrcpyA( this->baseUri, baseUri );
		}
};

class CRegistrationData : public CBaseAuth
{
	public:
		char	name[ 100 ];
		char	email[ 100 ];
		char	code[ REGISTER_CODE_LENGTH + 1 ];

	public:
		CRegistrationData( const char* server, int port, const char* baseUri ):
			CBaseAuth( server, port, baseUri )
		{
		}
};

class CMemoryManager
{
	private:
		static const int m_bufferSize = 102400;
		struct MemoryBlock
		{
			void* buf;
			DWORD lastAddTime;
			DWORD offset;
		};

		MemoryBlock *m_blocks;
		MemoryBlock *m_curBlock;
		DWORD m_totalBlocks;
		
		HANDLE m_hStopped;		
		HANDLE m_hThread;	
		DWORD m_ttl;		
		HANDLE m_hMutex;
		DWORD m_tid;

	private:
		static DWORD WINAPI threadProc( CMemoryManager* me );
		static __int64 getThreadTime();
		static __int64 getThreadTime( DWORD tid );
		void addBlock( DWORD len );
		void initBlocks();
		void freeBlocks();

	public:
		//void add( void* buf );
		void* add( DWORD len );
		DWORD getTid(){ return( m_tid ); }
		
		BOOL start();
		BOOL stop();
		
		void releaseByTid( DWORD tid );

	public:	
		CMemoryManager( DWORD ttl );
		~CMemoryManager();
};
/*
class CMemoryPool
{
	private:
		DWORD m_ttl;
		DWORD m_chunkSize;
		DWORD m_growBy;
		void* m_buf;
		DWORD* m_chunkCreation;
		HANDLE m_hMutex;
		DWORD m_nChunks;
	public:
		void* add();
	public:
		CMemoryPool( DWORD chunkSize, DWORD ttl, DWORD growBy = 10 );
		~CMemoryPool();
};
*/
/*#define TB_REALLOC( FIELD, SIZE )	ThreadBuffers* tb = ( ThreadBuffers* )TlsGetValue( g_tls );\
									tb->FIELD = ( char* )realloc( tb->FIELD, SIZE );
		

struct ThreadBuffers
{
	char* ss;	//StringSubstr
	char* c2s;	//CharToStr
	char* d2s;	//DoubleToStr
	char* i2s;	//IntToStr
	char* sc;	//StringConcatenate
	char* stl;	//StringTrimLeft
	char* str;	//StringTrimRight
	char* ssc;	//StringSetChar
	char* tts;	//TimeToStr
	char* mrvs;	//mlapiReturnValueS
};*/

class CMLReg : public CReg
{
	public:
		int open( DWORD access = KEY_READ | KEY_WRITE );
};

struct TimerData
{
	HWND		hChart;
	DWORD		period;
	UINT_PTR	timer;
};

struct CalcInstancesData
{
	DWORD res;
	const char* email;
};

struct AutoUpdateData
{
	HWND dlg;
	BOOL cancelled;
	char supportEmail[ 128 ];
	char supportName[ 128 ];
	char datadir[ MAX_PATH ];
	BOOL isPortableMode;
	HANDLE hMutex;

};
struct AutoUpdateNotificationData
{
	char supportEmail[ 128 ];
	char supportName[ 128 ];
};

struct ThreadLoadLicData
{
	char* server;
	HANDLE mutex;
	DWORD initStart;
	OUT MLError internetRes;
	OUT BOOL done;
	OUT DWORD res;
	DWORD tid;//for debug only
	BOOL forceStop;
};

class CChartData
{
	public:
		//DWORD	tid;
		HWND	hChart;				//0x00
		MLError	err;				//0x04
#ifndef MLL
		HWND	hIcon;				
		HANDLE	hIconMutex;			
		HANDLE	hIconWaitThread;	
		HANDLE	hDone;				
		HANDLE	hMT4GUIMutex;		

		BOOL subclassed;			

		CSizer*	sz;					
		HICON	ico;				
	
	private:
		DWORD	instances;			
		int		propIndex;

	private:
		static DWORD WINAPI iconWait( CChartData* cd );
		void setChartIcon();
		void updateProp();

	public:
		void traceIcon();
		int incInstances();
		int decInstances();
		int getInstances(){ return( instances ); }
		HANDLE getProp( OUT BOOL* isSet );
		void setProp();

#endif
		BOOL	closed;			
		DWORD	closeTime;
		BOOL	isChartDestroyed;
		DWORD	prevDeinitReason;
	public:
		CChartData( HWND hChart );
		~CChartData();
};

#define ACC_REAL 1
#define ACC_DEMO 2

class CLogin
{
	private:
		const char* loginKey;
		const char* passKey;

	public:
		char login[ 100 ];
		char pass[ 100 ];
		long expiration;
		void* userData;
		DWORD accType;
	public:
		int load();
		int save();
	public:
		CLogin( LicenseType t );
};

class CReceipt
{
	public:
		char receipt[ 100 ];
		long expiration;
		void* userData;
		DWORD accType;
	public:
		int load();
		int save();
};

struct EncLogin //used for LT_REMOTE_LOGIN license type
{
	BYTE		login[ 100 ];
	BYTE		pass[ 100 ];
	DWORD		nLogin;
	DWORD		nPass;
	long		expiration;
	DWORD		accType;
	BOOL		enabled;
};

struct EncReceipt	//used for LT_REMOTE_RECEIPT license type
{
	BYTE		receipt[ 50 ];
	DWORD		nReceipt;
	long		expiration;
	DWORD		accType;
	BOOL		enabled;
};

class ModelessMsgBoxData
{
	public:
		char *text;
		char* title;
		DWORD style;
		BOOL deletable;

	public:
		ModelessMsgBoxData( const char* _text, const char* _title, DWORD _style = 0, BOOL _deletable = TRUE )
		{
			DWORD len = lstrlenA( _text ) + 1;
			text = ( char* )mymalloc( len );
			memcpy( text, _text, len );

			if( _title )
			{
				len = lstrlenA( _title ) + 1;
				title = ( char* )mymalloc( len );
				memcpy( title, _title, len );
			}
			else
			{
				title = 0;
			}

			style = _style;
			deletable = _deletable;
		}
		~ModelessMsgBoxData()
		{
			free( text );
			if( title )
			{
				free( title );
			}
		}
};

struct PingCfg
{
	u_short port;
	int period;
};

#ifdef GEN_LT_REMOTE
enum AutoUpdate
{
	AU_NO_NEED,
	AU_CANNOT,
	AU_NEED
};

class CLic
{
	private:
		 BYTE m_rc4Key[ 32 ];
		 CSimpleArray<EncLogin> m_logins;
		 CSimpleArray<EncReceipt> m_receipts;
		 enum RevOp
		 {
			 REV_OP_EQ,
			 REV_OP_LT
		 };
	public:
		LicenseType type;
		DWORD		accType;		//account type required by license
		long		expiration;
		long		userExpiration;
		BOOL		isValid;
		BOOL		skipForDemo;
		BOOL		hold;
		char*		holdMessage;
		BOOL		update;
		char*		updateUrl;
		char*		updateMessage;
		AutoUpdate	autoUpdate;
		void*		userData;
		char*		websiteUrl;
		char*		supportEmail;
		char*		supportName;
		
		char		lastCre1[ 100 ];
		char		lastCre2[ 100 ];

		PingCfg		ping;

		struct
		{
			DWORD maxCids;
			DWORD maxAccounts;
			DWORD maxInstances;
			DWORD usedCids;
			DWORD usedAccounts;
			DWORD usedInstances;
			
			DWORD userExpiration;
		}
		al; //auto license info
		
	public:
		BOOL load( char* xml, const BYTE* xmlKey, DWORD keyLen );
		BOOL isValidLogin2( CLogin* l, ThreadLoadLicData* lld );
		BOOL isValidReceipt2( CReceipt* r, ThreadLoadLicData* lld );
	private:	
		char* copyStr( char** dst, const char* src );
	public:
		CLic();
		CLic( const CLic& lic );
		~CLic();
};
void processAutoUpdate( CLic& lic) ;
#endif // GEN_LT_REMOTE

class CGlobalIndicator
{
	public:
		struct GlobalIndicatorItem
		{
			HWND m_hwndLabel;
			HWND m_hwndProgressBar;
		};

	private:
		HWND m_hwnd;
		enum GlobalIndicatorMsg
		{
			_WM_ADD = ( WM_USER + 1 ),
			_WM_REMOVE
		};

		static const char* m_className;
		static const int m_itemHeight;
		static const int m_defHeight;
		static const int m_defWidth;
		static const int m_padding;
		static CSimpleArray<GlobalIndicatorItem*> m_items;
		static HANDLE m_hItemsMutex;
		HANDLE m_hInstanceMutex;
		BOOL m_isOwner;
		HANDLE m_hWndThread;
		HANDLE m_hDone;//event
		HANDLE m_hCreated;

	private:
		HWND findInstance();
		void createInstance();
		static LRESULT CALLBACK wndProc( HWND wnd, UINT msg, WPARAM wp, LPARAM lp );
		static DWORD WINAPI wndThread( CGlobalIndicator* gi );
		static void lock( HANDLE mutex );
		static void unlock( HANDLE mutex );

	public:
		GlobalIndicatorItem* start( const char* label );
		void stop( GlobalIndicatorItem* );

	public:
		CGlobalIndicator();
		~CGlobalIndicator();
};

enum FLG
{
	FLG_EXPIRATION_DATE_PASSED  = 1,
	FLG_INIT_CALLED				= 2,
	FLG_INIT_DONE				= 4,
	FLG_ERROR_SHOWN				= 8,
	FLG_EA						= 16,
	FLG_AUTH_OK					= 32,
	FLG_MUST_REMOVE_EXPERT		= 64,
	FLG_REAUTH					= 128
};

enum ExpirationReason
{
	EXPIRATION_REASON_PROJECT	= 1,
	EXPIRATION_REASON_PAYPAL	= 2,
	EXPIRATION_REASON_REVISION	= 3,
	EXPIRATION_REASON_USER		= 4
};



#pragma pack(1)
class Globals
{
#ifndef MLL
	private:
		struct
		{
			/*const*/ WORD	projectId;		
			/*const*/ WORD	revision;		
			DWORD			expirationDate;		//is updated if remote licensing xml received
			DWORD			origExpirationDate;
			/*const*/ DWORD		compilationDate;
			DWORD			lastDNSVerification;
			DWORD			lastLicenseVerificationTime;
			DWORD			initTime;
			struct
			{
				DWORD maxInstances;
				char email[ 128 ];
			}
			autolicense;
			BYTE flags;		//union of FLG's	
		}
		_protected;

		/*const*/ char* extId;				//0x00	1
		/*const*/ char* licId;				//0x04	2
		/*const*/ char* projectName;		//0x08	3

		/*const*/ char* servers;			//0x10	4
		/*const*/ char* fsauth;				//0x14	5
		/*const*/ char* dllName;			//0x18	6		//defined by vendor on project creation or on his page
		/*const*/ char* ex4Name;			//0x24	10

		BYTE	dllNameSize;				//0x20	9
		BYTE	ex4NameSize;				//0x28	11
		WORD	serversSize;				//0x14	6
		BYTE	projectNameSize;		

		/*const*/ BYTE	decryptKeySize;	
		/*const*/ BYTE *decryptKey;		
		/*const*/ BYTE* licenseKey;		

		BYTE* checksum;
		BYTE* salt1;
		BYTE* salt2;
		BYTE salt1Length;
		BYTE salt2Length;
		HANDLE hEx4Name;
		HANDLE hChecksum;

		char* vendorName;
		char* vendorEmail;
		char* productUrl;

	private:
		void fillChecksum();
		DWORD getPointerKey();
		void copyStr( char** dst, const char* str );

	public:
		DWORD initStart;						
		DWORD lastPingTime;
		MLError lastErr;

		HANDLE hThreadGuardian; //thread	
		HANDLE hThreadGuardianStop; //event
		HANDLE hThreadPing;//thread
		HANDLE hThreadPingStop;//event
#ifdef CPPDEBUG
		HANDLE hThreadDbgDlg;//thread
		HANDLE hThreadDbgDlgStop;//event
#endif

		BYTE expirationReason;					
		__int64 serverTimeShift;				

		struct
		{
			char build[ 32 ];
			char lang[ 32 ];
			char name[ 128 ];
			char company[ 128 ];
		} 
		mt4info;

		PingCfg ping;
	
	public:

		virtual void setEx4Name( const char* name );
		virtual void setVendorName( const char* name );
		virtual void setVendorEmail( const char* email );
		virtual void setProductUrl( const char* url );
		virtual const char* getVendorName(){ return( vendorName ? vendorName : "" ); }
		virtual const char* getVendorEmail(){ return( vendorEmail ? vendorEmail : "" ); }
		virtual const char* getProductUrl(){ return( productUrl ? productUrl : "" ); }

#endif
	public:

		virtual const char* useId();
#ifndef MLL
		virtual const char* useLicId();
		virtual const char* useProject();
		virtual const char* useServers();
		virtual const char* useEx4Name();
		virtual const char* useFSAuth();
		virtual const char* useDllName();
		virtual const BYTE* useLicenseKey();
#endif

		virtual void unuse( const char* usedVar );
#ifndef MLL
		virtual void setAutolicenseData( DWORD v, const char* email );
		virtual DWORD getAutolicenseMaxInstances(){ return( _protected.autolicense.maxInstances ); }
		virtual const char* getAutolicenseEmail(){ return( _protected.autolicense.email ); }

		virtual void setExpirationDate( DWORD ed );
		virtual DWORD getExpirationDate(){ return( _protected.expirationDate ); }

		virtual void setLastDNSVerification( DWORD t );
		virtual DWORD getLastDNSVerification(){ return( _protected.lastDNSVerification ); }

		virtual void setLastLicenseVerificationTime( DWORD t );
		virtual DWORD getLastLicenseVerificationTime(){ return( _protected.lastLicenseVerificationTime ); }

		virtual DWORD getCompilationDate(){ return( _protected.compilationDate ); }

		virtual DWORD getInitTime(){ return( _protected.initTime ); }

		virtual int getRevision(){ return( _protected.revision ); }
		virtual int getProjectId(){ return( _protected.projectId ); }

		virtual const BYTE* getDecryptKey(){ return( ( const BYTE* )( ( DWORD )decryptKey ^ getPointerKey() ) ); }
		virtual DWORD getDecryptKeyLength(){ return( decryptKeySize ); }

		virtual void setFlag( FLG flag, int val );
		virtual BOOL getFlag( FLG flag );
		virtual BYTE* getChecksum();
		virtual void calcChecksum( OUT BYTE** chk, DWORD* sz );
		virtual void lock(){ ECS( hChecksum ); }
		virtual void unlock(){ LCS( hChecksum ); }
#endif
		virtual void startThreads();
		virtual void waitAndStopThreads( int wt );
		
		virtual void reset();

	public:
		Globals(
#ifndef MLL
			int _projectId, int _revision, int _compilationDate, int _expirationDate,
					const char* _extId, const char* _licId,
					const BYTE* _decryptKey, int _decryptKeyLength,
					const char* _projectName, int _projectNamSize,
					const char* _dllName, int _dllNameSize,
					const BYTE* _licenseKey,
					const char* _fsauth,
					const char* _servers, int _serversSize
#endif
		);
		~Globals();

};
#pragma pack()

extern Globals *g_g;

extern FILE* g_f;
#define  DBG( fmt, ... )	fprintf(g_f, fmt ## "\n", ##__VA_ARGS__ );\
							fflush(g_f);


void initGlobals();
Globals* getGlobals();
void clearGlobals();


DWORD WINAPI threadGuardian( LPVOID );

extern CAtlArray<CChartData*> g_charts;
extern HANDLE g_hCharts;
extern HANDLE g_hLinks;
extern CGlobalIndicator *g_ind;
extern CAtlArray<LINK_HANDLE> g_links;
extern HWND g_hwndNavTree, g_hwndNav;
extern WNDPROC g_origProcNav;
extern CSimpleArray<TimerData*>	timers;
extern CMemoryManager* g_mm;
extern CAccounts* g_ac;
extern __int64 g_epochFt;
extern CRV *g_rv;

// -------------------------------------------
struct ModelessData
{
	DWORD idd;
	DLGPROC proc;
	LPVOID param;
};

enum ERROR_TYPE
{
	E_OK,
	E_TIMEOUT,
	E_CANCELLED,
	E_WITHINFO,
	E_INVALID_EMAIL,
	E_NOPASSWORD,
	E_PASSWORDS_MISMATCH,
	E_REGISTERED_ALREADY,
	E_INVALID_SERVER_RESPONSE,
	E_ACCOUNT_DISABLED,
	E_ACCOUNT_EXPIRED,
	E_ACCOUNT_NOT_CONFIRMED,
	E_ACCOUNT_NOT_FOUND,
	E_INVALID_AUTH,
	E_USAGE_LIMIT,
	E_INVALID_CODE,
	E_INVALID_CAPTCHA
};

class CAutolicense
{
	private:
		CBaseServerManager *m_mgr;
		CBaseServerManagerHandle *m_mgrh;
		int m_type;
		int m_rtype;
		char* m_resp;
		char authEmail[ 128 ];

	private:
		char* chooseServer( Globals* g );

	public:
		int auth( const char* email, const char* password );
		int reg( const char* email, const char* password );
		int confirm( const char* email, const char* code );
		int sendForgotPassword( const char* email, const char* password );


		int isProcessing();
		int isReady();
		ERROR_TYPE readResponse();
		void getResponseData( OUT DWORD* maxInstances, OUT DWORD* accType );
		void getResponseData( OUT DWORD* usedAccounts, OUT DWORD* usedCids, OUT DWORD* usedInstances,
								   OUT DWORD* maxAccounts, OUT DWORD* maxCids, OUT DWORD* maxInstances );
		void getResponseData( OUT DWORD *expiration );

	public:
		CAutolicense();
		~CAutolicense();

};

class CAccounts
{
	private:
		struct Account
		{
			DWORD number;
			BOOL real;
		};
		enum Mt4Ver
		{
			MT4_VER_UNDEFINED,
			MT4_VER_LESS_445,
			MT4_VER_445,
			MT4_VER_577
		};
		CSimpleArray<Account> m_accounts;
		DWORD m_lastScan;
		DWORD m_cur;
		Mt4Ver m_mt4Ver;
		HANDLE m_mutex;
	private:
		BOOL fillAccountsList();
		void scanMemory();
		DWORD searchPattern( LPVOID base, DWORD base_length, LPVOID search, DWORD search_length );
		BOOL isRealDemoOffset( BYTE* block, BYTE* base, DWORD offset, DWORD size, OUT DWORD* number );
		BOOL accountExists( DWORD account );
	public:
		const char* getNumber( BOOL forceUseCache = FALSE );
		DWORD getNumberI( BOOL forceUseCache = FALSE );

		BOOL isReal( BOOL forceUseCache = FALSE );
		void invalidateCache();
void scanMemoryBlock( void* block, DWORD size );
	public:
		CAccounts();
		~CAccounts();
};


#pragma pack(push, 1 )
struct Ping
{
	BYTE ver;
	WORD projectId;
	WORD encSize;
	DWORD encCRC32;
	struct
	{
		BYTE cid[ 16 ];
		DWORD account;
		char email[ 1 ];
	}
	enc;	//this data is encrypted with license key
};
#pragma pack(pop)


INT_PTR CALLBACK dlgRegister1( HWND dlg, UINT msg, WPARAM wp, LPARAM lp );
INT_PTR CALLBACK dlgRegister2( HWND dlg, UINT msg, WPARAM wp, LPARAM lp );
INT_PTR CALLBACK dlgLocalLogin( HWND dlg, UINT msg, WPARAM wp, LPARAM lp );
INT_PTR CALLBACK dlgRemoteLogin( HWND dlg, UINT msg, WPARAM wp, LPARAM lp );
INT_PTR CALLBACK dlgRemoteAutoUpdate( HWND dlg, UINT msg, WPARAM wp, LPARAM lp );
INT_PTR CALLBACK dlgRemoteAutoUpdateNotification( HWND dlg, UINT msg, WPARAM wp, LPARAM lp );
BOOL isValidEmail( const char* email );
BOOL isValidDlgPassword( HWND dlg, DWORD idePassword1, OPTIONAL DWORD idePassword2, OUT char* pass, DWORD nPass );
const char* GetComputerID2();
const BYTE* GetComputerID2Raw();
BOOL parseUrl( const char* url, OUT char* server = 0, OUT INTERNET_PORT* port = 0, OUT char* baseUri = 0, OUT char* scheme = 0 );

HWND getMt4Window();
int getMt4Build();
BOOL verifyBuild();
void ping( Globals* g, BOOL force = FALSE );
DWORD WINAPI threadPing( LPVOID );
void showError( ERROR_TYPE err, HWND dlg );

#ifndef UNICODE
#define openLink openLinkA
#else
BOOL openLink( const wchar_t* link );
#endif
BOOL openLinkA( const char* link );

time_t Now( const Globals*g = NULL );
void CALLBACK TimerProc( HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime );

struct CommonThreadProcParams
{
	const char* name;
	LPTHREAD_START_ROUTINE proc;
	LPVOID param;
};

HANDLE myCreateThread( LPTHREAD_START_ROUTINE proc, LPVOID param, const char* name, DWORD flags = 0, OUT LPDWORD ptid = 0 );
void myTerminateProcess( const char* dbg );
void delSelf();


BOOL isValidLocalLogin( const CLogin* ll );
DWORD systemTime2UnixTime( SYSTEMTIME* st );
void unixTime2SystemTime( __int64 ut, OUT SYSTEMTIME* st );
char* formatDate( DWORD unixtime );
BOOL isTimePassed( DWORD start, DWORD dur, DWORD now = 0 );
BOOL isThreadDone( HANDLE hThread, OUT DWORD* res, DWORD timeout );
HANDLE modelessMessageBox( const char* msg, const char* title, DWORD style );
HANDLE modelessDialog( DWORD idd, DLGPROC proc, LPVOID param = 0 );
void criticalError( const char* text );
void criticalError( const char* text, const char* title );
void criticalError( BYTE* fmt, DWORD fmtSize, const BYTE* key, DWORD keySize );
void criticalError( BYTE* titleFmt, DWORD titleFmtSize, const BYTE* titleKey, DWORD titleKeySize, BYTE* textFmt, DWORD textFmtSize, const BYTE* textKey, DWORD textKeySize );
void criticalError( ERROR_TYPE err );
void criticalError( ERROR_TYPE err, BYTE* fmt, DWORD fmtSize, const BYTE* key, DWORD keySize, BOOL justInfo );
int processLimitedTimer( HWND dlg, DWORD idc, IN OUT int* timeLeft, BYTE* fmt, DWORD fmtSize, BYTE* key, DWORD keySize );
DWORD WINAPI threadRemoteAuth( DWORD *initStart );
DWORD WINAPI threadRemoteAuth2( DWORD *initStart );//new auth version
int init( HWND chart, BOOL isInit, DWORD id );	// should be called from mql init/start
int deinit( HWND chart, DWORD reason );				// should be called from mql deinit
void deleteChartData( CChartData* cd );
void preventUnloading();
void allowUnloading();
void setError( HWND hChart, MLError err );
void tryHookControls();
void tryUnhookControls();
CChartData* findChartData( HWND hChart, OUT int* i = 0 );
void sendTick( HWND hChart );
BOOL detectEx4Name( HWND chart, OUT char* ex4Name );
INT_PTR CALLBACK dlgCidError( HWND dlg, UINT msg, WPARAM wp, LPARAM lp );
DWORD WINAPI threadCidErr( void* p );

#ifdef CODESIGN
BOOL verifyMySignature( HMODULE hm );
BOOL VerifyEmbeddedSignature( LPCWSTR pwszSourceFile );
#endif

void closeCharts(/* BOOL wait = FALSE */);
BOOL isProjectExpired();

#ifdef VMP_PROTECTION
void checkImageCRC();
void checkDebuggerPresent();
#endif

__int64 fetchServerTimeShift( HINTERNET hr );
ULONGLONG myGetTickCount64();
void getMD5( BYTE* input, DWORD len, OUT char* str );
void getMD5Raw( BYTE* input, DWORD len, OUT BYTE* out );
HANDLE createMT4GUIMutex( HWND hChart );
void setInfo( const TCHAR* ex4name, const TCHAR* build, const TCHAR* lang, const TCHAR* name, const TCHAR* company,
				int isConnected, int isOptimization, int isTesting, int account );
DWORD genMq4Id();
void dlgEnable( HWND dlg, BOOL enable, ... );

DWORD getInstancesCount( const char* email = 0 );
#ifdef CPPDEBUG
INT_PTR CALLBACK dlgDbg( HWND dlg, UINT msg, WPARAM wp, LPARAM lp );
DWORD WINAPI threadDbgDlg( LPVOID );
#endif
void centerDlgToScreen( HWND dlg, OUT RECT* pr = NULL );

__forceinline void verifyGlobalsChecksum()
{
	dbg( "verifyGlobalsChecksum" );

	BYTE* chk;
	DWORD sz;
	Globals * g = getGlobals();
	g->lock();

	g->calcChecksum( &chk, &sz );
	const BYTE* checksum = g->getChecksum();
#ifdef CPPDEBUG
	char *strhex;
	bin_to_strhex( chk, sz, &strhex );
	dbg( "calcChecksum output %s", strhex );
	bin_to_strhex( checksum, sz, &strhex );
	dbg( "getChecksum output %s", strhex );
	free( strhex );
#endif

	if( memcmp( chk, checksum, sz ) != 0 )
	{
		dbg("htgch");
		closeCharts();
		myTerminateProcess("flg");
	}
	g->unlock();

	free( chk );
}

__forceinline void verifyAuth()
{
	Globals *g = getGlobals();

	verifyGlobalsChecksum();

	//not authenticated for more than 2 minutes
	if( isTimePassed( g->getInitTime(), 2 * 60 * 1000 ) && 
		( !g->getFlag( FLG_AUTH_OK ) || !g->getFlag( FLG_INIT_CALLED ) || !g->getFlag( FLG_INIT_DONE ) 
				|| g->getFlag( FLG_MUST_REMOVE_EXPERT )		//only expert must be removed, without closing chart. If not removed within 2 minutes then force chart closing
		) 
	) 
	{
		dbg("noa");
		closeCharts();
		myTerminateProcess("noa");
	}
}


__forceinline void verifyThreadGuardian()
{
	Globals *g = getGlobals();
	DWORD wt = WaitForSingleObject( g->hThreadGuardian, 0 );
	if( wt == WAIT_OBJECT_0 || wt == WAIT_FAILED )
	{
		CloseHandle( g->hThreadGuardian );
		g->hThreadGuardian = myCreateThread( threadGuardian, 0, "tg" );
	}
	else if( wt != STATUS_TIMEOUT )
	{
dbg( "wt %d", wt );
		for( ;; )	//prevent suspending guardian thread from hacker dll
		{
dbg( "!!- prs" );
			int r = ( int )ResumeThread( g->hThreadGuardian );	//decrementing suspend count until thread is started
			if( r <= 1 )
			{
				break;
			}
		}
	}
}

BOOL inline my_isspace( TCHAR c )
{
	return( c == _T( ' ' ) || c == _T( '\t' ) || c == _T( '\r' ) || c == _T( '\n' ) );
}
int inline my_strlen( const TCHAR* string )
{
	if( string )
	{
		return( _tcslen( string ) );
	}
	return( 0 );
}
int inline my_strcmp( const TCHAR* str1, const TCHAR* str2 )
{
	if( str1 && str2 )
	{
		return( _tcscmp( str1, str2 ) );
	}
	return( -1 );
}

#ifndef MLL
// ====================
// isInternetError
// ====================
__forceinline BOOL isInternetError( MLError err  )
{
	return( err == ML_INVALID_SERVER ||
			err == ML_READ_FILE	||
			err == ML_SEND_REQUEST ||
			err == ML_OPEN_URI ||
			err == ML_CONNECT_SERVER ||
			err == ML_INTERNET_OPEN
		);
} // isInternetError


// ==============
// tryInit
// ==============
__forceinline BOOL tryInit( int isInit, Globals* g )
{
	if( !g->getFlag( FLG_INIT_CALLED ) )
	{
		//dbg( "try2" );
		return( FALSE );
	}

	/*DWORD ttl = 15 * 60 * 1000;//24 * 60 * 60 * 1000; 01.02.2017 update
	if( g_rv )
	{
dbg( "g_rvttl" );
		ttl = 15 * 60 * 1000; //for RV users ttl should be less to allow fetching vars updates frequently
	}*/
	DWORD ttl = 60 * 60 * 1000;	//1h always
//ttl = 30 * 1000;

	BOOL isTTLExpired = isTimePassed( g->getLastLicenseVerificationTime(), ttl );

	if( !isTTLExpired )
	{
		if( g->getFlag( FLG_INIT_DONE ) )
		{
			//dbg( "try1" );
			return( FALSE );
		}
	}
	//else we must reload lic

	ECS( g_hLic );	// avoid same dll load lic simultaneously from few charts

	//maybe another thread have loaded lic while was waiting for g_hLic?
	isTTLExpired = isTimePassed( g->getLastLicenseVerificationTime(), ttl );	//LastLicenseVerificationTime flag could be set by another thread which current one was waiting for. So refresh value
	if( !isTTLExpired && g->getFlag( FLG_INIT_DONE ) )	
	{
		LCS( g_hLic );
		return( FALSE );
	}

	dbg( "tri" );

#ifdef VMP_PROTECTION
	//checkImageCRC(); // disabled, cns vps error: restarted terminal did not came up
	//checkDebuggerPresent();// disabled, cns vps error: restarted terminal did not came up
#endif

	g->startThreads();
	g_mm->start();
	g_logger->start();

	const char* p = g->useProject();
#ifdef CPPDEBUG
	if( isInit )
	{
		dbg( "pr=%s", p );
	}
#endif

	strcpy_s( g_projectName, sizeof( g_projectName ), p );
	g->unuse( p );

#if defined( CODESIGN ) && !defined( _DEBUG )
	if( !verifyMySignature( g_hInstance ) )
	{
dbg( "wrong sig" );
		closeCharts();
		g->setFlag( FLG_AUTH_OK, 0 );
		g->setFlag( FLG_INIT_DONE, 1 );
		myTerminateProcess( "sig" );
		LCS( g_hLic );
		return( FALSE );
	}
#endif //CODESIGN
	if( isTTLExpired )
	{
		preventUnloading();
dbg( "llvt" );
#if defined( GEN_LT_REMOTE )
		DWORD authOk;
		g_ac->getNumber();	//caching account number else mt4 gets frozen if CheckMLValidity called from indicator's init() on GetWindowText() call

dbg( "stvt" );
		DWORD initStart = GetTickCount();
		HANDLE hThreadAuth = myCreateThread( ( LPTHREAD_START_ROUTINE )threadRemoteAuth2, &initStart, "tra" );

		isThreadDone( hThreadAuth, &authOk, INFINITE );//wait for thread done

		if( g->getFlag( FLG_AUTH_OK ) && g->getFlag( FLG_REAUTH ) && ( isInternetError( g->lastErr ) || g->lastErr == ML_UNKNOWN ) && !authOk )	//already authenticated but license re-validation failed due to internet error. 
		{
			//So we ignore internet error occured on re-validation. threadRemoteAuth2 also does not show any error in this case
			
			g->setLastLicenseVerificationTime( GetTickCount() - ttl + 5000 );	//will try next re-validation in 5sec
		}
		else
		{
			g->setFlag( FLG_AUTH_OK, authOk );
			g->setLastLicenseVerificationTime( GetTickCount() );
			g->setFlag( FLG_REAUTH, 1 );	//next auth call will be re-auth, need to remember this for correct error displaying
		}
		g->setFlag( FLG_INIT_DONE, 1 );
		

		CloseHandle( hThreadAuth );

		if( authOk )
		{
			verifyBuild();
		}
#endif
		allowUnloading();
	}
	else
	{
dbg( "cttl" );
		g->setFlag( FLG_AUTH_OK, 1 );
		g->setFlag( FLG_INIT_DONE, 1 );
	}
	dbg( "tIdn" );
	
	LCS( g_hLic );

	return( TRUE );
} // tryInit

// ================
// CheckMLValidity
// ================
__forceinline BOOL CheckMLValidity( int isInit = 0, int *outMustExpertRemoveOrChartIndicatorDelete = NULL );
__forceinline BOOL CheckMLValidity( int isInit, int *outMustExpertRemoveOrChartIndicatorDelete )
{
	BOOL res = FALSE;
	TRY
#ifdef CPPDEBUG
if( isInit )
{
	dbg( "CheckMLValidity" );
}
#endif

#if defined CPPDEBUG || defined HARDCODED_EXPIRATION
	static BOOL isDebugExpired = FALSE;
#endif

	static DWORD lastVerifications;
	if( isTimePassed( lastVerifications, 1000 * ( rand() % 3 + 3 ) ) )
	{
		lastVerifications = GetTickCount();

		verifyAuth();
		verifyThreadGuardian();
	}
	
	Globals* g = getGlobals();

	BOOL meInited = tryInit( isInit, g );//here FLG_INIT_DONE is set to TRUE

	if( meInited ) // tryInit loaded lic by this thread
	{
		if( !g->getFlag( FLG_EXPIRATION_DATE_PASSED ) && isProjectExpired() )
		{
dbg( "exp %d %d", g->getExpirationDate(), Now( g ) );

			g->setFlag( FLG_EXPIRATION_DATE_PASSED, 1 );

			if( !g->getFlag( FLG_ERROR_SHOWN ) )
			{
				/*%s rev%d has expired (%d)*/
				BYTE titleKey[4]={0xd9,0xe0,0xe7,0xb3};
				BYTE titleFmt[26]={0xa5,0x06,0xde,0x3b,0xce,0xf8,0x8c,0x06,0x27,0x5a,0xd5,0xbb,0x35,0x49,0xac,0xa8,0xe5,0xf1,0xdb,0x91,0x2f,0x79,0x97,0xa3,0xbf,0xe5};
				
				char *title = ( char* )mymalloc( 256 );
				DECR( titleFmt, titleKey );
				int titleLen = 1 + wsprintfA( title, ( char* )titleFmt, g_projectName, g->getRevision(), g->expirationReason );
				ENCR( titleFmt, titleKey );
				rc4.Encrypt( ( BYTE* )title, titleLen, titleKey, sizeof( titleKey ) );

				BOOL isEA = g->getFlag( FLG_EA );

				switch( g->expirationReason )
				{
					case EXPIRATION_REASON_USER:
					{
						/*You have a valid account for product %s but your account has expired on %s. Your EA/indicator has been removed from chart due expiration. You may try to contact your vendor to extend expiration date.*/
						BYTE textKey[4]={0x77,0x35,0x57,0x11};
						BYTE textFmt[200]={0x75,0x18,0xf4,0xfd,0x4b,0x3d,0x8f,0x13,0x4a,0x26,0xff,0x97,0x4a,0xa6,0x43,0x03,0xa6,0xa5,0xed,0x41,0x71,0xc8,0xef,0xd0,0x3c,0xe4,0x6e,0x60,0x0b,0x71,0x9d,0x21,0x4b,0xa8,0x78,0x4f,0x5b,0xce,0xb7,0xab,0x60,0x00,0x8a,0xf1,0x8e,0x33,0x34,0xda,0xb8,0xbd,0xa9,0xad,0x2f,0x67,0xd6,0x10,0xd0,0x62,0x0d,0xfb,0x44,0x61,0x9b,0x07,0xa2,0x55,0xab,0x25,0xdc,0x15,0xa0,0x2e,0x87,0xec,0x87,0x0b,0x09,0xd0,0x97,0xe9,0x09,0x66,0xfa,0xa6,0x7b,0x60,0xfd,0x55,0xb2,0xbf,0x3f,0x9a,0x73,0x5e,0xb5,0x4e,0x36,0x72,0x16,0x4a,0xe6,0x8a,0x0f,0x24,0xd7,0x22,0x77,0x45,0xff,0x63,0x56,0x07,0x42,0xcb,0x64,0xa1,0xd6,0xae,0x36,0x94,0x92,0x85,0x60,0xe8,0x82,0xca,0x76,0xd0,0x38,0xae,0xec,0x44,0x19,0xb3,0xc1,0x52,0x43,0xfd,0x66,0xfa,0x1e,0xa3,0xa1,0x93,0x41,0xf7,0xb2,0xd2,0xff,0x55,0x3f,0xa5,0xbb,0xda,0xa4,0x10,0x2f,0xfd,0x0f,0x36,0x04,0x0e,0x4a,0xe5,0xba,0x14,0xa9,0xb6,0x1f,0xa1,0xd5,0xc4,0x69,0x4e,0x35,0xba,0xd5,0x7f,0x69,0x82,0xe5,0x34,0x71,0x2c,0x8e,0xf5,0x25,0xb5,0x8d,0x2d,0x9c,0xeb,0x5e,0x1f,0x6c,0x29,0xe0,0x12,0x6b,0x8a};

						DECR( textFmt, textKey );

						char *text = ( char* )mymalloc( 512 );
						char* date = formatDate( g->getExpirationDate() );
						const char* projectName = g->useProject();
						int textLen = 1 + wsprintfA( text, ( const char* )textFmt, projectName, date );
						g->unuse( projectName );
						free( date );
					
						ENCR( textFmt, textKey );

						rc4.Encrypt( ( BYTE* )text, textLen, textKey, sizeof( textKey ) );
						criticalError( ( BYTE* )title, titleLen, titleKey, sizeof( titleKey ), ( BYTE* )text, textLen, textKey, sizeof( textKey ) );
						free( text );
						break;
					}

					case EXPIRATION_REASON_PROJECT:
					{
						/*You have a valid account for product %s but this product has expired. Your EA/indicator has been been removed from chart due expiration. Please contact your vendor to get it activated. Your vendor can give you more information.*/
						BYTE textKey[4]={0xed,0xb4,0x7d,0xf9};
						BYTE textFmt[228]={0x7a,0xc3,0x44,0x78,0x78,0x22,0xf7,0xf8,0x68,0xc2,0x23,0x00,0x0e,0xfb,0x11,0xa3,0xa6,0x74,0xc6,0xc1,0x92,0xfa,0xeb,0x12,0xfd,0x7a,0xb9,0x75,0x51,0x16,0x41,0x83,0xc7,0x29,0x52,0x8b,0x25,0x53,0xb9,0xba,0xb8,0xd9,0xc5,0x12,0x57,0x24,0xc8,0x0a,0xff,0xe4,0xd0,0xbc,0x91,0x37,0x07,0x4b,0x76,0x0f,0x21,0x56,0xe6,0x71,0x12,0x98,0xec,0xd2,0x3d,0x41,0xa6,0xce,0xac,0x10,0x1d,0xad,0x8c,0x1a,0xbd,0x49,0xe8,0x93,0x70,0x12,0xc8,0xe9,0x99,0x6c,0x2a,0x1d,0x1a,0xf7,0x3e,0x30,0xd8,0x8b,0x6b,0x81,0x9c,0xea,0x9a,0xfa,0x40,0x51,0x75,0x25,0x2a,0xe6,0x6f,0x73,0xc9,0x91,0xde,0x20,0x1f,0xe6,0x78,0x96,0xe0,0xf9,0xe8,0x77,0x77,0x42,0xd8,0xd1,0x3d,0x6b,0xf4,0xfc,0x51,0xee,0xd6,0x76,0x3a,0x36,0x37,0x7c,0xb7,0x6d,0x47,0x56,0x49,0xe6,0xd0,0x33,0x2b,0x98,0xd4,0x86,0x40,0x5d,0x46,0xd6,0x73,0xf0,0x8c,0x56,0x0f,0xf4,0x26,0xa9,0x2f,0xb7,0xc5,0x80,0x5b,0x21,0x06,0xdb,0xa1,0x74,0xd4,0xe2,0x64,0x1e,0xd3,0x10,0xae,0x9c,0x1c,0xd0,0x40,0x8c,0xd2,0x32,0x38,0x21,0x15,0xe8,0x77,0x23,0x45,0xd0,0x5c,0x3f,0xb5,0x9e,0x1f,0xb5,0xf0,0xb3,0xc9,0x2b,0xc8,0xb3,0x86,0x41,0xc8,0x7d,0x59,0x80,0xeb,0x03,0x29,0x31,0xd0,0xf0,0x34,0x90,0x63,0xc2,0x8e,0xf8,0xef,0x94,0x17,0x7d,0x44,0x90};

						criticalError( ( BYTE* )title, titleLen, titleKey, sizeof( titleKey ), textFmt, sizeof( textFmt ), textKey, sizeof( textKey ) );
						break;
					}

					case EXPIRATION_REASON_REVISION:
					{
						/*You have a valid account for product %s but this product has newer revision/version. Your EA/indicator has been removed from chart due expiration. You run the version %d, please contact your vendor to update you to newer revisions > %d. As soon you get new revision from vendor, you will be able to login to this product. Please also check your email, vendor might have sent you an update but you may have missed to install it.*/
						BYTE textKey[4]={0xb4,0xd9,0x2f,0xcf};
						BYTE textFmt[428]={0x37,0x34,0x16,0x20,0xca,0x7f,0xf5,0xf7,0xd5,0xb5,0x3b,0xcf,0x43,0x6a,0xa8,0x21,0x9d,0xf5,0xc4,0x5e,0xa6,0xa3,0x96,0x75,0x5f,0x36,0x5f,0xcd,0x90,0x5a,0x2a,0xa1,0xa6,0x98,0x13,0x1a,0x78,0x74,0x42,0x2c,0x40,0x1d,0x43,0xd9,0x03,0x51,0x59,0x66,0xfa,0x8b,0xab,0x4f,0xa7,0xfc,0xf9,0xdc,0x06,0x9a,0xf4,0x13,0x19,0x79,0xe2,0x00,0x96,0x32,0x1e,0xf0,0xda,0x81,0x70,0xda,0x85,0x15,0xfa,0x09,0x30,0x26,0xd8,0x33,0xfa,0xd2,0x41,0xd0,0xa3,0xdc,0x47,0x2c,0x14,0x43,0xc4,0x90,0x24,0x0c,0x97,0x19,0x3c,0xf6,0xab,0x1d,0x71,0x0a,0x14,0x4b,0x07,0x3c,0x6d,0xe5,0xaf,0x4c,0x2d,0x4f,0xa0,0xd5,0x07,0x93,0xf4,0xdf,0x52,0x0e,0x23,0xc9,0xa7,0x19,0xc7,0x5c,0x4f,0x35,0xf2,0xe4,0xdb,0x87,0xf0,0xf6,0x36,0xd2,0x45,0x67,0x27,0xba,0x4f,0x3e,0xef,0x40,0x43,0x68,0xfe,0x7f,0x55,0x50,0xdb,0x35,0x67,0x52,0xc5,0xf8,0x73,0x37,0x21,0x40,0x0e,0x62,0x4e,0x6d,0x66,0x9a,0x45,0x5f,0x80,0x8b,0x26,0x9b,0x75,0x99,0xe1,0x39,0xd2,0x68,0x65,0x01,0x6e,0x20,0x2e,0x06,0xa9,0x2c,0x83,0xfd,0x66,0x35,0x03,0x9f,0x55,0x05,0x6d,0x5d,0x06,0xce,0xde,0xcb,0xcd,0x9e,0x28,0xcb,0xb4,0x7c,0x87,0x11,0x69,0x63,0x48,0x39,0x79,0x4b,0x56,0xcc,0xef,0x0f,0x7c,0x13,0x75,0x9b,0x43,0x47,0xaa,0xd4,0x16,0x42,0x67,0x5a,0x05,0xe2,0x9f,0xb6,0x12,0x24,0x7a,0xe0,0x29,0x26,0x36,0x9f,0x94,0xaa,0x1d,0x01,0x8d,0x92,0x1d,0x39,0x34,0x9c,0x43,0x8b,0xf5,0x07,0x7e,0x6d,0x8c,0x0a,0x86,0xad,0xf0,0xfa,0xce,0xfa,0xad,0x0b,0xd1,0x6f,0x40,0xab,0x28,0x4d,0x3f,0xf4,0xd8,0x67,0x1f,0x8d,0x7d,0x23,0x62,0x34,0x6b,0x90,0xd6,0xa3,0x7d,0x00,0x93,0xa2,0xae,0x0b,0x8b,0x57,0x90,0x7e,0x51,0x4d,0x9a,0x3c,0xac,0xce,0xb2,0xf3,0x21,0xac,0x75,0x9f,0x09,0x81,0x15,0x9b,0x8b,0x0c,0x86,0xb3,0x2c,0xfd,0x09,0x80,0xf9,0x80,0x8d,0xd1,0xc1,0x74,0x14,0xae,0x41,0xc6,0xf5,0xe0,0xb1,0x94,0x92,0x39,0xc9,0x7e,0x81,0x10,0xd1,0x4d,0xf3,0x5e,0x0d,0x5a,0x0f,0x61,0x00,0xf9,0xb5,0xac,0x6d,0x25,0xdb,0x9d,0x7f,0xfb,0x6d,0x8d,0x95,0x51,0x20,0x87,0xd4,0x3d,0xf1,0x40,0x5d,0xeb,0xd4,0x70,0xb2,0xe4,0x00,0x64,0x91,0x94,0x94,0x52,0x9d,0x63,0x37,0x03,0xe8,0xb4,0x0c,0x94,0x6e,0xfa,0x15,0xa5,0xd6,0x75,0x7b,0x93,0x0e,0x9b,0x0b,0x4c,0xad,0xb8,0xb9,0x86,0x17,0x97,0x3e,0x90,0xae,0xd8,0xab,0x02,0xde,0xdb,0x65,0x67,0x0d,0xe0,0xc9,0x41,0x4e,0xc4,0x7e,0xba,0x8d,0x79};

						char* text = ( char* )mymalloc( 1024 );

						DECR( textFmt, textKey );

						const char* projectName = g->useProject();
						int textLen = 1 + wsprintfA( text, ( const char* )textFmt, projectName, g->getRevision(), g->getRevision() );
						g->unuse( projectName );
						
						ENCR( textFmt, textKey );

						rc4.Encrypt( ( BYTE* )text, textLen, textKey, sizeof( textKey ) );
						criticalError( ( BYTE* )title, titleLen, titleKey, sizeof( titleKey ), ( BYTE* )text, textLen, textKey, sizeof( textKey ) );
						free( text );
						
						break;
					}

					default:
					{
						/*Your product %s has expired. Your EA/indicator has been removed from chart due expiration. You may try to contact to extend expiration date.*/
						BYTE textKey[4]={0xa9,0xcb,0xd4,0x9b};
						BYTE textFmt[141]={0xfa,0xdc,0xde,0x57,0xfa,0x4f,0xfe,0x21,0x3f,0xb7,0x5c,0xec,0x5c,0xcf,0x33,0xf8,0xcb,0x35,0x84,0xf5,0xbf,0x26,0x1b,0x8d,0xeb,0xb1,0x1d,0x6a,0x74,0x09,0x7b,0x40,0x88,0x63,0x9f,0x21,0xa1,0xb8,0x41,0x34,0x0a,0xd0,0xd1,0x5e,0x9b,0x33,0xd1,0x48,0xa3,0x9f,0x5e,0xa1,0xa9,0xbc,0x0b,0x36,0x5f,0x20,0xcc,0x16,0x7c,0x22,0xbb,0xe4,0xe2,0xfe,0x35,0x9e,0x45,0xd5,0xcf,0x7d,0xc5,0xa7,0xf9,0xfb,0xe3,0x04,0xfe,0xc2,0xad,0xaa,0xb1,0x09,0x80,0x1b,0xfe,0x06,0x3e,0xc6,0x24,0x41,0x3a,0xa6,0x87,0x3b,0x6f,0x7f,0xd6,0x39,0xa7,0xb9,0x15,0xbe,0xa5,0x0c,0x80,0x76,0x4d,0x70,0x06,0x95,0xa9,0x6e,0x3b,0xff,0x4d,0x24,0x30,0xdd,0xc5,0x70,0xbd,0x77,0xfe,0xd4,0x71,0x19,0x76,0xeb,0xfc,0xed,0x37,0xfb,0x59,0xb0,0xa0,0x7d,0x71,0xeb,0xf6};

						criticalError( ( BYTE* )title, titleLen, titleKey, sizeof( titleKey ), textFmt, sizeof( textFmt ), textKey, sizeof( textKey ) );
					}
				}
				free( title );
				
				// registry cleanup if license expired
				// so next time auth dialog is shown
				CReg reg;
				char path[ MAX_PATH ];
				wsprintfA( path, "%s\\%d", REG_KEY_PATH, g->getProjectId() );
				reg.open( path );
				reg.del( "ll" );
				reg.del( "lp" );
				reg.del( "lr" );
			}
		}
	}
#ifdef HARDCODED_EXPIRATION
	if( !isDebugExpired && Now( g ) - g->getCompilationDate() > HARDCODED_EXPIRATION * 86400 )
	{
		isDebugExpired = TRUE;

		/*Dear user, this is a debug version, this version has stopped working due to expiration. Please contact owner of your project to get stable version. Error 0x283*/
		BYTE key[4]={0x50,0x9c,0x1c,0x62};
		BYTE str[160]={0x3e,0xca,0x6a,0xda,0x47,0xb9,0xde,0x06,0x75,0xfa,0x1f,0x48,0x23,0x1a,0x6d,0xc4,0xb4,0x2d,0x2a,0x98,0xb5,0x04,0x5a,0x9e,0x8f,0x3b,0xd6,0x8e,0xc6,0x2c,0x44,0x13,0xe1,0x62,0x2b,0xf1,0x6a,0xe6,0xe3,0xb0,0x5b,0x06,0x64,0xba,0x2c,0xeb,0xad,0xfa,0x3e,0x18,0xa9,0xb0,0x86,0x33,0x59,0x6a,0x9f,0x79,0x7a,0x46,0x2b,0xd7,0xde,0x33,0x1b,0x48,0xbc,0xba,0xf4,0xb1,0xe4,0x6c,0xd8,0x02,0xa7,0x4d,0xcd,0x48,0x30,0x9f,0xb6,0xc3,0x71,0xd2,0xd9,0x79,0x8b,0x4d,0x7e,0x51,0x26,0x2e,0xfc,0x6d,0x0d,0x74,0xee,0xca,0x21,0x11,0xbd,0xeb,0xf5,0x34,0x47,0x8f,0x4d,0x30,0xd1,0x20,0x8d,0x2a,0xbe,0x93,0x4c,0x63,0x1d,0xb7,0xc5,0x68,0x86,0x86,0xe3,0xed,0xc6,0xcc,0x97,0x44,0x6e,0xa0,0x51,0x0e,0x85,0x6f,0x5c,0x8d,0x35,0xd0,0x9a,0x89,0x9e,0xd2,0xda,0x4a,0x08,0xad,0x39,0x19,0x83,0x83,0x99,0xbb,0xc1,0x87,0x66,0x42,0xe8,0x22,0xc6,0x43};

		DECR( str, key );
		criticalError( ( const char* )str );
		ENCR( str, key );
	}
#	ifdef CPPDEBUG
	else //CPPDEBUG expiration is checked below
#	endif
#endif

#ifdef CPPDEBUG
	if( !isDebugExpired && Now( g ) - g->getCompilationDate() > 4 * 7 * 86400 )	//4 WEEKS
	{
dbg( "debug expired %d %d", Now( g ), g->getCompilationDate() );
		isDebugExpired = TRUE;

		//Dear user, this is a debug version, this version has stopped working due to expiration. Please contact owner of your project to get stable version. Error 0x284
		BYTE key[4] = {0xb4,0x07,0x30,0x7f};
		BYTE str[160]={0x1a,0xa2,0x49,0xb0,0xb4,0xbb,0x83,0x94,0x02,0xd7,0x88,0xea,0xdb,0xe0,0xe1,0x93,0x2f,0x2a,0x57,0x54,0x76,0x9d,0x7b,0x91,0x37,0x5a,0xb9,0xfe,0xa5,0x4e,0xd4,0xf7,0x97,0x87,0x05,0x34,0xa6,0x7b,0x13,0xf1,0x78,0xbf,0xc3,0x68,0xc9,0x82,0x6d,0x92,0x0a,0x35,0xdd,0x0b,0x0a,0x3a,0x52,0xa6,0xaf,0x12,0xdd,0x05,0x68,0xde,0x56,0x1c,0x6f,0x0b,0x1c,0x37,0x3a,0x9b,0x08,0x53,0xe5,0xea,0xcc,0x7b,0x88,0x4b,0x18,0xaf,0x5c,0xbf,0x82,0xaf,0xd3,0xa4,0xd0,0x0c,0x13,0xa1,0xaf,0xa9,0x36,0x41,0x79,0xa8,0xae,0x7d,0x64,0x80,0x78,0x3c,0x8f,0x64,0xc8,0xf0,0x9d,0x76,0x19,0x0e,0x36,0x58,0x52,0x16,0x0c,0x0d,0x16,0x8d,0xb2,0xc4,0x19,0x9f,0xd8,0xea,0x02,0xae,0xef,0x67,0xc7,0x47,0x3e,0x33,0x85,0x79,0x69,0x0a,0x2b,0xea,0x4b,0x3d,0x31,0xdf,0xb9,0xb0,0xda,0x97,0xef,0x62,0x76,0xdb,0x56,0xa7,0x78,0x1f,0x73,0x71,0x80,0xa9,0xbe,0x30};

		DECR( str, key );
		criticalError( ( const char* )str );
		ENCR( str, key );
	}
#endif

	HWND mt4 = getMt4Window();
/*#ifdef _DEBUG
mt4 = ( HWND )1;
#endif*/

	res = ( !g->getFlag( FLG_EXPIRATION_DATE_PASSED ) && ( g->getFlag( FLG_AUTH_OK ) || !g->getFlag( FLG_INIT_DONE ) ) && mt4 );

#if defined CPPDEBUG || defined HARDCODED_EXPIRATION
	if( isDebugExpired )
	{
		res = FALSE;
	}
#endif

	if( res )
	{

	}
	else
	{
#ifndef CPPDEBUG
dbg( "g_isExpirationDateExpired=%d, g_authOk=%d, g_isInitDone=%d, mt4=%08x, es=%d", g->getFlag( FLG_EXPIRATION_DATE_PASSED ), g->getFlag( FLG_AUTH_OK ), g->getFlag( FLG_INIT_DONE ), mt4, g->getFlag( FLG_ERROR_SHOWN ) );
#else
dbg( "ede=%d, auok=%d, idon=%d, mt4=%08x, de=%d, es=%d", 
	g->getFlag( FLG_EXPIRATION_DATE_PASSED ), g->getFlag( FLG_AUTH_OK ), g->getFlag( FLG_INIT_DONE ), mt4, isDebugExpired, g->getFlag( FLG_ERROR_SHOWN ) );
#endif

dbg("fmre");
		g->setFlag( FLG_MUST_REMOVE_EXPERT, 1 );

		if( outMustExpertRemoveOrChartIndicatorDelete )
		{
dbg("mer");
			*outMustExpertRemoveOrChartIndicatorDelete = 1;
		}
	}
dbg( "chmldn %d", res );

	CATCH
	return( res );
} // CheckMLValidity

#else
BOOL tryInit();
#endif//ifndef MLL

