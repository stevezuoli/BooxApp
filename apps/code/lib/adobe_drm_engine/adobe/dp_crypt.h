/*
 *  ADOBE CONFIDENTIAL
 *
 *  Copyright 2006-2009, Adobe Systems Incorporated
 *
 *  All Rights Reserved.
 *
 *  NOTICE:  All information contained herein is, and remains the property of
 *  Adobe Systems Incorporated and its suppliers, if any.  The intellectual and
 *  technical concepts contained herein are proprietary to Adobe Systems
 *  Incorporated and its suppliers and may be covered by U.S. and Foreign
 *  Patents, patents in process, and are protected by trade secret or copyright
 *  law.  Dissemination of this information or reproduction of this material is
 *  strictly forbidden unless prior written permission is obtained from Adobe
 *  Systems Incorporated.
 */
/**
 *  \file dp_crypt.h
 *
 *  Reader Mobile SDK public interface -- crypto services
 */
#ifndef _DP_CRYPT_H
#define _DP_CRYPT_H

#include "dp_core.h"

/**
 *  Reader Mobile SDK crypto services
 */
namespace dpcrypt
{

class Key;
class Certificate;
class Identity;
class Hash;
class Cryptor;
class StreamCryptor;

/**
 *  Hash algorithm identifier
 */
enum HashID
{
	HID_SHA1 /**< SHA-1 (The Secure Hash Algorithm), as defined in Secure Hash Standard, NIST FIPS 180-1 */
};

/**
 *  Encryption algorithm identifier
 */
enum EncAlgID
{
	EID_AES128_CBCPAD,	/**< AES-128 (Advanced Encryption Standard) with CBC padding (symmetric algorithm), as specified by NIST)*/
	EID_RSA				/**< RSA (public-private key algorithm), as defined by PKCS#1 */
};

/**
 *  Type of key: symmetric, public, private.
 */
enum KeyType
{
	KT_SYMMETRIC,		/**< symmetric key */
	KT_ASYM_PUBLIC,		/**< public key (asymmetric) */
	KT_ASYM_PRIVATE		/**< private key (asymmetric) */
};

/**
 *  Certificate role determines how its validity is verified and what it can be used for.
 *  Each role has a unique OID associated with it. 
 */
enum Role
{
	ROLE_AUTH,			/**< authentication server certificate, OID 1.2.840.113583.2.5 */
	ROLE_ACTIVATION,	/**< activation server certificate, OID 1.2.840.113583.2.4 */
	ROLE_LICENSE		/**< license server certificate, OID 1.2.840.113583.2.2 */
};

struct KeyPair
{
	dp::ref<Key> publicKey;
	dp::ref<Key> privateKey;
};

/**
 *  Interface to provide cryptographic services to Reader Mobile SDK.
 */
class CryptProvider : public dp::Unknown
{
protected:
	virtual ~CryptProvider() {}

public:

	virtual int getInterfaceID() { return IID_CryptProvider; }
	/**
	 *  Generates given number of cryptographically secure data
	 */
	virtual dp::Data getRandomBytes( size_t numBytes ) = 0;

	/**
	 *  Creates a hash of a specified type.
	 */
	virtual dp::ref<Hash> createHash( int id ) = 0;

	/**
	 *  Creates a key object from its serialization. Symmetric keys are just sequence of bytes
	 *  that represent the key. Private keys are serialized according to PKCS #8.
	 *  Public keys are encoded according to the ASN.1 type SubjectPublicKeyInfo
	 *  defined in X509 standard.
	 */
	virtual dp::ref<Key> createKey( int id, int kt, const dp::Data& keyBytes ) = 0;

	/**
	 *  Generates a random key suitable for specified symmetric algorithm. Returns NULL if
	 *  key cannot be generated
	 */
	virtual dp::ref<Key> generateKey( int id ) = 0;

	/**
	 *  Generates a random public/private key pair
	 */
	virtual bool generateKeyPair( int id, KeyPair * keys ) = 0;

	/**
	 *  Creates a cryptor suitable for specified symmetric or asymetric algorithm.
	 */
	virtual dp::ref<Cryptor> createCryptor( int id ) = 0;

	/**
	 *  Creates a cryptor suitable for specified symmetric algorithm.
	 */
	virtual dp::ref<StreamCryptor> createStreamCryptor( int id ) = 0;

	/**
	 *  Create a certificate from a serialized form
	 */
	virtual dp::ref<Certificate> createCertificate( const dp::Data& certBytes ) = 0;

	/**
	 *  Create an identity from PKCS #12 serialized form
	 */
	virtual dp::ref<Identity> createIdentity( const dp::Data& p12Bytes, const dp::String& password ) = 0;

	/**
	 *  Serialize PKCS12 with a new password
	 */
	virtual dp::Data changePKCS12Password( const dp::Data& p12Bytes, const dp::String& oldPassword , const dp::String& newPassword ) = 0;

