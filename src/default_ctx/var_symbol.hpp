#ifndef __EXTPL_CONTEXT_DEFAULT_VAR_SYMBOL_HPP
#define __EXTPL_CONTEXT_DEFAULT_VAR_SYMBOL_HPP

#include <extpl/symbol.hpp>
#include <extpl/error.hpp>

namespace ExTPL {
namespace ContextDefault {

class VarSymbol: public Symbol {
	Error err;
	Context::Default& defaultCxt;
	std::string text;
	
public:
	VarSymbol(Context::Default& df): defaultCxt(df) {}
	
	IStream& parse(IStream &is) {
		err = defaultCxt.parseJs(is, "bp", text);
		return is;
	}
	
	Error& exec(std::ostream& out) {
		Context::Default::JsCxtData data{out, defaultCxt};
		err = defaultCxt.js(std::string("try{with ($.vars)") + text + ".forEach(function(item){$.print(item)})}catch(e){}", data);
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
