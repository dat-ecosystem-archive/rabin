#!/usr/bin/env node
var fs = require('fs')
var rabin = require('./')()
var offset = 0
var rs = fs.createReadStream(process.argv[2])
rs.pipe(rabin).on('data', function (ch) {
  console.log(offset += ch.length)
})