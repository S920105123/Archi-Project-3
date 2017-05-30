#include "memsys.hpp"
#include <climits>
#include <algorithm>

Memory_system::Memory_system() { /* Do nothing */ }

Memory_system::Memory_system(int _pg_sz, int _mem_sz, int _cache_sz, int _blk_sz, int _assoc) {
	/* Construct memory system by given parameters */
	disk_sz=1024;
	
	/* Configurable terms */
	pg_sz=_pg_sz;
	mem_sz=_mem_sz;
	cache_sz=_cache_sz;
	blk_sz=_blk_sz;
	assoc=_assoc;
	
	/* Allocation */
	int num_pgt_entry=disk_sz/pg_sz;
	int num_tlb_entry=num_pgt_entry/4;
	int num_cache_entry=cache_sz/blk_sz/assoc;
	int num_mem_entry=mem_sz/pg_sz;
	memory.resize(num_mem_entry);
	cache.resize(num_cache_entry);
	tlb.resize(num_tlb_entry);
	pgt.resize(num_pgt_entry);
	init();
	
	/* Mask */
	tag_post=INT_MAX%num_cache_entry;
	tag_pre=~tag_post;
	pg_post=INT_MAX%num_pgt_entry;
	pg_pre=~pg_post;
}

void Memory_system::init() {
	/* Cold start */
	for (int i=0;i<memory.size();i++) {
		memory[i].valid=false;
	}
	for (int i=0;i<cache.size();i++) {
		cache[i].valid=false;
	}
	for (int i=0;i<tlb.size();i++) {
		tlb[i].valid=false;
	}
	for (int i=0;i<pgt.size();i++) {
		pgt[i].valid=false;
	}
}

/* TLB operations */

int Memory_system::tlb_find(int cycle, int vpn) {
	for (int i=0;i<tlb.size();i++) {
		Pagetable_entry &entry = tlb[i];
		if (entry.valid && entry.vpn==vpn) {
			tlb_hit++;
			entry.time=cycle;
			return entry.ppn;
		}
	}
	tlb_miss++;
	return NOT_FOUND;
}

void Memory_system::tlb_erase(int vpn) {
	for (int i=0;i<tlb.size();i++) {
		Pagetable_entry &entry = tlb[i];
		if (entry.valid && entry.vpn==vpn) {
			entry.valid=false;
			return;
		}
	}
}

void Memory_system::tlb_insert(int cycle, int vpn, int ppn) {
	int lru=INT_MAX,pos;
	for (int i=0;i<tlb.size();i++) {
		Pagetable_entry &entry = tlb[i];
		if (!entry.valid) {
			entry.valid=true;
			entry.time=cycle;
			entry.vpn=vpn;
			entry.ppn=ppn;
			return;
		}
		if (entry.time<lru) {
			lru=entry.time;
			pos=i;
		}
	}
	tlb[pos].valid=true;
	tlb[pos].time=cycle;
	tlb[pos].vpn=vpn;
	tlb[pos].ppn=ppn;
}

/* Page table operation */

int Memory_system::pgt_find(int cycle, int vpn) {
	if (pgt[vpn].valid) {
		pgt_hit++;
		pgt[vpn].time=cycle;
		return pgt[vpn].ppn;
	} else {
		pgt_mis++;
		return NOT_FOUND;
	}
}

int Memory_system::pgt_insert(int cycle, int vpn) {
	/* return the inserted ppn value.
	   Replace unused memory if any */
	for (int i=0;i<memory.size();i++) {
		if (!memory[i].valid) {
			memory[i].valid=true;
			pgt[vpn].valid=true;
			pgt[vpn].time=cycle;
			pgt[vpn].ppn=i;
			return i;
		}
	}
	
	/* Replace least recently used (LRU) ppn */
	int lru=INT_MAX,pos;
	for (int i=0;i<pgt.size();i++) {
		Pagetable_entry &entry = pgt[i];
		if (entry.valid && entry.time<lru) {
			lru=entry.time;
			pos=i;
		}
	}
	
	/* If record in pgt replaced, mark the same record in tlb as invalid */
	pgt[pos].valid=false;
	pgt[vpn].valid=true;
	pgt[vpn].ppn=pgt[pos].ppn;
	pgt[vpn].time=cycle;
	tlb_erase(pos);
}

/* Cache operations */

bool Memory_system::cache_find(int addr) {
	int num_set=cache.size()/assoc,idx,tag,num_one=0;
	bool miss=true;
	addr/=blk_sz;
	idx=addr%num_set;
	tag=addr/num_set;
	
	for (int i=0;i<assoc;i++) {
		if (cache[idx+i].valid && cache[idx+i].tag==tag) {
			cache[idx+i].used=true;
			miss=false;
			cache_hit++;
		}
		if (cache[idx+i].used) {
			num_one++;
		}
	}
	if (num_one==assoc) {
		for (int i=0;i<assoc;i++) {
			if (cache[idx+i].valid && cache[idx+i].tag!=tag) {
				cache[idx+i].used=false;
			}
		}
	}
	if (miss) {
		cache_miss++;
	}
}

bool Memory_system::cache_insert(int addr) {
	int num_set=cache.size()/assoc,idx,tag,num_one=0,pos=-1;
	bool insert=false;
	addr/=blk_sz;
	idx=addr%num_set;
	tag=addr/num_set;
	
	for (int i=0;i<assoc;i++) {
		if (!cache[idx+i].valid) {
			cache[idx+i].valid=true;
			cache[idx+i].used=true;
			cache[idx+i].tag=tag;
			insert=true;
		}
		if (pos==-1 && cache[idx+i].valid && !cache[idx+i].used) {
			pos=idx+i;
		}
		if (cache[idx+i].used) {
			num_one++;
		}
	}
	if (!insert) {
		cache[pos].used=true;
		cache[pos].tag=tag;
		num_one++;
	}
	if (num_one==assoc) {
		for (int i=0;i<assoc;i++) {
			if (cache[idx+i].valid && cache[idx+i].tag!=tag) {
				cache[idx+i].used=false;
			}
		}
	}
}

int Memory_system::get_ppn(int addr) {
	
}

void Memory_system::access(int cycle, int addr) {
	
}
