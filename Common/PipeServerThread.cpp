#include "stdafx.h"
#include "PipeServerThread.h"

namespace IPC
{

	PipeServerThread::PipeServerThread()
	{
	}

	PipeServerThread::~PipeServerThread()
	{
		if (m_thread.joinable())
			m_thread.join();
	}

	void PipeServerThread::Start(const StringT& pipeName, int numInstance, IPC::PipeDirection direction, IPC::PipeServerCallback callback)
	{
		if (m_thread.get_id() == std::thread::id())
		{
			m_pipeName = pipeName;
			m_numInstance = numInstance;
			m_direction = direction;
			m_callback = callback;

			m_thread = std::thread(&PipeServerThread::ListenFunc, this);
		}
	}

	void PipeServerThread::Stop()
	{
		std::lock_guard<std::mutex> lock(m_mutexStreams);
		for (auto server : m_streams)
			server->Stop();
	}


	void PipeServerThread::ListenFunc()
	{
		try
		{
			while (true)
			{
				std::shared_ptr<IPC::PipeServer> serverStream = std::make_shared<IPC::PipeServer>(m_pipeName, m_numInstance, m_direction, m_callback);
				{
					std::lock_guard<std::mutex> lock(m_mutexStreams);
					m_streams.push_back(serverStream);
				}
				serverStream->WaitForClientConnection();
				CleanStreams();
			}
		}
		catch (IPC::Exception& e)
		{

		}
	}

	void PipeServerThread::CleanStreams()
	{
		// clean up dead pipes
		std::lock_guard<std::mutex> lock(m_mutexStreams);
		auto new_end = std::remove_if(m_streams.begin(), m_streams.end(), [](const std::shared_ptr<IPC::PipeServer>& stream){ return stream->IsDone(); });
		m_streams.erase(new_end, m_streams.end());
	}
}
