#ifndef __EXTPL_CONTEXT_DEFAULT_RM_SYMBOL_HPP
#define __EXTPL_CONTEXT_DEFAULT_RM_SYMBOL_HPP

#include <extpl/symbol.hpp>
#include <extpl/error.hpp>
#include <cctype>

namespace ExTPL {
namespace ContextDefault {

class RmSymbol: public Symbol {
	Error err;
	mutable int lastChar;
	
public:
	RmSymbol(Context::Default&): lastChar(-1) {}
	
	bool validText(int c) const {	
		if (lastChar == '\n' && c == '}') {
			lastChar = -1;
			return false;
		}
		
		lastChar = c;
		return true;
	}
	
	Error& exec(std::ostream& out, const std::string& text) {
		return err;
	}
	
	Error& error() {
		return err;
	}
};

}
}

#endif
