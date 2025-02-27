#include <exception>
#include <iostream>

#include "Renderer.hpp"

int main(int argc, char** argv)
{
	try
	{
		Renderer renderer;

		renderer.run();
	}
	catch (const std::exception& exp)
	{
		std::cerr << "Exception: " << exp.what() << '\n';
	}

	return 0;
}