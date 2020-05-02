/*
 *  Copyright (C) 2002-2019  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
 
#include <math.h>
#include <assert.h>
#include "dosbox.h"
#include "inout.h"
#include "pic.h"
#include "mem.h"
#include "cpu.h"
#include "ide.h"
#include "isa.h"
#include "mixer.h"
#include "timer.h"
#include "setup.h"
#include "control.h"
#include "callback.h"
#include "bios_disk.h"
#include "../src/dos/cdrom.h"

/* if mem_systems 0 then size_extended is reported as the real size else 
 * zero is reported. ems and xms can increase or decrease the other_memsystems
 * counter using the BIOS_ZeroExtendedSize call */
static Bit16u size_extended;
static unsigned int ISA_PNP_WPORT = 0x20B;
static unsigned int ISA_PNP_WPORT_BIOS = 0x20B;
static IO_ReadHandleObject *ISAPNP_PNP_READ_PORT = NULL;		/* 0x200-0x3FF range */
static IO_WriteHandleObject *ISAPNP_PNP_ADDRESS_PORT = NULL;		/* 0x279 */
static IO_WriteHandleObject *ISAPNP_PNP_DATA_PORT = NULL;		/* 0xA79 */
static unsigned char ISA_PNP_CUR_CSN = 0;
static unsigned char ISA_PNP_CUR_ADDR = 0;
static unsigned char ISA_PNP_CUR_STATE = 0;
enum {
	ISA_PNP_WAIT_FOR_KEY=0,
	ISA_PNP_SLEEP,
	ISA_PNP_ISOLATE,
	ISA_PNP_CONFIG
};

const unsigned char isa_pnp_init_keystring[32] = {
	0x6A,0xB5,0xDA,0xED,0xF6,0xFB,0x7D,0xBE,
	0xDF,0x6F,0x37,0x1B,0x0D,0x86,0xC3,0x61,
	0xB0,0x58,0x2C,0x16,0x8B,0x45,0xA2,0xD1,
	0xE8,0x74,0x3A,0x9D,0xCE,0xE7,0x73,0x39
};

#ifdef C_DEBUG
#define fprintf silent_fprintf
static size_t silent_fprintf(FILE *f,const char *fmt,...) {
}
#endif

static unsigned char ISA_PNP_KEYMATCH=0;
bool ISAPNPBIOS=false;

ISAPnPDevice::ISAPnPDevice() {
	CSN = 0;
	logical_device = 0;
	memset(ident,0,sizeof(ident));
	ident_bp = 0;
	ident_2nd = 0;
	resource_data_len = 0;	
	resource_data_pos = 0;
	alloc_res = NULL;
	alloc_write = 0;
	alloc_sz = 0;	
}

bool ISAPnPDevice::alloc(size_t sz) {
	if (sz == alloc_sz)
		return true;

	if (alloc_res == resource_data) {
		resource_data_len = 0;
		resource_data_pos = 0;
		resource_data = NULL;
	}
	if (alloc_res != NULL)
		delete[] alloc_res;

	alloc_res = NULL;
	alloc_write = 0;
	alloc_sz = 0;

	if (sz == 0)
		return true;
	if (sz > 65536)
		return false;

	alloc_res = new unsigned char[sz];
	if (alloc_res == NULL) return false;
	memset(alloc_res,0xFF,sz);
	alloc_sz = sz;
	return true;
}

ISAPnPDevice::~ISAPnPDevice() {
	alloc(0);
}

void ISAPnPDevice::begin_write_res() {
	if (alloc_res == NULL) return;

	resource_data_pos = 0;
	resource_data_len = 0;
	resource_data = NULL;
	alloc_write = 0;
}

void ISAPnPDevice::write_byte(const unsigned char c) {
	if (alloc_res == NULL || alloc_write >= alloc_sz) return;
	alloc_res[alloc_write++] = c;
}

void ISAPnPDevice::write_begin_SMALLTAG(const ISAPnPDevice::SmallTags stag,unsigned char len) {
	if (len >= 8 || (unsigned int)stag >= 0x10) return;
	write_byte(((unsigned char)stag << 3) + len);
}

void ISAPnPDevice::write_begin_LARGETAG(const ISAPnPDevice::LargeTags stag,unsigned int len) {
	if (len >= 4096) return;
	write_byte(0x80 + ((unsigned char)stag));
	write_byte(len & 0xFF);
	write_byte(len >> 8);
}

void ISAPnPDevice::write_Device_ID(const char c1,const char c2,const char c3,const char c4,const char c5,const char c6,const char c7) {
	write_byte((((unsigned char)c1 & 0x1FU) << 2) + (((unsigned char)c2 & 0x18U) >> 3));
	write_byte((((unsigned char)c2 & 0x07U) << 5) + (((unsigned char)c3 & 0x1FU)     ));
	write_byte((((unsigned char)c4 & 0x0FU) << 4) + (((unsigned char)c5 & 0x0FU)     ));
	write_byte((((unsigned char)c6 & 0x0FU) << 4) + (((unsigned char)c7 & 0x0FU)     ));
}

void ISAPnPDevice::write_Logical_Device_ID(const char c1,const char c2,const char c3,const char c4,const char c5,const char c6,const char c7) {
	write_begin_SMALLTAG(SmallTags::LogicalDeviceID,5);
	write_Device_ID(c1,c2,c3,c4,c5,c6,c7);
	write_byte(0x00);
}

