#set -x
gcc -O3 call_rdrand.c -o call_rdrand
gcc -O3 -DTAA leak_rdrand.c -o leak_rdrand
echo running leak_rdrand and call_rdrand in background
taskset -c 4 ./call_rdrand &
taskset -c 3,7 ./leak_rdrand &
sleep 3
echo done, killing processes
killall leak_rdrand
killall call_rdrand
