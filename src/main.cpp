#include <extpl/template.hpp>
#include <iostream>
#include <fstream>
#include <sstream>

using ExTPL::Context;
using ExTPL::Template;
using ExTPL::IStream;
using ExTPL::Error;

int main() {
	Template::outputError = true;
	std::cout << Template("helloworld.html.xtpl", IStream::ST_FILE) << std::flush;
	return 0;
}
