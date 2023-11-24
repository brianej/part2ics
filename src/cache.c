#include "cache.h"
#include <stdlib.h>
#include <math.h>

// Cache statistics counters.
uint32_t cache_total_accesses;
uint32_t cache_hits;
uint32_t cache_misses;
uint32_t cache_read_accesses;
uint32_t cache_read_hits;
uint32_t cache_write_accesses;
uint32_t cache_write_hits;

// Input parameters to control the cache.
uint32_t cache_size;
uint32_t cache_associativity;
uint32_t cache_block_size;

block_t **cache;
uint32_t set;
uint32_t set_size;

/*
 * Perform a read from the memory for a particular address.
 * Since this is a cache-simulation, memory is not involved.
 * No need to implement the logic for this function.
 * Call this function when cache needs to read from memory.
 */
int read_from_memory(uint32_t pa) 
{
	return 0;
}

/*
 * Perform a write from the cache to memory for a particular address.
 * Since this is a cache-simulation, memory is not involved.
 * No need to implement the logic for this function.
 * Call this function when cache needs to write to memory.
 */
int write_to_memory(uint32_t pa)
{
	return 0;
}

/*
 *********************************************************
  Please edit the below functions for Part 1. 
  You are allowed to add more functions that may help you
  with your implementation. However, please do not add
  them in any file. All your new code should be in cache.c
  file below this line. 
 *********************************************************
*/

// Gets the tag of the address
uint32_t tag_getter(uint32_t pa, uint32_t associativity){
	// The offset in bits
	uint32_t offset = log2(cache_block_size);

	// Right shift till only tag left
	uint32_t tag = pa >> offset;

	// When cache has an index
	if (associativity != 2){
		// The index in bits
		uint32_t idx = log2(set);

		// Right shift till only tag left
		tag = tag >> idx;
	}

	return tag;
}

// Gets the set where to look in
uint32_t index_getter(uint32_t pa, uint32_t set){

	// The offset in bits
	uint32_t offset = log2(cache_block_size);

	// The index in bits
	uint32_t idx = log2(set);

	// Amount of bits in the address
	uint32_t amountbit = 20;

	// Right shift till only tag left
	uint32_t tag = pa >> offset;

	// Gets the index from the tag
	uint32_t index  = tag << (amountbit - idx - offset);

	// Removing the zeroes 
	index = index / pow(10,(amountbit - idx));

	// Gets the index depending on set
	index = index % set;

	return index;
}

// LRU
void LRU(block_t* Set[],uint32_t newTag)
{
	// free last value on the list
	free(Set[(set_size-1)]);
	// assign last value the new tag
	Set[set_size-1].tag = newTag;
	Set[set_size-1].valid = 1;

}

// recently used 
void recently_used(block_t* Set[],int index)
{
	block_t temp = Set[index];

	for(int i = index; i > 0;i--){
		Set[i]= Set[i-1];
	}
	Set[0] = temp;

}

/*
 * Initialize the cache depending on the input parameters S, A, and B 
 * and the statistics counter. The cache is declared in as extern in 
 * include/cache.h file.
 */
void initialize_cache()
{
	uint32_t line = cache_size / cache_block_size;

	// Sets how many sets on associativity
	if (cache_associativity == 3){
		set_size = 2;
	} else if(cache_associativity == 4){
		set_size = 4;
	} else if(cache_associativity == 1){
		set_size = 1;
	} else{
		set_size = line;
	}

	set = line / set_size;

	cache = malloc(set * sizeof(block_t*));

	// Allocating lines for each set
	for (int i = 0; i < set; i++){
        *(cache + i) = malloc(set_size * sizeof(block_t));
    }

	// Initialising each variable of block in every set
	for (int i = 0; i < set; i++){
		for (int j = 0; j < set_size; j++){
			cache[i][j].valid = 0;
			cache[i][j].dirty = 0;
			cache[i][j].tag = 0;
		}
	}

	cache_total_accesses = 0;
	cache_hits = 0;
	cache_misses = 0;
	cache_read_accesses = 0;
	cache_read_hits = 0;
	cache_write_accesses = 0;
	cache_write_hits = 0;
}

/*
 * Free the allocated memory for the cache to avoid memory leaks.
 */
void free_cache()
{
	uint32_t line = cache_size / cache_block_size;

	// Frees each of the double pointer
	for (int i = 0; i < line; i++) {
        free(cache[i]);
    }

	// Clears the cache
	free(cache);

    cache = NULL;
}

// Print cache statistics.
void print_cache_statistics()
{
	printf("\n* Cache Statistics *\n");
	printf("total accesses: %d\n", cache_total_accesses);
	printf("hits: %d\n", cache_hits);
	printf("misses: %d\n", cache_misses);
	printf("total reads: %d\n", cache_read_accesses);
	printf("read hits: %d\n", cache_read_hits);
	printf("total writes: %d\n", cache_write_accesses);
	printf("write hits: %d\n", cache_write_hits);
}

/*
 * Perform a read from the cache for a particular address.
 * Since this is a simulation, there is no data. Ignore the data part.
 * The return value is always a HIT or a MISS and never an ERROR.
 */
