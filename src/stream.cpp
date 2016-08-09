#include <extpl/stream.hpp>
#include <fstream>
#include <sstream>

using ExTPL::IStream;
using ExTPL::Error;

class IStream::LineNumberStreamBuf : public std::streambuf {
	std::unique_ptr<std::streambuf> internalSB;
    std::streambuf& sourceSB;	
    int ln;
    char cBuf;

protected:
    int underflow() {
        int ch = sourceSB.sbumpc();
        if ( ch != EOF ) {
            cBuf = ch;
            setg( &cBuf, &cBuf, &cBuf + 1 );
            if (cBuf == '\n') {
                ++ ln;
            }
        }
        return ch;
    }
	
public:
	LineNumberStreamBuf(std::streambuf* sb)
        : internalSB(sb),
		sourceSB(*sb),
        ln(1) {
    }
	
    LineNumberStreamBuf(std::streambuf& sb)
        : sourceSB(sb),
		ln(1) {
    }
	
    LineNumberStreamBuf( std::istream& owner )
        : sourceSB(*owner.rdbuf()),
		ln(1) {
    }
	
    ~LineNumberStreamBuf() {
    }
	
    int lineNumber() const {
        return ln;
    }
	
	std::streambuf* rdbuf() {
		return &sourceSB;
	}
};

IStream::IStream(const std::string& sfn, const StreamType& stp)
	: std::istream(new LineNumberStreamBuf(
		stp == ST_STRING ?
			static_cast<std::streambuf*>(new std::stringbuf(sfn, std::ios_base::in)) :
			static_cast<std::streambuf*>(new std::filebuf())
	)),
	internalSB(static_cast<LineNumberStreamBuf*>(rdbuf())) {
	
	if (stp == ST_FILE) {
		if (static_cast<std::filebuf*>(internalSB->rdbuf())->open(sfn, std::ios_base::in) == nullptr) {
			if (static_cast<std::filebuf*>(internalSB->rdbuf())->open(sfn + ".xtpl", std::ios_base::in) == nullptr) {
				setstate(std::ios::badbit);
			}
		}
	}
}

IStream::IStream(std::streambuf& sb)
	: std::istream(new LineNumberStreamBuf(sb)),
	internalSB(static_cast<LineNumberStreamBuf*>(rdbuf())) {
	
}

IStream::IStream(std::istream& owner)
	: std::istream(new LineNumberStreamBuf(owner)),
	internalSB(static_cast<LineNumberStreamBuf*>(rdbuf())) {

}

IStream::~IStream() {
	
}

int IStream::lineNumber() const {
	return internalSB->lineNumber();
}
	
const Error& IStream::error(const char* msg) {
	if (msg != nullptr) {
		setstate(std::ios::failbit);
		err.set(msg, internalSB->lineNumber());
	}
	
	return err;
}

const Error& IStream::error(const Error& e) {
	if (e) {
		setstate(std::ios::failbit);
		err.set(e.msg(), internalSB->lineNumber());
	}
	
	return err;
}

const Error& IStream::error() const {
	return err;
}
