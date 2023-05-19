#include "my_vm.h"
#include <math.h>
#include <memory.h>
#include <pthread.h>
#define BITS_PER_BYTE 8

struct translation{
    unsigned long long VPN;
    unsigned long long PFN;
};
typedef struct translation translation;
void *memSpace = NULL;
void *pageDirectory = NULL;

unsigned long pageDirectoryEntryCounter = 0;

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

char *virtualAddressBitmap;
char *physicalPageBitmap;


//defining math
const unsigned long offsetBitsLength = log2(PGSIZE);
const unsigned long resultingBitsLength = (BIT_MACHINE_SIZE_32 - offsetBitsLength);

int pageDirectoryBitOffsetSize = 0;
int pageTableBitOffsetSize = 0;

// if (resultingBitsLength % 2 == 0)
// {
//     pageDirectoryBitOffsetSize = resultingBitsLength / 2; 
//     pageTableBitOffsetSize = resultingBitsLength / 2;
// }
// //Uneven split in page table sizes. Make root page table the larger of the two
// else
// {
//     pageDirectoryBitOffsetSize = (resultingBitsLength / 2) + 1;
//     pageTableBitOffsetSize = resultingBitsLength / 2;
// }

// #if (resultingBitsLength % 2 == 0)
//     #define SPLIT_OFFSET (resultingBitsLength / 2)
// #else
//     #define SPLIT_OFFSET ((resultingBitsLength / 2) + 1)
// #endif
// pageDirectoryBitOffsetSize = SPLIT_OFFSET;
// pageTableBitOffsetSize = resultingBitsLength - SPLIT_OFFSET;


const unsigned long maxNumberOfPhysicalPages = MAX_MEMSIZE / PGSIZE;
const unsigned long maxNumberOfVirtualAddresses = MAX_MEMSIZE / PGSIZE;

const unsigned long numberOfBytesToRepresentPhysicalPagesAsBits = maxNumberOfPhysicalPages / BITS_PER_BYTE;
const unsigned long numberOfBytesToRepresentNumberOfEntriesAsBits = maxNumberOfVirtualAddresses / BITS_PER_BYTE;
//numberOfBytesToRepresentNumberOfEntriesAsBits = (PGSIZE / sizeof(pde_t)) * (PGSIZE / sizeof(pte_t));

const unsigned long numberOfPageDirectoryEntries = PGSIZE / sizeof(pde_t);
const unsigned long numberOfPageTableEntriesPerTable = PGSIZE / sizeof(pte_t);

const unsigned long numberOfBytesToRepresentOnePageDirectoryEntrysPageTables = numberOfBytesToRepresentNumberOfEntriesAsBits / numberOfPageDirectoryEntries;
const unsigned long NUMBER_OF_BITS_IN_VIRTUAL_BITMAP = numberOfBytesToRepresentNumberOfEntriesAsBits * BITS_PER_BYTE;

const unsigned long NUMBER_OF_BITS_IN_PHYSICAL_BITAMP = numberOfBytesToRepresentNumberOfEntriesAsBits * BITS_PER_BYTE;
translation* TLB = NULL;

// void print_constants(){
//     fprintf(stderr, "Max number of physical pages: %d\n", maxNumberOfPhysicalPages);
// fprintf(stderr, "Max number of virtual pages: %d\n", maxNumberOfVirtualAddresses);
// fprintf(stderr, "Number of bytes to represent physical pages as bits: %d\n", numberOfBytesToRepresentPhysicalPagesAsBits);
// fprintf(stderr, "Number of bytes to represent number of entries as bits: %d\n", numberOfBytesToRepresentNumberOfEntriesAsBits);
// fprintf(stderr, "Number of page directory entries: %d\n", numberOfPageDirectoryEntries);
// fprintf(stderr, "Number of page table entries per table: %d\n", numberOfPageTableEntriesPerTable);
// fprintf(stderr, "Number of bytes to represent one page directory entry's page table: %d\n", numberOfBytesToRepresentOnePageDirectoryEntrysPageTables);
// fprintf(stderr, "Number of bits in virtual bitmap: %d\n", NUMBER_OF_BITS_IN_VIRTUAL_BITMAP);
// }

/* 
 * Function 3: GETTING A BIT AT AN INDEX 
 * Function to get a bit at "index"
 */
