import subprocess
import os, time
from get_victim import *
import signal

TIMEOUT = 15 # in seconds
REPEAT = 5

vcore = get_victim()
acore = get_attacker()
#vcore = 1
#acore = 5
iters = [200000, 250000, 500000, 750000, 1000000]

def run_experiment(vcore=1, acore=7, run_victim=True):
    lines_iter = 5
    if run_victim:
        pb = subprocess.Popen("taskset -c " + str(vcore) + " ./broadcast", shell=True, stdout=subprocess.PIPE)
    p = subprocess.Popen("taskset -c " + str(acore) + " ./read_channel " + str(lines_iter), shell=True, stdout=subprocess.PIPE)
    
    #print("Running experiment")
    try: 
        p.wait(timeout = TIMEOUT)
    except subprocess.TimeoutExpired:
        p.kill()
        if run_victim:
            pb.kill()
        print("TIMEOUT")
        return (0, 0)

    if run_victim:
        pb.kill()
    data = p.stdout.read()
    results = []
    tot_bytes = lines_iter * 100 * 64
    best_bw = 0
    for l in data.splitlines():
        e = l.split()
        time_s = e[1].decode("utf-8")
        accuracy_s = e[4].decode("utf-8")
        a, b = accuracy_s.split("/")
        accuracy = int(a)/ float(b)
        time = int(time_s.rstrip("usec"))
        results.append((time, accuracy))
        bw = (100 * 64 * accuracy) / (time  / 1000000.0)
        best_bw = max(best_bw, bw)

    if len(results) == 0:
        return (0,0)
    
    tot_t = sum(map(lambda e: e[0], results))
    avg_t = sum(map(lambda e: e[0], results))/ len(results)
    avg_acc = sum(map(lambda e: e[1], results)) / len(results)
    #print(avg_t, avg_acc) 
    
    ts = (tot_t / float(1000000))
    #print("TOT bytes: {}".format(tot_bytes))
    #print("IN {} secs".format(ts))
    #print("{:.0f} bytes / s with {}% accuracy".format(tot_bytes/ ts, int(avg_acc * 100)))
    #return (tot_bytes/ ts, int(avg_acc * 100))
    return (best_bw, int(avg_acc * 100))

def compile(iters):
    #print("Compiling experiment")
    p1 = subprocess.Popen("gcc -o read_channel read_channel.c -O3", shell=True, stderr=subprocess.DEVNULL)
    p2 = subprocess.Popen("gcc -o broadcast broadcast.c -O3 -DITERS="+ str(iters), shell=True,  stderr=subprocess.DEVNULL)
    p1.wait()
    p2.wait()
    #print("Done Compiling")

def setup_kern_mod(iters):
    #p1 = subprocess.Popen("cd kmod && make clean && CFLAGS=\"-DITERS=" + str(iters) + "\" make && sudo insmod kvict.ko", shell=True, stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL)
    os.chdir("kmod/")
    p1 = subprocess.Popen("make -s clean > /dev/null 2>&1", shell=True)
    p1.wait()
    p1 = subprocess.Popen("CFLAGS=\"-DITERS=" + str(iters) + "\" make -s  > /dev/null 2>&1", shell=True)
    p1.wait()
    p1 = subprocess.Popen("sudo insmod kvict.ko", shell=True, stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL)
    p1.wait()
    os.chdir("../")

def clean_kern_mod():
    p1 = subprocess.Popen("cd kmod && make -s clean && sudo rmmod kvict", shell=True, stderr=subprocess.DEVNULL)
    p1.wait()

def run_local_bench():
    # Local
    print("LOCAL BENCH")
    for i in iters:
        compile(i)
        max_bw = 0
        max_acc = 0
        for _ in range(REPEAT):
            bw, acc = run_experiment(vcore=vcore, acore=acore)
            if bw > max_bw:
                max_bw = bw
                max_acc = max(max_acc, acc)
        print("{:.0f} bytes / s with {}% accuracy with {} iters".format(max_bw, max_acc, i))



