/*
 *  Copyright (C) 2002  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

	CASE_B(0x00)												/* ADD Eb,Gb */
		RMEbGb(ADDB);break;
	CASE_W(0x01)												/* ADD Ew,Gw */
		RMEwGw(ADDW);break;	
	CASE_B(0x02)												/* ADD Gb,Eb */
		RMGbEb(ADDB);break;
	CASE_W(0x03)												/* ADD Gw,Ew */
		RMGwEw(ADDW);break;
	CASE_B(0x04)												/* ADD AL,Ib */
		ALIb(ADDB);break;
	CASE_W(0x05)												/* ADD AX,Iw */
		AXIw(ADDW);break;
	CASE_W(0x06)												/* PUSH ES */		
		Push_16(SegValue(es));break;
	CASE_W(0x07)												/* POP ES */		
		CPU_SetSegGeneral(es,Pop_16());break;
	CASE_B(0x08)												/* OR Eb,Gb */
		RMEbGb(ORB);break;
	CASE_W(0x09)												/* OR Ew,Gw */
		RMEwGw(ORW);break;
	CASE_B(0x0a)												/* OR Gb,Eb */
		RMGbEb(ORB);break;
	CASE_W(0x0b)												/* OR Gw,Ew */
		RMGwEw(ORW);break;
	CASE_B(0x0c)												/* OR AL,Ib */
		ALIb(ORB);break;
	CASE_W(0x0d)												/* OR AX,Iw */
		AXIw(ORW);break;
	CASE_W(0x0e)												/* PUSH CS */		
		Push_16(SegValue(cs));break;
	CASE_B(0x0f)												/* 2 byte opcodes*/		
		core.opcode_index|=OPCODE_0F;
		goto restart_opcode;
		break;
	CASE_B(0x10)												/* ADC Eb,Gb */
		RMEbGb(ADCB);break;
	CASE_W(0x11)												/* ADC Ew,Gw */
		RMEwGw(ADCW);break;	
	CASE_B(0x12)												/* ADC Gb,Eb */
		RMGbEb(ADCB);break;
	CASE_W(0x13)												/* ADC Gw,Ew */
		RMGwEw(ADCW);break;
	CASE_B(0x14)												/* ADC AL,Ib */
		ALIb(ADCB);break;
	CASE_W(0x15)												/* ADC AX,Iw */
		AXIw(ADCW);break;
	CASE_W(0x16)												/* PUSH SS */		
		Push_16(SegValue(ss));break;
	CASE_W(0x17)												/* POP SS */		
		CPU_SetSegGeneral(ss,Pop_16());break;
	CASE_B(0x18)												/* SBB Eb,Gb */
		RMEbGb(SBBB);break;
	CASE_W(0x19)												/* SBB Ew,Gw */
		RMEwGw(SBBW);break;
	CASE_B(0x1a)												/* SBB Gb,Eb */
		RMGbEb(SBBB);break;
	CASE_W(0x1b)												/* SBB Gw,Ew */
		RMGwEw(SBBW);break;
	CASE_B(0x1c)												/* SBB AL,Ib */
		ALIb(SBBB);break;
	CASE_W(0x1d)												/* SBB AX,Iw */
		AXIw(SBBW);break;
	CASE_W(0x1e)												/* PUSH DS */		
		Push_16(SegValue(ds));break;
	CASE_W(0x1f)												/* POP DS */
		CPU_SetSegGeneral(ds,Pop_16());break;
	CASE_B(0x20)												/* AND Eb,Gb */
		RMEbGb(ANDB);break;
	CASE_W(0x21)												/* AND Ew,Gw */
		RMEwGw(ANDW);break;	
	CASE_B(0x22)												/* AND Gb,Eb */
		RMGbEb(ANDB);break;
	CASE_W(0x23)												/* AND Gw,Ew */
		RMGwEw(ANDW);break;
	CASE_B(0x24)												/* AND AL,Ib */
		ALIb(ANDB);break;
	CASE_W(0x25)												/* AND AX,Iw */
		AXIw(ANDW);break;
	CASE_B(0x26)												/* SEG ES: */
		DO_PREFIX_SEG(es);break;
	CASE_B(0x27)												/* DAA */
		DAA();break;
	CASE_B(0x28)												/* SUB Eb,Gb */
		RMEbGb(SUBB);break;
	CASE_W(0x29)												/* SUB Ew,Gw */
		RMEwGw(SUBW);break;
	CASE_B(0x2a)												/* SUB Gb,Eb */
		RMGbEb(SUBB);break;
	CASE_W(0x2b)												/* SUB Gw,Ew */
		RMGwEw(SUBW);break;
	CASE_B(0x2c)												/* SUB AL,Ib */
		ALIb(SUBB);break;
	CASE_W(0x2d)												/* SUB AX,Iw */
		AXIw(SUBW);break;
	CASE_B(0x2e)												/* SEG CS: */
		DO_PREFIX_SEG(cs);break;
	CASE_B(0x2f)												/* DAS */
		DAS();break;  
	CASE_B(0x30)												/* XOR Eb,Gb */
		RMEbGb(XORB);break;
	CASE_W(0x31)												/* XOR Ew,Gw */
		RMEwGw(XORW);break;	
	CASE_B(0x32)												/* XOR Gb,Eb */
		RMGbEb(XORB);break;
	CASE_W(0x33)												/* XOR Gw,Ew */
		RMGwEw(XORW);break;
	CASE_B(0x34)												/* XOR AL,Ib */
		ALIb(XORB);break;
	CASE_W(0x35)												/* XOR AX,Iw */
		AXIw(XORW);break;
	CASE_B(0x36)												/* SEG SS: */
		DO_PREFIX_SEG(ss);break;
	CASE_B(0x37)												/* AAA */
		AAA();break;  
	CASE_B(0x38)												/* CMP Eb,Gb */
		RMEbGb(CMPB);break;
	CASE_W(0x39)												/* CMP Ew,Gw */
		RMEwGw(CMPW);break;
	CASE_B(0x3a)												/* CMP Gb,Eb */
		RMGbEb(CMPB);break;
	CASE_W(0x3b)												/* CMP Gw,Ew */
		RMGwEw(CMPW);break;
	CASE_B(0x3c)												/* CMP AL,Ib */
		ALIb(CMPB);break;
	CASE_W(0x3d)												/* CMP AX,Iw */
		AXIw(CMPW);break;
	CASE_B(0x3e)												/* SEG DS: */
		DO_PREFIX_SEG(ds);break;
	CASE_B(0x3f)												/* AAS */
		AAS();break;
	CASE_W(0x40)												/* INC AX */
		INCW(reg_ax,LoadRw,SaveRw);break;
	CASE_W(0x41)												/* INC CX */
		INCW(reg_cx,LoadRw,SaveRw);break;
	CASE_W(0x42)												/* INC DX */
		INCW(reg_dx,LoadRw,SaveRw);break;
	CASE_W(0x43)												/* INC BX */
		INCW(reg_bx,LoadRw,SaveRw);break;
	CASE_W(0x44)												/* INC SP */
		INCW(reg_sp,LoadRw,SaveRw);break;
	CASE_W(0x45)												/* INC BP */
		INCW(reg_bp,LoadRw,SaveRw);break;
	CASE_W(0x46)												/* INC SI */
		INCW(reg_si,LoadRw,SaveRw);break;
	CASE_W(0x47)												/* INC DI */
		INCW(reg_di,LoadRw,SaveRw);break;
	CASE_W(0x48)												/* DEC AX */
		DECW(reg_ax,LoadRw,SaveRw);break;
	CASE_W(0x49)												/* DEC CX */
  		DECW(reg_cx,LoadRw,SaveRw);break;
	CASE_W(0x4a)												/* DEC DX */
		DECW(reg_dx,LoadRw,SaveRw);break;
	CASE_W(0x4b)												/* DEC BX */
		DECW(reg_bx,LoadRw,SaveRw);break;
	CASE_W(0x4c)												/* DEC SP */
		DECW(reg_sp,LoadRw,SaveRw);break;
	CASE_W(0x4d)												/* DEC BP */
		DECW(reg_bp,LoadRw,SaveRw);break;
	CASE_W(0x4e)												/* DEC SI */
		DECW(reg_si,LoadRw,SaveRw);break;
	CASE_W(0x4f)												/* DEC DI */
		DECW(reg_di,LoadRw,SaveRw);break;
	CASE_W(0x50)												/* PUSH AX */
		Push_16(reg_ax);break;
	CASE_W(0x51)												/* PUSH CX */
		Push_16(reg_cx);break;
	CASE_W(0x52)												/* PUSH DX */
		Push_16(reg_dx);break;
	CASE_W(0x53)												/* PUSH BX */
		Push_16(reg_bx);break;
	CASE_W(0x54)												/* PUSH SP */
