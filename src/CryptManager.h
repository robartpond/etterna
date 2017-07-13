#ifndef CryptManager_H
#define CryptManager_H

#include <memory>

class RageFileBasic;
struct lua_State;

const RString SIGNATURE_APPEND = ".sig";

class CryptManager
{
public:
	CryptManager();
	~CryptManager();

	static void GenerateGlobalKeys();
	static void GenerateRSAKey( unsigned int keyLength, RString &sPrivKey, RString &sPubKey );
	static void GenerateRSAKeyToFile( unsigned int keyLength, const RString &privFilename, const RString &pubFilename );
	static void SignFileToFile( const RString &sPath, RString sSignatureFile = "" );
	static bool Sign( const RString &sPath, RString &sSignatureOut, const RString &sPrivateKey );
	static bool VerifyFileWithFile( const RString &sPath, const RString &sSignatureFile = "" );
	static bool VerifyFileWithFile( const RString &sPath, RString sSignatureFile, const RString &sPublicKeyFile );
	static bool Verify( RageFileBasic &file, const RString &sSignature, const RString &sPublicKey );

	static void GetRandomBytes( void *pData, int iBytes );
	static RString GenerateRandomUUID();

	static RString GetMD5ForFile( const RString &fn );		// in binary
	static RString GetMD5ForString( const RString &sData );	// in binary
	static RString GetSHA1ForString( const RString &sData );	// in binary
	static RString GetSHA1ForFile( const RString &fn );		// in binary

	static RString GetPublicKeyFileName();

	// Lua
	void PushSelf( lua_State *L );
};

extern unique_ptr<CryptManager>	CRYPTMAN;	// global and accessible from anywhere in our program

#endif

/*
 * (c) 2004 Chris Danford
 * All rights reserved.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, and/or sell copies of the Software, and to permit persons to
 * whom the Software is furnished to do so, provided that the above
 * copyright notice(s) and this permission notice appear in all copies of
 * the Software and that both the above copyright notice(s) and this
 * permission notice appear in supporting documentation.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
 * THIRD PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS
 * INCLUDED IN THIS NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT
 * OR CONSEQUENTIAL DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
