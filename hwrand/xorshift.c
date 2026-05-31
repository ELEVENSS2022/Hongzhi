#include "xorshift.h"

// 全局状态
static uint64_t state;

// 初始化种子（如果种子为0，则设为默认值）
void init_xorshift64(uint64_t seed) {
    if (seed == 0) {
        seed = 88172645463325252ULL;  // 非零默认种子
    }
    state = seed;
}

// xorshift64 核心算法（基础版本，无乘法）
uint64_t xorshift64_random(void) {
    uint64_t x = state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    state = x;
    return x * 2685821657736338717ULL;
}
