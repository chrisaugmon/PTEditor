#include <asm/tlbflush.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/fs.h>
#include <linux/fs.h>
#include <linux/kallsyms.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/ptrace.h>
#include <linux/proc_fs.h>
#include <linux/kprobes.h>

#include "pteditor.h"

MODULE_AUTHOR("Michael Schwarz");
MODULE_DESCRIPTION("Device to play around with paging structures");
MODULE_LICENSE("GPL");

#if defined(__aarch64__)
#include <linux/hugetlb.h>

static inline pte_t native_make_pte(pteval_t val)
{
  return __pte(val);
}

static inline pgd_t native_make_pgd(pgdval_t val)
{
  return __pgd(val);
}

static inline pmd_t native_make_pmd(pmdval_t val)
{
  return __pmd(val);
}

static inline pud_t native_make_pud(pudval_t val)
{
  return __pud(val);
}

static inline pteval_t native_pte_val(pte_t pte)
{
  return pte_val(pte);
}

static inline int pud_large(pud_t pud) {
  return pud_huge(pud);
}

static inline int pmd_large(pmd_t pmd) {
  return pmd_huge(pmd);
}
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#define from_user raw_copy_from_user
#define to_user raw_copy_to_user
#else
#define from_user copy_from_user
#define to_user copy_to_user
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
unsigned long kallsyms_lookup_name(const char* name) {
  struct kprobe kp = {
    .symbol_name	= name,
  };

  int ret = register_kprobe(&kp);
  if (ret < 0) {
    return 0;
  };

  unsigned long addr = kp.addr;

  unregister_kprobe(&kp);

  return addr;
}
#endif

typedef struct {
    size_t pid;
    pgd_t *pgd;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
    p4d_t *p4d;
#else
    size_t *p4d;
#endif
    pud_t *pud;
    pmd_t *pmd;
    pte_t *pte;
    size_t valid;
} vm_t;

static bool device_busy = false;
static bool mm_is_locked = false;

static int device_open(struct inode *inode, struct file *file) {
  /* Check if device is busy */
  if (device_busy == true) {
    return -EBUSY;
  }

  /* Lock module */
  try_module_get(THIS_MODULE);

  device_busy = true;

  return 0;
}

static int device_release(struct inode *inode, struct file *file) {
  /* Unlock module */
  device_busy = false;

  module_put(THIS_MODULE);

  return 0;
}

noinline __noclone  static void
invalidate_tlb(unsigned long addr) {
  get_cpu();
  count_vm_tlb_event(NR_TLB_LOCAL_FLUSH_ALL);
  local_flush_tlb();
  // the following, if used correctly, can trace TLB count.
  // trace_tlb_flush(TLB_LOCAL_SHOOTDOWN, TLB_FLUSH_ALL);
  put_cpu();
}

static void _set_pat(void* _pat) {
#if defined(__i386__) || defined(__x86_64__)
    int low, high;
    size_t pat = (size_t)_pat;
    low = pat & 0xffffffff;
    high = (pat >> 32) & 0xffffffff;
    asm volatile("wrmsr" : : "a"(low), "d"(high), "c"(0x277));
#elif defined(__aarch64__)
    size_t pat = (size_t)_pat;
    asm volatile ("msr mair_el1, %0\n" : : "r"(pat));
#endif
}

static void set_pat(size_t pat) {
    on_each_cpu(_set_pat, (void*) pat, 1);
}

static struct mm_struct* get_mm(size_t pid) {
  struct task_struct *task;
  struct pid* vpid;

  /* Find mm */
  task = current;
  if(pid != 0) {
    vpid = find_vpid(pid);
    if(!vpid) return NULL;
    task = pid_task(vpid, PIDTYPE_PID);
    if(!task) return NULL;
  }
  if(task->mm) {
      return task->mm;
  } else {
      return task->active_mm;
  }
  return NULL;
}

