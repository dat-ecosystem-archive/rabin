#include <nan.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "src/rabinpoly.h"

using namespace v8;

NAN_METHOD(Rabin) {
  Nan::HandleScope scope;

  if (!info[0]->IsString()) return Nan::ThrowError("path must be a string");
}

NAN_MODULE_INIT(InitAll) {
  Nan::Set(target, Nan::New<String>("rabin").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(Rabin)).ToLocalChecked());
}

NODE_MODULE(rabin, InitAll)