def run_vm_bench():
    # VM
    print("VM BENCH")
    for i in iters:
        #compile(i)
        
        # Setup VM
        pb = subprocess.Popen("./start-vm-ridl", shell=True, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, preexec_fn=os.setsid)
        max_bw = 0
        max_acc = 0
        for _ in range(REPEAT):
            bw, acc = run_experiment(vcore=vcore, acore=acore, run_victim=False)
            if bw > max_bw:
                max_bw = bw
                max_acc = max(max_acc, acc)
        print("{:.0f} bytes / s with {}% accuracy with {} iters".format(max_bw, max_acc, i))

        # Kill VM
        pb.kill()
        os.killpg(os.getpgid(pb.pid), signal.SIGTERM)

def run_kern_bench():
    print("KERN BENCH")
    for i in iters:
        setup_kern_mod(i)
        max_bw = 0
        max_acc = 0
        for _ in range(REPEAT):
            p1 = subprocess.Popen("while true; do taskset -c " + str(vcore) + " cat /proc/kvict; done", shell=True, stderr=subprocess.DEVNULL, stdout=subprocess.DEVNULL)
            bw, acc = run_experiment(vcore=vcore, acore=acore, run_victim=False)
            if bw > max_bw:
                max_bw = bw
                max_acc = max(max_acc, acc)
            p1.kill()
        print("{:.0f} bytes / s with {}% accuracy with {} iters".format(max_bw, max_acc, i))
        clean_kern_mod()


def setup_sgx():
    os.chdir("sgx/")
    p1 = subprocess.Popen("bash ./build.sh > /dev/null 2>&1", shell=True)
    #p1 = subprocess.Popen("bash ./build.sh", shell=True)
    p1.wait()
    os.chdir("..")

def rebuild_sgx(iters):
    os.chdir("sgx/")
    p1 = subprocess.Popen("CFLAGS=\"-DITERS=" + str(iters)+ "\" bash ./rebuild.sh > /dev/null 2>&1", shell=True)
    #p1 = subprocess.Popen("bash ./build.sh", shell=True)
    p1.wait()
    os.chdir("..")


def has_sgx():
    os.chdir("tools/")
    p1 = subprocess.Popen("make > /dev/null 2>&1", shell=True)
    p1.wait()
    p1 = subprocess.Popen(["./sgxcheck"])
    p1.wait()
    os.chdir("..")
    return p1.returncode == 0


def run_sgx_bench():
    # SGX
    print("SGX BENCH")
    setup_sgx()
    for i in iters:
        rebuild_sgx(i)
        max_bw = 0
        max_acc = 0
        for _ in range(REPEAT):
            os.chdir("sgx/linux-sgx/SampleCode/LocalAttestation")
            p1 = subprocess.Popen("taskset -c " + str(vcore)+ " ./app  > /dev/null 2>&1", shell=True)
            os.chdir("../../../../")
            bw, acc = run_experiment(vcore=vcore, acore=acore, run_victim=False)
            if bw > max_bw:
                max_bw = bw
                max_acc = max(max_acc, acc)
            p1.kill()
        print("{:.0f} bytes / s with {}% accuracy with {} iters".format(max_bw, max_acc, i))

def setup_hugepages():
    p1 = subprocess.Popen("echo 128 > /proc/sys/vm/nr_hugepages", shell=True)
    p1.wait()



if __name__ == "__main__":
    setup_hugepages()
    run_local_bench()
    run_vm_bench()
    run_kern_bench()
    if has_sgx():
        run_sgx_bench()




"""
compile(250000)
bw, acc = run_experiment()
print("{:.0f} bytes / s with {}% accuracy".format(bw, acc))

compile(500000)
bw, acc = run_experiment()
print("{:.0f} bytes / s with {}% accuracy".format(bw, acc))

compile(750000)
bw, acc = run_experiment()
print("{:.0f} bytes / s with {}% accuracy".format(bw, acc))

compile(1000000)
bw, acc = run_experiment()
print("{:.0f} bytes / s with {}% accuracy".format(bw, acc))
"""
