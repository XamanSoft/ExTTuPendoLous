#ifndef __XTPL_ERROR_HPP
#define __XTPL_ERROR_HPP

#include <exception>
#include <string>
#include <ostream>
#include <sstream>

namespace ExTPL {

class Error: public std::exception {
	std::string errorMsg;
	std::string fmtStr;
	int ln;
public:
	explicit Error(const std::string& msg = std::string(), int ln = 0)
		: errorMsg(msg),
		ln(ln) {
		std::stringstream ss;
		ss << "Line " << ln << ": " << errorMsg;
		fmtStr = ss.str();
	}

	virtual bool hasError() const { return !errorMsg.empty(); }
	virtual operator bool() const { return !errorMsg.empty(); }
	
	virtual int lineNumber() const { return ln; }
	
	virtual void set(const std::string& msg = std::string(), int ln = 0) {
		errorMsg = msg;
		this->ln = ln;
		std::stringstream ss;
		ss << "Line " << ln << ": " << errorMsg;
		fmtStr = ss.str();
	}
	
	virtual const char* msg() const throw() {
		if (errorMsg.empty())
			return nullptr;
		
		return errorMsg.c_str();
	}
	
    virtual const char* what() const throw() {
		if (errorMsg.empty())
			return nullptr;
		
		return fmtStr.c_str();
	}
};

}

inline std::ostream& operator<<(std::ostream &out, const ExTPL::Error &error) {
	if (error)
		out << std::endl << error.what();
	return out;
}

#endif
