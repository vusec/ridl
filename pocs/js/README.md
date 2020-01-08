# RIDL.js

The PoC shows an attacker running within a JS sandbox leaking data from a native victim running on a different HyperThread.


## Victim (Native)

The victim code uses a non-temporal mov to improve the channel quality.
Compile the victim by simply running `make` and then run it by pinning it to a specific core:
`taskset -c 5 ./victim`


## Attacker (JS)
Our PoC targets the Firefox SpiderMonkey JS engine. Nonetheless with some tweaking you should be able to port this also to Chrome V8. 

As any timing attack leaking data from the cache this exploit requires a reliable high-resolution timer. While built-in high-resolution timers have been disabled as part of browser mitigations against side-channel attacks, prior work has demonstrated a variety of techniques to craft new high-resolution timers, ranging from counters based on SharedArrayBuffers [1,2] to GPU-based counters [3].

For simplicity, we patched the SpiderMonkey JS shell to have `performance.now()` return the output of `rdtscp`. The modified SpiderMonkey binary is available [here](https://download.vusec.net/projects/ridl/js-shell.tar.gz). 

- `ridl.wast` is the (hand-written) WebAssembly code carrying out the RIDL attack. The code contains some comments to make it a bit less indecipherable. Here we also provide a precompiled version of the code (`ridl.wasm`) in order to be able to run the PoC out of the box
- `ridl-shell.js` is the JS wrapper code you need to run in the JS shell. This code performs the initialization and the EVICT+RELOAD channel used to leak the secret. You can tweak the parameters at the top. The most relevant to tweak have comments. 

Run the attacker code by simply pinning the code to the other hyperthread:
`taskset -c 1 ./js ridl-shell.js`

![](https://github.com/vusec/ridl-artifacts/raw/master/ridl-js/js.gif)


#### References 

[1] ASLR on the Line: Practical Cache Attacks on the MMU. Gras, B.; Razavi, K.; Bosman, E.; Bos, H.; and Giuffrida, C. In NDSS'17

[2] Trusted browsers for uncertain times. Kohlbrenner, D.; and Shacham, H. In USENIX Sec'16

[3] Grand Pwning Unit: Accelerating Microarchitectural Attacks with the GPU. Frigo, P.; Giuffrida, C.; Bos, H.; Razavi, K.. In S&P'18


