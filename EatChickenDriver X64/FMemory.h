#ifndef __FMEMORY_H__
#define __FMEMORY_H__

#include "Common.h"

NTSTATUS KDR_ReadProcessMemory(ReadData readData, PVOID pIoBuffer, ULONG uOutLength);

NTSTATUS KDR_WriteProcessMemory(WriteData writeData);

#endif