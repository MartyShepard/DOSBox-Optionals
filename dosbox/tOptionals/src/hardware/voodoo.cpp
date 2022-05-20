/*
 *  Copyright (C) 2002-2011  The DOSBox Team
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "dosbox.h"
#include "config.h"
#include "setup.h"
#include "cross.h"
#include "control.h"

#include "paging.h"
#include "mem.h"

#include "voodoo.h"
#include "pci_bus.h"
#include "voodoo_interface.h"
#include <timer.h>

std::string nCurrent_Voodoo = "";

class VOODOO;
static VOODOO* voodoo_dev;

static Bit32u voodoo_current_lfb=(VOODOO_INITIAL_LFB&0xffff0000);

static bool voodoo_pci_enabled = false;
static MEM_Callout_t voodoo_lfb_cb = MEM_Callout_t_none;

PageHandler* voodoo_lfb_memio_cb(MEM_CalloutObject &co,Bitu phys_page) {
    (void)phys_page;//UNUSED
    (void)co;//UNUSED
    if (voodoo_current_lfb == 0)
        return NULL;

    return VOODOO_GetPageHandler();
}
void voodoo_lfb_cb_free(void) {
    // if (voodoo_lfb_cb != MEM_Callout_t_none) {
        // MEM_FreeCallout(voodoo_lfb_cb);
        // voodoo_lfb_cb = MEM_Callout_t_none;
    // }
}

void voodoo_lfb_cb_init() {
    // if (voodoo_lfb_cb == MEM_Callout_t_none) {
        // voodoo_lfb_cb = MEM_AllocateCallout(MEM_TYPE_PCI);
        // if (voodoo_lfb_cb == MEM_Callout_t_none) E_Exit("Unable to allocate voodoo cb for LFB");
    // }

    // {
        // MEM_CalloutObject *cb = MEM_GetCallout(voodoo_lfb_cb);

        // assert(cb != NULL);

        // cb->Uninstall();

        // if (voodoo_current_lfb != 0 && voodoo_pci_enabled) {
            // /* VOODOO_PAGES is a power of two already */
            // LOG_MSG("VOODOO LFB now at %x",voodoo_current_lfb);
            // cb->Install(voodoo_current_lfb>>12UL,MEMMASK_Combine(MEMMASK_FULL,MEMMASK_Range(VOODOO_PAGES)),voodoo_lfb_memio_cb);
        // }
        // else {
            // LOG_MSG("VOODOO LFB disabled");
        // }

        // MEM_PutCallout(cb);
    // }
}

class VOODOO:public Module_base{
private:
	Bits emulation_type;
public:
	VOODOO(Section* configuration):Module_base(configuration){
		emulation_type=-1;
		
		Section_prop * section=static_cast<Section_prop *>(configuration);
		
		std::string voodoo_type_str(section->Get_string("voodoo"));
		if (voodoo_type_str=="false")
		{
			
			emulation_type=0;
		}
		
		else if (voodoo_type_str=="software")
		{
			nCurrent_Voodoo = "3DFX (Software Mode)";
			emulation_type=1;

		}
		
		else if ((voodoo_type_str=="opengl") || (voodoo_type_str=="auto"))
		{
			nCurrent_Voodoo = "3dfx Voodoo Graphics (SST-1)";
			emulation_type=2;

		}
		
		else if (voodoo_type_str=="auto")
		{
			nCurrent_Voodoo = "3DFX (Software Mode)";	
			emulation_type=1;

		}
		
		else
		{
			emulation_type=0;
		}

		if (emulation_type<=0) return;


		Bits card_type = 1;
		bool max_voodoomem;
		std::string voodoo_mem_str(section->Get_string("Voodoo_Memory"));
		if (voodoo_mem_str=="max") {
			max_voodoomem = true;
		} else {
			max_voodoomem = false;
		}

		bool needs_pci_device = false;

		switch (emulation_type) {
			case 1:
			case 2:
				Voodoo_Initialize(emulation_type, card_type, max_voodoomem);
				needs_pci_device = true;
				#if defined(C_DEBUG)
					LOG(LOG_FXEMU, LOG_NORMAL)("[%d] Init Emuation Type (%d), Card Type: %d, MaxMem: %s", __LINE__, emulation_type, card_type, max_voodoomem? "true":"false");
				#endif
				break;
			default:
				break;
		}

		if (needs_pci_device) PCI_AddSST_Device((Bitu)card_type);
	}

	~VOODOO(){
		PCI_RemoveSST_Device();

		if (emulation_type<=0) return;

		switch (emulation_type) {
			case 1:
			case 2:
				Voodoo_Shut_Down();
				break;
			default:
				break;
		}

		emulation_type=-1;
	}

	void PCI_InitEnable(Bitu val) {
		switch (emulation_type) {
			case 1:
			case 2:
				Voodoo_PCI_InitEnable(val);
				break;
			default:
				break;
		}
	}

	void PCI_Enable(bool enable) {
		switch (emulation_type) {
			case 1:
			case 2:
				Voodoo_PCI_Enable(enable);
				break;
			default:
				break;
		}
	}

	PageHandler* GetPageHandler() {
		switch (emulation_type) {
			case 1:
			case 2:
				return Voodoo_GetPageHandler();
			default:
				break;
		}
		return NULL;
	}

};


void VOODOO_PCI_InitEnable(Bitu val) {
	if (voodoo_dev!=NULL) {
		voodoo_dev->PCI_InitEnable(val);
	}
}

void VOODOO_PCI_Enable(bool enable) {
	if (voodoo_dev!=NULL) {
		voodoo_dev->PCI_Enable(enable);
	}
}


void VOODOO_PCI_SetLFB(Bit32u lfbaddr) {
	lfbaddr &= 0xFFFF0000UL;
    if (lfbaddr == voodoo_current_lfb)
        return;
    voodoo_current_lfb = lfbaddr;
    voodoo_lfb_cb_init();
}

bool VOODOO_PCI_CheckLFBPage(Bitu page) {
    if (voodoo_current_lfb != 0 &&
        (page>=(voodoo_current_lfb>>12)) &&
        (page<(voodoo_current_lfb>>12)+VOODOO_PAGES))
        return true;
 	return false;
}

PageHandler* VOODOO_GetPageHandler() {
	if (voodoo_dev!=NULL) {
		return voodoo_dev->GetPageHandler();
	}
	return NULL;
}


void VOODOO_Destroy(Section* /*sec*/) {
	delete voodoo_dev;
	voodoo_dev=NULL;
}

void VOODOO_Init(Section* sec) {
					  
	voodoo_current_lfb=(VOODOO_INITIAL_LFB&0xffff0000);
	voodoo_dev = new VOODOO(sec);
	sec->AddDestroyFunction(&VOODOO_Destroy,true);
	
}
