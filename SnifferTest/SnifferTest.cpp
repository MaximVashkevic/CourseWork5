// SnifferTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "../SnifferLib/SnifferLib.h"

int main()
{
	HANDLE handle = StartSniffing();

	if (handle != 0)
	{
		const size_t size = 1600;
		PUCHAR buffer = (UCHAR*)malloc(size);
		for (int j = 0; j < 100; j++)
		{
			int readcout = GetPacket(handle, buffer, size);

			for (int i = 0; i < readcout; ++i)
			{
				printf("%02X-", buffer[i]);
			}

			printf("\n");
		}

		StopSniffing(handle);
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
