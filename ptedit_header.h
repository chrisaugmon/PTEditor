/* See LICENSE file for license and copyright information */

#ifndef PTEDITOR_MODULE_H
#define PTEDITOR_MODULE_H

#include <stddef.h>

#define PTEDITOR_DEVICE_NAME "pteditor"
#define PTEDITOR_DEVICE_PATH "/dev/" PTEDITOR_DEVICE_NAME

/**
 * Structure containing the page-table entries of all levels.
 * The Linux names are aliased with the Intel names.
 */
typedef struct {
    /** Process ID */
    size_t pid;
    /** Virtual address */
    size_t vaddr;

    /** Page global directory / Page map level 5 */
    union {
        size_t pgd;
        size_t pml5;
    };
    /** Page directory 4 / Page map level 4 */
    union {
        size_t p4d;
        size_t pml4;
    };
    /** Page upper directory / Page directory pointer table */
    union {
        size_t pud;
        size_t pdpt;
    };
    /** Page middle directory / Page directory */
    union {
        size_t pmd;
        size_t pd;
    };
    /** Page table entry */
    size_t pte;
    /** Bitmask indicating which entries are valid/should be updated */
    size_t valid;
} ptedit_entry_t;

/**
 * Structure to read/write physical pages
 */
typedef struct {
    /** Page-frame number */
    size_t pfn;
    /** Virtual address */
    size_t vaddr;
    /** Page size */
    size_t size;
    /** Page content */
    unsigned char* buffer;
} ptedit_page_t;


/**
 * Structure to get/set the root of paging
 */
typedef struct {
    /** Process id */
    size_t pid;
    /** Physical address of paging root */
    size_t root;
} ptedit_paging_t;

#define PTEDIT_VALID_MASK_PGD (1<<0)
#define PTEDIT_VALID_MASK_P4D (1<<1)
#define PTEDIT_VALID_MASK_PUD (1<<2)
#define PTEDIT_VALID_MASK_PMD (1<<3)
#define PTEDIT_VALID_MASK_PTE (1<<4)


#define PTEDITOR_IOCTL_MAGIC_NUMBER (long)0x3d17

#define PTEDITOR_IOCTL_CMD_VM_RESOLVE \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 1, size_t)

#define PTEDITOR_IOCTL_CMD_VM_UPDATE \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 2, size_t)

#define PTEDITOR_IOCTL_CMD_VM_LOCK \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 3, size_t)

#define PTEDITOR_IOCTL_CMD_VM_UNLOCK \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 4, size_t)

#define PTEDITOR_IOCTL_CMD_READ_PAGE \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 5, size_t)

#define PTEDITOR_IOCTL_CMD_WRITE_PAGE \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 6, size_t)

#define PTEDITOR_IOCTL_CMD_GET_ROOT \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 7, size_t)

#define PTEDITOR_IOCTL_CMD_SET_ROOT \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 8, size_t)

#define PTEDITOR_IOCTL_CMD_GET_PAGESIZE \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 9, size_t)

#define PTEDITOR_IOCTL_CMD_INVALIDATE_TLB \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 10, size_t)

#define PTEDITOR_IOCTL_CMD_GET_PAT \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 11, size_t)

#define PTEDITOR_IOCTL_CMD_SET_PAT \
  _IOR(PTEDITOR_IOCTL_MAGIC_NUMBER, 12, size_t)

  #endif // PTEDITOR_MODULE_H
/** @file */

#ifndef _PTEDITOR_H_
#define _PTEDITOR_H_


#include <sys/types.h>

/**
 * The implementation of PTEditor to use
 * 
 * @defgroup PTEDITOR_IMPLEMENTATION PTEditor Implementation
 * 
 * @{
 */

/** Use the kernel to resolve and update paging structures */
#define PTEDIT_IMPL_KERNEL 0
/** Use the user-space implemenation to resolve and update paging structures */
#define PTEDIT_IMPL_USER   1

/**
 * The bits in a page-table entry
 *
 * @defgroup PAGETABLE_BITS Page Table Bits
 *
 * @{
 *
 */

#if defined(__i386__) || defined(__x86_64__)

/** Page is present */
#define PTEDIT_PAGE_BIT_PRESENT 0
/** Page is writeable */
#define PTEDIT_PAGE_BIT_RW 1
/** Page is userspace addressable */
#define PTEDIT_PAGE_BIT_USER 2
/** Page write through */
#define PTEDIT_PAGE_BIT_PWT 3
/** Page cache disabled */
#define PTEDIT_PAGE_BIT_PCD 4
/** Page was accessed (raised by CPU) */
#define PTEDIT_PAGE_BIT_ACCESSED 5
/** Page was written to (raised by CPU) */
#define PTEDIT_PAGE_BIT_DIRTY 6
/** 4 MB (or 2MB) page */
#define PTEDIT_PAGE_BIT_PSE 7
/** PAT (only on 4KB pages) */
#define PTEDIT_PAGE_BIT_PAT 7
/** Global TLB entry PPro+ */
#define PTEDIT_PAGE_BIT_GLOBAL 8
/** Available for programmer */
#define PTEDIT_PAGE_BIT_SOFTW1 9
/** Available for programmer */
#define PTEDIT_PAGE_BIT_SOFTW2 10
/** Available for programmer */
#define PTEDIT_PAGE_BIT_SOFTW3 11
/** PAT (on 2MB or 1GB pages) */
#define PTEDIT_PAGE_BIT_PAT_LARGE 12
/** Available for programmer */
#define PTEDIT_PAGE_BIT_SOFTW4 58
/** Protection Keys, bit 1/4 */
#define PTEDIT_PAGE_BIT_PKEY_BIT0 59
/** Protection Keys, bit 2/4 */
#define PTEDIT_PAGE_BIT_PKEY_BIT1 60
/** Protection Keys, bit 3/4 */
#define PTEDIT_PAGE_BIT_PKEY_BIT2 61
/** Protection Keys, bit 4/4 */
#define PTEDIT_PAGE_BIT_PKEY_BIT3 62
/** No execute: only valid after cpuid check */
#define PTEDIT_PAGE_BIT_NX 63

#elif defined(__aarch64__)

