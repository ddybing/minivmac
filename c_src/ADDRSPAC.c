/*
	ADDRSPAC.c

	Copyright (C) 2002 Bernd Schmidt, Philip Cummins, Paul Pratt

	You can redistribute this file and/or modify it under the terms
	of version 2 of the GNU General Public License as published by
	the Free Software Foundation.  You should have received a copy
	of the license along with this file; see the file COPYING.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	license for more details.
*/

/*
	ADDRess SPACe

	Implements the address space of the Mac Plus.

	This code is descended from code in vMac by Philip Cummins, in
	turn descended from code in the Un*x Amiga Emulator by
	Bernd Schmidt.
*/

#ifndef AllFiles
#include "SYSDEPNS.h"
#include "MYOSGLUE.h"
#include "GLOBGLUE.h"
#endif
#include "ENDIANAC.h"

#include "ADDRSPAC.h"

IMPORTPROC SCSI_Access(CPTR addr);
IMPORTPROC SCC_Access(CPTR addr);
IMPORTPROC IWM_Access(CPTR addr);
IMPORTPROC VIA_Access(CPTR addr);
IMPORTPROC Sony_Access(CPTR addr);
IMPORTPROC SetAutoVector(void);

/* top 8 bits out of 32 are ignored, so total size of address space is 2 ** 24 bytes */

#define ln2TotAddrBytes 24
#define TotAddrBytes (1 << ln2TotAddrBytes)
#define kAddrMask (TotAddrBytes - 1)

/* map of address space */

LOCALVAR ui3b vOverlay;

#define kROM_Overlay_Base 0x00000000 /* when overlay on */
#define kROM_Overlay_Top  0x00100000

#define kRAM_Base 0x00000000 /* when overlay off */
#define kRAM_Top  0x00400000

#define kROM_Base 0x00400000
#define kROM_Top  0x00500000

#define kSCSI_Block_Base 0x00580000
#define kSCSI_Block_Top  0x00600000

#define kRAM_Overlay_Base 0x00600000 /* when overlay on */
#define kRAM_Overlay_Top  0x00700000

#define kSCCRd_Block_Base 0x00800000
#define kSCCRd_Block_Top  0x00A00000

#define kSCCWr_Block_Base 0x00A00000
#define kSCCWr_Block_Top  0x00C00000

#define kIWM_Block_Base 0x00C00000
#define kIWM_Block_Top  0x00E00000

#define kVIA_Block_Base 0x00E80000
#define kVIA_Block_Top  0x00F00000

#define kDSK_Block_Base 0x00F40000
#define kDSK_Block_Top  0x00F40020

#define kAutoVector_Base 0x00FFFFF0
#define kAutoVector_Top  0x01000000

/* implementation of read/write for everything but RAM and ROM */

#define kSCCRdBase 0x9FFFF8
#define kSCCWrBase 0xBFFFF9

#define kSCC_Mask 0x07

#define kVIA_Size 0x2000
#define kVIA_Base 0xEFE1FE

#define kIWM_Mask 0x001FFF // Allocated Memory Bandwidth for IWM
#define kIWM_Base 0xDFE1FF // IWM Memory Base

GLOBALVAR CPTR AddressBus;
GLOBALVAR ui5b DataBus;
GLOBALVAR blnr ByteSizeAccess;
GLOBALVAR blnr WriteMemAccess;

LOCALPROC MM_Access(void)
{
	AddressBus &= kAddrMask;

	if (AddressBus < kIWM_Block_Base) {
		if (AddressBus < kSCCRd_Block_Base) {
			if ((AddressBus >= kSCSI_Block_Base) && (AddressBus < kSCSI_Block_Top)) {
				SCSI_Access(AddressBus - kSCSI_Block_Base);
			}
		} else {
			if (AddressBus >= kSCCWr_Block_Base) {
				if (WriteMemAccess) {
					/* if ((AddressBus >= kSCCWr_Block_Base) && (AddressBus < kSCCWr_Block_Top)) */  // Write Only Address
					{
						SCC_Access((AddressBus - kSCCWrBase) & kSCC_Mask);
					}
				}
			} else {
				if (! WriteMemAccess) {
					/* if ((AddressBus >= kSCCRd_Block_Base) && (AddressBus < kSCCRd_Block_Top)) */ // Read Only Address
					{
						SCC_Access((AddressBus - kSCCRdBase) & kSCC_Mask);
					}
				}
			}
		}
	} else {
		if (AddressBus < kVIA_Block_Base) {
			if (/* (AddressBus >= kIWM_Block_Base) && */(AddressBus < kIWM_Block_Top)) {
				IWM_Access((AddressBus - kIWM_Base) & kIWM_Mask);
			}
		} else {
			if (/* (AddressBus >= kVIA_Block_Base) && */(AddressBus < kVIA_Block_Top)) {
				VIA_Access((AddressBus - kVIA_Base) % kVIA_Size);
			} else {
				if ((AddressBus >= kDSK_Block_Base) && (AddressBus < kDSK_Block_Top)) {
					Sony_Access(AddressBus - kDSK_Block_Base);
				} else
				if ((AddressBus >= kAutoVector_Base) && (AddressBus < kAutoVector_Top)) {
					SetAutoVector();
					/* Exception(regs.intmask+24, 0); */
				}
			}
		}
	}
}

