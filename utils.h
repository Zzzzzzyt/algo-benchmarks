// The following macros are taken from https://github.com/google/benchmark
// from the file include/benchmark/benchmark.h

#include <cmath>
#include <cstdio>
#include <cstring>
#include <type_traits>

// Used to annotate functions, methods and classes so they
// are not optimized by the compiler. Useful for tests
// where you expect loops to stay in place churning cycles
#if defined(__clang__)
#define BENCHMARK_DONT_OPTIMIZE __attribute__((optnone))
#elif defined(__GNUC__) || defined(__GNUG__)
#define BENCHMARK_DONT_OPTIMIZE __attribute__((optimize(0)))
#else
// MSVC & Intel do not have a no-optimize attribute, only line pragmas
#define BENCHMARK_DONT_OPTIMIZE
#endif

#if defined(__GNUC__) || defined(__clang__)
#define BENCHMARK_ALWAYS_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER) && !defined(__clang__)
#define BENCHMARK_ALWAYS_INLINE __forceinline
#define __func__ __FUNCTION__
#else
#define BENCHMARK_ALWAYS_INLINE
#endif

#if (!defined(__GNUC__) && !defined(__clang__)) || defined(__pnacl__) || \
    defined(__EMSCRIPTEN__)
#define BENCHMARK_HAS_NO_INLINE_ASSEMBLY
#endif

// The DoNotOptimize(...) function can be used to prevent a value or
// expression from being optimized away by the compiler. This function is
// intended to add little to no overhead.
// See: https://youtu.be/nXaxk27zwlk?t=2441
#ifndef BENCHMARK_HAS_NO_INLINE_ASSEMBLY
#if !defined(__GNUC__) || defined(__llvm__) || defined(__INTEL_COMPILER)
// template <class Tp>
// BENCHMARK_DEPRECATED_MSG(
//     "The const-ref version of this method can permit "
//     "undesired compiler optimizations in benchmarks")
// inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp const& value) {
//   asm volatile("" : : "r,m"(value) : "memory");
// }

template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp &value) {
#if defined(__clang__)
    asm volatile("" : "+r,m"(value) : : "memory");
#else
    asm volatile("" : "+m,r"(value) : : "memory");
#endif
}

template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp &&value) {
#if defined(__clang__)
    asm volatile("" : "+r,m"(value) : : "memory");
#else
    asm volatile("" : "+m,r"(value) : : "memory");
#endif
}
// !defined(__GNUC__) || defined(__llvm__) || defined(__INTEL_COMPILER)
#elif (__GNUC__ >= 5)
// Workaround for a bug with full argument copy overhead with GCC.
// See: #1340 and https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105519
// template <class Tp>
// BENCHMARK_DEPRECATED_MSG(
//     "The const-ref version of this method can permit "
//     "undesired compiler optimizations in benchmarks")
// inline BENCHMARK_ALWAYS_INLINE
//     typename std::enable_if<std::is_trivially_copyable<Tp>::value &&
//                             (sizeof(Tp) <= sizeof(Tp*))>::type
//     DoNotOptimize(Tp const& value) {
//   asm volatile("" : : "r,m"(value) : "memory");
// }

// template <class Tp>
// BENCHMARK_DEPRECATED_MSG(
//     "The const-ref version of this method can permit "
//     "undesired compiler optimizations in benchmarks")
// inline BENCHMARK_ALWAYS_INLINE
//     typename std::enable_if<!std::is_trivially_copyable<Tp>::value ||
//                             (sizeof(Tp) > sizeof(Tp*))>::type
//     DoNotOptimize(Tp const& value) {
//   asm volatile("" : : "m"(value) : "memory");
// }

template <class Tp>
inline BENCHMARK_ALWAYS_INLINE
    typename std::enable_if<std::is_trivially_copyable<Tp>::value &&
                            (sizeof(Tp) <= sizeof(Tp *))>::type
    DoNotOptimize(Tp &value) {
    asm volatile("" : "+m,r"(value) : : "memory");
}

template <class Tp>
inline BENCHMARK_ALWAYS_INLINE
    typename std::enable_if<!std::is_trivially_copyable<Tp>::value ||
                            (sizeof(Tp) > sizeof(Tp *))>::type
    DoNotOptimize(Tp &value) {
    asm volatile("" : "+m"(value) : : "memory");
}