static int resolve_vm(size_t addr, vm_t* entry, int lock) {
  struct mm_struct *mm;

  if(!entry) return 1;
  entry->pud = NULL;
  entry->pmd = NULL;
  entry->pgd = NULL;
  entry->pte = NULL;
  entry->p4d = NULL;
  entry->valid = 0;

  mm = get_mm(entry->pid);
  if(!mm) {
      return 1;
  }

  /* Lock mm */
  if(lock) down_read(&mm->mmap_sem);

  /* Return PGD (page global directory) entry */
  entry->pgd = pgd_offset(mm, addr);
  if (pgd_none(*(entry->pgd)) || pgd_bad(*(entry->pgd))) {
      entry->pgd = NULL;
      goto error_out;
  }
  entry->valid |= PTEDIT_VALID_MASK_PGD;


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
  /* Return p4d offset */
  entry->p4d = p4d_offset(entry->pgd, addr);
  if (p4d_none(*(entry->p4d)) || p4d_bad(*(entry->p4d))) {
    entry->p4d = NULL;
    goto error_out;
  }
  entry->valid |= PTEDIT_VALID_MASK_P4D;

  /* Get offset of PUD (page upper directory) */
  entry->pud = pud_offset(entry->p4d, addr);
  if (pud_none(*(entry->pud))) {
    entry->pud = NULL;
    goto error_out;
  }
  entry->valid |= PTEDIT_VALID_MASK_PUD;
#else
  /* Get offset of PUD (page upper directory) */
  entry->pud = pud_offset(entry->pgd, addr);
  if (pud_none(*(entry->pud))) {
    entry->pud = NULL;
    goto error_out;
  }
  entry->valid |= PTEDIT_VALID_MASK_PUD;
#endif


  /* Get offset of PMD (page middle directory) */
  entry->pmd = pmd_offset(entry->pud, addr);
  if (pmd_none(*(entry->pmd)) || pud_large(*(entry->pud))) {
    entry->pmd = NULL;
    goto error_out;
  }
  entry->valid |= PTEDIT_VALID_MASK_PMD;

  /* Map PTE (page table entry) */
  entry->pte = pte_offset_map(entry->pmd, addr);
  if (entry->pte == NULL || pmd_large(*(entry->pmd))) {
    goto error_out;
  }
  entry->valid |= PTEDIT_VALID_MASK_PTE;

  /* Unmap PTE, fine on x86 and ARM64 -> unmap is NOP */
  pte_unmap(entry->pte);

  /* Unlock mm */
  if(lock) up_read(&mm->mmap_sem);

  return 0;

error_out:

  /* Unlock mm */
  if(lock) up_read(&mm->mmap_sem);

  return 1;
}


static int update_vm(ptedit_entry_t* new_entry, int lock) {
  vm_t old_entry;
  size_t addr = new_entry->vaddr;
  struct mm_struct *mm = get_mm(new_entry->pid);
  if(!mm) return 1;

  old_entry.pid = new_entry->pid;

  /* Lock mm */
  if(lock) down_read(&mm->mmap_sem);

  resolve_vm(addr, &old_entry, 0);

  /* Update entries */
  if((old_entry.valid & PTEDIT_VALID_MASK_PGD) && (new_entry->valid & PTEDIT_VALID_MASK_PGD)) {
      printk("[pteditor-module] Updating PGD\n");
      set_pgd(old_entry.pgd, native_make_pgd(new_entry->pgd));
  }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
  if((old_entry.valid & PTEDIT_VALID_MASK_P4D) && (new_entry->valid & PTEDIT_VALID_MASK_P4D)) {
      printk("[pteditor-module] Updating P4D\n");
      set_p4d(old_entry.p4d, native_make_p4d(new_entry->p4d));
  }
#endif

  if((old_entry.valid & PTEDIT_VALID_MASK_PMD) && (new_entry->valid & PTEDIT_VALID_MASK_PMD)) {
      printk("[pteditor-module] Updating PMD\n");
      set_pmd(old_entry.pmd, native_make_pmd(new_entry->pmd));
  }

  if((old_entry.valid & PTEDIT_VALID_MASK_PUD) && (new_entry->valid & PTEDIT_VALID_MASK_PUD)) {
      printk("[pteditor-module] Updating PUD\n");
      set_pud(old_entry.pud, native_make_pud(new_entry->pud));
  }

  if((old_entry.valid & PTEDIT_VALID_MASK_PTE) && (new_entry->valid & PTEDIT_VALID_MASK_PTE)) {
     // printk("[pteditor-module] Updating PTE\n");
      set_pte(old_entry.pte, native_make_pte(new_entry->pte));
  }

  invalidate_tlb(addr);

  /* Unlock mm */
  if(lock) up_read(&mm->mmap_sem);

  return 0;
}


static void vm_to_user(ptedit_entry_t* user, vm_t* vm) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 11, 0)
#if CONFIG_PGTABLE_LEVELS > 4
    if(vm->p4d) user->p4d = (vm->p4d)->p4d;
#else
    if(vm->p4d) user->p4d = (vm->p4d)->pgd.pgd;
#endif
#endif