static int get_bit_at_index(char *bitmap, int index)
{
    pthread_mutex_lock(&mut);
    //Get to the location in the character bitmap array
    //Implement your code here

	int incrementer = index / 8;
	
	char c = bitmap[incrementer];
	
	int  pos = 1;
	//pos <<= (7- (index % 8));
	
	c >>= (7 - (index % 8));
    pthread_mutex_unlock(&mut);
	return (pos & c); 

	//return *bitmap
}

// void update_page_tables(int pageDirectoryIndex, int pageTableIndex){
    
// }

/* 
 * Function 2: SETTING A BIT AT AN INDEX 
 * Function to set a bit at "index" bitmap
 */
static void set_bit_at_index(char *bitmap, int index)
{
    pthread_mutex_lock(&mut);
	//32 bits to set starting at 0 so 0-31
	//do 32/ index to get which char to access
	int incrementer = index / 8;
	
	//we are in scope of character
	//with ; bitmap[incrementer];

	//uint8_t i = 1;
	int i = 1;
	i <<= (7 - (index % 8));
	
	bitmap[incrementer] |= i;
    pthread_mutex_unlock(&mut);

	return;
}

/*
Function responsible for allocating and setting your physical memory 
*/
void set_physical_mem() {
    //Allocate physical memory using mmap or malloc; this is the total size of your memory you are simulating
    //void *memSapce = (void*) malloc(MAX_MEMSIZE);

    //In the memSpace, should be 'MEMSIZE' of physical memory
    memSpace = calloc(maxNumberOfPhysicalPages, PGSIZE);

    //for (int i = 0; i < )
    
    //HINT: Also calculate the number of physical and virtual pages and allocate
    //virtual and physical bitmaps and initialize them

    //Allocate physical page bitmap. In 4gb memspace, 4kb page, this will be 131,072 bytes
    physicalPageBitmap = (char *) malloc(numberOfBytesToRepresentPhysicalPagesAsBits);

    //Allocate virtual addressing bitmap. In 4gb memspace, 4kb page, this will be 131,072 bytes.
    virtualAddressBitmap = (char *) malloc(numberOfBytesToRepresentNumberOfEntriesAsBits);

    //mark the virtual and physical address bitmaps. this is for the page directory
    set_bit_at_index(physicalPageBitmap, 0);
    set_bit_at_index(virtualAddressBitmap, 0);
}



static void clear_bit_at_index(char *bitmap, int index)
{
    pthread_mutex_lock(&mut);
	//32 bits to set starting at 0 so 0-31
	//do 32/ index to get which char to access
	int incrementer = index / 8;
	
	//we are in scope of character
	//with ; bitmap[incrementer];

	//uint8_t i = 1;
	int i = 1;
	i <<= (7 - (index % 8));
	
	bitmap[incrementer] &= ~i;
    pthread_mutex_unlock(&mut);
	return;
}

/*
 * Part 2: Add a virtual to physical page translation to the TLB.
 * Feel free to extend the function arguments or return type.
 */
int
add_TLB(void *va, void *pa)
{
    if(TLB = NULL)
    {
        TLB = malloc(sizeof(translation) * TLB_ENTRIES);
    }
    /*Part 2 HINT: Add a virtual to physical page translation to the TLB */
    return 0;
}


/*
 * Part 2: Check TLB for a valid translation.
 * Returns the physical page address.
 * Feel free to extend this function and change the return type.
 */
pte_t *
check_TLB(void *va) {

    /* Part 2: TLB lookup code here */
    if(TLB = NULL)
    {
        TLB = malloc(sizeof(translation) * TLB_ENTRIES);
        return NULL;
    }
    //If the address isnt present then we must look in main memory
    if((TLB[(((unsigned long ) va) % TLB_ENTRIES)]).VPN != (unsigned long) va >> (unsigned int) log2(PGSIZE))
    {
        return NULL;
    }
    else
    {
        
    }
   /*This function should return a pte_t pointer*/
}


/*
 * Part 2: Print TLB miss rate.
 * Feel free to extend the function arguments or return type.
 */
void
print_TLB_missrate()
{
    double miss_rate = 0;	

    /*Part 2 Code here to calculate and print the TLB miss rate*/


    fprintf(stderr, "TLB miss rate %lf \n", miss_rate);
}


void *generate_virtual_address(void *physicalAddress)
{

}


