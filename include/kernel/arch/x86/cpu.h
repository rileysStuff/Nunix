#ifndef _CPU_H
#define _CPU_H

#include <stdint.h>
#include <stdbool.h>

// CPU Vendor IDs
#define CPU_VENDOR_INTEL      "Genuine-Intel"
#define CPU_VENDOR_AMD        "Authentic-AMD"
#define CPU_VENDOR_HYPERV     "Microsoft Hv"
#define CPU_VENDOR_VMWARE     "VMwareVMware"
#define CPU_VENDOR_QEMU       "TCGTCGTCGTCG"
#define CPU_VENDOR_VIRTUALBOX "VBoxVBoxVBox"
#define CPU_VENDOR_UNKNOWN    "Unknown"

#define CPU_DEBUG_DUMP_CACHES   0x01
#define CPU_DEBUG_DUMP_FEATURES 0x02
#define CPU_DEBUG_DUMP_ALL      0xFF

// CPU Feature Flags (CPUID-based)
typedef struct {
    // Standard Feature Flags (EDX from CPUID 1)
    uint32_t fpu:1;
    uint32_t vme:1;
    uint32_t de:1;
    uint32_t pse:1;
    uint32_t tsc:1;
    uint32_t msr:1;
    uint32_t pae:1;
    uint32_t mce:1;
    uint32_t cx8:1;
    uint32_t apic:1;
    uint32_t sep:1;
    uint32_t mtrr:1;
    uint32_t pge:1;
    uint32_t mca:1;
    uint32_t cmov:1;
    uint32_t pat:1;
    uint32_t pse36:1;
    uint32_t psn:1;
    uint32_t clfsh:1;
    uint32_t ds:1;
    uint32_t acpi:1;
    uint32_t mmx:1;
    uint32_t fxsr:1;
    uint32_t sse:1;
    uint32_t sse2:1;
    uint32_t ss:1;
    uint32_t htt:1;
    uint32_t tm:1;
    uint32_t ia64:1;
    uint32_t pbe:1;

    // Extended Feature Flags (ECX from CPUID 1)
    uint32_t sse3:1;
    uint32_t pclmul:1;
    uint32_t dtes64:1;
    uint32_t monitor:1;
    uint32_t ds_cpl:1;
    uint32_t vmx:1;
    uint32_t smx:1;
    uint32_t est:1;
    uint32_t tm2:1;
    uint32_t ssse3:1;
    uint32_t cid:1;
    uint32_t sdbg:1;
    uint32_t fma:1;
    uint32_t cx16:1;
    uint32_t xtpr:1;
    uint32_t pdcm:1;
    uint32_t pcid:1;
    uint32_t dca:1;
    uint32_t sse4_1:1;
    uint32_t sse4_2:1;
    uint32_t x2apic:1;
    uint32_t movbe:1;
    uint32_t popcnt:1;
    uint32_t tsc_deadline:1;
    uint32_t aes:1;
    uint32_t xsave:1;
    uint32_t osxsave:1;
    uint32_t avx:1;
    uint32_t f16c:1;
    uint32_t rdrand:1;
    uint32_t hypervisor:1;

    // Extended Features (CPUID 7)
    uint32_t fsgsbase:1;
    uint32_t ia32_tsc_adjust:1;
    uint32_t sgx:1;
    uint32_t bmi1:1;
    uint32_t hle:1;
    uint32_t avx2:1;
    uint32_t fdp_excptn_only:1;
    uint32_t smep:1;
    uint32_t bmi2:1;
    uint32_t erms:1;
    uint32_t invpcid:1;
    uint32_t rtm:1;
    uint32_t pqm:1;
    uint32_t dep_fpu_csds:1;
    uint32_t mpx:1;
    uint32_t pqe:1;
    uint32_t avx512f:1;
    uint32_t avx512dq:1;
    uint32_t rdseed:1;
    uint32_t adx:1;
    uint32_t smap:1;
    uint32_t avx512ifma:1;
    uint32_t pcommit:1;
    uint32_t clflushopt:1;
    uint32_t clwb:1;
    uint32_t intel_pt:1;
    uint32_t avx512pf:1;
    uint32_t avx512er:1;
    uint32_t avx512cd:1;
    uint32_t sha:1;
    uint32_t avx512bw:1;
    uint32_t avx512vl:1;
    uint32_t prefetchwt1:1;
    uint32_t avx512vbmi:1;
    uint32_t umip:1;
    uint32_t pku:1;
    uint32_t ospke:1;
    uint32_t waitpkg:1;
    uint32_t avx512_vbmi2:1;
    uint32_t cet_ss:1;
    uint32_t gfni:1;
    uint32_t vaes:1;
    uint32_t vpclmulqdq:1;
    uint32_t avx512_vnni:1;
    uint32_t avx512_bitalg:1;
    uint32_t tme:1;
    uint32_t avx512_vpopcntdq:1;
    uint32_t la57:1;
    uint32_t mawau:5;
    uint32_t rdpid:1;
    uint32_t kl:1;
    uint32_t bus_lock_detect:1;
    uint32_t cldemote:1;
    uint32_t movdiri:1;
    uint32_t movdir64b:1;
    uint32_t enqcmd:1;
    uint32_t sgx_lc:1;
    uint32_t pks:1;

    // AMD-specific features
    uint32_t syscall:1;
    uint32_t mp:1;
    uint32_t nx:1;
    uint32_t mmxext:1;
    uint32_t fxsr_opt:1;
    uint32_t pdpe1gb:1;
    uint32_t rdtscp:1;
    uint32_t lm:1;
    uint32_t _3dnowext:1;
    uint32_t _3dnow:1;
    uint32_t lahf_lm:1;
    uint32_t cmp_legacy:1;
    uint32_t svm:1;
    uint32_t extapic:1;
    uint32_t cr8_legacy:1;
    uint32_t abm:1;
    uint32_t sse4a:1;
    uint32_t misalignsse:1;
    uint32_t _3dnowprefetch:1;
    uint32_t osvw:1;
    uint32_t ibs:1;
    uint32_t xop:1;
    uint32_t skinit:1;
    uint32_t wdt:1;
    uint32_t lwp:1;
    uint32_t fma4:1;
    uint32_t tce:1;
    uint32_t nodeid_msr:1;
    uint32_t tbm:1;
    uint32_t topoext:1;
    uint32_t perfctr_core:1;
    uint32_t perfctr_nb:1;
    uint32_t dbx:1;
    uint32_t perfctr_llc:1;
    uint32_t mwaitx:1;
    uint32_t hw_pstate:1;
} cpu_features_t;

