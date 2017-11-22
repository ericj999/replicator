// RepProc.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "RepRunner.h"

int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		RepRunner theApp;

		if (!theApp.ParseCommandLine(argc, argv))
			return 1;

		theApp.run();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what();
	}
	return 0;
}

