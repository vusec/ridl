#include <asm/pgalloc.h>
#include <asm/pgtable.h>

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/version.h>

#include <mkio.h>

void (* ksym_flush_tlb_all)(void);
void (* ksym_flush_tlb_mm)(struct mm_struct *mm);
void (* ksym_flush_tlb_range)(struct vm_area_struct *vma, unsigned long start,
	unsigned long end);
void (* ksym_flush_tlb_page)(struct vm_area_struct *vma, unsigned long addr);

static struct mm_struct *
get_mm(pid_t pid)
{
	struct task_struct *task;
	struct pid *vpid;

	task = current;

	if (pid != 0) {
		vpid = find_vpid(pid);

		if (!vpid)
			return NULL;

		task = pid_task(vpid, PIDTYPE_PID);

		if (!task)
			return NULL;
	}

	if (task->mm) {
		return task->mm;
	}

	return task->active_mm;
}

static int
tlb_open(struct inode *inode, struct file *file)
{
	(void)inode;
	(void)file;

	return 0;
}

static int
tlb_release(struct inode *inode, struct file *file)
{
	(void)inode;
	(void)file;

	return 0;
}

static int resolve_va(struct pte_walk *walk, pid_t pid, unsigned long va, int update)
{
	struct mm_struct *mm;
	pgd_t *pgd;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
	p4d_t *p4d;
#endif
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	mm = get_mm(pid);

	if (!mm) {
		return EINVAL;
	}

	if (!update) {
		walk->pgd = 0;
		walk->p4d = 0;
		walk->pud = 0;
		walk->pmd = 0;
		walk->pte = 0;
	}

	down_read(&mm->mmap_sem);
	spin_lock(&mm->page_table_lock);

	pgd = pgd_offset(mm, va);

	if (update) {
		/* TODO: __pti_set_user_pgtbl() is not exported. */
		//set_pgd(pgd, __pgd(walk->pgd));
	} else {
		walk->pgd = pgd_val(*pgd);
	}

	if (pgd_none(*pgd) || pgd_bad(*pgd)) {
		goto unlock_mmap_sem;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
	p4d = p4d_offset(pgd, va);

	if (update) {
		/* TODO: __pti_set_user_pgtbl() is not exported. */
		//set_p4d(p4d, __p4d(walk->p4d));
	} else {
		walk->p4d = p4d_val(*p4d);
	}

	if (p4d_none(*p4d) || p4d_bad(*p4d)) {
		goto unlock_mmap_sem;
	}

	pud = pud_offset(p4d, va);
#else
	pud = pud_offset(pgd, va);
#endif

	if (update) {
		set_pud(pud, __pud(walk->pud));
	} else {
		walk->pud = pud_val(*pud);
	}

	if (pud_none(*pud) || pud_bad(*pud)) {
		goto unlock_mmap_sem;
	}

	pmd = pmd_offset(pud, va);

	if (update) {
		set_pmd(pmd, __pmd(walk->pmd));
	} else {
		walk->pmd = pmd_val(*pmd);
	}

	if (pmd_none(*pmd) || pmd_bad(*pmd)) {
		goto unlock_mmap_sem;
	}

	pte = pte_offset_map(pmd, va);

	if (update) {
		set_pte(pte, __pte(walk->pte));
		ksym_flush_tlb_all();
	} else {
		walk->pte = pte_val(*pte);
	}

	if (!pte_none(*pte)) {
		goto unmap_pte;
	}

	pte_unmap(pte);
	spin_unlock(&mm->page_table_lock);
	up_read(&mm->mmap_sem);

	return 0;

unmap_pte:
	pte_unmap(pte);
unlock_mmap_sem:
	spin_unlock(&mm->page_table_lock);
	up_read(&mm->mmap_sem);
	return 0;
}

static long
tlb_flush(unsigned num, struct tlb_param *params)
{
	struct mm_struct *mm;
	struct vm_area_struct *vma;

	if (num == MKIO_FLUSH_ALL) {
		ksym_flush_tlb_all();
		return 0;
	}

	mm = get_mm(params->pid);

	if (!mm) {
		return EINVAL;
	}

	if (num == MKIO_FLUSH_PID) {
		ksym_flush_tlb_mm(mm);
		return 0;
	}

	vma = find_vma(mm, (unsigned long)params->base);

	if (!vma) {
		return 0;
	}

	if (num == MKIO_FLUSH_RANGE) {
		ksym_flush_tlb_range(vma, (unsigned long)params->base,
			(unsigned long)params->base + params->size);
	} else {
		ksym_flush_tlb_page(vma, (unsigned long)params->base);
	}

	return 0;
}

static long
tlb_ioctl(
          struct file *file,
          unsigned num,
          unsigned long param)
{
	struct tlb_param params;
	int ret;

	(void)file;

	ret = copy_from_user(&params, (const void *)param, sizeof params);

	if (ret < 0) {
		return ret;
	}

	switch (num) {
	case MKIO_FLUSH_ALL:
	case MKIO_FLUSH_PID:
	case MKIO_FLUSH_RANGE:
	case MKIO_FLUSH_PAGE:
		tlb_flush(num, &params);
		break;
	case MKIO_RESOLVE:
		resolve_va(&params.walk, params.pid, (unsigned long)params.base, 0);
		break;
	case MKIO_UPDATE:
		resolve_va(&params.walk, params.pid, (unsigned long)params.base, 1);
		break;
	default:
		break;
	}

	ret = copy_to_user((void *)param, &params, sizeof params);

	if (ret < 0) {
		return ret;
	}

	return 0;
}

static struct file_operations fops = {
	.open = tlb_open,
	.release = tlb_release,
	.unlocked_ioctl = tlb_ioctl,
};

static struct miscdevice tlb_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "memkit!tlb",
	.fops = &fops,
	.mode = 0666,
};

int
tlb_init(void)
{
	int ret;

	ksym_flush_tlb_all =
		(void (*)(void))
		kallsyms_lookup_name("flush_tlb_all");
	ksym_flush_tlb_mm =
		(void (*)(struct mm_struct *))
		kallsyms_lookup_name("flush_tlb_mm");
	ksym_flush_tlb_range =
		(void (*)(struct vm_area_struct *, unsigned long,
			unsigned long))
		kallsyms_lookup_name("flush_tlb_range");
	ksym_flush_tlb_page =
		(void (*)(struct vm_area_struct *, unsigned long))
		kallsyms_lookup_name("flush_tlb_page");

	ret = misc_register(&tlb_dev);

	if (ret != 0) {
		return -1;
	}

	return 0;
}

void
tlb_exit(void)
{
	misc_deregister(&tlb_dev);
}