/* devide address space into banks, some of which are mapped to real memory */

#define ln2BytesPerMemBank 17
#define ln2NumMemBanks (ln2TotAddrBytes - ln2BytesPerMemBank)

#define NumMemBanks (1 << ln2NumMemBanks)
#define BytesPerMemBank  (1 << ln2BytesPerMemBank)
#define MemBanksMask (NumMemBanks - 1)
#define MemBankAddrMask (BytesPerMemBank - 1)

LOCALVAR ui3b *BankReadAddr[NumMemBanks];
LOCALVAR ui3b *BankWritAddr[NumMemBanks]; /* if BankWritAddr[i] != NULL then BankWritAddr[i] == BankReadAddr[i] */

#define bankindex(addr) ((((CPTR)(addr)) >> ln2BytesPerMemBank) & MemBanksMask)

#define kROM_BaseBank (kROM_Base >> ln2BytesPerMemBank)
#define kROM_TopBank (kROM_Top >> ln2BytesPerMemBank)

#define kROM_Overlay_BaseBank (kROM_Overlay_Base >> ln2BytesPerMemBank)
#define kROM_Overlay_TopBank (kROM_Overlay_Top >> ln2BytesPerMemBank)

#define kRAM_Overlay_BaseBank (kRAM_Overlay_Base >> ln2BytesPerMemBank)
#define kRAM_Overlay_TopBank (kRAM_Overlay_Top >> ln2BytesPerMemBank)

#define kRAM_BaseBank (kRAM_Base >> ln2BytesPerMemBank)
#define kRAM_TopBank (kRAM_Top >> ln2BytesPerMemBank)

#define Overlay_RAMmem_mask (0x00010000 - 1)
#define Overlay_ROMmem_mask ROMmem_mask
#define ROMmem_mask (kROM_Size - 1)

LOCALPROC SetUpBankRange(ui5b StartBank, ui5b StopBank, ui3b * RealStart, CPTR VirtualStart, ui5b vMask, blnr Writeable)
{
	int i;

	for (i = StartBank; i < StopBank; i++) {
		BankReadAddr[i] = RealStart + (((i << ln2BytesPerMemBank) - VirtualStart) & vMask);
		if (Writeable) {
			BankWritAddr[i] = BankReadAddr[i];
		}
	}
}

LOCALPROC SetPtrVecToNULL(ui3b **x, ui5b n)
{
	int i;

	for (i = 0; i < n; i++) {
		*x++ = nullpr;
	}
}

LOCALPROC SetUpMemBanks(void)
{
	ui5b RAMmem_mask = kRAM_Size - 1;

	SetPtrVecToNULL(BankReadAddr, NumMemBanks);
	SetPtrVecToNULL(BankWritAddr, NumMemBanks);

	SetUpBankRange(kROM_BaseBank, kROM_TopBank, (ui3b *) ROM, kROM_Base, ROMmem_mask, falseblnr);

	if (vOverlay) {
		SetUpBankRange(kROM_Overlay_BaseBank, kROM_Overlay_TopBank, (ui3b *)ROM, kROM_Overlay_Base, Overlay_ROMmem_mask, falseblnr);
		SetUpBankRange(kRAM_Overlay_BaseBank, kRAM_Overlay_TopBank, (ui3b *)RAM, kRAM_Overlay_Base, Overlay_RAMmem_mask, trueblnr);
	} else {
		SetUpBankRange(kRAM_BaseBank, kRAM_TopBank, (ui3b *)RAM, kRAM_Base, RAMmem_mask, trueblnr);
	}
}

GLOBALPROC ZapNMemoryVars(void)
{
	vOverlay = 2; /* MemBanks uninitialized */
}

