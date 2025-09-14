#include "utils.h"
#include <algorithm>
#include <math.h>
#include <stdio.h>

typedef long long ll;

bool isprime_common(ll n) {
    if (n < 2) return false;
    for (ll i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

bool isprime_6kpm(ll n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    for (ll i = 5; i <= llround(sqrt((double)n)); i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return false;
    }
    return true;
}

ll quick_pow(__int128_t x, ll a, ll m) {
    __int128_t res = 1;
    for (; a; a >>= 1) {
        if (a & 1) res = (res * x) % m;
        x = (x * x) % m;
    }
    return res;
}

const ll bases[] = {2, 325, 9375, 28178, 450775, 9780504, 1795265022ll};

// from oi-wiki
bool miller_rabin(ll n) {
    if (n < 3 || n % 2 == 0) return n == 2;
    if (n % 3 == 0) return n == 3;
    ll u = n - 1, t = 0;
    while (u % 2 == 0)
        u /= 2, ++t;
    for (int i = 0; i < 7; ++i) {
        ll a = bases[i] % n;
        if (a == 0) return true;
        if (a == 1 || a == n - 1) continue;
        ll v = quick_pow(a, u, n);
        if (v == 1) continue;
        int s;
        for (s = 0; s < t; ++s) {
            if (v == n - 1) break;
            v = __int128_t(v) * v % n;
        }
        if (s == t) return false;
    }
    return true;
}

ll a[405];

int main(int argc, char *argv[]) {
    benchmark_init(argc, argv);

    ll range = std::min(1ll << 28, (ll)BENCHMARK_N / 100);

    ll n = BENCHMARK_N - range + rng() % (range * 2);
    int repeats = std::max(1, std::min(100, (int)(1000000 / sqrt(n))));
    int repeats2 = std::max(20, std::min(500, (int)(10000000 / sqrt(n))));

    ull st = get_cpu_time();
    for (ll x = n - repeats2; x < n + repeats2; ++x) {
        DoNotOptimize(isprime_common(x));
    }
    ull et = get_cpu_time();

    printf("math.isprime.common.random:\t%lld %d %llu\n", (ll)BENCHMARK_N, repeats2 * 2, et - st);

    st = get_cpu_time();
    for (ll x = n - repeats2; x < n + repeats2; ++x) {
        DoNotOptimize(isprime_6kpm(x));
    }
    et = get_cpu_time();

    printf("math.isprime.6kpm.random:\t%lld %d %llu\n", (ll)BENCHMARK_N, repeats2 * 2, et - st);

    st = get_cpu_time();
    for (ll x = n - 500; x < n + 500; ++x) {
        DoNotOptimize(miller_rabin(x));
    }
    et = get_cpu_time();

    printf("math.isprime.miller_rabin.random:\t%lld %d %llu\n", (ll)BENCHMARK_N, 1000, et - st);

    int i = 0;
    while (i < 400) {
        ll x = BENCHMARK_N - range + rng() % (range * 2);
        if (miller_rabin(x)) {
            a[i++] = x;
        }
    }

    st = get_cpu_time();
    for (i = 0; i < repeats; i++) {
        DoNotOptimize(isprime_common(a[i]));
    }
    et = get_cpu_time();
    printf("math.isprime.common.prime:\t%lld %d %llu\n", (ll)BENCHMARK_N, repeats, et - st);

    st = get_cpu_time();
    for (i = 0; i < repeats; i++) {
        DoNotOptimize(isprime_6kpm(a[i]));
    }
    et = get_cpu_time();
    printf("math.isprime.6kpm.prime:\t%lld %d %llu\n", (ll)BENCHMARK_N, repeats, et - st);

    st = get_cpu_time();
    for (i = 0; i < 400; i++) {
        DoNotOptimize(miller_rabin(a[i]));
    }
    et = get_cpu_time();
    printf("math.isprime.miller_rabin.prime:\t%lld %d %llu\n", (ll)BENCHMARK_N, 400, et - st);

    return 0;
}