//TODO Check if this is correct i think it's SP+2 or something
		Push_16(reg_sp);break;
	CASE_W(0x55)												/* PUSH BP */
		Push_16(reg_bp);break;
	CASE_W(0x56)												/* PUSH SI */
		Push_16(reg_si);break;
	CASE_W(0x57)												/* PUSH DI */
		Push_16(reg_di);break;
	CASE_W(0x58)												/* POP AX */
		reg_ax=Pop_16();break;
	CASE_W(0x59)												/* POP CX */
		reg_cx=Pop_16();break;
	CASE_W(0x5a)												/* POP DX */
		reg_dx=Pop_16();break;
	CASE_W(0x5b)												/* POP BX */
		reg_bx=Pop_16();break;
	CASE_W(0x5c)												/* POP SP */
		reg_sp=Pop_16();break;
	CASE_W(0x5d)												/* POP BP */
		reg_bp=Pop_16();break;
	CASE_W(0x5e)												/* POP SI */
		reg_si=Pop_16();break;
	CASE_W(0x5f)												/* POP DI */
		reg_di=Pop_16();break;
	CASE_W(0x60)												/* PUSHA */
		{
			Bit16u old_sp=reg_sp;
			Push_16(reg_ax);Push_16(reg_cx);Push_16(reg_dx);Push_16(reg_bx);
			Push_16(old_sp);Push_16(reg_bp);Push_16(reg_si);Push_16(reg_di);
		}
		break;
	CASE_W(0x61)												/* POPA */
		reg_di=Pop_16();reg_si=Pop_16();reg_bp=Pop_16();Pop_16();//Don't save SP
		reg_bx=Pop_16();reg_dx=Pop_16();reg_cx=Pop_16();reg_ax=Pop_16();
		break;
	CASE_W(0x62)												/* BOUND */
		{
			Bit16s bound_min, bound_max;
			GetRMrw;GetEAa;
			bound_min=LoadMw(eaa);
			bound_max=LoadMw(eaa+2);
			if ( (((Bit16s)*rmrw) < bound_min) || (((Bit16s)*rmrw) > bound_max) ) {
				EXCEPTION(5);
			}
		}
		break;
	CASE_W(0x63)												/* ARPL Ew,Rw */
		{
			FillFlags();
			GetRMrw;
			if (rm >= 0xc0 ) {
				GetEArw;Bitu new_sel=*earw;
				CPU_ARPL(new_sel,*rmrw);
				*earw=(Bit16u)new_sel;
			} else {
				GetEAa;Bitu new_sel=LoadMw(eaa);
				CPU_ARPL(new_sel,*rmrw);
				SaveMw(eaa,(Bit16u)new_sel);
			}
		}
		break;
	CASE_B(0x64)												/* SEG FS: */
		DO_PREFIX_SEG(fs);break;
	CASE_B(0x65)												/* SEG GS: */
		DO_PREFIX_SEG(gs);break;
	CASE_B(0x66)												/* Operand Size Prefix */
		core.opcode_index^=OPCODE_SIZE;
		goto restart_opcode;
	CASE_B(0x67)												/* Address Size Prefix */
		DO_PREFIX_ADDR();
	CASE_W(0x68)												/* PUSH Iw */
		Push_16(Fetchw());break;
	CASE_W(0x69)												/* IMUL Gw,Ew,Iw */
		RMGwEwOp3(DIMULW,Fetchws());
		break;
	CASE_W(0x6a)												/* PUSH Ib */
		Push_16(Fetchbs());
		break;
	CASE_W(0x6b)												/* IMUL Gw,Ew,Ib */
		RMGwEwOp3(DIMULW,Fetchbs());
		break;
	CASE_B(0x6c)												/* INSB */
		DoString(R_INSB);break;
	CASE_W(0x6d)												/* INSW */
		DoString(R_INSW);break;
	CASE_B(0x6e)												/* OUTSB */
		DoString(R_OUTSB);break;
	CASE_W(0x6f)												/* OUTSW */
		DoString(R_OUTSW);break;
	CASE_B(0x70)												/* JO */
		JumpSIb(get_OF());break;
	CASE_B(0x71)												/* JNO */
		JumpSIb(!get_OF());break;
	CASE_B(0x72)												/* JB */
		JumpSIb(get_CF());break;
	CASE_B(0x73)												/* JNB */
		JumpSIb(!get_CF());break;
	CASE_B(0x74)												/* JZ */
  		JumpSIb(get_ZF());break;
	CASE_B(0x75)												/* JNZ */
		JumpSIb(!get_ZF());break;
	CASE_B(0x76)												/* JBE */
		JumpSIb(get_CF() || get_ZF());break;
	CASE_B(0x77)												/* JNBE */
		JumpSIb(!get_CF() && !get_ZF());break;
	CASE_B(0x78)												/* JS */
		JumpSIb(get_SF());break;
	CASE_B(0x79)												/* JNS */
		JumpSIb(!get_SF());break;
	CASE_B(0x7a)												/* JP */
		JumpSIb(get_PF());break;
	CASE_B(0x7b)												/* JNP */
		JumpSIb(!get_PF());break;
	CASE_B(0x7c)												/* JL */
		JumpSIb(get_SF() != get_OF());break;
	CASE_B(0x7d)												/* JNL */
		JumpSIb(get_SF() == get_OF());break;
	CASE_B(0x7e)												/* JLE */
		JumpSIb(get_ZF() || (get_SF() != get_OF()));break;
	CASE_B(0x7f)												/* JNLE */
		JumpSIb((get_SF() == get_OF()) && !get_ZF());break;
	CASE_B(0x80)												/* Grpl Eb,Ib */
	CASE_B(0x82)												/* Grpl Eb,Ib Mirror instruction*/
		{
			GetRM;Bitu which=(rm>>3)&7;
			if (rm>= 0xc0) {
				GetEArb;Bit8u ib=Fetchb();
				switch (which) {
				case 0x00:ADDB(*earb,ib,LoadRb,SaveRb);break;
				case 0x01: ORB(*earb,ib,LoadRb,SaveRb);break;
				case 0x02:ADCB(*earb,ib,LoadRb,SaveRb);break;
				case 0x03:SBBB(*earb,ib,LoadRb,SaveRb);break;
				case 0x04:ANDB(*earb,ib,LoadRb,SaveRb);break;
				case 0x05:SUBB(*earb,ib,LoadRb,SaveRb);break;
				case 0x06:XORB(*earb,ib,LoadRb,SaveRb);break;
				case 0x07:CMPB(*earb,ib,LoadRb,SaveRb);break;
				}
			} else {
				GetEAa;Bit8u ib=Fetchb();
				switch (which) {
				case 0x00:ADDB(eaa,ib,LoadMb,SaveMb);break;
				case 0x01: ORB(eaa,ib,LoadMb,SaveMb);break;
				case 0x02:ADCB(eaa,ib,LoadMb,SaveMb);break;
				case 0x03:SBBB(eaa,ib,LoadMb,SaveMb);break;
				case 0x04:ANDB(eaa,ib,LoadMb,SaveMb);break;
				case 0x05:SUBB(eaa,ib,LoadMb,SaveMb);break;
				case 0x06:XORB(eaa,ib,LoadMb,SaveMb);break;
				case 0x07:CMPB(eaa,ib,LoadMb,SaveMb);break;
				}
			}
			break;
		}
	CASE_W(0x81)												/* Grpl Ew,Iw */
		{
			GetRM;Bitu which=(rm>>3)&7;
			if (rm>= 0xc0) {
				GetEArw;Bit16u iw=Fetchw();
				switch (which) {
				case 0x00:ADDW(*earw,iw,LoadRw,SaveRw);break;
				case 0x01: ORW(*earw,iw,LoadRw,SaveRw);break;
				case 0x02:ADCW(*earw,iw,LoadRw,SaveRw);break;
				case 0x03:SBBW(*earw,iw,LoadRw,SaveRw);break;
				case 0x04:ANDW(*earw,iw,LoadRw,SaveRw);break;
				case 0x05:SUBW(*earw,iw,LoadRw,SaveRw);break;
				case 0x06:XORW(*earw,iw,LoadRw,SaveRw);break;
				case 0x07:CMPW(*earw,iw,LoadRw,SaveRw);break;
				}
			} else {
				GetEAa;Bit16u iw=Fetchw();
				switch (which) {
				case 0x00:ADDW(eaa,iw,LoadMw,SaveMw);break;
				case 0x01: ORW(eaa,iw,LoadMw,SaveMw);break;
				case 0x02:ADCW(eaa,iw,LoadMw,SaveMw);break;
				case 0x03:SBBW(eaa,iw,LoadMw,SaveMw);break;
				case 0x04:ANDW(eaa,iw,LoadMw,SaveMw);break;
				case 0x05:SUBW(eaa,iw,LoadMw,SaveMw);break;
				case 0x06:XORW(eaa,iw,LoadMw,SaveMw);break;
				case 0x07:CMPW(eaa,iw,LoadMw,SaveMw);break;
				}
			}
			break;
		}
	CASE_W(0x83)												/* Grpl Ew,Ix */
		{
			GetRM;Bitu which=(rm>>3)&7;
			if (rm>= 0xc0) {
				GetEArw;Bit16u iw=(Bit16s)Fetchbs();
				switch (which) {
				case 0x00:ADDW(*earw,iw,LoadRw,SaveRw);break;
				case 0x01: ORW(*earw,iw,LoadRw,SaveRw);break;
				case 0x02:ADCW(*earw,iw,LoadRw,SaveRw);break;
				case 0x03:SBBW(*earw,iw,LoadRw,SaveRw);break;
				case 0x04:ANDW(*earw,iw,LoadRw,SaveRw);break;
				case 0x05:SUBW(*earw,iw,LoadRw,SaveRw);break;
				case 0x06:XORW(*earw,iw,LoadRw,SaveRw);break;
				case 0x07:CMPW(*earw,iw,LoadRw,SaveRw);break;
				}
			} else {
				GetEAa;Bit16u iw=(Bit16s)Fetchbs();
				switch (which) {
				case 0x00:ADDW(eaa,iw,LoadMw,SaveMw);break;
				case 0x01: ORW(eaa,iw,LoadMw,SaveMw);break;
				case 0x02:ADCW(eaa,iw,LoadMw,SaveMw);break;
				case 0x03:SBBW(eaa,iw,LoadMw,SaveMw);break;
				case 0x04:ANDW(eaa,iw,LoadMw,SaveMw);break;
				case 0x05:SUBW(eaa,iw,LoadMw,SaveMw);break;
				case 0x06:XORW(eaa,iw,LoadMw,SaveMw);break;
				case 0x07:CMPW(eaa,iw,LoadMw,SaveMw);break;
				}
			}
			break;
		}
	CASE_B(0x84)												/* TEST Eb,Gb */
		RMEbGb(TESTB);
		break;
	CASE_W(0x85)												/* TEST Ew,Gw */
		RMEwGw(TESTW);
		break;
	CASE_B(0x86)												/* XCHG Eb,Gb */
		{	
			GetRMrb;Bit8u oldrmrb=*rmrb;
			if (rm >= 0xc0 ) {GetEArb;*rmrb=*earb;*earb=oldrmrb;}
			else {GetEAa;*rmrb=LoadMb(eaa);SaveMb(eaa,oldrmrb);}
			break;
		}
	CASE_W(0x87)												/* XCHG Ew,Gw */
		{	
			GetRMrw;Bit16u oldrmrw=*rmrw;
			if (rm >= 0xc0 ) {GetEArw;*rmrw=*earw;*earw=oldrmrw;}
			else {GetEAa;*rmrw=LoadMw(eaa);SaveMw(eaa,oldrmrw);}
			break;
		}
	CASE_B(0x88)												/* MOV Eb,Gb */
		{	
			GetRMrb;
			if (rm >= 0xc0 ) {GetEArb;*earb=*rmrb;}
			else {GetEAa;SaveMb(eaa,*rmrb);}
			break;
		}
	CASE_W(0x89)												/* MOV Ew,Gw */
		{	
			GetRMrw;
			if (rm >= 0xc0 ) {GetEArw;*earw=*rmrw;}
			else {GetEAa;SaveMw(eaa,*rmrw);}
			break;
		}
	CASE_B(0x8a)												/* MOV Gb,Eb */
		{	
			GetRMrb;
			if (rm >= 0xc0 ) {GetEArb;*rmrb=*earb;}
			else {GetEAa;*rmrb=LoadMb(eaa);}
			break;
		}
	CASE_W(0x8b)												/* MOV Gw,Ew */
		{	
			GetRMrw;
			if (rm >= 0xc0 ) {GetEArw;*rmrw=*earw;}
			else {GetEAa;*rmrw=LoadMw(eaa);}
			break;
		}
	CASE_W(0x8c)												/* Mov Ew,Sw */
		{
			GetRM;Bit16u val;Bitu which=(rm>>3)&7;
			switch (which) {
			case 0x00:					/* MOV Ew,ES */
				val=SegValue(es);break;
			case 0x01:					/* MOV Ew,CS */
				val=SegValue(cs);break;
			case 0x02:					/* MOV Ew,SS */
				val=SegValue(ss);break;
			case 0x03:					/* MOV Ew,DS */
				val=SegValue(ds);break;
			case 0x04:					/* MOV Ew,FS */
				val=SegValue(fs);break;
			case 0x05:					/* MOV Ew,GS */
				val=SegValue(gs);break;
			default:
				val=0;
				E_Exit("CPU:8c:Illegal RM Byte");
			}
			if (rm >= 0xc0 ) {GetEArw;*earw=val;}
			else {GetEAa;SaveMw(eaa,val);}
			break;
		}
	CASE_W(0x8d)												/* LEA Gw */
		{
			//Little hack to always use segprefixed version
			core.seg_prefix_base=0;
			GetRMrw;
			if (TEST_PREFIX_ADDR) {
				*rmrw=(Bit16u)(*GetEA_SEG_ADDR[rm])();
			} else {
				*rmrw=(Bit16u)(*GetEA_SEG[rm])();
			}
			break;
		}
	CASE_B(0x8e)												/* MOV Sw,Ew */
		{
			GetRM;Bit16u val;Bitu which=(rm>>3)&7;
			if (rm >= 0xc0 ) {GetEArw;val=*earw;}
			else {GetEAa;val=LoadMw(eaa);}
			switch (which) {
			case 0x00:					/* MOV ES,Ew */
				CPU_SetSegGeneral(es,val);break;
			case 0x01:					/* MOV CS,Ew Illegal*/
				E_Exit("CPU:Illegal MOV CS Call");
				break;
			case 0x02:					/* MOV SS,Ew */
				CPU_SetSegGeneral(ss,val);
				break;
			case 0x03:					/* MOV DS,Ew */
				CPU_SetSegGeneral(ds,val);break;
			case 0x04:					/* MOV FS,Ew */
				CPU_SetSegGeneral(fs,val);break;
			case 0x05:					/* MOV GS,Ew */
				CPU_SetSegGeneral(gs,val);break;
			default:
				E_Exit("CPU:8E:Illegal RM Byte");
			}
			break;
		}							
	CASE_W(0x8f)												/* POP Ew */
		{
			GetRM;
			if (rm >= 0xc0 ) {GetEArw;*earw=Pop_16();}
			else {GetEAa;SaveMw(eaa,Pop_16());}
			break;
		}
	CASE_B(0x90)												/* NOP */
		break;
	CASE_W(0x91)												/* XCHG CX,AX */
		{ Bit16u temp=reg_ax;reg_ax=reg_cx;reg_cx=temp; }
		break;
	CASE_W(0x92)												/* XCHG DX,AX */
		{ Bit16u temp=reg_ax;reg_ax=reg_dx;reg_dx=temp; }
		break;
	CASE_W(0x93)												/* XCHG BX,AX */
		{ Bit16u temp=reg_ax;reg_ax=reg_bx;reg_bx=temp; }
		break;
	CASE_W(0x94)												/* XCHG SP,AX */
		{ Bit16u temp=reg_ax;reg_ax=reg_sp;reg_sp=temp; }
		break;
	CASE_W(0x95)												/* XCHG BP,AX */
		{ Bit16u temp=reg_ax;reg_ax=reg_bp;reg_bp=temp; }
		break;
	CASE_W(0x96)												/* XCHG SI,AX */
		{ Bit16u temp=reg_ax;reg_ax=reg_si;reg_si=temp; }
		break;
	CASE_W(0x97)												/* XCHG DI,AX */
		{ Bit16u temp=reg_ax;reg_ax=reg_di;reg_di=temp; }
		break;
	CASE_W(0x98)												/* CBW */
		reg_ax=(Bit8s)reg_al;break;
	CASE_W(0x99)												/* CWD */
		if (reg_ax & 0x8000) reg_dx=0xffff;else reg_dx=0;
		break;
	CASE_W(0x9a)												/* CALL Ap */
		{ 
			Bit16u newip=Fetchw();Bit16u newcs=Fetchw();
			SAVEIP;
			if (CPU_CALL(false,newcs,newip)) {
				LOADIP;
			} else {
				FillFlags();return CBRET_NONE;
			}
			break;
		}
	CASE_B(0x9b)												/* WAIT */
		break; /* No waiting here */
	CASE_W(0x9c)												/* PUSHF */
		FillFlags();
		Push_16(flags.word);
		break;
	CASE_W(0x9d)												/* POPF */
		SETFLAGSw(Pop_16());
