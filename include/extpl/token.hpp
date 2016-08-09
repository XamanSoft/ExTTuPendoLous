#ifndef __XTPL_TOKEN_HPP
#define __XTPL_TOKEN_HPP

namespace ExTPL {
	
class IStream;

class Token {
public:
	virtual ~Token() {}
	
	virtual IStream& parse(IStream &is) =0;
};

}

inline ExTPL::IStream& operator>>(ExTPL::IStream &is, ExTPL::Token &tk) {
	return tk.parse(is);
}

#endif