#if defined(__i386__) || defined(__x86_64__)
    if(vm->pgd) user->pgd = (vm->pgd)->pgd;
    if(vm->pmd) user->pmd = (vm->pmd)->pmd;
    if(vm->pud) user->pud = (vm->pud)->pud;
    if(vm->pte) user->pte = (vm->pte)->pte;
#elif defined(__aarch64__)
    if(vm->pgd) user->pgd = pgd_val(*(vm->pgd));
    if(vm->pmd) user->pmd = pmd_val(*(vm->pmd));
    if(vm->pud) user->pud = pud_val(*(vm->pud));
    if(vm->pte) user->pte = pte_val(*(vm->pte));
#endif
    user->valid = vm->valid;
}


static long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param) {
  switch (ioctl_num) {
    case PTEDITOR_IOCTL_CMD_VM_RESOLVE:
    {
        ptedit_entry_t vm_user;
        vm_t vm;
        (void)from_user(&vm_user, (void*)ioctl_param, sizeof(vm_user));
        vm.pid = vm_user.pid;
        resolve_vm(vm_user.vaddr, &vm, !mm_is_locked);
        vm_to_user(&vm_user, &vm);
        (void)to_user((void*)ioctl_param, &vm_user, sizeof(vm_user));
        return 0;
    }
    case PTEDITOR_IOCTL_CMD_VM_UPDATE:
    {
        ptedit_entry_t vm_user;
        (void)from_user(&vm_user, (void*)ioctl_param, sizeof(vm_user));
        update_vm(&vm_user, !mm_is_locked);
        return 0;
    }
    case PTEDITOR_IOCTL_CMD_VM_LOCK:
    {
        struct mm_struct *mm = current->active_mm;
        if(mm_is_locked) {
            printk("[pteditor-module] VM is already locked\n");
            return -1;
        }
        down_read(&mm->mmap_sem);
        mm_is_locked = true;
        return 0;
    }
    case PTEDITOR_IOCTL_CMD_VM_UNLOCK:
    {
        struct mm_struct *mm = current->active_mm;
        if(!mm_is_locked) {
            printk("[pteditor-module] VM is not locked\n");
            return -1;
        }
        up_read(&mm->mmap_sem);
        mm_is_locked = false;
        return 0;
    }
    case PTEDITOR_IOCTL_CMD_READ_PAGE:
    {
        ptedit_page_t page;
        (void)from_user(&page, (void*)ioctl_param, sizeof(page));
        to_user(page.buffer, phys_to_virt(page.pfn * PAGE_SIZE), PAGE_SIZE);
        return 0;
    }
    case PTEDITOR_IOCTL_CMD_WRITE_PAGE:
    {
        ptedit_page_t page;
        (void)from_user(&page, (void*)ioctl_param, sizeof(page));
        (void)from_user(phys_to_virt(page.pfn * PAGE_SIZE), page.buffer, PAGE_SIZE);
        return 0;
    }
    case PTEDITOR_IOCTL_CMD_GET_ROOT:
    {
        struct mm_struct *mm;
        ptedit_paging_t paging;

        (void)from_user(&paging, (void*)ioctl_param, sizeof(paging));
        mm = get_mm(paging.pid);
        if(!mm) return 1;
        if(!mm_is_locked) down_read(&mm->mmap_sem);
        paging.root = virt_to_phys(mm->pgd);
        if(!mm_is_locked) up_read(&mm->mmap_sem);
        (void)to_user((void*)ioctl_param, &paging, sizeof(paging));
        return 0;
    }
    case PTEDITOR_IOCTL_CMD_SET_ROOT:
    {
        struct mm_struct *mm;
        ptedit_paging_t paging = {0};

        (void)from_user(&paging, (void*)ioctl_param, sizeof(paging));
        mm = get_mm(paging.pid);
        if(!mm) return 1;
        if(!mm_is_locked) down_read(&mm->mmap_sem);
        mm->pgd = (pgd_t*)phys_to_virt(paging.root);
        if(!mm_is_locked) up_read(&mm->mmap_sem);
        return 0;
    }
    case PTEDITOR_IOCTL_CMD_GET_PAGESIZE:
        return PAGE_SIZE;
    case PTEDITOR_IOCTL_CMD_INVALIDATE_TLB:
        invalidate_tlb(ioctl_param);
        return 0;
    case PTEDITOR_IOCTL_CMD_GET_PAT:
    {
#if defined(__i386__) || defined(__x86_64__)
        int low, high;
        size_t pat;
        asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(0x277));
        pat = low | (((size_t)high) << 32);
        (void)to_user((void*)ioctl_param, &pat, sizeof(pat));
        return 0;
#elif defined(__aarch64__)
        uint64_t value;
        asm volatile ("mrs %0, mair_el1\n" : "=r"(value));
        (void)to_user((void*)ioctl_param, &value, sizeof(value));
        return 0;
#endif
    }
    case PTEDITOR_IOCTL_CMD_SET_PAT:
    {
        set_pat(ioctl_param);
        return 0;
    }
	case PTEDITOR_IOCTL_CMD_TLB_SHOOTDOWN:
	{
		struct cpumask *mask;
        struct mm_struct *mm = this_cpu_read(cpu_tlbstate.loaded_mm);
		struct flush_tlb_info info = {
                .mm = mm, .start = 0, .end = TLB_FLUSH_ALL,
				.stride_shift = 0, .freed_tables = true,
				.new_tlb_gen = inc_mm_tlb_gen(mm)};

		mask = vmalloc(sizeof(struct cpumask));
		mask->bits[0] = (unsigned long)ioctl_param;
		flush_tlb_others(mask, &info);
		vfree((void*)mask);
		return 0;
	}

    default:
        return -1;
  }

  return 0;
}