#if CPU_TRAP_CHECK
		if (GETFLAG(TF)) {	
			cpudecoder=CPU_Core_Normal_Decode_Trap;
			goto decode_end;
		}
#endif
#ifdef CPU_PIC_CHECK
		if (GETFLAG(IF) && PIC_IRQCheck) goto decode_end;
#endif
		break;
	CASE_B(0x9e)												/* SAHF */
		SETFLAGSb(reg_ah);
		break;
	CASE_B(0x9f)												/* LAHF */
		FillFlags();
		reg_ah=flags.word&0xff;
		break;
	CASE_B(0xa0)												/* MOV AL,Ob */
		{
			GetEADirect;
			reg_al=LoadMb(eaa);
		}
		break;
	CASE_W(0xa1)												/* MOV AX,Ow */
		{
			GetEADirect;
			reg_ax=LoadMw(eaa);
		}
		break;
	CASE_B(0xa2)												/* MOV Ob,AL */
		{
			GetEADirect;
			SaveMb(eaa,reg_al);
		}
		break;
	CASE_W(0xa3)												/* MOV Ow,AX */
		{
			GetEADirect;
			SaveMw(eaa,reg_ax);
		}
		break;
	CASE_B(0xa4)												/* MOVSB */
		DoString(R_MOVSB);break;
	CASE_W(0xa5)												/* MOVSW */
		DoString(R_MOVSW);break;
	CASE_B(0xa6)												/* CMPSB */
		DoString(R_CMPSB);break;
	CASE_W(0xa7)												/* CMPSW */
		DoString(R_CMPSW);break;
	CASE_B(0xa8)												/* TEST AL,Ib */
		ALIb(TESTB);break;
	CASE_W(0xa9)												/* TEST AX,Iw */
		AXIw(TESTW);break;
	CASE_B(0xaa)												/* STOSB */
		DoString(R_STOSB);break;
	CASE_W(0xab)												/* STOSW */
		DoString(R_STOSW);break;
	CASE_B(0xac)												/* LODSB */
		DoString(R_LODSB);break;
	CASE_W(0xad)												/* LODSW */
		DoString(R_LODSW);break;
	CASE_B(0xae)												/* SCASB */
		DoString(R_SCASB);break;
	CASE_W(0xaf)												/* SCASW */
		DoString(R_SCASW);break;
	CASE_B(0xb0)												/* MOV AL,Ib */
		reg_al=Fetchb();break;
	CASE_B(0xb1)												/* MOV CL,Ib */
		reg_cl=Fetchb();break;
	CASE_B(0xb2)												/* MOV DL,Ib */
		reg_dl=Fetchb();break;
	CASE_B(0xb3)												/* MOV BL,Ib */
		reg_bl=Fetchb();break;
	CASE_B(0xb4)												/* MOV AH,Ib */
		reg_ah=Fetchb();break;
	CASE_B(0xb5)												/* MOV CH,Ib */
		reg_ch=Fetchb();break;
	CASE_B(0xb6)												/* MOV DH,Ib */
		reg_dh=Fetchb();break;
	CASE_B(0xb7)												/* MOV BH,Ib */
		reg_bh=Fetchb();break;
	CASE_W(0xb8)												/* MOV AX,Iw */
		reg_ax=Fetchw();break;
	CASE_W(0xb9)												/* MOV CX,Iw */
		reg_cx=Fetchw();break;
	CASE_W(0xba)												/* MOV DX,Iw */
		reg_dx=Fetchw();break;
	CASE_W(0xbb)												/* MOV BX,Iw */
		reg_bx=Fetchw();break;
	CASE_W(0xbc)												/* MOV SP,Iw */
		reg_sp=Fetchw();break;
	CASE_W(0xbd)												/* MOV BP.Iw */
		reg_bp=Fetchw();break;
	CASE_W(0xbe)												/* MOV SI,Iw */
		reg_si=Fetchw();break;
	CASE_W(0xbf)												/* MOV DI,Iw */
		reg_di=Fetchw();break;
	CASE_B(0xc0)												/* GRP2 Eb,Ib */
		GRP2B(Fetchb());break;
	CASE_W(0xc1)												/* GRP2 Ew,Ib */
		GRP2W(Fetchb());break;
	CASE_W(0xc2)												/* RETN Iw */
		{ 
			Bit16u addsp=Fetchw();
			SETIP(Pop_16());reg_esp+=addsp;
			break;  
		}
	CASE_W(0xc3)												/* RETN */
		SETIP(Pop_16());
		break;
	CASE_W(0xc4)												/* LES */
		{	
			GetRMrw;GetEAa;
			*rmrw=LoadMw(eaa);CPU_SetSegGeneral(es,LoadMw(eaa+2));
			break;
		}
	CASE_W(0xc5)												/* LDS */
		{	
			GetRMrw;GetEAa;
			*rmrw=LoadMw(eaa);CPU_SetSegGeneral(ds,LoadMw(eaa+2));
			break;
		}
	CASE_B(0xc6)												/* MOV Eb,Ib */
		{
			GetRM;
			if (rm >= 0xc0) {GetEArb;*earb=Fetchb();}
			else {GetEAa;SaveMb(eaa,Fetchb());}
			break;
		}
	CASE_W(0xc7)												/* MOV EW,Iw */
		{
			GetRM;
			if (rm >= 0xc0) {GetEArw;*earw=Fetchw();}
			else {GetEAa;SaveMw(eaa,Fetchw());}
			break;
		}
	CASE_W(0xc8)												/* ENTER Iw,Ib */
		{
			Bitu bytes=Fetchw();Bitu level=Fetchb() & 0x1f;
			Bitu frame_ptr=reg_esp-2;
			if (cpu.stack.big) {
				reg_esp-=2;
				mem_writew(SegBase(ss)+reg_esp,reg_bp);
				for (Bitu i=1;i<level;i++) {	
					reg_ebp-=2;reg_esp-=2;
					mem_writew(SegBase(ss)+reg_esp,mem_readw(SegBase(ss)+reg_ebp));
				}
				if (level) {
					reg_esp-=2;
					mem_writew(SegBase(ss)+reg_esp,(Bit16u)frame_ptr);
				}
				reg_esp-=bytes;
			} else {
				reg_sp-=2;
				mem_writew(SegBase(ss)+reg_sp,reg_bp);
				for (Bitu i=1;i<level;i++) {	
					reg_bp-=2;reg_sp-=2;
					mem_writew(SegBase(ss)+reg_sp,mem_readw(SegBase(ss)+reg_bp));
				}
				if (level) {
					reg_sp-=2;
					mem_writew(SegBase(ss)+reg_sp,(Bit16u)frame_ptr);
				}
				reg_sp-=bytes;
			}
			reg_bp=frame_ptr;
			break;
		}
	CASE_W(0xc9)												/* LEAVE */
		reg_esp&=~cpu.stack.mask;
		reg_esp|=(reg_ebp&cpu.stack.mask);
		reg_bp=Pop_16();
		break;
	CASE_W(0xca)												/* RETF Iw */
		{ 
			if (CPU_RET(false,Fetchw())) {
				LOADIP;
			} else {
				FillFlags();return CBRET_NONE;
			}
			break;
		}
	CASE_W(0xcb)												/* RETF */			
		{ 
			if (CPU_RET(false,0)) {
				LOADIP;
			} else {
				FillFlags();return CBRET_NONE;
			}
			break;
		}
	CASE_B(0xcc)												/* INT3 */
		LEAVECORE;
