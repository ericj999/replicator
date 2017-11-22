#include "stdafx.h"
#include "MessageServer.h"


MessageServer::MessageServer()
{
}


MessageServer::~MessageServer()
{
}

void MessageServer::Broadcast(void* data, size_t size)
{
	IPC::PipeData pd(size);
	memcpy(pd.data(), data, size);

	for (auto& stream : m_streams)
	{
		if (stream->IsConnected() && stream->CanWrite())
		{
			try
			{
				stream->Write(pd);
			}
			catch (IPC::Exception e)
			{
				CString err{ e.what() };
			}
		}
	}
}

