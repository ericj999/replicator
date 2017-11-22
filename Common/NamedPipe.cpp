#include "stdafx.h"
#include <assert.h>
#include <array>

#include "NamedPipe.h"

namespace IPC
{
	// Named Pipe Base Class
	Pipe::Pipe(const StringT& pipeName, PipeDirection direction, int bufferSize) :
		m_pipeReadEvent{ NULL }, m_pipeWriteEvent{ NULL }, m_pipeHandle{ INVALID_HANDLE_VALUE }, m_pipeName(pipeName), m_direction{ direction },
		m_bufferSize{ bufferSize }
	{
		Ctor();
	}

	Pipe::Pipe() :
		m_pipeReadEvent{ NULL }, m_pipeWriteEvent{ NULL }, m_pipeHandle{ INVALID_HANDLE_VALUE }, m_direction{ none }, m_stopHandle{ NULL }
	{
		Ctor();
	}

	Pipe::~Pipe()
	{
		if (m_pipeHandle != INVALID_HANDLE_VALUE)
			CloseHandle(m_pipeHandle);

		if (m_pipeReadEvent) CloseHandle(m_pipeReadEvent);
		if (m_pipeWriteEvent) CloseHandle(m_pipeWriteEvent);
		if (m_stopHandle) CloseHandle(m_stopHandle);
	}

