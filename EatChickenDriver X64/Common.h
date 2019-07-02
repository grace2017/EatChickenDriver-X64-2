#ifndef __COMMON_H_
#define __COMMON_H_

#include <ntifs.h>

#include "Cr3Register.h"

typedef struct
{
	ULONG64		procId;				// 进程ID
	PULONG64	lpBaseAddress;		// 从这个地址开始读
}ReadData, *PReadData;

typedef struct
{
	ULONG64		procId;				// 进程ID
	PULONG64	lpBaseAddress;		// 从这个地址开始读
	PULONG64	lpBuffer;			// 写的数据
	ULONG64		nSize;				// 写多少字节
}WriteData, *PWriteData;

// 定义各种回调函数原型
typedef VOID(*Type_TestCallFun)(IN ULONG64 process);

#endif