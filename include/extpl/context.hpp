#ifndef __XTPL_CONTEXT_HPP
#define __XTPL_CONTEXT_HPP

#include <istream>
#include <string>

namespace ExTPL {
	
class Symbol;

class Context {
public:
	virtual ~Context() {}
	
	virtual Symbol* symbol(const std::string& symbolName) const =0;
	
	class Default;
};

}

#endif
