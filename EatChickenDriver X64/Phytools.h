#ifndef __PHY_TOOLS_H__
#define __PHY_TOOLS_H__
#include <ntifs.h>

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef unsigned long long uint64_t;

typedef union VIRT_ADDR_
{
	ULONG_PTR value;
	void *pointer;
	struct
	{
		ULONG_PTR offset : 12;
		ULONG_PTR pt_index : 9;
		ULONG_PTR pd_index : 9;
		ULONG_PTR pdpt_index : 9;
		ULONG_PTR pml4_index : 9;
		ULONG_PTR reserved : 16;
	};
} VIRT_ADDR;

typedef struct HardwarePteX64 {
	ULONG64 valid : 1;               //!< [0]
	ULONG64 write : 1;               //!< [1]
	ULONG64 owner : 1;               //!< [2]
	ULONG64 write_through : 1;       //!< [3]
	ULONG64 cache_disable : 1;       //!< [4]
	ULONG64 accessed : 1;            //!< [5]
	ULONG64 dirty : 1;               //!< [6]
	ULONG64 large_page : 1;          //!< [7]
	ULONG64 global : 1;              //!< [8]
	ULONG64 copy_on_write : 1;       //!< [9]
	ULONG64 prototype : 1;           //!< [10]
	ULONG64 reserved0 : 1;           //!< [11]
	ULONG64 page_frame_number : 36;  //!< [12:47]
	ULONG64 reserved1 : 4;           //!< [48:51]
	ULONG64 software_ws_index : 11;  //!< [52:62]
	ULONG64 no_execute : 1;          //!< [63]
}HardwarePte;

typedef union _MMPTE {
	ULONG_PTR value;
	VIRT_ADDR vaddr;
	struct
	{
		ULONG_PTR present : 1;
		ULONG_PTR rw : 1;
		ULONG_PTR user : 1;
		ULONG_PTR write_through : 1;
		ULONG_PTR cache_disable : 1;
		ULONG_PTR accessed : 1;
		ULONG_PTR dirty : 1;
		ULONG_PTR pat : 1;
		ULONG_PTR global : 1;
		ULONG_PTR ignored_1 : 3;
		ULONG_PTR page_frame : 40;
		ULONG_PTR ignored_3 : 11;
		ULONG_PTR xd : 1;
	};
} MMPTE,*PMMPTE;

#define MiGetPxeOffset(va) ((ULONG)(((ULONG_PTR)(va) >> PXI_SHIFT) & PXI_MASK))

HardwarePte * NTAPI GetPxeAddress(__in PVOID VirtualAddress);
HardwarePte * NTAPI GetPpeAddress(__in PVOID VirtualAddress);
HardwarePte * NTAPI GetPdeAddress(__in PVOID VirtualAddress);
HardwarePte * NTAPI GetPteAddress(__in PVOID VirtualAddress);

NTSTATUS NTAPI GetPdptePhysicsAddressByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_phyAddr);
NTSTATUS NTAPI GetPdpteVirtualAddressByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_virAddr);

NTSTATUS NTAPI GetPdePhysicsAddressByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_phyAddr);
NTSTATUS NTAPI GetPdeVirtualAddressByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_virAddr);

NTSTATUS NTAPI GetPtePhysicsAddressByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_phyAddr);
NTSTATUS NTAPI GetPteVirtualAddressByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_virAddr);

NTSTATUS NTAPI GetDataPhysicsAddressByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_phyAddr);
NTSTATUS NTAPI GetDataVirtualAddressByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_virAddr);

NTSTATUS NTAPI GetPdpteByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_pdpte);
NTSTATUS NTAPI GetPdeByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_pde);
NTSTATUS NTAPI GetPteByCr3(IN ULONG64 address, IN ULONG64 cr3Val, OUT PULONG64 p_pte);

ULONG64 NTAPI GetPtePhysicsAddress(IN ULONG64 address, IN PULONG64 pdeVirtualAddress);
ULONG64 NTAPI GetPteVirtualAddress(IN ULONG64 address, IN PULONG64 pdeVirtualAddress);

ULONG64 NTAPI GetDataPhysicsAddress(IN ULONG64 address, IN PULONG64 pdeVirtualAddress);
ULONG64 NTAPI GetDateVirtualAddress(IN ULONG64 address, IN PULONG64 pdeVirtualAddress);

NTSTATUS NTAPI SetPxePhyAddrValid(IN ULONG64 address, IN PULONG64 cr3Val, OUT ULONG64 pxePhyAddr);

void SetAddrToLineAddr(PVOID addr, ULONG64 lineAddr, ULONG sizeOfImage);

#endif