/*
The function takes a virtual address and page directories starting address and
performs translation to return the physical address
*/
pte_t *translate(pde_t *pgdir, void *va) {
    pthread_mutex_lock(&mut);
    /* Part 1 HINT: Get the Page directory index (1st level) Then get the
    * 2nd-level-page table index using the virtual address.  Using the page
    * directory index and page table index get the physical address.
    *
    * Part 2 HINT: Check the TLB before performing the translation. If
    * translation exists, then you can return physical address from the TLB.
    */
    //Physical Address to be returned
    //pte_t *physicalAddress = NULL;

    if (resultingBitsLength == 0)
    {
        if (resultingBitsLength % 2 == 0)
        {
            pageDirectoryBitOffsetSize = resultingBitsLength / 2; 
            pageTableBitOffsetSize = resultingBitsLength / 2;
        }
        //Uneven split in page table sizes. Make root page table the larger of the two
        else
        {
            pageDirectoryBitOffsetSize = (resultingBitsLength / 2) + 1;
            pageTableBitOffsetSize = resultingBitsLength / 2;
        }
    }


    // unsigned long pageDirectoryIndex = (unsigned long) va >> (pageTableBitOffsetSize + offsetBitsLength);
    // unsigned long pageTableIndex = (unsigned long) va << (pageDirectoryBitOffsetSize);
    // pageTableIndex >>= (BIT_MACHINE_SIZE_32 - pageTableBitOffsetSize);
    // //pageDirectoryIndex

    //Get the pageTableOneIndex from the virtual address, using the pageTableOneBitOffsetSize
    unsigned long pageDirectoryIndex = (unsigned long) va >> (pageTableBitOffsetSize + offsetBitsLength);

    //nextPage is an unsigned long, which represents the address of the next page 
    char *pgDirPtr = ( ((char *) pgdir) + (sizeof(pde_t) * pageDirectoryIndex));
    pde_t *pgDirPtrPde = (pde_t*) pgDirPtr;

    if (pgDirPtrPde == NULL)
    {
        return NULL;
    }

    unsigned long pageTableIndex = (unsigned long) va << (pageDirectoryBitOffsetSize);
    pageTableIndex >>= (BIT_MACHINE_SIZE_32 - pageTableBitOffsetSize);

    char *pageTableAddress = ((char *) memSpace) + ((*pgDirPtrPde) * PGSIZE);

    //get the entry in the page table
    char *pageTableEntryPtr = ( ((char *) pageTableAddress) + (sizeof(pte_t) * pageTableIndex)); 
    pte_t *pageTableEntryPtrPte = (pte_t*) pageTableEntryPtr;

    if (*pageTableEntryPtrPte == 0)
    {
        return NULL;
    }

    char *physicalAddressCharPtr = (((char *) memSpace) + (*pageTableEntryPtrPte * PGSIZE));
    pte_t *physicalAddressPtePtr = (pte_t *) physicalAddressCharPtr;

    pthread_mutex_unlock(&mut);
    //return NULL; 
    return physicalAddressPtePtr;
}


