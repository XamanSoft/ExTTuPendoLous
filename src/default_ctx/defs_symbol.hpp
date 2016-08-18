#ifndef __EXTPL_CONTEXT_DEFAULT_DEFS_SYMBOL_HPP
#define __EXTPL_CONTEXT_DEFAULT_DEFS_SYMBOL_HPP

#include <extpl/symbol.hpp>
#include <extpl/error.hpp>

namespace ExTPL {
namespace ContextDefault {

class DefsSymbol: public Symbol {
	Error err;
	Context::Default& defaultCxt;
	std::string text;
	
public:
	DefsSymbol(Context::Default& df): defaultCxt(df) {}
	
	IStream& parse(IStream &is) {
		err = defaultCxt.parseJs(is, "o", text);
		return is;
	}
	
	Error& exec(std::ostream& out) {
		defaultCxt.defVarsJsonStr(text);
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
