#include "Phytools.h"

HardwarePte * NTAPI GetPxeAddress(IN PVOID VirtualAddress)
{
	return (HardwarePte *)(PXE_BASE + (MiGetPxeOffset(VirtualAddress) * 8));
}

HardwarePte * NTAPI GetPpeAddress(IN PVOID VirtualAddress)
{
	return (HardwarePte *)(PPE_BASE + (((ULONG64)VirtualAddress >> 27) & 0x1FFFF8));
}

HardwarePte * NTAPI GetPdeAddress(IN PVOID VirtualAddress)
{
	return (HardwarePte *)((((ULONG64)VirtualAddress >> 18) & 0x3FFFFFF8) - 0x904C0000000);
}

HardwarePte * NTAPI GetPteAddress(IN PVOID VirtualAddress)
{
	return (HardwarePte *)((((ULONG64)VirtualAddress >> 9) & 0x7FFFFFFFF8) - 0x98000000000);
}

void SetAddrToLineAddr(PVOID addr, ULONG64 lineAddr,ULONG sizeOfImage)
{
	//挂物理页
	HardwarePte * pxe = GetPxeAddress(addr);
	HardwarePte * ppe = GetPpeAddress(addr);
	HardwarePte * pde = GetPdeAddress(addr);
	HardwarePte * pte = GetPteAddress(addr);

	//2M大页
	if (pde->large_page)
	{
		pxe->valid = 1;
		ppe->valid = 1;
		pde->valid = 1;

		pxe->owner = 1;
		ppe->owner = 1;
		pde->owner = 1;


		ppe->write = 1;
		pde->write = 1;
	}
	else
	{
		pxe->valid = 1;
		ppe->valid = 1;
		pde->valid = 1;

		pxe->owner = 1;
		ppe->owner = 1;
		pde->owner = 1;
		pte->valid = 1;
		pte->owner = 1;
		pde->write = 1;
		pte->write = 1;

		pte->no_execute = 0;
	}

	PMMPTE PxeAddress = GetPxeAddress((PVOID)lineAddr);
	PMMPTE PpeAddress = GetPpeAddress((PVOID)lineAddr);
	//PMMPTE PdeAddress = GetPdeAddress((PVOID)lineAddr);
	//PMMPTE PteAddress = GetPteAddress((PVOID)lineAddr);
	
	//挂物理页
	if (PxeAddress && PxeAddress->page_frame)
	{
		PxeAddress->user = 1;
	}
	else
	{
		memcpy(PxeAddress, pxe, sizeof(MMPTE));
		PxeAddress->user = 1;
	}

	if (PpeAddress  && PpeAddress->page_frame)
	{
		PpeAddress->user = 1;
	}
	else
	{
		memcpy(PpeAddress, ppe, sizeof(MMPTE));
		PpeAddress->user = 1;
	}

	ULONG count = (sizeOfImage % 0x200000) == 0 ? 0 : 1;
	count += sizeOfImage / 0x200000;

	for (ULONG i = 0; i < count; i++)
	{

		HardwarePte * pde1 = GetPdeAddress((PVOID)((ULONG64)addr + 0x200000 * i));
		pde1->owner = 1;
		pde1->write = 1;
		HardwarePte * pde2 = GetPdeAddress((PVOID)(lineAddr + 0x200000 * i));
		memcpy(pde2, pde1, sizeof(HardwarePte));
		pde2->no_execute = 0;
		
		//判断 pde1是否是大页 
		if (!pde1->large_page)
		{
			//修复
			
			//映射物理地址
			PHYSICAL_ADDRESS phyAddr = {0};
			phyAddr.QuadPart = (*(PULONG64)pde1) & 0xfffffffffffff000l;
			ULONG64 * lineaddr = MmMapIoSpace(phyAddr, PAGE_SIZE, MmNonCached);
			 
			for (int j = 0; j < 0x1FF; j++)
			{
				//HardwarePte * pte = GetPteAddress((PVOID)(((ULONG64)addr + 0x200000 * i) + (j << 3)));
				HardwarePte * pte = (HardwarePte *)(lineaddr + j);
				
				if (pte && MmIsAddressValid(pte))
				{
					pte->write = 1;
					pte->owner = 1;
					pte->no_execute = 0;
					__invlpg((PUCHAR)lineaddr);
				}
				

			}

			MmUnmapIoSpace(lineaddr, PAGE_SIZE);
		}
	}
}

