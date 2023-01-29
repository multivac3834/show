#include <exception>
#include <iostream>
#include <memory>
#include <cassert>

#include "util.h"
#include "tree_util.h"


auto main(int argc, char** argv) -> int
{
	try
	{
		g::ARGUMENTS = std::make_unique<util::Args>(argc, argv);
		tree::traverse<tree::Root>();
	}
	catch(std::exception const& e)
	{
		std::cout << e.what();
	}
	catch(...)
	{
		assert(true);
		std::cout << "ups";
	}
}
