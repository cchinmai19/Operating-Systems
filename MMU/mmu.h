#include <linux/types.h>
#include <linux/syscalls.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <asm/linkage.h>
#include <linux/highmem.h>
#include <asm/page.h>
#include <asm/uaccess.h>
#include <asm/current.h>
#include <asm/pgtable.h>
#include <linux/mm.h>
#include <asm/tlbflush.h>
#include <linux/mm_types.h>
#include <asm/kmap_types.h>
#include <asm/io.h>

asmlinkage long sys_readMMU(unsigned long vaddr,unsigned long * pte);
asmlinkage long sys_writeMMU(unsigned long vaddr, unsigned long pte);


