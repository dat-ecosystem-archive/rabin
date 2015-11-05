var rabin = require('./')
rabin(process.argv[2], function (err, result) {
  console.log('DONE', [err, result])
})