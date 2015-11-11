#include <nan.h>
#include "src/rabin.h"

using namespace v8;

#define FINGERPRINT_PT  0xbfe6b8a5bf378d83LL
#define MIN_CHUNK_SIZE  (16 * 1024)
#define AVG_CHUNK_SIZE  (32 * 1024)
#define MAX_CHUNK_SIZE  (64 * 1024 -1)

static window *fingerprints[1024];
static int fingerprint_counter = 0;

void get_fingerprints(window *rabin, Local<Array> bufs, Local<Array> offsets) {
  int count = bufs->Length();
  u_int64_t rabinf;
  int chunk_idx = 0;
  u_int BREAKMARK = (1 << (fls32(AVG_CHUNK_SIZE) - 1)) - 1;
  for (int i = 0; i < count; i++) {
    char *buf = node::Buffer::Data(bufs->Get(i));
    int byte_count = node::Buffer::Length(bufs->Get(i));
    for (int j = 0; j < byte_count; j++) {
      rabin->end++;
      int cs = rabin->end - rabin->start;
      rabinf = rabin->slide8(*buf++);
      if ((rabinf & BREAKMARK) == BREAKMARK && cs > MIN_CHUNK_SIZE) { 
        offsets->Set(chunk_idx++, Nan::New<Number>(rabin->end));
        rabin->start = rabin->end;
        rabin->reset();
      }
    }
  }
}

NAN_METHOD(Initialize) {
  if (fingerprint_counter >= 1024) return Nan::ThrowError("the value of fingerprint_counter is too damn high");
  window *myRabin = new window(FINGERPRINT_PT, 16);
  myRabin->start = 0;
  myRabin->end = 0;
  fingerprints[fingerprint_counter++] = myRabin;
  info.GetReturnValue().Set(fingerprint_counter - 1);
}

NAN_METHOD(Fingerprint) {
  if (!info[0]->IsNumber()) return Nan::ThrowError("fingerprint_idx must be a number");
  int fingerprint_idx = info[0]->Uint32Value();
  window *fingerprint_ptr = fingerprints[fingerprint_idx];

  if (!info[1]->IsArray()) return Nan::ThrowError("source must be an array"); 
  Local<Array> src = info[1].As<Array>();

  if (!info[2]->IsArray()) return Nan::ThrowError("dest must be an array"); 
  Local<Array> dest = info[2].As<Array>();
  
  get_fingerprints(fingerprint_ptr, src, dest);
}

NAN_METHOD(End) {
  if (!info[0]->IsNumber()) return Nan::ThrowError("fingerprint_idx must be a number");
  int fingerprint_idx = info[0]->Uint32Value();
  window *fingerprint_ptr = fingerprints[fingerprint_idx];
  fingerprints[fingerprint_idx] = NULL;
  while (fingerprint_counter > 0 && fingerprints[fingerprint_counter - 1] == NULL) {
    fingerprint_counter--;
  }
  
  delete fingerprint_ptr;
}

NAN_MODULE_INIT(InitAll) {
  Nan::Set(target, Nan::New<String>("initialize").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(Initialize)).ToLocalChecked());
  Nan::Set(target, Nan::New<String>("fingerprint").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(Fingerprint)).ToLocalChecked());
  Nan::Set(target, Nan::New<String>("end").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(End)).ToLocalChecked());
}

NODE_MODULE(rabin, InitAll)