/** Entry type 1/2 */
#define PTEDIT_PAGE_BIT_TYPE_BIT0 0
/** Entry type 1/2 */
#define PTEDIT_PAGE_BIT_TYPE_BIT1 1
/** Memory attribute index 1/3 */
#define PTEDIT_PAGE_BIT_MAIR_BIT0 2
/** Memory attribute index 2/3 */
#define PTEDIT_PAGE_BIT_MAIR_BIT1 3
/** Memory attribute index 3/3 */
#define PTEDIT_PAGE_BIT_MAIR_BIT2 4
/** Page is non-secure */
#define PTEDIT_PAGE_BIT_NON_SECURE 5
/** Page permissions 1/2 */
#define PTEDIT_PAGE_BIT_PERMISSION_BIT0 6
/** Page permissions 2/2 */
#define PTEDIT_PAGE_BIT_PERMISSION_BIT1 7
/** Shareability domain 1/2 */
#define PTEDIT_PAGE_BIT_SHARE_BIT0 8
/** Shareability domain 2/2 */
#define PTEDIT_PAGE_BIT_SHARE_BIT1 9
/** Page was accessed (raised by CPU) */
#define PTEDIT_PAGE_BIT_ACCESSED 10
/** Page is not global */
#define PTEDIT_PAGE_BIT_NOT_GLOBAL 11
/** Contiguous */
#define PTEDIT_PAGE_BIT_CONTIGUOUS 52
/** Privileged execute never */
#define PTEDIT_PAGE_BIT_PXN 53
/** Execute never */
#define PTEDIT_PAGE_BIT_XN 54
/** Available for programmer */
#define PTEDIT_PAGE_BIT_SOFTW1 55
/** Available for programmer */
#define PTEDIT_PAGE_BIT_SOFTW2 56
/** Available for programmer */
#define PTEDIT_PAGE_BIT_SOFTW3 57
/** Available for programmer */
#define PTEDIT_PAGE_BIT_SOFTW4 58
/** Available for programmer */
#define PTEDIT_PAGE_BIT_SOFTW5 59
/** Available for programmer */
#define PTEDIT_PAGE_BIT_SOFTW6 60
/** Available for programmer */
#define PTEDIT_PAGE_BIT_SOFTW7 61
/** Available for programmer */
#define PTEDIT_PAGE_BIT_SOFTW8 62
/** Available for programmer */
#define PTEDIT_PAGE_BIT_SOFTW9 63

#endif
/** @} */

/**
 * The memory types (PAT/MAIR)values
 *
 * @defgroup MEMORY_TYPES Memory Types (PAT/MAIR values)
 *
 * @{
 */
#if defined(__i386__) || defined(__x86_64__)

/** Strong uncachable (nothing is cached) */
#define PTEDIT_MT_UC      0
/** Write combining (consecuite writes are combined in a WC buffer and then written once) */
#define PTEDIT_MT_WC      1
/** Write through (read accesses are cached, write access are written to cache and memory) */
#define PTEDIT_MT_WT      4
/** Write protected (only read access is cached) */
#define PTEDIT_MT_WP      5
/** Write back (read and write accesses are cached) */
#define PTEDIT_MT_WB      6
/** Uncachable (as UC, but can be changed to WC through MTRRs) */
#define PTEDIT_MT_UCMINUS 7

#elif defined(__aarch64__)

/** Strong uncachable (nothing is cached) */
#define PTEDIT_MT_UC      0x44
/** Write through (read accesses are cached, write access are written to cache and memory) */
#define PTEDIT_MT_WT      0xbb
/** Write back (read and write accesses are cached) */
#define PTEDIT_MT_WB      0xff

#endif
/** @} */


/**
 * Basic functionality required in every program
 *
 * @defgroup BASIC Basic Functionality
 *
 * @{
 */

/**
 * Initializes (and acquires) PTEditor kernel module
 *
 * @return 0 Initialization was successful
 * @return -1 Initialization failed
 */
int ptedit_init();

/**
 * Releases PTEditor kernel module
 *
 */
void ptedit_cleanup();

/**
 * Switch between kernel and user-space implementation
 * 
 * @param[in] implementation The implementation to use, either PTEDIT_IMPL_KERNEL or PTEDIT_IMPL_USER
 * 
 */
void ptedit_use_implementation(int implementation);

/** @} */




/**
 * Functions to read and write page tables
 *
 * @defgroup PAGETABLE Page tables
 *
 * @{
 */

typedef ptedit_entry_t (*ptedit_resolve_t)(void*, pid_t);
typedef void (*ptedit_update_t)(void*, pid_t, ptedit_entry_t*);


/**
 * Resolves the page-table entries of all levels for a virtual address of a given process.
 *
 * @param[in] address The virtual address to resolve
 * @param[in] pid The pid of the process (0 for own process)
 *
 * @return A structure containing the page-table entries of all levels.
 */
ptedit_resolve_t ptedit_resolve;
// ptedit_entry_t ptedit_resolve(void* address, pid_t pid);

/**
 * Updates one or more page-table entries for a virtual address of a given process.
 * The TLB for the given address is flushed after updating the entries.
 *
 * @param[in] address The virtual address
 * @param[in] pid The pid of the process (0 for own process)
 * @param[in] vm A structure containing the values for the page-table entries and a bitmask indicating which entries to update
 *
 */
ptedit_update_t ptedit_update;

// void ptedit_update(void* address, pid_t pid, ptedit_entry_t* vm);

/**
 * Sets a bit directly in the PTE of an address.
 *
 * @param[in] address The virtual address
 * @param[in] pid The pid of the process (0 for own process)
 * @param[in] bit The bit to set (one of PTEDIT_PAGE_BIT_*)
 *
 */
void ptedit_pte_set_bit(void* address, pid_t pid, int bit);

/**
 * Clears a bit directly in the PTE of an address.
 *
 * @param[in] address The virtual address
 * @param[in] pid The pid of the process (0 for own process)
 * @param[in] bit The bit to clear (one of PTEDIT_PAGE_BIT_*)
 *
 */
void ptedit_pte_clear_bit(void* address, pid_t pid, int bit);

/**
 * Returns the value of a bit directly from the PTE of an address.
 *
 * @param[in] address The virtual address
 * @param[in] pid The pid of the process (0 for own process)
 * @param[in] bit The bit to get (one of PTEDIT_PAGE_BIT_*)
 *
 * @return The value of the bit (0 or 1)
 *
 */
unsigned char ptedit_pte_get_bit(void* address, pid_t pid, int bit);

/**
 * Reads the PFN directly from the PTE of an address.
 *
 * @param[in] address The virtual address
 * @param[in] pid The pid of the process (0 for own process)
 *
 * @return The page-frame number (PFN)
 *
 */
size_t ptedit_pte_get_pfn(void* address, pid_t pid);

/**
 * Sets the PFN directly in the PTE of an address.
 *
 * @param[in] address The virtual address
 * @param[in] pid The pid of the process (0 for own process)
 * @param[in] pfn The new page-frame number (PFN)
 *
 */
void ptedit_pte_set_pfn(void* address, pid_t pid, size_t pfn);


#if defined(__i386__) || defined(__x86_64__)
#define PTEDIT_PAGE_PRESENT 1

typedef struct {
  size_t present                   :1;
  size_t writeable                 :1;
  size_t user_access               :1;
  size_t write_through             :1;
  size_t cache_disabled            :1;
  size_t accessed                  :1;
  size_t ignored_3                 :1;
  size_t size                      :1;
  size_t ignored_2                 :4;
  size_t pfn                       :28;
  size_t reserved_1                :12;
  size_t ignored_1                 :11;
  size_t execution_disabled        :1;
} __attribute__((__packed__)) ptedit_pgd_t;

typedef struct {
  size_t present                   :1;
  size_t writeable                 :1;
  size_t user_access               :1;
  size_t write_through             :1;
  size_t cache_disabled            :1;
  size_t accessed                  :1;
  size_t ignored_3                 :1;
  size_t size                      :1;
  size_t ignored_2                 :4;
  size_t pfn                       :28;
  size_t reserved_1                :12;
  size_t ignored_1                 :11;
  size_t execution_disabled        :1;
} __attribute__((__packed__)) ptedit_p4d_t;