void ISAPnPDevice::write_Compatible_Device_ID(const char c1,const char c2,const char c3,const char c4,const char c5,const char c6,const char c7) {
	write_begin_SMALLTAG(SmallTags::CompatibleDeviceID,4);
	write_Device_ID(c1,c2,c3,c4,c5,c6,c7);
}

void ISAPnPDevice::write_IRQ_Format(const uint16_t IRQ_mask,const unsigned char IRQ_signal_type) {
	bool write_irq_info = (IRQ_signal_type != 0);

	write_begin_SMALLTAG(SmallTags::IRQFormat,write_irq_info?3:2);
	write_byte(IRQ_mask & 0xFF);
	write_byte(IRQ_mask >> 8);
	if (write_irq_info) write_byte(((unsigned char)IRQ_signal_type & 0x0F));
}

void ISAPnPDevice::write_DMA_Format(const uint8_t DMA_mask,const unsigned char transfer_type_preference,const bool is_bus_master,const bool byte_mode,const bool word_mode,const unsigned char speed_supported) {
	write_begin_SMALLTAG(SmallTags::DMAFormat,2);
	write_byte(DMA_mask);
	write_byte(
		(transfer_type_preference & 0x03) |
		(is_bus_master ? 0x04 : 0x00) |
		(byte_mode ? 0x08 : 0x00) |
		(word_mode ? 0x10 : 0x00) |
		((speed_supported & 3) << 5));
}

void ISAPnPDevice::write_IO_Port(const uint16_t min_port,const uint16_t max_port,const uint8_t count,const uint8_t alignment,const bool full16bitdecode) {
	write_begin_SMALLTAG(SmallTags::IOPortDescriptor,7);
	write_byte((full16bitdecode ? 0x01 : 0x00));
	write_byte(min_port & 0xFF);
	write_byte(min_port >> 8);
	write_byte(max_port & 0xFF);
	write_byte(max_port >> 8);
	write_byte(alignment);
	write_byte(count);
}

void ISAPnPDevice::write_Dependent_Function_Start(const ISAPnPDevice::DependentFunctionConfig cfg,const bool force) {
	bool write_cfg_byte = force || (cfg != ISAPnPDevice::DependentFunctionConfig::AcceptableDependentConfiguration);

	write_begin_SMALLTAG(SmallTags::StartDependentFunctions,write_cfg_byte ? 1 : 0);
	if (write_cfg_byte) write_byte((unsigned char)cfg);
}

void ISAPnPDevice::write_End_Dependent_Functions() {
	write_begin_SMALLTAG(SmallTags::EndDependentFunctions,0);
}

void ISAPnPDevice::write_nstring(const char *str,const size_t l) {
	if (alloc_res == NULL || alloc_write >= alloc_sz) return;

	while (*str != 0 && alloc_write < alloc_sz)
		alloc_res[alloc_write++] = *str++;
}

void ISAPnPDevice::write_Identifier_String(const char *str) {
	const size_t l = strlen(str);
	if (l > 4096) return;

	write_begin_LARGETAG(LargeTags::IdentifierStringANSI,l);
	if (l != 0) write_nstring(str,l);
}

void ISAPnPDevice::write_ISAPnP_version(unsigned char major,unsigned char minor,unsigned char vendor) {
	write_begin_SMALLTAG(SmallTags::PlugAndPlayVersionNumber,2);
	write_byte((major << 4) + minor);
	write_byte(vendor);
}

void ISAPnPDevice::write_END() {
	unsigned char sum = 0;
	size_t i;

	write_begin_SMALLTAG(SmallTags::EndTag,/*length*/1);

	for (i=0;i < alloc_write;i++) sum += alloc_res[i];
	write_byte((0x100 - sum) & 0xFF);
}

void ISAPnPDevice::end_write_res() {
	if (alloc_res == NULL) return;

	write_END();

	if (alloc_write >= alloc_sz) LOG(LOG_MISC,LOG_WARN)("ISA PNP generation overflow");

	resource_data_pos = 0;
	resource_data_len = alloc_sz; // the device usually has a reason for allocating the fixed size it does
	resource_data = alloc_res;
	alloc_write = 0;
}

void ISAPnPDevice::config(Bitu val) {
}

void ISAPnPDevice::wakecsn(Bitu val) {
	ident_bp = 0;
	ident_2nd = 0;
	resource_data_pos = 0;
	resource_ident = 0;
}

void ISAPnPDevice::select_logical_device(Bitu val) {
}
	
void ISAPnPDevice::checksum_ident() {
	unsigned char checksum = 0x6a,bit;
	int i,j;

	for (i=0;i < 8;i++) {
		for (j=0;j < 8;j++) {
			bit = (ident[i] >> j) & 1;
			checksum = ((((checksum ^ (checksum >> 1)) & 1) ^ bit) << 7) | (checksum >> 1);
		}
	}

	ident[8] = checksum;
}

void ISAPnPDevice::on_pnp_key() {
	resource_ident = 0;
}

uint8_t ISAPnPDevice::read(Bitu addr) {
	return 0x00;
}

void ISAPnPDevice::write(Bitu addr,Bitu val) {
}

#define MAX_ISA_PNP_DEVICES	64

