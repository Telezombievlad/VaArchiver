#include "Includes.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
	try
	{
		if (argc < 3)
		{
			std::cout << "Error: va_archive needs 2 aruments [va_archive <src> <dst>]" << std::endl;
		}

		archive(argv[1], argv[2]);
	}
	catch (VaExc::Exception& exc)
	{
		std::cout << exc.what() << std::endl;
	}
}