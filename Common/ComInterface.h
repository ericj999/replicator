#pragma once

#include <Objbase.h>
#include <string>
#include <stdexcept>

#include <PortableDeviceApi.h>
#include <PortableDevice.h>
#include <Propvarutil.h>

#include "ExceptionString.h"

std::string IIDToString(REFIID iid);

template<typename T>
class ComInterface
{
public:
	ComInterface() :
		m_interface{ nullptr }
	{
	}

	ComInterface(REFCLSID clsid) :
		m_interface{ nullptr }
	{
		if (FAILED(CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_interface))))
		{
			throw std::runtime_error{ std::string{ EXCEPSTR_CREATE_INSTANCE_FAILURE } + "|" + IIDToString(clsid) };
		}
	}

	ComInterface(REFCLSID clsid, REFIID iid) :
		m_interface{ nullptr }
	{
		if (FAILED(CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, iid, reinterpret_cast<void**>(&m_interface))))
		{
			throw std::runtime_error{ std::string{ EXCEPSTR_CREATE_INSTANCE_FAILURE } + "|" +IIDToString(clsid) };
		}
	}

	ComInterface(ComInterface& cif) :
		m_interface{ nullptr }
	{
		m_interface = cif.m_interface;
		if(m_interface) m_interface->AddRef();
	}

	ComInterface(ComInterface&& cif) :
		m_interface{ nullptr }
	{
		m_interface = cif.m_interface;
		cif.m_interface = nullptr;
	}

	ComInterface(T* ptr) :
		m_interface{ nullptr }
	{
		m_interface = ptr;
	}

	~ComInterface()
	{
		if (m_interface)
			m_interface->Release();
	}

	bool operator!() { return m_interface ? false : true; }
	bool isValid() { return m_interface ? true : false; }
	T* Get() const { return m_interface;  }
	T* operator ->() const { return m_interface; }
	// update the raw pointer, original interface will be released if any 
	T** operator &() { if (m_interface) m_interface->Release(); m_interface = nullptr; return &m_interface; }

	ComInterface& operator=(ComInterface& cif)
	{
		if (m_interface != cif.m_interface)
		{
			if (m_interface)
			{
				m_interface->Release();
				m_interface = nullptr;
			}
			m_interface = cif.m_interface;
			if (m_interface) m_interface->AddRef();
		}
		return *this;
	}

	ComInterface& operator=(ComInterface&& cif)
	{
		if (m_interface)
		{
			m_interface->Release();
			m_interface = nullptr;
		}
		m_interface = cif.m_interface;
		cif.m_interface = nullptr;
		return *this;
	}

protected:
	T* m_interface;
};

class ComPropVariant
{
public:
	ComPropVariant()
	{
		PropVariantInit(&m_prop);
	}
	~ComPropVariant()
	{
		PropVariantClear(&m_prop);
	}

	ComPropVariant(const std::wstring& str)
	{
		PropVariantInit(&m_prop);
		InitPropVariantFromString(str.c_str(), &m_prop);
	}

	ComPropVariant(const FILETIME* ft)
	{
		PropVariantInit(&m_prop);
		InitPropVariantFromFileTime(ft, &m_prop);
	}

	PROPVARIANT* Get() { return &m_prop; }
	VARTYPE type() { return m_prop.vt; }

	const std::wstring GetStringValue() const { return m_prop.pwszVal; }
	const FILETIME& GetFileTimeValue() const { return m_prop.filetime; }

protected:
	PROPVARIANT m_prop;

};