/*
The function takes a page directory address, virtual address, physical address
as an argument, and sets a page table entry. This function will walk the page
directory to see if there is an existing mapping for a virtual address. If the
virtual address is not present, then a new entry will be added
*/
int page_map(pde_t *pgdir, void *va, void *pa)
{
    if (resultingBitsLength != 0)
    {
        if (resultingBitsLength % 2 == 0)
        {
            pageDirectoryBitOffsetSize = resultingBitsLength / 2; 
            pageTableBitOffsetSize = resultingBitsLength / 2;
        }
        //Uneven split in page table sizes. Make root page table the larger of the two
        else
        {
            pageDirectoryBitOffsetSize = (resultingBitsLength / 2) + 1;
            pageTableBitOffsetSize = resultingBitsLength / 2;
        }
    }


    pthread_mutex_lock(&mut);

    //Get the pageTableOneIndex from the virtual address, using the pageTableOneBitOffsetSize
    unsigned long pageDirectoryIndex = (unsigned long) va >> (pageTableBitOffsetSize + offsetBitsLength);

    //nextPage is an unsigned long, which represents the address of the next page 
    char *pgDirPtr = ( ((char *) pgdir) + (sizeof(pde_t) * pageDirectoryIndex));
    pde_t *pgDirPtrPde = (pde_t*) pgDirPtr;

    //Go to PDE and check if it is a new page table or if the page table has been allocated already
    if (pgDirPtrPde == NULL)
    {
        //look in the physical page bitmap to allocate another page
        for (int i = 0; i < NUMBER_OF_BITS_IN_PHYSICAL_BITAMP; i ++)
        {
            if (get_bit_at_index(physicalPageBitmap, i ) == 0)
            {
                //get a virtual address for the page table that we are adding
                //void *pageTableVirtualAddress = get_next_avail(1);

                set_bit_at_index(physicalPageBitmap, i);
                
                *pgDirPtrPde = i;
                // //get the address of the page table based on i
                // void *pageTableAddress;
                // char *memSpacePtr = (char*) memSpace;
                // //pageTableAddress = (void *) (memSpacePtr + (PGSIZE * i));
                // memSpacePtr = (memSpacePtr + (PGSIZE * i));
                // pageTableAddress = (void *) memSpacePtr;
                //only need to store 'i' in the page directory entry. with i we can compute the address where the page is

                //put a reference to the page table at the page directory index
                //void *addressToPlacePDEIn = (void *) (pgdir + (sizeof(pde_t*) * pageDirectoryIndex));
                //*nextPageAsPDE = (pde_t *) pageTableAddress;

                break;
            }
        }

        //STILL HAVE TO MAP THE Page table if it was null !!!!
    }
    //char *memSpacePtr = (char*) memSpace;
    unsigned long pageTableIndex = (unsigned long) va << (pageDirectoryBitOffsetSize);
    pageTableIndex >>= (BIT_MACHINE_SIZE_32 - pageTableBitOffsetSize);

    char *pageTableAddress = ((char *) memSpace) + ((*pgDirPtrPde) * PGSIZE);

    //get the entry in the page table
    char *pageTableEntryPtr = ( ((char *) pageTableAddress) + (sizeof(pte_t) * pageTableIndex)); 
    pte_t *pageTableEntryPtrPte = (pte_t*) pageTableEntryPtr;

    int allPagesFull = 1;
    if (*pageTableEntryPtrPte == 0)
    {
                //look in the physical page bitmap to allocate another page
        for (int i = 0; i < NUMBER_OF_BITS_IN_PHYSICAL_BITAMP; i ++)
        {
            if (get_bit_at_index(physicalPageBitmap, i ) == 0)
            {
                //get a virtual address for the page table that we are adding
                //void *pageTableVirtualAddress = get_next_avail(1);

                set_bit_at_index(physicalPageBitmap, i);
                allPagesFull = 0;
                *pageTableEntryPtrPte = i;
                // //get the address of the page table based on i
                // void *pageTableAddress;
                // char *memSpacePtr = (char*) memSpace;
                // //pageTableAddress = (void *) (memSpacePtr + (PGSIZE * i));
                // memSpacePtr = (memSpacePtr + (PGSIZE * i));
                // pageTableAddress = (void *) memSpacePtr;
                //only need to store 'i' in the page directory entry. with i we can compute the address where the page is

                //put a reference to the page table at the page directory index
                //void *addressToPlacePDEIn = (void *) (pgdir + (sizeof(pde_t*) * pageDirectoryIndex));
                //*nextPageAsPDE = (pde_t *) pageTableAddress;

                break;
            }
        }

        //STILL HAVE TO MAP THE Page table if it was null !!!!
    }



    // pte_t *page = (pte_t*) (nextPageAsPDE + (sizeof(pte_t*) * pageTableIndex));
    
    // int pageIndex = (unsigned long) *page;
    // if (get_bit_at_index(physicalPageBitmap, pageIndex) == 0)
    // {
    //     //if the page has not been set, allocate it
        
    // }

    
    /*HINT: Similar to translate(), find the page directory (1st level)
    and page table (2nd-level) indices. If no mapping exists, set the
    virtual to physical mapping */
    pthread_mutex_unlock(&mut);
    
    if (allPagesFull == 1)
    {
        return -1;
    }
    return 1;
}

//Returns the index of the page directory 0-1023
int calculate_page_directory_index(int bitPositionOfPageDirectoryPageTableCombination){
    return (unsigned long ) bitPositionOfPageDirectoryPageTableCombination / numberOfBytesToRepresentOnePageDirectoryEntrysPageTables ;
}

//Returns the index of the page table 0-1023
int calculate_page_table_index(int bitPositionOfPageDirectoryPageTableCombination){
    return (unsigned long) bitPositionOfPageDirectoryPageTableCombination % numberOfPageTableEntriesPerTable;
}



