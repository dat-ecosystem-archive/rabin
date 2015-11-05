var fs = require('fs')
var rabin = require('./')()

var rs = fs.createReadStream(process.argv[2])
rs.pipe(rabin).on('data', function (ch) {
  console.log(ch.length)
})