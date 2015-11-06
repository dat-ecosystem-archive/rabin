# rabin

Node native addon module (C/C++) for [Rabin fingerprinting](https://en.wikipedia.org/wiki/Rabin_fingerprint) data streams.

Uses the implementation of Rabin fingerprinting from [LBFS](https://github.com/fd0/lbfs/tree/bdf4f17d23b68536e7805c88e269026c74c32d59/liblbfs).

Rabin fingerprinting is useful for finding the chunks of a file that differ from a previous version. It's one implementation of a technique called "Content-defined chunking", meaning the chunk boundaries are determinstic to the content (as opposed to "fixed-sized chunking").

## API

### `var createRabin = require('rabin')`

`createRabin` can be used to create multiple fingerprinting streams

### `var rabin = createRabin()`

`rabin` is a duplex stream. You write raw data in, and buffers chunked by rabin fingerprints will be written out.

## Example

```js
// require and create an instance
var rabin = require('rabin')()

// pipe some data in
var rs = fs.createReadStream('somefile.dat')
rs.pipe(rabin)

// handle output chunks
rabin.on('data', function (chunk) {
  // chunks are created by taking your input data
  // and splitting on each rabin fingerprint found
})
```
