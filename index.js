var bindings = require('bindings')('rabin')
var noop = function () {}

module.exports = function (path, mode, dev, cb) {
  bindings.rabin(path, mode, dev, cb || noop)
}
