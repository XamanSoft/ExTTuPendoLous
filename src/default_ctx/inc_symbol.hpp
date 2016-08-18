#ifndef __EXTPL_CONTEXT_DEFAULT_INC_SYMBOL_HPP
#define __EXTPL_CONTEXT_DEFAULT_INC_SYMBOL_HPP

#include <extpl/symbol.hpp>
#include <extpl/error.hpp>

namespace ExTPL {
namespace ContextDefault {

class IncSymbol: public Symbol {
	Error err;
	Context::Default& defaultCxt;
	std::string text;
	
public:
	IncSymbol(Context::Default& df): defaultCxt(df) {}
	
	IStream& parse(IStream &is) {
		err = defaultCxt.parseJs(is, "a", text);
		return is;
	}
	
	Error& exec(std::ostream& out) {
		Context::Default::JsCxtData data{out, defaultCxt};
		err = defaultCxt.js(std::string("try{with ($.vars)") + text + ".forEach(function(item){$.include(item)})}catch(e){}", data);
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
