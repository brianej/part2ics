#include "pt.h"
#include <stdlib.h>
#include "tlb.h"
#include "types.h"
#include "ll.h"

page_t* free_page_list;
page_t* used_page_list;
pt_entry_t* page_table;

// Page table statistics counters.
uint32_t page_table_total_accesses;
uint32_t page_table_faults;
uint32_t page_table_faults_with_dirty_page;

uint32_t empty = 0;

// Gets the tag of the address
uint32_t vpn_getter(uint32_t pa){
	// The offset in bits
	uint32_t offset = 1;

	// Right shift till only tag left
	uint32_t vpn = pa >> offset;

	return vpn;
}

// LRU
 void LRU(block_t* Set[],block_t entry)
 {
 	// assign last value the new block
 	Set[set_size-1].tag = entry.tag;
 	Set[set_size-1].valid = entry.valid;
	Set[set_size-1].dirty = entry.dirty;
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

void initialize_pt_system()
{
    //free page list is being initialized
    init_free_page_list(&free_page_list);
    init_pt();
	return; 
}

/*
 * Initialize the "page_table" 
 * The "page_table" is declared in as extern in include/pt.h file.
 */
void init_pt() {
    uint32_t indexes = pow(2,14);
    page_table = malloc(indexes  * sizeof(pt_entry_t));
    used_page_list = malloc(256 * sizeof(page_t));

    for (int i = 0; i < 256; i++){
		used_page_list[i].ppn = 0;
	}

    for (int i = 0; i < indexes; i++){
		page_table[i].present = 0;
		page_table[i].dirty = 0;
		page_table[i].PPN = 0;
	}
}

void init_free_page_list(page_t** free_page_list){
    for (int i = 255; i >=0; i--)
    {
        page_t *new_page = (page_t *)malloc(sizeof(page_t));
        new_page->ppn = i;
        new_page->next = NULL;
        insert_in_ll(free_page_list, new_page);
    }
}

// Extract the VPN from the address and use it.
int check_page_table(uint32_t address){
    //return -1 if the entry is missing or present bit is 0 aka page fault
    //return PPN if the page is hit
    uint32_t vpn = vpn_getter(address);

    // If the valid bit is there
    if (page_table[vpn].present == 1){
        return page_table[vpn].PPN;
    }else {
        // if theres no more free page list to use, use lru on the used page list
        if (empty){

        }

        page_t *free_ppn = get_free_page();
        uint32_t address_ppn =  free_ppn->ppn;
        page_table[vpn].PPN = address_ppn;
        page_table[vpn].present = 1;
    }

    return vpn;
}

// Extract the VPN from the address and use it.
void update_page_table(uint32_t address, uint32_t PPN){
    //set PPN for VPN in page table entry
    //set present bit in page table entry
    insert_or_update_tlb_entry(address, PPN);
}

//set the dirty bit of the entry to 1
void set_dirty_bit_in_page_table(uint32_t address){
    
}

// LRU is to be use to find the victim page
page_t *get_victim_page(){
    return page_table[255];
}

// pops a page from the free page linked-list
page_t *get_free_page(){
    if (free_page_list == NULL){
        empty = 1;
        return NULL;
    } else{
        return delete_from_top_of_ll(&free_page_list);
    }
    return 0;
}

// print pt entries as per the spec
void print_pt_entries(){
    printf("\nPage Table Entries (Present-Bit Dirty-Bit PPN)\n");
}

// print pt statistics as per the spec
void print_pt_statistics(){
    printf("\n* Page Table Statistics *\n");
    printf("total accesses: %d\n", page_table_total_accesses);
    printf("page faults: %d\n", page_table_faults);
    printf("page faults with dirty bit: %d\n", page_table_faults_with_dirty_page);
}
