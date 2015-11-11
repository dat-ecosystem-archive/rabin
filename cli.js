#!/usr/bin/env node
var fs = require('fs')
var crypto = require('crypto')
var rabin = require('./')()
var offset = 0
var rs = fs.createReadStream(process.argv[2])
var count = 0
rs.pipe(rabin).on('data', function (ch) {
  offset += ch.length
  count++
  var hash = crypto.createHash('sha256').update(ch).digest('hex')
  var data = {
    length: ch.length,
    offset: offset,
    hash: hash
  }
  console.log(JSON.stringify(data))
}).on('end', function () {
  console.error('average', ~~(offset / count))
})

// var averageChunkSize = 16 * 1024
// Math.pow(Math.ceil(Math.log2(averageChunkSize)) - 1, 2) - 1