#pragma once
#include <windows.h>
class CXOR
{
public:
	CXOR(void);
	~CXOR(void);

public:
	void Decrypt( BYTE* data, DWORD dlen, const BYTE* key, DWORD klen );
	void Encrypt( BYTE* data, DWORD dlen, const BYTE* key, DWORD klen ){ return( Decrypt( data, dlen, key, klen ) ); }
};
