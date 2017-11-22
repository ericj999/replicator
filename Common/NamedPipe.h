#pragma once

#include <thread>
#include <functional>

#include "StringT.h"

namespace IPC
{
#define PIPE_MESSAGE_TIMEOUT		(300 * 1000)	// 5 minutes
#define PIPE_TIMEOUT_CHECK_INTVAL	500
#define DEFAULT_TIMEOUT_VALUE		1000			// default timeout for WaitForNamedPipe() until client calling CreateFile(), set by server in CreateNamedPipe

#define PIPE_BUFFER_SZIE	4096

	using PipeServerCallback = std::function < void(const unsigned char*, size_t) >;


	enum PipeDirection
	{
		both,
		in,
		out,
		none
	};

	class Exception : public std::exception
	{
	public:
		Exception(const char* error) : exception(error) {}
		Exception(DWORD error)
		{
			std::string errorString;
			char* errorText = NULL;
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (char*)&errorText,  0, NULL);

			if (errorText)
			{
				errorString = errorText;
				LocalFree(errorText);
				errorText = NULL;
			}
			exception(errorString.c_str());
		}
		~Exception() {}
	};

	using PipeData = std::vector<unsigned char>;

	class Pipe
	{
	public:
		virtual ~Pipe();

		PipeData Read(DWORD waitTime = PIPE_MESSAGE_TIMEOUT);
		void Write(const PipeData& data, DWORD waitTime = PIPE_MESSAGE_TIMEOUT);

		bool CanRead() { return (m_pipeHandle != INVALID_HANDLE_VALUE) && ((m_direction == in) || (m_direction == both)); }
		bool CanWrite() { return (m_pipeHandle != INVALID_HANDLE_VALUE) && ((m_direction == out) || (m_direction == both)); }

		void Stop();

	protected:
		Pipe();
		Pipe(const StringT& pipeName, PipeDirection direction, int bufferSize = PIPE_BUFFER_SZIE);
		void Ctor();

		StringT m_pipeName;
		PipeDirection m_direction;
		HANDLE m_pipeHandle;
		HANDLE m_pipeReadEvent;
		HANDLE m_pipeWriteEvent;
		int m_bufferSize;
		HANDLE m_stopHandle;
	};

	class PipeServer : public Pipe
	{
	public:
		PipeServer(const StringT& pipeName, int numInstance, PipeDirection direction = in, PipeServerCallback callback = nullptr, int defaultTimeout = DEFAULT_TIMEOUT_VALUE);
		~PipeServer();

		void WaitForClientConnection();
		bool IsConnected() { return m_connected;  }	// clinet has connected
		bool IsDone() { return m_done; }	// done reading from pipe

	protected:
		HANDLE m_connectHandle;
		std::thread m_thread;
		PipeServerCallback m_callback;
		bool m_done;
		bool m_connected;

		PipeServer() {}
		void ReadFunc();
	};

	class PipeClient : public Pipe
	{
	public:
		PipeClient(const StringT& pipeName, PipeDirection direction = out);
		~PipeClient();

		bool Connect(DWORD timeout = NMPWAIT_USE_DEFAULT_WAIT);
	};
}

