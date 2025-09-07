#include <time.h>

typedef unsigned long long ull;
typedef long long ll;

#ifndef BENCHMARK_N
#define BENCHMARK_N 1024
#endif

#ifndef BENCHMARK_MICRO_REPEATS
#define BENCHMARK_MICRO_REPEATS 1
#endif

#include <random>

std::minstd_rand rng(time(NULL));

// The following macros are taken from https://github.com/google/benchmark
// from the file include/benchmark/benchmark.h

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

inline BENCHMARK_ALWAYS_INLINE ull get_cpu_time() {
    timespec ts;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
    return ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

inline BENCHMARK_ALWAYS_INLINE ull get_monotonic_time() {
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000ull + ts.tv_nsec;
}

#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

#define MEMORY_BARRIER asm volatile("" : : : "memory")

inline BENCHMARK_ALWAYS_INLINE ull get_tsc() {
    MEMORY_BARRIER;
    return __rdtsc();
}

#ifndef BENCHMARK_TSC_FREQ
#define BENCHMARK_TSC_FREQ 3.0e9
#endif

ull tsc_to_ns(ull tsc) {
    return (ull)(tsc / (double)(BENCHMARK_TSC_FREQ));
}

#ifdef BENCHMARK_HIGH_PRIORITY
#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <limits.h>
#include <sys/resource.h>
#endif
#endif

void benchmark_init(int argc, char *argv[]) {
#ifdef BENCHMARK_HIGH_PRIORITY
#if defined(_WIN32)
    SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
#elif defined(__linux__)
    setpriority(PRIO_PROCESS, 0, -NZERO);
#endif
#endif
}