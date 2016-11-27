var execSync = require('child_process').execSync
var spawn = require('win-spawn')
var join = require('path').join

function version (rev) {
  var diff = execSync('git show '+rev+':package.json', {
    encoding: 'utf8'
  })
  if (!diff) return
  return JSON.parse(diff).version
}

function prebuild (args, cb) {
  var cli = join(__dirname, 'node_modules', '.bin', 'prebuild')
  var ps = spawn(cli, args)
  ps.stdout.pipe(process.stdout, { end: false })
  ps.stderr.pipe(process.stderr, { end: false })
  ps.on('exit', cb)
}

if (version('HEAD') === version('HEAD~1')) process.exit(0)

prebuild([
  '-b', process.version,
  '-u', process.env.PREBUILD_TOKEN
], function (code) {
  process.exit(code)
})
