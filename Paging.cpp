#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

// maximum memory size 1MB
#define MAX_MEM_SIZE (1024*1024) 
#define PAGE_SIZE 4096 //4KB
#define MAX_PAGES 256
#define PAGE_MEM_ADDR_SIZE 0x256
#define BLOCK_SIZE sizeof(_SBLOCK)
#define MEM_PAGE_BLOCK_SIZE sizeof(_MEMPAGEBLOCKS)

//structure for memory block
typedef struct __s_block
{
    struct __s_block* next;
    struct __s_block* prev;
    bool isfree;
    size_t size;
    int process;
} _SBLOCK;

//structure for allocate memory using paging
typedef struct __mempageblocks {
    struct __mempageblocks* next;
    bool isfree;
    void* memoryaddress;
    int percent;
}_MEMPAGEBLOCKS;

//structure for store virtual memory data using paging
typedef struct __virtmempageblocks {
    struct __virtmempageblocks* next;
    size_t size;
    int process;
    int pagenumber;
    void* memoryaddress;
    int percent;
}_VIRTMEMPAGEBLOCKS;

// structure of process
typedef struct __procintnode
{
    int process;
    int size;
    struct __procintnode* next;
} _PROCINTNODE;

//////////////////////////////////////////////
/*allocate memory block*/
_SBLOCK* allocateMemBlock(size_t size)
{
    _SBLOCK* block = sbrk(0);
    if (sbrk(BLOCK_SIZE + size) == (void*)-1) {
        return NULL;
    }
    else {
        block->next = NULL;
        block->prev = NULL;
        block->isfree = false;
        block->size = size;
        return block;
    }
}

void allocateNextMemBlock(size_t size, _SBLOCK** head)
{
    _SBLOCK* current = *head;
    void* allocate_mem = NULL;
    if (current == NULL) {
        *head = allocateMemBlock(size);
    }
    else {
        while (current->next != NULL) {
            current = current->next;
        }
        _SBLOCK* newblock = sbrk(0);

        allocate_mem = sbrk(BLOCK_SIZE + size);
        if (allocate_mem == (void*)-1) {}
        else {
            sbrk(BLOCK_SIZE + size);
            newblock->next = NULL;
            newblock->prev = current;
            newblock->isfree = false;
            newblock->size = size;
            current->next = newblock;
        }
    }
}

//////////////////////////////////////////////
/*allocate memory block for Paging*/

_MEMPAGEBLOCKS* allocateMemPageBlock(size_t size)
{
    _MEMPAGEBLOCKS* block = sbrk(0);
    if (sbrk(MEM_PAGE_BLOCK_SIZE + size) == (void*)-1) { return NULL; }
    else {
        sbrk(MEM_PAGE_BLOCK_SIZE + size);
        block->next = NULL;
        block->isfree = true;
        block->memoryaddress = sbrk(0);
        return block;
    }
}


void allocateNextMemPageBlock(size_t size, _MEMPAGEBLOCKS** head)  //linked list of memory block for paging
{
    _MEMPAGEBLOCKS* current = *head;
    void* allocate_mem = NULL;
    if (current == NULL) {
        *head = allocateMemPageBlock(size);
    }
    else {
        while (current->next != NULL) {
            current = current->next;
        }
        _MEMPAGEBLOCKS* newblock = sbrk(0);

        allocate_mem = sbrk(MEM_PAGE_BLOCK_SIZE + size);
        if (allocate_mem == (void*)-1) {}
        else {
            sbrk(MEM_PAGE_BLOCK_SIZE + size);
            newblock->next = NULL;
            newblock->isfree = true;
            newblock->memoryaddress = sbrk(0);
            current->next = newblock;
        }
    }
}

