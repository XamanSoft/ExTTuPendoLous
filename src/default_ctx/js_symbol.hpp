#ifndef __EXTPL_CONTEXT_DEFAULT_JS_SYMBOL_HPP
#define __EXTPL_CONTEXT_DEFAULT_JS_SYMBOL_HPP

#include <extpl/symbol.hpp>
#include <extpl/error.hpp>

namespace ExTPL {
namespace ContextDefault {

class JsSymbol: public Symbol {
	Context::Default& defaultCtx;
	Error err;
	std::string text;
	
public:
	JsSymbol(Context::Default& df): defaultCtx(df) {}
	
	IStream& parse(IStream &is) {
		err = defaultCtx.parseJs(is, "b", text);
		return is;
	}

	Error& exec(std::ostream& out) {
		Context::Default::JsCxtData data{out, defaultCtx};
		err = defaultCtx.js(text, data);
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
