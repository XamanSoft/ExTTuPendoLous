#ifndef __EXTPL_CONTEXT_DEFAULT_EXISTS_SYMBOL_HPP
#define __EXTPL_CONTEXT_DEFAULT_EXISTS_SYMBOL_HPP

#include <extpl/symbol.hpp>
#include <extpl/error.hpp>

namespace ExTPL {
namespace ContextDefault {

class ExistsSymbol: public Symbol {
	Error err;
	Context::Default& defaultCxt;
	bool res;
	
public:
	ExistsSymbol(Context::Default& df): defaultCxt(df), res(false) {}
	
	bool validText(int c) const {
		return c != ')';
	}
	
	Error& exec(std::ostream& out, const std::string& text) {
		Context::Default::JsCxtData data{out, defaultCxt};
		err = defaultCxt.js(std::string("$.exists('") + text + "')", data);
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