/*Function that gets the next available page
*/

void *get_next_avail(int num_pages) {
    pthread_mutex_lock(&mut);
    //Use virtual address bitmap to find the next free page
    // numberOfBytesToRepresentOnePageDirectoryEntrysPageTables = numberOfBytesToRepresentNumberOfEntriesAsBits / numberOfPageDirectoryEntries;

    //NUMBER_OF_BITS_IN_VIRTUAL_BITMAP = numberOfBytesToRepresentNumberOfEntriesAsBits * BITS_PER_BYTE;
    // printf("%d\n", );
    int contiguousElementsFound = 0;
    //Iterates through each bit in the bitmap
    unsigned long pageDirectoryIndex;
    unsigned long pageTableIndex;
    for (int i = 0 ; i < NUMBER_OF_BITS_IN_VIRTUAL_BITMAP; i ++)
    {
        //The bit at position i is free
        if (get_bit_at_index(virtualAddressBitmap, i) == 0)
        {
            contiguousElementsFound += 1;
        }

        else
        {
            contiguousElementsFound = 0;
        }

        if (contiguousElementsFound == num_pages)
        {
            //go back num_pages and mark them as visited
            //int firstAddress = 0;
            for (int k = i; k > (i - num_pages); k --)
            {
                //Mark it as set
                set_bit_at_index(virtualAddressBitmap, k);

                pageDirectoryIndex = calculate_page_directory_index(i);
                pageTableIndex = calculate_page_table_index(i);
                //update_page_tables(pageDirectoryIndex, pageTableIndex);
                //firstAddress = k;
            }

            contiguousElementsFound = 0;


            //compute virtual address by combining pagedirectoryindex and pagetableindex and add the offset 
            pageTableIndex <<= offsetBitsLength;
            pageDirectoryIndex <<= (offsetBitsLength + pageTableBitOffsetSize);

            unsigned long virtualAddress = pageTableIndex | pageDirectoryIndex;
            pthread_mutex_unlock(&mut);
            return (void*) virtualAddress;

            //return NULL;

        }

    }

    // //If reached here no more space available in page directory/tables 
    pthread_mutex_unlock(&mut);
    return NULL;


    // if (contiguousElementsFound > 0)
    // {
    //     //Reached end and no contiguous space was found
    //     return NULL;
    // }
}

/* Creates page table directory in the memory allocated, memSpace
*/
void create_page_directory(){
    /*
    Page Size = PGSIZE
    Need to allocate 1024 slots in case of 4kB table.
    */
    
    //Cast the void* into pde_t* so that we can access 4 byte chunks to create the page table

    pde_t *buff = (pde_t*) memSpace;


    //Initialize Page Directory Table in the memspace to NULL
    for (int i = 0; i < numberOfPageDirectoryEntries; i ++)
    //increments every sizeof(pde_t) bytes starting at memSpace, to NULL;
    {
        *(buff + i) = (pde_t) NULL;
    }

    //Calculate the end address of the page directory 
    //void *endOfPDE = (void*) (&buff + PGSIZE);
    pageDirectory = (void*) buff;
}


void create_first_page(){
    //Check the bitmap to find 
}

