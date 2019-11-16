class CacheBlock {
public:
	void Clear(void);
	void LinkTo(Bitu index,CacheBlock * toblock) {
		assert(toblock);
		link[index].to=toblock;
		link[index].next=toblock->link[index].from;
		toblock->link[index].from=this;
	}
	struct {
		Bit16u start,end;				//Where the page is the original code
		CodePageHandler * handler;		//Page containing this code
	} page;
	struct {
		Bit8u * start;					//Where in the cache are we
		Bitu size;
		CacheBlock * next;
	} cache;
	struct {
		Bitu index;
		CacheBlock * next;
	} hash;
	struct {
		CacheBlock * to;
		CacheBlock * next;
		CacheBlock * from;
	} link[2];
	CacheBlock * crossblock;
};

class CacheBlock;
static struct {
	struct {
		CacheBlock * first;
		CacheBlock * active;
		CacheBlock * free;
		CacheBlock * running;
	} block;
	Bit8u * pos;
	CodePageHandler * free_pages;
	CodePageHandler * used_pages;
	CodePageHandler * last_page;
} cache;

#if (C_HAVE_MPROTECT)
static Bit8u cache_code_link_blocks[2][16] GCC_ATTRIBUTE(aligned(PAGESIZE));
#else
static Bit8u cache_code_link_blocks[2][16];
#endif

static CacheBlock link_blocks[2];

