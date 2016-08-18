#ifndef __EXTPL_CONTEXT_DEFAULT_IF_SYMBOL_HPP
#define __EXTPL_CONTEXT_DEFAULT_IF_SYMBOL_HPP

#include <extpl/symbol.hpp>
#include <extpl/error.hpp>

namespace ExTPL {
namespace ContextDefault {

class IfSymbol: public Symbol {
	Context::Default& defaultCtx;
	Error err;
	bool res;
	std::string text;
	
public:
	IfSymbol(Context::Default& df): defaultCtx(df), res(false) {}
	
	IStream& parse(IStream &is) {
		err = defaultCtx.parseJs(is, "p", text);
		return is;
	}
	
	Error& exec(std::ostream& out) {
		Context::Default::JsCxtData data{out, defaultCtx};
		err = defaultCtx.js(std::string("with ($.vars) (")+text+").every(function(val){return !!val;})", data);
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
