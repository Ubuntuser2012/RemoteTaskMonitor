#include "License.h"

#include <openssl/ossl_typ.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include "../common/util.h"
extern "C" {
#include "../base64/base64.h"
}
#include "../XML/xml.h"

#include "resource.h"

LICENSE::LICENSE()
	: m_license_ts(0)
	, m_license_type(LicenseType_Invalid)
	, m_license_valid(false)
{
}

RSA *LICENSE::loadPrivateKey() const
{
	RSA * rsa = NULL;
	HRSRC hrsc;
	HGLOBAL hglob;
	const char* keyBuffer;
	int keySz;

	hrsc = FindResource(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_PRIVATE_KEY), L"KEY");
	if (!hrsc)
	{
		logg(TXT("FindResource() KEY failed %i\n"), GetLastError());
		goto Exit;
	}
	hglob = LoadResource(::AfxGetInstanceHandle(), hrsc);
	if (!hglob)
	{
		logg(TXT("LoadResource() KEY failed %i\n"), GetLastError());
		goto Exit;
	}
	keyBuffer = (const char*)LockResource(hglob);
	if (!keyBuffer)
	{
		logg(TXT("LockResource() KEY failed %i\n"), GetLastError());
		goto Exit;
	}
	keySz = SizeofResource(::AfxGetInstanceHandle(), hrsc);

	rsa = d2i_RSAPrivateKey(NULL, (const unsigned char**) &keyBuffer, keySz);

Exit:
	return rsa;
}

int LICENSE::getRsaEncryptedSecret(const char* secret, int secret_sz, RSA *privateKey) const
{
	if (!privateKey)
	{
		logg(L"d2i_RSAPrivateKey failed");
		return 0;
	}
	unsigned char rsa_decrypted_string[32];

	ERR_load_crypto_strings();
	int iret = RSA_private_decrypt(secret_sz,
		(const unsigned char*)secret,
		rsa_decrypted_string,
		privateKey,
		RSA_PKCS1_PADDING);
	if (-1 == iret)
	{
		unsigned long err = ERR_get_error();
		char* errs = ERR_error_string(err, NULL);
		if (!errs)
		{
			logg(L"RSA_private_decrypt failed with error: %i", err);
			return 0;
		}
		else
		{
			logg(L"RSA_private_decrypt failed with error: %i:%s", err, errs);
			return 0;
		}
	}
	else
	{
		return atoi((const char*)rsa_decrypted_string);
	}
	ERR_free_strings();
	RSA_free(privateKey);
}

STRING LICENSE::xmlDataFromLicense(int secret, const char* xored, int xored_sz) const
{
	STRING xml;
	for (int i = 0; i < xored_sz; i += 2)
	{
		char hexdigit[3];
		hexdigit[0] = xored[i];
		hexdigit[1] = xored[i+1];
		hexdigit[2] = 0;
		char* ptr = 0;
		unsigned long byte = strtoul(hexdigit, &ptr, 16);
		byte ^= secret;
		
		STRING s;
		s.sprintf(L"%c", (char)byte);
		xml.append(s);
	}

	return xml;
}

bool LICENSE::SaveLicense(const STRING& license) const
{
	STRING license_file;
	license_file.set(m_app_dir);
	license_file.append(L"\\license.key");

	FILE *fp = fopen(license_file.ansi(), "wb");
	if (!fp)
	{
		return false;
	}
	fwrite(license.ansi(), 1, license.size(), fp);
	fclose(fp);

	return true;
}

STRING LICENSE::LoadLicense() const
{
	STRING license_file;
	license_file.set(m_app_dir);
	license_file.append(L"\\license.key");

	FILE *fp = fopen(license_file.ansi(), "rb");
	if (!fp)
	{
		return false;
	}
	size_t sz = 10240;
	char* buffer = new char[sz];
	sz = fread(buffer, 1, sz, fp);
	fclose(fp);

	if (sz < 1)
	{
		return false;
	}

	STRING license;
	license.set(buffer, sz);
	delete[] buffer;

	return license;
}

