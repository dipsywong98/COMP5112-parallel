const { spawn } = require('child_process')

// execSync('scp mpi_smith_waterman_skeleton.cpp ycwongal@csl2wk18.cse.ust.hk:~/COMP5112/mpi/', console.log)
const p = spawn('sh', ['./watch_mpi.sh', 'test.cpp', '2'])
p.stdout.on('data', data => console.log(data.toString()))
p.stderr.on('data', data => console.log(data.toString()))
p.on('exit', function (code) {
  console.log('child process exited with code ' + code.toString());
});