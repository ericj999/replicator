#pragma once

#include "PipeServerThread.h"

class MessageServer :
	public IPC::PipeServerThread
{
public:
	MessageServer();
	~MessageServer();

	void Broadcast(void* data, size_t size);
};

