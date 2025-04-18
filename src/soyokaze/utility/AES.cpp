#include "pch.h"
#include "AES.h"
#include <wincrypt.h>
#pragma comment(lib, "Advapi32.lib")

namespace launcherapp { namespace utility { namespace aes {

constexpr size_t EXPECTED_SIZE_HASH = 32;
constexpr size_t KEY_SIZE = 16;

class CryptProvider
{
public:
	CryptProvider() {}
	~CryptProvider() {
    if (mProvider) {
        CryptReleaseContext(mProvider, 0);
    }
	}

	operator HCRYPTPROV() {
		return mProvider;
	}

	HCRYPTPROV* operator &() { 
		return &mProvider;
	}

	HCRYPTPROV mProvider{0};
};

class CryptHash
{
public:
	CryptHash()
	{}
	~CryptHash(){
		if (mHash) {
			CryptDestroyHash(mHash);
		}
	}

	operator HCRYPTHASH() {
		return mHash;
	}

	HCRYPTHASH* operator &() { 
		return &mHash;
	}

	HCRYPTHASH  mHash{0};
};


struct AES::PImpl
{
	bool Initialize(const std::vector<uint8_t>& keyData);

	CryptProvider& GetProvider()
	{
		CryptProvider& provider = mProvider;
		BOOL isOK = CryptAcquireContext(&provider, nullptr, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, 0);
		if (isOK == FALSE) {
			isOK = CryptAcquireContext(&provider, nullptr, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_NEWKEYSET);
		}
		return provider;
	}

	HCRYPTKEY mKey{0};
	CryptProvider mProvider;
	CryptHash mHash;
	std::vector<uint8_t> mAESKey;
};


bool AES::PImpl::Initialize(const std::vector<uint8_t>& keyData)
{
	BOOL isOK = TRUE ;

	struct KeyBLOB {
		BLOBHEADER hdr;
		DWORD cbKeySize;
		BYTE rgbKeyData[KEY_SIZE];
	};

	KeyBLOB kb = {};
	kb.hdr.bType    = PLAINTEXTKEYBLOB;
	kb.hdr.bVersion = CUR_BLOB_VERSION;
	kb.hdr.reserved = 0;
	kb.hdr.aiKeyAlg = CALG_AES_128;
	kb.cbKeySize    = KEY_SIZE;

	memcpy(kb.rgbKeyData, keyData.data(), KEY_SIZE);

	isOK = CryptImportKey(GetProvider(),(BYTE*)&kb, sizeof(kb), 0, 0, &mKey);
	if (isOK == FALSE) {
		return false;
	}       

	DWORD mode{CRYPT_MODE_CBC};
	if (CryptSetKeyParam(mKey, KP_MODE, (BYTE*)&mode, 0) == FALSE){
		return false;
	}

	BYTE iv[KEY_SIZE] = {};
	if (CryptSetKeyParam(mKey, KP_IV, iv, 0) == FALSE) {
		return false;
	}

	DWORD padding_mode = PKCS5_PADDING;
	if (CryptSetKeyParam(mKey, KP_PADDING, (BYTE*)&padding_mode, 0) == FALSE){
		return false;
	}
	return true;
}
 


AES::AES() : in(new PImpl)
{
}

AES::~AES()
{
	if (in->mKey) {
		CryptDestroyKey(in->mKey);
		in->mKey = 0;
	}
}
 
bool AES::SetPassphrase(const std::string& passPhrase)
{
	BOOL isOK = FALSE;

	CryptProvider& provider = in->mProvider;
	isOK = CryptAcquireContext(&provider, nullptr, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, 0);
	if (isOK == FALSE) {
		if (CryptAcquireContext(&provider, nullptr, MS_ENH_RSA_AES_PROV, PROV_RSA_AES, CRYPT_NEWKEYSET) == FALSE){
			return false;
		}
	}

	CryptHash hash;
	isOK = CryptCreateHash(provider, CALG_SHA_256, 0, 0, &hash);
	if (isOK == FALSE){
		return false;
	}

	isOK = CryptHashData(hash, (BYTE*)passPhrase.c_str(), (DWORD)passPhrase.size(), 0);
	if (isOK == false){
		return false;
	}

	DWORD hashSize = 0 ;
	isOK = CryptGetHashParam(hash, HP_HASHVAL, NULL, &hashSize, 0);
	if (isOK == FALSE) {
		return false;
	}

	if (hashSize != EXPECTED_SIZE_HASH) {
		return false;
	}

	unsigned char hashResult[EXPECTED_SIZE_HASH];
	isOK = CryptGetHashParam( hash, HP_HASHVAL, (BYTE*)hashResult, &hashSize, 0 );
	if (isOK == FALSE) {
		return false;
	}

	std::vector<uint8_t> aesKey(KEY_SIZE);
	for (size_t i= 0; i < KEY_SIZE; ++i) {
		aesKey[i] = hashResult[i*2] ;
	}

	in->mAESKey.swap(aesKey);

	return true;
}
 
bool AES::Encrypt(
		const std::vector<uint8_t>& plainData,
		std::vector<uint8_t>& cryptData
)
{
	if (in->mKey == 0) {
		if (in->Initialize(in->mAESKey) == false) {
			return false;
		}
	}

	size_t srcSize = plainData.size();
	size_t dstSize = ((srcSize+16)/16)*16;
	cryptData.resize(dstSize);
	memcpy(cryptData.data(), plainData.data(), srcSize);

	DWORD sizeData = (DWORD)srcSize;
	BOOL isOK = CryptEncrypt(in->mKey, 0, TRUE, 0, cryptData.data(), &sizeData, (DWORD)dstSize);
	return (isOK != FALSE);
}

bool AES::Decrypt(
		const std::vector<uint8_t>& cryptData,
		std::vector<uint8_t>& plainData
)
{
	if (in->mKey == 0) {
		if (in->Initialize(in->mAESKey) == false) {
			return false;
		}
	}

	size_t srcSize = cryptData.size();

	plainData.resize(srcSize);
	memcpy(plainData.data(), cryptData.data(), srcSize);

	DWORD sizeData = (DWORD)srcSize;
	BOOL isOK = CryptDecrypt(in->mKey, 0, TRUE, 0, plainData.data(), &sizeData);
	if (isOK) {
		plainData.resize(sizeData);
	}
	return (isOK != FALSE);
}
 
 
}}} // end of namespace launcherapp::utility::aes

