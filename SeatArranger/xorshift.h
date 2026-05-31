#ifndef XORSHIFT_H
#define XORSHIFT_H

#include <stdint.h>
extern "C"{
// 初始化种子
void init_xorshift64(uint64_t seed);

// 生成下一个64位随机数
uint64_t xorshift64_random(void);
}
#endif
