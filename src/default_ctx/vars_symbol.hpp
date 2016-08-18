#ifndef __EXTPL_CONTEXT_DEFAULT_VARS_SYMBOL_HPP
#define __EXTPL_CONTEXT_DEFAULT_VARS_SYMBOL_HPP

#include <extpl/symbol.hpp>
#include <extpl/error.hpp>

namespace ExTPL {
namespace ContextDefault {

class VarsSymbol: public Symbol {
	Error err;
	Context::Default& defaultCxt;
	std::string text;
	
public:
	VarsSymbol(Context::Default& df): defaultCxt(df) {}
	
	IStream& parse(IStream &is) {
		err = defaultCxt.parseJs(is, "o", text);
		return is;
	}
	
	Error& exec(std::ostream& out) {
		defaultCxt.varsJsonStr(text);
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