static ISAPnPDevice *ISA_PNP_selected = NULL;
static ISAPnPDevice *ISA_PNP_devs[MAX_ISA_PNP_DEVICES] = {NULL};
static Bitu ISA_PNP_devnext = 0;

static const unsigned char ISAPnPTestDevice_sysdev[] = {
	ISAPNP_IO_RANGE(
			0x01,					/* decodes 16-bit ISA addr */
			0x28,0x28,				/* min-max range I/O port */
			0x04,0x04),				/* align=4 length=4 */
	ISAPNP_IRQ_SINGLE(
			8,					/* IRQ 8 */
			0x09),					/* HTE=1 LTL=1 */
	ISAPNP_END
};

class ISAPnPTestDevice : public ISAPnPDevice {
	public:
		ISAPnPTestDevice() : ISAPnPDevice() {
			resource_ident = 0;
			resource_data = (unsigned char*)ISAPnPTestDevice_sysdev;
			resource_data_len = sizeof(ISAPnPTestDevice_sysdev);
			*((uint32_t*)(ident+0)) = ISAPNP_ID('D','O','S',0x1,0x2,0x3,0x4);
			*((uint32_t*)(ident+4)) = 0xFFFFFFFFUL;
			checksum_ident();
		}
};

static ISAPnPTestDevice *isapnp_test = NULL;

void ISA_PNP_devreg(ISAPnPDevice *x) {
	if (ISA_PNP_devnext < MAX_ISA_PNP_DEVICES) {
		ISA_PNP_devs[ISA_PNP_devnext++] = x;
		x->CSN = ISA_PNP_devnext;
	}
	//else
	//	fprintf(stderr,"WARNING: ignored PnP device registration\n");
}

static Bitu isapnp_read_port(Bitu port,Bitu /*iolen*/) {
	Bitu ret=0xff;

	switch (ISA_PNP_CUR_ADDR) {
		case 0x01:	/* serial isolation */
			   if (ISA_PNP_selected && ISA_PNP_selected->CSN == 0) {
				   if (ISA_PNP_selected->ident_bp < 72) {
					   if (ISA_PNP_selected->ident[ISA_PNP_selected->ident_bp>>3] & (1 << (ISA_PNP_selected->ident_bp&7)))
						   ret = ISA_PNP_selected->ident_2nd ? 0xAA : 0x55;
					   else
						   ret = 0xFF;

					   if (++ISA_PNP_selected->ident_2nd >= 2) {
						   ISA_PNP_selected->ident_2nd = 0;
						   ISA_PNP_selected->ident_bp++;
					   }
				   }
			   }
			   else {
				   ret = 0xFF;
			   }
			   break;
		case 0x04:	/* read resource data */
			   if (ISA_PNP_selected) {
				   if (ISA_PNP_selected->resource_ident < 9)
					   ret = ISA_PNP_selected->ident[ISA_PNP_selected->resource_ident++];			   
				   else {
					   /* real-world hardware testing shows that devices act as if there was some fixed block of ROM,
					    * that repeats every 128, 256, 512, or 1024 bytes if you just blindly read from this port. */
					   if (ISA_PNP_selected->resource_data_pos < ISA_PNP_selected->resource_data_len)
						   ret = ISA_PNP_selected->resource_data[ISA_PNP_selected->resource_data_pos++];

					   /* that means that if you read enough bytes the ROM loops back to returning the ident */
					   if (ISA_PNP_selected->resource_data_pos >= ISA_PNP_selected->resource_data_len) {
						   ISA_PNP_selected->resource_data_pos = 0;
						   ISA_PNP_selected->resource_ident = 0;
					   }
				   }
			   }
			   break;
		case 0x05:	/* read resource status */
			   if (ISA_PNP_selected) {
				   /* real-world hardware testing shows that devices act as if there was some fixed block of ROM,
				    * that repeats every 128, 256, 512, or 1024 bytes if you just blindly read from this port.
				    * therefore, there's always a byte to return. */
				   ret = 0x01;	/* TODO: simulate hardware slowness */
			   }
			   break;
		case 0x06:	/* card select number */
			   if (ISA_PNP_selected) ret = ISA_PNP_selected->CSN;
			   break;
		case 0x07:	/* logical device number */
			   if (ISA_PNP_selected) ret = ISA_PNP_selected->logical_device;
			   break;
		default:	/* pass the rest down to the class */
			   if (ISA_PNP_selected) ret = ISA_PNP_selected->read(ISA_PNP_CUR_ADDR);
			   break;
	}

//	if (1) fprintf(stderr,"PnP read(%02X) = %02X\n",ISA_PNP_CUR_ADDR,ret);
	return ret;
}