/***************************************************************************/
/* function create new node in linked list of process*/
_PROCINTNODE* _procintcreateNewNode(int process, int size)
{
    _PROCINTNODE *newNode = NULL;
    newNode=(_PROCINTNODE*)malloc(sizeof(_PROCINTNODE));
    if(newNode == NULL) {
        printf("Memory allocation error\n");
        return NULL;
    }
    newNode->process = process;
    newNode->size = size;
    newNode->next = NULL;
    return newNode;
}

_PROCINTNODE* _procintinsertAtEnd(int process, int size, _PROCINTNODE **head)  
{
    _PROCINTNODE *current = *head;
    if (current == NULL) {
        *head = _procintcreateNewNode(process, size);
    }else{
        while(current->next != NULL){
            current = current->next;
        }
        current->next = _procintcreateNewNode(process, size);
    }
    return *head;
}

_PROCINTNODE* InsertData(int Data[], int length, _PROCINTNODE** listData)
{
    for (int i = 0; i < length; i++)
    {
        *listData = _procintinsertAtEnd(i + 1, Data[i], &(*listData));
    }
    return *listData;
}

/***************************************************************************/
/* function create new node of linked list _VIRTMEMPAGEBLOCKS*/
_VIRTMEMPAGEBLOCKS* _virtmempageblockscreateNewNode(int process, size_t size, int pagenumber, void *memoryaddress, int percent)
{
    _VIRTMEMPAGEBLOCKS *newNode = NULL;
    newNode=(_VIRTMEMPAGEBLOCKS*)malloc(sizeof(_VIRTMEMPAGEBLOCKS));
    if(newNode == NULL) {
        printf("Memory allocation error\n");
        return NULL;
    }
    newNode->process = process;
    newNode->size = size;
    newNode->pagenumber=pagenumber;
    newNode->memoryaddress=memoryaddress;
    newNode->next = NULL;
    newNode->percent = percent;
    return newNode;
}

// linked list of virtual memory block
_VIRTMEMPAGEBLOCKS* _virtmempageblocksinsertAtEnd(int process,size_t size,int pagenumber,
                                                void *memoryaddress, _VIRTMEMPAGEBLOCKS **head, int percent)
{
    _VIRTMEMPAGEBLOCKS *current = *head;
    if (current == NULL) {
        *head = _virtmempageblockscreateNewNode(process, size, pagenumber, memoryaddress, percent);
    }else{
        while(current->next != NULL){
            current = current->next;
        }
        current->next = _virtmempageblockscreateNewNode(process, size, pagenumber, memoryaddress, percent);
    }
    return *head;
}

// divide process memory into pages (4KB)
// divide physical / virtual memory to frame (size PAGE_MEM_ADDR_SIZE)
void divideProc_Mem_IntoPageBlocks(_PROCINTNODE* procinthead, _VIRTMEMPAGEBLOCKS** virtmempageblockshead,
    _MEMPAGEBLOCKS** mempageblocksHead)
{
    _PROCINTNODE* current = procinthead;
    unsigned int pagenumber = 0;
    int percent = 0;
    void* address = (void*)0x12345678;

    while (current != NULL) {
        if (current->size <= PAGE_SIZE) {
            percent = (int)(current->size*100/ PAGE_SIZE);
            printf("\n Page %u da dung het : %d %% ",pagenumber,percent);
            _virtmempageblocksinsertAtEnd(current->process, PAGE_SIZE, pagenumber, address, &(*virtmempageblockshead), percent); (virtual)
            current = current->next;
            pagenumber++;
            address += PAGE_MEM_ADDR_SIZE;
        }
        else {
            percent = 100;
            _virtmempageblocksinsertAtEnd(current->process, PAGE_SIZE, pagenumber, address, &(*virtmempageblockshead), percent);         
            current->size = (current->size - PAGE_SIZE);
            pagenumber++;
            address += PAGE_MEM_ADDR_SIZE;
        }
    }

    for (int i = 0; i < MAX_PAGES; i++) {
        allocateNextMemPageBlock(PAGE_MEM_ADDR_SIZE, &(*mempageblocksHead)); //physical (frame)
    }
}