class CodePageHandler :public PageHandler {
public:
	CodePageHandler() {}
	void SetupAt(Bitu _phys_page,PageHandler * _old_pagehandler) {
		phys_page=_phys_page;
		old_pagehandler=_old_pagehandler;
		flags=old_pagehandler->flags|PFLAG_HASCODE;
		flags&=~PFLAG_WRITEABLE;
		active_blocks=0;
		active_count=16;
		memset(&hash_map,0,sizeof(hash_map));
		memset(&write_map,0,sizeof(write_map));
	}
	bool InvalidateRange(Bitu start,Bitu end) {
		Bits index=1+(start>>DYN_HASH_SHIFT);
		bool is_current_block=false;
		Bit32u ip_point=SegPhys(cs)+reg_eip;
		ip_point=((paging.tlb.phys_page[ip_point>>12]-phys_page)<<12)+(ip_point&0xfff);
		while (index>=0) {
			Bitu map=0;
			for (Bitu count=start;count<=end;count++) map+=write_map[count];
			if (!map) return is_current_block;
			CacheBlock * block=hash_map[index];
			while (block) {
				CacheBlock * nextblock=block->hash.next;
				if (start<=block->page.end && end>=block->page.start) {
					if (ip_point<=block->page.end && ip_point>=block->page.start) is_current_block=true;
					block->Clear();
				}
				block=nextblock;
			}
			index--;
		}
		return is_current_block;
	}
	void writeb(PhysPt addr,Bitu val){
		addr&=4095;
		host_writeb(hostmem+addr,val);
		if (!*(Bit8u*)&write_map[addr]) {
			if (active_blocks) return;
			active_count--;
			if (!active_count) Release();
		} else InvalidateRange(addr,addr);
	}
	void writew(PhysPt addr,Bitu val){
		addr&=4095;
		host_writew(hostmem+addr,val);
		if (!*(Bit16u*)&write_map[addr]) {
			if (active_blocks) return;
			active_count--;
			if (!active_count) Release();
		} else InvalidateRange(addr,addr+1);
	}
	void writed(PhysPt addr,Bitu val){
		addr&=4095;
		host_writed(hostmem+addr,val);
		if (!*(Bit32u*)&write_map[addr]) {
			if (active_blocks) return;
			active_count--;
			if (!active_count) Release();
		} else InvalidateRange(addr,addr+3);
	}
	bool writeb_checked(PhysPt addr,Bitu val) {
		addr&=4095;
		if (!*(Bit8u*)&write_map[addr]) {
			if (!active_blocks) {
				active_count--;
				if (!active_count) Release();
			}
		} else if (InvalidateRange(addr,addr)) {
			cpu.exception.which=SMC_CURRENT_BLOCK;
			return true;
		}
		host_writeb(hostmem+addr,val);
		return false;
	}
	bool writew_checked(PhysPt addr,Bitu val) {
		addr&=4095;
		if (!*(Bit16u*)&write_map[addr]) {
			if (!active_blocks) {
				active_count--;
				if (!active_count) Release();
			}
		} else if (InvalidateRange(addr,addr+1)) {
			cpu.exception.which=SMC_CURRENT_BLOCK;
			return true;
		}
		host_writew(hostmem+addr,val);
		return false;
	}
	bool writed_checked(PhysPt addr,Bitu val) {
		addr&=4095;
		if (!*(Bit32u*)&write_map[addr]) {
			if (!active_blocks) {
				active_count--;
				if (!active_count) Release();
			}
		} else if (InvalidateRange(addr,addr+3)) {
			cpu.exception.which=SMC_CURRENT_BLOCK;
			return true;
		}
		host_writed(hostmem+addr,val);
		return false;
	}
    void AddCacheBlock(CacheBlock * block) {
		Bitu index=1+(block->page.start>>DYN_HASH_SHIFT);
		block->hash.next=hash_map[index];
		block->hash.index=index;
		hash_map[index]=block;
		block->page.handler=this;
		active_blocks++;
	}
    void AddCrossBlock(CacheBlock * block) {
		block->hash.next=hash_map[0];
		block->hash.index=0;
		hash_map[0]=block;
		block->page.handler=this;
		active_blocks++;
	}
	void DelCacheBlock(CacheBlock * block) {
		active_blocks--;
		active_count=16;
		CacheBlock * * where=&hash_map[block->hash.index];
		while (*where!=block) {
			where=&((*where)->hash.next);
			//Will crash if a block isn't found, which should never happen.
		}
		*where=block->hash.next;
		for (Bitu i=block->page.start;i<=block->page.end;i++) {
			if (write_map[i]) write_map[i]--;
		}
	}
	void Release(void) {
		MEM_SetPageHandler(phys_page,1,old_pagehandler);
		PAGING_ClearTLB();
		if (prev) prev->next=next;
		else cache.used_pages=next;
		if (next) next->prev=prev;
		else cache.last_page=prev;
		next=cache.free_pages;
		cache.free_pages=this;
		prev=0;
	}
	void ClearRelease(void) {
		for (Bitu index=0;index<(1+DYN_PAGE_HASH);index++) {
			CacheBlock * block=hash_map[index];
			while (block) {
				CacheBlock * nextblock=block->hash.next;
				block->page.handler=0;			//No need, full clear
				block->Clear();
				block=nextblock;
			}
		}
		Release();
	}
	CacheBlock * FindCacheBlock(Bitu start) {
		CacheBlock * block=hash_map[1+(start>>DYN_HASH_SHIFT)];
		while (block) {
			if (block->page.start==start) return block;
			block=block->hash.next;
		}
		return 0;
	}
	HostPt GetHostReadPt(Bitu phys_page) { 
		hostmem=old_pagehandler->GetHostReadPt(phys_page);
		return hostmem;
	}
	HostPt GetHostWritePt(Bitu phys_page) { 
		return GetHostReadPt( phys_page );
	}
public:
	Bit8u write_map[4096];
	CodePageHandler * next, * prev;
private:
	PageHandler * old_pagehandler;
	CacheBlock * hash_map[1+DYN_PAGE_HASH];
	Bitu active_blocks;
	Bitu active_count;
	HostPt hostmem;	
	Bitu phys_page;
};


