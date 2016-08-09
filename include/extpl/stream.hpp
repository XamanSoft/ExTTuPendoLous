#ifndef __XTPL_STREAM_HPP
#define __XTPL_STREAM_HPP

#include <istream>
#include <memory>
#include "error.hpp"

namespace ExTPL {
	
class IStream: public std::istream
{
	class LineNumberStreamBuf;
	
	std::unique_ptr<LineNumberStreamBuf> internalSB;
	Error err;
	
public:	
	enum StreamType {
		ST_STRING,
		ST_FILE
	};
	
	explicit IStream(const std::string& sfn, const StreamType& stp);
	explicit IStream(std::streambuf& sb);	
	explicit IStream(std::istream& owner);
	
	virtual ~IStream();

	int lineNumber() const;
	
	const Error& error(const char* msg = nullptr);
	const Error& error(const Error& e);
	const Error& error() const;
};

}

#endif