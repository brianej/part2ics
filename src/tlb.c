#include "tlb.h"
#include <stdlib.h>
#include <math.h>

// Input parameters to control the tlb.
uint32_t tlb_entries;
uint32_t tlb_associativity;

// TLB statistics counters.
uint32_t tlb_total_accesses;
uint32_t tlb_hits;
uint32_t tlb_misses;

tlb_entry_t** tlb;
tlb_entry_t** tlb_used;
uint32_t tlb_set_size;
uint32_t tlb_set;

uint32_t power2_tlb(uint32_t n) {
    return n > 0 && ((int)log2(n) == log2(n));
}

// gets the vpn from an address
uint32_t tlb_vpn_getter(uint32_t address)
{
    // offset for 4KB is 12
    uint32_t vpn = address >> 12;
	return vpn;
}

// gets the index from an address
uint32_t tlb_index_getter(uint32_t pa){

	// offset for 4KB is 12
	uint32_t offset = 12;

	// The index in bits
	uint32_t idx = log2(tlb_set);

	// Right shift till only tag left
	uint32_t tag = pa >> offset;

	// Create a bitmask with the least significant n bits set to 1
    uint32_t bitmask = (1 << idx) - 1;

    // Use bitwise AND to extract the last n bits
    uint32_t index = tag & bitmask;

	return index;
}

// enters new address into the tlb when there is still space
void tlb_used_enter(uint32_t set,uint32_t vpn,uint32_t ppn){
	for (int i = 0; i < tlb_set; i++){
		if (tlb_used[set][i].valid == 0){
			tlb_used[set][i].valid = 1;
			tlb_used[set][i].PPN = ppn;
			tlb_used[set][i].VPN = vpn;
			tlb_used[set][i].dirty = 0;
		}
	}
}

 // recently used 
void tlb_recently_used(uint32_t set,uint32_t vpn)
{
	uint32_t index;

	for (int i = 0; i < tlb_set_size; i++){
		if (tlb_used[set][i].VPN == vpn){
			index = i;
			break;
		}
	}

 	tlb_entry_t temp = tlb_used[set][index];

 	for(int i = index; i > 0;i--){
 		tlb_used[set][i]= tlb_used[set][i-1];
 	}

 	tlb_used[set][0] = temp; 
}

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
		tlb_set_size = 2;
	} else if(tlb_associativity == 4){
		tlb_set_size = 4;
	} else if(tlb_associativity == 1){
		tlb_set_size = 1;
	} else{
		tlb_set_size = tlb_entries;
	}

	tlb_set = tlb_entries / tlb_set_size;

	tlb = malloc(tlb_set * sizeof(tlb_entry_t*));

	tlb_used = malloc(tlb_set * sizeof(tlb_entry_t*));

	// Allocating lines for each set
	for (int i = 0; i < tlb_set; i++){
        *(tlb + i) = malloc(tlb_set_size * sizeof(tlb_entry_t));
    }

	// Allocating lines for each set, for used 
	for (int i = 0; i < tlb_set; i++){
        *(tlb_used + i) = malloc(tlb_set_size * sizeof(tlb_entry_t));
    }

	// Initialising each variable of block in every set
	for (int i = 0; i < tlb_set; i++){
		for (int j = 0; j < tlb_set_size; j++){
			tlb[i][j].valid = 0;
			tlb[i][j].dirty = 0;
			tlb[i][j].VPN = 0;
            tlb[i][j].PPN = 0;
		}
	}

	// Initialising each variable of block in every set, for used
	for (int i = 0; i < tlb_set; i++){
		for (int j = 0; j < tlb_set_size; j++){
			tlb_used[i][j].valid = 0;
			tlb_used[i][j].dirty = 0;
			tlb_used[i][j].VPN = 0;
            tlb_used[i][j].PPN = 0;
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
	} else return -1;

 	if ((tlb_entries < 2)){
 		return -1;
 	}

 	if (!(power2_tlb(tlb_entries))){
 		return -1;
 	}

 	return 0;
}

// Process the A parameter properly and initialize `tlb_associativity`.
// Return 0 when everything is good. Otherwise return -1.
int process_arg_L(int opt, char *optarg)
{
    if (opt == 'L'){
		tlb_associativity = (uint32_t)atoi(optarg);
	}  else return -1;
	
 	if ((tlb_associativity < 0) || (tlb_associativity > 4)){
 		return -1;
 	}

 	return 0;
}

// Check if the tlb hit or miss.
// Extract the VPN from the address and use it.
// Keep associativity in mind while searching.
int check_tlb(uint32_t address){
    //return -1 if the entry is missing or valid bit is 0 aka tlb miss
    //return PPN if the entry is valid and TAG matches aka tlb hit
	tlb_total_accesses++;

	// If it needs the loop
	int loop = 0;
	// Which set to look in
	uint32_t inset;

	uint32_t vpn = tlb_vpn_getter(address);

	if (tlb_associativity == 1){
		inset =  tlb_index_getter(address);
	// Fully associative
	} else if (tlb_associativity == 2){
		loop = 1;
		inset = 0;
	} else if (tlb_associativity == 3){
		loop = 1;
		inset =  tlb_index_getter(address);
	} else if (tlb_associativity == 4){
		loop = 1;
		inset =  tlb_index_getter(address);
	}
	
	// Loops through the needed set to search for empty line for associativity > 1
	if (loop){
		for (uint32_t i = 0; i < tlb_set_size; i++){
			if (tlb[inset][i].VPN == vpn){
				if (tlb[inset][i].valid == 1){
					tlb_recently_used(inset,vpn);
					tlb_hits++;
					return tlb[inset][i].PPN;
				// return miss
				}else{
					tlb_misses++;
					return -1;
				}
			// Already at the end of the cache and return miss
			}else if ((i + 1) == tlb_set_size){
				tlb_misses++;
				return -1;
			}
		}
	// For associativity == 1
	} else {
		if ((tlb[inset][0].VPN == vpn) && (tlb[inset][0].valid == 1)){
			tlb_hits++;
			return tlb[inset][0].PPN;	
		}else {
			tlb_misses++;
			return -1;
		}
	}
	return -1;
}

