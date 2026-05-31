#include<stdio.h>
#include"hwrand.h"
#include"xorshift.h"
#include"mt.h"
#include<windows.h>
int main(){
    init_mt19937((unsigned)hwrand());
    init_xorshift64(hwrand());

    while (1)
    {
        
        printf("%llu\n%u\n%llu\n",hwrand(),mt19937_random(),xorshift64_random());
        Sleep(500);
    }
    
    return 0;
}