typedef struct {
  size_t present                   :1;
  size_t writeable                 :1;
  size_t user_access               :1;
  size_t write_through             :1;
  size_t cache_disabled            :1;
  size_t accessed                  :1;
  size_t ignored_3                 :1;
  size_t size                      :1;
  size_t ignored_2                 :4;
  size_t pfn                       :28;
  size_t reserved_1                :12;
  size_t ignored_1                 :11;
  size_t execution_disabled        :1;
} __attribute__((__packed__)) ptedit_pud_t;


typedef struct {
  size_t present                   :1;
  size_t writeable                 :1;
  size_t user_access               :1;
  size_t write_through             :1;
  size_t cache_disabled            :1;
  size_t accessed                  :1;
  size_t ignored_3                 :1;
  size_t size                      :1;
  size_t ignored_2                 :4;
  size_t pfn                       :28;
  size_t reserved_1                :12;
  size_t ignored_1                 :11;
  size_t execution_disabled        :1;
} __attribute__((__packed__)) ptedit_pmd_t;

typedef struct {
  size_t present                   :1;
  size_t writeable                 :1;
  size_t user_access               :1;
  size_t write_through             :1;
  size_t cache_disabled            :1;
  size_t accessed                  :1;
  size_t dirty                     :1;
  size_t size                      :1;
  size_t global                    :1;
  size_t ignored_2                 :3;
  size_t pat                       :1;
  size_t reserved_2                :8;
  size_t pfn                  :19;
  size_t reserved_1                :12;
  size_t ignored_1                 :11;
  size_t execution_disabled        :1;
} __attribute__((__packed__)) ptedit_pmd_large_t;


typedef struct {
  size_t present                   :1;
  size_t writeable                 :1;
  size_t user_access               :1;
  size_t write_through             :1;
  size_t cache_disabled            :1;
  size_t accessed                  :1;
  size_t dirty                     :1;
  size_t size                      :1;
  size_t global                    :1;
  size_t ignored_2                 :3;
  size_t pfn                  :28;
  size_t reserved_1                :12;
  size_t ignored_1                 :11;
  size_t execution_disabled        :1;
} __attribute__((__packed__)) ptedit_pte_t;

#elif defined(__aarch64__)
#define PTEDIT_PAGE_PRESENT 3


typedef struct {
    size_t present                 :2;
    size_t ignored_1               :10;
    size_t table_address           :36;
    size_t reserved                :4;
    size_t ignored_2               :7;
    size_t pxn_table               :1;
    size_t xn_table                :1;
    size_t ap_table                :2;
    size_t ns_table                :1;
}__attribute__((__packed__)) ptedit_pgd_t;

typedef ptedit_pgd_t ptedit_p4d_t;
typedef ptedit_pgd_t ptedit_pud_t;
typedef ptedit_pgd_t ptedit_pmd_t;

typedef struct {
    size_t present                 :2;
    size_t memory_attributes_index :3;
    size_t non_secure              :1;
    size_t access_permissions      :2;
    size_t shareability_field      :2;
    size_t access_flag             :1;
    size_t not_global              :1;
    size_t reserved_1              :18;
    size_t pfn                     :18;
    size_t reserved_2              :4;
    size_t contiguous              :1;
    size_t privileged_execute_never:1;
    size_t execute_never           :1;
    size_t ingored_1               :4;
    size_t ignored_2               :5;
}__attribute__((__packed__)) ptedit_pgd_large_t;

typedef struct {
    size_t present                 :2;
    size_t memory_attributes_index :3;
    size_t non_secure              :1;
    size_t access_permissions      :2;
    size_t shareability_field      :2;
    size_t access_flag             :1;
    size_t not_global              :1;
    size_t reserved_1              :9;
    size_t pfn                     :27;
    size_t reserved_2              :4;
    size_t contiguous              :1;
    size_t privileged_execute_never:1;
    size_t execute_never           :1;
    size_t ingored_1               :4;
    size_t ignored_2               :5;
}__attribute__((__packed__)) ptedit_pmd_large_t;


typedef struct {
    size_t present                 :2;
    size_t memory_attributes_index :3;
    size_t non_secure              :1;
    size_t access_permissions      :2;
    size_t shareability_field      :2;
    size_t access_flag             :1;
    size_t not_global              :1;
    size_t pfn                     :36;
    size_t reserved_1              :4;
    size_t contiguous              :1;
    size_t privileged_execute_never:1;
    size_t execute_never           :1;
    size_t ingored_1               :4;
    size_t ignored_2               :5;
}__attribute__((__packed__)) ptedit_pte_t;
#endif


#define ptedit_cast(v, type) (*((type*)&(v)))

/** @} */



/**
 * General system info
 *
 * @defgroup SYSTEMINFO System info
 *
 * @{
 */

/**
 * Returns the default page size of the system
 *
 * @return Page size of the system in bytes
 */
int ptedit_get_pagesize();

/** @} */



/**
 * Get and set page frame numbers
 *
 * @defgroup PFN Page frame numbers (PFN)
 *
 * @{
 */

/**
 * Returns a new page-table entry where the page-frame number (PFN) is replaced by the specified one.
 *
 * @param[in] entry The page-table entry to modify
 * @param[in] pfn The new page-frame number (PFN)
 *
 * @return A new page-table entry with the given page-frame number
 */
size_t ptedit_set_pfn(size_t entry, size_t pfn);

/**
 * Returns the page-frame number (PFN) of a page-table entry.
 *
 * @param[in] entry The page-table entry to extract the PFN from
 *
 * @return The page-frame number
 */
size_t ptedit_get_pfn(size_t entry);

/** @} */




/**
 * Reading and writing of physical pages
 *
 * @defgroup PHYSICALPAGE Physical pages
 *
 * @{
 */

/**
 * Retrieves the content of a physical page.
 *
 * @param[in] pfn The page-frame number (PFN) of the page to read
 * @param[out] buffer A buffer which is large enough to hold the content of the page
 *
 */
void ptedit_read_physical_page(size_t pfn, char* buffer);

/**
 * Replaces the content of a physical page.
 *
 * @param[in] pfn The page-frame number (PFN) of the page to update
 * @param[in] content A buffer containing the new content of the page (must be the size of a physical page)
 *
 */
void ptedit_write_physical_page(size_t pfn, char* content);

/** @} */




/**
 * Read and modify the root of paging structure
 *
 * @defgroup PAGING Paging
 *
 * @{
 */

/**
 * Returns the root of the paging structure (i.e., CR3 on x86 and TTBR0 on ARM).
 *
 * @param[in] pid The proccess id (0 for own process)
 *
 * @return The phyiscal address (not PFN!) of the first page table (i.e., the PGD)
 *
 */
size_t ptedit_get_paging_root(pid_t pid);

/**
 * Sets the root of the paging structure (i.e., CR3 on x86 and TTBR0 on ARM).
 *
 * @param[in] pid The proccess id (0 for own process)
 * @param[in] root The physical address (not PFN!) of the first page table (i.e., the PGD)
 *
 */
void ptedit_set_paging_root(pid_t pid, size_t root);

/** @} */


/**
 * Invalidations and barriers
 *
 * @defgroup BARRIERS TLB/Barriers
 *
 * @{
 */

/**
 * Invalidates the TLB for a given address on all CPUs.
 *
 * @param[in] address The address to invalidate
 *
 */
void ptedit_invalidate_tlb(void* address);


