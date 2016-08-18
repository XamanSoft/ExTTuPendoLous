#include <extpl/template.hpp>
#include <extpl/command.hpp>

using ExTPL::Context;
using ExTPL::Template;
using ExTPL::IStream;
using ExTPL::Error;

bool Template::outputError = false;

Template::Template(const std::string& sfn, const IStream::StreamType& stp, const Context& cxt)
	: context(cxt),
	in(sfn, stp) {
		
}

Template::Template(const std::string& sfn, const IStream::StreamType& stp, const Context* cxt)
	: internalCxt(cxt),
	context(*cxt),
	in(sfn, stp) {
		
}

Template::Template(std::istream& in, const Context& cxt)
	: context(cxt),
	in(in) {
	
}

Template::Template(std::istream& in, const Context* cxt)
	: internalCxt(cxt),
	context(*cxt),
	in(in) {
	
}

Template::~Template() {}

std::ostream& Template::render(std::ostream &out) {
	Command cmd(context);
		
	while (in && (in.peek() == '@' || in.get(*out.rdbuf(), DELIM))) {
		if (in.peek() == DELIM) {
			in.ignore();
			
			if (in.peek() == DELIM) {
				out << DELIM;
				in.ignore();
				continue;
			}
			
			if (in >> cmd) {
				in.error(cmd.exec(out));
			}
			
			bool result = cmd.result();
			while (in && in.peek() == ':') {
				in.ignore();
				
				if (in.peek() == ':') {
					out << ':';
					in.ignore();
					break;
				} else if (in.peek() == '!') {
					in.ignore();
					result = !result;
				}
				
				if (in >> cmd) {
					if (result) {
						in.error(cmd.exec(out));
						result = cmd.result();
					}
				}
			}
		}
	}
	
	if (outputError)
		out << in.error();

	return out;
}

const Error& Template::error() const {
	return in.error();
}