#if C_DEBUG	
		if (DEBUG_Breakpoint()) {
			return debugCallback;
		}
#endif			
		if (!Interrupt(3)) return CBRET_NONE;
		goto decode_start;
	CASE_B(0xcd)												/* INT Ib */	
		{
			Bit8u num=Fetchb();
			LEAVECORE;
#if C_DEBUG
			if (DEBUG_IntBreakpoint(num)) {
				return debugCallback;
			}
#endif
			if (!Interrupt(num)) return CBRET_NONE;
			goto decode_start;			//Restore IP with a LOADIP
		}
		break;
	CASE_B(0xce)												/* INTO */
		if (get_OF()) {
			LEAVECORE;
			if (!Interrupt(4)) return CBRET_NONE;
			goto decode_start;			//Restore IP with a LOADIP
		}
		break;
	CASE_W(0xcf)												/* IRET */
		{
			if (CPU_IRET(false)) {
#ifdef CPU_PIC_CHECK
				if (GETFLAG(IF) && PIC_IRQCheck) return CBRET_NONE;
#endif
#if CPU_TRAP_CHECK
				if (GETFLAG(TF)) {	
					cpudecoder=CPU_Core_Normal_Decode_Trap;
					return CBRET_NONE;
				}
#endif
			goto decode_start;
		} else {
			return CBRET_NONE;
		}
		break;
	}
	CASE_B(0xd0)												/* GRP2 Eb,1 */
		GRP2B(1);break;
	CASE_W(0xd1)												/* GRP2 Ew,1 */
		GRP2W(1);break;
	CASE_B(0xd2)												/* GRP2 Eb,CL */
		GRP2B(reg_cl);break;
	CASE_W(0xd3)												/* GRP2 Ew,CL */
		GRP2W(reg_cl);break;
	CASE_B(0xd4)												/* AAM Ib */
		AAM(Fetchb());break;
	CASE_B(0xd5)												/* AAD Ib */
		AAD(Fetchb());break;
	CASE_B(0xd6)												/* SALC */
		reg_al = get_CF() ? 0xFF : 0;
		break;
	CASE_B(0xd7)												/* XLAT */
		if (TEST_PREFIX_SEG) {
			if (TEST_PREFIX_ADDR) {
				reg_al=LoadMb(core.seg_prefix_base+(Bit32u)(reg_ebx+reg_al));
			} else {
				reg_al=LoadMb(core.seg_prefix_base+(Bit16u)(reg_bx+reg_al));
			}
		} else {
			if (TEST_PREFIX_ADDR) {
				reg_al=LoadMb(SegBase(ds)+(Bit32u)(reg_ebx+reg_al));
			} else {
				reg_al=LoadMb(SegBase(ds)+(Bit16u)(reg_bx+reg_al));
			}
		}
		break;
