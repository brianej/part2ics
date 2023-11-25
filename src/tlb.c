#include "tlb.h"
#include <stdlib.h>

// Input parameters to control the tlb.
uint32_t tlb_entries;
uint32_t tlb_associativity;

// TLB statistics counters.
uint32_t tlb_total_accesses;
uint32_t tlb_hits;
uint32_t tlb_misses;

tlb_entry_t** tlb;
uint32_t set_size;
uint32_t set;

// Check if all the necessary paramaters for the tlb are provided and valid.
// Return 0 when everything is good. Otherwise return -1.
int check_tlb_parameters_valid()
{
	if ((tlb_entries == 0) || (tlb_associativity == 0) ){
		return -1;
	}

	if ((tlb_associativity < 0) || (tlb_associativity > 4)){
		return -1;
	}

	if ((tlb_entries < 2)){
		return -1;
	}

	return 0;
}

/*
 * Initialize the "tlb" depending on the input parameters T and L. 
 * The "tlb" is declared in as extern in include/tlb.h file.
 */
void initialize_tlb()
{

	// Sets how many sets on associativity
	if (tlb_associativity == 3){
		set_size = 2;
	} else if(tlb_associativity == 4){
		set_size = 4;
	} else if(tlb_associativity == 1){
		set_size = 1;
	} else{
		set_size = tlb_entries;
	}

	set = tlb_entries / set_size;

	tlb = malloc(set * sizeof(tlb_entry_t*));

	// Allocating lines for each set
	for (int i = 0; i < set; i++){
        *(tlb + i) = malloc(set_size * sizeof(tlb_entry_t));
    }

	// Initialising each variable of block in every set
	for (int i = 0; i < set; i++){
		for (int j = 0; j < set_size; j++){
			tbl[i][j].valid = 0;
			tbl[i][j].dirty = 0;
			tbl[i][j].VPN = 0;
            tlb[i][j].PPN = 0;
		}
	}
    tlb_total_accesses = 0;
    tlb_hits = 0;
    tlb_misses = 0;

	
}


// Process the T parameter properly and initialize `tlb_entries`.
// Return 0 when everything is good. Otherwise return -1.
int process_arg_T(int opt, char *optarg)
{
    if (opt == 'T'){
		tlb_entries = (uint32_t)atoi(optarg);
		return 0;
	}

	return -1;
}

// Process the A parameter properly and initialize `tlb_associativity`.
// Return 0 when everything is good. Otherwise return -1.
int process_arg_L(int opt, char *optarg)
{
    if (opt == 'L'){
		tlb_associativity = (uint32_t)atoi(optarg);
		return 0;
	}

	return -1;
}

// Check if the tlb hit or miss.
// Extract the VPN from the address and use it.
// Keep associativity in mind while searching.
int check_tlb(uint32_t address){
    //return -1 if the entry is missing or valid bit is 0 aka tlb miss
    //return PPN if the entry is valid and TAG matches aka tlb hit
    return 0;
}

void set_dirty_bit_in_tlb(uint32_t address){
    //set the dirty bit of the entry to 1
}

// LRU replacement policy has to be implemented.
void insert_or_update_tlb_entry(uint32_t address, uint32_t PPN){
    // if the entry is free, insert the entry
    // if the entry is not free, identify the victim entry and replace it
    //set PPN for VPN in tlb
    //set valid bit in tlb
}

// print pt entries as per the spec
void print_tlb_entries(){
    //print the tlb entries
    printf("\nTLB Entries (Valid-Bit Dirty-Bit VPN PPN)\n");
}

// print tlb statistics as per the spec
void print_tlb_statistics(){
    //print the tlb statistics
    printf("\n* TLB Statistics *\n");
    printf("total accesses: %d\n", tlb_total_accesses);
    printf("hits: %d\n", tlb_hits);
    printf("misses: %d\n", tlb_misses);
}
