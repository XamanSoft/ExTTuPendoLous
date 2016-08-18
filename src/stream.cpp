#include <extpl/stream.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>

using ExTPL::IStream;
using ExTPL::Error;

std::vector<std::string> findPath;

class IStream::LineNumberStreamBuf : public std::streambuf {
	std::unique_ptr<std::streambuf> internalSB;
    std::streambuf& sourceSB;	
    int ln;
    char cBuf;
	char lastChar;
	bool lc;

protected:
    int underflow() {
		if (lc) {
			lc = false;
			cBuf = lastChar;
			lastChar = -1;
            setg( &cBuf, &cBuf, &cBuf + 1 );
			return cBuf;
		}
		
        int ch = sourceSB.sbumpc();
        if ( ch != EOF ) {
			lastChar = cBuf;
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
        ln(1),
		lastChar(-1),
		lc(false) {
    }
	
    LineNumberStreamBuf(std::streambuf& sb)
        : sourceSB(sb),
		ln(1),
		lastChar(-1),
		lc(false) {
    }
	
    LineNumberStreamBuf( std::istream& owner )
        : sourceSB(*owner.rdbuf()),
		ln(1),
		lastChar(-1),
		lc(false) {
    }
	
    ~LineNumberStreamBuf() {
    }
	
    int lineNumber() const {
        return ln;
    }
	
	std::streambuf* rdbuf() {
		return &sourceSB;
	}
	
	void putLC() {
        if ( lastChar != -1 ) {
			int ch = cBuf;
			cBuf = lastChar;
            lastChar = ch;
            //setg( &cBuf, &cBuf, &cBuf + 1 );
			lc = true;
        }
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
				bool fail = true;
				
				for (auto& path : findPath) {
					if (static_cast<std::filebuf*>(internalSB->rdbuf())->open(path+'/'+sfn, std::ios_base::in) == nullptr) {
						if (static_cast<std::filebuf*>(internalSB->rdbuf())->open(path+'/'+sfn + ".xtpl", std::ios_base::in) != nullptr) {
							fail = false;
							break;
						}
					} else {
						fail = false;
						break;
					}
				}
				
				if (fail)
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

void IStream::putLC() {
	internalSB->putLC();
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

bool IStream::addFindDir(const std::string& dirname) {
	struct stat info;

	if(!dirname.empty() && stat(dirname.c_str(), &info ) == 0 && info.st_mode & S_IFDIR) {
		findPath.push_back(dirname);
		return true;
	}
	
	return false;
}

void IStream::rmFindDir(const std::string& dirname) {
	if (dirname.empty()) return;
	findPath.erase(std::remove(findPath.begin(), findPath.end(), dirname), findPath.end());
}
