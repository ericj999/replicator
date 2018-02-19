#include "stdafx.h"
#include <atldef.h>
#include <Windows.Services.Store.h>
#include <wrl.h>
#include <StringT.h>
#include "Log.h"
#include "StoreFuncs.h"

#pragma comment(lib, "runtimeobject.lib")

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Services::Store;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

#define CHECK_RESULT(failed, msg) if(failed) { Log::logger.error(StringT(msg) + GetLastErrorStr(GetLastError())); return ret; }

bool StoreIsRegistered(HWND hwnd)
{
	bool ret = false;
	HANDLE hEvent = NULL;

	Log::logger.info(L"Enter checking license from store...");
	ATLASSERT(::IsWindow(hwnd));

	RoInitializeWrapper roinit(RO_INIT_MULTITHREADED);
	HRESULT hr = roinit;
	if (FAILED(hr))
	{
		Log::logger.error(StringT(L"RoInitialize failed. ") + GetLastErrorStr(GetLastError()));
	}

	ComPtr<IStoreContextStatics> storeContextStatics;
	hr = GetActivationFactory(HStringReference(L"Windows.Services.Store.StoreContext").Get(), &storeContextStatics);
	CHECK_RESULT(FAILED(hr), L"Failed to create store context. ");

	ComPtr<IStoreContext> storeContext;
	hr = storeContextStatics->GetDefault(&storeContext);
	CHECK_RESULT(FAILED(hr), L"Could not get the default store context. ");
	// desktop app requirements
	ComPtr<IInitializeWithWindow> initWindow;
	hr = storeContext->QueryInterface(IID_PPV_ARGS(&initWindow));
	CHECK_RESULT(FAILED(hr), L"Failed to create Window initialization interfce. ");
	hr = initWindow->Initialize(hwnd);
	CHECK_RESULT(FAILED(hr), L"Failed to initialize owner window. ");

	ComPtr<IAsyncOperation<StoreAppLicense*>> getLicenseOperation;
	hr = storeContext->GetAppLicenseAsync(&getLicenseOperation);
	CHECK_RESULT(FAILED(hr), L"Could not get the store app license object. ");

	ComPtr<IStoreAppLicense> appLicense;
	CEvent event;

	//Callback<Implements<RuntimeClassFlags<ClassicCom>, IAsyncOperationCompletedHandler<StoreAppLicense*>, FtmBase>>
	auto onCompletedCallback = Callback<IAsyncOperationCompletedHandler<StoreAppLicense*>>(
		[&appLicense, &event](IAsyncOperation<StoreAppLicense*>* operation, AsyncStatus status)
		{
			HRESULT hr = S_OK;
			Log::logger.info(L"Async check starts.");
			if (status != AsyncStatus::Completed)
			{
				ComPtr<IAsyncInfo> asyncInfo;
				hr = operation->QueryInterface(IID_PPV_ARGS(&asyncInfo));
				if (FAILED(hr))
				{
					Log::logger.error(StringT(L"Failed to create async info interface. ") + GetLastErrorStr(GetLastError()));
				}
				else
				{
					hr = asyncInfo->get_ErrorCode(&hr);
					if (FAILED(hr))
					{
						Log::logger.error(StringT(L"Failed to get async errorr code. ") + GetLastErrorStr(GetLastError()));
					}
				}
			}
			else
			{
				hr = operation->GetResults(&appLicense);
				if (FAILED(hr))
				{
					Log::logger.error(StringT(L"Failed to get async operation result. ") + GetLastErrorStr(GetLastError()));
				}
			}
			event.SetEvent();
			Log::logger.info(StringT(L"Async check ends. Result = ") + ToStringT(static_cast<long>(hr)));
			return hr;
		});

	hr = getLicenseOperation->put_Completed(onCompletedCallback.Get());
	CHECK_RESULT(FAILED(hr), L"Failed to set completion callback. ");
	// wait for the operation to complete
	WaitForSingleObject(event.m_hObject, INFINITE);
	Log::logger.info(L"License check completed.");
	if (appLicense)
	{
		boolean isActive = false;
		hr = appLicense->get_IsActive(&isActive);
		CHECK_RESULT(FAILED(hr), L"Could not get active status. ");

		if (isActive)
		{
			ret = true;
			Log::logger.info(L"License status is active.");
		}
	}
	Log::logger.info(StringT(L"Done license checking. Result = ") + ToStringT(static_cast<long>(hr)));
	return ret;
}

std::wstring GetLastErrorStr(DWORD error)
{
	std::wstring str;
	LPTSTR lpMsgBuf = nullptr;

	DWORD bufLen = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL);

	if (bufLen)
		str = lpMsgBuf;
	else
		str = ToStringT(error);

	LocalFree(lpMsgBuf);
	return str;
}
