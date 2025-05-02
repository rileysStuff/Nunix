#include "../../../include/kernel/arch/x86/cpu.h"
#include "../../../include/video/vga.h"
#include "../../../include/kernel/ports/ports.h"
#include <string.h>
#include <stddef.h>

static inline uint32_t read_cr0(void) {
    uint32_t val;
    asm volatile("mov %%cr0, %0" : "=r"(val));
    return val;
}

static inline void write_cr0(uint32_t val) {
    asm volatile("mov %0, %%cr0" : : "r"(val));
}

static inline uint32_t read_cr3(void) {
    uint32_t val;
    asm volatile("mov %%cr3, %0" : "=r"(val));
    return val;
}

static inline void write_cr3(uint32_t val) {
    asm volatile("mov %0, %%cr3" : : "r"(val));
}

static inline uint32_t read_cr4(void) {
    uint32_t val;
    asm volatile("mov %%cr4, %0" : "=r"(val));
    return val;
}

static inline void write_cr4(uint32_t val) {
    asm volatile("mov %0, %%cr4" : : "r"(val));
}

// 32-bit flags access
static inline uint32_t read_eflags(void) {
    uint32_t flags;
    asm volatile("pushfl; pop %0" : "=r"(flags));
    return flags;
}

static int is_digit(char c) {
    return c >= '0' && c <= '9';
}

static uint32_t str_to_ul(const char* str, char** end, int base) {
    uint32_t result = 0;
    while (*str) {
        if (*str >= '0' && *str <= '9') {
            result = result * base + (*str - '0');
        } else {
            break;
        }
        str++;
    }
    if (end) *end = (char*)str;
    return result;
}

static void vga_printf(const char* str) {
    vga_puts(str);
}

static void print_hex(uint32_t num) {
    vga_puthex(num);
}

static void print_dec(uint32_t value, uint8_t digits) {
    vga_putdec(value, digits);
}

static void print_feature(const char* name, bool present) {
    if(present) {
        vga_puts("  ");
        vga_puts(name);
        vga_puts("\n");
    }
}


// Global CPU information
static cpu_info_t global_cpu_info;
static bool cpu_initialized = false;

// Internal helper functions
static void cpuid(uint32_t eax, uint32_t ecx, uint32_t* eax_out, uint32_t* ebx_out, uint32_t* ecx_out, uint32_t* edx_out) {
    asm volatile("cpuid"
                 : "=a"(*eax_out), "=b"(*ebx_out), "=c"(*ecx_out), "=d"(*edx_out)
                 : "a"(eax), "c"(ecx));
}

