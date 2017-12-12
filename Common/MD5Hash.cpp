#include "stdafx.h"
#include <Wincrypt.h>
#include "MD5Hash.h"

#define BUFSIZE (16 * 1024)

namespace Util
{

	MD5Hash::MD5Hash(const PathT& file)
	{
		Calculate(file);
	}

	MD5Hash::MD5Hash(ShellWrapper::ShellItem& shellItem)
	{
		Calculate(shellItem);
	}

	MD5Hash::~MD5Hash()
	{
	}

	bool MD5Hash::Calculate(const PathT& file)
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

	bool MD5Hash::Calculate(ShellWrapper::ShellItem& shellItem)
	{
		HCRYPTPROV hProv = NULL;
		HCRYPTHASH hHash = NULL;
		BYTE rgbFile[BUFSIZE];
		DWORD cbRead = 0;
		HRESULT hr = E_FAIL;

		if (CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
		{
			if (CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash))
			{
				ShellWrapper::BindCtx bindCtx{ STGM_READ | STGM_SHARE_DENY_NONE };
				ShellWrapper::Stream stream;

				if(SUCCEEDED(shellItem->BindToHandler(bindCtx.Get(), BHID_Stream, IID_IStream, (void**)&stream)))
				{
					while (SUCCEEDED(hr = stream->Read(rgbFile, BUFSIZE, &cbRead)))
					{
						if (cbRead == 0)
							break;

						if (!CryptHashData(hHash, rgbFile, cbRead, 0))
						{
							hr = E_FAIL;
							break;
						}
						if (hr == S_FALSE)
							break;
					}

					if (SUCCEEDED(hr))
					{
						DWORD cbHash = MD5LEN;
						if (!CryptGetHashParam(hHash, HP_HASHVAL, m_data, &cbHash, 0))
							hr = E_FAIL;
					}
				}
				CryptDestroyHash(hHash);
			}
			CryptReleaseContext(hProv, 0);
		}
		return SUCCEEDED(hr) ? true : false;
	}
}