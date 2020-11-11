/** @file */

#pragma once
#include "module/pteditor.h"
#include <sys/types.h>

/**
 * The implementation of PTEditor to use
 *
 * @defgroup PTEDITOR_IMPLEMENTATION PTEditor Implementation
 *
 * @{
 */

 /** Use the kernel to resolve and update paging structures */
#define PTEDIT_IMPL_KERNEL       0
/** Use the user-space implemenation to resolve and update paging structures, using pread to read from the memory mapping */
#define PTEDIT_IMPL_USER_PREAD   1
/** Use the user-space implemenation that maps the physical memory into user space to resolve and update paging structures */
#define PTEDIT_IMPL_USER         2

/**
 * The bits in a page-table entry
 *
 * @defgroup PAGETABLE_BITS Page Table Bits
 *
 * @{
 *
 */

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

/** @} */

/**
 * The memory types (PAT/MAIR)values
 *
 * @defgroup MEMORY_TYPES Memory Types (PAT/MAIR values)
 *
 * @{
 */

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
 * @param[in] implementation The implementation to use, either PTEDIT_IMPL_KERNEL, PTEDIT_IMPL_USER, or PTEDIT_IMPL_USER_PREAD
 *
 */
void ptedit_use_implementation(int implementation);

/** @} */



// Functions to read and write physical pages
typedef size_t(*ptedit_phys_read_t)(size_t);
typedef void(*ptedit_phys_write_t)(size_t, size_t);

/**
 * Functions to read and write page tables
 *
 * @defgroup PAGETABLE Page tables
 *
 * @{
 */

typedef ptedit_entry_t(*ptedit_resolve_t)(void*, pid_t);
typedef void (*ptedit_update_t)(void*, pid_t, ptedit_entry_t*);




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


#define PTEDIT_PAGE_PRESENT 1

/**
 * Struct to access the fields of the PGD
 */
#pragma pack(push,1)
typedef struct {
    size_t present : 1;
    size_t writeable : 1;
    size_t user_access : 1;
    size_t write_through : 1;
    size_t cache_disabled : 1;
    size_t accessed : 1;
    size_t ignored_3 : 1;
    size_t size : 1;
    size_t ignored_2 : 4;
    size_t pfn : 28;
    size_t reserved_1 : 12;
    size_t ignored_1 : 11;
    size_t execution_disabled : 1;
} ptedit_pgd_t;
#pragma pack(pop)


/**
 * Struct to access the fields of the P4D
 */
typedef ptedit_pgd_t ptedit_p4d_t;


/**
 * Struct to access the fields of the PUD
 */
typedef ptedit_pgd_t ptedit_pud_t;


/**
 * Struct to access the fields of the PMD
 */
typedef ptedit_pgd_t ptedit_pmd_t;


/**
 * Struct to access the fields of the PMD when mapping a  large page (2MB)
 */
#pragma pack(push,1)
typedef struct {
    size_t present : 1;
    size_t writeable : 1;
    size_t user_access : 1;
    size_t write_through : 1;
    size_t cache_disabled : 1;
    size_t accessed : 1;
    size_t dirty : 1;
    size_t size : 1;
    size_t global : 1;
    size_t ignored_2 : 3;
    size_t pat : 1;
    size_t reserved_2 : 8;
    size_t pfn : 19;
    size_t reserved_1 : 12;
    size_t ignored_1 : 11;
    size_t execution_disabled : 1;
} ptedit_pmd_large_t;
#pragma pack(pop)

/**
 * Struct to access the fields of the PTE
 */
#pragma pack(push,1)
typedef struct {
    size_t present : 1;
    size_t writeable : 1;
    size_t user_access : 1;
    size_t write_through : 1;
    size_t cache_disabled : 1;
    size_t accessed : 1;
    size_t dirty : 1;
    size_t size : 1;
    size_t global : 1;
    size_t ignored_2 : 3;
    size_t pfn : 28;
    size_t reserved_1 : 12;
    size_t ignored_1 : 11;
    size_t execution_disabled : 1;
} ptedit_pte_t;
#pragma pack(pop)


/**
 * Casts a paging structure entry (e.g., page table) to a structure with easy access to its fields
 *
 * @param[in] v Entry to Cast
 * @param[in] type Data type of struct to cast to, e.g., ptedit_pte_t
 *
 * @return Struct of type "type" with easily accessible fields
 */
#define ptedit_cast(v, type) (*((type*)(&(v))))

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

/**
 * Map a physical address range.
 *
 * @param[in] physical The physical address to map
 * @param[in] length The length of the physical memory range to map
 *
 * @return A virtual address that can be used to access the physical range
 */
void* ptedit_pmap(size_t physical, size_t length);

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

// Issue shootdowns to CPUs specified by a mask
void ptedit_tlb_shootdown(size_t cpu_mask);

// Map an address to a provided pfn
void ptedit_map_page(void* address, size_t pfn);

/* Previously missing declarations */
ptedit_entry_t ptedit_resolve_kernel(void* address, pid_t pid);
void ptedit_update_kernel(void* address, pid_t pid, ptedit_entry_t* vm);
void ptedit_update_user_ext(void* address, pid_t pid, ptedit_entry_t* vm, ptedit_phys_write_t pset);
