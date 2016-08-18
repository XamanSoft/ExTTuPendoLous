#ifndef __EXTPL_CONTEXT_DEFAULT_NOT_EXISTS_SYMBOL_HPP
#define __EXTPL_CONTEXT_DEFAULT_NOT_EXISTS_SYMBOL_HPP

#include <extpl/symbol.hpp>
#include <extpl/error.hpp>

namespace ExTPL {
namespace ContextDefault {

class NotExistsSymbol: public Symbol {
	Error err;
	Context::Default& defaultCxt;
	bool res;
	std::string text;
	
public:
	NotExistsSymbol(Context::Default& df): defaultCxt(df), res(false) {}
	
	IStream& parse(IStream &is) {
		while (is && is.peek() != ')') {
			text += is.get();
		}
		return is;
	}
	
	Error& exec(std::ostream& out) {
		Context::Default::JsCxtData data{out, defaultCxt};
		err = defaultCxt.js(std::string("with ($.vars) typeof ") + text + " == 'undefined'", data);
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
