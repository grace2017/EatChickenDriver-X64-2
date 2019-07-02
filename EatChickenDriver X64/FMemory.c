#include "Common.h"
#include "process.h"
#include "Phytools.h"

NTSTATUS KDR_WriteProcessMemory(WriteData writeData)
{
	NTSTATUS Status = STATUS_SUCCESS;

	// 参数验证
	if (0 == writeData.procId)
	{
		DbgPrint("procId=0 \n");

		return STATUS_UNSUCCESSFUL;
	}

	if (NULL == writeData.lpBaseAddress)
	{
		DbgPrint("lpBaseAddress=NULL \n");

		return STATUS_UNSUCCESSFUL;
	}

	if (NULL == writeData.lpBuffer)
	{
		DbgPrint("lpBuffer=NULL \n");

		return STATUS_UNSUCCESSFUL;
	}

	if (0 == writeData.nSize)
	{
		DbgPrint("nSize=0 \n");

		return STATUS_UNSUCCESSFUL;
	}

	// 计算cr3，映射线性地址
	ULONG64 cr3 = GetProcessCr3ByPid(writeData.procId);
	if (0 == cr3)
	{
		DbgPrint("cr3=0[procId=%d] \n", writeData.procId);

		return STATUS_UNSUCCESSFUL;
	}

	ULONG64 dataVirAddr = 0;
	if (!NT_SUCCESS(GetDataVirtualAddressByCr3(writeData.lpBaseAddress, cr3, &dataVirAddr)))
	{
		DbgPrint("GetDataVirtualAddressByCr3 fail[readAddr=%llx] \n", writeData.lpBaseAddress);

		return STATUS_UNSUCCESSFUL;
	}

	// 写入数据：将R3传入的数据写到指定地址
	memcpy(dataVirAddr, writeData.lpBuffer, writeData.nSize);

	MmUnmapIoSpace(dataVirAddr, 8);

	return Status;
}

NTSTATUS KDR_ReadProcessMemory(ReadData readData, PVOID pIoBuffer, ULONG uOutLength)
{
	NTSTATUS Status = STATUS_SUCCESS;

	// 参数验证
	if (0 == readData.procId)
	{
		DbgPrint("procId=0 \n");

		return STATUS_UNSUCCESSFUL;
	}

	if (NULL == readData.lpBaseAddress)
	{
		DbgPrint("lpBaseAddress=NULL \n");

		return STATUS_UNSUCCESSFUL;
	}

	if (NULL == pIoBuffer)
	{
		DbgPrint("pIoBuffer=NULL \n");

		return STATUS_UNSUCCESSFUL;
	}

	if (0 == uOutLength)
	{
		DbgPrint("uOutLength=0 \n");

		return STATUS_UNSUCCESSFUL;
	}

	// 计算cr3，映射线性地址
	ULONG64 cr3 = GetProcessCr3ByPid(readData.procId);
	if (0 == cr3)
	{
		DbgPrint("cr3=0[procId=%d] \n", readData.procId);

		return STATUS_UNSUCCESSFUL;
	}

	ULONG64 dataVirAddr = 0;
	if (!NT_SUCCESS(GetDataVirtualAddressByCr3(readData.lpBaseAddress, cr3, &dataVirAddr))) 
	{
		DbgPrint("GetDataVirtualAddressByCr3 fail[readAddr=%llx] \n", readData.lpBaseAddress);

		return STATUS_UNSUCCESSFUL;
	}

	// 清空缓冲区
	RtlZeroMemory(pIoBuffer, uOutLength);

	// 向R3写入数据
	memcpy(pIoBuffer, dataVirAddr, uOutLength);

	MmUnmapIoSpace(dataVirAddr, 8);

	return Status;
}
