#ifndef __FMEMORY_H__
#define __FMEMORY_H__

#include "Common.h"

NTSTATUS FReadProcessMemoryByName(IN PSTR procName, IN PVOID64 fromAddr, IN ULONG64 readSize, OUT PVOID64 Buffer);

NTSTATUS FWriteProcessMemory(IN PVOID64 fromAddr, IN ULONG64 writeSize, IN PVOID64 Buffer);

#endif