#ifndef __XTPL_COMMAND_HPP
#define __XTPL_COMMAND_HPP

#include "token.hpp"
#include "error.hpp"
#include "stream.hpp"
#include <string>
#include <ios>

namespace ExTPL {
	
class Context;
class Symbol;

class Command: public Token {
	const Context& context;
	Symbol* currentSymbol;
	std::string textBlock;
	Error err;
	
public:
	Command(const Context& cxt);
	~Command();
	
	IStream& parse(IStream &is);
	Error& exec(std::ostream& out);
	Error& error();
	
private:
	inline std::string name(std::istream &is);
	inline bool startSymbol(int c);
	inline int endSymbol(int st);
	inline std::string text(std::istream &is);
};

}

#endif
