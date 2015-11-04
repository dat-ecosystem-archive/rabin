# mknod

Node bindings for [mknod](http://man7.org/linux/man-pages/man2/mknod.2.html)

```
npm install mknod
```

## Usage

``` js
var mknod = require('mknod')

mknod('some-path', modeInteger, deviceInteger, function (err) {
  if (err) throw err
  console.log('mknod worked!')
})
```

## API

#### `mknod(path, mode, dev, [callback])`

Runs the mknod syscall with the given parameters. See `man 2 mknod` for more info.

## License

MIT
