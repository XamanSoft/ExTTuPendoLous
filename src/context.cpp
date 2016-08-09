#include <extpl/default_cxt.hpp>
#include <extpl/template.hpp>
#include <map>
#include "duktape/duktape.h"
#include <iostream>

using ExTPL::Template;
using ExTPL::Context;
using ExTPL::Symbol;
using ExTPL::Error;
using ExTPL::IStream;
using ExTPL::Json;

struct Context::Default::JsCxtData {
	std::ostream& out;
	Context::Default& df;
};

#include "default_ctx/var_symbol.hpp"
#include "default_ctx/inc_symbol.hpp"
#include "default_ctx/defs_symbol.hpp"
#include "default_ctx/vars_symbol.hpp"
#include "default_ctx/js_symbol.hpp"
#include "default_ctx/rm_symbol.hpp"

template<typename T> Symbol* createSymbol(Context::Default& def) { return new T(def); }

std::map<std::string, Symbol*(*)(Context::Default&)> symbolTable{
	{ "{}",		&createSymbol<ExTPL::ContextDefault::VarSymbol> },
	{ "[]",		&createSymbol<ExTPL::ContextDefault::IncSymbol> },
	{ "defs{}",		&createSymbol<ExTPL::ContextDefault::DefsSymbol> },
	{ "vars{}",		&createSymbol<ExTPL::ContextDefault::VarsSymbol> },
	{ "js{}",	&createSymbol<ExTPL::ContextDefault::JsSymbol> },
	{ "rm{}",	&createSymbol<ExTPL::ContextDefault::RmSymbol> }
};

static duk_ret_t do_print(duk_context *ctx);
static duk_ret_t do_include(duk_context *ctx);
static duk_ret_t do_template_load(duk_context *ctx);

Context::Default::Default(): jsData(nullptr), ctx(duk_create_heap_default()) {
	const duk_function_list_entry sigil_funcs[] = {
		{ "print", &do_print, DUK_VARARGS },
		{ "include", &do_include, 1 },
		{ NULL, NULL, 0 }
	};
	
	duk_push_global_object(ctx);
	duk_push_c_function(ctx, do_template_load, DUK_VARARGS);  /* -> [ ... global obj ] */
	duk_put_function_list(ctx, -1, sigil_funcs);
	duk_push_pointer(ctx, &jsData);
	duk_put_prop_string(ctx, -2, "\xff""\xff""data");
	duk_push_object(ctx);
	duk_put_prop_string(ctx, -2, "vars");
	duk_put_prop_string(ctx, -2, "$");  /* -> [ ... global ] */
	duk_pop(ctx);
	
	duk_peval_string_noresult(ctx, 
		"$.printVar = function(varName) {"
			"if (typeof varName !== 'string')"
				"return;"
			//"try {"
				"var value = varName.split('.').reduce(function(o, i) {return o[i];}, $.vars);"
				"if (typeof value !== 'undefined')"
					"$.print(value)"
			/*"} catch(err) {"
				"$.print(err);"
			"}"*/
		"}"
	);
	
	duk_peval_string_noresult(ctx, 
		"$.includeVar = function(varName) {"
			"if (typeof varName !== 'string')"
				"return;"

				"var value = varName.split('.').reduce(function(o, i) {return o[i];}, $.vars);"
				"if (typeof value !== 'undefined')"
					"$.include(value); "
				"else throw 'variable not defined'"
		"}"
	);
}

Context::Default::~Default() {
	duk_destroy_heap(ctx);
}

Symbol* Context::Default::symbol(const std::string& symbolName) const {
	if (symbolTable.count(symbolName)) {
		return symbolTable[symbolName](const_cast<Context::Default&>(*this));
	}
	
	return nullptr;
}

Error& Context::Default::js(const std::string& code, JsCxtData& data, bool setError) {
	JsCxtData* d = nullptr;
	if (jsData != nullptr)
		d = jsData;
	jsData = &data;
	
	if (duk_peval_string(ctx, code.c_str()))
		err.set(duk_safe_to_string(ctx, -1));
	
	duk_pop(ctx);
	
	jsData = d;
	
	return err;
}

void Context::Default::vars(const Json& value) {
	duk_push_global_object(ctx);
	duk_get_prop_string(ctx, -1, "$");
	duk_push_string(ctx, value.dump().c_str());
	duk_json_decode(ctx, -1);
	duk_put_prop_string(ctx, -2, "vars");
	duk_pop_2(ctx);
}