static void isapnp_write_port(Bitu port,Bitu val,Bitu /*iolen*/) {
	Bitu i;

	if (port == 0x279) {
//		if (1) fprintf(stderr,"PnP addr(%02X)\n",val);
		if (val == isa_pnp_init_keystring[ISA_PNP_KEYMATCH]) {
			if (++ISA_PNP_KEYMATCH == 32) {
//				fprintf(stderr,"ISA PnP key -> going to sleep\n");
				ISA_PNP_CUR_STATE = ISA_PNP_SLEEP;
				ISA_PNP_KEYMATCH = 0;
				for (i=0;i < MAX_ISA_PNP_DEVICES;i++) {
					if (ISA_PNP_devs[i] != NULL) {
						ISA_PNP_devs[i]->on_pnp_key();
					}
				}
			}
		}
		else {
			ISA_PNP_KEYMATCH = 0;
		}

		ISA_PNP_CUR_ADDR = val;
	}
	else if (port == 0xA79) {
//		if (1) fprintf(stderr,"PnP write(%02X) = %02X\n",ISA_PNP_CUR_ADDR,val);
		switch (ISA_PNP_CUR_ADDR) {
			case 0x00: {	/* RD_DATA */
				unsigned int np = ((val & 0xFF) << 2) | 3;
				if (np != ISA_PNP_WPORT) {
					unsigned int old = ISA_PNP_WPORT;
					ISA_PNP_WPORT = np;
					delete ISAPNP_PNP_READ_PORT;
					ISAPNP_PNP_READ_PORT = new IO_ReadHandleObject;
					//fprintf(stderr,"PNP OS changed I/O read port to 0x%03X (from 0x%03X)\n",ISA_PNP_WPORT,old);
					ISAPNP_PNP_READ_PORT->Install(ISA_PNP_WPORT,isapnp_read_port,IO_MB);
					if (ISA_PNP_selected != NULL) {
						ISA_PNP_selected->ident_bp = 0;
						ISA_PNP_selected->ident_2nd = 0;
						ISA_PNP_selected->resource_data_pos = 0;
					}
				}
			} break;
			case 0x02:	/* config control */
				   if (val & 4) {
					   /* ALL CARDS RESET CSN to 0 */
					   for (i=0;i < MAX_ISA_PNP_DEVICES;i++) {
						   if (ISA_PNP_devs[i] != NULL) {
							   ISA_PNP_devs[i]->CSN = 0;
						   }
					   }
				   }
				   if (val & 2) ISA_PNP_CUR_STATE = ISA_PNP_WAIT_FOR_KEY;
				   if ((val & 1) && ISA_PNP_selected) ISA_PNP_selected->config(val);
				   for (i=0;i < MAX_ISA_PNP_DEVICES;i++) {
					   if (ISA_PNP_devs[i] != NULL) {
						   ISA_PNP_devs[i]->ident_bp = 0;
						   ISA_PNP_devs[i]->ident_2nd = 0;
						   ISA_PNP_devs[i]->resource_data_pos = 0;
					   }
				   }
				   break;
			case 0x03: {	/* wake[CSN] */
				ISA_PNP_selected = NULL;
				for (i=0;ISA_PNP_selected == NULL && i < MAX_ISA_PNP_DEVICES;i++) {
					if (ISA_PNP_devs[i] == NULL)
						continue;
					if (ISA_PNP_devs[i]->CSN == val) {
						ISA_PNP_selected = ISA_PNP_devs[i];
						ISA_PNP_selected->wakecsn(val);
					}
				}
				if (val == 0)
					ISA_PNP_CUR_STATE = ISA_PNP_ISOLATE;
				else
					ISA_PNP_CUR_STATE = ISA_PNP_CONFIG;
				} break;
			case 0x06:	/* card select number */
				if (ISA_PNP_selected) ISA_PNP_selected->CSN = val;
				break;
			case 0x07:	/* logical device number */
				if (ISA_PNP_selected) ISA_PNP_selected->select_logical_device(val);
				break;
			default:	/* pass the rest down to the class */
				if (ISA_PNP_selected) ISA_PNP_selected->write(ISA_PNP_CUR_ADDR,val);
				break;
		}
	}
}

void ISAPNP_Cfg_Init(Section *s) {
	Section_prop * section=static_cast<Section_prop *>(s);
	//ISAPNPBIOS = section->Get_bool("isapnpbios");
	ISAPNPBIOS = true;
}

/* the PnP callback registered two entry points. One for real, one for protected mode. */
static Bitu PNPentry_real,PNPentry_prot;

static bool ISAPNP_Verify_BiosSelector(Bitu seg) {
	if (!cpu.pmode || (reg_flags & FLAG_VM)) {
		return (seg == 0xF000);
	} else if (seg == 0)
		return 0;
	else {
#if 1
		/* FIXME: Always return true. But figure out how to ask DOSBox the linear->phys
			  mapping to determine whether the segment's base address maps to 0xF0000.
			  In the meantime disabling this check makes PnP BIOS emulation work with
			  Windows 95 OSR2 which appears to give us a segment mapped to a virtual
			  address rather than linearly mapped to 0xF0000 as Windows 95 original
			  did. */
		return true;
#else
		Descriptor desc;
		cpu.gdt.GetDescriptor(seg,desc);

		/* TODO: Check desc.Type() to make sure it's a writeable data segment */
		return (desc.GetBase() == 0xF0000);
#endif
	}
}

static bool ISAPNP_CPU_ProtMode() {
	if (!cpu.pmode || (reg_flags & FLAG_VM))
		return 0;

	return 1;
}

static Bitu ISAPNP_xlate_address(Bitu far_ptr) {
	if (!cpu.pmode || (reg_flags & FLAG_VM))
		return Real2Phys(far_ptr);
	else {
		Descriptor desc;
		cpu.gdt.GetDescriptor(far_ptr >> 16,desc);

		/* TODO: Check desc.Type() to make sure it's a writeable data segment */
		return (desc.GetBase() + (far_ptr & 0xFFFF));
	}
}

