#include "CXOR.h"

CXOR::CXOR(void)
{
}

CXOR::~CXOR(void)
{
}

// ==============
// Decrypt
// ==============
void CXOR::Decrypt( BYTE* data, DWORD dlen, const BYTE* key, DWORD klen )
{
	register DWORD i, j;
	for( i = j = 0; i < dlen; i ++ )
	{
		data[ i ] ^= key[ j ];
		if( ++ j == klen )
		{
			j = 0;
		}
	}
} // Decrypt
