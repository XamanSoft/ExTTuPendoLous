#ifndef __EXTPL_CONTEXT_DEFAULT_DEFS_SYMBOL_HPP
#define __EXTPL_CONTEXT_DEFAULT_DEFS_SYMBOL_HPP

#include <extpl/symbol.hpp>
#include <extpl/error.hpp>

namespace ExTPL {
namespace ContextDefault {

class DefsSymbol: public Symbol {
	Error err;
	Context::Default& defaultCxt;
	mutable int lastChar;
	
public:
	DefsSymbol(Context::Default& df): defaultCxt(df), lastChar(-1) {}
	
	bool validText(int c) const {	
		if (lastChar == '\n' && c == '}') {
			lastChar = -1;
			return false;
		}
		
		lastChar = c;
		return true;
	}
	
	Error& exec(std::ostream& out, const std::string& text) {
		defaultCxt.defVarsJsonStr(std::string("{") + text +  "}");
		return err;
	}
	
	Error& error() {
		return err;
	}
};

}
}

#endif