ULONG64 NTAPI GetPtePhysicsAddress(IN ULONG64 address, IN PULONG64 pdeVirtualAddress)
{
	ULONG64 pdeIndex = (address >> 21) & 0x1ff;
	ULONG64 pteIndex = (address >> 12) & 0x1ff;

	ULONG64 ptePhyAddr = *(pdeVirtualAddress + pdeIndex);

	ptePhyAddr &= 0x000ffffffffff000;
	ptePhyAddr += pteIndex * 8;

	return ptePhyAddr;
}

ULONG64 NTAPI GetPteVirtualAddress(IN ULONG64 address, IN PULONG64 pdeVirtualAddress)
{
	ULONG64 ptePhyAddr = GetPtePhysicsAddress(address, pdeVirtualAddress);

	PHYSICAL_ADDRESS pa = { 0 };
	pa.LowPart = ptePhyAddr;

	ULONG64 pteVirtualAddress = MmMapIoSpace(pa, 8, MmNonCached);
	if (NULL == pteVirtualAddress)
	{
#ifdef _START_DEBUG
		DbgPrint("MmGetVirtualForPhysical 映射PTE线性地址失败 \n");
#endif

		return 0;
	}

	return pteVirtualAddress;
}

ULONG64 NTAPI GetDataPhysicsAddress(IN ULONG64 address, IN PULONG64 pteVirtualAddress)
{
	ULONG64 addrOffset = address & 0xfff;

	ULONG64 phyAddr = *pteVirtualAddress;

	// NOTICE：物理地址52位
	phyAddr &= 0x000ffffffffff000;
	phyAddr += addrOffset;

	return phyAddr;
}

ULONG64 NTAPI GetDateVirtualAddress(IN ULONG64 address, IN PULONG64 pteVirtualAddress)
{
	ULONG64 dataPhyAddr = GetDataPhysicsAddress(address, pteVirtualAddress);

	PHYSICAL_ADDRESS pa = { 0 };
	pa.LowPart = dataPhyAddr;

	ULONG64 virtualAddress = MmMapIoSpace(pa, 8, MmNonCached);
	if (NULL == virtualAddress)
	{
#ifdef _START_DEBUG
		DbgPrint("MmGetVirtualForPhysical 映射DATA线性地址失败 \n");
#endif

		return 0;
	}

	return virtualAddress;
}

