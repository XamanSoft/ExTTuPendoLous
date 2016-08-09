#ifndef __XTPL_TEMPLATE_HPP
#define __XTPL_TEMPLATE_HPP

#include <ios>
#include <string>
#include <memory>
#include "stream.hpp"
#include "context.hpp"
#include "default_cxt.hpp"

namespace ExTPL {

class Template {
public:
	static bool outputError;
	
	Template(const std::string& sfn, const IStream::StreamType& stp, const Context& cxt);
	Template(const std::string& sfn, const IStream::StreamType& stp = IStream::ST_STRING, const Context* cxt = new Context::Default());
	Template(std::istream& in, const Context& cxt);
	Template(std::istream& in, const Context* cxt = new Context::Default());
	virtual ~Template();
	
	std::ostream& render(std::ostream &out);
	
	const Error& error() const;
	
private:
	const char DELIM = '@';
		
	std::unique_ptr<const Context> internalCxt;
	const Context& context;
	IStream in;
};

}

inline std::ostream& operator<<(std::ostream &out, ExTPL::Template &tpl) {
	return tpl.render(out);
}

#endif
