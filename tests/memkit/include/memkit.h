#pragma once

#include <stdint.h>

#if defined(__i386__) || defined(__x86_64__)
#define PTE_PRESENT  (1 << 0)
#define PTE_WRITE    (1 << 1)
#define PTE_USER     (1 << 2)
#define PTE_WT       (1 << 3)
#define PTE_CD       (1 << 4)
#define PTE_ACCESS   (1 << 5)
#define PTE_DIRTY    (1 << 6)
#define PTE_HUGE     (1 << 7)
#define PTE_PAT      (1 << 7)
#define PTE_GLOBAL   (1 << 8)
#define PTE_NO_EXEC  (1ull << 63)

#define MEM_UC       0
#define MEM_WC       1
#define MEM_WT       4
#define MEM_WP       5
#define MEM_WB       6
#define MEM_UC_MINUS 7
#endif

struct ptes {
	uint64_t pgd, p4d, pud, pmd, pte;
};

int memkit_init(void);
void memkit_exit(void);

int tlb_resolve_va(struct ptes *ptes, pid_t pid, void *va);
int tlb_update_va(pid_t pid, void *va, struct ptes *ptes);
int tlb_flush_all(void);
int tlb_flush_pid(pid_t pid);
int tlb_flush_range(pid_t pid, void *va, size_t size);
int tlb_flush_page(pid_t pid, void *va);

void *kmalloc(size_t size);
void kfree(void *addr);
void *vmalloc(size_t size);
void vfree(void *addr);
size_t kmem_read(void *buf, size_t count, void *addr);
size_t kmem_write(const void *buf, size_t count, void *addr);
uint64_t kload(void *addr);
void kprefetcht0(void *addr);
void kprefetcht1(void *addr);
void kprefetcht2(void *addr);
void kprefetchnta(void *addr);
void kclflush(void *addr);
void kclflushopt(void *addr);

uint64_t pat_read(void);
int pat_write(uint64_t val);
unsigned pat_find_type(unsigned mem_type);
int pat_get_type(unsigned *mem_type, uint64_t entry);
int pat_set_type(uint64_t *entry, unsigned mem_type);
const char *mem_type_to_str(unsigned mem_type);

int clr_memory_ac(void *addr);
int set_cacheability(void *addr, unsigned mem_type);
int set_memory_wb(void *addr);
int set_memory_wt(void *addr);
int set_memory_wc(void *addr);
int set_memory_uc_minus(void *addr);
int set_memory_uc(void *addr);

