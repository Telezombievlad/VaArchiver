#include "Includes.hpp"

#Includese <iostream>

int main(int argc, char* argv[]) 
{
	try
	{
		if (argc < 3)
		{
			std::cout << "Error: va_archive needs 2 aruments [va_dearchive <src> <dst>]" << std::endl;
		}

		unarchive(argv[1], argv[2]);
	}
	catch (VaExc::Exception& exc)
	{
		std::cout << exc.what() << std::endl;
	}
}