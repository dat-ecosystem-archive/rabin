#!/usr/bin/env node
var fs = require('fs')
var crypto = require('crypto')
var args = require('minimist')(process.argv.slice(2))
var rabin = require('./')(args)
var offset = 0
var rs = fs.createReadStream(args._[0])
var count = 0
rs.pipe(rabin).on('data', function (ch) {
  if (args.metadata) {
    var hash = crypto.createHash('sha256').update(ch.buffer).digest('hex')
    var data = {
      length: ch.buffer.length,
      offset: offset,
      hash: hash,
      trailingZeros: ch.trailingZeros
    }
    offset += ch.buffer.length
  }
  else {
    var hash = crypto.createHash('sha256').update(ch).digest('hex')
    var data = {
      length: ch.length,
      offset: offset,
      hash: hash
    }
    offset += ch.length
  }
  console.log(JSON.stringify(data))
  count++
}).on('end', function () {
  console.error('average', ~~(offset / count))
})