static CodePageHandler * MakeCodePage(Bitu lin_page) {
	mem_readb(lin_page << 12);		//Ensure page contains memory
	PageHandler * handler=paging.tlb.handler[lin_page];
	if (handler->flags & PFLAG_HASCODE) return ( CodePageHandler *)handler;
	if (handler->flags & PFLAG_NOCODE) {
		LOG_MSG("DYNX86:Can't run code in this page");
		return 0;
	} 
	Bitu phys_page=lin_page;
	if (!PAGING_MakePhysPage(phys_page)) {
		LOG_MSG("DYNX86:Can't find physpage");
		return 0;
	}
	/* Find a free CodePage */
	if (!cache.free_pages) {
		cache.used_pages->ClearRelease();
	}
	CodePageHandler * cpagehandler=cache.free_pages;
	cache.free_pages=cache.free_pages->next;
	cpagehandler->prev=cache.last_page;
	cpagehandler->next=0;
	if (cache.last_page) cache.last_page->next=cpagehandler;
	cache.last_page=cpagehandler;
	if (!cache.used_pages) cache.used_pages=cpagehandler;
	cpagehandler->SetupAt(phys_page,handler);
	MEM_SetPageHandler(phys_page,1,cpagehandler);
	PAGING_UnlinkPages(lin_page,1);
	return cpagehandler;
}

static INLINE void cache_addunsedblock(CacheBlock * block) {
	block->cache.next=cache.block.free;
	cache.block.free=block;
}

static CacheBlock * cache_getblock(void) {
	CacheBlock * ret=cache.block.free;
	if (!ret) E_Exit("Ran out of CacheBlocks" );
	cache.block.free=ret->cache.next;
	ret->cache.next=0;
	return ret;
}

void CacheBlock::Clear(void) {
	Bitu ind;
	/* Check if this is not a cross page block */
	if (hash.index) for (ind=0;ind<2;ind++) {
		CacheBlock * fromlink=link[ind].from;
		link[ind].from=0;
		while (fromlink) {
			CacheBlock * nextlink=fromlink->link[ind].next;
			fromlink->link[ind].next=0;
			fromlink->link[ind].to=&link_blocks[ind];
			fromlink=nextlink;
		}
		if (link[ind].to!=&link_blocks[ind]) {
			CacheBlock * * wherelink=&link[ind].to->link[ind].from;
			while (*wherelink != this && *wherelink) {
				wherelink = &(*wherelink)->link[ind].next;
			}
			if(*wherelink) 
				*wherelink = (*wherelink)->link[ind].next;
			else
				LOG(LOG_CPU,LOG_ERROR)("Cache anomaly. please investigate");
		}
	} else 
		cache_addunsedblock(this);
	if (crossblock) {
		crossblock->crossblock=0;
		crossblock->Clear();
		crossblock=0;
	}
	if (page.handler) {
		page.handler->DelCacheBlock(this);
		page.handler=0;
	}
}


static CacheBlock * cache_openblock(void) {
	CacheBlock * block=cache.block.active;
	/* check for enough space in this block */
	Bitu size=block->cache.size;
	CacheBlock * nextblock=block->cache.next;
	if (block->page.handler) 
		block->Clear();
	while (size<CACHE_MAXSIZE) {
		if (!nextblock) 
			goto skipresize;
		size+=nextblock->cache.size;
		CacheBlock * tempblock=nextblock->cache.next;
		if (nextblock->page.handler) 
			nextblock->Clear();
		cache_addunsedblock(nextblock);
		nextblock=tempblock;
	}
skipresize:
	block->cache.size=size;
	block->cache.next=nextblock;
	cache.pos=block->cache.start;
	return block;
}

static void cache_closeblock(void) {
	CacheBlock * block=cache.block.active;
	block->link[0].to=&link_blocks[0];
	block->link[1].to=&link_blocks[1];
	block->link[0].from=0;
	block->link[1].from=0;
	block->link[0].next=0;
	block->link[1].next=0;
	/* Close the block with correct alignments */
	Bitu written=cache.pos-block->cache.start;
	if (written>block->cache.size) {
		if (!block->cache.next) {
			if (written>block->cache.size+CACHE_MAXSIZE) E_Exit("CacheBlock overrun 1 %d",written-block->cache.size);	
		} else E_Exit("CacheBlock overrun 2 written %d size %d",written,block->cache.size);	
	} else {
		Bitu new_size;
		Bitu left=block->cache.size-written;
		/* Smaller than cache align then don't bother to resize */
		if (left>CACHE_ALIGN) {
			new_size=((written-1)|(CACHE_ALIGN-1))+1;
			CacheBlock * newblock=cache_getblock();
			newblock->cache.start=block->cache.start+new_size;
			newblock->cache.size=block->cache.size-new_size;
			newblock->cache.next=block->cache.next;
			block->cache.next=newblock;
			block->cache.size=new_size;
		}
	}
	/* Advance the active block pointer */
	if (!block->cache.next) {
//		LOG_MSG("Cache full restarting");
		cache.block.active=cache.block.first;
	} else {
		cache.block.active=block->cache.next;
	}
}

