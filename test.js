var rabin = require('./')
rabin('/Users/max/Downloads/veloroutes_id_134279.kml', function (err, result) {
  console.log('DONE', [err, result])
})