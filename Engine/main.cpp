#include <exception>
#include <iostream>
/*
#include <array>
#include <string>
#include <string_view>
#include <fstream>
#include <vector>
#include <sstream>
#include <optional>
#include <unordered_map>
*/

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