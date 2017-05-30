#ifndef MEMSYS_H
#define MEMSYS_H
#include <vector>
#define NOT_FOUND -1

struct Memory_entry {
	bool valid;
};

struct Pagetable_entry {
	bool valid;
	int time,ppn;
};

struct Cache_entry {
	bool valid,used;
	int tag;
};

class Memory_system {
public:
	Memory_system();
	Memory_system(int _pg_sz, int _mem_sz, int _cache_sz, int _blk_sz, int _cache_ass);
	
	void access(int addr);
	
private:
	int disk_sz;
	int pg_sz, mem_sz, cache_sz, blk_sz, assoc;
	int cache_hit, cache_miss, pgt_hit, pgt_miss, tlb_hit, tlb_miss;
	int tag_pre, tag_post, pg_pre, pg_post;
	std::vector<Pagetable_entry> pgt;
	std::vector<Pagetable_entry> tlb;
	std::vector<Memory_entry> memory;
	std::vector<Cache_entry> cache;
	
	void init();
};

#endif
