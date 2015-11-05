#include <nan.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for 'read/close'
#include <sys/types.h>  // for 'open'
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>  // for 'isprint'

#include "src/rabinpoly.h"
#include "src/sha1.h"

#define FINGERPRINT_PT  0xbfe6b8a5bf378d83LL
#define BREAKMARK_VALUE 0x78
#define MIN_CHUNK_SIZE  (2 * 1024)
#define MAX_CHUNK_SIZE  (64 * 1024 -1)
#define MEAN_CHUNK_SIZE MIN_CHUNK_SIZE

#define BUFSIZE 1024

struct chunk {
    off_t offset;
    size_t size;
    unsigned char sha1sum[20];
};

char* progname;

char *sha1_to_hex(const unsigned char *sha1)
{
    static char buffer[50];
    static const char hex[] = "0123456789abcdef";
    char *buf = buffer;
    int i;

    for (i = 0; i < 20; i++) {
    unsigned int val = *sha1++;
    *buf++ = hex[val >> 4];
    *buf++ = hex[val & 0xf];
    }
    return buffer;
}

int run_task(char* filename, int windowsize)
{
    int count;
    int fd;
    off_t _cur_pos = 0, _last_pos = 0;
    int min_size_suppress = 0, max_size_suppress = 0;
    u_int64_t rabinf;
    unsigned char buf[BUFSIZE]; // buffer to read data in
    struct sha1_ctx c;
    unsigned char sha1[20];

    // INITIALIZE RABINPOLY CLASS
    window myRabin(FINGERPRINT_PT, windowsize);

    // RESET if you want to clear up the buffer in the RABINPOLY CLASS
    myRabin.reset();

    // OPEN THE FILE
    fd = open(filename, O_RDONLY);
    if (!fd) {
      fprintf(stderr, "Failed to open %s\n", filename);
      return -1;
    }

    sha1_init(&c);

    while( (count = read(fd, buf, BUFSIZE)) > 0 ) {
      // FEED RABINPOLY CLASS BYTE-BY-BYTE
      // count the offset in file
      for (int i=0; i<count; i++, _cur_pos++) {
        rabinf = myRabin.slide8(buf[i]);
        size_t cs = _cur_pos - _last_pos;
        sha1_update(&c, &buf[i], 1); //update sha1 for each data

        if ((rabinf % MEAN_CHUNK_SIZE) == BREAKMARK_VALUE && cs < MIN_CHUNK_SIZE)
          min_size_suppress++;
        else if (cs == MAX_CHUNK_SIZE)
          max_size_suppress++;
        if (((rabinf % MEAN_CHUNK_SIZE) == BREAKMARK_VALUE && cs >= MIN_CHUNK_SIZE)
        || cs >= MAX_CHUNK_SIZE) {
          /*reset rabin poly buf*/
          myRabin.reset();
          /* finish sha1 hash */
          sha1_final(&c, sha1);

          struct chunk new_chunk;
          new_chunk.offset = _last_pos;
          new_chunk.size = cs;
          memcpy(new_chunk.sha1sum, sha1, 20);

          /* reset cursor */
          _last_pos = _cur_pos;
          /* */
          printf("%s\t%lu\n", sha1_to_hex(new_chunk.sha1sum),
                 new_chunk.size);

          /* reset sha1 digest */
          sha1_init(&c);
        }
      }
    }
    /*FIXME : we should handle last chunk here*/
    sha1_final(&c, sha1);
    size_t cs = _cur_pos - _last_pos;
    printf("%s\t%lu\n", sha1_to_hex(sha1), cs);
    
    close(fd);
    return 1;
}

using namespace v8;

class RabinWorker : public Nan::AsyncWorker {
 public:
  RabinWorker(Nan::Callback *callback, char *path)
    : Nan::AsyncWorker(callback), path(path) {}
  ~RabinWorker() {}

  void Execute () {
    // READ THE FILE AND COMPUTE RABIN FINGERPRINT
    this->error = run_task(path, 16);
    free(path);
  }

  void HandleOKCallback () {
    Nan::HandleScope scope;
    if (this->error == -1) {
      Local<Value> tmp[] = {
        Nan::Error("rabin failed")
      };
      callback->Call(1, tmp);
    } else {
      callback->Call(0, NULL);
    }
  }

 private:
  char *path;
  int error;
};

NAN_METHOD(Rabin) {
  Nan::HandleScope scope;
  if (!info[0]->IsString()) return Nan::ThrowError("path must be a string");

  if (!info[1]->IsFunction()) return Nan::ThrowError("callback must be a function");
  Local<Function> callback = info[1].As<Function>();

  Nan::Utf8String path(info[0]);
  char *path_alloc = (char *) malloc(1024);
  stpcpy(path_alloc, *path);
  
  Nan::AsyncQueueWorker(new RabinWorker(new Nan::Callback(callback), path_alloc));
}

NAN_MODULE_INIT(InitAll) {
  Nan::Set(target, Nan::New<String>("rabin").ToLocalChecked(), Nan::GetFunction(Nan::New<FunctionTemplate>(Rabin)).ToLocalChecked());
}

NODE_MODULE(rabin, InitAll)
