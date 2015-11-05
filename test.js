var fs = require('fs')
var rabin = require('./')
var ptr = rabin.initialize()
var rs = fs.createReadStream(process.argv[2])
rs.on('data', function (ch) {
  var offsets = []
  var result = rabin.fingerprint(ptr, [ch, ch, ch, ch], offsets)
  console.log(result)
  console.log(offsets)
})