/**
 * A full serializing barrier which stops everything.
 *
 */
void ptedit_full_serializing_barrier();

/** @} */



/**
 * Memory types (x86 PATs / ARM MAIR)
 *
 * @defgroup MTS Memory types (PATs / MAIR)
 *
 * @{
 */

/**
 * Reads the value of all memory types (x86 PATs / ARM MAIRs). This is equivalent to reading the MSR 0x277 (x86) / MAIR_EL1 (ARM).
 *
 * @return The memory types in the same format as in the IA32_PAT MSR / MAIR_EL1
 *
 */
size_t ptedit_get_mts();

/**
 * Programs the value of all memory types (x86 PATs / ARM MAIRs). This is equivalent to writing to the MSR 0x277 (x86) / MAIR_EL1 (ARM) on all CPUs.
 *
 * @param[in] mts The memory types in the same format as in the IA32_PAT MSR / MAIR_EL1
 *
 */
void ptedit_set_mts(size_t mts);

/**
 * Reads the value of a specific memory type attribute (PAT/MAIR).
 *
 * @param[in] mt The PAT/MAIR ID (from 0 to 7)
 *
 * @return The PAT/MAIR value (can be one of PTEDIT_MT_*)
 *
 */
char ptedit_get_mt(unsigned char mt);

/**
 * Programs the value of a specific memory type attribute (PAT/MAIR).
 *
 * @param[in] mt The PAT/MAIR ID (from 0 to 7)
 * @param[in] value The PAT/MAIR value (can be one of PTEDIT_MT_*)
 *
 */
void ptedit_set_mt(unsigned char mt, unsigned char value);

/**
 * Generates a bitmask of all memory type attributes (PAT/MAIR) which are programmed to the given value.
 *
 * @param[in] type A memory type, i.e., PAT/MAIR value (one of PTEDIT_MT_*)
 *
 * @return A bitmask where a set bit indicates that the corresponding PAT/MAIR has the given type
 *
 */
unsigned char ptedit_find_mt(unsigned char type);

/**
 * Returns the first memory type attribute (PAT/MAIR) which is programmed to the given memory type.
 *
 * @param[in] type A memory type, i.e., PAT/MAIR value (one of PTEDIT_MT_*)
 *
 * @return A PAT/MAIR ID, or -1 if no PAT/MAIR of this type was found
 *
 */
int ptedit_find_first_mt(unsigned char type);

/**
 * Returns a new page-table entry which uses the given memory type (PAT/MAIR).
 *
 * @param[in] entry A page-table entry
 * @param[in] mt A PAT/MAIR ID (between 0 and 7)
 *
 * @return A new page-table entry with the given memory type (PAT/MAIR)
 *
 */
size_t ptedit_apply_mt(size_t entry, unsigned char mt);

/**
 * Returns the memory type (i.e., PAT/MAIR ID) which is used by a page-table entry.
 *
 * @param[in] entry A page-table entry
 *
 * @return A PAT/MAIR ID (between 0 and 7)
 *
 */
unsigned char ptedit_extract_mt(size_t entry);

/**
 * Returns a human-readable representation of a memory type (PAT/MAIR value).
 *
 * @param[in] mt A memory type (PAT/MAIR value, e.g., one of PTEDIT_MT_*)
 *
 * @return A human-readable representation of the memory type
 *
 */
const char* ptedit_mt_to_string(unsigned char mt);

/** @} */



/**
 * Pretty print
 *
 * @defgroup PRETTYPRINT Pretty print
 *
 * @{
 */

/**
 * Pretty prints a ptedit_entry_t struct.
 *
 * @param[in] entry A ptedit_entry_t struct
 *
 */
void ptedit_print_entry_t(ptedit_entry_t entry);

/**
 * Pretty prints a page-table entry.
 *
 * @param[in] entry A page-table entry
 *
 */
void ptedit_print_entry(size_t entry);

/**
 * Prints a single line of the pretty-print representation of a page-table entry.
 *
 * @param[in] entry A page-table entry
 * @param[in] line The line to print (0 to 3)
 *
 */
void ptedit_print_entry_line(size_t entry, int line);

/** @} */

#endif

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>



#define PTEDIT_COLOR_RED     "\x1b[31m"
#define PTEDIT_COLOR_GREEN   "\x1b[32m"
#define PTEDIT_COLOR_RESET   "\x1b[0m"

static int ptedit_fd;
static int ptedit_umem;
static int ptedit_pagesize;
static size_t ptedit_paging_root;

typedef struct {
    int has_pgd, has_p4d, has_pud, has_pmd, has_pt;
    int pgd_entries, p4d_entries, pud_entries, pmd_entries, pt_entries;
    int page_offset;
} ptedit_paging_definition_t;

ptedit_paging_definition_t ptedit_paging_definition;



// ---------------------------------------------------------------------------
ptedit_entry_t ptedit_resolve_kernel(void* address, pid_t pid) {
    ptedit_entry_t vm;
    vm.vaddr = (size_t)address;
    vm.pid = (size_t)pid;
    ioctl(ptedit_fd, PTEDITOR_IOCTL_CMD_VM_RESOLVE, (size_t)&vm);
    return vm;
}