Json Context::Default::vars() const {
	std::string res;
	duk_push_global_object(ctx);
	duk_get_prop_string(ctx, -1, "$");
	duk_get_prop_string(ctx, -1, "vars");
	res = duk_json_encode(ctx, -1);
	duk_pop_2(ctx);
	return Json::parse(res);
}

void Context::Default::defVars(const Json& value) {
	defVarsJsonStr(value);
}

void Context::Default::defVarsJsonStr(const std::string& jsc) {
	duk_push_global_object(ctx);
	duk_get_prop_string(ctx, -1, "$");
	duk_push_string(ctx, "(function(obj, src){Object.keys(src).forEach(function(key){ if (!obj.hasOwnProperty(key)) obj[key]=src[key]});return obj;})");
	duk_eval(ctx);
	duk_get_prop_string(ctx, -2, "vars");
	duk_push_string(ctx, (std::string("(")+jsc+")").c_str());
	duk_eval(ctx);
	duk_call(ctx, 2);
	duk_pop_3(ctx);
}

void Context::Default::varsJsonStr(const std::string& jsc) {
	duk_push_global_object(ctx);
	duk_get_prop_string(ctx, -1, "$");
	duk_push_string(ctx, "(function(obj, src){Object.keys(src).forEach(function(key){obj[key]=src[key]});return obj;})");
	duk_eval(ctx);
	duk_get_prop_string(ctx, -2, "vars");
	duk_push_string(ctx, (std::string("(")+jsc+")").c_str());
	duk_eval(ctx);
	duk_call(ctx, 2);
	duk_pop_3(ctx);
}

std::string Context::Default::varsJsonStr() const {
	std::string res;
	duk_push_global_object(ctx);
	duk_get_prop_string(ctx, -1, "$");
	duk_get_prop_string(ctx, -1, "vars");
	res = duk_json_encode(ctx, -1);
	duk_pop_2(ctx);
	return res;
}

void Context::Default::var(const std::string& name, const Json& value) {
	duk_push_global_object(ctx);
	duk_get_prop_string(ctx, -1, "$");
	duk_get_prop_string(ctx, -1, "vars");
	duk_push_string(ctx, value.dump().c_str());
	duk_json_decode(ctx, -1);
	duk_put_prop_string(ctx, -2, name.c_str());
	duk_pop_3(ctx);
}

Json Context::Default::var(const std::string& name) const {
	std::string res;
	duk_push_global_object(ctx);
	duk_get_prop_string(ctx, -1, "$");
	duk_get_prop_string(ctx, -1, "vars");
	duk_get_prop_string(ctx, -1, name.c_str());
	res = duk_json_encode(ctx, -1);
	duk_pop_3(ctx);
	return Json::parse(res);
}

duk_ret_t do_include(duk_context *ctx) {
	int n = duk_get_top(ctx);
	
	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, "\xff""\xff""data");
	Context::Default::JsCxtData* data = *static_cast<Context::Default::JsCxtData**>(duk_to_pointer(ctx, -1));
	duk_pop(ctx);
	
	if (!n || data == nullptr)
		return 0;

	std::string filename = duk_safe_to_string(ctx, 0);
	std::string suffix(".js");
	 
	if (filename.empty())
		return 0;
	
	if (filename.size() >= suffix.size() && filename.compare(filename.size() - suffix.size(), suffix.size(), suffix) == 0) {
		duk_peval_file(ctx, filename.c_str());
		duk_pop(ctx);
	} else {
		data->out << Template(filename, IStream::ST_FILE, data->df);
	}

	return 0;  /* no return value */
}

duk_ret_t do_template_load(duk_context *ctx) {
	int n = duk_get_top(ctx);
	
	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, "\xff""\xff""data");
	Context::Default::JsCxtData* data = *static_cast<Context::Default::JsCxtData**>(duk_to_pointer(ctx, -1));
	duk_pop(ctx);
	
	if (n < 2 || data == nullptr)
		return 0;
	
	return 0;
}

duk_ret_t do_print(duk_context *ctx) {
	int n = duk_get_top(ctx);
	
	duk_push_this(ctx);
	duk_get_prop_string(ctx, -1, "\xff""\xff""data");
	Context::Default::JsCxtData* data = *static_cast<Context::Default::JsCxtData**>(duk_to_pointer(ctx, -1));
	duk_pop(ctx);
	
	if (data == nullptr)
		return 0;

	for (int i = 0; i < n; i++) {
		data->out << duk_safe_to_string(ctx, i);
	}

	return 0;  /* no return value */
}
