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