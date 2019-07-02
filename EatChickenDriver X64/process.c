#include "process.h"

/*
遍历进程
*/
VOID TraverseProcess()
{
	ANSI_STRING tmpProcessName = { 0 };

	ULONG64 currentProcess = 0;
	ULONG64 tmpProcess = 0;

	PCSTR processNameAddr = 0;

	PLIST_ENTRY pProcessListHead = NULL;
	PLIST_ENTRY pProcessNext = NULL;

	currentProcess = (ULONG64)PsGetCurrentProcess();
	tmpProcess = currentProcess;

	pProcessListHead = (PLIST_ENTRY)(currentProcess + 0x188);
	pProcessNext = pProcessListHead;

	do {
		processNameAddr = (PCSTR)(tmpProcess + 0x2e0);
		RtlInitAnsiString(&tmpProcessName, processNameAddr);

		if (0 == *(PUCHAR)processNameAddr) {
			DbgPrint("进程名称为空\n");
		}
		else
		{
			DbgPrint("进程名称地址：%s \n", tmpProcessName.Buffer);
		}

		pProcessNext = pProcessNext->Blink;
		tmpProcess = (ULONG64)pProcessNext - 0x188;
	} while (pProcessListHead != pProcessNext);
}

/*
	根据进程名获取eprocess
*/
ULONG64 GetProcessByName(IN PSTR procName)
{
	ULONG64 result = NULL;

	if (NULL == procName)
	{
#ifdef _START_DEBUG
		DbgPrint("GetProcessByName: procName=NULL");
#endif

		return result;
	}

	ANSI_STRING cutProcessName = { 0 };
	ANSI_STRING tmpProcessName = { 0 };

	ULONG64 currentProcess = 0;
	ULONG64 tmpProcess = 0;

	PCSTR processNameAddr = 0;

	PLIST_ENTRY pProcessListHead = NULL;
	PLIST_ENTRY pProcessNext = NULL;

	RtlInitAnsiString(&cutProcessName, procName);

	currentProcess = (ULONG64)PsGetCurrentProcess();
	tmpProcess = currentProcess;

	pProcessListHead = (PLIST_ENTRY)(currentProcess + 0x188);
	pProcessNext = pProcessListHead;

	do {
		processNameAddr = (PCSTR)(tmpProcess + 0x2e0);
		RtlInitAnsiString(&tmpProcessName, processNameAddr);

		if (0 == *(PUCHAR)processNameAddr) {
#ifdef _START_DEBUG
			DbgPrint("进程名称为空, pass \n");
#endif
		}
		else
		{
#ifdef _START_DEBUG
			DbgPrint("进程名称：%s \n", tmpProcessName.Buffer);
#endif

			if (0 == RtlCompareString(&cutProcessName, &tmpProcessName, FALSE)) {
				return tmpProcess;
			}
		}

		pProcessNext = pProcessNext->Blink;
		tmpProcess = (ULONG64)pProcessNext - 0x188;
	} while (pProcessListHead != pProcessNext);

#ifdef _START_DEBUG
	DbgPrint("进程[%s]不存在，可能被断链、未启动 \n", procName);
#endif

	return result;
}

ULONG64 GetProcessCr3ByName(IN PSTR procName)
{
	ULONG64 result = NULL;

	if (NULL == procName)
	{
#ifdef _START_DEBUG
		DbgPrint("GetProcessByName: procName=NULL");
#endif

		return result;
	}

	ANSI_STRING cutProcessName = { 0 };
	ANSI_STRING tmpProcessName = { 0 };

	ULONG64 currentProcess = 0;
	ULONG64 tmpProcess = 0;

	PCSTR processNameAddr = 0;

	PLIST_ENTRY pProcessListHead = NULL;
	PLIST_ENTRY pProcessNext = NULL;

	RtlInitAnsiString(&cutProcessName, procName);

	currentProcess = (ULONG64)PsGetCurrentProcess();
	tmpProcess = currentProcess;

	pProcessListHead = (PLIST_ENTRY)(currentProcess + 0x188);
	pProcessNext = pProcessListHead;

	do {
		processNameAddr = (PCSTR)(tmpProcess + 0x2e0);
		RtlInitAnsiString(&tmpProcessName, processNameAddr);

		if (0 == *(PUCHAR)processNameAddr) {
#ifdef _START_DEBUG
			DbgPrint("进程名称为空, pass \n");
#endif
		}
		else
		{
#ifdef _START_DEBUG
			DbgPrint("进程名称：%s \n", tmpProcessName.Buffer);
#endif

			if (0 == RtlCompareString(&cutProcessName, &tmpProcessName, FALSE)) {
				PULONG64 pDirectoryTableBase = tmpProcess + 0x28;

				return *pDirectoryTableBase;
			}
		}

		pProcessNext = pProcessNext->Blink;
		tmpProcess = (ULONG64)pProcessNext - 0x188;
	} while (pProcessListHead != pProcessNext);

#ifdef _START_DEBUG
	DbgPrint("进程[%s]不存在，可能被断链、未启动 \n", procName);
#endif

	return result;
}

ULONG64 GetProcessCr3ByPid(IN ULONG64 pid)
{
	ULONG64 result = NULL;

	if (pid <= 0)
	{
#ifdef _START_DEBUG
		DbgPrint("GetProcessCr3ByPid: pid=%d \n", pid);
#endif

		return result;
	}

	ULONG64 currentProcess = 0;
	ULONG64 tmpProcess = 0;

	PLIST_ENTRY pProcessListHead = NULL;
	PLIST_ENTRY pProcessNext = NULL;

	currentProcess = (ULONG64)PsGetCurrentProcess();
	tmpProcess = currentProcess;

	pProcessListHead = (PLIST_ENTRY)(currentProcess + 0x188);
	pProcessNext = pProcessListHead;

	do {
		if (pid == *(PULONG64)(tmpProcess + 0x180)) {
			return *(PULONG64)(tmpProcess + 0x28);
		}

		pProcessNext = pProcessNext->Blink;
		tmpProcess = (ULONG64)pProcessNext - 0x188;
	} while (pProcessListHead != pProcessNext);

#ifdef _START_DEBUG
	DbgPrint("进程[%d]不存在，可能被断链、未启动 \n", pid);
#endif

	return result;
}

VOID IsNotepad(IN Type_TestCallFun callFun)
{
	ANSI_STRING cutProcessName = { 0 };
	ANSI_STRING tmpProcessName = { 0 };

	ULONG64 currentProcess = 0;
	ULONG64 tmpProcess = 0;

	PCSTR processNameAddr = 0;

	PLIST_ENTRY pProcessListHead = NULL;
	PLIST_ENTRY pProcessNext = NULL;

	RtlInitAnsiString(&cutProcessName, "TslGame.exe");

	currentProcess = (ULONG64)PsGetCurrentProcess();
	tmpProcess = currentProcess;

	pProcessListHead = (PLIST_ENTRY)(currentProcess + 0x188);
	pProcessNext = pProcessListHead;

	do {
		processNameAddr = (PCSTR)(tmpProcess + 0x2e0);
		RtlInitAnsiString(&tmpProcessName, processNameAddr);

		if (0 != *(PUCHAR)processNameAddr) {
			if (0 == RtlCompareString(&cutProcessName, &tmpProcessName, FALSE)) {
				callFun(tmpProcess);
			}
		}

		pProcessNext = pProcessNext->Blink;
		tmpProcess = (ULONG64)pProcessNext - 0x188;
	} while (pProcessListHead != pProcessNext);
}

