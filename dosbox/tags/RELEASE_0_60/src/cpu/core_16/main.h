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

restart:
	switch(Fetchb()) {
	case 0x00:												/* ADD Eb,Gb */
		RMEbGb(ADDB);break;
	case 0x01:												/* ADD Ew,Gw */
		RMEwGw(ADDW);break;	
	case 0x02:												/* ADD Gb,Eb */
		RMGbEb(ADDB);break;
	case 0x03:												/* ADD Gw,Ew */
		RMGwEw(ADDW);break;
	case 0x04:												/* ADD AL,Ib */
		ALIb(ADDB);break;
	case 0x05:												/* ADD AX,Iw */
		AXIw(ADDW);break;
	case 0x06:												/* PUSH ES */		
		Push_16(SegValue(es));break;
	case 0x07:												/* POP ES */		
		SegSet16(es,Pop_16());break;
	case 0x08:												/* OR Eb,Gb */
		RMEbGb(ORB);break;
	case 0x09:												/* OR Ew,Gw */
		RMEwGw(ORW);break;
	case 0x0a:												/* OR Gb,Eb */
		RMGbEb(ORB);break;
	case 0x0b:												/* OR Gw,Ew */
		RMGwEw(ORW);break;
	case 0x0c:												/* OR AL,Ib */
		ALIb(ORB);break;
	case 0x0d:												/* OR AX,Iw */
		AXIw(ORW);break;
	case 0x0e:												/* PUSH CS */		
		Push_16(SegValue(cs));break;
	case 0x0f:												/* 2 byte opcodes*/		
		#include "prefix_of.h"
		break;
	case 0x10:												/* ADC Eb,Gb */
		RMEbGb(ADCB);break;
	case 0x11:												/* ADC Ew,Gw */
		RMEwGw(ADCW);break;	
	case 0x12:												/* ADC Gb,Eb */
		RMGbEb(ADCB);break;
	case 0x13:												/* ADC Gw,Ew */
		RMGwEw(ADCW);break;
	case 0x14:												/* ADC AL,Ib */
		ALIb(ADCB);break;
	case 0x15:												/* ADC AX,Iw */
		AXIw(ADCW);break;
	case 0x16:												/* PUSH SS */		
		Push_16(SegValue(ss));break;
	case 0x17:												/* POP SS */		
		SegSet16(ss,Pop_16());
		CPU_Cycles++;//Be sure we run another instruction
		break;
	case 0x18:												/* SBB Eb,Gb */
		RMEbGb(SBBB);break;
	case 0x19:												/* SBB Ew,Gw */
		RMEwGw(SBBW);break;
	case 0x1a:												/* SBB Gb,Eb */
		RMGbEb(SBBB);break;
	case 0x1b:												/* SBB Gw,Ew */
		RMGwEw(SBBW);break;
	case 0x1c:												/* SBB AL,Ib */
		ALIb(SBBB);break;
	case 0x1d:												/* SBB AX,Iw */
		AXIw(SBBW);break;
	case 0x1e:												/* PUSH DS */		
		Push_16(SegValue(ds));break;
	case 0x1f:												/* POP DS */
		SegSet16(ds,Pop_16());break;
	case 0x20:												/* AND Eb,Gb */
		RMEbGb(ANDB);break;
	case 0x21:												/* AND Ew,Gw */
		RMEwGw(ANDW);break;	
	case 0x22:												/* AND Gb,Eb */
		RMGbEb(ANDB);break;
	case 0x23:												/* AND Gw,Ew */
		RMGwEw(ANDW);break;
	case 0x24:												/* AND AL,Ib */
		ALIb(ANDB);break;
	case 0x25:												/* AND AX,Iw */
		AXIw(ANDW);break;
	case 0x26:												/* SEG ES: */
		SegPrefix(es);break;
	case 0x27:												/* DAA */
		DAA();
		break;
	case 0x28:												/* SUB Eb,Gb */
		RMEbGb(SUBB);break;
	case 0x29:												/* SUB Ew,Gw */
		RMEwGw(SUBW);break;
	case 0x2a:												/* SUB Gb,Eb */
		RMGbEb(SUBB);break;
	case 0x2b:												/* SUB Gw,Ew */
		RMGwEw(SUBW);break;
	case 0x2c:												/* SUB AL,Ib */
		ALIb(SUBB);break;
	case 0x2d:												/* SUB AX,Iw */
		AXIw(SUBW);break;
	case 0x2e:												/* SEG CS: */
		SegPrefix(cs);break;
	case 0x2f:												/* DAS */
		DAS();
		break;  
	case 0x30:												/* XOR Eb,Gb */
		RMEbGb(XORB);break;
	case 0x31:												/* XOR Ew,Gw */
		RMEwGw(XORW);break;	
	case 0x32:												/* XOR Gb,Eb */
		RMGbEb(XORB);break;
	case 0x33:												/* XOR Gw,Ew */
		RMGwEw(XORW);break;
	case 0x34:												/* XOR AL,Ib */
		ALIb(XORB);break;
	case 0x35:												/* XOR AX,Iw */
		AXIw(XORW);break;
	case 0x36:												/* SEG SS: */
		SegPrefix(ss);break;
	case 0x37:												/* AAA */
		AAA();
		break;  
	case 0x38:												/* CMP Eb,Gb */
		RMEbGb(CMPB);break;
	case 0x39:												/* CMP Ew,Gw */
		RMEwGw(CMPW);break;
	case 0x3a:												/* CMP Gb,Eb */
		RMGbEb(CMPB);break;
	case 0x3b:												/* CMP Gw,Ew */
		RMGwEw(CMPW);break;
	case 0x3c:												/* CMP AL,Ib */
		ALIb(CMPB);break;
	case 0x3d:												/* CMP AX,Iw */
		AXIw(CMPW);break;
	case 0x3e:												/* SEG DS: */
		SegPrefix(ds);break;
	case 0x3f:												/* AAS */
		AAS();
		break;
	case 0x40:												/* INC AX */
		INCW(reg_ax,LoadRw,SaveRw);break;
	case 0x41:												/* INC CX */
		INCW(reg_cx,LoadRw,SaveRw);break;
	case 0x42:												/* INC DX */
		INCW(reg_dx,LoadRw,SaveRw);break;
	case 0x43:												/* INC BX */
		INCW(reg_bx,LoadRw,SaveRw);break;
	case 0x44:												/* INC SP */
		INCW(reg_sp,LoadRw,SaveRw);break;
	case 0x45:												/* INC BP */
		INCW(reg_bp,LoadRw,SaveRw);break;
	case 0x46:												/* INC SI */
		INCW(reg_si,LoadRw,SaveRw);break;
	case 0x47:												/* INC DI */
		INCW(reg_di,LoadRw,SaveRw);break;
	case 0x48:												/* DEC AX */
		DECW(reg_ax,LoadRw,SaveRw);break;
	case 0x49:												/* DEC CX */
  	DECW(reg_cx,LoadRw,SaveRw);break;
		case 0x4a:												/* DEC DX */
			DECW(reg_dx,LoadRw,SaveRw);break;
		case 0x4b:												/* DEC BX */
			DECW(reg_bx,LoadRw,SaveRw);break;
		case 0x4c:												/* DEC SP */
			DECW(reg_sp,LoadRw,SaveRw);break;
		case 0x4d:												/* DEC BP */
			DECW(reg_bp,LoadRw,SaveRw);break;
		case 0x4e:												/* DEC SI */
			DECW(reg_si,LoadRw,SaveRw);break;
		case 0x4f:												/* DEC DI */
			DECW(reg_di,LoadRw,SaveRw);break;
		case 0x50:												/* PUSH AX */
			Push_16(reg_ax);break;
		case 0x51:												/* PUSH CX */
			Push_16(reg_cx);break;
		case 0x52:												/* PUSH DX */
			Push_16(reg_dx);break;
		case 0x53:												/* PUSH BX */
			Push_16(reg_bx);break;
	case 0x54:													/* PUSH SP */
//TODO Check if this is correct i think it's SP+2 or something
			Push_16(reg_sp);break;
		case 0x55:												/* PUSH BP */
			Push_16(reg_bp);break;
		case 0x56:												/* PUSH SI */
			Push_16(reg_si);break;
		case 0x57:												/* PUSH DI */
			Push_16(reg_di);break;
		case 0x58:												/* POP AX */
			reg_ax=Pop_16();break;
		case 0x59:												/* POP CX */
			reg_cx=Pop_16();break;
		case 0x5a:												/* POP DX */
			reg_dx=Pop_16();break;
		case 0x5b:												/* POP BX */
			reg_bx=Pop_16();break;
		case 0x5c:												/* POP SP */
			reg_sp=Pop_16();break;
		case 0x5d:												/* POP BP */
			reg_bp=Pop_16();break;
		case 0x5e:												/* POP SI */
			reg_si=Pop_16();break;
		case 0x5f:												/* POP DI */
			reg_di=Pop_16();break;
		case 0x60:												/* PUSHA */
			{
				Bit16u old_sp=reg_sp;
				Push_16(reg_ax);Push_16(reg_cx);Push_16(reg_dx);Push_16(reg_bx);
				Push_16(old_sp);Push_16(reg_bp);Push_16(reg_si);Push_16(reg_di);
			}
			break;
		case 0x61:												/* POPA */
			reg_di=Pop_16();reg_si=Pop_16();reg_bp=Pop_16();Pop_16();//Don't save SP
			reg_bx=Pop_16();reg_dx=Pop_16();reg_cx=Pop_16();reg_ax=Pop_16();
			break;
		case 0x62:												/* BOUND */
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
		case 0x63:												/* ARPL */
			NOTDONE;break;
#ifdef CPU_386
		case 0x64:												/* SEG FS: */
			SegPrefix(fs);break;
		case 0x65:												/* SEG GS: */
			SegPrefix(gs);break;
		case 0x66:												/* Operand Size Prefix */
			#include "prefix_66.h"
			break;
		case 0x67:												/* Address Size Prefix */
#ifdef CPU_PREFIX_67
			core_16.prefixes^=PREFIX_ADDR;
			lookupEATable=EAPrefixTable[core_16.prefixes];
			goto restart;
#else
			NOTDONE;
#endif
			break;
#endif
		case 0x68:												/* PUSH Iw */
			Push_16(Fetchw());break;
		case 0x69:												/* IMUL Gw,Ew,Iw */
			RMGwEwOp3(DIMULW,Fetchws());
			break;
		case 0x6a:												/* PUSH Ib */
			Push_16(Fetchbs());
			break;
		case 0x6b:												/* IMUL Gw,Ew,Ib */
			RMGwEwOp3(DIMULW,Fetchbs());
			break;
		case 0x6c:												/* INSB */
			{
				stringDI;
				SaveMb(to,IO_Read(reg_dx));
				if (GETFLAG(DF)) reg_di--; else reg_di++;
				break;
			}
		case 0x6d:												/* INSW */
			{ 
				stringDI;
				SaveMb(to,IO_Read(reg_dx));
				SaveMb((to+1),IO_Read(reg_dx+1));
				if (GETFLAG(DF)) reg_di-=2; else reg_di+=2;
				break;
			}
		case 0x6e:												/* OUTSB */
			{
				stringSI;
				IO_Write(reg_dx,LoadMb(from));
				if (GETFLAG(DF)) reg_si--; else reg_si++;
				break;
			}
		case 0x6f:												/* OUTSW */
			{
				stringSI;
				IO_Write(reg_dx,LoadMb(from));
				IO_Write(reg_dx+1,LoadMb(from+1));
				if (GETFLAG(DF)) reg_si-=2; else reg_si+=2;
				break;
			}
		case 0x70:												/* JO */
			JumpSIb(get_OF());break;
		case 0x71:												/* JNO */
			JumpSIb(!get_OF());break;
		case 0x72:												/* JB */
			JumpSIb(get_CF());break;
		case 0x73:												/* JNB */
			JumpSIb(!get_CF());break;
		case 0x74:												/* JZ */
   			JumpSIb(get_ZF());break;
		case 0x75:												/* JNZ */
			JumpSIb(!get_ZF());	break;
		case 0x76:												/* JBE */
			JumpSIb(get_CF() || get_ZF());break;
		case 0x77:												/* JNBE */
			JumpSIb(!get_CF() && !get_ZF());break;
		case 0x78:												/* JS */
			JumpSIb(get_SF());break;
		case 0x79:												/* JNS */
			JumpSIb(!get_SF());break;
		case 0x7a:												/* JP */
			JumpSIb(get_PF());break;
		case 0x7b:												/* JNP */
			JumpSIb(!get_PF());break;
		case 0x7c:												/* JL */
			JumpSIb(get_SF() != get_OF());break;
		case 0x7d:												/* JNL */
			JumpSIb(get_SF() == get_OF());break;
		case 0x7e:												/* JLE */
			JumpSIb(get_ZF() || (get_SF() != get_OF()));break;
		case 0x7f:												/* JNLE */
			JumpSIb((get_SF() == get_OF()) && !get_ZF());break;
		case 0x80:												/* Grpl Eb,Ib */
		case 0x82:												/* Grpl Eb,Ib Mirror instruction*/
			{
				GetRM;
				if (rm>= 0xc0) {
					GetEArb;Bit8u ib=Fetchb();
					switch (rm & 0x38) {
					case 0x00:ADDB(*earb,ib,LoadRb,SaveRb);break;
					case 0x08: ORB(*earb,ib,LoadRb,SaveRb);break;
					case 0x10:ADCB(*earb,ib,LoadRb,SaveRb);break;
					case 0x18:SBBB(*earb,ib,LoadRb,SaveRb);break;
					case 0x20:ANDB(*earb,ib,LoadRb,SaveRb);break;
					case 0x28:SUBB(*earb,ib,LoadRb,SaveRb);break;
					case 0x30:XORB(*earb,ib,LoadRb,SaveRb);break;
					case 0x38:CMPB(*earb,ib,LoadRb,SaveRb);break;
					}
				} else {
					GetEAa;Bit8u ib=Fetchb();
					switch (rm & 0x38) {
					case 0x00:ADDB(eaa,ib,LoadMb,SaveMb);break;
					case 0x08: ORB(eaa,ib,LoadMb,SaveMb);break;
					case 0x10:ADCB(eaa,ib,LoadMb,SaveMb);break;
					case 0x18:SBBB(eaa,ib,LoadMb,SaveMb);break;
					case 0x20:ANDB(eaa,ib,LoadMb,SaveMb);break;
					case 0x28:SUBB(eaa,ib,LoadMb,SaveMb);break;
					case 0x30:XORB(eaa,ib,LoadMb,SaveMb);break;
					case 0x38:CMPB(eaa,ib,LoadMb,SaveMb);break;
					}
				}
				break;
			}
		case 0x81:												/* Grpl Ew,Iw */
			{
				GetRM;
				if (rm>= 0xc0) {
					GetEArw;Bit16u iw=Fetchw();
					switch (rm & 0x38) {
					case 0x00:ADDW(*earw,iw,LoadRw,SaveRw);break;
					case 0x08: ORW(*earw,iw,LoadRw,SaveRw);break;
					case 0x10:ADCW(*earw,iw,LoadRw,SaveRw);break;
					case 0x18:SBBW(*earw,iw,LoadRw,SaveRw);break;
					case 0x20:ANDW(*earw,iw,LoadRw,SaveRw);break;
					case 0x28:SUBW(*earw,iw,LoadRw,SaveRw);break;
					case 0x30:XORW(*earw,iw,LoadRw,SaveRw);break;
					case 0x38:CMPW(*earw,iw,LoadRw,SaveRw);break;
					}
				} else {
					GetEAa;Bit16u iw=Fetchw();
					switch (rm & 0x38) {
					case 0x00:ADDW(eaa,iw,LoadMw,SaveMw);break;
					case 0x08: ORW(eaa,iw,LoadMw,SaveMw);break;
					case 0x10:ADCW(eaa,iw,LoadMw,SaveMw);break;
					case 0x18:SBBW(eaa,iw,LoadMw,SaveMw);break;
					case 0x20:ANDW(eaa,iw,LoadMw,SaveMw);break;
					case 0x28:SUBW(eaa,iw,LoadMw,SaveMw);break;
					case 0x30:XORW(eaa,iw,LoadMw,SaveMw);break;
					case 0x38:CMPW(eaa,iw,LoadMw,SaveMw);break;
					}
				}
				break;
			}
		case 0x83:												/* Grpl Ew,Ix */
			{
				GetRM;
				if (rm>= 0xc0) {
					GetEArw;Bit16u iw=(Bit16s)Fetchbs();
					switch (rm & 0x38) {
					case 0x00:ADDW(*earw,iw,LoadRw,SaveRw);break;
					case 0x08: ORW(*earw,iw,LoadRw,SaveRw);break;
					case 0x10:ADCW(*earw,iw,LoadRw,SaveRw);break;
					case 0x18:SBBW(*earw,iw,LoadRw,SaveRw);break;
					case 0x20:ANDW(*earw,iw,LoadRw,SaveRw);break;
					case 0x28:SUBW(*earw,iw,LoadRw,SaveRw);break;
					case 0x30:XORW(*earw,iw,LoadRw,SaveRw);break;
					case 0x38:CMPW(*earw,iw,LoadRw,SaveRw);break;
					}
				} else {
					GetEAa;Bit16u iw=(Bit16s)Fetchbs();
					switch (rm & 0x38) {
					case 0x00:ADDW(eaa,iw,LoadMw,SaveMw);break;
					case 0x08: ORW(eaa,iw,LoadMw,SaveMw);break;
					case 0x10:ADCW(eaa,iw,LoadMw,SaveMw);break;
					case 0x18:SBBW(eaa,iw,LoadMw,SaveMw);break;
					case 0x20:ANDW(eaa,iw,LoadMw,SaveMw);break;
					case 0x28:SUBW(eaa,iw,LoadMw,SaveMw);break;
					case 0x30:XORW(eaa,iw,LoadMw,SaveMw);break;
					case 0x38:CMPW(eaa,iw,LoadMw,SaveMw);break;
					}
				}
				break;
			}
		case 0x84:												/* TEST Eb,Gb */
			RMEbGb(TESTB);
			break;
		case 0x85:												/* TEST Ew,Gw */
			RMEwGw(TESTW);
			break;
		case 0x86:												/* XCHG Eb,Gb */
			{	
				GetRMrb;Bit8u oldrmrb=*rmrb;
				if (rm >= 0xc0 ) {GetEArb;*rmrb=*earb;*earb=oldrmrb;}
				else {GetEAa;*rmrb=LoadMb(eaa);SaveMb(eaa,oldrmrb);}
				break;
			}
		case 0x87:												/* XCHG Ew,Gw */
			{	
				GetRMrw;Bit16u oldrmrw=*rmrw;
				if (rm >= 0xc0 ) {GetEArw;*rmrw=*earw;*earw=oldrmrw;}
				else {GetEAa;*rmrw=LoadMw(eaa);SaveMw(eaa,oldrmrw);}
				break;
			}
		case 0x88:												/* MOV Eb,Gb */
			{	
				GetRMrb;
				if (rm >= 0xc0 ) {GetEArb;*earb=*rmrb;}
				else {GetEAa;SaveMb(eaa,*rmrb);}
				break;
			}
		case 0x89:												/* MOV Ew,Gw */
			{	
				GetRMrw;
				if (rm >= 0xc0 ) {GetEArw;*earw=*rmrw;}
				else {GetEAa;SaveMw(eaa,*rmrw);}
				break;
			}
		case 0x8a:												/* MOV Gb,Eb */
			{	
				GetRMrb;
				if (rm >= 0xc0 ) {GetEArb;*rmrb=*earb;}
				else {GetEAa;*rmrb=LoadMb(eaa);}
				break;
			}
		case 0x8b:												/* MOV Gw,Ew */
			{	
				GetRMrw;
				if (rm >= 0xc0 ) {GetEArw;*rmrw=*earw;}
				else {GetEAa;*rmrw=LoadMw(eaa);}
				break;
			}
		case 0x8c:												/* Mov Ew,Sw */
			{
				GetRM;Bit16u val;
				switch (rm & 0x38) {
				case 0x00:					/* MOV Ew,ES */
					val=SegValue(es);break;
				case 0x08:					/* MOV Ew,CS */
					val=SegValue(cs);break;
				case 0x10:					/* MOV Ew,SS */
					val=SegValue(ss);break;
				case 0x18:					/* MOV Ew,DS */
					val=SegValue(ds);break;
				case 0x20:					/* MOV Ew,FS */
					val=SegValue(fs);break;
				case 0x28:					/* MOV Ew,GS */
					val=SegValue(gs);break;
				default:
					val=0;
					E_Exit("CPU:8c:Illegal RM Byte");
				}
				if (rm >= 0xc0 ) {GetEArw;*earw=val;}
				else {GetEAa;SaveMw(eaa,val);}
				break;
			}
		case 0x8d:												/* LEA */
			{
				core_16.segbase=0;
				core_16.prefixes|=PREFIX_SEG;
				lookupEATable=EAPrefixTable[core_16.prefixes];
				GetRMrw;GetEAa;
				*rmrw=(Bit16u)eaa;
				break;
			}
		case 0x8e:												/* MOV Sw,Ew */
			{
				GetRM;Bit16u val;
				if (rm >= 0xc0 ) {GetEArw;val=*earw;}
				else {GetEAa;val=LoadMw(eaa);}
				switch (rm & 0x38) {
				case 0x00:					/* MOV ES,Ew */
					SegSet16(es,val);break;
				case 0x08:					/* MOV CS,Ew Illegal*/
					E_Exit("CPU:Illegal MOV CS Call");
					break;
				case 0x10:					/* MOV SS,Ew */
					SegSet16(ss,val);
					CPU_Cycles++;//Be sure we run another instruction
					break;
				case 0x18:					/* MOV DS,Ew */
					SegSet16(ds,val);break;
				case 0x20:					/* MOV FS,Ew */
					SegSet16(fs,val);break;
				case 0x28:					/* MOV GS,Ew */
					SegSet16(gs,val);break;
				default:
					E_Exit("CPU:8e:Illegal RM Byte");
				}
				break;
			}							
		case 0x8f:												/* POP Ew */
			{
				GetRM;
				if (rm >= 0xc0 ) {GetEArw;*earw=Pop_16();}
				else {GetEAa;SaveMw(eaa,Pop_16());}
				break;
			}
		case 0x90:												/* NOP */
			break;
		case 0x91:												/* XCHG CX,AX */
			{ Bit16u temp=reg_ax;reg_ax=reg_cx;reg_cx=temp; }
			break;
		case 0x92:												/* XCHG DX,AX */
			{ Bit16u temp=reg_ax;reg_ax=reg_dx;reg_dx=temp; }
			break;
		case 0x93:												/* XCHG BX,AX */
			{ Bit16u temp=reg_ax;reg_ax=reg_bx;reg_bx=temp; }
			break;
		case 0x94:												/* XCHG SP,AX */
			{ Bit16u temp=reg_ax;reg_ax=reg_sp;reg_sp=temp; }
			break;
		case 0x95:												/* XCHG BP,AX */
			{ Bit16u temp=reg_ax;reg_ax=reg_bp;reg_bp=temp; }
			break;
		case 0x96:												/* XCHG SI,AX */
			{ Bit16u temp=reg_ax;reg_ax=reg_si;reg_si=temp; }
			break;
		case 0x97:												/* XCHG DI,AX */
			{ Bit16u temp=reg_ax;reg_ax=reg_di;reg_di=temp; }
			break;
		case 0x98:												/* CBW */
			reg_ax=(Bit8s)reg_al;break;
		case 0x99:												/* CWD */
			if (reg_ax & 0x8000) reg_dx=0xffff;
			else reg_dx=0;
			break;
		case 0x9a:												/* CALL Ap */
			{ 
				Bit16u newip=Fetchw();Bit16u newcs=Fetchw();
				Push_16(SegValue(cs));Push_16(GETIP);
				SegSet16(cs,newcs);SETIP(newip);
				break;
			}
		case 0x9b:												/* WAIT */
			break; /* No waiting here */
		case 0x9c:												/* PUSHF */
			FillFlags();
			Push_16(flags.word);
			break;
		case 0x9d:												/* POPF */
			SETFLAGSw(Pop_16());
			CheckTF();
#ifdef CPU_PIC_CHECK
			if (GETFLAG(IF) && PIC_IRQCheck) goto decode_end;
#endif
			break;
		case 0x9e:												/* SAHF */
			SETFLAGSb(reg_ah);
			break;
		case 0x9f:												/* LAHF */
			{
				FillFlags();
				reg_ah=(Bit8u)flags.word;
				break;
			}
		case 0xa0:												/* MOV AL,Ob */
			{
				reg_al=LoadMb(GetEADirect[core_16.prefixes]());
			}
			break;
		case 0xa1:												/* MOV AX,Ow */
			{
				reg_ax=LoadMw(GetEADirect[core_16.prefixes]());
			}
			break;
		case 0xa2:												/* MOV Ob,AL */
			{
				SaveMb(GetEADirect[core_16.prefixes](),reg_al);
			}
			break;
		case 0xa3:												/* MOV Ow,AX */
			{
				SaveMw(GetEADirect[core_16.prefixes](),reg_ax);
			}
			break;
		case 0xa4:												/* MOVSB */
			{	
				stringSI;stringDI;
				SaveMb(to,LoadMb(from));;
				if (GETFLAG(DF)) { reg_si--;reg_di--; }
				else {reg_si++;reg_di++;}
				break;
			}
		case 0xa5:												/* MOVSW */
			{	
				stringSI;stringDI;
				SaveMw(to,LoadMw(from));
				if (GETFLAG(DF)) { reg_si-=2;reg_di-=2; }
				else {reg_si+=2;reg_di+=2;}
				break;
			}
		case 0xa6:												/* CMPSB */
			{	
				stringSI;stringDI;
				CMPB(from,LoadMb(to),LoadMb,0);
				if (GETFLAG(DF)) { reg_si--;reg_di--; }
				else {reg_si++;reg_di++;}
				break;
			}
		case 0xa7:												/* CMPSW */
			{	
				stringSI;stringDI;
				CMPW(from,LoadMw(to),LoadMw,0);
				if (GETFLAG(DF)) { reg_si-=2;reg_di-=2; }
				else {reg_si+=2;reg_di+=2;}
				break;
			}
		case 0xa8:												/* TEST AL,Ib */
			ALIb(TESTB);break;
		case 0xa9:												/* TEST AX,Iw */
			AXIw(TESTW);break;
		case 0xaa:												/* STOSB */
			{
				stringDI;
				SaveMb(to,reg_al);
				if (GETFLAG(DF)) { reg_di--; }
				else {reg_di++;}
				break;
			}
		case 0xab:												/* STOSW */
			{
				stringDI;
				SaveMw(to,reg_ax);
				if (GETFLAG(DF)) { reg_di-=2; }
				else {reg_di+=2;}
				break;
			}
		case 0xac:												/* LODSB */
			{
				stringSI;
				reg_al=LoadMb(from);
				if (GETFLAG(DF)) { reg_si--; }
				else {reg_si++;}
				break;
			}
		case 0xad:												/* LODSW */
			{
				stringSI;
				reg_ax=LoadMw(from);
				if (GETFLAG(DF)) { reg_si-=2;}
				else {reg_si+=2;}
				break;
			}
		case 0xae:												/* SCASB */
			{
				stringDI;
				CMPB(reg_al,LoadMb(to),LoadRb,0);
				if (GETFLAG(DF)) { reg_di--; }
				else {reg_di++;}
				break;
			}
		case 0xaf:												/* SCASW */
			{
				stringDI;
				CMPW(reg_ax,LoadMw(to),LoadRw,0);
				if (GETFLAG(DF)) { reg_di-=2; }
				else {reg_di+=2;}
				break;
			}
		case 0xb0:												/* MOV AL,Ib */
			reg_al=Fetchb();break;
		case 0xb1:												/* MOV CL,Ib */
			reg_cl=Fetchb();break;
		case 0xb2:												/* MOV DL,Ib */
			reg_dl=Fetchb();break;
		case 0xb3:												/* MOV BL,Ib */
			reg_bl=Fetchb();break;
		case 0xb4:												/* MOV AH,Ib */
			reg_ah=Fetchb();break;
		case 0xb5:												/* MOV CH,Ib */
			reg_ch=Fetchb();break;
		case 0xb6:												/* MOV DH,Ib */
			reg_dh=Fetchb();break;
		case 0xb7:												/* MOV BH,Ib */
			reg_bh=Fetchb();break;
		case 0xb8:												/* MOV AX,Iw */
			reg_ax=Fetchw();break;
		case 0xb9:												/* MOV CX,Iw */
			reg_cx=Fetchw();break;
		case 0xba:												/* MOV DX,Iw */
			reg_dx=Fetchw();break;
		case 0xbb:												/* MOV BX,Iw */
			reg_bx=Fetchw();break;
		case 0xbc:												/* MOV SP,Iw */
			reg_sp=Fetchw();break;
		case 0xbd:												/* MOV BP.Iw */
			reg_bp=Fetchw();break;
		case 0xbe:												/* MOV SI,Iw */
			reg_si=Fetchw();break;
		case 0xbf:												/* MOV DI,Iw */
			reg_di=Fetchw();break;
		case 0xc0:												/* GRP2 Eb,Ib */
			GRP2B(Fetchb());break;
		case 0xc1:												/* GRP2 Ew,Ib */
			GRP2W(Fetchb());break;
		case 0xc2:												/* RETN Iw */
			{ 
				Bit16u addsp=Fetchw();
				SETIP(Pop_16());reg_sp+=addsp;
				break;  
			}
		case 0xc3:												/* RETN */
			SETIP(Pop_16());
			break;
		case 0xc4:												/* LES */
			{	
				GetRMrw;GetEAa;
				*rmrw=LoadMw(eaa);SegSet16(es,LoadMw(eaa+2));
				break;
			}
		case 0xc5:												/* LDS */
			{	
				GetRMrw;GetEAa;
				*rmrw=LoadMw(eaa);SegSet16(ds,LoadMw(eaa+2));
				break;
			}
		case 0xc6:												/* MOV Eb,Ib */
			{
				GetRM;
				if (rm>0xc0) {GetEArb;*earb=Fetchb();}
				else {GetEAa;SaveMb(eaa,Fetchb());}
				break;
			}
		case 0xc7:												/* MOV EW,Iw */
			{
				GetRM;
				if (rm>0xc0) {GetEArw;*earw=Fetchw();}
				else {GetEAa;SaveMw(eaa,Fetchw());}
				break;
			}
		case 0xc8:												/* ENTER Iw,Ib */
			{
				Bit16u bytes=Fetchw();Bit8u level=Fetchb();
				Push_16(reg_bp);reg_bp=reg_sp;reg_sp-=bytes;
				PhysPt reader=SegBase(ss)+reg_bp;
				for (Bit8u i=1;i<level;i++) {Push_16(LoadMw(reader));reader-=2;}
				if (level) Push_16(reg_bp);
				break;
			}
		case 0xc9:												/* LEAVE */
			reg_sp=reg_bp;reg_bp=Pop_16();break;
		case 0xca:												/* RETF Iw */
			{ 
				Bit16u addsp=Fetchw();
				Bit16u newip=Pop_16();Bit16u newcs=Pop_16();
				reg_sp+=addsp;SegSet16(cs,newcs);SETIP(newip);
				break;
			}
		case 0xcb:												/* RETF */			
			{
				Bit16u newip=Pop_16();Bit16u newcs=Pop_16();
				SegSet16(cs,newcs);SETIP(newip);
			}
			break;
		case 0xcc:												/* INT3 */
			LEAVECORE;
#if C_DEBUG	
			if (DEBUG_Breakpoint()) {
				return debugCallback;
			}
#endif		
			if (Interrupt(3)) {
				if (GETFLAG(TF)) {
					LOG_MSG("Switch to trap decoder");
					cpudecoder=CPU_Real_16_Slow_Decode_Trap;
					return CBRET_NONE;
				}
				goto decode_start;
			} else return CBRET_NONE;
		case 0xcd:												/* INT Ib */	
			{
				Bit8u num=Fetchb();
				LEAVECORE;
#if C_DEBUG
				if (DEBUG_IntBreakpoint(num)) {
					return debugCallback;
				}
#endif
				if (Interrupt(num)) {
					if (GETFLAG(TF)) {
						LOG_MSG("Switch to trap decoder");
						cpudecoder=CPU_Real_16_Slow_Decode_Trap;
						return CBRET_NONE;
					}
					goto decode_start;
				} else return CBRET_NONE;
			}
			break;
		case 0xce:												/* INTO */
			if (get_OF()) EXCEPTION(4);
			break;
		case 0xcf:												/* IRET */
			{
				Bit16u newip=Pop_16();Bit16u newcs=Pop_16();
				SegSet16(cs,newcs);SETIP(newip);
				Bit16u pflags=Pop_16();
				SETFLAGSw(pflags);
				CheckTF();
#ifdef CPU_PIC_CHECK
				if (GETFLAG(IF) && PIC_IRQCheck) goto decode_end;
#endif
				break;
			}
		case 0xd0:												/* GRP2 Eb,1 */
			GRP2B(1);break;
		case 0xd1:												/* GRP2 Ew,1 */
			GRP2W(1);break;
		case 0xd2:												/* GRP2 Eb,CL */
			GRP2B(reg_cl);break;
		case 0xd3:												/* GRP2 Ew,CL */
			GRP2W(reg_cl);break;
		case 0xd4:												/* AAM Ib */
			AAM(Fetchb());
			break;
		case 0xd5:												/* AAD Ib */
			AAD(Fetchb());
			break;
			
		case 0xd6:												/* SALC */
			reg_al = get_CF() ? 0xFF : 0;
			break;
		case 0xd7:												/* XLAT */
			if (core_16.prefixes & PREFIX_SEG) {
				reg_al=LoadMb(core_16.segbase+(Bit16u)(reg_bx+reg_al));
			} else {
				reg_al=LoadMb(SegBase(ds)+(Bit16u)(reg_bx+reg_al));
			}
			break;
#ifdef CPU_FPU
		case 0xd8:												/* FPU ESC 0 */
			 FPU_ESC(0);break;
		case 0xd9:												/* FPU ESC 1 */
			 FPU_ESC(1);break;
		case 0xda:												/* FPU ESC 2 */
			 FPU_ESC(2);break;
		case 0xdb:												/* FPU ESC 3 */
			 FPU_ESC(3);break;
		case 0xdc:												/* FPU ESC 4 */
			 FPU_ESC(4);break;
		case 0xdd:												/* FPU ESC 5 */
			 FPU_ESC(5);break;
		case 0xde:												/* FPU ESC 6 */
			 FPU_ESC(6);break;
		case 0xdf:												/* FPU ESC 7 */
			 FPU_ESC(7);break;
#else 
		case 0xd8:												/* FPU ESC 0 */
		case 0xd9:												/* FPU ESC 1 */
		case 0xda:												/* FPU ESC 2 */
		case 0xdb:												/* FPU ESC 3 */
		case 0xdc:												/* FPU ESC 4 */
		case 0xdd:												/* FPU ESC 5 */
		case 0xde:												/* FPU ESC 6 */
		case 0xdf:												/* FPU ESC 7 */
			{
				LOG(LOG_CPU,LOG_NORMAL)("FPU used");
				Bit8u rm=Fetchb();
				if (rm<0xc0) GetEAa;
			}
			break;
#endif
		case 0xe0:												/* LOOPNZ */
			if ((--reg_cx) && !get_ZF()) ADDIPFAST(Fetchbs());
			else ADDIPFAST(1);
			break;
		case 0xe1:												/* LOOPZ */
			if ((--reg_cx) && get_ZF()) ADDIPFAST(Fetchbs());
			else ADDIPFAST(1);
			break;
		case 0xe2:												/* LOOP */
			if ((--reg_cx)) ADDIPFAST(Fetchbs());
			else ADDIPFAST(1);
			break;
		case 0xe3:												/* JCXZ */
			{ 
				Bitu test;
				if (core_16.prefixes & PREFIX_ADDR) {
					test=reg_ecx;
				} else test=reg_cx;
				if (!test) ADDIPFAST(Fetchbs());
				else ADDIPFAST(1);
				break;
			}
		case 0xe4:												/* IN AL,Ib */
			{ Bit16u port=Fetchb();reg_al=IO_Read(port);}
			break;
		case 0xe5:												/* IN AX,Ib */
			{ Bit16u port=Fetchb();reg_al=IO_Read(port);reg_ah=IO_Read(port+1);}
			break;
		case 0xe6:												/* OUT Ib,AL */
			{ Bit16u port=Fetchb();IO_Write(port,reg_al);}
			break;
		case 0xe7:												/* OUT Ib,AX */
			{ Bit16u port=Fetchb();IO_Write(port,reg_al);IO_Write(port+1,reg_ah);}
			break;
		case 0xe8:												/* CALL Jw */
			{ 
				Bit16s newip=Fetchws();
				Push_16(GETIP);
				ADDIP(newip);
				break;
			}
		case 0xe9:												/* JMP Jw */
			ADDIP(Fetchws());
			break;
		case 0xea:												/* JMP Ap */
			{ 
				Bit16u newip=Fetchw();
				Bit16u newcs=Fetchw();
				SegSet16(cs,newcs);
				SETIP(newip);
				break;
			}
		case 0xeb:												/* JMP Jb*/
			ADDIPFAST(Fetchbs());
			break;
		case 0xec:												/* IN AL,DX */
			reg_al=IO_Read(reg_dx);
			break;
		case 0xed:												/* IN AX,DX */
			reg_al=IO_Read(reg_dx);
			reg_ah=IO_Read(reg_dx+1);
			break;
		case 0xee:												/* OUT DX,AL */
			IO_Write(reg_dx,reg_al); 
			break;
		case 0xef:												/* OUT DX,AX */
			IO_Write(reg_dx,reg_al);
			IO_Write(reg_dx+1,reg_ah);
			break;
		case 0xf0:												/* LOCK */
//			LOG(LOG_CPU,LOG_NORMAL)("CPU:LOCK");
			break;
		case 0xf1:												/* Weird call undocumented */
//			INTERRUPT(1);
			E_Exit("CPU:F1:Not Handled");
			break;
		case 0xf2:												/* REPNZ */
			Repeat_Normal(false,false);
			continue;
		case 0xf3:												/* REPZ */
			Repeat_Normal(true,false);
			continue;
		case 0xf4:												/* HLT */
			LEAVECORE;
			CPU_HLT();
			return 0x0;
		case 0xf5:												/* CMC */
			SETFLAGBIT(CF,!get_CF());
			if (flags.type!=t_CF) flags.prev_type=flags.type;
			flags.type=t_CF;
			break;
		case 0xf6:												/* GRP3 Eb(,Ib) */
		{	
			GetRM;
			switch ((rm & 0x38)>>3) {
			case 0x00:					/* TEST Eb,Ib */
			case 0x01:					/* TEST Eb,Ib Undocumented*/
				{
					if (rm >= 0xc0 ) {GetEArb;TESTB(*earb,Fetchb(),LoadRb,0)}
					else {GetEAa;TESTB(eaa,Fetchb(),LoadMb,0);}
					break;
				}
			case 0x02:					/* NOT Eb */
				{
					if (rm >= 0xc0 ) {GetEArb;*earb=~*earb;}
					else {GetEAa;SaveMb(eaa,~LoadMb(eaa));}
					break;
				}
			case 0x03:					/* NEG Eb */
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
			case 0x04:					/* MUL AL,Eb */
				RMEb(MULB);
				break;
			case 0x05:					/* IMUL AL,Eb */
				RMEb(IMULB);
				break;
			case 0x06:					/* DIV Eb */
				RMEb(DIVB);
				break;
			case 0x07:					/* IDIV Eb */
				RMEb(IDIVB);
				break;
			}
			break;
		}
		case 0xf7:												/* GRP3 Ew(,Iw) */
			{ GetRM;
			switch ((rm & 0x38)>>3) {
			case 0x00:					/* TEST Ew,Iw */
			case 0x01:					/* TEST Ew,Iw Undocumented*/
				{
					if (rm >= 0xc0 ) {GetEArw;TESTW(*earw,Fetchw(),LoadRw,SaveRw);}
					else {GetEAa;TESTW(eaa,Fetchw(),LoadMw,SaveMw);}
					break;
				}
			case 0x02:					/* NOT Ew */
				{
					if (rm >= 0xc0 ) {GetEArw;*earw=~*earw;}
					else {GetEAa;SaveMw(eaa,~LoadMw(eaa));}
					break;
				}
			case 0x03:					/* NEG Ew */
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
			case 0x04:					/* MUL AX,Ew */
				RMEw(MULW);
				break;
			case 0x05:					/* IMUL AX,Ew */
				RMEw(IMULW)
				break;
			case 0x06:					/* DIV Ew */
				RMEw(DIVW)
				break;
			case 0x07:					/* IDIV Ew */
				RMEw(IDIVW)
				break;
			}
			break;
			}
		case 0xf8:												/* CLC */
			SETFLAGBIT(CF,false);
			if (flags.type!=t_CF) flags.prev_type=flags.type;
			flags.type=t_CF;
			break;
		case 0xf9:												/* STC */
			SETFLAGBIT(CF,true);
			if (flags.type!=t_CF) flags.prev_type=flags.type;
			flags.type=t_CF;
			break;
		case 0xfa:												/* CLI */
			SETFLAGBIT(IF,false);
			break;
		case 0xfb:												/* STI */
			SETFLAGBIT(IF,true);
#ifdef CPU_PIC_CHECK
			if (GETFLAG(IF) && PIC_IRQCheck) goto decode_end;
#endif
			break;
		case 0xfc:												/* CLD */
			SETFLAGBIT(DF,false);
			break;
		case 0xfd:												/* STD */
			SETFLAGBIT(DF,true);
			break;
		case 0xfe:												/* GRP4 Eb */
			{
				GetRM;
				switch (rm & 0x38) {
				case 0x00:					/* INC Eb */
					RMEb(INCB);
					break;		
				case 0x08:					/* DEC Eb */
					RMEb(DECB);
					break;
				case 0x38:					/* CallBack */
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
		case 0xff:												/* GRP5 Ew */
			{
				GetRM;
				switch (rm & 0x38) {
				case 0x00:					/* INC Ew */
					RMEw(INCW);
					break;		
				case 0x08:					/* DEC Ew */
					RMEw(DECW);
					break;		
				case 0x10:					/* CALL Ev */
					if (rm >= 0xc0 ) {GetEArw;Push_16(GETIP);SETIP(*earw);}
					else {GetEAa;Push_16(GETIP);SETIP(LoadMw(eaa));}
					break;
				case 0x18:					/* CALL Ep */
					{
						Push_16(SegValue(cs));
						GetEAa;Push_16(GETIP);
						Bit16u newip=LoadMw(eaa);
						Bit16u newcs=LoadMw(eaa+2);
						SegSet16(cs,newcs);
						SETIP(newip);
					}
					break;
				case 0x20:					/* JMP Ev */	
					if (rm >= 0xc0 ) {GetEArw;SETIP(*earw);}
					else {GetEAa;SETIP(LoadMw(eaa));}
					break;
				case 0x28:					/* JMP Ep */	
					{
						GetEAa;
						Bit16u newip=LoadMw(eaa);
						Bit16u newcs=LoadMw(eaa+2);
						SegSet16(cs,newcs);
						SETIP(newip);
					}
					break;
				case 0x30:					/* PUSH Ev */
					if (rm >= 0xc0 ) {GetEArw;Push_16(*earw);}
					else {GetEAa;Push_16(LoadMw(eaa));}
					break;
				default:
					E_Exit("CPU:GRP5:Illegal Call %2X",rm & 0x38);
					break;
				}
				break;
		}
		default:	
			NOTDONE;
			break;
	}
	
