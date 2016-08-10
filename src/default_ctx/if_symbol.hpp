#ifndef __EXTPL_CONTEXT_DEFAULT_IF_SYMBOL_HPP
#define __EXTPL_CONTEXT_DEFAULT_IF_SYMBOL_HPP

#include <extpl/symbol.hpp>
#include <extpl/error.hpp>
//#include <cstring>
//#include <iostream>

namespace ExTPL {
namespace ContextDefault {

class IfSymbol: public Symbol {
	enum CharType {
		NORMAL,
		COMMENT_LN,
		COMMENT_MLN,
		LITERAL_S,
		LITERAL_D,
		IGNORE
	};
	
	Context::Default& defaultCtx;
	Error err;
	bool res;
	mutable int lastChar;	
	
public:
	IfSymbol(Context::Default& df): defaultCtx(df), res(false), lastChar(-1) {}
	
	bool validText(int c) const {	
		if (lastChar == '\n' && c == ')') {
			lastChar = -1;
			return false;
		}
		
		lastChar = c;
		return true;
	}
	
	Error& exec(std::ostream& out, const std::string& text) {
		Context::Default::JsCxtData data{out, defaultCtx};
		err = defaultCtx.js(std::string("(")+text+")", data);
		res = data.result;
		return err;
	}
	
	Error& error() {
		return err;
	}
	
	bool result() const {
		return res;
	}
};

}
}

#endif
