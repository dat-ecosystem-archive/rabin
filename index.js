var stream = require('readable-stream')
var util = require('util')
var rabin = require('bindings')('rabin')
var BufferList = require('bl')
var debug = require('debug')('rabin')

module.exports = Rabin

function Rabin (opts) {
  if (!(this instanceof Rabin)) return new Rabin(opts)
  if (!opts) opts = {}
  stream.Duplex.call(this, {objectMode: true})
  this.destroyed = false
  this.rabinEnded = false
  var avgBits = +opts.bits || 12
  var min = +opts.min || 8 * 1024
  var max = +opts.max || 32 * 1024
  this.metadata = opts.metadata ? true : false
  this.rabin = rabin.initialize(avgBits, min, max)
  this.nextCb = null
  this.buffers = new BufferList()
  this.on('finish', function () {
    if (this.buffers.length) {
      if (this.metadata) {
        this.push({
          buffer: this.buffers.slice(0, this.buffers.length),
          trailingZeros: 0,
        })
      }
      else {
        this.push(this.buffers.slice(0, this.buffers.length))
      }
    }
    this.push(null)
    this.rabinEnd()
  })
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
    var chunk = toBuffer(b.chunk)
    self.buffers.append(chunk)
    return chunk
  })
  var lengths = []
  var trailingZeros = []
  rabin.fingerprint(this.rabin, bufs, lengths, trailingZeros)
  debug('chunks', lengths)
  for (var i = 0; i < lengths.length; i++) {
    var size = lengths[i]
    var buf = this.buffers.slice(0, size)
    this.buffers.consume(size)
    drained = this.push(this.metadata ? {
        buffer: buf,
        trailingZeros: trailingZeros[i]
      } : buf)
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

function toBuffer (buf) {
  return Buffer.isBuffer(buf) ? buf : new Buffer(buf)
}
