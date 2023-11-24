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
    page_table = malloc(256 * sizeof(pt_entry_t));
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
    return -1;
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
    // you may use the used_page_list to find the victim page
    // return the victim page
    return 0;
}

// pops a page from the free page linked-list
page_t *get_free_page(){
    if (free_page_list == NULL){
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