template <class Tp>
inline BENCHMARK_ALWAYS_INLINE
    typename std::enable_if<std::is_trivially_copyable<Tp>::value &&
                            (sizeof(Tp) <= sizeof(Tp *))>::type
    DoNotOptimize(Tp &&value) {
    asm volatile("" : "+m,r"(value) : : "memory");
}

template <class Tp>
inline BENCHMARK_ALWAYS_INLINE
    typename std::enable_if<!std::is_trivially_copyable<Tp>::value ||
                            (sizeof(Tp) > sizeof(Tp *))>::type
    DoNotOptimize(Tp &&value) {
    asm volatile("" : "+m"(value) : : "memory");
}
// !defined(__GNUC__) || defined(__llvm__) || defined(__INTEL_COMPILER)
#endif

#elif defined(_MSC_VER)
// template <class Tp>
// BENCHMARK_DEPRECATED_MSG(
//     "The const-ref version of this method can permit "
//     "undesired compiler optimizations in benchmarks")
// inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp const& value) {
//   internal::UseCharPointer(&reinterpret_cast<char const volatile&>(value));
//   _ReadWriteBarrier();
// }

template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp &value) {
    internal::UseCharPointer(&reinterpret_cast<char const volatile &>(value));
    _ReadWriteBarrier();
}

template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp &&value) {
    internal::UseCharPointer(&reinterpret_cast<char const volatile &>(value));
    _ReadWriteBarrier();
}
#else
template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp &&value) {
    internal::UseCharPointer(&reinterpret_cast<char const volatile &>(value));
}
// FIXME Add ClobberMemory() for non-gnu and non-msvc compilers, before C++11.
#endif

#ifndef BENCHMARK_N
#define BENCHMARK_N 1024
#endif

#ifndef BENCHMARK_MICRO_REPEATS
#define BENCHMARK_MICRO_REPEATS 1
#endif

#define BENCHMARK_COMPILER_BARRIER asm volatile("" : : : "memory")

#include <linux/perf_event.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