static struct file_operations f_ops = {.unlocked_ioctl = device_ioctl,
                                       .open = device_open,
                                       .release = device_release};

static struct miscdevice misc_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = PTEDITOR_DEVICE_NAME,
    .fops = &f_ops,
    .mode = S_IRWXUGO,
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static struct proc_ops umem_ops = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 7, 0)
  .proc_flags = 0,
#endif
  .proc_open = NULL,
  .proc_read = NULL,
  .proc_write = NULL,
  .proc_lseek = NULL,
  .proc_release = NULL,
  .proc_poll = NULL,
  .proc_ioctl = NULL,
#ifdef CONFIG_COMPAT
  .proc_compat_ioctl = NULL,
#endif
  .proc_mmap = NULL,
  .proc_get_unmapped_area = NULL,
};
#define OP_lseek lseek
#define OPCAT(a, b) a ## b
#define OPS(o) OPCAT(umem_ops.proc_, o)
#else
static struct file_operations umem_ops = {.owner = THIS_MODULE};
#define OP_lseek llseek
#define OPS(o) umem_ops.o
#endif

static int open_umem(struct inode *inode, struct file *filp) { return 0; }
static int has_umem = 0;

#if !defined(__aarch64__)
static const char *devmem_hook = "devmem_is_allowed";


static int devmem_bypass(struct kretprobe_instance *p, struct pt_regs *regs) {
  if (regs->ax == 0) {
    regs->ax = 1;
  }
  return 0;
}

static struct kretprobe probe_devmem = {.handler = devmem_bypass, .maxactive = 20};
#endif

int init_module(void) {
  int r;

  /* Register device */
  r = misc_register(&misc_dev);
  if (r != 0) {
    printk(KERN_ALERT "[pteditor-module] Failed registering device with %d\n", r);
    return 1;
  }
  
#if !defined(__aarch64__)
  probe_devmem.kp.symbol_name = devmem_hook;

  if (register_kretprobe(&probe_devmem) < 0) {
    printk(KERN_ALERT "[pteditor-module] Could not bypass /dev/mem restriction\n");
  } else {
    printk(KERN_INFO "[pteditor-module] /dev/mem is now superuser read-/writable\n");
  }
#endif

  OPS(OP_lseek) = (void*)kallsyms_lookup_name("memory_lseek");
  OPS(read) = (void*)kallsyms_lookup_name("read_mem");
  OPS(write) = (void*)kallsyms_lookup_name("write_mem");
  OPS(mmap) = (void*)kallsyms_lookup_name("mmap_mem");
  OPS(open) = open_umem;

  if (!OPS(OP_lseek) || !OPS(read) || !OPS(write) ||
      !OPS(mmap) || !OPS(open)) {
    printk(KERN_ALERT"[pteditor-module] Could not create unprivileged memory access\n");
  } else {
    proc_create("umem", 0666, NULL, &umem_ops);
    printk(KERN_INFO "[pteditor-module] Unprivileged memory access via /proc/umem set up\n");
    has_umem = 1;
  }
  printk(KERN_INFO "[pteditor-module] Loaded.\n");

  return 0;
}

void cleanup_module(void) {
  misc_deregister(&misc_dev);
  
#if !defined(__aarch64__)
  unregister_kretprobe(&probe_devmem);
#endif

  if (has_umem) {
    printk(KERN_INFO "[pteditor-module] Remove unprivileged memory access\n");
    remove_proc_entry("umem", NULL);
  }
  printk(KERN_INFO "[pteditor-module] Removed.\n");
}