// ---------------------------------------------------------------------------
ptedit_entry_t ptedit_resolve_user(void* address, pid_t pid) {
    size_t pgd[ptedit_pagesize / sizeof(size_t)], p4d[ptedit_pagesize / sizeof(size_t)], 
        pud[ptedit_pagesize / sizeof(size_t)], pmd[ptedit_pagesize / sizeof(size_t)], 
        pt[ptedit_pagesize / sizeof(size_t)];

    size_t root = (pid == 0) ? ptedit_paging_root : ptedit_get_paging_root(pid);
    ptedit_read_physical_page(root / ptedit_pagesize, (char *)pgd);

    int pgdi, p4di, pudi, pmdi, pti;
    size_t addr = (size_t)address;
    pgdi = (addr >> (ptedit_paging_definition.page_offset 
                    + ptedit_paging_definition.pt_entries 
                    + ptedit_paging_definition.pmd_entries 
                    + ptedit_paging_definition.pud_entries 
                    + ptedit_paging_definition.p4d_entries)) % (1 << ptedit_paging_definition.pgd_entries);
    p4di = (addr >> (ptedit_paging_definition.page_offset 
                    + ptedit_paging_definition.pt_entries 
                    + ptedit_paging_definition.pmd_entries 
                    + ptedit_paging_definition.pud_entries)) % (1 << ptedit_paging_definition.p4d_entries);
    pudi = (addr >> (ptedit_paging_definition.page_offset 
                    + ptedit_paging_definition.pt_entries 
                    + ptedit_paging_definition.pmd_entries)) % (1 << ptedit_paging_definition.pud_entries);
    pmdi = (addr >> (ptedit_paging_definition.page_offset 
                    + ptedit_paging_definition.pt_entries)) % (1 << ptedit_paging_definition.pmd_entries);
    pti = (addr >> ptedit_paging_definition.page_offset) % (1 << ptedit_paging_definition.pt_entries);
    
    ptedit_entry_t resolved;
    memset(&resolved, 0, sizeof(resolved));
    resolved.vaddr = (size_t)address;
    resolved.pid = (size_t)pid;
    resolved.valid = 0;
    
    size_t pgd_entry, p4d_entry, pud_entry, pmd_entry, pt_entry;
    
    pgd_entry = pgd[pgdi];
    if(ptedit_cast(pgd_entry, ptedit_pgd_t).present != PTEDIT_PAGE_PRESENT) {
        return resolved;
    }
    resolved.pgd = pgd_entry;
    resolved.valid |= PTEDIT_VALID_MASK_PGD;
    if(ptedit_paging_definition.has_p4d) {    
        ptedit_read_physical_page(ptedit_get_pfn(pgd_entry), (char *)p4d);
        p4d_entry = p4d[p4di];
        if(ptedit_cast(p4d_entry, ptedit_p4d_t).present != PTEDIT_PAGE_PRESENT) {
            return resolved;
        }
    } else {
        p4d_entry = pgd_entry;
    }
    resolved.p4d = p4d_entry;
    resolved.valid |= PTEDIT_VALID_MASK_P4D;

    if(ptedit_paging_definition.has_pud) {
        ptedit_read_physical_page(ptedit_get_pfn(p4d_entry), (char *)pud);
        pud_entry = pud[pudi];
        if(ptedit_cast(pud_entry, ptedit_pud_t).present != PTEDIT_PAGE_PRESENT) {
            return resolved;
        }
    } else {
        pud_entry = p4d_entry;
    }
    resolved.pud = pud_entry;
    resolved.valid |= PTEDIT_VALID_MASK_PUD;
    
    if(ptedit_paging_definition.has_pmd) {
        ptedit_read_physical_page(ptedit_get_pfn(pud_entry), (char *)pmd);
        pmd_entry = pmd[pmdi];
        if(ptedit_cast(pmd_entry, ptedit_pmd_t).present != PTEDIT_PAGE_PRESENT) {
            return resolved;
        }
    } else {
        pmd_entry = pud_entry;
    }
    resolved.pmd = pmd_entry;
    resolved.valid |= PTEDIT_VALID_MASK_PMD;
    
#if defined(__i386__) || defined(__x86_64__)
    if(!ptedit_cast(pmd_entry, ptedit_pmd_t).size) {
#endif
        // normal 4kb page
        ptedit_read_physical_page(ptedit_get_pfn(pmd_entry), (char *)pt);
        pt_entry = pt[pti];
        if(ptedit_cast(pt_entry, ptedit_pte_t).present != PTEDIT_PAGE_PRESENT) {
            return resolved;
        }
        resolved.pte = pt_entry;
        resolved.valid |= PTEDIT_VALID_MASK_PTE;
#if defined(__i386__) || defined(__x86_64__)
    }
#endif
    return resolved;
}

// ---------------------------------------------------------------------------
void ptedit_update_kernel(void* address, pid_t pid, ptedit_entry_t* vm) {
    vm->vaddr = (size_t)address;
    vm->pid = (size_t)pid;
    ioctl(ptedit_fd, PTEDITOR_IOCTL_CMD_VM_UPDATE, (size_t)vm);
}

// ---------------------------------------------------------------------------
void ptedit_update_user(void* address, pid_t pid, ptedit_entry_t* vm) {
    size_t pgd[ptedit_pagesize / sizeof(size_t)], p4d[ptedit_pagesize / sizeof(size_t)], 
        pud[ptedit_pagesize / sizeof(size_t)], pmd[ptedit_pagesize / sizeof(size_t)], 
        pt[ptedit_pagesize / sizeof(size_t)];

    size_t root = (pid == 0) ? ptedit_paging_root : ptedit_get_paging_root(pid);
    ptedit_read_physical_page(root / ptedit_pagesize, (char *)pgd);

    int pgdi, p4di, pudi, pmdi, pti;
    size_t addr = (size_t)address;
    pgdi = (addr >> (ptedit_paging_definition.page_offset 
                    + ptedit_paging_definition.pt_entries 
                    + ptedit_paging_definition.pmd_entries 
                    + ptedit_paging_definition.pud_entries 
                    + ptedit_paging_definition.p4d_entries)) % (1 << ptedit_paging_definition.pgd_entries);
    p4di = (addr >> (ptedit_paging_definition.page_offset 
                    + ptedit_paging_definition.pt_entries 
                    + ptedit_paging_definition.pmd_entries 
                    + ptedit_paging_definition.pud_entries)) % (1 << ptedit_paging_definition.p4d_entries);
    pudi = (addr >> (ptedit_paging_definition.page_offset 
                    + ptedit_paging_definition.pt_entries 
                    + ptedit_paging_definition.pmd_entries)) % (1 << ptedit_paging_definition.pud_entries);
    pmdi = (addr >> (ptedit_paging_definition.page_offset 
                    + ptedit_paging_definition.pt_entries)) % (1 << ptedit_paging_definition.pmd_entries);
    pti = (addr >> ptedit_paging_definition.page_offset) % (1 << ptedit_paging_definition.pt_entries);
    
    size_t pgd_entry, p4d_entry, pud_entry, pmd_entry, pt_entry;
    int valid = 0;
    
    pgd_entry = pgd[pgdi];
    if(vm->valid & PTEDIT_VALID_MASK_PGD) pgd[pgdi] = vm->pgd;
    if(ptedit_cast(pgd_entry, ptedit_pgd_t).present != PTEDIT_PAGE_PRESENT) {
        goto update;
    }
    valid |= PTEDIT_VALID_MASK_PGD;
    
    if(ptedit_paging_definition.has_p4d) {    
        ptedit_read_physical_page(ptedit_get_pfn(pgd_entry), (char *)p4d);
        p4d_entry = p4d[p4di];
        if(vm->valid & PTEDIT_VALID_MASK_P4D) p4d[p4di] = vm->p4d;
        if(ptedit_cast(p4d_entry, ptedit_p4d_t).present != PTEDIT_PAGE_PRESENT) {
            goto update;
        }
    } else {
        p4d_entry = pgd_entry;
    }
    valid |= PTEDIT_VALID_MASK_P4D;

    if(ptedit_paging_definition.has_pud) {
        ptedit_read_physical_page(ptedit_get_pfn(p4d_entry), (char *)pud);
        pud_entry = pud[pudi];
        if(vm->valid & PTEDIT_VALID_MASK_PUD) pud[pudi] = vm->pud;
        if(ptedit_cast(pud_entry, ptedit_pud_t).present != PTEDIT_PAGE_PRESENT) {
            goto update;
        }
    } else {
        pud_entry = p4d_entry;
    }
    valid |= PTEDIT_VALID_MASK_PUD;
    
    if(ptedit_paging_definition.has_pmd) {
        ptedit_read_physical_page(ptedit_get_pfn(pud_entry), (char *)pmd);
        pmd_entry = pmd[pmdi];
        if(vm->valid & PTEDIT_VALID_MASK_PMD) pmd[pmdi] = vm->pmd;
        if(ptedit_cast(pmd_entry, ptedit_pmd_t).present != PTEDIT_PAGE_PRESENT) {
            goto update;
        }
    } else {
        pmd_entry = pud_entry;
    }
    valid |= PTEDIT_VALID_MASK_PMD;
    
#if defined(__i386__) || defined(__x86_64__)
    if(!ptedit_cast(pmd_entry, ptedit_pmd_t).size) {
#endif
        // normal 4kb page
        ptedit_read_physical_page(ptedit_get_pfn(pmd_entry), (char *)pt);
        pt_entry = pt[pti];
        if(vm->valid & PTEDIT_VALID_MASK_PTE) pt[pti] = vm->pte;
        if(ptedit_cast(pt_entry, ptedit_pte_t).present != PTEDIT_PAGE_PRESENT) {
            goto update;
        }
        valid |= PTEDIT_VALID_MASK_PTE;
#if defined(__i386__) || defined(__x86_64__)
    }
#endif
    
    update:
    if((vm->valid & PTEDIT_VALID_MASK_PTE) && (valid & PTEDIT_VALID_MASK_PTE)) {
        ptedit_write_physical_page(ptedit_get_pfn(pmd_entry), (char*)pt);
    }
    if((vm->valid & PTEDIT_VALID_MASK_PMD) && (valid & PTEDIT_VALID_MASK_PMD) && ptedit_paging_definition.has_pmd) {
        ptedit_write_physical_page(ptedit_get_pfn(pud_entry), (char*)pmd);
    }
    if((vm->valid & PTEDIT_VALID_MASK_PUD) && (valid & PTEDIT_VALID_MASK_PUD) && ptedit_paging_definition.has_pud) {
        ptedit_write_physical_page(ptedit_get_pfn(p4d_entry), (char*)pud);
    }
    if((vm->valid & PTEDIT_VALID_MASK_P4D) && (valid & PTEDIT_VALID_MASK_P4D) && ptedit_paging_definition.has_p4d) {
        ptedit_write_physical_page(ptedit_get_pfn(pgd_entry), (char*)p4d);
    }
    if((vm->valid & PTEDIT_VALID_MASK_PGD) && (valid & PTEDIT_VALID_MASK_PGD) && ptedit_paging_definition.has_pgd) {
        ptedit_write_physical_page(root / ptedit_pagesize, (char*)pgd);
    }
    
    ptedit_invalidate_tlb(address);
}

