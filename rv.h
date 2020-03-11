#pragma once
#include "xmlparser.h"
#include "ml_api.h"

class CRV
{
	private:	
		CXMLNodeEnc* m_xml;
		MLError m_err;
		BYTE* m_xmlKey;
		DWORD m_keyLen;
	private:
		const CXMLNodeEnc* findVar( const TCHAR* name );
	public:
#ifdef MLL
		BOOL auth( const TCHAR* authkey );
#else
		BOOL loadXml( const CXMLNodeEnc* xml );
#endif
		BOOL exists( const TCHAR* name );
		double getDouble( const TCHAR* name );
		int		getInt( const TCHAR* name );
		const TCHAR* getString( const TCHAR* name );
		long getDate( const TCHAR* name );
		BOOL getBool( const TCHAR* name );
		MLError getError(){ return( m_err ); }
	public:
		CRV( const BYTE* xmlKey, DWORD keyLen );
		~CRV();
};
typedef CRV*	RVHANDLE;

