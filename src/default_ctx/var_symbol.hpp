#ifndef __EXTPL_CONTEXT_DEFAULT_VAR_SYMBOL_HPP
#define __EXTPL_CONTEXT_DEFAULT_VAR_SYMBOL_HPP

#include <extpl/symbol.hpp>
#include <extpl/error.hpp>

namespace ExTPL {
namespace ContextDefault {

class VarSymbol: public Symbol {
	Error err;
	Context::Default& defaultCxt;
	
public:
	VarSymbol(Context::Default& df): defaultCxt(df) {}
	
	bool validText(int c) const {
		return c != '}';
	}
	
	Error& exec(std::ostream& out, const std::string& text) {
		size_t first = text.find_first_not_of(' ');
		size_t last = text.find_last_not_of(' ');
		Context::Default::JsCxtData data{out, defaultCxt};
		bool istr = text[first] == '"' || text[first] == '\'';
		std::string ttext = text.substr(first, (last-first+1));
		if (!istr) { ttext.insert(0, "'"); ttext.append("'"); }
		defaultCxt.js(std::string("$.") +
			(istr ? "print" : "printVar") +
				"(" + ttext + ")", data);
		return err;
	}
	
	Error& error() {
		return err;
	}
	
	bool result() const {
		return true;
	}
};

}
}

#endif