// ---------------------------------------------------------------------------
size_t ptedit_set_pfn(size_t pte, size_t pfn) {
#if defined(__i386__) || defined(__x86_64__)
  pte &= ~(((1ull << 40) - 1) << 12);
#elif defined(__aarch64__)
  pte &= ~(((1ull << 36) - 1) << 12);
#endif
  pte |= pfn << 12;
  return pte;
}


// ---------------------------------------------------------------------------
size_t ptedit_get_pfn(size_t pte) {
#if defined(__i386__) || defined(__x86_64__)
    return (pte & (((1ull << 40) - 1) << 12)) >> 12;
#elif defined(__aarch64__)
    return (pte & (((1ull << 36) - 1) << 12)) >> 12;
#endif
}


// ---------------------------------------------------------------------------
#define PTEDIT_B(val, bit) (!!((val) & (1ull << (bit))))

#define PEDIT_PRINT_B(fmt, bit)                                                \
  if ((bit)) {                                                                 \
    printf(PTEDIT_COLOR_GREEN);                                                       \
    printf((fmt), (bit));                                                      \
    printf(PTEDIT_COLOR_RESET);                                                       \
  } else {                                                                     \
    printf((fmt), (bit));                                                      \
  }                                                                            \
  printf("|");


// ---------------------------------------------------------------------------
void ptedit_print_entry_line(size_t entry, int line) {
#if defined(__i386__) || defined(__x86_64__)
  if(line == 0 || line == 3) printf("+--+------------------+-+-+-+-+-+-+-+-+--+--+-+-+-+\n");
  if(line == 1) printf("|NX|       PFN        |H|?|?|?|G|S|D|A|UC|WT|U|W|P|\n");
  if(line == 2) {
      printf("|");
      PEDIT_PRINT_B(" %d", PTEDIT_B(entry, PTEDIT_PAGE_BIT_NX));
      printf(" %16p |", (void *)((entry >> 12) & ((1ull << 40) - 1)));
      PEDIT_PRINT_B("%d", PTEDIT_B(entry, PTEDIT_PAGE_BIT_PAT_LARGE));
      PEDIT_PRINT_B("%d", PTEDIT_B(entry, PTEDIT_PAGE_BIT_SOFTW3));
      PEDIT_PRINT_B("%d", PTEDIT_B(entry, PTEDIT_PAGE_BIT_SOFTW2));
      PEDIT_PRINT_B("%d", PTEDIT_B(entry, PTEDIT_PAGE_BIT_SOFTW1));
      PEDIT_PRINT_B("%d", PTEDIT_B(entry, PTEDIT_PAGE_BIT_GLOBAL));
      PEDIT_PRINT_B("%d", PTEDIT_B(entry, PTEDIT_PAGE_BIT_PSE));
      PEDIT_PRINT_B("%d", PTEDIT_B(entry, PTEDIT_PAGE_BIT_DIRTY));
      PEDIT_PRINT_B("%d", PTEDIT_B(entry, PTEDIT_PAGE_BIT_ACCESSED));
      PEDIT_PRINT_B(" %d", PTEDIT_B(entry, PTEDIT_PAGE_BIT_PCD));
      PEDIT_PRINT_B(" %d", PTEDIT_B(entry, PTEDIT_PAGE_BIT_PWT));
      PEDIT_PRINT_B("%d", PTEDIT_B(entry, PTEDIT_PAGE_BIT_USER));
      PEDIT_PRINT_B("%d", PTEDIT_B(entry, PTEDIT_PAGE_BIT_RW));
      PEDIT_PRINT_B("%d", PTEDIT_B(entry, PTEDIT_PAGE_BIT_PRESENT));
      printf("\n");
  }
#elif defined(__aarch64__)
  if(line == 0 || line == 3) {
      printf("+--+--+--+---+-+--+------------------+--+-+-+-+--+---+-+\n");
  }
  if(line == 1) {
    printf("| ?| ?|XN|PXN|C| ?|        PFN       |NG|A|S|P|NS|MAI|T|\n");
  }
  if(line == 2) {
      printf("|");
      PEDIT_PRINT_B("%2d", (PTEDIT_B(entry, 63) << 4) | (PTEDIT_B(entry, 62) << 3) | (PTEDIT_B(entry, 61) << 2) | (PTEDIT_B(entry, 60) << 1) | PTEDIT_B(entry, 59));
      PEDIT_PRINT_B("%2d", (PTEDIT_B(entry, 58) << 3) | (PTEDIT_B(entry, 57) << 2) | (PTEDIT_B(entry, 56) << 1) | PTEDIT_B(entry, 55));
      PEDIT_PRINT_B(" %d", PTEDIT_B(entry, 54));
      PEDIT_PRINT_B(" %d ", PTEDIT_B(entry, 53));
      PEDIT_PRINT_B("%d", PTEDIT_B(entry, 52));
      PEDIT_PRINT_B("%2d", (PTEDIT_B(entry, 51) << 3) | (PTEDIT_B(entry, 50) << 2) | (PTEDIT_B(entry, 49) << 1) | PTEDIT_B(entry, 48));
      printf(" %16p |", (void *)((entry >> 12) & ((1ull << 36) - 1)));
      PEDIT_PRINT_B(" %d", PTEDIT_B(entry, 11));
      PEDIT_PRINT_B("%d", PTEDIT_B(entry, 10));
      PEDIT_PRINT_B("%d", (PTEDIT_B(entry, 9) << 1) | PTEDIT_B(entry, 8));
      PEDIT_PRINT_B("%d", (PTEDIT_B(entry, 7) << 1) | PTEDIT_B(entry, 6));
      PEDIT_PRINT_B(" %d", PTEDIT_B(entry, 5));
      PEDIT_PRINT_B(" %d ", (PTEDIT_B(entry, 4) << 2) | (PTEDIT_B(entry, 3) << 1) | PTEDIT_B(entry, 2));
      PEDIT_PRINT_B("%d", (PTEDIT_B(entry, 1) << 1) | PTEDIT_B(entry, 0));
      printf("\n");
  }
#endif
}