static uint64_t rdmsr(uint32_t msr) {
    uint32_t low, high;
    asm volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

static void wrmsr(uint32_t msr, uint64_t value) {
    uint32_t low = (uint32_t)value;
    uint32_t high = (uint32_t)(value >> 32);
    asm volatile("wrmsr" : : "a"(low), "d"(high), "c"(msr));
}

static void detect_caches(cpu_info_t* info) {
    uint32_t eax, ebx, ecx, edx;
    uint8_t cache_count = 0;
    
    // Check deterministic cache parameters are available
    cpuid(0x80000000, 0, &eax, &ebx, &ecx, &edx);
    if (eax < 0x80000006) {
        return; // Not supported
    }
    
    // Get L1 cache
    cpuid(0x80000005, 0, &eax, &ebx, &ecx, &edx);
    if (cache_count < 16) {
        info->caches[cache_count].level = 1;
        info->caches[cache_count].type = CACHE_TYPE_DATA;
        info->caches[cache_count].line_size = ecx & 0xFF;
        info->caches[cache_count].ways = (ecx >> 16) & 0xFF;
        info->caches[cache_count].size = (ecx >> 24) * 1024;
        cache_count++;
    }
    
    // Get L2/L3 cache
    cpuid(0x80000006, 0, &eax, &ebx, &ecx, &edx);
    if (cache_count < 16) {
        info->caches[cache_count].level = 2;
        info->caches[cache_count].type = CACHE_TYPE_UNIFIED;
        info->caches[cache_count].line_size = ecx & 0xFF;
        info->caches[cache_count].ways = (ecx >> 16) & 0xFF;
        info->caches[cache_count].size = (ecx >> 24) * 1024;
        cache_count++;
    }
    
    if (eax >= 0x8000001D) {
        // Enumerate cache topology using CPUID leaf 0x1D
        for (uint32_t i = 0; ; i++) {
            cpuid(0x8000001D, i, &eax, &ebx, &ecx, &edx);
            uint8_t type = eax & 0x1F;
            if (type == 0) break; // No more caches
            
            if (cache_count < 16) {
                info->caches[cache_count].level = (eax >> 5) & 0x7;
                info->caches[cache_count].type = type;
                info->caches[cache_count].line_size = (ebx & 0xFFF) + 1;
                info->caches[cache_count].sets = ecx + 1;
                info->caches[cache_count].ways = ((ebx >> 22) & 0x3FF) + 1;
                info->caches[cache_count].size = info->caches[cache_count].ways * 
                                               info->caches[cache_count].line_size * 
                                               info->caches[cache_count].sets;
                info->caches[cache_count].fully_associative = (ebx >> 9) & 1;
                info->caches[cache_count].self_initializing = (ebx >> 8) & 1;
                info->caches[cache_count].inclusive = (ebx >> 1) & 1;
                cache_count++;
            }
        }
    }
    
    info->num_caches = cache_count;
}

static void detect_topology(cpu_info_t* info) {
    uint32_t eax, ebx, ecx, edx;
    
    // Check if topology leaf is available
    cpuid(0x80000000, 0, &eax, &ebx, &ecx, &edx);
    if (eax < 0x8000001E) {
        // Fallback to legacy method
        cpuid(1, 0, &eax, &ebx, &ecx, &edx);
        info->topology.smt_id = (ebx >> 24) & 0xFF;
        info->topology.core_id = 0;
        info->topology.package_id = 0;
        info->topology.numa_node = 0;
        return;
    }
    
    // AMD topology extension
    if (strcmp(info->identity.vendor_id, CPU_VENDOR_AMD) == 0 && info->features.topoext) {
        cpuid(0x8000001E, 0, &eax, &ebx, &ecx, &edx);
        info->topology.core_id = eax & 0xFF;
        info->topology.smt_id = (eax >> 8) & 0xFF;
        info->topology.package_id = (eax >> 16) & 0xFF;
        info->topology.numa_node = ecx & 0xFF;
        return;
    }
    
    // Intel topology detection
    if (strcmp(info->identity.vendor_id, CPU_VENDOR_INTEL) == 0) {
        // Get x2APIC ID
        cpuid(1, 0, &eax, &ebx, &ecx, &edx);
        uint32_t x2apic_id = (ebx >> 24) & 0xFF;
        
        // Get topology information
        cpuid(0xB, 0, &eax, &ebx, &ecx, &edx);
        uint32_t bits_shift = eax & 0x1F;
        uint32_t logical_per_core = ebx & 0xFFFF;
        
        if (logical_per_core == 0) {
            info->topology.smt_id = 0;
            info->topology.core_id = 0;
            info->topology.package_id = x2apic_id;
        } else {
            info->topology.smt_id = x2apic_id & ((1 << bits_shift) - 1);
            info->topology.core_id = (x2apic_id >> bits_shift) & ((1 << (eax >> 16 & 0x1F)) - 1);
            info->topology.package_id = x2apic_id >> (bits_shift + (eax >> 16 & 0x1F));
        }
        
        // NUMA node (not always available)
        info->topology.numa_node = 0;
    }
}

static void detect_features(cpu_info_t* info) {
    uint32_t eax, ebx, ecx, edx;
    
    // Clear all features first
    memset(&info->features, 0, sizeof(cpu_features_t));
    
    // Standard features (CPUID 1)
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    
    // EDX features
    info->features.fpu = edx & (1 << 0);
    info->features.vme = edx & (1 << 1);
    info->features.de = edx & (1 << 2);
    info->features.pse = edx & (1 << 3);
    info->features.tsc = edx & (1 << 4);
    info->features.msr = edx & (1 << 5);
    info->features.pae = edx & (1 << 6);
    info->features.mce = edx & (1 << 7);
    info->features.cx8 = edx & (1 << 8);
    info->features.apic = edx & (1 << 9);
    info->features.sep = edx & (1 << 11);
    info->features.mtrr = edx & (1 << 12);
    info->features.pge = edx & (1 << 13);
    info->features.mca = edx & (1 << 14);
    info->features.cmov = edx & (1 << 15);
    info->features.pat = edx & (1 << 16);
    info->features.pse36 = edx & (1 << 17);
    info->features.psn = edx & (1 << 18);
    info->features.clfsh = edx & (1 << 19);
    info->features.ds = edx & (1 << 21);
    info->features.acpi = edx & (1 << 22);
    info->features.mmx = edx & (1 << 23);
    info->features.fxsr = edx & (1 << 24);
    info->features.sse = edx & (1 << 25);
    info->features.sse2 = edx & (1 << 26);
    info->features.ss = edx & (1 << 27);
    info->features.htt = edx & (1 << 28);
    info->features.tm = edx & (1 << 29);
    info->features.ia64 = edx & (1 << 30);
    info->features.pbe = edx & (1 << 31);
    
    // ECX features
    info->features.sse3 = ecx & (1 << 0);
    info->features.pclmul = ecx & (1 << 1);
    info->features.dtes64 = ecx & (1 << 2);
    info->features.monitor = ecx & (1 << 3);
    info->features.ds_cpl = ecx & (1 << 4);
    info->features.vmx = ecx & (1 << 5);
    info->features.smx = ecx & (1 << 6);
    info->features.est = ecx & (1 << 7);
    info->features.tm2 = ecx & (1 << 8);
    info->features.ssse3 = ecx & (1 << 9);
    info->features.cid = ecx & (1 << 10);
    info->features.sdbg = ecx & (1 << 11);
    info->features.fma = ecx & (1 << 12);
    info->features.cx16 = ecx & (1 << 13);
    info->features.xtpr = ecx & (1 << 14);
    info->features.pdcm = ecx & (1 << 15);
    info->features.pcid = ecx & (1 << 17);
    info->features.dca = ecx & (1 << 18);
    info->features.sse4_1 = ecx & (1 << 19);
    info->features.sse4_2 = ecx & (1 << 20);
    info->features.x2apic = ecx & (1 << 21);
    info->features.movbe = ecx & (1 << 22);
    info->features.popcnt = ecx & (1 << 23);
    info->features.tsc_deadline = ecx & (1 << 24);
    info->features.aes = ecx & (1 << 25);
    info->features.xsave = ecx & (1 << 26);
    info->features.osxsave = ecx & (1 << 27);
    info->features.avx = ecx & (1 << 28);
    info->features.f16c = ecx & (1 << 29);
    info->features.rdrand = ecx & (1 << 30);
    info->features.hypervisor = ecx & (1 << 31);
    
    // Extended features (CPUID 7)
    cpuid(7, 0, &eax, &ebx, &ecx, &edx);
    
    // EBX features
    info->features.fsgsbase = ebx & (1 << 0);
    info->features.ia32_tsc_adjust = ebx & (1 << 1);
    info->features.sgx = ebx & (1 << 2);
    info->features.bmi1 = ebx & (1 << 3);
    info->features.hle = ebx & (1 << 4);
    info->features.avx2 = ebx & (1 << 5);
    info->features.fdp_excptn_only = ebx & (1 << 6);
    info->features.smep = ebx & (1 << 7);
    info->features.bmi2 = ebx & (1 << 8);
    info->features.erms = ebx & (1 << 9);
    info->features.invpcid = ebx & (1 << 10);
    info->features.rtm = ebx & (1 << 11);
    info->features.pqm = ebx & (1 << 12);
    info->features.dep_fpu_csds = ebx & (1 << 13);
    info->features.mpx = ebx & (1 << 14);
    info->features.pqe = ebx & (1 << 15);
    info->features.avx512f = ebx & (1 << 16);
    info->features.avx512dq = ebx & (1 << 17);
    info->features.rdseed = ebx & (1 << 18);
    info->features.adx = ebx & (1 << 19);
    info->features.smap = ebx & (1 << 20);
    info->features.avx512ifma = ebx & (1 << 21);
    info->features.pcommit = ebx & (1 << 22);
    info->features.clflushopt = ebx & (1 << 23);
    info->features.clwb = ebx & (1 << 24);
    info->features.intel_pt = ebx & (1 << 25);
    info->features.avx512pf = ebx & (1 << 26);
    info->features.avx512er = ebx & (1 << 27);
    info->features.avx512cd = ebx & (1 << 28);
    info->features.sha = ebx & (1 << 29);
    info->features.avx512bw = ebx & (1 << 30);
    info->features.avx512vl = ebx & (1 << 31);
    
    // ECX features
    info->features.prefetchwt1 = ecx & (1 << 0);
    info->features.avx512vbmi = ecx & (1 << 1);
    info->features.umip = ecx & (1 << 2);
    info->features.pku = ecx & (1 << 3);
    info->features.ospke = ecx & (1 << 4);
    info->features.waitpkg = ecx & (1 << 5);
    info->features.avx512_vbmi2 = ecx & (1 << 6);
    info->features.cet_ss = ecx & (1 << 7);
    info->features.gfni = ecx & (1 << 8);
    info->features.vaes = ecx & (1 << 9);
    info->features.vpclmulqdq = ecx & (1 << 10);
    info->features.avx512_vnni = ecx & (1 << 11);
    info->features.avx512_bitalg = ecx & (1 << 12);
    info->features.tme = ecx & (1 << 13);
    info->features.avx512_vpopcntdq = ecx & (1 << 14);
    info->features.la57 = ecx & (1 << 16);
    info->features.rdpid = ecx & (1 << 22);
    info->features.kl = ecx & (1 << 23);
    info->features.bus_lock_detect = ecx & (1 << 24);
    info->features.cldemote = ecx & (1 << 25);
    info->features.movdiri = ecx & (1 << 27);
    info->features.movdir64b = ecx & (1 << 28);
    info->features.enqcmd = ecx & (1 << 29);
    info->features.sgx_lc = ecx & (1 << 30);
    info->features.pks = ecx & (1 << 31);
    
    // EDX features
    info->features.mawau = (edx >> 17) & 0x1F;
    
    // AMD-specific features
    cpuid(0x80000001, 0, &eax, &ebx, &ecx, &edx);
    info->features.syscall = edx & (1 << 11);
    info->features.mp = edx & (1 << 19);
    info->features.nx = edx & (1 << 20);
    info->features.mmxext = edx & (1 << 22);
    info->features.fxsr_opt = edx & (1 << 25);
    info->features.pdpe1gb = edx & (1 << 26);
    info->features.rdtscp = edx & (1 << 27);
    info->features.lm = edx & (1 << 29);
    info->features._3dnowext = edx & (1 << 30);
    info->features._3dnow = edx & (1 << 31);
    
    info->features.lahf_lm = ecx & (1 << 0);
    info->features.cmp_legacy = ecx & (1 << 1);
    info->features.svm = ecx & (1 << 2);
    info->features.extapic = ecx & (1 << 3);
    info->features.cr8_legacy = ecx & (1 << 4);
    info->features.abm = ecx & (1 << 5);
    info->features.sse4a = ecx & (1 << 6);
    info->features.misalignsse = ecx & (1 << 7);
    info->features._3dnowprefetch = ecx & (1 << 8);
    info->features.osvw = ecx & (1 << 9);
    info->features.ibs = ecx & (1 << 10);
    info->features.xop = ecx & (1 << 11);
    info->features.skinit = ecx & (1 << 12);
    info->features.wdt = ecx & (1 << 13);
    info->features.lwp = ecx & (1 << 15);
    info->features.fma4 = ecx & (1 << 16);
    info->features.tce = ecx & (1 << 17);
    info->features.nodeid_msr = ecx & (1 << 19);
    info->features.tbm = ecx & (1 << 21);
    info->features.topoext = ecx & (1 << 22);
    info->features.perfctr_core = ecx & (1 << 23);
    info->features.perfctr_nb = ecx & (1 << 24);
    info->features.dbx = ecx & (1 << 26);
    info->features.perfctr_llc = ecx & (1 << 28);
    info->features.mwaitx = ecx & (1 << 29);
    info->features.hw_pstate = ecx & (1 << 31);
}

static void detect_tsc_frequency(cpu_info_t* info) {
    if (!info->features.tsc) {
        info->tsc_frequency = 0;
        return;
    }
    
    uint32_t eax, ebx, ecx, edx;
    cpuid(0x15, 0, &eax, &ebx, &ecx, &edx);
    
    if (eax != 0 && ebx != 0 && ecx != 0) {
        info->tsc_frequency = (ecx * ebx) / eax;
    } else {
        if (strcmp(info->identity.vendor_id, CPU_VENDOR_INTEL) == 0) {
            uint64_t msr_platform_info = rdmsr(0xCE);
            uint64_t bus_ratio = (msr_platform_info >> 8) & 0xFF;
            uint64_t bus_freq = 100000; // Default 100MHz
            info->tsc_frequency = bus_ratio * bus_freq;
        } else if (strcmp(info->identity.vendor_id, CPU_VENDOR_AMD) == 0) {
            cpuid(0x80000007, 0, &eax, &ebx, &ecx, &edx);
            if (edx & (1 << 8)) { // TSC invariant
                const char* brand = info->identity.brand_string;
                const char* mhz = brand + strlen(brand) - 1;
                
                // Find MHz marker
                while (mhz > brand && !(mhz[0] == 'M' && mhz[1] == 'H' && mhz[2] == 'z')) {
                    mhz--;
                }
                
                // Find start of number
                const char* num_start = mhz;
                while (num_start > brand && !is_digit(*num_start)) {
                    num_start--;
                }
                
                if (is_digit(*num_start)) {
                    const char* num_end = num_start;
                    while (num_start > brand && is_digit(*(num_start-1))) {
                        num_start--;
                    }
                    
                    char* end;
                    uint32_t freq = str_to_ul(num_start, &end, 10);
                    if (end > num_start) {
                        info->tsc_frequency = freq * 1000;
                    }
                }
            }
        }
    }
    
    if (info->tsc_frequency == 0) {
        info->tsc_frequency = 2000000; // Default to 2GHz
    }
}

// Public API Implementation

void cpu_early_init(void) {
    memset(&global_cpu_info, 0, sizeof(cpu_info_t));
    
    // Basic CPU identification
    uint32_t eax, ebx, ecx, edx;
    cpuid(0, 0, &eax, &ebx, &ecx, &edx);
    
    global_cpu_info.identity.max_cpuid = eax;
    *((uint32_t*)&global_cpu_info.identity.vendor_id[0]) = ebx;
    *((uint32_t*)&global_cpu_info.identity.vendor_id[4]) = edx;
    *((uint32_t*)&global_cpu_info.identity.vendor_id[8]) = ecx;
    global_cpu_info.identity.vendor_id[12] = '\0';
    
    // Get processor info
    cpuid(1, 0, &eax, &ebx, &ecx, &edx);
    global_cpu_info.identity.stepping = eax & 0xF;
    global_cpu_info.identity.model = (eax >> 4) & 0xF;
    global_cpu_info.identity.family = (eax >> 8) & 0xF;
    
    // Extended model/family for Intel
    if (global_cpu_info.identity.family == 0xF) {
        global_cpu_info.identity.model |= ((eax >> 16) & 0xF) << 4;
        global_cpu_info.identity.family += ((eax >> 20) & 0xFF);
    }
    
    global_cpu_info.apic_id = (ebx >> 24) & 0xFF;
    
    // Get brand string if available
    cpuid(0x80000000, 0, &eax, &ebx, &ecx, &edx);
    global_cpu_info.identity.max_ext_cpuid = eax;
    
    if (eax >= 0x80000004) {
        cpuid(0x80000002, 0, 
              (uint32_t*)&global_cpu_info.identity.brand_string[0],
              (uint32_t*)&global_cpu_info.identity.brand_string[4],
              (uint32_t*)&global_cpu_info.identity.brand_string[8],
              (uint32_t*)&global_cpu_info.identity.brand_string[12]);
        cpuid(0x80000003, 0, 
              (uint32_t*)&global_cpu_info.identity.brand_string[16],
              (uint32_t*)&global_cpu_info.identity.brand_string[20],
              (uint32_t*)&global_cpu_info.identity.brand_string[24],
              (uint32_t*)&global_cpu_info.identity.brand_string[28]);
        cpuid(0x80000004, 0, 
              (uint32_t*)&global_cpu_info.identity.brand_string[32],
              (uint32_t*)&global_cpu_info.identity.brand_string[36],
              (uint32_t*)&global_cpu_info.identity.brand_string[40],
              (uint32_t*)&global_cpu_info.identity.brand_string[44]);
        global_cpu_info.identity.brand_string[48] = '\0';
    }
    
    // Early feature detection
    detect_features(&global_cpu_info);
    
    // Enable required features
    cpu_enable_features(&global_cpu_info);
    
    cpu_initialized = true;
}

void cpu_late_init(void) {
    if (!cpu_initialized) return;
    
    // Detect cache topology
    detect_caches(&global_cpu_info);
    
    // Detect CPU topology
    detect_topology(&global_cpu_info);
    
    // Detect TSC frequency
    detect_tsc_frequency(&global_cpu_info);
    
    // Get address sizes
    uint32_t eax, ebx, ecx, edx;
    cpuid(0x80000008, 0, &eax, &ebx, &ecx, &edx);
    global_cpu_info.max_phy_addr_bits = eax & 0xFF;
    global_cpu_info.max_lin_addr_bits = (eax >> 8) & 0xFF;
}

void cpu_identify(cpu_info_t* info) {
    if (info) {
        memcpy(info, &global_cpu_info, sizeof(cpu_info_t));
    }
}

const char* cpu_vendor_name(const cpu_info_t* info) {
    if (!info) info = &global_cpu_info;
    return info->identity.vendor_id;
}

bool cpu_has_feature(const cpu_info_t* info, const char* feature_name) {
    if (!info) info = &global_cpu_info;
    
    // This is a simplified version - a real implementation would have a full mapping
    // of feature names to feature flags. Here's just a few examples:
    if (strcmp(feature_name, "sse") == 0) return info->features.sse;
    if (strcmp(feature_name, "sse2") == 0) return info->features.sse2;
    if (strcmp(feature_name, "avx") == 0) return info->features.avx;
    if (strcmp(feature_name, "avx2") == 0) return info->features.avx2;
    if (strcmp(feature_name, "aes") == 0) return info->features.aes;
    if (strcmp(feature_name, "vmx") == 0) return info->features.vmx;
    if (strcmp(feature_name, "svm") == 0) return info->features.svm;
    
    return false;
}

void cpu_detect_topology(cpu_info_t* info) {
    if (!info) info = &global_cpu_info;
    detect_topology(info);
}

uint32_t cpu_count_cores(void) {
    // Simplified - real implementation would need to enumerate all APIC IDs
    return 1;
}

uint32_t cpu_count_logical(void) {
    // Simplified - real implementation would need to enumerate all APIC IDs
    return 1;
}

void cpu_enable_features(cpu_info_t* info) {
    if (!info) info = &global_cpu_info;
    
    uint32_t cr0 = read_cr0();
    cr0 |= (1 << 5);  // Set NE bit
    write_cr0(cr0);
    
    uint32_t cr4 = read_cr4();
    if (info->features.sse || info->features.sse2 || info->features.sse3) {
        cr4 |= (1 << 9);   // OSFXSR
        cr4 |= (1 << 10);  // OSXMMEXCPT
    }
    if (info->features.pge) cr4 |= (1 << 7);
    if (info->features.smep) cr4 |= (1 << 20);
    if (info->features.smap) cr4 |= (1 << 21);
    if (info->features.umip) cr4 |= (1 << 11);
    if (info->features.fsgsbase) cr4 |= (1 << 16);
    write_cr4(cr4);
    
    // EFER is still MSR but we'll use 32-bit parts
    uint32_t efer_lo, efer_hi;
    asm volatile("rdmsr" : "=a"(efer_lo), "=d"(efer_hi) : "c"(0xC0000080));
    if (info->features.nx) efer_lo |= (1 << 11);
    asm volatile("wrmsr" : : "a"(efer_lo), "d"(efer_hi), "c"(0xC0000080));
}

void cpu_disable_write_protect(void) {
    uint32_t cr0 = read_cr0();
    cr0 &= ~(1 << 16);
    write_cr0(cr0);
}

void cpu_enable_write_protect(void) {
    uint32_t cr0 = read_cr0();
    cr0 |= (1 << 16);
    write_cr0(cr0);
}


void cpu_enable_nx_bit(void) {
    uint64_t efer = rdmsr(0xC0000080);
    efer |= (1 << 11);
    wrmsr(0xC0000080, efer);
}

void cpu_enable_smep(void) {
    uint32_t cr4 = read_cr4();
    cr4 |= (1 << 20);
    write_cr4(cr4);
}

void cpu_enable_smap(void) {
    uint32_t cr4 = read_cr4();
    cr4 |= (1 << 21);
    write_cr4(cr4);
}

void cpu_enable_umip(void) {
    uint32_t cr4 = read_cr4();
    cr4 |= (1 << 11);
    write_cr4(cr4);
}

void cpu_enable_paging(void) {
    uint32_t cr0 = read_cr0();
    cr0 |= (1 << 31);
    write_cr0(cr0);
}

void cpu_disable_paging(void) {
    uint32_t cr0 = read_cr0();
    cr0 &= ~(1 << 31);
    write_cr0(cr0);
}

void cpu_invalidate_cache(void) {
    asm volatile("wbinvd");
}

void cpu_flush_tlb(void) {
    uint32_t cr3 = read_cr3();
    write_cr3(cr3);
}

void cpu_flush_tlb_entry(uintptr_t addr) {
    asm volatile("invlpg (%0)" : : "r"(addr));
}

void cpu_prefetch(const void* addr) {
    asm volatile("prefetchnta (%0)" : : "r"(addr));
}

uint64_t cpu_rdtsc(void) {
    uint32_t low, high;
    asm volatile("rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

uint64_t cpu_rdtscp(uint32_t* aux) {
    uint32_t low, high, c;
    asm volatile("rdtscp" : "=a"(low), "=d"(high), "=c"(c));
    if (aux) *aux = c;
    return ((uint64_t)high << 32) | low;
}

void cpu_calibrate_tsc(void) {
    // TODO: Implement proper TSC calibration
    // This would typically use PIT or HPET to measure real time
    // and compare with TSC counts
}

void cpu_pause(void) {
    asm volatile("pause");
}

void cpu_spinwait(uint32_t iterations) {
    for (uint32_t i = 0; i < iterations; i++) {
        cpu_pause();
    }
}

void cpu_halt(void) {
    asm volatile("hlt");
}

void cpu_sleep(void) {
    asm volatile("sti; hlt; cli");
}

void cpu_wait_for_interrupt(void) {
    cpu_sleep();
}

void cpu_shutdown(void) {
    // Try ACPI shutdown first
    // Fallback to keyboard controller if ACPI not available
    // Final fallback to CPU reset
    asm volatile("cli");
    
    // Try ACPI shutdown
    uint16_t* SMI_CMD = (uint16_t*)0xB2;
    uint16_t* PM1a_CNT = (uint16_t*)0x1004;
    uint16_t* PM1b_CNT = (uint16_t*)0x1005;
    uint16_t ACPI_ENABLE = 0x1F;
    uint16_t ACPI_DISABLE = 0x1E06;
    
    if (*SMI_CMD != 0) {
        *SMI_CMD = ACPI_ENABLE;
        *PM1a_CNT = ACPI_DISABLE;
        if (*PM1b_CNT != 0) {
            *PM1b_CNT = ACPI_DISABLE;
        }
    }
    
    // If still running, try keyboard controller
    uint8_t good = 0x02;
    while (good & 0x02) {
        good = inb(0x64);
    }
    outb(0x64, 0xFE);
    
    // If still running, triple fault
    asm volatile("int $0xFF");
}

void cpu_breakpoint(void) {
    asm volatile("int $3");
}

void cpu_debug_dump(const cpu_info_t* info) {
    if (!info) info = &global_cpu_info;
    
    // TODO: Implement detailed debug dump of CPU information
    // This would print all detected features, topology, etc.
}

void cpu_install_exception_handler(uint8_t vector, void (*handler)(cpu_exception_frame_t*)) {
    // TODO: Implement exception handler installation
    // This would typically modify the IDT
}

void cpu_enable_interrupts(void) {
    asm volatile("sti");
}

void cpu_disable_interrupts(void) {
    asm volatile("cli");
}

bool cpu_interrupts_enabled(void) {
    return (read_eflags() & (1 << 9)) != 0;
}

bool cpu_vmx_supported(void) {
    return global_cpu_info.features.vmx;
}

bool cpu_svm_supported(void) {
    return global_cpu_info.features.svm;
}

void cpu_vmx_on(void) {
    if (!cpu_vmx_supported()) return;
    
    uint32_t cr0 = read_cr0();
    cr0 &= ~(1 << 2); // Clear EM
    cr0 |= (1 << 1);  // Set MP
    cr0 |= (1 << 5);  // Set NE
    write_cr0(cr0);
    
    uint32_t cr4 = read_cr4();
    cr4 |= (1 << 13); // Enable VMXE
    write_cr4(cr4);
    
    // Execute VMXON
    // TODO: Implement VMXON with proper VMXON region
}

void cpu_svm_enable(void) {
    if (!cpu_svm_supported()) return;
    
    // Set EFER.SVME
    uint64_t efer = rdmsr(0xC0000080);
    efer |= (1 << 12);
    wrmsr(0xC0000080, efer);
    
    // TODO: Implement SVM initialization
}

uint64_t cpu_read_msr(uint32_t msr) {
    return rdmsr(msr);
}

void cpu_write_msr(uint32_t msr, uint64_t value) {
    wrmsr(msr, value);
}

void cpu_cpuid(uint32_t eax, uint32_t ecx, uint32_t* eax_out, uint32_t* ebx_out, uint32_t* ecx_out, uint32_t* edx_out) {
    cpuid(eax, ecx, eax_out, ebx_out, ecx_out, edx_out);
}

void cpu_invlpg(uintptr_t addr) {
    asm volatile("invlpg (%0)" : : "r"(addr));
}

void cpu_wbinvd(void) {
    asm volatile("wbinvd");
}

void cpu_clflush(const void* addr) {
    asm volatile("clflush (%0)" : : "r"(addr));
}

void cpu_clwb(const void* addr) {
    if (global_cpu_info.features.clwb) {
        asm volatile(".byte 0x66, 0x0F, 0xAE, 0x30" : : "r"(addr));
    }
}

void cpu_clflushopt(const void* addr) {
    if (global_cpu_info.features.clflushopt) {
        asm volatile(".byte 0x66, 0x0F, 0xAE, 0x38" : : "r"(addr));
    }
}

void cpu_debug_dump_ex(const cpu_info_t* info, uint32_t flags) {
    if (!info) info = &global_cpu_info;
    
    // Save original color
    uint8_t original_color = vga_get_color();
    enum vga_color old_fg = (enum vga_color)(original_color & 0x0F);
    enum vga_color old_bg = (enum vga_color)((original_color >> 4) & 0x0F);
    
    // Helper function for hex output
    void print_hex32(uint32_t value) {
        char hex_buf[9];
        const char hex_chars[] = "0123456789ABCDEF";
        for(int i = 7; i >= 0; i--) {
            hex_buf[i] = hex_chars[value & 0xF];
            value >>= 4;
        }
        hex_buf[8] = '\0';
        vga_puts(hex_buf);
    }

    // Section headers
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts("\n=== CPU Identification ===\n");
    
    // Vendor information
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("Vendor ID: ");
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts(info->identity.vendor_id);
    vga_puts("\n");
    
    // Brand string
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("Brand String: ");
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts(info->identity.brand_string);
    vga_puts("\n");

    // CPU version information
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("Family: 0x");
    print_hex32(info->identity.family);
    vga_puts("  Model: 0x");
    print_hex32(info->identity.model);
    vga_puts("  Stepping: 0x");
    print_hex32(info->identity.stepping);
    vga_puts("\n");

    // TSC information
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("TSC Frequency: ");
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_putdec(info->tsc_frequency / 1000, 0);
    vga_puts(" MHz (");
    vga_putdec(info->tsc_frequency, 0);
    vga_puts(" KHz)\n");

    // Topology information
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("Topology - Package: ");
    vga_putdec(info->topology.package_id, 0);
    vga_puts("  Core: ");
    vga_putdec(info->topology.core_id, 0);
    vga_puts("  Thread: ");
    vga_putdec(info->topology.smt_id, 0);
    vga_puts("  NUMA: ");
    vga_putdec(info->topology.numa_node, 0);
    vga_puts("\n");

    // Address sizes
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("Address Sizes - Physical: ");
    vga_putdec(info->max_phy_addr_bits, 0);
    vga_puts(" bits  Virtual: ");
    vga_putdec(info->max_lin_addr_bits, 0);
    vga_puts(" bits\n");

    // Cache information
    if (flags & CPU_DEBUG_DUMP_CACHES) {
        vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        vga_puts("\n=== Cache Hierarchy ===\n");
        
        for (int i = 0; i < info->num_caches; i++) {
            const cache_descriptor_t* c = &info->caches[i];
            vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
            vga_puts("L");
            vga_putdec(c->level, 0);
            
            switch(c->type) {
                case CACHE_TYPE_DATA: 
                    vga_puts(" Data       ");
                    break;
                case CACHE_TYPE_INSTRUCTION: 
                    vga_puts(" Instruction");
                    break;
                default: 
                    vga_puts(" Unified    ");
                    break;
            }
            
            vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
            vga_puts("  Size: ");
            vga_putdec(c->size / 1024, 3);
            vga_puts(" KB");
            
            vga_puts("  Ways: ");
            vga_putdec(c->ways, 2);
            
            vga_puts("  Line: ");
            vga_putdec(c->line_size, 3);
            vga_puts(" B");
            
            if(c->fully_associative) vga_puts("  Fully-Assoc");
            if(c->inclusive) vga_puts("  Inclusive");
            vga_puts("\n");
        }
    }

    // Feature flags
    if (flags & CPU_DEBUG_DUMP_FEATURES) {
        vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
        vga_puts("\n=== CPU Features ===\n");
        
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_puts("Feature flags [");
        vga_putdec(sizeof(cpu_features_t)*8, 0);
        vga_puts(" total]:\n");
        
        // Feature print helper
        #define PRINT_FEATURE(name) do { \
            vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK); \
            vga_puts("  • "); \
            vga_set_color(info->features.name ? VGA_COLOR_LIGHT_GREEN : VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK); \
            vga_puts(#name); \
            if(!info->features.name) vga_puts(" (unavailable)"); \
            vga_puts("\n"); \
        } while(0)

        // Basic features
        vga_set_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK);
        vga_puts("\n[ Basic Features ]\n");
        PRINT_FEATURE(fpu);
        PRINT_FEATURE(vme);
        PRINT_FEATURE(de);
        PRINT_FEATURE(pse);
        /* ... continue with all features ... */
        PRINT_FEATURE(sse);
        PRINT_FEATURE(sse2);
        PRINT_FEATURE(sse3);
        PRINT_FEATURE(ssse3);
        PRINT_FEATURE(sse4_1);
        PRINT_FEATURE(sse4_2);
        PRINT_FEATURE(avx);
        PRINT_FEATURE(avx2);

        // Virtualization
        vga_set_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK);
        vga_puts("\n[ Virtualization ]\n");
        PRINT_FEATURE(vmx);
        PRINT_FEATURE(svm);

        // Security
        vga_set_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK);
        vga_puts("\n[ Security Features ]\n");
        PRINT_FEATURE(smep);
        PRINT_FEATURE(smap);
        PRINT_FEATURE(umip);

        // Advanced
        vga_set_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK);
        vga_puts("\n[ Advanced Features ]\n");
        PRINT_FEATURE(avx512f);
        PRINT_FEATURE(avx512dq);
        PRINT_FEATURE(avx512vl);
        PRINT_FEATURE(avx512bw);

        // AMD-specific
        if(strcmp(info->identity.vendor_id, CPU_VENDOR_AMD) == 0) {
            vga_set_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK);
            vga_puts("\n[ AMD Features ]\n");
            PRINT_FEATURE(svm);
            PRINT_FEATURE(topoext);
            PRINT_FEATURE(mwaitx);
        }

        // Intel-specific
        if(strcmp(info->identity.vendor_id, CPU_VENDOR_INTEL) == 0) {
            vga_set_color(VGA_COLOR_LIGHT_MAGENTA, VGA_COLOR_BLACK);
            vga_puts("\n[ Intel Features ]\n");
            PRINT_FEATURE(vmx);
            PRINT_FEATURE(sgx);
            PRINT_FEATURE(intel_pt);
        }

        #undef PRINT_FEATURE
    }

    // Restore original color
    vga_set_color(old_fg, old_bg);
    vga_puts("\n");
}