void set_dirty_bit_in_tlb(uint32_t address){
    //set the dirty bit of the entry to 1
	if (tlb_associativity != 2){
		uint32_t index = tlb_index_getter(address);
		uint32_t vpn = tlb_vpn_getter(address);

		for (int i = 0; i < tlb_set_size; i++){
			if (tlb[index][i].VPN == vpn){
				tlb[index][i].dirty = 1;
			}
		}
	}else {
		for (int i = 0; i < tlb_set_size; i++){
			if (tlb[0][i].VPN == tlb_vpn_getter(address)){
				tlb[0][i].dirty = 1;
			}
		}
	}
}

// LRU replacement policy has to be implemented.
void insert_or_update_tlb_entry(uint32_t address, uint32_t PPN){
    // if the entry is free, insert the entry
    // if the entry is not free, identify the victim entry and replace it
    //set PPN for VPN in tlb
    //set valid bit in tlb
	uint32_t idx = tlb_index_getter(address);
	uint32_t victim_vpn = tlb_used[idx][tlb_set_size - 1].VPN;
	uint32_t vpn = tlb_vpn_getter(address);

	if (tlb_associativity == 1){
		tlb[idx][0].valid = 1;
		tlb[idx][0].PPN = PPN;
		tlb[idx][0].VPN = vpn;
		tlb[idx][0].dirty = 0;
	// for set associativity
	} else if (tlb_associativity > 2){
		for (int j = 0; j < tlb_set_size; j++){
			// if theres still space to add in
			if (tlb[idx][j].valid == 0){
				tlb[idx][j].valid = 1;
				tlb[idx][j].PPN = PPN;
				tlb[idx][j].VPN = vpn;
				tlb[idx][j].dirty = 0;
				tlb_used_enter(idx,vpn,PPN);
				tlb_recently_used(idx,vpn);
				break;
			// if theres no more space to add
			}else if((j + 1) == tlb_set_size){
				for (int k = 0; k < tlb_set_size; k++){
					if (tlb[idx][k].VPN == victim_vpn){
						tlb[idx][k].valid = 1;
						tlb[idx][k].PPN = PPN;
						tlb[idx][k].VPN = vpn;
						tlb[idx][k].dirty = 0;
						break;
					}
				}
				tlb_used[idx][tlb_set_size - 1].valid = 1;
				tlb_used[idx][tlb_set_size - 1].PPN = PPN;
				tlb_used[idx][tlb_set_size - 1].VPN = vpn;
				tlb_used[idx][tlb_set_size - 1].dirty = 0;
				tlb_recently_used(idx,vpn);
				break;
			}
		}
	// for fully associative
	} else{
		// loops throught every tlb entry to see if theres free space, if not replace the last used
		for (int i = 0; i < tlb_set; i++){
			for (int j = 0; j < tlb_set_size; j++){
				if (tlb[i][j].valid == 0){
					tlb[i][j].valid = 1;
					tlb[i][j].PPN = PPN;
					tlb[i][j].VPN = tlb_vpn_getter(address);
					tlb[i][j].dirty = 0;
					tlb_used_enter(idx,vpn,PPN);
					tlb_recently_used(idx,j);
					break;
				}else if((j + 1) == tlb_set_size){
					for (int k = 0; k < tlb_set_size; k++){
						if (tlb[i][k].VPN == victim_vpn){
							tlb[idx][k].valid = 1;
							tlb[idx][k].PPN = PPN;
							tlb[idx][k].VPN = vpn;
							tlb[idx][k].dirty = 0;
							break;
						}
					}
					tlb_used[idx][tlb_set_size - 1].valid = 1;
					tlb_used[idx][tlb_set_size - 1].PPN = PPN;
					tlb_used[idx][tlb_set_size - 1].VPN = vpn;
					tlb_used[idx][tlb_set_size - 1].dirty = 0;
					tlb_recently_used(idx,vpn);
					break;
				}
			}
		}
	}
}

// print pt entries as per the spec
void print_tlb_entries(){
    //print the tlb entries
	printf("\nTLB Entries (Valid-Bit Dirty-Bit VPN PPN)\n");
	for (int i = 0; i < tlb_set; i++){
		for (int j = 0; j < tlb_set_size; j++){
			if (tlb[i][j].valid == 1){
            	printf("%d %d 0x%05x 0x%05x\n", tlb[i][j].valid,tlb[i][j].dirty,tlb[i][j].VPN,tlb[i][j].PPN);
        	}else{
				printf("0 0 - -\n");
			}
		}
    }
}

// print tlb statistics as per the spec
void print_tlb_statistics(){
    //print the tlb statistics
    printf("\n* TLB Statistics *\n");
    printf("total accesses: %d\n", tlb_total_accesses);
    printf("hits: %d\n", tlb_hits);
    printf("misses: %d\n", tlb_misses);
}