// ---------------------------------------------------------------------------
void ptedit_print_entry(size_t entry) {
    for(int i = 0; i < 4; i++) {
        ptedit_print_entry_line(entry, i);
    }
}

// ---------------------------------------------------------------------------
void ptedit_print_entry_t(ptedit_entry_t entry) {
  if(entry.valid & PTEDIT_VALID_MASK_PGD) {
    printf("PGD of address\n");
    ptedit_print_entry(entry.pgd);
  }
  if(entry.valid & PTEDIT_VALID_MASK_P4D) {
    printf("P4D of address\n");
    ptedit_print_entry(entry.p4d);
  }
  if(entry.valid & PTEDIT_VALID_MASK_PUD) {
    printf("PUD of address\n");
    ptedit_print_entry(entry.pud);
  }
  if(entry.valid & PTEDIT_VALID_MASK_PMD) {
    printf("PMD of address\n");
    ptedit_print_entry(entry.pmd);
  }
  if(entry.valid & PTEDIT_VALID_MASK_PTE) {
    printf("PTE of address\n");
    ptedit_print_entry(entry.pte);
  }
}

// ---------------------------------------------------------------------------
int ptedit_init() {
  ptedit_fd = open(PTEDITOR_DEVICE_PATH, O_RDONLY);
  if (ptedit_fd < 0) {
    fprintf(stderr, PTEDIT_COLOR_RED "[-]" PTEDIT_COLOR_RESET "Error: Could not open PTEditor device: %s\n", PTEDITOR_DEVICE_PATH);
    return -1;
  }
  ptedit_umem = open("/proc/umem", O_RDWR);
  if(ptedit_umem > 0) {
      ptedit_use_implementation(PTEDIT_IMPL_USER);
  } else {
      ptedit_use_implementation(PTEDIT_IMPL_KERNEL);
  }
  ptedit_pagesize = getpagesize();
#if defined(__i386__) || defined(__x86_64__)
    ptedit_paging_definition.has_pgd = 1;
    ptedit_paging_definition.has_p4d = 0;
    ptedit_paging_definition.has_pud = 1;
    ptedit_paging_definition.has_pmd = 1;
    ptedit_paging_definition.has_pt = 1;
    ptedit_paging_definition.pgd_entries = 9;
    ptedit_paging_definition.p4d_entries = 0;
    ptedit_paging_definition.pud_entries = 9;
    ptedit_paging_definition.pmd_entries = 9;
    ptedit_paging_definition.pt_entries = 9;
    ptedit_paging_definition.page_offset = 12;
    #elif defined(__aarch64__)
    ptedit_paging_definition.has_pgd = 1;
    ptedit_paging_definition.has_p4d = 0;
    ptedit_paging_definition.has_pud = 0;
    ptedit_paging_definition.has_pmd = 1;
    ptedit_paging_definition.has_pt = 1;
    ptedit_paging_definition.pgd_entries = 9;
    ptedit_paging_definition.p4d_entries = 0;
    ptedit_paging_definition.pud_entries = 0;
    ptedit_paging_definition.pmd_entries = 9;
    ptedit_paging_definition.pt_entries = 9;
    ptedit_paging_definition.page_offset = 12;
#endif
  return 0;
}


// ---------------------------------------------------------------------------
void ptedit_cleanup() {
   if(ptedit_fd >= 0) {
       close(ptedit_fd);
   }
   if(ptedit_umem > 0) {
       close(ptedit_umem);
   }
}


// ---------------------------------------------------------------------------
void ptedit_use_implementation(int implementation) {
    if(implementation == PTEDIT_IMPL_KERNEL) {
        ptedit_resolve = ptedit_resolve_kernel;
        ptedit_update = ptedit_update_kernel;
    } else if(implementation == PTEDIT_IMPL_USER) {
        ptedit_resolve = ptedit_resolve_user;
        ptedit_update = ptedit_update_user;
        ptedit_paging_root = ptedit_get_paging_root(0);
    } else {
        fprintf(stderr, PTEDIT_COLOR_RED "[-]" PTEDIT_COLOR_RESET "Error: PTEditor implementation not supported!\n");
    }
}


// ---------------------------------------------------------------------------
int ptedit_get_pagesize() {
  return (int)ioctl(ptedit_fd, PTEDITOR_IOCTL_CMD_GET_PAGESIZE, 0);
}


// ---------------------------------------------------------------------------
void ptedit_read_physical_page(size_t pfn, char* buffer) {
    if(ptedit_umem > 0) {
        pread(ptedit_umem, buffer, ptedit_pagesize, pfn * ptedit_pagesize);
    } else {
        ptedit_page_t page;
        page.buffer = (unsigned char*)buffer;
        page.pfn = pfn;
        ioctl(ptedit_fd, PTEDITOR_IOCTL_CMD_READ_PAGE, (size_t)&page);
    }
}


// ---------------------------------------------------------------------------
void ptedit_write_physical_page(size_t pfn, char* content) {
    if(ptedit_umem > 0) {
        pwrite(ptedit_umem, content, ptedit_pagesize, pfn * ptedit_pagesize);
    } else {
        ptedit_page_t page;
        page.buffer = (unsigned char*)content;
        page.pfn = pfn;
        ioctl(ptedit_fd, PTEDITOR_IOCTL_CMD_WRITE_PAGE, (size_t)&page);
    }
}


// ---------------------------------------------------------------------------
size_t ptedit_get_paging_root(pid_t pid) {
  ptedit_paging_t cr3;
  cr3.pid = (size_t)pid;
  ioctl(ptedit_fd, PTEDITOR_IOCTL_CMD_GET_ROOT, (size_t)&cr3);
  return cr3.root;
}


// ---------------------------------------------------------------------------
void ptedit_set_paging_root(pid_t pid, size_t root) {
  ptedit_paging_t cr3;
  cr3.pid = (size_t)pid;
  cr3.root = root;
  ioctl(ptedit_fd, PTEDITOR_IOCTL_CMD_SET_ROOT, (size_t)&cr3);
}


// ---------------------------------------------------------------------------
void ptedit_invalidate_tlb(void* address) {
  ioctl(ptedit_fd, PTEDITOR_IOCTL_CMD_INVALIDATE_TLB, (size_t)address);
}


// ---------------------------------------------------------------------------
size_t ptedit_get_mts() {
    size_t mt;
    ioctl(ptedit_fd, PTEDITOR_IOCTL_CMD_GET_PAT, (size_t)&mt);
    return mt;
}


