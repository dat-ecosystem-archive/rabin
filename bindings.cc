#include <stdlib.h>
#include <nan.h>
#include "src/rabin.h"

using namespace v8;

static rabin_t *instances[1024];
static int instance_counter = 0;

void get_fingerprints(rabin_t *hasher, Local<Array> bufs, Local<Array> lengths) {
  int count = bufs->Length();
  int chunk_idx = 0;
  for (int i = 0; i < count; i++) {
    uint8_t *buf = (uint8_t*) node::Buffer::Data(bufs->Get(i));
    int len = node::Buffer::Length(bufs->Get(i));
    uint8_t *ptr = &buf[0];

    while (1) {
        int remaining = rabin_next_chunk(hasher, ptr, len);
        if (remaining < 0) {
            break;
        }

        len -= remaining;
        ptr += remaining;

        lengths->Set(chunk_idx++, Nan::New<Number>(hasher->chunk_length));
    }
  }
}

NAN_METHOD(Initialize) {
  if (instance_counter >= 1024) return Nan::ThrowError("the value of instance_counter is too damn high");
  struct rabin_t *hasher = (struct rabin_t *) malloc(sizeof(struct rabin_t));
  hasher->polynomial = 0x3DA3358B4DC173LL;
  hasher->polynomial_degree = 53;
  hasher->average_bits = 14;
  hasher->minsize = 8 * 1024;
  hasher->maxsize = 32 * 1024;
  hasher->mask = ((1<<hasher->average_bits)-1);
  hasher->polynomial_shift = (hasher->polynomial_degree-8);
  rabin_init(hasher);
  instances[instance_counter++] = hasher;
  info.GetReturnValue().Set(instance_counter - 1);
}

NAN_METHOD(Fingerprint) {
  if (!info[0]->IsNumber()) return Nan::ThrowError("fingerprint_idx must be a number");
  int fingerprint_idx = info[0]->Uint32Value();
  rabin_t *fingerprint_ptr = instances[fingerprint_idx];

  if (!info[1]->IsArray()) return Nan::ThrowError("source must be an array");
  Local<Array> src = info[1].As<Array>();

  if (!info[2]->IsArray()) return Nan::ThrowError("dest must be an array");
  Local<Array> dest = info[2].As<Array>();

  get_fingerprints(fingerprint_ptr, src, dest);
}

NAN_METHOD(End) {
  if (!info[0]->IsNumber()) return Nan::ThrowError("fingerprint_idx must be a number");
  int fingerprint_idx = info[0]->Uint32Value();
  rabin_t *fingerprint_ptr = instances[fingerprint_idx];
  instances[fingerprint_idx] = NULL;
  while (instance_counter > 0 && instances[instance_counter - 1] == NULL) {
    instance_counter--;
  }

  free(fingerprint_ptr);
}

NAN_MODULE_INIT(InitAll) {
  Nan::Set(target, Nan::New<String>("initialize").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(Initialize)).ToLocalChecked());
  Nan::Set(target, Nan::New<String>("fingerprint").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(Fingerprint)).ToLocalChecked());
  Nan::Set(target, Nan::New<String>("end").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(End)).ToLocalChecked());
}

NODE_MODULE(rabin, InitAll)
