#pragma once
#include <atlcoll.h>
#include "Base64_RC4.h"


class CXMLNode;
typedef CSimpleArray<CXMLNode*> CXMLNodes;
class CXMLNodeEnc;
typedef CSimpleArray<CXMLNodeEnc*> CXMLNodesEnc;

class CXMLNode
{
	protected:
		TCHAR*		name;
		TCHAR*		text;
#ifdef UNICODE
		char*		atext;
#endif
		CXMLNodes children;
		static const char* m_str;
		static int m_len;

		enum 
		{
			WAIT_OPEN_TAG_OPEN_BRAKET,
			WAIT_OPEN_TAG_CLOSE_BRAKET,
			WAIT_CLOSE_TAG_OPEN_BRAKET,
			WAIT_CLOSE_TAG_CLOSE_BRAKET
		};

	private:
		BOOL parseName( IN OUT char**p, OUT TCHAR**name, OUT BOOL *closed );
		BOOL parseText( IN OUT char**p, OUT TCHAR**text );
		template<class T> BOOL load( char**p );
		void unescape( char** str );
		//std::deque<int> unicode_to_utf8( int charcode );
		//int utf8_to_unicode( std::deque<int> &coded );


	public:
		template<class T> BOOL Load( const char* str );

		const CXMLNode* GetChild( const TCHAR *name ) const;
		CXMLNodes GetChildren( const TCHAR *name = NULL ) const;
		const TCHAR* GetChildText( const TCHAR* name ) const;
		const TCHAR* GetText() const{ return( text ); }
#ifdef UNICODE
		const CXMLNode* GetChildA( const char *name ) const;
		CXMLNodes GetChildrenA( const char* name = NULL ) const;
		const char* GetChildTextA( const char* name ) const;
		const char* GetTextA() const;
#endif
		//static int ucs2ToAscii( const wchar_t* in, int inlen = 0, char* out = 0 );
		static int utf8ToUcs2( const char* in, int inlen = 0, wchar_t* out = 0 );
		static int ucs2ToUtf8( const wchar_t* in, int inlen=0, char* out =0 );
		static int ucs2_to_utf8( int ucs2, unsigned char * utf8 );
	
		
		const TCHAR* GetName() const{ return( name ); }
		
		template<class T> void Close();
		BOOL RemoveChild( CXMLNode* child );

	public:
		CXMLNode();
		CXMLNode( const CXMLNode* xml );
		virtual ~CXMLNode();
};

class CXMLNodeEnc : public CXMLNode
{
	private:
		TCHAR* decName;
		TCHAR* decText;
#ifdef UNICODE
		char* decAText;
#endif
	private:
		char* enc( const char* text, const BYTE* key, DWORD keyLen ) const;
		char* dec( const char* text, const BYTE* key, DWORD keyLen ) const;
	public:
		BOOL Load( const char* str );
		
		const TCHAR* GetChildText( const TCHAR* name, const BYTE* key, DWORD keyLen ) const;
		const TCHAR* GetText( const BYTE* key, DWORD keyLen ) const;
		CXMLNodesEnc GetChildren( const TCHAR *name/* = NULL*/, const BYTE* key, DWORD keyLen ) const;
		const CXMLNodeEnc* GetChild( const TCHAR *name, const BYTE* key, DWORD keyLen ) const;
#ifdef UNICODE
		const char* GetChildTextA( const char* name, const BYTE* key, DWORD keyLen ) const;
		const char* GetTextA( const BYTE* key, DWORD keyLen ) const;
		CXMLNodesEnc GetChildrenA( const char *name/* = NULL*/, const BYTE* key, DWORD keyLen ) const;
		const CXMLNodeEnc* GetChildA( const char* name, const BYTE* key, DWORD keyLen ) const;
#endif
	public:
		CXMLNodeEnc();
		CXMLNodeEnc( const CXMLNodeEnc* xml );
		~CXMLNodeEnc();
};