// ---------------------------------------------------------------------------
char ptedit_get_mt(unsigned char mt) {
    size_t mts = ptedit_get_mts();
#if defined(__i386__) || defined(__x86_64__)
    return ((mts >> (mt * 8)) & 7);
#elif defined(__aarch64__)
    return ((mts >> (mt * 8)) & 0xff);
#endif
}


// ---------------------------------------------------------------------------
const char* ptedit_mt_to_string(unsigned char mt) {
#if defined(__i386__) || defined(__x86_64__)
    const char* mts[] = {"UC", "WC", "Rsvd", "Rsvd", "WT", "WP", "WB", "UC-", "Rsvd"};
    if(mt <= 7) return mts[mt];
    return NULL;
#elif defined(__aarch64__)
    static char mts[16];
    int i;
    mts[0] = 0;
    for(i = 0; i < 2; i++) {
        strcat(mts, i == 0 ? "I" : "O");
        if((mt & 0xf) == ((mt >> 4) & 0xf)) strcpy(mts, "");
        switch((mt >> (i * 4)) & 0xf) {
            case 0:
                strcat(mts, "DM");
                break;
            case 1: /* Fall through */
            case 2: /* Fall through */
            case 3:
                strcat(mts, "WT");
                break;
            case 4:
                strcat(mts, "UC");
                break;
            case 5: /* Fall through */
            case 6: /* Fall through */
            case 7:
                strcat(mts, "WB");
                break;
            case 8: /* Fall through */
            case 9: /* Fall through */
            case 10: /* Fall through */
            case 11:
                strcat(mts, "WT");
                break;
            case 12: /* Fall through */
            case 13: /* Fall through */
            case 14: /* Fall through */
            case 15:
                strcat(mts, "WB");
        }
    }
    return mts;
#endif
}


// ---------------------------------------------------------------------------
void ptedit_set_mts(size_t mts) {
    ioctl(ptedit_fd, PTEDITOR_IOCTL_CMD_SET_PAT, mts);
}


// ---------------------------------------------------------------------------
void ptedit_set_mt(unsigned char mt, unsigned char value) {
    size_t mts = ptedit_get_mts();
#if defined(__i386__) || defined(__x86_64__)
    mts &=~(7 << (mt * 8));
#elif defined(__aarch64__)
    mts &=~(0xff << (mt * 8));
#endif
    mts |= (value << (mt * 8));
    ptedit_set_mts(mts);
}


// ---------------------------------------------------------------------------
unsigned char ptedit_find_mt(unsigned char type) {
    size_t mts = ptedit_get_mts();
    unsigned char found = 0;
    int i;
    for(i = 0; i < 8; i++) {
#if defined(__i386__) || defined(__x86_64__)
        if(((mts >> (i * 8)) & 7) == type) found |= (1 << i);
#elif defined(__aarch64__)
        if(((mts >> (i * 8)) & 0xff) == type) {
            found |= (1 << i);
        } else {
            unsigned char plow, phigh;
            plow = (mts >> (i * 8)) & 0xf;
            phigh = ((mts >> (i * 8)) >> 4) & 0xf;
            if((plow == phigh) && (plow == type)) {
                found |= (1 << i);
            }
        }
#endif
    }
    return found;
}


// ---------------------------------------------------------------------------
int ptedit_find_first_mt(unsigned char type) {
    return __builtin_ffs(ptedit_find_mt(type)) - 1;
}


// ---------------------------------------------------------------------------
size_t ptedit_apply_mt(size_t entry, unsigned char mt) {
#if defined(__i386__) || defined(__x86_64__)
    entry &=~((1ull << PTEDIT_PAGE_BIT_PWT) | (1ull << PTEDIT_PAGE_BIT_PCD) | (1ull << PTEDIT_PAGE_BIT_PAT));
    if(mt & 1) entry |= (1ull << PTEDIT_PAGE_BIT_PWT);
    if(mt & 2) entry |= (1ull << PTEDIT_PAGE_BIT_PCD);
    if(mt & 4) entry |= (1ull << PTEDIT_PAGE_BIT_PAT);
#elif defined(__aarch64__)
    entry &=~0x1c;
    entry |= (mt & 7) << 2;
#endif
    return entry;
}

// ---------------------------------------------------------------------------
unsigned char ptedit_extract_mt(size_t entry) {
#if defined(__i386__) || defined(__x86_64__)
    return (!!(entry & (1ull << PTEDIT_PAGE_BIT_PWT))) | ((!!(entry & (1ull << PTEDIT_PAGE_BIT_PCD))) << 1) | ((!!(entry & (1ull << PTEDIT_PAGE_BIT_PAT))) << 2);
#elif defined(__aarch64__)
    return (entry >> 2) & 7;
#endif
}

// ---------------------------------------------------------------------------
void ptedit_full_serializing_barrier() {
#if defined(__i386__) || defined(__x86_64__)
    asm volatile("mfence\nlfence\n" ::: "memory");
#elif defined(__aarch64__)
    asm volatile("DSB SY");
    asm volatile("DSB ISH");
    asm volatile("ISB");
#endif
    ptedit_set_paging_root(0, ptedit_get_paging_root(0));
#if defined(__i386__) || defined(__x86_64__)
    asm volatile("mfence\nlfence\n" ::: "memory");
#elif defined(__aarch64__)
    asm volatile("ISB");
    asm volatile("DSB ISH");
    asm volatile("DSB SY");
#endif
}


// ---------------------------------------------------------------------------
void ptedit_pte_set_bit(void* address, pid_t pid, int bit) {
   ptedit_entry_t vm = ptedit_resolve(address, pid);
   if(!(vm.valid & PTEDIT_VALID_MASK_PTE)) return;
   vm.pte |= (1ull << bit);
   vm.valid = PTEDIT_VALID_MASK_PTE;
   ptedit_update(address, pid, &vm);
}

// ---------------------------------------------------------------------------
void ptedit_pte_clear_bit(void* address, pid_t pid, int bit) {
   ptedit_entry_t vm = ptedit_resolve(address, pid);
   if(!(vm.valid & PTEDIT_VALID_MASK_PTE)) return;
   vm.pte &=~(1ull << bit);
   vm.valid = PTEDIT_VALID_MASK_PTE;
   ptedit_update(address, pid, &vm);
}

// ---------------------------------------------------------------------------
unsigned char ptedit_pte_get_bit(void* address, pid_t pid, int bit) {
   ptedit_entry_t vm = ptedit_resolve(address, pid);
   return !!(vm.pte & (1ull << bit));
}

// ---------------------------------------------------------------------------
size_t ptedit_pte_get_pfn(void* address, pid_t pid) {
   ptedit_entry_t vm = ptedit_resolve(address, pid);
   if(!(vm.valid & PTEDIT_VALID_MASK_PTE)) return 0;
   else return ptedit_get_pfn(vm.pte);
}

// ---------------------------------------------------------------------------
void ptedit_pte_set_pfn(void* address, pid_t pid, size_t pfn) {
   ptedit_entry_t vm = ptedit_resolve(address, pid);
   if(!(vm.valid & PTEDIT_VALID_MASK_PTE)) return;
   vm.pte = ptedit_set_pfn(vm.pte, pfn);
   vm.valid = PTEDIT_VALID_MASK_PTE;
   ptedit_update(address, pid, &vm);
}

