#include <extpl/default_cxt.hpp>
#include <extpl/template.hpp>
#include <map>
#include "duktape/duktape.h"
#include <iostream>

#include "parser.js.h"

using ExTPL::Template;
using ExTPL::Context;
using ExTPL::Symbol;
using ExTPL::Error;
using ExTPL::IStream;
using ExTPL::Json;

struct Context::Default::JsCxtData {
	std::ostream& out;
	Context::Default& df;
	bool result;
	
	JsCxtData(std::ostream& out, Context::Default& df, bool result = false): out(out), df(df), result(result){}
};

Error import_string(const std::string& str, duk_context* ctx);

#include "default_ctx/var_symbol.hpp"
#include "default_ctx/inc_symbol.hpp"
#include "default_ctx/exists_symbol.hpp"
#include "default_ctx/not_exists_symbol.hpp"
#include "default_ctx/if_symbol.hpp"
#include "default_ctx/defs_symbol.hpp"
#include "default_ctx/vars_symbol.hpp"
#include "default_ctx/js_symbol.hpp"
#include "default_ctx/rm_symbol.hpp"

template<typename T> Symbol* createSymbol(Context::Default& def) { return new T(def); }

std::map<std::string, Symbol*(*)(Context::Default&)> symbolTable{
	{ "{}",		&createSymbol<ExTPL::ContextDefault::VarSymbol> },
	{ "[]",		&createSymbol<ExTPL::ContextDefault::IncSymbol> },
	{ "exists()",	&createSymbol<ExTPL::ContextDefault::ExistsSymbol> },
	{ "not.exists()",	&createSymbol<ExTPL::ContextDefault::NotExistsSymbol> },
	{ "if()",	&createSymbol<ExTPL::ContextDefault::IfSymbol> },
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
	
	duk_eval_string_noresult(ctx, "var exports={};");
	import_string(std::string(reinterpret_cast<char*>(&parser_js), sizeof(parser_js)), ctx);

	duk_peval_string_noresult(ctx, 
		"$.exists = function(varName) {"
			"if (typeof varName !== 'string')"
				"return false;"

				"var value = varName.split('.').reduce(function(o, i){return o[i];}, $.vars);"
				"return (typeof value !== 'undefined');"
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

Error& Context::Default::js(const std::string& code, JsCxtData& data) {
	JsCxtData* d = nullptr;
	if (jsData != nullptr)
		d = jsData;
	jsData = &data;
	
	if (duk_peval_string(ctx, code.c_str()))
		err.set(duk_safe_to_string(ctx, -1));
	
	jsData->result = duk_is_boolean(ctx, -1) && duk_get_boolean(ctx, -1);
	duk_pop(ctx);
	
	jsData = d;
	
	return err;
}

class JsParser {
	std::istream& is;
	duk_context* ctx;
	std::string output;

public:
	JsParser(std::istream& is, duk_context *ctx): is(is), ctx(ctx) {}
	
	void pushStream() {
		const duk_function_list_entry stream_funcs[] = {
			{ "peek", &do_stream_peek, 0 },
			{ "next", &do_stream_next, 0 },
			{ "looking_at", &do_stream_looking_at, 1 },
			{ "raw", &do_stream_raw, 2 },
			{ "output", &do_stream_output, 0 },
			{ NULL, NULL, 0 }
		};
		
		duk_push_object(ctx);  /* -> [ ... global obj ] */
		duk_put_function_list(ctx, -1, stream_funcs);
		duk_push_pointer(ctx, this);
		duk_put_prop_string(ctx, -2, "\xff""\xff""data");
		duk_peval_string(ctx, R"sfunc(
	(function(find_str) {
		var ret = '';
		if (typeof find_str == 'function') {
			var ch = ret = this.next();
			while (!find_str(ch)) {ch=this.next(); if (!ch) break; ret+=ch;}
		} else {
			var comp = '';
			for (var i=0; i < find_str.length; i++)
				comp += this.next();
			while (comp != find_str) {
				var ch = this.next();
				
				if (!ch) break;
				ret += comp.charAt(0);
				comp = comp.substr(1);
				comp += ch;
			}
		}
		return ret;
	})
)sfunc");
		duk_put_prop_string(ctx, -2, "extract");
	}
	
private:
	int peek() {
		int front = is.peek();
		if (front == EOF) front = 0;
		return front;
	}
	
	int next() {
		int front = is.get();
		if (front == EOF) front = 0;
		output += front;
		return front;
	}
	
	bool looking_at(const std::string& str) {
		return false;
	}
	
	std::string raw(int pos, int endpos) {
		return output.substr(pos, endpos);
	}
	
	static duk_ret_t do_stream_peek(duk_context *ctx) {
		duk_push_this(ctx);
		duk_get_prop_string(ctx, -1, "\xff""\xff""data");
		JsParser* data = static_cast<JsParser*>(duk_to_pointer(ctx, -1));
		duk_pop_2(ctx);

		duk_push_string(ctx, std::string(1, data->peek()).c_str());
		return 1;  /* one return value */
	}
	
	static duk_ret_t do_stream_next(duk_context *ctx) {
		duk_push_this(ctx);
		duk_get_prop_string(ctx, -1, "\xff""\xff""data");
		JsParser* data = static_cast<JsParser*>(duk_to_pointer(ctx, -1));
		duk_pop_2(ctx);

		duk_push_string(ctx, std::string(1, data->next()).c_str());
		return 1;  /* one return value */
	}
	
	static duk_ret_t do_stream_looking_at(duk_context *ctx) {
		int n = duk_get_top(ctx);
		
		if (!n)
			return 0;
		
		std::string str = duk_safe_to_string(ctx, 0);
		
		duk_push_this(ctx);
		duk_get_prop_string(ctx, -1, "\xff""\xff""data");
		JsParser* data = static_cast<JsParser*>(duk_to_pointer(ctx, -1));
		duk_pop_2(ctx);

		duk_push_boolean(ctx, data->looking_at(str));
		return 1;  /* one return value */
	}
	
	static duk_ret_t do_stream_raw(duk_context *ctx) {
		int n = duk_get_top(ctx);
		
		if (n < 2)
			return 0;
		
		int pos = duk_to_int(ctx, 0);
		int endpos = duk_to_int(ctx, 1);
		
		duk_push_this(ctx);
		duk_get_prop_string(ctx, -1, "\xff""\xff""data");
		JsParser* data = static_cast<JsParser*>(duk_to_pointer(ctx, -1));
		duk_pop_2(ctx);

		duk_push_string(ctx, data->raw(pos, endpos).c_str());
		return 1;  /* one return value */
	}
	
	static duk_ret_t do_stream_output(duk_context *ctx) {
		duk_push_this(ctx);
		duk_get_prop_string(ctx, -1, "\xff""\xff""data");
		JsParser* data = static_cast<JsParser*>(duk_to_pointer(ctx, -1));
		duk_pop_2(ctx);

		std::cout << data->output << std::endl;
		return 1;  /* one return value */
	}
};

Error& Context::Default::parseJs(IStream& is, const std::string& type, std::string& output) {
	JsParser jsparser(is, ctx);
	
	duk_push_global_object(ctx);
	duk_get_prop_string(ctx, -1, "parse");
	jsparser.pushStream();
	duk_call(ctx, 1);
	duk_push_string(ctx, type.c_str());
	is.putLC();
	if (duk_pcall(ctx, 1) != DUK_EXEC_SUCCESS) {
		std::cout << duk_safe_to_string(ctx, -1) << std::endl;
		err.set(duk_safe_to_string(ctx, -1));
		duk_pop_2(ctx);
		return err;
	}
	is.putLC();
	duk_get_prop_string(ctx, -1, "print_to_string");
	duk_dup(ctx, -2);
	duk_call_method(ctx, 0);
	output = duk_safe_to_string(ctx, -1);
	duk_pop_3(ctx);
	
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

Error import_string(const std::string& str, duk_context* ctx) {
	Error err;
	
	if (duk_pcompile_string(ctx, 0, str.c_str()) != 0) {
		err.set(duk_safe_to_string(ctx, -1));
	} else {
		if (duk_pcall(ctx, 0) != DUK_EXEC_SUCCESS) {
			err.set(duk_safe_to_string(ctx, -1));
		}
	}
	duk_pop(ctx);
	
	return err;
}