	void Pipe::Ctor()
	{
		m_stopHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!m_stopHandle)
			throw Exception(GetLastError());
	}

	void Pipe::Stop()
	{
		if (m_stopHandle)
		{
			if (!SetEvent(m_stopHandle))
				throw Exception(GetLastError());
		}
	}	// stop the current operation, the pipe should not be used afterward.


	PipeData Pipe::Read(DWORD waitTime /* = PIPE_MESSAGE_TIMEOUT*/)
	{
		assert(CanRead());

		PipeData data(m_bufferSize);
		BOOL rc = TRUE;

		OVERLAPPED overlapped;
		memset(&overlapped, 0, sizeof(OVERLAPPED));
		overlapped.hEvent = m_pipeReadEvent;
		ResetEvent(m_pipeReadEvent);

		DWORD dwRead = 0;
		rc = ReadFile(m_pipeHandle, data.data(), static_cast<DWORD>(data.capacity()), &dwRead, &overlapped);
		if (!rc)
		{
			DWORD dwErr = GetLastError();
			if (dwErr == ERROR_IO_PENDING)
			{
				std::array<HANDLE, 2> waitHandles = { m_pipeReadEvent, m_stopHandle };

				DWORD dwWait = WaitForMultipleObjects(static_cast<DWORD>(waitHandles.size()), waitHandles.data(), FALSE, waitTime);
				if (dwWait == WAIT_OBJECT_0)
				{
					if (GetOverlappedResult(m_pipeHandle, &overlapped, &dwRead, FALSE))
						data.resize(dwRead);
					else
						throw Exception(GetLastError());
				}
				else if (dwWait == (WAIT_OBJECT_0 + 1))
					throw Exception("Pipe read was aborted!");
				else if (dwWait == WAIT_TIMEOUT)
					throw Exception("Pipe read timeout!");
				else
					throw Exception(GetLastError());

				ResetEvent(m_pipeReadEvent);
			}
			else
				throw Exception(dwErr);
		}
		else
		{
			data.resize(dwRead);
		}

		return data;
	}

	void Pipe::Write(const PipeData& data, DWORD waitTime /* = PIPE_MESSAGE_TIMEOUT*/)
	{
		assert(CanWrite());

		OVERLAPPED overlapped;
		DWORD written = 0;

		memset(&overlapped, 0, sizeof(OVERLAPPED));
		overlapped.hEvent = m_pipeWriteEvent;
		ResetEvent(m_pipeWriteEvent);

		BOOL bRC = TRUE;
		bRC = WriteFile(m_pipeHandle, data.data(), static_cast<DWORD>(data.size()), &written, &overlapped);
		if(!bRC)
		{
			DWORD dwErr = GetLastError();
			if (dwErr == ERROR_IO_PENDING)
			{
				std::array<HANDLE, 2> waitHandles = { m_pipeWriteEvent, m_stopHandle };

				DWORD dwWait = WaitForMultipleObjects(static_cast<DWORD>(waitHandles.size()), waitHandles.data(), FALSE, waitTime);
				if (dwWait == WAIT_OBJECT_0)
					bRC = GetOverlappedResult(m_pipeHandle, &overlapped, &written, FALSE);
				else if (dwWait == (WAIT_OBJECT_0 + 1))
					throw Exception("Pipe write was aborted!");
				else if (dwWait == WAIT_TIMEOUT)
					throw Exception("Pipe write timeout!");
				else
					throw Exception(GetLastError());

				ResetEvent(m_pipeWriteEvent);
			}
			else
				throw Exception(dwErr);
		}
		if (data.size() != written)
		{
			std::string error = "Only " + std::to_string(written) + " of " + std::to_string(data.size()) + " bytes were sent.";
			throw Exception(error.c_str());
		}
	}

	// Named Pipe Server
	PipeServer::PipeServer(const StringT& pipeName, int numInstance, PipeDirection direction /*= in*/, PipeServerCallback callback, int defaultTimeout /*= DEFAULT_TIMEOUT_VALUE*/) :
		Pipe(pipeName, direction), m_connectHandle{ NULL }, m_callback{ callback }, m_done{ false }, m_connected{ false }
	{
		SECURITY_ATTRIBUTES sa;
		SECURITY_DESCRIPTOR sd;

		memset(&sa, 0, sizeof(SECURITY_ATTRIBUTES));
		memset(&sd, 0, sizeof(SECURITY_DESCRIPTOR));
		sa.nLength = sizeof(SECURITY_ATTRIBUTES);

		if (InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION))
		{
			if (SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE))
				sa.lpSecurityDescriptor = &sd;
		}

		m_connectHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!m_connectHandle)
			throw Exception(GetLastError());

		DWORD openMode = FILE_FLAG_OVERLAPPED;
		if (m_direction == in)
		{
			openMode |= PIPE_ACCESS_INBOUND;
			m_pipeReadEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		}
		else if (m_direction == out)
		{
			openMode |= PIPE_ACCESS_OUTBOUND;
			m_pipeWriteEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		}
		else if (m_direction == both)
		{
			openMode |= PIPE_ACCESS_DUPLEX;
			m_pipeReadEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
			m_pipeWriteEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		}
		m_pipeHandle = CreateNamedPipe(m_pipeName.c_str(), openMode, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
				numInstance, m_bufferSize, m_bufferSize,  static_cast<DWORD>(defaultTimeout), &sa);

		if (m_pipeHandle == INVALID_HANDLE_VALUE)
			throw Exception(GetLastError());
	}

	PipeServer::~PipeServer()
	{
		if (m_thread.joinable())
			m_thread.join();

		if (m_connectHandle)
			CloseHandle(m_connectHandle);
	}

	void PipeServer::WaitForClientConnection()
	{
		OVERLAPPED overlapped;
		memset(&overlapped, 0, sizeof(OVERLAPPED));
		overlapped.hEvent = m_connectHandle;

		ResetEvent(m_connectHandle);

		DWORD dwErr = 0;
		if(!ConnectNamedPipe(m_pipeHandle, &overlapped))
		{
			dwErr = GetLastError();
			if (dwErr == ERROR_IO_PENDING)
			{
				std::array<HANDLE, 2> waitHandles = { m_connectHandle, m_stopHandle };

				DWORD dwWait = WAIT_TIMEOUT;
				while (dwWait == WAIT_TIMEOUT)
				{
					dwWait = WaitForMultipleObjects(static_cast<DWORD>(waitHandles.size()), waitHandles.data(), FALSE, PIPE_TIMEOUT_CHECK_INTVAL);
					if (dwWait == WAIT_OBJECT_0)
						break;
					if (dwWait == (WAIT_OBJECT_0 + 1))
						throw Exception("Pipe operation was aborted!");
					else if (dwWait != WAIT_TIMEOUT)
						throw Exception(GetLastError());
				}
			}
			else if (dwErr != ERROR_PIPE_CONNECTED)
				throw Exception(dwErr);
		}
		m_connected = true;
		if (CanRead())
			m_thread = std::thread(&PipeServer::ReadFunc, this);
	}

	void PipeServer::ReadFunc()
	{
		try
		{
			while (true)
			{
				PipeData pd = Read(INFINITE);
				if ((pd.size() > 0) && m_callback)
					m_callback(pd.data(), pd.size());
			}
		}
		catch (IPC::Exception& e)
		{

		}
		m_connected = false;
		m_done = true;
	}



	// Named Pipe Client
	PipeClient::PipeClient(const StringT& pipeName, PipeDirection direction /*= out*/) :
		Pipe(pipeName, direction)
	{
	}

	PipeClient::~PipeClient()
	{

	}

	bool PipeClient::Connect(DWORD timeout /*= NMPWAIT_USE_DEFAULT_WAIT*/)
	{
		bool rc = false;
		assert(!m_pipeName.empty());

		if (m_pipeHandle != INVALID_HANDLE_VALUE)
			return true;

		DWORD access = 0;
		
		if (m_direction == in)
		{
			access = GENERIC_READ;
			m_pipeReadEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		}
		else if (m_direction == out)
		{
			access = GENERIC_WRITE;
			m_pipeWriteEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		}
		else if (m_direction == both)
		{
			access = GENERIC_WRITE | GENERIC_READ;
			m_pipeReadEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
			m_pipeWriteEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		}
		while (true)
		{
			m_pipeHandle = CreateFile(m_pipeName.c_str(), access, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
			if (m_pipeHandle != INVALID_HANDLE_VALUE)
				break;

			if (GetLastError() != ERROR_PIPE_BUSY)
				break;

			if (!WaitNamedPipe(m_pipeName.c_str(), timeout))
				break;
		}

		if (m_pipeHandle != INVALID_HANDLE_VALUE)
		{
			DWORD dwMode = PIPE_READMODE_MESSAGE;
			if (SetNamedPipeHandleState(m_pipeHandle, &dwMode, NULL, NULL))
				rc = true;
			else
			{
				CloseHandle(m_pipeHandle);
				m_pipeHandle = INVALID_HANDLE_VALUE;
			}
		}
		return rc;
	}
}
