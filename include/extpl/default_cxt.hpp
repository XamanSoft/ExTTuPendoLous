#ifndef __XTPL_CONTEXT_DEFAULT_HPP
#define __XTPL_CONTEXT_DEFAULT_HPP

#include "context.hpp"
#include "json.hpp"
#include "error.hpp"
#include <string>
#include <sstream>

typedef struct duk_hthread duk_context;

namespace ExTPL {
	
using Json = nlohmann::json;
	
class Symbol;

class Context::Default: public Context {
public:
	struct JsCxtData;
	
private:
	JsCxtData* jsData;
	duk_context *ctx;
	Error err;
	
public:
	Default();
	virtual ~Default();
	
	virtual Symbol* symbol(const std::string& symbolName) const;
	
	virtual Error& js(const std::string& code, JsCxtData& data, bool setError = true);
	
	virtual void defVars(const Json& value);
	virtual void defVarsJsonStr(const std::string& jsc);
	virtual void vars(const Json& value);
	virtual Json vars() const;
	virtual void varsJsonStr(const std::string& jsc);
	virtual std::string varsJsonStr() const;
	virtual void var(const std::string& name, const Json& value);
	virtual Json var(const std::string& name) const;
};

}

#endif
