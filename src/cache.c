#include "cache.h"
#include <stdlib.h>

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
u_int32_t set_size;

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

uint32_t valid_getter(uint32_t pa){
	uint32_t offset;
	uint32_t valid;
	
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
	// If it needs the loop
	int loop = 0;
	// Which set to look in
	u_int32_t inset;

	if (cache_associativity == 1){

	// Fully associative
	} else if (cache_associativity == 2){
		loop = 1;

	} else if (cache_associativity == 3){
		loop = 1;
	} else if (cache_associativity == 4){
		loop = 1;
	}
	
	// Loops through the needed set to search for empty line
	if (loop){
		for (u_int32_t i = 0; i < set_size; i++){
			if (cache[inset][i].tag == pa){

			// Already at the end of the cache and everything is already full
			} else if ((i + 1) == set_size){

			}
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
	return ERROR;
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

// When verbose is true, print the details of each operation -- MISS or HIT.
void handle_cache_verbose(memory_access_entry_t entry, op_result_t ret)
{
	if (ret == ERROR) {
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
