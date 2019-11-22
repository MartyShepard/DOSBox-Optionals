/*
 * IDE ATA/ATAPI emulation
 */

#ifndef DOSBOX_IDE_H
#define DOSBOX_IDE_H

#define MAX_IDE_CONTROLLERS 	8

extern const char *ide_names[MAX_IDE_CONTROLLERS];
extern void (*ide_inits[MAX_IDE_CONTROLLERS])(Section *);

void IDE_Auto(signed char &index,bool &slave);
void IDE_CDROM_Attach(signed char index,bool slave,unsigned char drive_index);
void IDE_CDROM_Detach(unsigned char drive_index);
void IDE_Hard_Disk_Attach(signed char index,bool slave,unsigned char bios_drive_index);
void IDE_Hard_Disk_Detach(unsigned char bios_drive_index);
void IDE_ResetDiskByBIOS(unsigned char disk);


bool ISAPNP_RegisterSysDev(const unsigned char *raw,Bitu len,bool already=false);

class ISAPnPDevice {
public:
	ISAPnPDevice();
	virtual ~ISAPnPDevice();
public:
	void checksum_ident();
public:
	virtual void config(Bitu val);
	virtual void wakecsn(Bitu val);
	virtual void select_logical_device(Bitu val);
	virtual void on_pnp_key();
	/* ISA PnP I/O data read/write */
	virtual uint8_t read(Bitu addr);
	virtual void write(Bitu addr,Bitu val);
public:
	unsigned char		CSN;
	unsigned char		logical_device;
	unsigned char		ident[9];		/* 72-bit vendor + serial + checksum identity */
	unsigned char		ident_bp;		/* bit position of identity read */
	unsigned char		ident_2nd;
	unsigned char		resource_ident;
	unsigned char*		resource_data;
	size_t			resource_data_len;
	unsigned int		resource_data_pos;
};

/* abc = ASCII letters of the alphabet
 * defg = hexadecimal digits */
#define ISAPNP_ID(a,b,c,d,e,f,g) \
	 (((a)&0x1F)    <<  2) | \
	 (((b)&0x1F)    >>  3) | \
	((((b)&0x1F)&7) << 13) | \
	 (((c)&0x1F)    <<  8) | \
	 (((d)&0x1F)    << 20) | \
	 (((e)&0x1F)    << 16) | \
	 (((f)&0x1F)    << 28) | \
	 (((g)&0x1F)    << 24)

#define ISAPNP_TYPE(a,b,c) (a),(b),(c)
#define ISAPNP_SMALL_TAG(t,l) (((t) << 3) | (l))

#define ISAPNP_SYSDEV_HEADER(id,type,ctrl) \
	( (id)        & 0xFF), \
	(((id) >>  8) & 0xFF), \
	(((id) >> 16) & 0xFF), \
	(((id) >> 24) & 0xFF), \
	type, \
	( (ctrl)       & 0xFF), \
	(((ctrl) >> 8) & 0xFF)

#define ISAPNP_IO_RANGE(info,min,max,align,len) \
	ISAPNP_SMALL_TAG(8,7), \
	(info), \
	((min) & 0xFF), (((min) >> 8) & 0xFF), \
	((max) & 0xFF), (((max) >> 8) & 0xFF), \
	(align), \
	(len)

#define ISAPNP_IRQ_SINGLE(irq,flags) \
	ISAPNP_SMALL_TAG(4,3), \
	((1 << (irq)) & 0xFF), \
	(((1 << (irq)) >> 8) & 0xFF), \
	(flags)

#define ISAPNP_IRQ(irqmask,flags) \
	ISAPNP_SMALL_TAG(4,3), \
	((irqmask) & 0xFF), \
	(((irqmask) >> 8) & 0xFF), \
	(flags)

#define ISAPNP_DMA_SINGLE(dma,flags) \
	ISAPNP_SMALL_TAG(5,2), \
	((1 << (dma)) & 0xFF), \
	(flags)

#define ISAPNP_DMA(dma,flags) \
	ISAPNP_SMALL_TAG(5,2), \
	((dma) & 0xFF), \
	(flags)

#define ISAPNP_END \
	ISAPNP_SMALL_TAG(0xF,1), 0x00

void ISA_PNP_devreg(ISAPnPDevice *x);

/* From VGA.cpp */
static inline bool is_power_of_2(Bitu val) {
	return (val != 0) && ((val&(val-1)) == 0);
	/* To explain: if val is a power of 2, then only one bit is set.
	 * Decrementing val would change that one bit to 0, and all bits to the right to 1.
	 * Example:
	 *
	 * Power of 2: val = 1024
	 *
	 *      1024 = 0000 0100 0000 0000
	 *  AND 1023 = 0000 0011 1111 1111
	 *  ------------------------------
	 *         0 = 0000 0000 0000 0000
	 *
	 * Non-power of 2: val = 713
	 *
	 *       713 = 0000 0010 1100 1001
	 *   AND 712 = 0000 0010 1100 1000
	 *  ------------------------------
	 *       712 = 0000 0010 1100 1000
	 *
	 * See how that works?
	 *
	 * For more info see https://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2*/
}

#endif