op_result_t read_from_cache(uint32_t pa) 
{
	cache_total_accesses++;
	cache_read_accesses++;

	// If it needs the loop
	int loop = 0;
	// Which set to look in
	uint32_t inset;

	uint32_t tag = tag_getter(pa,cache_associativity);

	if (cache_associativity == 1){
		inset =  index_getter(pa,set);
	// Fully associative
	} else if (cache_associativity == 2){
		loop = 1;
		inset = 0;
	} else if (cache_associativity == 3){
		loop = 1;
		inset =  index_getter(pa,set);
	} else if (cache_associativity == 4){
		loop = 1;
		inset =  index_getter(pa,set);
	}
	
	// Loops through the needed set to search for empty line for associativity > 1
	if (loop){
		for (uint32_t i = 0; i < set_size; i++){
			if (cache[inset][i].tag == tag){
				if (cache[inset][i].valid == 1){
					cache_hits++;
					cache_read_hits++;
					dummy_read_page_from_disk(char *page_data, uint32_t disk_block);
					return HIT;
				}else {
					for (uint32_t j = 0; j < set_size; j++){
						if (cache[inset][i].valid == 0){
							cache[inset][j].valid = 1;
							cache[inset][j].tag = tag;
						// When theres no more empty space, use lru
						} else if ((j + 1) == set_size){
							
						}
					}
					cache_misses++;
					return MISS;
				}
			// Already at the end of the cache and everything is already full
			} else if ((i + 1) == set_size){
				for (uint32_t j = 0; j < set_size; j++){
						if (cache[inset][i].valid == 0){
							cache[inset][j].valid = 1;
							cache[inset][j].tag = tag;
						// When theres no more empty space, use lru  
						} else if ((j + 1) == set_size){
						
						}
					}
				cache_misses++;
				return MISS;
			}
		}
	// For associativity == 1
	} else {
		if ((cache[inset][0].tag == tag) && (cache[inset][0].valid == 1)){
			cache_hits++;
			cache_read_hits++;
			dummy_read_page_from_disk(char *page_data, uint32_t disk_block);
			return HIT;	
		}else {
			cache[inset][0].tag = tag;
			cache[inset][0].valid = 1;
			return MISS;
		}
	}
}

/*
 * Perform a write from the cache for a particular address.
 * Since this is a simulation, there is no data. Ignore the data part.
 * The return value is always a HIT or a MISS and never an ERROR.
 */
op_result_t write_to_cache(uint32_t pa)
{
	cache_total_accesses++;
	cache_write_accesses++;

	// If it needs the loop
	int loop = 0;
	// Which set to look in
	uint32_t inset;

	uint32_t tag = tag_getter(pa,cache_associativity);

	if (cache_associativity == 1){
		inset =  index_getter(pa,set);
	// Fully associative
	} else if (cache_associativity == 2){
		loop = 1;
		inset = 0;
	} else if (cache_associativity == 3){
		loop = 1;
		inset =  index_getter(pa,set);
	} else if (cache_associativity == 4){
		loop = 1;
		inset =  index_getter(pa,set);
	}
	
	// Loops through the needed set to search for empty line
	if (loop){
		for (uint32_t i = 0; i < set_size; i++){
			if (cache[inset][i].tag == tag){
				if (cache[inset][i].valid == 1){
					cache_write_hits++;
					dummy_write_page_to_disk(char *page_data);
					return HIT;
				}else {
					cache_hits++;
					cache_misses++;
					return MISS;
				}
			// Already at the end of the cache and everything is already full
			} else if ((i + 1) == set_size){

			}
		}
	// For associativity == 1
	}else {
		if ((cache[inset][0].tag == tag) && (cache[inset][0].valid == 1)){
			cache_hits++;
			cache_read_hits++;
			dummy_write_page_to_disk(char *page_data);
			return HIT;	
		}else {
			cache[inset][0].tag = tag;
			cache[inset][0].valid = 1;
			return MISS;
		}
	}
}

// Process the S parameter properly and initialize `cache_size`.
// Return 0 when everything is good. Otherwise return -1.
int process_arg_S(int opt, char *optarg)
{
	if (opt == 'S'){
		cache_size = (uint32_t)atoi(optarg);
		return 0;
	}

	return -1;
}

// Process the A parameter properly and initialize `cache_associativity`.
// Return 0 when everything is good. Otherwise return -1.
int process_arg_A(int opt, char *optarg)
{
	if (opt == 'A'){
		cache_associativity = (uint32_t)atoi(optarg);
		return 0;
	}

	return -1;
}

// Process the B parameter properly and initialize `cache_block_size`.
// Return 0 when everything is good. Otherwise return -1.
int process_arg_B(int opt, char *optarg)
{
	if (opt == 'B'){
		cache_block_size = (uint32_t)atoi(optarg);
		return 0;
	}

	return -1;
}

char* accestype_string(access_t var){
	if (var == READ){
		return "R";
	}else if(var == WRITE){
		return "W";
	}else if(var == INVALID){
		return "I";
	}
}
// When verbose is true, print the details of each operation -- MISS or HIT.
void handle_cache_verbose(memory_access_entry_t entry, op_result_t ret)
{
	if (ret == MISS){
		printf(accestype_string(entry.accesstype),entry.address,"CACHE-MISS");
	} else if(ret == HIT){
		printf(accestype_string(entry.accesstype),entry.address,"CACHE-HIT");
	}else if(ret == ERROR) {
		printf("This message should not be printed. Fix your code\n");
	}
}

// Check if all the necessary paramaters for the cache are provided and valid.
// Return 0 when everything is good. Otherwise return -1.
int check_cache_parameters_valid()
{
	if ((cache_size == 0) || (cache_associativity == 0) ||(cache_block_size == 0)){
		return -1;
	}

	if (cache_size % 2 != 0){
		return -1;
	}

	if ((cache_associativity < 0) || (cache_associativity > 4)){
		return -1;
	}

	if ((cache_block_size < 4) || (cache_block_size % 4 != 0) || (cache_size % cache_block_size != 0)){
		return -1;
	}

	return 0;
}
