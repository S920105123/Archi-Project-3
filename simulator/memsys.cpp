#include "memsys.hpp"
#include <climits>

Memory_system::Memory_system() { /* Do nothing */ }

Memory_system::Memory_system(int _pg_sz, int _mem_sz, int _cache_sz, int _blk_sz, int _cache_ass) {
	/* Construct memory system by given parameters */
	disk_sz=1024;
	
	/* Configurable terms */
	pg_sz=_pg_sz;
	mem_sz=_mem_sz;
	cache_sz=_cache_sz;
	blk_sz=_blk_sz;
	cache_ass=_cache_ass;
	
	/* Allocation */
	int num_pgt_entry=disk_sz/pg_sz;
	int num_tlb_entry=num_pgt_entry/4;
	int num_cache_entry=cache_sz/blk_sz/cache_ass;
	int num_mem_entry=mem_sz/pg_sz;
	memory=std::vector<Memory_entry>(num_mem_entry);
	cache=std::vector<Cache_entry>(num_cache_entry);
	tlb=std::vector<Pagetable_entry>(num_tlb_entry);
	pgt=std::vector<int>(num_pgt_entry);
	init();
	
	/* Mask */
	tag_post=INT_MAX%num_cache_entry;
	tag_pre=~tag_post;
	pg_post=INT_MAX%num_pgt_entry;
	pg_pre=~pg_post;
}

void Memory_system::init() {
	/* Cold start */
	for (auto entry : memory) {
		entry.valid=false;
	}
	for (auto entry : cache) {
		entry.valid=false;
	}
	for (auto entry : tlb) {
		entry.valid=false;
	}
	for (auto entry : pgt) {
		entry.valid=false;
	}
}

void Memory_system::access(int addr) {
	
}