#ifdef CPU_FPU
	CASE_B(0xd8)												/* FPU ESC 0 */
		 FPU_ESC(0);break;
	CASE_B(0xd9)												/* FPU ESC 1 */
		 FPU_ESC(1);break;
	CASE_B(0xda)												/* FPU ESC 2 */
		 FPU_ESC(2);break;
	CASE_B(0xdb)												/* FPU ESC 3 */
		 FPU_ESC(3);break;
	CASE_B(0xdc)												/* FPU ESC 4 */
		 FPU_ESC(4);break;
	CASE_B(0xdd)												/* FPU ESC 5 */
		 FPU_ESC(5);break;
	CASE_B(0xde)												/* FPU ESC 6 */
		 FPU_ESC(6);break;
	CASE_B(0xdf)												/* FPU ESC 7 */
		 FPU_ESC(7);break;
#else 
	CASE_B(0xd8)												/* FPU ESC 0 */
	CASE_B(0xd9)												/* FPU ESC 1 */
	CASE_B(0xda)												/* FPU ESC 2 */
	CASE_B(0xdb)												/* FPU ESC 3 */
	CASE_B(0xdc)												/* FPU ESC 4 */
	CASE_B(0xdd)												/* FPU ESC 5 */
	CASE_B(0xde)												/* FPU ESC 6 */
	CASE_B(0xdf)												/* FPU ESC 7 */
		{
			LOG(LOG_CPU,LOG_NORMAL)("FPU used");
			Bit8u rm=Fetchb();
			if (rm<0xc0) GetEAa;
		}
		break;