static INLINE void cache_addb(Bit8u val) {
	*cache.pos++=val;
}

static INLINE void cache_addw(Bit16u val) {
	*(Bit16u*)cache.pos=val;
	cache.pos+=2;
}

static INLINE void cache_addd(Bit32u val) {
	*(Bit32u*)cache.pos=val;
	cache.pos+=4;
}


static void gen_return(BlockReturn retcode);

static Bit8u * cache_code=NULL;
static CacheBlock * cache_blocks=NULL;

/* Define temporary pagesize so the MPROTECT case and the regular case share as much code as possible */
#if (C_HAVE_MPROTECT)
#define PAGESIZE_TEMP PAGESIZE
#else 
#define PAGESIZE_TEMP 1
#endif


static void cache_init(bool enable) {
	static bool cache_initialized = false;
	Bits i;
	if (enable) {
		if (cache_initialized) return;
		cache_initialized = true;
		if (cache_blocks == NULL) {
			cache_blocks=(CacheBlock*)malloc(CACHE_BLOCKS*sizeof(CacheBlock));
			if(!cache_blocks) E_Exit("Allocating cache_blocks has failed");
			memset(cache_blocks,0,sizeof(CacheBlock)*CACHE_BLOCKS);
			cache.block.free=&cache_blocks[0];
			for (i=0;i<CACHE_BLOCKS-1;i++) {
				cache_blocks[i].link[0].to=(CacheBlock *)1;
				cache_blocks[i].link[1].to=(CacheBlock *)1;
				cache_blocks[i].cache.next=&cache_blocks[i+1];
			}
		}
#if (C_HAVE_MPROTECT)
		if(mprotect(cache_code_link_blocks,sizeof(cache_code_link_blocks),PROT_WRITE|PROT_READ|PROT_EXEC))
			LOG_MSG("Setting excute permission on cache code link blocks has failed");
#endif
		if (cache_code==NULL) {
			cache_code=(Bit8u*)malloc(CACHE_TOTAL+CACHE_MAXSIZE+PAGESIZE_TEMP-1);
			if(!cache_code) E_Exit("Allocating dynamic cache failed");
#if (C_HAVE_MPROTECT)
			cache_code=(Bit8u*)(((int)cache_code + PAGESIZE-1) & ~(PAGESIZE-1)); //MEM LEAK. store old pointer if you want to free it.
			if(mprotect(cache_code,CACHE_TOTAL+CACHE_MAXSIZE,PROT_WRITE|PROT_READ|PROT_EXEC))
				LOG_MSG("Setting excute permission on the code cache has failed!");
#endif
			CacheBlock * block=cache_getblock();
			cache.block.first=block;
			cache.block.active=block;
			block->cache.start=&cache_code[0];
			block->cache.size=CACHE_TOTAL;
			block->cache.next=0;								//Last block in the list
		}
		/* Setup the default blocks for block linkage returns */
		cache.pos=&cache_code_link_blocks[0][0];
		link_blocks[0].cache.start=cache.pos;
		gen_return(BR_Link1);
		cache.pos=&cache_code_link_blocks[1][0];
		link_blocks[1].cache.start=cache.pos;
		gen_return(BR_Link2);
		cache.free_pages=0;
		cache.last_page=0;
		cache.used_pages=0;
		/* Setup the code pages */
		for (i=0;i<CACHE_PAGES-1;i++) {
			CodePageHandler * newpage=new CodePageHandler();
			newpage->next=cache.free_pages;
			cache.free_pages=newpage;
		}
	}
}
