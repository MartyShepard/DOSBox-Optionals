/*
 *  Copyright (C) 2002-2006  The DOSBox Team
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

	CASE_0F_W(0x00)												/* GRP 6 Exxx */
		{
			if ((reg_flags & FLAG_VM) || (!cpu.pmode)) goto illegal_opcode;
			GetRM;Bitu which=(rm>>3)&7;
			switch (which) {
			case 0x00:	/* SLDT */
			case 0x01:	/* STR */
				{
					Bitu saveval;
					if (!which) CPU_SLDT(saveval);
					else CPU_STR(saveval);
					if (rm >= 0xc0) {GetEArw;*earw=saveval;}
					else {GetEAa;SaveMw(eaa,saveval);}
				}
				break;
			case 0x02:case 0x03:case 0x04:case 0x05:
				{
					FillFlags();
					Bitu loadval;
					if (rm >= 0xc0 ) {GetEArw;loadval=*earw;}
					else {GetEAa;loadval=LoadMw(eaa);}
					switch (which) {
					case 0x02:
						if (cpu.cpl) EXCEPTION(EXCEPTION_GP);
						if (CPU_LLDT(loadval)) RUNEXCEPTION();
						break;
					case 0x03:
						if (cpu.cpl) EXCEPTION(EXCEPTION_GP);
						if (CPU_LTR(loadval)) RUNEXCEPTION();
						break;
					case 0x04:
						CPU_VERR(loadval);
						break;
					case 0x05:
						CPU_VERW(loadval);
						break;
					}
				}
				break;
			default:
				goto illegal_opcode;
			}
		}
		break;
	CASE_0F_W(0x01)												/* Group 7 Ew */
		{
			GetRM;Bitu which=(rm>>3)&7;
			if (rm < 0xc0)	{ //First ones all use EA
				GetEAa;Bitu limit,base;
				switch (which) {
				case 0x00:										/* SGDT */
					CPU_SGDT(limit,base);
					SaveMw(eaa,limit);
					SaveMd(eaa+2,base);
					break;
				case 0x01:										/* SIDT */
					CPU_SIDT(limit,base);
					SaveMw(eaa,limit);
					SaveMd(eaa+2,base);
					break;
				case 0x02:										/* LGDT */
					if (cpu.pmode && cpu.cpl) EXCEPTION(EXCEPTION_GP);
					CPU_LGDT(LoadMw(eaa),LoadMd(eaa+2) & 0xFFFFFF);
					break;
				case 0x03:										/* LIDT */
					if (cpu.pmode && cpu.cpl) EXCEPTION(EXCEPTION_GP);
					CPU_LIDT(LoadMw(eaa),LoadMd(eaa+2) & 0xFFFFFF);
					break;
				case 0x04:										/* SMSW */
					CPU_SMSW(limit);
					SaveMw(eaa,limit);
					break;
				case 0x06:										/* LMSW */
					limit=LoadMw(eaa);
					if (CPU_LMSW(limit)) RUNEXCEPTION();
					break;
				}
			} else {
				GetEArw;Bitu limit;
				switch (which) {
				case 0x02:										/* LGDT */
					if (cpu.pmode && cpu.cpl) EXCEPTION(EXCEPTION_GP);
					goto illegal_opcode;
				case 0x03:										/* LIDT */
					if (cpu.pmode && cpu.cpl) EXCEPTION(EXCEPTION_GP);
					goto illegal_opcode;
				case 0x04:										/* SMSW */
					CPU_SMSW(limit);
					*earw=limit;
					break;
				case 0x06:										/* LMSW */
					if (CPU_LMSW(*earw)) RUNEXCEPTION();
					break;
				default:
					goto illegal_opcode;
				}
			}
		}
		break;
	CASE_0F_W(0x02)												/* LAR Gw,Ew */
		{
			if ((reg_flags & FLAG_VM) || (!cpu.pmode)) goto illegal_opcode;
			FillFlags();
			GetRMrw;Bitu ar=*rmrw;
			if (rm >= 0xc0) {
				GetEArw;CPU_LAR(*earw,ar);
			} else {
				GetEAa;CPU_LAR(LoadMw(eaa),ar);
			}
			*rmrw=(Bit16u)ar;
		}
		break;
	CASE_0F_W(0x03)												/* LSL Gw,Ew */
		{
			if ((reg_flags & FLAG_VM) || (!cpu.pmode)) goto illegal_opcode;
			FillFlags();
			GetRMrw;Bitu limit=*rmrw;
			if (rm >= 0xc0) {
				GetEArw;CPU_LSL(*earw,limit);
			} else {
				GetEAa;CPU_LSL(LoadMw(eaa),limit);
			}
			*rmrw=(Bit16u)limit;
		}
		break;
	CASE_0F_B(0x06)												/* CLTS */
		if (cpu.pmode && cpu.cpl) EXCEPTION(EXCEPTION_GP);
		cpu.cr0&=(~CR0_TASKSWITCH);
		break;
	CASE_0F_B(0x20)												/* MOV Rd.CRx */
		{
			GetRM;
			Bitu which=(rm >> 3) & 7;
			if (rm < 0xc0 ) {
				rm |= 0xc0;
				LOG(LOG_CPU,LOG_ERROR)("MOV XXX,CR%d with non-register",which);
			}
			GetEArd;
			Bit32u crx_value;
			if (CPU_READ_CRX(which,crx_value)) RUNEXCEPTION();
			*eard=crx_value;
		}
		break;
	CASE_0F_B(0x21)												/* MOV Rd,DRx */
		{
			GetRM;
			Bitu which=(rm >> 3) & 7;
			if (rm < 0xc0 ) {
				rm |= 0xc0;
				LOG(LOG_CPU,LOG_ERROR)("MOV XXX,DR% with non-register",which);
			}
			GetEArd;
			Bit32u drx_value;
			if (CPU_READ_DRX(which,drx_value)) RUNEXCEPTION();
			*eard=drx_value;
		}
		break;
	CASE_0F_B(0x22)												/* MOV CRx,Rd */
		{
			GetRM;
			Bitu which=(rm >> 3) & 7;
			if (rm < 0xc0 ) {
				rm |= 0xc0;
				LOG(LOG_CPU,LOG_ERROR)("MOV XXX,CR% with non-register",which);
			}
			GetEArd;
			if (CPU_WRITE_CRX(which,*eard)) RUNEXCEPTION();
		}
		break;
	CASE_0F_B(0x23)												/* MOV DRx,Rd */
		{
			GetRM;
			Bitu which=(rm >> 3) & 7;
			if (rm < 0xc0 ) {
				rm |= 0xc0;
				LOG(LOG_CPU,LOG_ERROR)("MOV DR%,XXX with non-register",which);
			}
			GetEArd;
			if (CPU_WRITE_DRX(which,*eard)) RUNEXCEPTION();
		}
		break;
	CASE_0F_W(0x80)												/* JO */
		JumpCond16_w(TFLG_O);break;
	CASE_0F_W(0x81)												/* JNO */
		JumpCond16_w(TFLG_NO);break;
	CASE_0F_W(0x82)												/* JB */
		JumpCond16_w(TFLG_B);break;
	CASE_0F_W(0x83)												/* JNB */
		JumpCond16_w(TFLG_NB);break;
	CASE_0F_W(0x84)												/* JZ */
		JumpCond16_w(TFLG_Z);break;
	CASE_0F_W(0x85)												/* JNZ */
		JumpCond16_w(TFLG_NZ);break;
	CASE_0F_W(0x86)												/* JBE */
		JumpCond16_w(TFLG_BE);break;
	CASE_0F_W(0x87)												/* JNBE */
		JumpCond16_w(TFLG_NBE);break;
	CASE_0F_W(0x88)												/* JS */
		JumpCond16_w(TFLG_S);break;
	CASE_0F_W(0x89)												/* JNS */
		JumpCond16_w(TFLG_NS);break;
	CASE_0F_W(0x8a)												/* JP */
		JumpCond16_w(TFLG_P);break;
	CASE_0F_W(0x8b)												/* JNP */
		JumpCond16_w(TFLG_NP);break;
	CASE_0F_W(0x8c)												/* JL */
		JumpCond16_w(TFLG_L);break;
	CASE_0F_W(0x8d)												/* JNL */
		JumpCond16_w(TFLG_NL);break;
	CASE_0F_W(0x8e)												/* JLE */
		JumpCond16_w(TFLG_LE);break;
	CASE_0F_W(0x8f)												/* JNLE */
		JumpCond16_w(TFLG_NLE);break;
	CASE_0F_B(0x90)												/* SETO */
		SETcc(TFLG_O);break;
	CASE_0F_B(0x91)												/* SETNO */
		SETcc(TFLG_NO);break;
	CASE_0F_B(0x92)												/* SETB */
		SETcc(TFLG_B);break;
	CASE_0F_B(0x93)												/* SETNB */
		SETcc(TFLG_NB);break;
	CASE_0F_B(0x94)												/* SETZ */
		SETcc(TFLG_Z);break;
	CASE_0F_B(0x95)												/* SETNZ */
		SETcc(TFLG_NZ);	break;
	CASE_0F_B(0x96)												/* SETBE */
		SETcc(TFLG_BE);break;
	CASE_0F_B(0x97)												/* SETNBE */
		SETcc(TFLG_NBE);break;
	CASE_0F_B(0x98)												/* SETS */
		SETcc(TFLG_S);break;
	CASE_0F_B(0x99)												/* SETNS */
		SETcc(TFLG_NS);break;
	CASE_0F_B(0x9a)												/* SETP */
		SETcc(TFLG_P);break;
	CASE_0F_B(0x9b)												/* SETNP */
		SETcc(TFLG_NP);break;
	CASE_0F_B(0x9c)												/* SETL */
		SETcc(TFLG_L);break;
	CASE_0F_B(0x9d)												/* SETNL */
		SETcc(TFLG_NL);break;
	CASE_0F_B(0x9e)												/* SETLE */
		SETcc(TFLG_LE);break;
	CASE_0F_B(0x9f)												/* SETNLE */
		SETcc(TFLG_NLE);break;

	CASE_0F_W(0xa0)												/* PUSH FS */		
		Push_16(SegValue(fs));break;
	CASE_0F_W(0xa1)												/* POP FS */	
		if (CPU_PopSeg(fs,false)) RUNEXCEPTION();
		break;
	CASE_0F_B(0xa2)												/* CPUID */
		CPU_CPUID();break;
	CASE_0F_W(0xa3)												/* BT Ew,Gw */
		{
			FillFlags();GetRMrw;
			Bit16u mask=1 << (*rmrw & 15);
			if (rm >= 0xc0 ) {
				GetEArw;
				SETFLAGBIT(CF,(*earw & mask));
			} else {
				GetEAa;eaa+=(((Bit16s)*rmrw)>>4)*2;
				Bit16u old=LoadMw(eaa);
				SETFLAGBIT(CF,(old & mask));
			}
			break;
		}
	CASE_0F_W(0xa4)												/* SHLD Ew,Gw,Ib */
		RMEwGwOp3(DSHLW,Fetchb());
		break;
	CASE_0F_W(0xa5)												/* SHLD Ew,Gw,CL */
		RMEwGwOp3(DSHLW,reg_cl);
		break;
	CASE_0F_W(0xa8)												/* PUSH GS */		
		Push_16(SegValue(gs));break;
	CASE_0F_W(0xa9)												/* POP GS */		
		if (CPU_PopSeg(gs,false)) RUNEXCEPTION();
		break;
	CASE_0F_W(0xab)												/* BTS Ew,Gw */
		{
			FillFlags();GetRMrw;
			Bit16u mask=1 << (*rmrw & 15);
			if (rm >= 0xc0 ) {
				GetEArw;
				SETFLAGBIT(CF,(*earw & mask));
				*earw|=mask;
			} else {
				GetEAa;eaa+=(((Bit16s)*rmrw)>>4)*2;
				Bit16u old=LoadMw(eaa);
				SETFLAGBIT(CF,(old & mask));
				SaveMw(eaa,old | mask);
			}
			break;
		}
	CASE_0F_W(0xac)												/* SHRD Ew,Gw,Ib */
		RMEwGwOp3(DSHRW,Fetchb());
		break;
	CASE_0F_W(0xad)												/* SHRD Ew,Gw,CL */
		RMEwGwOp3(DSHRW,reg_cl);
		break;
	CASE_0F_W(0xaf)												/* IMUL Gw,Ew */
		RMGwEwOp3(DIMULW,*rmrw);
		break;
	CASE_0F_W(0xb2)												/* LSS Ew */
		{	
			GetRMrw;GetEAa;
			if (CPU_SetSegGeneral(ss,LoadMw(eaa+2))) RUNEXCEPTION();
			*rmrw=LoadMw(eaa);
			break;
		}
	CASE_0F_W(0xb3)												/* BTR Ew,Gw */
		{
			FillFlags();GetRMrw;
			Bit16u mask=1 << (*rmrw & 15);
			if (rm >= 0xc0 ) {
				GetEArw;
				SETFLAGBIT(CF,(*earw & mask));
				*earw&= ~mask;
			} else {
				GetEAa;eaa+=(((Bit16s)*rmrw)>>4)*2;
				Bit16u old=LoadMw(eaa);
				SETFLAGBIT(CF,(old & mask));
				SaveMw(eaa,old & ~mask);
			}
			break;
		}
	CASE_0F_W(0xb4)												/* LFS Ew */
		{	
			GetRMrw;GetEAa;
			if (CPU_SetSegGeneral(fs,LoadMw(eaa+2))) RUNEXCEPTION();
			*rmrw=LoadMw(eaa);
			break;
		}
	CASE_0F_W(0xb5)												/* LGS Ew */
		{	
			GetRMrw;GetEAa;
			if (CPU_SetSegGeneral(gs,LoadMw(eaa+2))) RUNEXCEPTION();
			*rmrw=LoadMw(eaa);
			break;
		}
	CASE_0F_W(0xb6)												/* MOVZX Gw,Eb */
		{
			GetRMrw;															
			if (rm >= 0xc0 ) {GetEArb;*rmrw=*earb;}
			else {GetEAa;*rmrw=LoadMb(eaa);}
			break;
		}
	CASE_0F_W(0xb7)												/* MOVZX Gw,Ew */
	CASE_0F_W(0xbf)												/* MOVSX Gw,Ew */
		{
			GetRMrw;															
			if (rm >= 0xc0 ) {GetEArw;*rmrw=*earw;}
			else {GetEAa;*rmrw=LoadMw(eaa);}
			break;
		}
	CASE_0F_W(0xba)												/* GRP8 Ew,Ib */
		{
			FillFlags();GetRM;
			if (rm >= 0xc0 ) {
				GetEArw;
				Bit16u mask=1 << (Fetchb() & 15);
				SETFLAGBIT(CF,(*earw & mask));
				switch (rm & 0x38) {
				case 0x20:										/* BT */
					break;
				case 0x28:										/* BTS */
					*earw|=mask;
					break;
				case 0x30:										/* BTR */
					*earw&= ~mask;
					break;
				case 0x38:										/* BTC */
					*earw^=mask;
					break;
				default:
					E_Exit("CPU:0F:BA:Illegal subfunction %X",rm & 0x38);
				}
			} else {
				GetEAa;Bit16u old=LoadMw(eaa);
				Bit16u mask=1 << (Fetchb() & 15);
				SETFLAGBIT(CF,(old & mask));
				switch (rm & 0x38) {
				case 0x20:										/* BT */
					break;
				case 0x28:										/* BTS */
					SaveMw(eaa,old|mask);
					break;
				case 0x30:										/* BTR */
					SaveMw(eaa,old & ~mask);
					break;
				case 0x38:										/* BTC */
					SaveMw(eaa,old ^ mask);
					break;
				default:
					E_Exit("CPU:0F:BA:Illegal subfunction %X",rm & 0x38);
				}
			}
			break;
		}
	CASE_0F_W(0xbb)												/* BTC Ew,Gw */
		{
			FillFlags();GetRMrw;
			Bit16u mask=1 << (*rmrw & 15);
			if (rm >= 0xc0 ) {
				GetEArw;
				SETFLAGBIT(CF,(*earw & mask));
				*earw^=mask;
			} else {
				GetEAa;eaa+=(((Bit16s)*rmrw)>>4)*2;
				Bit16u old=LoadMw(eaa);
				SETFLAGBIT(CF,(old & mask));
				SaveMw(eaa,old ^ mask);
			}
			break;
		}
	CASE_0F_W(0xbc)												/* BSF Gw,Ew */
		{
			GetRMrw;
			Bit16u result,value;
			if (rm >= 0xc0) { GetEArw; value=*earw; } 
			else			{ GetEAa; value=LoadMw(eaa); }
			if (value==0) {
				SETFLAGBIT(ZF,true);
			} else {
				result = 0;
				while ((value & 0x01)==0) { result++; value>>=1; }
				SETFLAGBIT(ZF,false);
				*rmrw = result;
			}
			lflags.type=t_UNKNOWN;
			break;
		}
	CASE_0F_W(0xbd)												/* BSR Gw,Ew */
		{
			GetRMrw;
			Bit16u result,value;
			if (rm >= 0xc0) { GetEArw; value=*earw; } 
			else			{ GetEAa; value=LoadMw(eaa); }
			if (value==0) {
				SETFLAGBIT(ZF,true);
			} else {
				result = 15;	// Operandsize-1
				while ((value & 0x8000)==0) { result--; value<<=1; }
				SETFLAGBIT(ZF,false);
				*rmrw = result;
			}
			lflags.type=t_UNKNOWN;
			break;
		}
	CASE_0F_W(0xbe)												/* MOVSX Gw,Eb */
		{
			GetRMrw;															
			if (rm >= 0xc0 ) {GetEArb;*rmrw=*(Bit8s *)earb;}
			else {GetEAa;*rmrw=LoadMbs(eaa);}
			break;
		}
	CASE_0F_B(0xc0)												/* XADD Gb,Eb */
		{
			GetRMrb;Bit8u oldrmrb=*rmrb;
			if (rm >= 0xc0 ) {GetEArb;*rmrb=*earb;*earb+=oldrmrb;}
			else {GetEAa;*rmrb=LoadMb(eaa);SaveMb(eaa,LoadMb(eaa)+oldrmrb);}
			break;
		}
	CASE_0F_W(0xc1)												/* XADD Gw,Ew */
		{
			GetRMrw;Bit16u oldrmrw=*rmrw;
			if (rm >= 0xc0 ) {GetEArw;*rmrw=*earw;*earw+=oldrmrw;}
			else {GetEAa;*rmrw=LoadMw(eaa);SaveMw(eaa,LoadMw(eaa)+oldrmrw);}
			break;
		}
	CASE_0F_B(0xc8)												/* BSWAP EAX */
		BSWAP(reg_eax);break;
	CASE_0F_B(0xc9)												/* BSWAP ECX */
		BSWAP(reg_ecx);break;
	CASE_0F_B(0xca)												/* BSWAP EDX */
		BSWAP(reg_edx);break;
	CASE_0F_B(0xcb)												/* BSWAP EBX */
		BSWAP(reg_ebx);break;
	CASE_0F_B(0xcc)												/* BSWAP ESP */
		BSWAP(reg_esp);break;
	CASE_0F_B(0xcd)												/* BSWAP EBP */
		BSWAP(reg_ebp);break;
	CASE_0F_B(0xce)												/* BSWAP ESI */
		BSWAP(reg_esi);break;
	CASE_0F_B(0xcf)												/* BSWAP EDI */
		BSWAP(reg_edi);break;
		