/* TODO: ISA PnP BIOS emulation complete. Now what we need is ISA PnP I/O port emulation. That means catching
 *       writes to 0x279 (ADDRESS), 0xA79 (WRITE_DATA), and emulating a moveable I/O port (READ_DATA) */

class ISAPNP_SysDevNode {
public:
	ISAPNP_SysDevNode(const unsigned char *ir,int len,bool already_alloc=false) {
		if (already_alloc) {
			raw = (unsigned char*)ir;
			raw_len = len;
			own = false;
		}
		else {
			if (len > 65535) E_Exit("ISAPNP_SysDevNode data too long");
			raw = new unsigned char[len+1];
			if (ir == NULL) E_Exit("ISAPNP_SysDevNode cannot allocate buffer");
			memcpy(raw,ir,len);
			raw_len = len;
			raw[len] = 0;
			own = true;
		}
	}
	~ISAPNP_SysDevNode() {
		if (own) delete[] raw;
	}
public:
	unsigned char*		raw;
	int			raw_len;
	bool			own;
};

static const unsigned char ISAPNP_sysdev_Keyboard[] = {
	ISAPNP_SYSDEV_HEADER(
			ISAPNP_ID('P','N','P',0x0,0x3,0x0,0x3), /* PNP0303 IBM Enhanced 101/102 key with PS/2 */
			ISAPNP_TYPE(0x09,0x00,0x00),		/* type: input, keyboard */
			0x0001 | 0x0002),			/* can't disable, can't configure */
	/*----------allocated--------*/
	ISAPNP_IO_RANGE(
			0x01,					/* decodes 16-bit ISA addr */
			0x60,0x60,				/* min-max range I/O port */
			0x01,0x01),				/* align=1 length=1 */
	ISAPNP_IO_RANGE(
			0x01,					/* decodes 16-bit ISA addr */
			0x64,0x64,				/* min-max range I/O port */
			0x01,0x01),				/* align=1 length=1 */
	ISAPNP_IRQ_SINGLE(
			1,					/* IRQ 1 */
			0x09),					/* HTE=1 LTL=1 */
	ISAPNP_END,
	/*----------possible--------*/
	ISAPNP_END,
	/*----------compatible--------*/
	ISAPNP_END
};

static const unsigned char ISAPNP_sysdev_Mouse[] = {
	ISAPNP_SYSDEV_HEADER(
			ISAPNP_ID('P','N','P',0x0,0xF,0x0,0xE), /* PNP0F0E Microsoft compatible PS/2 */
			ISAPNP_TYPE(0x09,0x02,0x00),		/* type: input, keyboard */
			0x0001 | 0x0002),			/* can't disable, can't configure */
	/*----------allocated--------*/
	ISAPNP_IRQ_SINGLE(
			12,					/* IRQ 12 */
			0x09),					/* HTE=1 LTL=1 */
	ISAPNP_END,
	/*----------possible--------*/
	ISAPNP_END,
	/*----------compatible--------*/
	ISAPNP_END
};

static const unsigned char ISAPNP_sysdev_DMA_Controller[] = {
	ISAPNP_SYSDEV_HEADER(
			ISAPNP_ID('P','N','P',0x0,0x2,0x0,0x0), /* PNP0200 AT DMA controller */
			ISAPNP_TYPE(0x08,0x01,0x00),		/* type: input, keyboard */
			0x0001 | 0x0002),			/* can't disable, can't configure */
	/*----------allocated--------*/
	ISAPNP_IO_RANGE(
			0x01,					/* decodes 16-bit ISA addr */
			0x00,0x00,				/* min-max range I/O port (DMA channels 0-3) */
			0x10,0x10),				/* align=16 length=16 */
	ISAPNP_IO_RANGE(
			0x01,					/* decodes 16-bit ISA addr */
			0x81,0x81,				/* min-max range I/O port (DMA page registers) */
			0x01,0x0F),				/* align=1 length=15 */
	ISAPNP_IO_RANGE(
			0x01,					/* decodes 16-bit ISA addr */
			0xC0,0xC0,				/* min-max range I/O port (AT DMA channels 4-7) */
			0x20,0x20),				/* align=32 length=32 */
	ISAPNP_DMA_SINGLE(
			4,					/* DMA 4 */
			0x01),					/* 8/16-bit transfers, compatible speed */
	ISAPNP_END,
	/*----------possible--------*/
	ISAPNP_END,
	/*----------compatible--------*/
	ISAPNP_END
};

static const unsigned char ISAPNP_sysdev_PIC[] = {
	ISAPNP_SYSDEV_HEADER(
			ISAPNP_ID('P','N','P',0x0,0x0,0x0,0x0), /* PNP0000 Interrupt controller */
			ISAPNP_TYPE(0x08,0x00,0x01),		/* type: ISA interrupt controller */
			0x0001 | 0x0002),			/* can't disable, can't configure */
	/*----------allocated--------*/
	ISAPNP_IO_RANGE(
			0x01,					/* decodes 16-bit ISA addr */
			0x20,0x20,				/* min-max range I/O port */
			0x01,0x02),				/* align=1 length=2 */
	ISAPNP_IO_RANGE(
			0x01,					/* decodes 16-bit ISA addr */
			0xA0,0xA0,				/* min-max range I/O port */
			0x01,0x02),				/* align=1 length=2 */
	ISAPNP_IRQ_SINGLE(
			2,					/* IRQ 2 */
			0x09),					/* HTE=1 LTL=1 */
	ISAPNP_END,
	/*----------possible--------*/
	ISAPNP_END,
	/*----------compatible--------*/
	ISAPNP_END
};