// CPU Cache Types
typedef enum {
    CACHE_TYPE_NULL = 0,
    CACHE_TYPE_DATA = 1,
    CACHE_TYPE_INSTRUCTION = 2,
    CACHE_TYPE_UNIFIED = 3
} cache_type_t;

// CPU Cache Descriptor
typedef struct {
    uint8_t level;
    cache_type_t type;
    uint32_t line_size;
    uint32_t sets;
    uint16_t ways;
    uint32_t size; // in bytes
    bool fully_associative;
    bool self_initializing;
    bool inclusive;
} cache_descriptor_t;

// CPU Topology Information
typedef struct {
    uint32_t package_id;
    uint32_t core_id;
    uint32_t smt_id;
    uint32_t numa_node;
} cpu_topology_t;

// CPU Vendor Information
typedef struct {
    char vendor_id[13]; // 12 chars + null terminator
    uint8_t family;
    uint8_t model;
    uint8_t stepping;
    char brand_string[49]; // 48 chars + null terminator
    uint32_t max_cpuid;
    uint32_t max_ext_cpuid;
} cpu_identity_t;

// CPU State Information
typedef struct {
    cpu_identity_t identity;
    cpu_features_t features;
    cpu_topology_t topology;
    uint32_t apic_id;
    uint32_t tsc_frequency; // in kHz
    uint32_t bus_frequency; // in kHz
    uint32_t max_phy_addr_bits;
    uint32_t max_lin_addr_bits;
    uint8_t num_caches;
    cache_descriptor_t caches[16]; // Support up to 16 cache descriptors
} cpu_info_t;

// CPU Exception Frame (x86_64)
typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
} cpu_exception_frame_t;

// Function Prototypes

// Initialization
void cpu_early_init(void);
void cpu_late_init(void);

// Identification
void cpu_identify(cpu_info_t* info);
const char* cpu_vendor_name(const cpu_info_t* info);
bool cpu_has_feature(const cpu_info_t* info, const char* feature_name);

// Topology
void cpu_detect_topology(cpu_info_t* info);
uint32_t cpu_count_cores(void);
uint32_t cpu_count_logical(void);

// Control
void cpu_enable_features(cpu_info_t* info);
void cpu_disable_write_protect(void);
void cpu_enable_write_protect(void);
void cpu_enable_nx_bit(void);
void cpu_enable_smep(void);
void cpu_enable_smap(void);
void cpu_enable_umip(void);
void cpu_enable_paging(void);
void cpu_disable_paging(void);

// Cache Control
void cpu_invalidate_cache(void);
void cpu_flush_tlb(void);
void cpu_flush_tlb_entry(uintptr_t addr);
void cpu_prefetch(const void* addr);

// Timing
uint64_t cpu_rdtsc(void);
uint64_t cpu_rdtscp(uint32_t* aux);
void cpu_calibrate_tsc(void);

// Synchronization
void cpu_pause(void);
void cpu_spinwait(uint32_t iterations);
void cpu_debug_dump_ex(const cpu_info_t* info, uint32_t flags);

// Power Management
void cpu_halt(void);
void cpu_sleep(void);
void cpu_wait_for_interrupt(void);
void cpu_shutdown(void);

// Debugging
void cpu_breakpoint(void);
void cpu_debug_dump(const cpu_info_t* info);

// Exception Handling
void cpu_install_exception_handler(uint8_t vector, void (*handler)(cpu_exception_frame_t*));
void cpu_enable_interrupts(void);
void cpu_disable_interrupts(void);
bool cpu_interrupts_enabled(void);

// Virtualization
bool cpu_vmx_supported(void);
bool cpu_svm_supported(void);
void cpu_vmx_on(void);
void cpu_svm_enable(void);

// MSR Operations
uint64_t cpu_read_msr(uint32_t msr);
void cpu_write_msr(uint32_t msr, uint64_t value);

// CPUID Wrapper
void cpu_cpuid(uint32_t eax, uint32_t ecx, uint32_t* eax_out, uint32_t* ebx_out, uint32_t* ecx_out, uint32_t* edx_out);

// Specialized Instructions
void cpu_invlpg(uintptr_t addr);
void cpu_wbinvd(void);
void cpu_clflush(const void* addr);
void cpu_clwb(const void* addr);
void cpu_clflushopt(const void* addr);

#endif // _CPU_H
