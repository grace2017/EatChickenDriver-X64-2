#include "Common.h"
#include "process.h"

NTSTATUS FReadProcessMemoryByName(IN PSTR procName, IN PVOID64 fromAddr, IN ULONG64 NumberOfBytesToRead, OUT PVOID64 Buffer)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PVOID64 buff = NULL;
	ULONG64 currentCr3 = 0;
	ULONG64 otherCr3 = 0;

	if ((NULL == fromAddr) || (NULL == Buffer) || (0 == NumberOfBytesToRead))
	{
		DbgPrint("params error \n");

		return STATUS_UNSUCCESSFUL;
	}

	//----------
	buff = ExAllocatePool(NonPagedPool, NumberOfBytesToRead);
	if (NULL == buff)
	{
		DbgPrint("ExAllocatePool failed \n");

		return STATUS_UNSUCCESSFUL;
	}

	RtlZeroMemory(buff, NumberOfBytesToRead);

	currentCr3 = GetCurrentProcessCR3();
	if (0 == currentCr3)
	{
		DbgPrint("GetCurrentProcessCR3 failed(result: 0) \n");

		ExFreePool(buff);

		return STATUS_UNSUCCESSFUL;
	}

	//----------
	otherCr3 = GetProcessCr3ByName(procName);
	if (0 == otherCr3)
	{
		DbgPrint("GetProcessDirectoryTableBase failed(result: 0) \n");

		ExFreePool(buff);

		return STATUS_UNSUCCESSFUL;
	}

	//----------将其他进程的内存读入内核空间刚申请的内存中
	SetCurrentProcessCR3(otherCr3);

	RtlCopyMemory(buff, fromAddr, NumberOfBytesToRead);

	SetCurrentProcessCR3(currentCr3);

	//----------
	RtlCopyMemory(Buffer, buff, NumberOfBytesToRead);

	//----------
	ExFreePool(buff);

	return Status;
}