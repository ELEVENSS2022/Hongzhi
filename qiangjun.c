#include<stdio.h>
#include<windows.h>
#include<mmsystem.h>
#pragma comment(lib, "winmm.lib")
#include <tchar.h>

typedef NTSTATUS (NTAPI *pNtSetInformationProcess)(
    HANDLE ProcessHandle,
    ULONG ProcessInformationClass,
    PVOID ProcessInformation,
    ULONG ProcessInformationLength
);

FILE* TMP;
char mci[]={
  #embed "qiangjun.mp3"  
};
int main(){
    TMP=fopen("cache.mp3","wb");
    fwrite(mci,sizeof(mci),1,TMP);
    fclose(TMP);
    mciSendStringA("open cache.mp3 alias song",NULL,0,NULL);
    mciSendStringA("play song wait",NULL,0,NULL);
    mciSendStringA("close song",NULL,0,NULL);
    remove("cache.mp3");
    return 0;
}
