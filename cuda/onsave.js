const { spawn, exec } = require('child_process')

exec('scp cuda_smith_waterman_skeleton.cu ycwongal@csl2wk18.cse.ust.hk:~/COMP5112/cuda/')
const p = spawn('sh', ['./run_cuda.sh', './datasets/input2.txt', 8, 32])
p.stdout.on('data', data => onStdOut(data.toString()))
p.stderr.on('data', data => onStdErr(data.toString()))
p.on('exit', function (code) {
  console.log('child process exited with code ' + code.toString());
});

let parallelScore
let parallelTime
let processes

const onStdOut = (str) => {
  const pstr = str.replace(/\s+/g, '')
  if(/^(\d+)$/.test(pstr)){
    const score = Number.parseInt(pstr.match(/\d+/)[0])
    if(parallelScore === undefined){
      parallelScore = score
    } else {
      console.log(parallelScore === score ? 'correct: ' : 'wrong: ', parallelScore, score)
    }
  } else if(/^processes:(\d+)/.test(pstr)){
    processes = Number.parseInt(pstr.match(/\d+/))
    console.log(str)
  } else {
    console.log(`^${str}$`)
  }
}

const onStdErr = str => {
  const pstr = str.replace(/\s+/g, '')
  if(/^Time:([\d.]+)s$/.test(pstr)) {
    const time = Number.parseFloat(str.match(/([\d.]+)/))
    if(parallelTime === undefined){
      parallelTime = time
    } else {
      console.log('ratio: ', time / parallelTime, parallelTime, time)
    }
  } else {
    console.log(str)
  }
}
