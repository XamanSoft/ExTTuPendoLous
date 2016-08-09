#ifndef __XTPL_SYMBOL_HPP
#define __XTPL_SYMBOL_HPP

#include <ostream>
#include <string>

namespace ExTPL {
	
class Error;

class Symbol {
public:
	virtual ~Symbol() {}
	
	virtual bool validText(int c) const =0;
	virtual Error& exec(std::ostream& out, const std::string& text) =0;
	virtual Error& error() =0;
};

}

#endif