#endif
	CASE_B(0xe0)												/* LOOPNZ */
		if (TEST_PREFIX_ADDR) {
				if ((--reg_ecx) && !get_ZF()) ADDIPFAST(Fetchbs());
				else ADDIPFAST(1);
		} else {
				if ((--reg_cx) && !get_ZF()) ADDIPFAST(Fetchbs());
				else ADDIPFAST(1);
		}
		break;
	CASE_B(0xe1)												/* LOOPZ */
		if (TEST_PREFIX_ADDR) {
				if ((--reg_ecx) && get_ZF()) ADDIPFAST(Fetchbs());
				else ADDIPFAST(1);
		} else {
				if ((--reg_cx) && get_ZF()) ADDIPFAST(Fetchbs());
				else ADDIPFAST(1);
		}
		break;
	CASE_B(0xe2)												/* LOOP */
		if (TEST_PREFIX_ADDR) {	
			if ((--reg_ecx)) ADDIPFAST(Fetchbs());
			else ADDIPFAST(1);
		} else {
			if ((--reg_cx)) ADDIPFAST(Fetchbs());
			else ADDIPFAST(1);
		}
		break;
	CASE_B(0xe3)												/* JCXZ */
		{ 
			Bitu test;
			if (TEST_PREFIX_ADDR) {
				test=reg_ecx;
			} else test=reg_cx;
			if (!test) ADDIPFAST(Fetchbs());
			else ADDIPFAST(1);
			break;
		}
	CASE_B(0xe4)												/* IN AL,Ib */
		{
			Bit16u port=Fetchb();
			reg_al=IO_Read(port);
			break;
		}
	CASE_W(0xe5)												/* IN AX,Ib */
		{ 
			Bit16u port=Fetchb();
			reg_al=IO_Read(port);
			reg_ah=IO_Read(port+1);
			break;
		}
	CASE_B(0xe6)												/* OUT Ib,AL */
		{ 
			Bit16u port=Fetchb();
			IO_Write(port,reg_al);
			break;
		}
	CASE_W(0xe7)												/* OUT Ib,AX */
		{	
			Bit16u port=Fetchb();
			IO_Write(port,reg_al);
			IO_Write(port+1,reg_ah);
			break;
		}
	CASE_W(0xe8)												/* CALL Jw */
		{ 
			Bit16s newip=Fetchws();
			Push_16((Bit16u)GETIP);
			ADDIPw(newip);
			break;
		}
	CASE_W(0xe9)												/* JMP Jw */
		ADDIPw(Fetchws());
		break;
	CASE_W(0xea)												/* JMP Ap */
		{ 
			Bit16u newip=Fetchw();
			Bit16u newcs=Fetchw();
			SAVEIP;
			if (CPU_JMP(false,newcs,newip)) {
				LOADIP;
			} else {
				FillFlags();return CBRET_NONE;
			}
			break;
		}
	CASE_B(0xeb)												/* JMP Jb */
		ADDIPFAST(Fetchbs());break;
	CASE_B(0xec)												/* IN AL,DX */
		reg_al=IO_Read(reg_dx);break;
	CASE_W(0xed)												/* IN AX,DX */
		reg_al=IO_Read(reg_dx);reg_ah=IO_Read(reg_dx+1);
		break;
	CASE_B(0xee)												/* OUT DX,AL */
		IO_Write(reg_dx,reg_al);break; 
	CASE_W(0xef)												/* OUT DX,AX */
		IO_Write(reg_dx,reg_al);IO_Write(reg_dx+1,reg_ah);break;
	CASE_B(0xf0)												/* LOCK */
		LOG(LOG_CPU,LOG_NORMAL)("CPU:LOCK");
		break;
	CASE_B(0xf2)												/* REPNZ */
		DO_PREFIX_REP(false);	
		break;		
	CASE_B(0xf3)												/* REPZ */
		DO_PREFIX_REP(true);	
		break;		
	CASE_B(0xf4)												/* HLT */
		LEAVECORE;
		CPU_HLT();
		return CBRET_NONE;
	CASE_B(0xf5)												/* CMC */
		SETFLAGBIT(CF,!get_CF());
		if (flags.type!=t_CF) flags.prev_type=flags.type;
		flags.type=t_CF;
		break;
	CASE_B(0xf6)												/* GRP3 Eb(,Ib) */
		{	
			GetRM;Bitu which=(rm>>3)&7;
			switch (which) {
			case 0x00:											/* TEST Eb,Ib */
			case 0x01:											/* TEST Eb,Ib Undocumented*/
				{
					if (rm >= 0xc0 ) {GetEArb;TESTB(*earb,Fetchb(),LoadRb,0)}
					else {GetEAa;TESTB(eaa,Fetchb(),LoadMb,0);}
					break;
				}
			case 0x02:											/* NOT Eb */
				{
					if (rm >= 0xc0 ) {GetEArb;*earb=~*earb;}
					else {GetEAa;SaveMb(eaa,~LoadMb(eaa));}
					break;
				}
			case 0x03:											/* NEG Eb */
				{
					flags.type=t_NEGb;
					if (rm >= 0xc0 ) {
						GetEArb;flags.var1.b=*earb;flags.result.b=0-flags.var1.b;
						*earb=flags.result.b;
					} else {
						GetEAa;flags.var1.b=LoadMb(eaa);flags.result.b=0-flags.var1.b;
 						SaveMb(eaa,flags.result.b);
					}
					break;
				}
			case 0x04:											/* MUL AL,Eb */
				RMEb(MULB);
				break;
			case 0x05:											/* IMUL AL,Eb */
				RMEb(IMULB);
				break;
			case 0x06:											/* DIV Eb */
				RMEb(DIVB);
				break;
			case 0x07:											/* IDIV Eb */
				RMEb(IDIVB);
				break;
			}
			break;
		}
	CASE_W(0xf7)												/* GRP3 Ew(,Iw) */
		{ 
			GetRM;Bitu which=(rm>>3)&7;
			switch (which) {
			case 0x00:											/* TEST Ew,Iw */
			case 0x01:											/* TEST Ew,Iw Undocumented*/
				{
					if (rm >= 0xc0 ) {GetEArw;TESTW(*earw,Fetchw(),LoadRw,SaveRw);}
					else {GetEAa;TESTW(eaa,Fetchw(),LoadMw,SaveMw);}
					break;
				}
			case 0x02:											/* NOT Ew */
				{
					if (rm >= 0xc0 ) {GetEArw;*earw=~*earw;}
					else {GetEAa;SaveMw(eaa,~LoadMw(eaa));}
					break;
				}
			case 0x03:											/* NEG Ew */
				{
					flags.type=t_NEGw;
					if (rm >= 0xc0 ) {
						GetEArw;flags.var1.w=*earw;flags.result.w=0-flags.var1.w;
						*earw=flags.result.w;
					} else {
						GetEAa;flags.var1.w=LoadMw(eaa);flags.result.w=0-flags.var1.w;
 						SaveMw(eaa,flags.result.w);
					}
					break;
				}
			case 0x04:											/* MUL AX,Ew */
				RMEw(MULW);
				break;
			case 0x05:											/* IMUL AX,Ew */
				RMEw(IMULW)
				break;
			case 0x06:											/* DIV Ew */
				RMEw(DIVW)
				break;
			case 0x07:											/* IDIV Ew */
				RMEw(IDIVW)
				break;
			}
			break;
		}
	CASE_B(0xf8)												/* CLC */
		SETFLAGBIT(CF,false);
		if (flags.type!=t_CF) flags.prev_type=flags.type;
		flags.type=t_CF;
		break;
	CASE_B(0xf9)												/* STC */
		SETFLAGBIT(CF,true);
		if (flags.type!=t_CF) flags.prev_type=flags.type;
		flags.type=t_CF;
		break;
	CASE_B(0xfa)												/* CLI */
		SETFLAGBIT(IF,false);
		break;
	CASE_B(0xfb)												/* STI */
		SETFLAGBIT(IF,true);
