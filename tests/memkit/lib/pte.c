#include <stdlib.h>

#include <memkit.h>

int clr_memory_ac(void *addr)
{
	struct ptes ptes;

	if (tlb_resolve_va(&ptes, 0, addr) < 0) {
		return -1;
	}

	ptes.pte &= ~PTE_ACCESS;

	if (tlb_update_va(0, addr, &ptes) < 0) {
		return -1;
	}

	return 0;
}

int set_cacheability(void *addr, unsigned mem_type)
{
	struct ptes ptes;

	mem_type = pat_find_type(mem_type);
	mem_type = __builtin_ffs(mem_type);

	if (!mem_type) {
		return -1;
	}

	--mem_type;

	if (tlb_resolve_va(&ptes, 0, addr) < 0) {
		return -1;
	}

	ptes.pte &= ~(PTE_WT | PTE_CD | PTE_PAT);

	if (mem_type & 1) {
		ptes.pte |= PTE_WT;
	}

	if (mem_type & 2) {
		ptes.pte |= PTE_CD;
	}

	if (mem_type & 4) {
		ptes.pte |= PTE_PAT;
	}

	if (tlb_update_va(0, addr, &ptes) < 0) {
		return -1;
	}

	return 0;
}

int set_memory_wb(void *addr)
{
	return set_cacheability(addr, MEM_WB);
}

int set_memory_wt(void *addr)
{
	return set_cacheability(addr, MEM_WT);
}

int set_memory_wc(void *addr)
{
	return set_cacheability(addr, MEM_WC);
}

int set_memory_uc_minus(void *addr)
{
	return set_cacheability(addr, MEM_UC_MINUS);
}

int set_memory_uc(void *addr)
{
	return set_cacheability(addr, MEM_UC);
}

