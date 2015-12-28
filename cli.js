#!/usr/bin/env node
var fs = require('fs')
var crypto = require('crypto')
var args = require('minimist')(process.argv.slice(2))
var rabin = require('./')(args)
var offset = 0
var rs = fs.createReadStream(args._[0])
var count = 0
rs.pipe(rabin).on('data', function (ch) {
  var hash = crypto.createHash('sha256').update(ch).digest('hex')
  var data = {
    length: ch.length,
    offset: offset,
    hash: hash
  }
  console.log(JSON.stringify(data))
  offset += ch.length
  count++
}).on('end', function () {
  console.error('average', ~~(offset / count))
})