/* Function responsible for allocating pages
and used by the benchmark
*/
void *t_malloc(unsigned int num_bytes) {
    pthread_mutex_lock(&mut);
    /* 
     * HINT: If the physical memory is not yet initialized, then allocate and initialize.
     */
    if (memSpace == NULL)
    {
        set_physical_mem();
    }

    /*
    * HINT: If the page directory is not initialized, then initialize the
    * page directory. 
    */
    if (pageDirectory == NULL)
    {
        create_page_directory();
    }

    //Gets physical address of next available page ... this rounds up to the highest page
    int pagesNeeded = (num_bytes + (PGSIZE - 1)) / PGSIZE;
    void *virtualAddress = get_next_avail(pagesNeeded);

    for (int i = 0; i < pagesNeeded; i ++)
    {
        //Step 1 find a free physical page
        void *tempPa = NULL;

        //get rid of the offset bits. this way we can add i to the end and it will get the correct pagedirindex and pagetableindex if the malloc goes onto another pde
        //void *tempVa = (unsigned long) (virtualAddress >> offsetBitsLength);
        unsigned long newVa = (unsigned long) (((unsigned long)virtualAddress) >> offsetBitsLength);


        //cast it to an integer and add 1.
        //unsigned long newVa = (unsigned long) tempVa;
        newVa += i;

        //shift the bits back 
        newVa <<= offsetBitsLength;

        //cast it back to a void*
        void *tempVa = (void *) newVa;

        for (int i = 0; i < NUMBER_OF_BITS_IN_PHYSICAL_BITAMP; i ++)
        {
            //if we find a free page, page number i is a free page, we can put a page there
            if (get_bit_at_index(physicalPageBitmap, i) == 0)
            {
                //Page i is free, *pa is (memSpace + (PGSIZE*i))
                set_bit_at_index(physicalPageBitmap, i);
                char *memSpacePtr = (char*) memSpace;
                tempPa = (void *) (memSpacePtr + (PGSIZE * i));
            
                page_map((pde_t*) pageDirectory, tempVa, tempPa);
                break;
            }

            // memSpace + (i * )
        }

        //goal is to map each virtual address that was allocated per page to a physical page. We have the first phycial page number, we want the other page numbers
        // int pageDirIndex = reverse_engineer_page_dir(virtualAddress);
        // int pageTableIndex = reverse_engineer_page_table(virtualAddress);

        //The virtual address to pass in
        //unsigned int virtualAddresBuilder = ; 



    //     page_map(pageDirectory, virtualAddress, nextAvailablePage);
    }
    //at this point virtual bitmap is set, we have the virtual address
    //void *virtualAddress = generate_virtual_address();


    //map the virtual address to the physical address
    //physical address is nextAvailablePage

   /* 
    * HINT: If the page directory is not initialized, then initialize the
    * page directory. 
    * Next, using get_next_avail(), check if there are free pages. If
    * free pages are available, set the bitmaps and map a new page. Note, you will 
    * have to mark which physical pages are used. 
    */
     pthread_mutex_unlock(&mut);
    return NULL;
}

/* Responsible for releasing one or more memory pages using virtual address (va)
*/
void t_free(void *va, int size) {
    pthread_mutex_lock(&mut);

    /* Part 1: Free the page table entries starting from this virtual address
     * (va). Also mark the pages free in the bitmap. Perform free only if the 
     * memory from "va" to va+size is valid.
     *
     * Part 2: Also, remove the translation from the TLB
     */

    //Lets say we have va 400 each page size is 4 and the size of our value is 9
    //This means we need 3 pages 400 404 and 408 so va[0] = 400 and va[1] = 404 and va[2] = 408
    int numpages = 1;
    if(size > PGSIZE)
    {
        numpages = size/PGSIZE + 1;
    }

    //Create array containing all virtual addresses for pages
    void** vas = malloc(numpages * sizeof(void*));
    for(int i = 0; i < numpages; i++)
    {
        vas[i] = va + PGSIZE * i;
        //Need to check if these pages are valid, if one of them are invalid we cannot free
        if(get_bit_at_index(virtualAddressBitmap, ((unsigned long) vas[i]) >> offsetBitsLength) == 0)
        {
            pthread_mutex_unlock(&mut);
            return;
        }
        else
        {
            clear_bit_at_index(virtualAddressBitmap, ((unsigned long) vas[i]) >> offsetBitsLength); 
        }
    } 
    pthread_mutex_unlock(&mut);
}