	/**
	 *  Register a provider for Reader Mobile SDK to use
	 */
	static void setProvider( CryptProvider * provider );
	/**
	 *  Get a provider for Reader Mobile SDK to use
	 */
	static CryptProvider * getProvider();
};

/**
 *  Interface to represent an implementation of a hash calculation algorithm, such as SHA-1.
 */
class Hash : public dp::RefCounted
{
protected:
	virtual ~Hash() {}

public:
	virtual int getInterfaceID() { return IID_Hash; }
	/**
	 *  Update internal data for hash calculation with additional message data
	 */
	virtual void update( const dp::Data& data ) = 0;
	/**
	 *  Get hash value. This object should not be used after this call.
	 */
	virtual dp::Data finalize() = 0;
};
	
/**
 *  Interface to represent a cryptographic key.
 */
class Key : public dp::RefCounted
{
protected:
	virtual ~Key() {}

public:

	virtual int getInterfaceID() { return IID_Key; }
	/**
	 *  Get key size in bits. AES-128 key is 128 bit (or 16 bytes). RSA keys used are typically 1024
	 *  or 2048 bits.
	 */
	virtual unsigned int getKeyBitSize() = 0;
	/**
	 *  Get the type of the key.
	 */
	virtual int getKeyType() = 0;
	/**
	 *  Convert this key to a byte arry. Symmetric keys are just sequence of bytes
	 *  that represent the key. Private keys are serialized according to PKCS #8.
	 *  Public keys are encoded according to the ASN.1 type SubjectPublicKeyInfo
	 *  defined in X509 standard.
	 */
	virtual dp::Data serialize() = 0;		
};	

/**
 *  Interface to represent an implementation of encryption/decryption algorithm suitable for small data.
 */
class Cryptor : public dp::RefCounted
{
protected:
	virtual ~Cryptor() {}

public:
	virtual int getInterfaceID() { return IID_Cryptor; }
	/**
	 *  Encrypt plaintext with asymmetric key (public or private)
	 */
	virtual dp::Data encrypt( const dp::ref<Key>& key, const dp::Data& plainText ) = 0;
	/**
	 *  Encrypt plaintext with symmetric key and initialization vector (IV). If prependIV is true,
	 *  IV is prepended to the resulting ciphertext.
	 */
	virtual dp::Data encrypt( const dp::ref<Key>& key, const dp::Data& iv, const dp::Data& plainText, bool prependIV ) = 0;
	/**
	 *  Decrypt ciphertext with asymmetric key (public or private)
	 */
	virtual dp::Data decrypt( const dp::ref<Key>& key, const dp::Data& cipherText ) = 0;
	/**
	 *  Decrypt ciphertext with symmetric key and initialization vector (IV).
	 */
	virtual dp::Data decrypt( const dp::ref<Key>& key, const dp::Data& iv, const dp::Data& cipherText ) = 0;
};

/**
 *  Interface to represent an implementation of decryption algorithm suitable for streaming.
 */
class StreamCryptor : public dp::RefCounted
{
protected:
	virtual ~StreamCryptor() {}
		
public:

	virtual int getInterfaceID() { return IID_StreamCryptor; }
	/**
	 *  Initialize decryption algorithm using a symmetric key and an initialization vector (IV).
	 */
	virtual bool initDecrypt( const dp::ref<Key>& key, const dp::Data& iv ) = 0;
	/**
	 *  Decrypt the next portion of ciphertext stream (pointed by inBuf of length inSize). Plaintext
	 *  is written into a buffer outBuf of length outSize. Actual number of ciphertext bytes processed
	 *  is written into a variable pointed to by consumed. Number of plaintext bytes produced is returned.
	 */
	virtual size_t decrypt( const unsigned char * inBuf, size_t inSize, size_t * consumed,
				 unsigned char * outBuf, size_t outSize ) = 0;
	/**
	 *  Indicate cipher stream end of file, finish decryption writing result in outBuf and return the
	 *  number of final plaintext bytes. outSize is the size of outBuf and should hold at least one
	 *  block of data (16 bytes for AES-128).
	 */
	virtual size_t finalize(unsigned char * outBuf, size_t outSize ) = 0;
};

/**
 *  Interface to represent X509 certificate.
 */
class Certificate : public dp::RefCounted
{
protected:
	virtual ~Certificate() {}

public:
	virtual int getInterfaceID() { return IID_Certificate; }
	/**
	 *  Get certificate principal common name (CN).
	 */
	virtual dp::String getCommonName() = 0;
	/**
	 *  Get certificate public key.
	 */
	virtual dp::ref<Key> getPublicKey() = 0;
	/**
	 *  Serialize certificate in ASN.1 form defined by X509 standard.
	 */
	virtual dp::Data serialize() = 0;
	/**
	 *  Check if the certificate is valid for a given role.
	 */
	virtual bool isValidForRole( int role ) = 0;
};

/**
 *  Interface to represent PKCS12 file (packaged password-protected private key and certificate(s)).
 */
class Identity : public dp::RefCounted
{
protected:
	virtual ~Identity() {}

public:

	virtual int getInterfaceID() { return IID_Identity; }
	/**
	 *  get the End Entity Certificate
	 */
	virtual dp::ref<Certificate> getEECert() = 0;
	/**
	 *  get the private key, should only be one
	 */
	virtual dp::ref<Key> getPrivateKey() = 0;
};


}

#endif // _DP_CRYPT_H
