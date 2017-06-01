#ifndef MEMSYS_H
#define MEMSYS_H
#include <vector>
#include <cstdio> 
#define NOT_FOUND -1

struct Memory_entry {
	bool valid;
};

struct Pagetable_entry {
	bool valid;
	int time,vpn,ppn;
};

struct Cache_entry {
	bool valid,used;
	int tag;
};

class Memory_system {
public:
	Memory_system();
	Memory_system(char _id, int _pg_sz, int _mem_sz, int _cache_sz, int _blk_sz, int _cache_ass);
	void init();
	void access(int cycle, int addr);
	int cache_hit, cache_miss, pgt_hit, pgt_miss, tlb_hit, tlb_miss;
	
private:
	char id;
	int disk_sz;
	int pg_sz, mem_sz, cache_sz, blk_sz, assoc;
	std::vector<Pagetable_entry> pgt;
	std::vector<Pagetable_entry> tlb;
	std::vector<Memory_entry> memory;
	std::vector<Cache_entry> cache;

	/* TLB operations */
	int tlb_find(int cycle, int vpn);
	void tlb_erase(int vpn);
	void tlb_insert(int cycle, int vpn, int ppn);

	/* Page table operation */
	int pgt_find(int cycle, int vpn);
	int pgt_insert(int cycle, int vpn);

	/* Cache operations */
	bool cache_find(int addr);
	void cache_insert(int addr);
	void cache_erase(int ppn);
};

void init_memsys(int argc, char **argv);

extern Memory_system *i_memsys;
extern Memory_system *d_memsys;
extern FILE *ftrace;
const bool trace=true;

#endif
