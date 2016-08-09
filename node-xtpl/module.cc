// module.cc
#include <node.h>
#include <nan.h>
#include <string>
#include <sstream>
#include <extpl/template.hpp>
#include <iostream>

using namespace Nan;
using namespace v8;

/// Stringify V8 value to JSON
/// return empty string for empty value
/// https://github.com/pmed/v8pp/blob/master/v8pp/json.hpp#L19
std::string json_str(v8::Isolate* isolate, v8::Handle<v8::Value> value)
{
	if (value.IsEmpty()) {
		return std::string();
	}

	v8::HandleScope scope(isolate);

	v8::Local<v8::Object> json = isolate->GetCurrentContext()->
		Global()->Get(v8::String::NewFromUtf8(isolate, "JSON"))->ToObject();
	v8::Local<v8::Function> stringify = json->Get(v8::String::NewFromUtf8(isolate, "stringify")).As<v8::Function>();

	v8::Local<v8::Value> result = stringify->Call(json, 1, &value);
	v8::String::Utf8Value const str(result);

	return std::string(*str, str.length());
}

class RenderWorker : public AsyncWorker {
    public:
        RenderWorker(Callback *callback, std::string filename, std::string options)
        : AsyncWorker(callback), filename(filename), options(options) {}

        ~RenderWorker() {}

        void Execute () {
			ExTPL::Context::Default df;
			df.vars(ExTPL::Json::parse(options));
            output << ExTPL::Template(filename, ExTPL::IStream::ST_FILE, df);
        }

        void HandleOKCallback () {
            Local<Value> argv[] = {
                Null(),
                New<String>(output.str().c_str()).ToLocalChecked()
            };

            callback->Call(2, argv);
        }
    private:
        std::string filename;
		std::string options;
		std::stringstream output;
};

NAN_METHOD(Render) {
	if (info.Length() < 3) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}
	
	if (!info[0]->IsString()) {
		Nan::ThrowTypeError("First argument must be string");
		return;
	}
	
	if (!info[1]->IsObject()) {
		Nan::ThrowTypeError("Second argument must be object");
		return;
	}
	
	if (!info[2]->IsFunction()) {
		Nan::ThrowTypeError("Second argument must be function");
		return;
	}
	
    std::string filename = *Utf8String(info[0]->ToString());
	std::string options = json_str(v8::Isolate::GetCurrent(), info[1]);
    Callback *callback = new Callback(info[2].As<Function>());
    AsyncQueueWorker(new RenderWorker(callback, filename, options));
}

NAN_METHOD(RenderSync) {
	if (info.Length() < 2) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}
	
	if (!info[0]->IsString()) {
		Nan::ThrowTypeError("First argument must be string");
		return;
	}
	
	if (!info[1]->IsObject()) {
		Nan::ThrowTypeError("Second argument must be object");
		return;
	}
	
	std::string filename = *Utf8String(info[0]->ToString());
	std::string options = json_str(v8::Isolate::GetCurrent(), info[1]);
	std::stringstream output;
	
	ExTPL::Context::Default df;
	df.vars(ExTPL::Json::parse(options));
	output << ExTPL::Template(filename, ExTPL::IStream::ST_FILE, df);
	
	info.GetReturnValue().Set(Nan::New<String>(output.str().c_str()).ToLocalChecked());
}

NAN_METHOD(AddFindPath) {
	if (info.Length() < 1) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}
	
	if (!info[0]->IsString()) {
		Nan::ThrowTypeError("First argument must be string");
		return;
	}
	
	std::string path = *Utf8String(info[0]->ToString());
	
	info.GetReturnValue().Set(Nan::New<Boolean>(ExTPL::IStream::addFindDir(path)));
}

NAN_METHOD(RmFindPath) {
	if (info.Length() < 1) {
		Nan::ThrowTypeError("Wrong number of arguments");
		return;
	}
	
	if (!info[0]->IsString()) {
		Nan::ThrowTypeError("First argument must be string");
		return;
	}
	
	std::string path = *Utf8String(info[0]->ToString());
	ExTPL::IStream::rmFindDir(path);
}

NAN_MODULE_INIT(init) {
    Nan::SetMethod(target, "render", Render);
	Nan::SetMethod(target, "__express", Render);
	Nan::SetMethod(target, "renderSync", RenderSync);
	Nan::SetMethod(target, "addFindPath", AddFindPath);
	Nan::SetMethod(target, "rmFindPath", RmFindPath);
}

NODE_MODULE(addon, init)
