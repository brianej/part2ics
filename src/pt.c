#include "pt.h"
#include <stdlib.h>
#include "tlb.h"
#include "types.h"
#include "ll.h"
#include <math.h>

page_t* free_page_list;
page_t* used_page_list;
pt_entry_t* page_table;

// Page table statistics counters.
uint32_t page_table_total_accesses;
uint32_t page_table_faults;
uint32_t page_table_faults_with_dirty_page;

uint32_t empty = 0;
int last_used = 0;

// Gets the tag of the address
uint32_t vpn_getter(uint32_t pa){
	// The offset in bits
	uint32_t offset = 12;

	// Right shift till only tag left
	uint32_t vpn = pa >> offset;

	return vpn;
}

 // recently used 
 void pt_recently_used(uint32_t PPN)
 {
    int index = 0;

    // find where the index of the ppn in the used page list
    for (int i = 0; i < 256; i++){
        if (used_page_list[i].ppn == PPN){
            index = i;
            break;
        }
    }

 	page_t temp = used_page_list[index];

    // move down the page_t to make space for the recently used ppn
 	for(int i = index; i > 0;i--){
 		used_page_list[i]= used_page_list[i-1];
 	}

 	used_page_list[0] = temp;
 }

 // add ppn into used page list
 void add_to_used(uint32_t PPN, uint32_t index){
    pt_entry_t *page_table_entry = &page_table[index];
    used_page_list[last_used].ppn = PPN;
    used_page_list[last_used].page_table_entry = page_table_entry;
    last_used++;
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

    page_table_total_accesses = 0;
    page_table_faults = 0;
    page_table_faults_with_dirty_page = 0;
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

    page_table_total_accesses++;

    // If the valid bit is there
    if (page_table[vpn].present == 1){
        pt_recently_used(page_table[vpn].PPN);
        return page_table[vpn].PPN;
    }else {
        page_table_faults++;
       return -1;
    }
}

// Extract the VPN from the address and use it.
void update_page_table(uint32_t address, uint32_t PPN){
    //set PPN for VPN in page table entry
    //set present bit in page table entry
    uint32_t vpn = vpn_getter(address);

    // clear out any entry where the vicitm page was used in the page table
    for (int i = 0; i < pow(2,14); i++){
        if (page_table[i].PPN == PPN){
            page_table[i].PPN = 0;
            page_table[i].present = 0;
            break;
        }
    }

    // creating new page in page table
    page_table[vpn].PPN = PPN;
    page_table[vpn].present = 1;

    // if the used page is full, change the page table entry of the page 
    if (last_used == 256){
        pt_entry_t *page_table_entry = &page_table[vpn];
        used_page_list[255].ppn = PPN;
        used_page_list[255].page_table_entry = page_table_entry;
    }else {
        add_to_used(PPN,vpn);
    }

    pt_recently_used(PPN);
    insert_or_update_tlb_entry(address, PPN);
}

//set the dirty bit of the entry to 1
void set_dirty_bit_in_page_table(uint32_t address){
    page_table[vpn_getter(address)].dirty = 1;
}

// LRU is to be use to find the victim page
page_t *get_victim_page(){
    if (used_page_list[255].page_table_entry->dirty == 1){
        page_table_faults_with_dirty_page++;
    }
    // victim page is always at the bottom using our lru
    return &used_page_list[255];
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
    printf("\nPage Table Entries (Present-Bit Dirty-Bit VPN PPN)\n");
    for (int i = 0; i < pow(2,14); i++){
        if (page_table[i].present == 1){
            printf("%d %d 0x%05x 0x%05x\n", page_table[i].present,page_table[i].dirty,(uint32_t)i,page_table[i].PPN);
        }
    }
}

// print pt statistics as per the spec
void print_pt_statistics(){
    printf("\n* Page Table Statistics *\n");
    printf("total accesses: %d\n", page_table_total_accesses);
    printf("page faults: %d\n", page_table_faults);
    printf("page faults with a dirty bit: %d\n", page_table_faults_with_dirty_page);
}
