var stream = require('stream')
var util = require('util')
var rabin = require('bindings')('rabin')
var BufferList = require('bl')

module.exports = Rabin

function Rabin () {
  if (!(this instanceof Rabin)) return new Rabin()
  this.destroyed = false
  this.rabinEnded = false
  this.rabin = rabin.initialize()
  this.last = 0
  this.nextCb = null
  this.buffers = new BufferList()
  this.on('finish', function () {
    if (this.buffers.length) this.push(this.buffers.slice(0, this.buffers.length))
    this.push(null)
    
  })
  stream.Duplex.call(this, {objectMode: true})
}

util.inherits(Rabin, stream.Duplex)

Rabin.prototype.rabinEnd = function () {
  if (this.rabinEnded) return
  this.rabinEnded = true
  rabin.end(this.rabin)
}

Rabin.prototype._writev = function (batch, cb) {
  var self = this
  var drained = true
  if (this.destroyed) return cb()
  var bufs = batch.map(function (b) {
    self.buffers.append(b.chunk)
    return b.chunk
  })
  var offsets = []
  rabin.fingerprint(this.rabin, bufs, offsets)
  for (var i = 0; i < offsets.length; i++) {
    var offset = offsets[i]
    var size = offset - this.last
    this.last += size
    var buf = this.buffers.slice(0, size)
    this.buffers.consume(size)
    drained = this.push(buf)
  }
  if (drained) cb()
  else this.nextCb = cb
}

Rabin.prototype._read = function (size) {
  var nextCb = this.nextCb
  if (nextCb) {
    this.nextCb = null
    nextCb()
  }
}

Rabin.prototype._write = function (data, enc, cb) {
  this._writev([{chunk: data}], cb)
}

Rabin.prototype.destroy = function (err) {
  if (this.destroyed) return
  this.destroyed = true
  this.rabinEnd()
  if (err) this.emit('error', err)
  this.emit('close')
}
