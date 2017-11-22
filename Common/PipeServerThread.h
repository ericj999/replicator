#pragma once
#include <thread>
#include <mutex>
#include "StringT.h"
#include "NamedPipe.h"
#include <functional>

namespace IPC
{
	class PipeServerThread
	{
	public:
		PipeServerThread();
		~PipeServerThread();

		void Start(const StringT& pipeName, int numInstance, IPC::PipeDirection direction, IPC::PipeServerCallback callback);
		void Stop();
		bool IsRunning() { return (m_thread.get_id() != std::thread::id()) ? true : false; }

	protected:
		std::thread m_thread;
		std::mutex m_mutexStreams;
		std::vector<std::shared_ptr<IPC::PipeServer>> m_streams;

		StringT m_pipeName;
		int m_numInstance;
		IPC::PipeDirection m_direction;
		IPC::PipeServerCallback m_callback;

		void ListenFunc();
		void CleanStreams();
	};
}
