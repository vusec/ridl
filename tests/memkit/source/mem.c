/* Use kprobe to make sure /dev/mem is accessible */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/ktime.h>
#include <linux/limits.h>
#include <linux/sched.h>

static int
devmem_bypass(struct kretprobe_instance *probe, struct pt_regs *regs)
{
	/* Just return one to indicate that access is always allowed. */
	regs->ax = 1;
	return 0;
}

static struct kretprobe probe = {
	.handler = devmem_bypass,
	.maxactive = 20,
};

int
devmem_init(void)
{
	int ret;

	/* Register a kprobe for devmem_is_allowed(). */
	probe.kp.symbol_name = "devmem_is_allowed";
	ret = register_kretprobe(&probe);

	if (ret < 0) {
		pr_err("register_kretprobe() failed with %d\n", ret);
		return -1;
	}

	pr_info("Planted return probe at %s: %p\n",
		probe.kp.symbol_name, probe.kp.addr);

	return 0;
}

void
devmem_exit(void)
{
	/* Unregister the kprobe. */
	unregister_kretprobe(&probe);
	pr_info("kretprobe at %p unregistered\n", probe.kp.addr);
	pr_info("missed probing %d instance of %s\n",
		probe.nmissed, probe.kp.symbol_name);
}

