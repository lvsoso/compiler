#include "stdio.h"

int lrsc_ins(int* lock, int cmp, int lockval); // 声明lrsc_ins函数

int lock = 0;

//初始化锁
void LockInit(int* lock)
{
    *lock = 0;
    return;
}
//加锁
int Locked(int* lock)
{
    int status;
    status = lrsc_ins(lock, 0, 1);
    if(status == 0)
    {
        return 1;//加锁成功
    }
    return 0; //加锁失败
}
//解锁
int UnLock(int* lock)
{
    int status;
    status = lrsc_ins(lock, 1, 0);
    if(status == 0)
    {
        return 1;//解锁成功
    }
    return 0; //解锁失败
}

//定义锁类型
typedef struct Lock
{
    int LockVal;    //锁值
}Lock;

//自旋锁初始化
void SpinLockInit(Lock* lock)
{
    //锁值初始化为0
    lock->LockVal = 0;
    return;
}
//自旋锁加锁
void SpinLock(Lock* lock)
{
    int status;
    do
    {
        status = lrsc_ins(&lock->LockVal, 0, 1); //加锁
    }while(status);  //循环加锁，直到成功
    return;
}
//自旋锁解锁
void SpinUnLock(Lock* lock)
{
    SpinLockInit(lock);//直接初始化 解锁
    return;
}

void testspinlock()
{
    Lock slock;
    SpinLockInit(&slock);
    printf("1 SLock:%d\n", slock.LockVal);
    SpinLock(&slock);
    printf("2 SLock:%d\n", slock.LockVal);
    SpinUnLock(&slock);
    printf("3 SLock:%d\n", slock.LockVal);
    SpinLock(&slock);
    printf("4 SLock:%d\n", slock.LockVal);
    SpinUnLock(&slock);
    printf("5 SLock:%d\n", slock.LockVal);
    return;
}