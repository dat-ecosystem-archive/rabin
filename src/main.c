#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include "rabin.h"

// 1MiB buffer
uint8_t buf[1024*1024];
size_t bytes;

int main(void) {
    struct rabin_t *hash;
    hash = rabin_init();

    unsigned int chunks = 0;

    while (!feof(stdin)) {
        size_t len = fread(buf, 1, sizeof(buf), stdin);
        uint8_t *ptr = &buf[0];

        bytes += len;

        while (1) {
            int remaining = rabin_next_chunk(hash, ptr, len);

            if (remaining < 0) {
                break;
            }

            len -= remaining;
            ptr += remaining;

            printf("%d %016llx\n",
                last_chunk.length,
                (long long unsigned int)last_chunk.cut_fingerprint);

            chunks++;
        }
    }

    if (rabin_finalize(hash) != NULL) {
        chunks++;
        printf("%d %016llx\n",
            last_chunk.length,
            (long long unsigned int)last_chunk.cut_fingerprint);
    }

    unsigned int avg = 0;
    if (chunks > 0)
        avg = bytes / chunks;
    printf("%d chunks, average chunk size %d\n", chunks, avg);

    return 0;
}