static const unsigned char ISAPNP_sysdev_Timer[] = {
	ISAPNP_SYSDEV_HEADER(
			ISAPNP_ID('P','N','P',0x0,0x1,0x0,0x0), /* PNP0100 Timer */
			ISAPNP_TYPE(0x08,0x02,0x01),		/* type: ISA timer */
			0x0001 | 0x0002),			/* can't disable, can't configure */
	/*----------allocated--------*/
	ISAPNP_IO_RANGE(
			0x01,					/* decodes 16-bit ISA addr */
			0x40,0x40,				/* min-max range I/O port */
			0x04,0x04),				/* align=4 length=4 */
	ISAPNP_IRQ_SINGLE(
			0,					/* IRQ 0 */
			0x09),					/* HTE=1 LTL=1 */
	ISAPNP_END,
	/*----------possible--------*/
	ISAPNP_END,
	/*----------compatible--------*/
	ISAPNP_END
};

static const unsigned char ISAPNP_sysdev_RTC[] = {
	ISAPNP_SYSDEV_HEADER(
			ISAPNP_ID('P','N','P',0x0,0xB,0x0,0x0), /* PNP0B00 Real-time clock */
			ISAPNP_TYPE(0x08,0x03,0x01),		/* type: ISA real-time clock */
			0x0001 | 0x0002),			/* can't disable, can't configure */
	/*----------allocated--------*/
	ISAPNP_IO_RANGE(
			0x01,					/* decodes 16-bit ISA addr */
			0x70,0x70,				/* min-max range I/O port */
			0x01,0x02),				/* align=1 length=2 */
	ISAPNP_IRQ_SINGLE(
			8,					/* IRQ 8 */
			0x09),					/* HTE=1 LTL=1 */
	ISAPNP_END,
	/*----------possible--------*/
	ISAPNP_END,
	/*----------compatible--------*/
	ISAPNP_END
};

static const unsigned char ISAPNP_sysdev_PC_Speaker[] = {
	ISAPNP_SYSDEV_HEADER(
			ISAPNP_ID('P','N','P',0x0,0x8,0x0,0x0), /* PNP0800 PC speaker */
			ISAPNP_TYPE(0x04,0x01,0x00),		/* type: PC speaker */
			0x0001 | 0x0002),			/* can't disable, can't configure */
	/*----------allocated--------*/
	ISAPNP_IO_RANGE(
			0x01,					/* decodes 16-bit ISA addr */
			0x61,0x61,				/* min-max range I/O port */
			0x01,0x01),				/* align=1 length=1 */
	ISAPNP_END,
	/*----------possible--------*/
	ISAPNP_END,
	/*----------compatible--------*/
	ISAPNP_END
};

static const unsigned char ISAPNP_sysdev_Numeric_Coprocessor[] = {
	ISAPNP_SYSDEV_HEADER(
			ISAPNP_ID('P','N','P',0x0,0xC,0x0,0x4), /* PNP0C04 Numeric Coprocessor */
			ISAPNP_TYPE(0x0B,0x80,0x00),		/* type: FPU */
			0x0001 | 0x0002),			/* can't disable, can't configure */
	/*----------allocated--------*/
	ISAPNP_IO_RANGE(
			0x01,					/* decodes 16-bit ISA addr */
			0xF0,0xF0,				/* min-max range I/O port */
			0x10,0x10),				/* align=16 length=16 */
	ISAPNP_IRQ_SINGLE(
			13,					/* IRQ 13 */
			0x09),					/* HTE=1 LTL=1 */
	ISAPNP_END,
	/*----------possible--------*/
	ISAPNP_END,
	/*----------compatible--------*/
	ISAPNP_END
};

static const unsigned char ISAPNP_sysdev_System_Board[] = {
	ISAPNP_SYSDEV_HEADER(
			ISAPNP_ID('P','N','P',0x0,0xC,0x0,0x1), /* PNP0C01 System board */
			ISAPNP_TYPE(0x08,0x80,0x00),		/* type: System peripheral, Other */
			0x0001 | 0x0002),			/* can't disable, can't configure */
	/*----------allocated--------*/
	ISAPNP_IO_RANGE(
			0x01,					/* decodes 16-bit ISA addr */
			0x24,0x24,				/* min-max range I/O port */
			0x01,0x0C),				/* align=1 length=12 */
	ISAPNP_END,
	/*----------possible--------*/
	ISAPNP_END,
	/*----------compatible--------*/
	ISAPNP_END
};

static ISAPNP_SysDevNode*	ISAPNP_SysDevNodes[256];
static Bitu			ISAPNP_SysDevNodeCount=0;
static Bitu			ISAPNP_SysDevNodeLargest=0;

