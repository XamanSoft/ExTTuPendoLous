#ifndef __XTPL_SYMBOL_HPP
#define __XTPL_SYMBOL_HPP

#include <ostream>
#include <string>
#include "token.hpp"

namespace ExTPL {
	
class Error;
class IStream;

class Symbol: public Token {
public:
	virtual ~Symbol() {}
	
	virtual Error& exec(std::ostream& out) =0;
	virtual bool result() const=0;
	virtual Error& error() =0;
};

}

#endif
