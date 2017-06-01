#include "memsys.hpp"
#include <climits>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <iostream>

FILE *ftrace; 
Memory_system *i_memsys, *d_memsys;

Memory_system::Memory_system() { /* Do nothing */ }

Memory_system::Memory_system(char _id, int _pg_sz, int _mem_sz, int _cache_sz, int _blk_sz, int _assoc) {
	/* Construct memory system by given parameters */
	disk_sz=1024;
	id=_id;
	
	/* Configurable terms */
	pg_sz=_pg_sz;
	mem_sz=_mem_sz;
	cache_sz=_cache_sz;
	blk_sz=_blk_sz;
	assoc=_assoc;
	
	/* Allocation */
	int num_pgt_entry=disk_sz/pg_sz;
	int num_tlb_entry=num_pgt_entry/4;
	int num_cache_entry=cache_sz/blk_sz;
	int num_mem_entry=mem_sz/pg_sz;
	memory.resize(num_mem_entry);
	cache.resize(num_cache_entry);
	tlb.resize(num_tlb_entry);
	pgt.resize(num_pgt_entry);
	init();
}

void Memory_system::init() {
	/* Cold start */
	pgt_hit=pgt_miss=0;
	tlb_hit=tlb_miss=0;
	cache_hit=cache_miss=0;
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
			entry.time=cycle;
			return entry.ppn;
		}
	}
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
		if (entry.valid && entry.time<lru) {
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
	int lru=INT_MAX,pos=-1;
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
	cache_erase(pgt[pos].ppn);
	return pgt[vpn].ppn;
}

/* Cache operations */

bool Memory_system::cache_find(int addr) {
	int num_set=cache.size()/assoc,idx,tag,num_one=0;
	bool miss=true;
	addr/=blk_sz;
	idx=addr%num_set;
	tag=addr/num_set;
	
	for (int i=0;i<assoc;i++) {
		if (miss && cache[idx+i].valid && cache[idx+i].tag==tag) {
			cache[idx+i].used=true;
			miss=false;
		}
		if (cache[idx+i].valid && cache[idx+i].used) {
			num_one++;
		}
	}
	if (num_one==assoc) {
		for (int i=0;i<assoc;i++) {
			if (assoc==1 || cache[idx+i].tag!=tag) {
				cache[idx+i].used=false;
			}
		}
	}
	return !miss;
}

void Memory_system::cache_insert(int addr) {
	int num_set=cache.size()/assoc,idx,tag,num_one=0,pos=-1;
	bool insert=false;
	addr/=blk_sz;
	idx=addr%num_set;
	tag=addr/num_set;
	
	for (int i=0;i<assoc;i++) {
		if (!insert && !cache[idx+i].valid) {
			cache[idx+i].valid=true;
			cache[idx+i].used=true;
			cache[idx+i].tag=tag;
			insert=true;
		}
		if (pos==-1 && cache[idx+i].valid && !cache[idx+i].used) {
			pos=idx+i;
		}
		if (cache[idx+i].valid && cache[idx+i].used) {
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
			if (assoc==1 || cache[idx+i].tag!=tag) {
				cache[idx+i].used=false;
			}
		}
	}
}

void Memory_system::cache_erase(int ppn) {
	int num_set=cache.size()/assoc;
	for (int i=0;i<cache.size();i++) {
		int set_idx=i/assoc, addr=((cache[i].tag*num_set)|set_idx)*blk_sz;
		fprintf(stderr,"Try %d...\n",addr);
		if (cache[i].valid && addr/pg_sz==ppn) {
			fprintf(stderr,"Erase %d\n",addr);	
			cache[i].valid=false;
		}
	}
}

void Memory_system::access(int cycle, int addr) {
	int vpn,ppn;
	int where=-1;
	bool found;
	
	fprintf(stderr,"Cycle %d, %c access %d...%d\n",cycle,id,addr,pgt_hit);
	/* Table looking up */
	vpn=addr/pg_sz;
	ppn=tlb_find(cycle,vpn);
	if (ppn==NOT_FOUND) {
		tlb_miss++;
		ppn=pgt_find(cycle,vpn);
		if (ppn==NOT_FOUND) {
			pgt_miss++;
			cache_miss++;
			ppn=pgt_insert(cycle,vpn);
    		fprintf(ftrace," Disk ");
			addr=(ppn*pg_sz)|(addr%pg_sz);
			cache_insert(addr);
			tlb_insert(cycle,vpn,ppn);
			return;
		} else {
			pgt_hit++;
			where=1;
		}
		tlb_insert(cycle,vpn,ppn);
	} else {
		tlb_hit++;
		//pgt_hit++;
		where=0;
	}
	
	/* Got physical address, find in memory */
	addr=(ppn*pg_sz)|(addr%pg_sz);
	found=cache_find(addr);
	if (!found) {
		cache_miss++;
		cache_insert(addr);
	} else {
		cache_hit++;
		if (trace) fprintf(ftrace," %cCache ",id);
	}
	
	/* Print page table information */
	if (trace) {
		if (where==0) fprintf(ftrace," %cTLB ",id);
		else fprintf(ftrace," %cPageTable ",id);
	}
}

void init_memsys(int argc, char **argv) {
	int param[10];
	if (trace) ftrace=fopen("trace.rpt","w");
	if (argc!=11) {
		/* Default parameters */
		param[0] = 64;  // I memory size.
		param[1] = 32;  // D memory size.
		param[2] = 8;   // I page size.
		param[3] = 16;  // D page size.
		param[4] = 16;  // I cache size
		param[5] = 4;   // I cache block size
		param[6] = 4;   // I cache associativity
		param[7] = 16;  // D cache size
		param[8] = 4;   // D cache block size 
		param[9] = 1;   // D cache associativity
	} else {
		for (int i=0;i<10;i++) {
			param[i]=atoi(argv[i+1]);
		}
	}
	i_memsys=new Memory_system('I',param[2],param[0],param[4],param[5],param[6]);
	d_memsys=new Memory_system('D',param[3],param[1],param[7],param[8],param[9]);
}