///////////////////////////////////////////////////////////////////////////////////
/* map virtual address to Physical address in paging*/
void mapVirtPhyAddressPageTable(_VIRTMEMPAGEBLOCKS** virtmempageblockshead, _MEMPAGEBLOCKS** mempageblocksHead)
{
    _MEMPAGEBLOCKS* currentmem = *mempageblocksHead;
    _VIRTMEMPAGEBLOCKS* currentvirt = *virtmempageblockshead;
    int count = 0;

    printf("\n[ Memory mapped Virtual/Physical Page Table ]\n");
    printf("\n--------------------------------------------------------------------------------------------------------------");
    printf("\n|    Process    |    PageNumber    |    VirtualAddress    |       PhysicalAddress      |       Percent       |\n");
    printf("--------------------------------------------------------------------------------------------------------------\n");

    while (currentmem != NULL) {
        if (currentmem->isfree) {
            printf("|       P%d      |       %3d        |     %p       |        %p      |        %3d %%        |\n",
                currentvirt->process, currentvirt->pagenumber, currentvirt->memoryaddress, currentmem->memoryaddress, currentvirt->percent);
            currentmem->isfree = false;
            count++;
            if (count == 256) {
                printf("***- 1MB block of pages loaded -***\n");
                count = 0;
            }
        }
        currentvirt = currentvirt->next;
        currentmem = currentmem->next;

        if (currentvirt == NULL) {
            goto exitLoop;
        }
    }

    printf("-----------------------------------------------------------------------------------\n");
    //free the allocated memory(_MEMPAGEBLOCKS)
    currentmem = *mempageblocksHead;
    while (currentmem != NULL) {
        if (!(currentmem->isfree)) {
            currentmem->isfree = true;
            printf("Memory Frame freed, Address = %p\n", currentmem->memoryaddress);
        }
        currentmem = currentmem->next;
    }

    //call again function to complete all processes
    mapVirtPhyAddressPageTable(&currentvirt, &(*mempageblocksHead));

exitLoop:

    //free the allocated memory(_MEMPAGEBLOCKS)
    currentmem = *mempageblocksHead;
    while (currentmem != NULL) {
        if (!(currentmem->isfree)) {
            currentmem->isfree = true;
            printf("Memory Frame freed, Address = %p\n", currentmem->memoryaddress);
        }
        currentmem = currentmem->next;
    }
}

//cap phat bo nho theo cac page
void AllocatePAGING(_SBLOCK* s_blockHead, _VIRTMEMPAGEBLOCKS* virtmempageBlocks,
    _MEMPAGEBLOCKS* mempageBlocks, _PROCINTNODE* procintHead)
{
    divideProc_Mem_IntoPageBlocks(procintHead, &virtmempageBlocks, &mempageBlocks);

    printf("\n\n\t\t[ Allocate memory using Paging ]\n\n");

    mapVirtPhyAddressPageTable(&virtmempageBlocks, &mempageBlocks);
}


int main()
{
    int proc_num, i = 0;
    int *Data = NULL;
    printf("Enter process number you want : ");
    scanf("%d",&proc_num);
    Data =  (int *)calloc(proc_num, sizeof(int));
    if (Data == NULL)
    {
        printf("No more memory to allocate!!!");
        exit(0);
    }
    else
    {
        for ( i = 0; i < proc_num; i++)
        {
            printf("Kich thuoc tien trinh %d : ",i+1);
            scanf("%d",&Data[i]);
        }  
        _PROCINTNODE* listData = NULL;
        _SBLOCK* sBlock = NULL;
        _VIRTMEMPAGEBLOCKS* virtmempageBlocks = NULL;
        _MEMPAGEBLOCKS* mempageBlocks = NULL;
        listData = InsertData(Data, proc_num, &listData);
        AllocatePAGING(sBlock, virtmempageBlocks, mempageBlocks, listData);
    }
    free(Data);
    return 0;
}