LicenseType LICENSE::parseDataFromLicenseXmlString(const STRING &xml, 
	STRING &version, STRING &timestamp, STRING &buyer_email) const
{
	STRING licesne_verification_string;

	XML xml_parser;
	xml_parser.ParseBuffer(xml.ansi());
	xml_parser.m_root;

	if (!xml_parser.m_root)
	{
		return LicenseType_Invalid;
	}
	if (xml_parser.m_root->m_name != L"license")
	{
		return LicenseType_Invalid;
	}

	XML_ELEMENT* elem = xml_parser.m_root->m_child;
	if (!elem || elem->m_name != L"app")
	{
		return LicenseType_Invalid;
	}
	XML_ATTRIBUTE *attrib = elem->m_attribute;
	while (attrib)
	{
		if (attrib->m_name == L"version")
		{
			version = attrib->m_value;
		}
		else if (attrib->m_name == L"timestamp")
		{
			timestamp = attrib->m_value;
		}
		attrib = attrib->m_next;
	}
	elem = elem->m_child;
	if (!elem || elem->m_name != L"verification_string")
	{
		return LicenseType_Invalid;
	}
	licesne_verification_string = elem->m_data;

	elem = elem->m_sibling;
	if (!elem || elem->m_name != L"buyer_email")
	{
		return LicenseType_Invalid;
	}
	buyer_email = elem->m_data;

	if (licesne_verification_string == L"oosman\'s CPU performance utility")
	{
		return LicenseType_Master;
	}
	else if (licesne_verification_string == L"oosman\'s CPU performance utility trial")
	{
		return LicenseType_Trial;
	}
	return LicenseType_Invalid;
}

LicenseType LICENSE::ValidateAndGetUserEmail(const STRING& licenseB64,
	STRING &version, STRING &timestamp, STRING &buyer_email) const
{
	if (!licenseB64.size())
	{
		logg(L"there is no license");
		return LicenseType_Invalid;
	}

	unsigned long mlen = licenseB64.size();
	char* base64_decoded_string = (char*)base64_decode(
		(const unsigned char*)licenseB64.ansi(), &mlen);
	if (!base64_decoded_string)
	{
		return LicenseType_Invalid;
	}

	RSA *privateKey = loadPrivateKey();
	if (!privateKey)
	{
		return LicenseType_Invalid;
	}

	const int SECRET_SZ = 256;
	int secret = getRsaEncryptedSecret(base64_decoded_string, SECRET_SZ, privateKey);
	if (secret < 1)
	{
		free(base64_decoded_string);
		return LicenseType_Invalid;
	}

	const int XORED_XML_SZ = 512;
	STRING xml = xmlDataFromLicense(secret, &base64_decoded_string[SECRET_SZ], XORED_XML_SZ);
	free(base64_decoded_string);

	return parseDataFromLicenseXmlString(xml, version, timestamp, buyer_email);
}

void LICENSE::SetApplicationDirectory(const STRING &dir)
{
	m_app_dir = dir;
}
STRING LICENSE::GetBuyerEmail() const
{
	return m_buyer_email;
}
bool LICENSE::IsLicenseValid() const
{
	return m_license_valid;
}
LicenseType LICENSE::GetLicenseType() const
{ 
	return m_license_type;
}
int LICENSE::TrialSecondsElapsedSinceStart() const
{
	int timeSinceEpoch = (int)time(0);
	return (timeSinceEpoch - m_license_ts);

}
bool LICENSE::TrialExpired() const
{
	return (TrialSecondsElapsedSinceStart() > MAX_TRIAL_TIME_SEC);
}

bool LICENSE::GetAdapterMac(unsigned char mac[MAX_ADAPTER_ADDRESS_LENGTH]) const
{
	bool ret = false;
	IP_ADAPTER_ADDRESSES *adapter;
	const int WORKING_BUFFER_SIZE = 15000;
	char adapters[WORKING_BUFFER_SIZE];
	unsigned long sz = sizeof(adapters);
	unsigned long err = GetAdaptersAddresses(AF_INET, 0, 0, (IP_ADAPTER_ADDRESSES*)adapters, &sz);
	if (err != ERROR_SUCCESS)
	{
		//swprintf(msg, L"Failed to retrieve network adapter information [error: %i]", err);
		//MessageBox(0, msg, L"Error", MB_ICONEXCLAMATION);
	}
	else
	{
		adapter = (IP_ADAPTER_ADDRESSES*)adapters;

		while (adapter)
		{
			if (adapter->OperStatus == IfOperStatusUp)
			{
				memcpy(mac, adapter->PhysicalAddress, MAX_ADAPTER_ADDRESS_LENGTH);
				break;
			}
			adapter = adapter->Next;
		}

		ret = true;
	}
	return ret;
}

bool LICENSE::Validate(const STRING &license)
{
	STRING version;
	STRING timestamp;
	LicenseType type = ValidateAndGetUserEmail(license,
		version, timestamp, m_buyer_email);
	if (type > LicenseType_Invalid)
	{
		m_license_valid = true;
		m_license_ts = timestamp.atoi();
		SaveLicense(license);
	}
	else
	{
		m_license_valid = false;
	}
	m_version = version.atoi();
	m_license_type = type;
	return m_license_valid;
}