#ifdef CPU_PIC_CHECK
		if (GETFLAG(IF) && PIC_IRQCheck) goto decode_end;
#endif
		break;
	CASE_B(0xfc)												/* CLD */
		SETFLAGBIT(DF,false);
		break;
	CASE_B(0xfd)												/* STD */
		SETFLAGBIT(DF,true);
		break;
	CASE_B(0xfe)												/* GRP4 Eb */
		{
			GetRM;Bitu which=(rm>>3)&7;
			switch (which) {
			case 0x00:										/* INC Eb */
				RMEb(INCB);
				break;		
			case 0x01:										/* DEC Eb */
				RMEb(DECB);
				break;
			case 0x07:										/* CallBack */
				{
					Bitu cb=Fetchw();
					LEAVECORE;
					return cb;
				}
			default:
				E_Exit("Illegal GRP4 Call %d",(rm>>3) & 7);
				break;
			}
			break;
		}
	CASE_W(0xff)												/* GRP5 Ew */
		{
			GetRM;Bitu which=(rm>>3)&7;
			switch (which) {
			case 0x00:										/* INC Ew */
				RMEw(INCW);
				break;		
			case 0x01:										/* DEC Ew */
				RMEw(DECW);
				break;		
			case 0x02:										/* CALL Ev */
				if (rm >= 0xc0 ) {GetEArw;Push_16((Bit16u)GETIP);SETIP(*earw);}
				else {GetEAa;Push_16((Bit16u)GETIP);SETIP(LoadMw(eaa));}
				break;
			case 0x03:										/* CALL Ep */
				{
					GetEAa;
					Bit16u newip=LoadMw(eaa);
					Bit16u newcs=LoadMw(eaa+2);
					SAVEIP;
					if (CPU_CALL(false,newcs,newip)) {
						LOADIP;
					} else {
						FillFlags();return CBRET_NONE;
					}
				}
				break;
			case 0x04:										/* JMP Ev */	
				if (rm >= 0xc0 ) {GetEArw;SETIP(*earw);}
				else {GetEAa;SETIP(LoadMw(eaa));}
				break;
			case 0x05:										/* JMP Ep */	
				{
					GetEAa;
					Bit16u newip=LoadMw(eaa);
					Bit16u newcs=LoadMw(eaa+2);
					SAVEIP;
					if (CPU_JMP(false,newcs,newip)) {
						LOADIP;
					} else {
						FillFlags();return CBRET_NONE;
					}					}
				break;
			case 0x06:										/* PUSH Ev */
				if (rm >= 0xc0 ) {GetEArw;Push_16(*earw);}
				else {GetEAa;Push_16(LoadMw(eaa));}
				break;
			default:
				E_Exit("CPU:GRP5:Illegal Call %2X",which);
				break;
			}
			break;
		}
			



