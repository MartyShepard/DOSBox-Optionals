/*
 * ISA Plug'n'play emulation
 */

#ifndef DOSBOX_ISA_H
#define DOSBOX_ISA_H

bool ISAPNP_RegisterSysDev(const unsigned char *raw,Bitu len,bool already=false);

class ISAPnPDevice {
public:
	ISAPnPDevice();
	virtual ~ISAPnPDevice();
public:
	void checksum_ident();
public:
	enum SmallTags {
		PlugAndPlayVersionNumber =		0x1,
		LogicalDeviceID =			0x2,
		CompatibleDeviceID =			0x3,
		IRQFormat =				0x4,
		DMAFormat =				0x5,
		StartDependentFunctions =		0x6,
		EndDependentFunctions =			0x7,
		IOPortDescriptor =			0x8,
		FixedLocationIOPortDescriptor =		0x9,
		SmallVendorDefined =			0xE,
		EndTag =				0xF
	};
	enum LargeTags {
		MemoryRangeDescriptor =			0x1,
		IdentifierStringANSI =			0x2,
		IdentifierStringUNICODE =		0x3,
		LargeVendorDefined =			0x4,
		MemoryRange32Descriptor =		0x5,
		FixedLocationMemoryRangeDescriptor =	0x6
	};
	// StartDependentFunction config
	enum DependentFunctionConfig {
		PreferredDependentConfiguration =	0x0,
		AcceptableDependentConfiguration =	0x1,
		SubOptimalDependentConfiguration =	0x2
	};
	// IRQ format, signal types (bitfield)
	enum {
		IRQFormatInfo_HighTrueEdgeSensitive =	0x1,
		IRQFormatInfo_LowTrueEdgeSensitive =	0x2,
		IRQFormatInfo_HighTrueLevelSensitive =	0x4,
		IRQFormatInfo_LowTrueLevelSensitive =	0x8
	};
	// IRQ format, helper IRQ mask generator
	static inline const uint16_t irq2mask(const int IRQ) {
		if (IRQ < 0 || IRQ > 15) return 0;
		return (uint16_t)1 << (unsigned char)IRQ;
	}
	static inline const uint16_t irq2mask(const int a,const int b) {
		return irq2mask(a) | irq2mask(b);
	}
	static inline const uint16_t irq2mask(const int a,const int b,const int c) {
		return irq2mask(a) | irq2mask(b) | irq2mask(c);
	}
	static inline const uint16_t irq2mask(const int a,const int b,const int c,const int d) {
		return irq2mask(a) | irq2mask(b) | irq2mask(c) | irq2mask(d);
	}
	static inline const uint16_t irq2mask(const int a,const int b,const int c,const int d,const int e) {
		return irq2mask(a) | irq2mask(b) | irq2mask(c) | irq2mask(d) | irq2mask(e);
	}
	// DMA format transfer type
	enum {
		DMATransferType_8bitOnly=0,
		DMATransferType_8_and_16bit=1,
		DMATransferType_16bitOnly=2
	};
	enum {
		DMASpeedSupported_Compat=0,
		DMASpeedSupported_TypeA=1,
		DMASpeedSupported_TypeB=2,
		DMASpeedSupported_TypeF=3
	};
	// DMA format
	static inline const uint16_t dma2mask(const int DMA) {
		if (DMA < 0 || DMA > 7 || DMA == 4) return 0;
		return (uint16_t)1 << (unsigned char)DMA;
	}
	static inline const uint16_t dma2mask(const int a,const int b) {
		return dma2mask(a) | dma2mask(b);
	}
	static inline const uint16_t dma2mask(const int a,const int b,const int c) {
		return dma2mask(a) | dma2mask(b) | dma2mask(c);
	}
	static inline const uint16_t dma2mask(const int a,const int b,const int c,const int d) {
		return dma2mask(a) | dma2mask(b) | dma2mask(c) | dma2mask(d);
	}
	static inline const uint16_t dma2mask(const int a,const int b,const int c,const int d,const int e) {
		return dma2mask(a) | dma2mask(b) | dma2mask(c) | dma2mask(d) | dma2mask(e);
	}	
public:
	virtual void config(Bitu val);
	virtual void wakecsn(Bitu val);
	virtual void select_logical_device(Bitu val);
	virtual void on_pnp_key();
	/* ISA PnP I/O data read/write */
	virtual uint8_t read(Bitu addr);
	virtual void write(Bitu addr,Bitu val);
	virtual bool alloc(size_t sz);
	void write_begin_SMALLTAG(const ISAPnPDevice::SmallTags stag,unsigned char len);
	void write_begin_LARGETAG(const ISAPnPDevice::LargeTags stag,unsigned int len);
	void write_nstring(const char *str,const size_t l);
	void write_byte(const unsigned char c);
	void begin_write_res();
	void end_write_res();
	void write_END();
	void write_ISAPnP_version(unsigned char major,unsigned char minor,unsigned char vendor);
	void write_Identifier_String(const char *str);
	void write_Logical_Device_ID(const char c1,const char c2,const char c3,const char c4,const char c5,const char c6,const char c7);
	void write_Compatible_Device_ID(const char c1,const char c2,const char c3,const char c4,const char c5,const char c6,const char c7);
	void write_Device_ID(const char c1,const char c2,const char c3,const char c4,const char c5,const char c6,const char c7);
	void write_Dependent_Function_Start(const ISAPnPDevice::DependentFunctionConfig cfg,const bool force=false);
	void write_IRQ_Format(const uint16_t IRQ_mask,const unsigned char IRQ_signal_type=0);
	void write_DMA_Format(const uint8_t DMA_mask,const unsigned char transfer_type_preference,const bool is_bus_master,const bool byte_mode,const bool word_mode,const unsigned char speed_supported);
	void write_IO_Port(const uint16_t min_port,const uint16_t max_port,const uint8_t count,const uint8_t alignment=1,const bool full16bitdecode=true);
	void write_End_Dependent_Functions();
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
	size_t			alloc_write;
	unsigned char*		alloc_res;
	size_t			alloc_sz;	
	
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

#endif
