#ifndef __CR3_REGISTER_H__
#define __CR3_REGISTER_H__
#include <ntifs.h>

extern ULONG64 GetCurrentProcessCR3();

extern VOID SetCurrentProcessCR3(IN ULONG64 cr3Val);

#endif