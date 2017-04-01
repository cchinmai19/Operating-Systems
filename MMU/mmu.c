#include "mmu.h"
#define MASK 0x3ffffffff000
#define FLAG_MASK 0xffffc00000000fff

static inline unsigned char *get_pagetable (void);
unsigned long *last_vaddr; 

/* function name : readMMU
 * arguments : vaddr(virtual address) pte(memory pointer)
 * Description:
 * returns 0 on storing page table entry 
 * for vaddr into memory pointed by pte
 * returns -1 if first second and third table are not present
 * returns pte for page even if 4th table not present
 */
 
asmlinkage long sys_readMMU(unsigned long vaddr, unsigned long * pte)
{
	struct mm_struct *mm;
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *ptep;
	pgd_t *pgd1;
	unsigned long temp_last = vaddr;
	unsigned long PGD_offset,PUD_offset,PMD_offset,PTE_offset, Page_offset;
	
	/* obtain PTE PUD PMD page offset from virtual address by shifting virtual address 
	 * and masking virtual address */
	Page_offset = vaddr & 4095;
        vaddr = vaddr >> 12;
	PTE_offset = vaddr & 511;
	vaddr = vaddr >> 9;
	PMD_offset = vaddr & 511;
	vaddr = vaddr >> 9 ;
	PUD_offset = vaddr & 511;
	vaddr = vaddr >> 9;
	PGD_offset = vaddr & 511;
	mm = current->mm;
	pgd = mm->pgd + PGD_offset; 
	//pgd = get_pagetable();
	//pgd = pgd+PGD_offset;

	if(pgd->pgd & 1) /* check if pgd is present for virtual address */
	{
		pud =(unsigned long) __va((pgd->pgd & MASK));
		pud = pud + PUD_offset;
	}
	else
       	{
		printk("readMMU: %lu PGD not Found!\n",vaddr);
		return -1;
	} 
	
	if(pud->pud & 1) /* check if pud is present */
	{
		pmd =(unsigned long) __va((pud->pud & MASK)) ;
		pmd = pmd + PMD_offset;
	}
	else
	{
		printk("readMMU: %lu PUD not Found!\n",vaddr);
		return -1;
	}
        
	if(pmd->pmd & 1) /* check if pmd is present */
	{
	 	ptep =(unsigned long) __va((pmd->pmd & MASK));
		ptep = ptep + PTE_offset;
		if(copy_to_user(pte,&ptep->pte,sizeof(unsigned long)))
		{	

			printk("readMMU: copy to user failed \n");	
			return -1;

		}
	}
	else
	{
		printk("readMMU: %lu PMD not Found!\n",vaddr);
		return -1;
	}
	
	return 0;		
}

/* function name : writeMMU 
 * arguments : vaddr(virtual address) pte(memory pointer)
 * Description:
 * return 0 stores the value of page table entry pte in 
 * for a correct vaddr.
 * return -1 if first second and third page table entry 
 * is not there
 */

asmlinkage long sys_writeMMU(unsigned long vaddr, unsigned long pte)
{	
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *ptep;
	pgd_t *pgd1;
	struct mm_struct *mm;
	int *temp_v = vaddr; 
	
	unsigned long PGD_offset,PUD_offset,PMD_offset,PTE_offset, Page_offset;

	/* obtain PTE PUD PMD page offset from virtual address by shifting virtual address 
	 * and masking virtual address */
	
	Page_offset = vaddr & 4095;
        vaddr = vaddr >> 12;
	PTE_offset = vaddr & 511;
	vaddr = vaddr >> 9;
	PMD_offset = vaddr & 511;
	vaddr = vaddr >> 9 ;
	PUD_offset = vaddr & 511;
	vaddr = vaddr >> 9;
	PGD_offset = vaddr & 511;
	mm = current->mm;
	pgd = mm->pgd + PGD_offset; 
	//pgd = __va(get_pagetable());
	//pgd = pgd + PGD_offset;


	if((pgd->pgd & 1) && pgd != NULL) /* check if pgd is present */
	{
		pud = (unsigned long)__va(pgd->pgd & MASK); 
		pud = pud + PUD_offset ;
	}
	else
       	{
		printk("writeMMU: %lu PGD not Found!\n",vaddr);
		return -1;
	} 
	
	if((pud->pud & 1) && pud != NULL) /* check if the page table entry is present in pud*/
	{
		pmd = (unsigned long)__va(pud->pud & MASK);
		pmd = pmd + PMD_offset ;
	}
	else
	{
		printk("writeMMU: %lu PUD not Found!\n",vaddr);
		return -1;
	}
        
	if((pmd->pmd & 1) && pmd != NULL) /* check if the page table entry is in pmd */
	{
	 	ptep = (unsigned long)__va(pmd->pmd & MASK);
		ptep = ptep + PTE_offset;
		ptep->pte = pte;
	}
	else
	{
		printk("writeMMU: %lu PMD not Found!\n",vaddr);
		return -1;
	}

	/* flush the TLB after writing to the page table entry */

	flush_tlb_mm(mm);
	
	
	return 0;	

}

/*
 * Function: get_pagetable()
 * 
 * Description:
 * Return a physical address that can be used to access the current page table.
 */

static inline unsigned char *get_pagetable (void) {
  /* Value of the CR3 register */
  uintptr_t cr3;

  /* Get the page table value out of CR3 */
  __asm__ __volatile__ ("movq %%cr3, %0\n" : "=r" (cr3));

 /*
 * Shift the value over 12 bits.  The lower-order 12 bits of the page table
 * pointer are assumed to be zero, and so they are reserved or used by the
 * hardware.
 */
  return (unsigned char *)((((uintptr_t)cr3) & 0x000ffffffffff000u));
}


