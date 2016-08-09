#ifndef __EXTPL_CONTEXT_DEFAULT_INC_SYMBOL_HPP
#define __EXTPL_CONTEXT_DEFAULT_INC_SYMBOL_HPP

#include <extpl/symbol.hpp>
#include <extpl/error.hpp>

namespace ExTPL {
namespace ContextDefault {

class IncSymbol: public Symbol {
	Error err;
	Context::Default& defaultCxt;
	
public:
	IncSymbol(Context::Default& df): defaultCxt(df) {}
	
	bool validText(int c) const {
		return c != ']';
	}
	
	Error& exec(std::ostream& out, const std::string& text) {
		size_t first = text.find_first_not_of(' ');
		size_t last = text.find_last_not_of(' ');
		Context::Default::JsCxtData data{out, defaultCxt};
		bool istr = text[first] == '"' || text[first] == '\'';
		std::string ttext = text.substr(first, (last-first+1));
		if (!istr) { ttext.insert(0, "'"); ttext.append("'"); }
		return defaultCxt.js(std::string("$.") +
			(istr ? "include" : "includeVar") +
				"(" + ttext + ")", data);
	}
	
	Error& error() {
		return err;
	}
};

}
}

#endif