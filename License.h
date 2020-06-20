#ifndef LICENSE_H
#define LICENSE_H


#include "stdafx.h"
#include "../ds/ds.h"
#include "../crypt/crypt.h"

#include "LicenseType.h"
#define MAX_TRIAL_TIME_SEC (25 * 60)

const char LICENSE_TYPE[LicenseType_Max][64] = {
	"invalid",
	"RTM Trial",
	"A Task Manager for Windows Embedded devices",
};

struct rsa_st;
typedef struct rsa_st RSA;

class LICENSE
{
public:
	LICENSE();

	bool SaveLicense(const STRING& license) const;
	STRING LoadLicense() const;
	bool GetAdapterMac(unsigned char mac[MAX_ADAPTER_ADDRESS_LENGTH]) const;
	void SetApplicationDirectory(const STRING &dir);
	bool IsLicenseValid() const;
	LicenseType GetLicenseType() const;
	int TrialSecondsElapsedSinceStart() const;
	bool TrialExpired() const;
	bool Validate(const STRING &license);
	STRING GetBuyerEmail() const;

private:
	LicenseType ValidateAndGetUserEmail(const STRING &license, STRING &version, STRING &timestamp, STRING &buyer_email) const;
	RSA *loadPrivateKey() const;
	int getRsaEncryptedSecret(const char* license, int len, RSA *privateKey) const;
	STRING xmlDataFromLicense(int secret, const char* xored, int xored_sz) const;
	LicenseType parseDataFromLicenseXmlString(const STRING &xml, STRING &version, STRING &timestamp, STRING &buyer_email) const;

	STRING m_app_dir;
	STRING m_buyer_email;
	int m_version;
	int m_license_ts;
	LicenseType m_license_type;
	bool m_license_valid;
};

#endif // LICENSE_H
