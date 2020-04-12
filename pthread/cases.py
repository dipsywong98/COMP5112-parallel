import os

inputs = [
  './datasets/sample.in',
  './datasets/1k.in',
  './datasets/20k.in',
  './datasets/20k2k.in',
  './datasets/input1.txt',
  './datasets/input2.txt',
  './datasets/input3.txt'
]

threads = [
  '1', '2', '4', '8', '16'
]

out = {}

def run(cmd):
  proc = subprocess.Popen(cmd,
                          stdout = subprocess.PIPE,
                          stderr = subprocess.PIPE,
                          )
  stdout, stderr = proc.communicate()

  return proc.returncode, stdout, stderr

def main():
  for input in inputs:
    empty = { 'score': None, 'time': None }
    out[input] = { 'serial': empty.copy() }
    for thread in threads:
      out[input][thread] = empty.copy()

  print('compile')
  os.system('g++ -std=c++11 -pthread main.cpp pthreads_smith_waterman_skeleton.cpp -o pthreads_smith_waterman')

  for input in inputs:
    print('serial ' + input)
    os.system('./serial/serial_smith_waterman '+ input)
    # code, stdout, stderr = run([sys.executable, './serial/serial_smith_waterman', input])
    # onStdOut(stdout , './serial/serial_smith_waterman', input, 'serial')
    # onStdErr(stderr , './serial/serial_smith_waterman', input, 'serial')
    for thread in threads:
      print(thread + input)
      os.system('./pthreads_smith_waterman '+input + ' ' + thread)
      # code, stdout, stderr = run([sys.executable, './pthreads_smith_waterman', input, thread])
      # onStdOut(stdout , './pthreads_smith_waterman', input, thread)
      # onStdErr(stderr , './pthreads_smith_waterman', input, thread)
  # printWhenDone()

def onStdOut(str, cmd, input, num):
  print(str)
  # print(cmd+input+num+str)

def onStdErr(str, cmd, input, num):
  pass
  # print(cmd+input+num+str)

main()
