#ifndef __EXTPL_CONTEXT_DEFAULT_RM_SYMBOL_HPP
#define __EXTPL_CONTEXT_DEFAULT_RM_SYMBOL_HPP

#include <extpl/symbol.hpp>
#include <extpl/error.hpp>
#include <extpl/stream.hpp>

namespace ExTPL {
namespace ContextDefault {

class RmSymbol: public Symbol {
	Error err;
	const int endChar;
	
public:
	RmSymbol(Context::Default&): endChar('}') {}
	
	IStream& parse(IStream& is) {
		int c = -1;
		int lastChar = -1;
		
		while ((c = is.peek()) != EOF) {
			if (lastChar == '\n' && c == endChar) {
				break;
			}
			
			lastChar = c;
			is.ignore();
		}	
		
		return is;
	}
	
	Error& exec(std::ostream& out) {
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
