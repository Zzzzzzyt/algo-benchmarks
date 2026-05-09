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

inline ll divmod(ll a, ll b, ll m) {
    ll res;
    asm("imul %%rdx\n\t"
        "idiv %%rcx" : "=d"(res) : "a"(a), "d"(b), "c"(m));
    return res;
}

ll quick_pow(ll x, ll a, ll m) {
    ll res = 1;
    for (; a; a >>= 1) {
        if (a & 1) res = divmod(res, x, m);
        x = divmod(x, x, m);
    }
    return res;
}

const ll bases[] = {2, 325, 9375, 28178, 450775, 9780504, 1795265022ll};

// from oi-wiki
bool miller_rabin(ll n) {
    if (n < 3 || n % 2 == 0) return n == 2;
    if (n % 3 == 0) return n == 3;
    ll u = n - 1;
    int t = __builtin_ctzll(u);
    u >>= t;
    for (int i = 0; i < 7; ++i) {
        ll a = bases[i] % n;
        if (a == 0) return true;
        if (a == 1 || a == n - 1) continue;
        ll v = quick_pow(a, u, n);
        if (v == 1) continue;
        int s;
        for (s = 0; s < t; ++s) {
            if (v == n - 1) break;
            v = divmod(v, v, n);
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

    measurement_handle_t st = start_measurement();
    for (ll x = n - repeats2; x < n + repeats2; ++x) {
        DoNotOptimize(isprime_common(x));
    }
    end_measurement(st, "math.isprime.common.random", repeats2 * 2);

    st = start_measurement();
    for (ll x = n - repeats2; x < n + repeats2; ++x) {
        DoNotOptimize(isprime_6kpm(x));
    }
    end_measurement(st, "math.isprime.6kpm.random", repeats2 * 2);

    st = start_measurement();
    for (ll x = n - 500; x < n + 500; ++x) {
        DoNotOptimize(miller_rabin(x));
    }
    end_measurement(st, "math.isprime.miller_rabin.random", 1000);

    int i = 0;
    while (i < 400) {
        ll x = BENCHMARK_N - range + rng() % (range * 2);
        if (miller_rabin(x)) {
            a[i++] = x;
        }
    }

    st = start_measurement();
    for (i = 0; i < repeats; i++) {
        DoNotOptimize(isprime_common(a[i]));
    }
    end_measurement(st, "math.isprime.common.prime", repeats);

    st = start_measurement();
    for (i = 0; i < repeats; i++) {
        DoNotOptimize(isprime_6kpm(a[i]));
    }
    end_measurement(st, "math.isprime.6kpm.prime", repeats);

    st = start_measurement();
    for (i = 0; i < 400; i++) {
        DoNotOptimize(miller_rabin(a[i]));
    }
    end_measurement(st, "math.isprime.miller_rabin.prime", 400);

    return 0;
}