bool ISAPNP_RegisterSysDev(const unsigned char *raw,int len,bool already) {
	if (ISAPNP_SysDevNodeCount >= 0xFF)
		return false;

	ISAPNP_SysDevNodes[ISAPNP_SysDevNodeCount] = new ISAPNP_SysDevNode(raw,len,already);
	if (ISAPNP_SysDevNodes[ISAPNP_SysDevNodeCount] == NULL)
		return false;
	
	ISAPNP_SysDevNodeCount++;
	if (ISAPNP_SysDevNodeLargest < (len+3))
		ISAPNP_SysDevNodeLargest = len+3;

	return true;
}

/* ISA PnP function calls have their parameters stored on the stack "C" __cdecl style. Parameters
 * are either int, long, or FAR pointers. Like __cdecl an assembly language implementation pushes
 * the function arguments on the stack BACKWARDS */
static Bitu ISAPNP_Handler(bool protmode /* called from protected mode interface == true */) {
	Bitu arg;
	Bitu func,BiosSelector;

	/* I like how the ISA PnP spec says that the 16-bit entry points (real and protected) are given 16-bit data segments
	 * which implies that all segments involved might as well be 16-bit.
	 *
	 * Right?
	 *
	 * Well, guess what Windows 95 gives us when calling this entry point:
	 *
	 *     Segment SS = DS = 0x30  base=0 limit=0xFFFFFFFF
	 *       SS:SP = 0x30:0xC138BADF or something like that from within BIOS.VXD
	 *
	 * Yeah... for a 16-bit code segment call. Right. Typical Microsoft. >:(
	 *
	 * ------------------------------------------------------------------------
	 * Windows 95 OSR2:
	 *
	 * Windows 95 OSR2 however uses a 16-bit stack (where the stack segment is based somewhere
	 * around 0xC1xxxxxx), all we have to do to correctly access it is work through the page tables.
	 * This is within spec, but now Microsoft sends us a data segment that is based at virtual address
	 * 0xC2xxxxxx, which is why I had to disable the "verify selector" routine */
	if (protmode)
		arg = SegPhys(ss) + reg_esp + (2*2); /* entry point (real and protected) is 16-bit, expected to RETF (skip CS:IP) */
	else
		arg = SegPhys(ss) + reg_sp + (2*2); /* entry point (real and protected) is 16-bit, expected to RETF (skip CS:IP) */

	if (protmode != ISAPNP_CPU_ProtMode()) {
		//fprintf(stderr,"ISA PnP %s entry point called from %s. On real BIOSes this would CRASH\n",protmode ? "Protected mode" : "Real mode",
		//	ISAPNP_CPU_ProtMode() ? "Protected mode" : "Real mode");
		reg_ax = 0x84;/* BAD_PARAMETER */
		return 0;
	}

	fprintf(stderr,"PnP prot=%u DS=%04x (base=0x%08lx) SS:ESP=%04x:%04x (base=0x%08lx phys=0x%08lx)\n",protmode,
		SegValue(ds),SegPhys(ds),
		SegValue(ss),reg_esp,SegPhys(ss),arg);

	/* every function takes the form
	 *
	 * int __cdecl FAR (*entrypoint)(int Function...);
	 *
	 * so the first argument on the stack is an int that we read to determine what the caller is asking
	 *
	 * Dont forget in the real-mode world:
	 *    sizeof(int) == 16 bits
	 *    sizeof(long) == 32 bits
	 */    
	func = mem_readw(arg);
	switch (func) {
		case 0: {		/* Get Number of System Nodes */
			/* int __cdecl FAR (*entrypoint)(int Function,unsigned char FAR *NumNodes,unsigned int FAR *NodeSize,unsigned int BiosSelector);
			 *                               ^ +0         ^ +2                        ^ +6                       ^ +10                       = 12 */
			Bitu NumNodes_ptr = mem_readd(arg+2);
			Bitu NodeSize_ptr = mem_readd(arg+6);
			BiosSelector = mem_readw(arg+10);

			if (!ISAPNP_Verify_BiosSelector(BiosSelector))
				goto badBiosSelector;

			if (NumNodes_ptr != 0) mem_writeb(ISAPNP_xlate_address(NumNodes_ptr),ISAPNP_SysDevNodeCount);
			if (NodeSize_ptr != 0) mem_writew(ISAPNP_xlate_address(NodeSize_ptr),ISAPNP_SysDevNodeLargest);

			reg_ax = 0x00;/* SUCCESS */
		} break;
		case 1: {		/* Get System Device Node */
			/* int __cdecl FAR (*entrypoint)(int Function,unsigned char FAR *Node,struct DEV_NODE FAR *devNodeBuffer,unsigned int Control,unsigned int BiosSelector);
			 *                               ^ +0         ^ +2                    ^ +6                               ^ +10                ^ +12                       = 14 */
			Bitu Node_ptr = mem_readd(arg+2);
			Bitu devNodeBuffer_ptr = mem_readd(arg+6);
			Bitu Control = mem_readw(arg+10);
			BiosSelector = mem_readw(arg+12);
			unsigned char Node;
			Bitu i;

			if (!ISAPNP_Verify_BiosSelector(BiosSelector))
				goto badBiosSelector;

			/* control bits 0-1 must be '01' or '10' but not '00' or '11' */
			if (Control == 0 || (Control&3) == 3) {
				reg_ax = 0x84;/* BAD_PARAMETER */
				break;
			}

			//fprintf(stderr,"devNodePtr = 0x%08lx\n",devNodeBuffer_ptr);
			devNodeBuffer_ptr = ISAPNP_xlate_address(devNodeBuffer_ptr);
			//fprintf(stderr,"        to = 0x%08lx\n",devNodeBuffer_ptr);

			Node_ptr = ISAPNP_xlate_address(Node_ptr);
			Node = mem_readb(Node_ptr);
			if (Node >= ISAPNP_SysDevNodeCount) {
				reg_ax = 0x84;/* BAD_PARAMETER */
				break;
			}

			ISAPNP_SysDevNode *nd = ISAPNP_SysDevNodes[Node];

			mem_writew(devNodeBuffer_ptr+i+0,nd->raw_len+3); /* Length */
			mem_writeb(devNodeBuffer_ptr+i+2,Node); /* on most PnP BIOS implementations I've seen "handle" is set to the same value as Node */
			for (i=0;i < nd->raw_len;i++)
				mem_writeb(devNodeBuffer_ptr+i+3,nd->raw[i]);

			if (++Node >= ISAPNP_SysDevNodeCount) Node = 0xFF; /* no more nodes */
			mem_writeb(Node_ptr,Node);

			reg_ax = 0x00;/* SUCCESS */
		} break;
		case 4: {		/* Send Message */
			/* int __cdecl FAR (*entrypoint)(int Function,unsigned int Message,unsigned int BiosSelector);
			 *                               ^ +0         ^ +2                 ^ +4                        = 6 */
			Bitu Message = mem_readw(arg+2);
			BiosSelector = mem_readw(arg+4);

			if (!ISAPNP_Verify_BiosSelector(BiosSelector))
				goto badBiosSelector;

			switch (Message) {
				case 0x41:	/* POWER_OFF */
					//fprintf(stderr,"Plug & Play OS requested power off.\n");
					throw 1;	/* NTS: Based on the Reboot handler code, causes DOSBox to cleanly shutdown and exit */
					reg_ax = 0;
					break;
				case 0x42:	/* PNP_OS_ACTIVE */
					//fprintf(stderr,"Plug & Play OS reports itself active\n");
					reg_ax = 0;
					break;
				case 0x43:	/* PNP_OS_INACTIVE */
					//fprintf(stderr,"Plug & Play OS reports itself inactive\n");
					reg_ax = 0;
					break;
				default:
					//fprintf(stderr,"Unknown ISA PnP message 0x%04x\n",Message);
					reg_ax = 0x82;/* FUNCTION_NOT_SUPPORTED */
					break;
			}
		} break;
		case 0x40: {		/* Get PnP ISA configuration */
			/* int __cdecl FAR (*entrypoint)(int Function,unsigned char far *struct,unsigned int BiosSelector);
			 *                               ^ +0         ^ +2                      ^ +6                        = 8 */
			Bitu struct_ptr = mem_readd(arg+2);
			BiosSelector = mem_readw(arg+6);

			if (!ISAPNP_Verify_BiosSelector(BiosSelector))
				goto badBiosSelector;

			/* struct isapnp_pnp_isa_cfg {
				 uint8_t	revision;
				 uint8_t	total_csn;
				 uint16_t	isa_pnp_port;
				 uint16_t	reserved;
			 }; */

			if (struct_ptr != 0) {
				Bitu ph = ISAPNP_xlate_address(struct_ptr);
				mem_writeb(ph+0,0x01);		/* ->revision = 0x01 */
				mem_writeb(ph+1,ISA_PNP_devnext); /* ->total_csn (FIXME) */
				mem_writew(ph+2,ISA_PNP_WPORT_BIOS);	/* ->isa_pnp_port */
				mem_writew(ph+4,0);		/* ->reserved */
			}

			reg_ax = 0x00;/* SUCCESS */
		} break;
		default:
			//fprintf(stderr,"Unsupported ISA PnP function 0x%04x\n",func);
			reg_ax = 0x82;/* FUNCTION_NOT_SUPPORTED */
			break;
	};

	return 0;
badBiosSelector:
	/* return an error. remind the user (possible developer) how lucky he is, a real
	 * BIOS implementation would CRASH when misused like this */
	//fprintf(stderr,"ISA PnP function 0x%04x called with incorrect BiosSelector parameter 0x%04x\n",func,BiosSelector);
	//fprintf(stderr," > STACK %04X %04X %04X %04X %04X %04X %04X %04X\n",
	//	mem_readw(arg),
	//	mem_readw(arg+2),
	//	mem_readw(arg+4),
	//	mem_readw(arg+6),
	//	mem_readw(arg+8),
	//	mem_readw(arg+10),
	//	mem_readw(arg+12),
	//	mem_readw(arg+14));

	if (cpu.pmode && !(reg_flags & FLAG_VM) && BiosSelector != 0) {
		Descriptor desc;

		//if (cpu.gdt.GetDescriptor(BiosSelector,desc))
			//fprintf(stderr," > BiosSelector base=0x%08lx\n",(unsigned long)desc.GetBase());
		//else
			//fprintf(stderr," > BiosSelector N/A\n");
	}

	reg_ax = 0x84;/* BAD_PARAMETER */
	return 0;
}

static Bitu ISAPNP_Handler_PM(void) {
	return ISAPNP_Handler(true);
}

static Bitu ISAPNP_Handler_RM(void) {
	return ISAPNP_Handler(false);
}