NTSTATUS NTAPI GetPdptePhysicsAddressByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_phyAddr)
{
	NTSTATUS status = STATUS_SUCCESS;

	// 验证参数
	if (NULL == p_phyAddr)
	{
#ifdef _START_DEBUG
		DbgPrint("GetPdptePhysicsAddress:接收返回结果的参数为NULL \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

	// 映射CR3线性地址
	PHYSICAL_ADDRESS pa = { 0 };
	pa.LowPart = cr3Val;
	pa.HighPart = cr3Val >> 32;

	PULONG64 cr3VirAddr = MmMapIoSpace(pa, 8, MmNonCached);
	if (NULL == cr3VirAddr)
	{
#ifdef _START_DEBUG
		DbgPrint("MmMapIoSpace映射cr3线性地址失败 \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

	// 计算PML4、PDPTE index
	ULONG64 pxeIndex = (address >> 39) & 0x1ff;
	ULONG64 pdpteIndex = (address >> 30) & 0x1ff;

	ULONG64 pdptePhyAddr = *(cr3VirAddr + pxeIndex);

	pdptePhyAddr &= 0x000ffffffffff000;
	pdptePhyAddr += pdpteIndex * 8;

	// 返回结果
	*p_phyAddr = pdptePhyAddr;

	// 释放映射
	MmUnmapIoSpace(cr3VirAddr, 8);

	return status;
}

/*
	调用该函数映射的线性地址用完记得释放
*/
NTSTATUS NTAPI GetPdpteVirtualAddressByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_virAddr)
{
	NTSTATUS status = STATUS_SUCCESS;

	// 验证参数
	if (NULL == p_virAddr)
	{
#ifdef _START_DEBUG
		DbgPrint("GetPdpteVirtualAddress:接收返回结果的参数为NULL \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

	// 获取物理地址
	ULONG64 pdptePyhAddr = 0;
	if (!NT_SUCCESS(GetPdptePhysicsAddressByCr3(address, cr3Val, &pdptePyhAddr)))
	{
#ifdef _START_DEBUG
		DbgPrint("GetPdpteVirtualAddress:获取PDPTE物理地址失败 \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

#ifdef _START_DEBUG
	DbgPrint("pdptePyhAddr=%llx \n", pdptePyhAddr);
#endif

	// 映射线性地址
	PHYSICAL_ADDRESS pa = { 0 };
	pa.LowPart = pdptePyhAddr;
	pa.HighPart = pdptePyhAddr >> 32;

	PULONG64 virAddr = MmMapIoSpace(pa, 8, MmNonCached);
	if (NULL == virAddr)
	{
#ifdef _START_DEBUG
		DbgPrint("GetPdpteVirtualAddress:PDPTE映射线性地址失败 \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

#ifdef _START_DEBUG
	DbgPrint("virAddr=%llx \n", virAddr);
#endif

	// 返回结果
	*p_virAddr = virAddr;

	// 释放（不能释放，释放了）
	//MmUnmapIoSpace(virAddr, 8);

	return status;
}

NTSTATUS NTAPI GetPdePhysicsAddressByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_phyAddr)
{
	NTSTATUS status = STATUS_SUCCESS;

	// 验证参数
	if (NULL == p_phyAddr)
	{
#ifdef _START_DEBUG
		DbgPrint("GetPdePhysicsAddress:接收返回结果的参数为NULL \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

	// 获取PDPTE线性地址
	ULONG64 pdpteVirAddr = 0;

	if (!NT_SUCCESS(GetPdpteVirtualAddressByCr3(address, cr3Val, &pdpteVirAddr)))
	{
#ifdef _START_DEBUG
		DbgPrint("GetPdePhysicsAddress:获取PDPTE线性地址失败 \n");
#endif

		return STATUS_UNSUCCESSFUL;
	};

	// 计算PML4、PDPTE index
	ULONG64 pdeIndex = (address >> 21) & 0x1ff;

	ULONG64 pdePhyAddr = *(PULONG64)pdpteVirAddr;

	pdePhyAddr &= 0x000ffffffffff000;
	pdePhyAddr += pdeIndex * 8;

	// 返回结果
	*p_phyAddr = pdePhyAddr;

	// 释放映射
	MmUnmapIoSpace(pdpteVirAddr, 8);

	return status;
}

/*
	调用该函数映射的线性地址用完记得释放
*/
NTSTATUS NTAPI GetPdeVirtualAddressByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_virAddr)
{
	NTSTATUS status = STATUS_SUCCESS;

	// 验证参数
	if (NULL == p_virAddr)
	{
#ifdef _START_DEBUG
		DbgPrint("GetPdeVirtualAddress:接收返回结果的参数为NULL \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

	// 获取物理地址
	ULONG64 pdePyhAddr = 0;
	if (!NT_SUCCESS(GetPdePhysicsAddressByCr3(address, cr3Val, &pdePyhAddr)))
	{
#ifdef _START_DEBUG
		DbgPrint("GetPdeVirtualAddress:获取PDPTE物理地址失败 \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

#ifdef _START_DEBUG
	DbgPrint("pdePyhAddr=%llx \n", pdePyhAddr);
#endif

	// 映射线性地址
	PHYSICAL_ADDRESS pa = { 0 };

	pa.LowPart = pdePyhAddr;
	pa.HighPart = pdePyhAddr >> 32;

	PULONG64 virAddr = MmMapIoSpace(pa, 8, MmNonCached);
	if (NULL == virAddr)
	{
#ifdef _START_DEBUG
		DbgPrint("GetPdpteVirtualAddress:PDPTE映射线性地址失败 \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

#ifdef _START_DEBUG
	DbgPrint("virAddr=%llx \n", virAddr);
#endif

	// 返回结果
	*p_virAddr = virAddr;

	// 释放（不能释放，释放了）
	//MmUnmapIoSpace(virAddr, 8);

	return status;
}

NTSTATUS NTAPI GetPtePhysicsAddressByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_phyAddr)
{
	NTSTATUS status = STATUS_SUCCESS;

	// 验证参数
	if (NULL == p_phyAddr)
	{
#ifdef _START_DEBUG
		DbgPrint("GetPtePhysicsAddressByCr3:接收返回结果的参数为NULL \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

	// 获取PDE线性地址
	ULONG64 pdeVirAddr = 0;

	if (!NT_SUCCESS(GetPdeVirtualAddressByCr3(address, cr3Val, &pdeVirAddr)))
	{
#ifdef _START_DEBUG
		DbgPrint("GetPtePhysicsAddressByCr3:获取PDPTE线性地址失败 \n");
#endif

		return STATUS_UNSUCCESSFUL;
	};

	ULONG64 pteIndex = (address >> 12) & 0x1ff;

	ULONG64 ptePhyAddr = *(PULONG64)pdeVirAddr;

	ptePhyAddr &= 0x000ffffffffff000;
	ptePhyAddr += pteIndex * 8;

	// 返回结果
	*p_phyAddr = ptePhyAddr;

	// 释放映射
	MmUnmapIoSpace(pdeVirAddr, 8);

	return status;
}

/*
	调用该函数映射的线性地址用完记得释放
*/
NTSTATUS NTAPI GetPteVirtualAddressByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_virAddr)
{
	NTSTATUS status = STATUS_SUCCESS;

	// 验证参数
	if (NULL == p_virAddr)
	{
#ifdef _START_DEBUG
		DbgPrint("GetPteVirtualAddressByCr3:接收返回结果的参数为NULL \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

	// 获取物理地址
	ULONG64 ptePyhAddr = 0;
	if (!NT_SUCCESS(GetPtePhysicsAddressByCr3(address, cr3Val, &ptePyhAddr)))
	{
#ifdef _START_DEBUG
		DbgPrint("GetPteVirtualAddressByCr3:获取PTE物理地址失败 \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

#ifdef _START_DEBUG
	DbgPrint("ptePyhAddr=%llx \n", ptePyhAddr);
#endif

	// 映射线性地址
	PHYSICAL_ADDRESS pa = { 0 };

	pa.LowPart = ptePyhAddr;
	pa.HighPart = ptePyhAddr >> 32;

	PULONG64 virAddr = MmMapIoSpace(pa, 8, MmNonCached);
	if (NULL == virAddr)
	{
#ifdef _START_DEBUG
		DbgPrint("GetPteVirtualAddressByCr3:PTE映射线性地址失败 \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

#ifdef _START_DEBUG
	DbgPrint("virAddr=%llx \n", virAddr);
#endif

	// 返回结果
	*p_virAddr = virAddr;

	return status;
}

NTSTATUS NTAPI GetDataPhysicsAddressByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_phyAddr)
{
	NTSTATUS status = STATUS_SUCCESS;

	// 验证参数
	if (NULL == p_phyAddr)
	{
#ifdef _START_DEBUG
		DbgPrint("GetDataPhysicsAddressByCr3:接收返回结果的参数为NULL \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

	// 获取PDE线性地址
	ULONG64 pteVirAddr = 0;

	if (!NT_SUCCESS(GetPteVirtualAddressByCr3(address, cr3Val, &pteVirAddr)))
	{
#ifdef _START_DEBUG
		DbgPrint("GetDataPhysicsAddressByCr3:获取PDPTE线性地址失败 \n");
#endif

		return STATUS_UNSUCCESSFUL;
	};

	ULONG64 addrOffset = address & 0xfff;

	ULONG64 phyAddr = *(PULONG64)pteVirAddr;

	// NOTICE：物理地址52位
	phyAddr &= 0x000ffffffffff000;
	phyAddr += addrOffset;

	// 返回结果
	*p_phyAddr = phyAddr;

	// 释放映射
	MmUnmapIoSpace(pteVirAddr, 8);

	return status;
}

/*
	调用该函数映射的线性地址用完记得释放
*/
NTSTATUS NTAPI GetDataVirtualAddressByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_virAddr)
{
	NTSTATUS status = STATUS_SUCCESS;

	// 验证参数
	if (NULL == p_virAddr)
	{
#ifdef _START_DEBUG
		DbgPrint("GetDataVirtualAddressByCr3:接收返回结果的参数为NULL \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

	// 获取物理地址
	ULONG64 pyhAddr = 0;
	if (!NT_SUCCESS(GetDataPhysicsAddressByCr3(address, cr3Val, &pyhAddr)))
	{
#ifdef _START_DEBUG
		DbgPrint("GetDataVirtualAddressByCr3:获取PTE物理地址失败 \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

#ifdef _START_DEBUG
	DbgPrint("pyhAddr=%llx \n", pyhAddr);
#endif

	// 映射线性地址
	PHYSICAL_ADDRESS pa = { 0 };

	pa.LowPart = pyhAddr;
	pa.HighPart = pyhAddr >> 32;

	PULONG64 virAddr = MmMapIoSpace(pa, 8, MmNonCached);
	if (NULL == virAddr)
	{
#ifdef _START_DEBUG
		DbgPrint("GetDataVirtualAddressByCr3:PTE映射线性地址失败 \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

#ifdef _START_DEBUG
	DbgPrint("virAddr=%llx \n", virAddr);
#endif

	// 返回结果
	*p_virAddr = virAddr;

	return status;
}

// 未测试
NTSTATUS NTAPI MySteryReadMemoryByCr3(
	IN ULONG64 cr3Val, 
	IN PVOID64 readAddr,
	IN ULONG64 readSize, 
	OUT PVOID64 p_buff)
{
	NTSTATUS status = STATUS_SUCCESS;
	PVOID64 buff = NULL;

	// 验证参数
	if (NULL == p_buff)
	{
#ifdef _START_DEBUG
		DbgPrint("MySteryReadMemoryByCr3:接收返回结果的参数为NULL \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

	// 申请内存并初始化
	buff = ExAllocatePool(NonPagedPool, readSize);
	if (NULL == buff)
	{
		DbgPrint("ExAllocatePool failed \n");

		return STATUS_UNSUCCESSFUL;
	}

	RtlZeroMemory(buff, readSize);

	// 获取新的线性地址
	ULONG64 dataAddr = 0;
	if (!NT_SUCCESS(GetDataVirtualAddressByCr3((ULONG64)readAddr, cr3Val, &dataAddr)))
	{
#ifdef _START_DEBUG
		DbgPrint("MySteryReadMemoryByCr3:获取PTE物理地址失败 \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

	//
	RtlCopyMemory(buff, readAddr, readSize);

	RtlCopyMemory(p_buff, buff, readSize);

	// 释放资源
	MmUnmapIoSpace((PVOID)dataAddr, readSize);
	ExFreePool(buff);

	return status;
}

NTSTATUS NTAPI GetPdpteByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_pdpte)
{
	return 0;
}

NTSTATUS NTAPI GetPdeByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_pde)
{
	NTSTATUS status = STATUS_SUCCESS;

	// 验证参数
	if (NULL == p_pde)
	{
#ifdef _START_DEBUG
		DbgPrint("GetPdeByCr3:接收返回结果的参数为NULL \n");
#endif

		return STATUS_UNSUCCESSFUL;
	}

	// 获取PDPTE线性地址
	ULONG64 pdpteVirAddr = 0;

	if (!NT_SUCCESS(GetPdpteVirtualAddressByCr3(address, cr3Val, &pdpteVirAddr)))
	{
#ifdef _START_DEBUG
		DbgPrint("GetPdeByCr3:获取PDPTE线性地址失败 \n");
#endif

		return STATUS_UNSUCCESSFUL;
	};

	// 计算PML4、PDPTE index
	ULONG64 pdeIndex = (address >> 21) & 0x1ff;

	ULONG64 pdePhyAddr = *(PULONG64)pdpteVirAddr;

	// 返回结果
	*p_pde = pdePhyAddr;

	// 释放映射
	MmUnmapIoSpace(pdpteVirAddr, 8);

	return status;
}

NTSTATUS NTAPI GetPteByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_pte);