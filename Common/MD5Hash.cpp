#include "stdafx.h"
#include <Wincrypt.h>
#include "MD5Hash.h"

#define BUFSIZE (16 * 1024)

namespace Util
{

	MD5Hash::MD5Hash(const PathT file)
	{
		Calculate(file);
	}

	MD5Hash::~MD5Hash()
	{
	}

	bool MD5Hash::Calculate(const PathT file)
	{
		HCRYPTPROV hProv = NULL;
		HCRYPTHASH hHash = NULL;
		BYTE rgbFile[BUFSIZE];
		DWORD cbRead = 0;
		BOOL bResult = FALSE;

		if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
		{
			if (CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
			{
				HANDLE hFile = CreateFile(file.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					while (bResult = ReadFile(hFile, rgbFile, BUFSIZE, &cbRead, NULL))
					{
						if (0 == cbRead)
							break;

						if (!CryptHashData(hHash, rgbFile, cbRead, 0))
						{
							bResult = FALSE;
							break;
						}
					}

					if (bResult)
					{
						DWORD cbHash = MD5LEN;
						bResult = CryptGetHashParam(hHash, HP_HASHVAL, m_data, &cbHash, 0);
					}
					CloseHandle(hFile);
				}
				CryptDestroyHash(hHash);
			}
			CryptReleaseContext(hProv, 0);
		}
		return bResult;
	}

}