/* The function copies data pointed by "val" to physical
 * memory pages using virtual address (va)
 * The function returns 0 if the put is successfull and -1 otherwise.
*/
int put_value(void *va, void *val, int size) {
    pthread_mutex_lock(&mut);

    /* HINT: Using the virtual address and translate(), find the physical page. Copy
     * the contents of "val" to a physical page. NOTE: The "size" value can be larger 
     * than one page. Therefore, you may have to find multiple pages using translate()
     * function.
     */
    //See how many pages we need to split the value
    int numpages = 1;
    if(size > PGSIZE)
    {
        numpages = size/PGSIZE + 1;
    }

    //Create array containing all virtual addresses for pages
    void** vas = malloc(numpages * sizeof(void*));
    for(int i = 0; i < numpages; i++)
    {
        //Lets say we have va 400 each page size is 4 and the size of our value is 9
        //This means we need 3 pages 400 404 and 408 so va[0] = 400 and va[1] = 404 and va[2] = 408
        vas[i] = va + PGSIZE * i;
        //Need to check if these pages are valid 
        if(get_bit_at_index(virtualAddressBitmap, ((unsigned long) vas[i]) >> offsetBitsLength) == 0)
        {
            return -1;
        }
    } 

    //Now we need to translate to physical pages
    void** pas = malloc(numpages * sizeof(void*));
    for(int i = 0; i < numpages; i++)
    {
        pas[i] = translate(pageDirectory, vas[i]);
    }

    //Now we put the value
    for(int i = 0; i < numpages; i++)
    {
        //If the size is not disvisible by PGSIZE
        if(i = numpages - 1 && size % PGSIZE != 0)
        {
            //Last page so we may not need to copy fully
            memcpy(pas[i], val + PGSIZE * i, size % PGSIZE);
        }
        else
        {
            //Copy to full page
            memcpy(pas[i], val + PGSIZE * i, PGSIZE);
        }
    }
    free(pas);
    free(vas);
    pthread_mutex_unlock(&mut);
    return 0;
    /*return -1 if put_value failed and 0 if put is successfull*/

}


/*Given a virtual address, this function copies the contents of the page to val*/
void get_value(void *va, void *val, int size) {
    pthread_mutex_lock(&mut);
    /* HINT: put the values pointed to by "va" inside the physical memory at given
    * "val" address. Assume you can access "val" directly by derefencing them.
    */
   //See how many pages the value is split 
    int numpages = 1;
    if(size > PGSIZE)
    {
        numpages = size/PGSIZE + 1;
    }

    void** vas = malloc(numpages * sizeof(void*));
    for(int i = 0; i < numpages; i++)
    {
        //Need to check if these pages are valid 
        if(get_bit_at_index(virtualAddressBitmap, ((unsigned long) vas[i]) >> offsetBitsLength) == 0)
        {
            return;
        }
        //Lets say we have va 400 each page size is 4 and the size of our value is 9
        //This means we need 3 pages 400 404 and 408 so va[0] = 400 and va[1] = 404 and va[2] = 408
        vas[i] = va + PGSIZE * i;
    }
    //Now we need to translate to physical pages
    void** pas = malloc(numpages * sizeof(void*));
    for(int i = 0; i < numpages; i++)
    {
        pas[i] = translate(pageDirectory, vas[i]);
    }

    //Now we get the value
    for(int i = 0; i < numpages; i++)
    {
        if(i = numpages - 1 && size % PGSIZE != 0)
        {
            //Last page so we may not need to copy 
            memcpy(val + PGSIZE * i, pas[i], size % PGSIZE);
        }
        else
        {
            //Copy to full page
            memcpy(val + PGSIZE * i, pas[i], PGSIZE);
        }
    }
    free(pas);
    free(vas);
    pthread_mutex_unlock(&mut);
    return;
}



/*
This function receives two matrices mat1 and mat2 as an argument with size
argument representing the number of rows and columns. After performing matrix
multiplication, copy the result to answer.
*/
void mat_mult(void *mat1, void *mat2, int size, void *answer) {

    /* Hint: You will index as [i * size + j] where  "i, j" are the indices of the
     * matrix accessed. Similar to the code in test.c, you will use get_value() to
     * load each element and perform multiplication. Take a look at test.c! In addition to 
     * getting the values from two matrices, you will perform multiplication and 
     * store the result to the "answer array"
     */
    int x, y, val_size = sizeof(int);
    int i, j, k;
    for (i = 0; i < size; i++) {
        for(j = 0; j < size; j++) {
            unsigned int a, b, c = 0;
            for (k = 0; k < size; k++) {
                int address_a = (unsigned int)mat1 + ((i * size * sizeof(int))) + (k * sizeof(int));
                int address_b = (unsigned int)mat2 + ((k * size * sizeof(int))) + (j * sizeof(int));
                get_value( (void *)address_a, &a, sizeof(int));
                get_value( (void *)address_b, &b, sizeof(int));
                // printf("Values at the index: %d, %d, %d, %d, %d\n", 
                //     a, b, size, (i * size + k), (k * size + j));
                c += (a * b);
            }
            int address_c = (unsigned int)answer + ((i * size * sizeof(int))) + (j * sizeof(int));
            // printf("This is the c: %d, address: %x!\n", c, address_c);
            put_value((void *)address_c, (void *)&c, sizeof(int));
        }
    }
}



