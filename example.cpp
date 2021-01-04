#include "meval.h"
#include <iostream>

int main()
{
	while (true)
	{
		std::cout << ">>";
		std::string e;
		std::getline(std::cin, e);
		std::cout << eval(e) << "\n\n";
	}
}