inline BENCHMARK_ALWAYS_INLINE __u64 get_cpu_time() {
    BENCHMARK_COMPILER_BARRIER;
    timespec ts;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    return ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

inline BENCHMARK_ALWAYS_INLINE __u64 get_monotonic_time() {
    BENCHMARK_COMPILER_BARRIER;
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

#include <x86intrin.h>

inline BENCHMARK_ALWAYS_INLINE __u64 get_tsc() {
    BENCHMARK_COMPILER_BARRIER;
    unsigned int aux;
    return __rdtscp(&aux);
}

#ifndef BENCHMARK_TSC_FREQ
#define BENCHMARK_TSC_FREQ 3.0e9
#endif

inline double tsc_to_ns(__u64 tsc) {
    return (double)tsc / (BENCHMARK_TSC_FREQ);
}

struct measurement_counters_t {
    double cpu_cycles;
    double stalled_cycles_frontend;
    double branches;
    double branch_misses;
    double context_switches;
    double page_faults;
    double l1i_cache_references;
    double l1i_cache_misses;
    double l1d_cache_references;
    double l1d_cache_misses;
    double llc_cache_references;
    double llc_cache_misses;
};

enum perf_counter_index_t {
    PERF_IDX_BRANCHES = 0,
    PERF_IDX_BRANCH_MISSES,
    PERF_IDX_CONTEXT_SWITCHES,
    PERF_IDX_PAGE_FAULTS,
    PERF_IDX_L1I_CACHE_REFERENCES,
    PERF_IDX_L1I_CACHE_MISSES,
    PERF_IDX_L1D_CACHE_REFERENCES,
    PERF_IDX_L1D_CACHE_MISSES,
    PERF_IDX_LLC_CACHE_REFERENCES,
    PERF_IDX_LLC_CACHE_MISSES,
    PERF_IDX_COUNT
};

struct measurement_handle_t {
    __u64 system_time_clock;
    __u64 tsc_time;
    int fds[PERF_IDX_COUNT];
};

inline int perf_event_open_wrapper(struct perf_event_attr *attr, pid_t pid, int cpu, int group_fd, unsigned long flags) {
    return (int)syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);
}

int open_perf_event(__u32 type, __u64 config, bool exclude_kernel) {
    struct perf_event_attr attr;
    memset(&attr, 0, sizeof(attr));
    attr.type = type;
    attr.size = sizeof(attr);
    attr.config = config;
    attr.disabled = 1;
    attr.read_format = PERF_FORMAT_TOTAL_TIME_ENABLED | PERF_FORMAT_TOTAL_TIME_RUNNING;
    attr.exclude_kernel = exclude_kernel ? 1 : 0;
    attr.exclude_hv = exclude_kernel ? 1 : 0;
    attr.inherit = 0;
    return perf_event_open_wrapper(&attr, 0, -1, -1, 0);
}

double read_perf_counter(int fd) {
    // struct read_format {
    //     u64 value;         /* The value of the event */
    //     u64 time_enabled;  /* if PERF_FORMAT_TOTAL_TIME_ENABLED */
    //     u64 time_running;  /* if PERF_FORMAT_TOTAL_TIME_RUNNING */
    //     u64 id;            /* if PERF_FORMAT_ID */
    //     u64 lost;          /* if PERF_FORMAT_LOST */
    // };

    if (fd < 0) {
        return 0.0;
    }

    struct {
        __u64 value;
        __u64 time_enabled;
        __u64 time_running;
    } buf;

    size_t sz = read(fd, &buf, sizeof(buf));
    if (sz != sizeof(buf)) {
        return 0.0;
    }
    return (double)buf.value * ((double)buf.time_enabled / buf.time_running);
}

inline measurement_counters_t read_measurement_counters(measurement_handle_t handle) {
    measurement_counters_t counters;
    double buf[2];

    counters.branches = read_perf_counter(handle.fds[PERF_IDX_BRANCHES]);
    counters.branch_misses = read_perf_counter(handle.fds[PERF_IDX_BRANCH_MISSES]);
    counters.context_switches = read_perf_counter(handle.fds[PERF_IDX_CONTEXT_SWITCHES]);
    counters.page_faults = read_perf_counter(handle.fds[PERF_IDX_PAGE_FAULTS]);
    counters.l1i_cache_references = read_perf_counter(handle.fds[PERF_IDX_L1I_CACHE_REFERENCES]);
    counters.l1i_cache_misses = read_perf_counter(handle.fds[PERF_IDX_L1I_CACHE_MISSES]);
    counters.l1d_cache_references = read_perf_counter(handle.fds[PERF_IDX_L1D_CACHE_REFERENCES]);
    counters.l1d_cache_misses = read_perf_counter(handle.fds[PERF_IDX_L1D_CACHE_MISSES]);
    counters.llc_cache_references = read_perf_counter(handle.fds[PERF_IDX_LLC_CACHE_REFERENCES]);
    counters.llc_cache_misses = read_perf_counter(handle.fds[PERF_IDX_LLC_CACHE_MISSES]);

    return counters;
}

inline BENCHMARK_ALWAYS_INLINE measurement_handle_t start_measurement() {
    measurement_handle_t handle;

    handle.fds[PERF_IDX_BRANCHES] = open_perf_event(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_INSTRUCTIONS, true);
    handle.fds[PERF_IDX_BRANCH_MISSES] = open_perf_event(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES, true);

    handle.fds[PERF_IDX_CONTEXT_SWITCHES] = open_perf_event(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CONTEXT_SWITCHES, false);
    handle.fds[PERF_IDX_PAGE_FAULTS] = open_perf_event(PERF_TYPE_SOFTWARE, PERF_COUNT_SW_PAGE_FAULTS, false);

    handle.fds[PERF_IDX_L1I_CACHE_REFERENCES] = open_perf_event(
        PERF_TYPE_HW_CACHE,
        PERF_COUNT_HW_CACHE_L1I | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
            (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16),
        true);
    handle.fds[PERF_IDX_L1I_CACHE_MISSES] = open_perf_event(
        PERF_TYPE_HW_CACHE,
        PERF_COUNT_HW_CACHE_L1I | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
            (PERF_COUNT_HW_CACHE_RESULT_MISS << 16),
        true);

    handle.fds[PERF_IDX_L1D_CACHE_REFERENCES] = open_perf_event(
        PERF_TYPE_HW_CACHE,
        PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
            (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16),
        true);
    handle.fds[PERF_IDX_L1D_CACHE_MISSES] = open_perf_event(
        PERF_TYPE_HW_CACHE,
        PERF_COUNT_HW_CACHE_L1D | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
            (PERF_COUNT_HW_CACHE_RESULT_MISS << 16),
        true);

    handle.fds[PERF_IDX_LLC_CACHE_REFERENCES] = open_perf_event(
        PERF_TYPE_HW_CACHE,
        PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
            (PERF_COUNT_HW_CACHE_RESULT_ACCESS << 16),
        true);
    handle.fds[PERF_IDX_LLC_CACHE_MISSES] = open_perf_event(
        PERF_TYPE_HW_CACHE,
        PERF_COUNT_HW_CACHE_LL | (PERF_COUNT_HW_CACHE_OP_READ << 8) |
            (PERF_COUNT_HW_CACHE_RESULT_MISS << 16),
        true);

    for (int i = 0; i < PERF_IDX_COUNT; ++i) {
        if (handle.fds[i] >= 0) {
            ioctl(handle.fds[i], PERF_EVENT_IOC_RESET, 0);
        }
    }

    for (int i = 0; i < PERF_IDX_COUNT; ++i) {
        if (handle.fds[i] >= 0) {
            ioctl(handle.fds[i], PERF_EVENT_IOC_ENABLE, 0);
        }
    }

    handle.system_time_clock = get_monotonic_time();
    handle.tsc_time = get_tsc();
    return handle;
}

inline BENCHMARK_ALWAYS_INLINE void end_measurement(const measurement_handle_t &handle, const char *test_name, int micro_repeats) {
    __u64 end_tsc = get_tsc();
    __u64 end_system_time = get_monotonic_time();

    for (int i = 0; i < PERF_IDX_COUNT; ++i) {
        ioctl(handle.fds[i], PERF_EVENT_IOC_DISABLE, 0);
    }

    __u64 elapsed_clock_ns = end_system_time - handle.system_time_clock;
    double elapsed_tsc_ns = tsc_to_ns(end_tsc - handle.tsc_time);

    if (micro_repeats <= 0) {
        micro_repeats = 1;
    }

    measurement_counters_t c = read_measurement_counters(handle);
    std::printf(
        "%s:\t%llu %d %llu %.17g %.17g %.17g %.17g %.17g %.17g %.17g %.17g %.17g %.17g %.17g\n",
        test_name,
        (__u64)(BENCHMARK_N),
        micro_repeats,
        elapsed_clock_ns,
        elapsed_tsc_ns,
        c.branches,
        c.branch_misses,
        c.context_switches,
        c.page_faults,
        c.l1i_cache_references,
        c.l1i_cache_misses,
        c.l1d_cache_references,
        c.l1d_cache_misses,
        c.llc_cache_references,
        c.llc_cache_misses);
}

#ifdef BENCHMARK_PROCESS_PRIORITY
#include <limits.h>
#include <sys/resource.h>
#endif

#ifdef BENCHMARK_CPU_AFFINITY
#include <sched.h>
#endif

inline BENCHMARK_ALWAYS_INLINE void benchmark_init(int argc, char *argv[]) {
#ifdef BENCHMARK_PROCESS_PRIORITY
#if BENCHMARK_PROCESS_PRIORITY == 20
    setpriority(PRIO_PROCESS, 0, -NZERO);
#elif BENCHMARK_PROCESS_PRIORITY == 99
    sched_param param;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    sched_setscheduler(0, SCHED_FIFO, &param);
#endif
#endif
#ifdef BENCHMARK_CPU_AFFINITY
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(BENCHMARK_CPU_AFFINITY, &cpu_set);
    sched_setaffinity(0, sizeof(cpu_set), &cpu_set);
#endif
}

#include <random>
#include <algorithm>

std::minstd_rand rng(time(NULL));

inline BENCHMARK_ALWAYS_INLINE __u64 rng64() {
    return rng() * 2147483647ll + rng();
}

std::vector<int> make_shuffled_ints(int n) {
    std::vector<int> values(n);
    for (int i = 0; i < n; ++i) {
        values[i] = i;
    }
    std::shuffle(values.begin(), values.end(), rng);
    return values;
}