GLOBALPROC VIA_PORA4(ui3b Data)
{
	if (vOverlay != Data) {
		vOverlay = Data;
		SetUpMemBanks();
	}
}

GLOBALFUNC ui3b VIA_GORA4(void) // Overlay/Normal Memory Mapping
{
#ifdef _VIA_Interface_Debug
	printf("VIA ORA4 attempts to be an input\n");
#endif
	return 0;
}

GLOBALPROC Memory_Reset(void)
{
	VIA_PORA4(1);
}


/*
	unlike in the real Mac Plus, Mini vMac
	will allow misaligned memory access,
	since it is easier to allow it than
	it is to correctly simulate a bus error
	and back out of the current instruction.
*/

GLOBALFUNC ui5b get_long(CPTR addr)
{
	ui3b *ba = BankReadAddr[bankindex(addr)];

	if (ba != nullpr) {
		ui5b *m = (ui5b *)((addr & MemBankAddrMask) + ba);
		return do_get_mem_long(m);
	} else {
		if (addr != -1) {
#ifdef MyCompilerMrC
#pragma noinline_site get_word
#endif
			ui4b hi = get_word(addr);
			ui4b lo = get_word(addr+2);
			return (ui5b) (((ui5b)hi) << 16) | ((ui5b)lo);
		} else {
			/* A work around for someone's bug. When booting
			from system 6.0.8, address -1 is referenced once.
			if don't check and use above code, then everything
			seems to work fine, except have found one reproducible
			bug: lode runner crashes when saving a new high
			score */

			/* Debugger(); */
			return 0;
		}
	}
}

GLOBALFUNC ui5b get_word(CPTR addr)
{
	ui3b *ba = BankReadAddr[bankindex(addr)];

	if (ba != nullpr) {
		ui4b *m = (ui4b *)((addr & MemBankAddrMask) + ba);
		return do_get_mem_word(m);
	} else {
		AddressBus = addr;
		DataBus = 0;
		ByteSizeAccess = falseblnr;
		WriteMemAccess = falseblnr;
		MM_Access();
		return (ui4b) DataBus;
	}
}

GLOBALFUNC ui5b get_byte(CPTR addr)
{
	ui3b *ba = BankReadAddr[bankindex(addr)];

	if (ba != nullpr) {
		ui3b *m = (ui3b *)((addr & MemBankAddrMask) + ba);
		return *m;
	} else {
		AddressBus = addr;
		DataBus = 0;
		ByteSizeAccess = trueblnr;
		WriteMemAccess = falseblnr;
		MM_Access();
		return (ui3b) DataBus;
	}
}

GLOBALPROC put_long(CPTR addr, ui5b l)
{
	ui3b *ba = BankWritAddr[bankindex(addr)];

	if (ba != nullpr) {
		ui5b *m = (ui5b *)((addr & MemBankAddrMask) + ba);
		do_put_mem_long(m, l);
	} else {
#ifdef MyCompilerMrC
#pragma noinline_site put_word
#endif
		put_word(addr, (l >> 16) & (0x0000FFFF));
		put_word(addr+2, l & (0x0000FFFF));
	}
}

GLOBALPROC put_word(CPTR addr, ui5b w)
{
	ui3b *ba = BankWritAddr[bankindex(addr)];

	if (ba != nullpr) {
		ui4b *m = (ui4b *)((addr & MemBankAddrMask) + ba);
		do_put_mem_word(m, w);
	} else {
		AddressBus = addr;
		DataBus = w;
		ByteSizeAccess = falseblnr;
		WriteMemAccess = trueblnr;
		MM_Access();
	}
}

GLOBALPROC put_byte(CPTR addr, ui5b b)
{
	ui3b *ba = BankWritAddr[bankindex(addr)];

	if (ba != nullpr) {
		ui3b *m = (ui3b *)((addr & MemBankAddrMask) + ba);
		*m = b;
	} else {
		AddressBus = addr;
		DataBus = b;
		ByteSizeAccess = trueblnr;
		WriteMemAccess = trueblnr;
		MM_Access();
	}
}

LOCALFUNC ui3b *default_xlate(CPTR a)
{
	UnusedParam(a);
	return (ui3b *) RAM; /* So we don't crash. */
}

GLOBALFUNC ui3b *get_real_address(CPTR addr)
{
	ui3b *ba = BankReadAddr[bankindex(addr)];

	if (ba != nullpr) {
		return (ui3b *)((addr & MemBankAddrMask) + ba);
	} else {
		return default_xlate(addr);
	}
}
