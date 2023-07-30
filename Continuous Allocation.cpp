#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#define BLOCK_SIZE sizeof(_SBLOCK)

// maximum memory size 1KB
#define MAX_MEM_SIZE (1024) 

// structure of a memory block
typedef struct __s_block
{
    struct __s_block *next;
    struct __s_block *prev;
    bool isfree;
    uint64_t size;
    int process;
} _SBLOCK;


// structure of process 
typedef struct __procintnode
{
    int process;
    int size;
    struct __procintnode *next;
} _PROCINTNODE;

// check memory is enough or not
bool _isMemoryEnough(int requiredMem)
{
    return (requiredMem < MAX_MEM_SIZE ? true : false);
}

int getProcessMemSizeSum(_PROCINTNODE *procinthead)
{
    int sum = 0;
    while (procinthead != NULL)
    {
        if (procinthead->process)
        {
            sum += procinthead->size;
        }
        procinthead = procinthead->next;
    }
    return sum;
}

// allocate memory block
_SBLOCK *allocateMemBlock(uint64_t size, bool check, int process)
{
    _SBLOCK *block = sbrk(0);   //get the current program break location , argument means size of memory added
    if (sbrk(BLOCK_SIZE + size) == (void *)-1)  //-1 pointer, invalid pointer 
    {
        return NULL;
    }
    else
    {
        block->process = process;
        block->next = NULL;
        block->prev = NULL;
        block->isfree = check;
        block->size = size;
        return block;
    }
}


void allocateNextMemBlock(size_t size, _SBLOCK **head, bool check, int process)
{
    _SBLOCK *current = *head;
    void *allocate_mem = NULL;
    if (current == NULL)
    {
        *head = allocateMemBlock(size, check, process);
    }
    else
    {
        while (current->next != NULL)
        {
            current = current->next;
        }
        _SBLOCK *newblock = sbrk(0);

        allocate_mem = (void *)(BLOCK_SIZE + size);
        if (allocate_mem == (void *)-1)
        {
        }
        else
        {
            sbrk(BLOCK_SIZE + size);
            newblock->process = process;
            newblock->next = NULL;
            newblock->prev = current;
            newblock->isfree = check;
            newblock->size = size;
            current->next = newblock;
        }
    }
}

int _countPROCINTNodes(_PROCINTNODE *prochead) //number of process
{
    int count = 0;
    while (prochead != NULL)
    {
        count++;
        prochead = prochead->next;
    }
    return count;
}

void showTotalMemory(_SBLOCK *sBlock) 
{
    _SBLOCK *current = sBlock;
    char clear = ' ', use = '*';

    printf("\nMemory status:\n[|");
    while (current != NULL)
    {
        float c = (float) ((float)(current->size) / MAX_MEM_SIZE * 100); // % of total memory
        printf("(P%d)%.2f%%", current->process, c);
        c /= 2;
        for (int i = 0; i <= c; i++)
        {
            if (current->isfree)
            {
                printf("%c", clear);
            }
            else
            {
                printf("%c", use);
            }
        }
        printf("|");
        current = current->next;
    }
    printf("]");
}

void releaseAllBlock(_SBLOCK **blockHead)
{
    // release the allocated memory
    printf("\n[ Memory released ]");
    _SBLOCK *lastblock, *blockcurrent = *blockHead;
    while (blockcurrent != NULL)
    {
        blockcurrent->isfree = true;
        if (blockcurrent->next == NULL)
        {
            lastblock = blockcurrent;
        }
        blockcurrent = blockcurrent->next;
    }

    blockcurrent = lastblock;
    while (blockcurrent->prev != NULL)
    {
        if (blockcurrent->isfree)
        {
            sbrk(0 - (BLOCK_SIZE + blockcurrent->size));
            (blockcurrent->prev)->next = NULL;
        }
        blockcurrent = blockcurrent->prev;
    }

    int start_block = blockcurrent->process;
    sbrk(0 - (BLOCK_SIZE + blockcurrent->size));
    printf("\n[ Release done! ]\n");
    *blockHead = NULL;
}

void allocate_allMemory(_SBLOCK **blockHead, _PROCINTNODE *procinthead) // continuous allocation
{
    _PROCINTNODE *current = procinthead;

    printf("\n\t\t[ All Memory allocated to processes]\n");
    printf("---------------------------------------------------------------------------------");
    printf("\n|  Process   |     Start Address      |      End Address      |      Size       |\n");
    printf("---------------------------------------------------------------------------------\n");

    void *start_Memory = sbrk(0);
    int total_Memory = 0;
    while (current != NULL)
    {
        void *start_address = sbrk(0);
        total_Memory += current->size;
        allocateNextMemBlock(current->size, &(*blockHead), !(bool)(current->process), current->process);
        void *end_address = sbrk(0) - 1;
        float size_kb = (current->size) / 1024.0;

        printf("|     P%d     |     %p     |    %p     |     %.3fKB     |\n",
               current->process, start_address, end_address, size_kb);
        current = current->next;
    }
    printf("\nCurrent brk pointer is here (sbrk(0)) = %p\n", sbrk(0));
    showTotalMemory(*blockHead);
    printf("\n\n-------------------------------------------------------------------------");
}

void AllocateALLMemory(_SBLOCK **s_blockHead, _PROCINTNODE *procintHead) 
{
    
    if (_isMemoryEnough(getProcessMemSizeSum(procintHead)))	// if memory is enough then call allocate_allMemory() otherwise exit
    {
        printf("\n\n\t -- Memory is enough to fulfill all processes --\n");
        allocate_allMemory(&(*s_blockHead), procintHead);
    }
    else
    {
        printf("\nError: Memory is not enough to fulfill all processes\nExiting...\n\n");
        exit(0);
    }
}

/* function create new node in linked list of process*/
_PROCINTNODE *_procintcreateNewNode(int process, int size)  
{
    _PROCINTNODE *newNode = NULL;
    newNode = (_PROCINTNODE *)malloc(sizeof(_PROCINTNODE));
    if (newNode == NULL)
    {
        printf("Memory allocation error\n");
        return NULL;
    }
    newNode->process = process;
    newNode->size = size;
    newNode->next = NULL;
    return newNode;
}


_PROCINTNODE *_procintinsertAtEnd(int process, int size, _PROCINTNODE **head)    //Insert process at the end
{
    _PROCINTNODE *current = *head;
    if (current == NULL)
    {
        *head = _procintcreateNewNode(process, size);
    }
    else
    {
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = _procintcreateNewNode(process, size);
    }
    return *head;
}

// Insert process 
_PROCINTNODE *InsertData(int Data[], int length, _PROCINTNODE **listData)  	//listData linkedlist of process
{
    int total_memory_data = 0;
    for (int i = 0; i < length; i++)
    {
        total_memory_data += Data[i];
        *listData = _procintinsertAtEnd(i + 1, Data[i], &(*listData));
    }
    if (total_memory_data < MAX_MEM_SIZE)
    {
        *listData = _procintinsertAtEnd(0, MAX_MEM_SIZE - total_memory_data, &(*listData));
    }
    return *listData;
}

_SBLOCK *deleteSBlock(_SBLOCK **sBlock, int process)   //xoa vung nho cua tien trinh da ket thuc
{
    _SBLOCK *current = *sBlock;
    _SBLOCK *copy = NULL;
    while (current != NULL)
    {
        if (current->process == process)
        {
            current->isfree = true;
            current->process = 0;
            break;
        }
        current = current->next;
    }
}

// Tim kiem sap xep bo nho trong cap phat lien tuc
void searchAllMemory(_SBLOCK *sBlock, int process, int size) //size cua tien trinh
{
    _SBLOCK *s1 = NULL,
            *s2 = NULL,
            *s3 = NULL,
            *current;
    int check1 = 0, check2 = 0, check3 = 0;

    // Check external fragmentation
    current = sBlock;
    int c1 = 1, totalMF = 0;
    while (current != NULL)
    {
        if (current->isfree)
        {
            totalMF += current->size;
            if (size <= current->size)  // can allocate space
            {
                c1 = 0;
            }
        }
        current = current->next;
    }

    if (c1)
    {
        if (size <= totalMF)
        {
            printf("\nTotal memory free: %d bytes", totalMF);
            printf("\n-----------External Fragmentation.----------\n");
        }
        else printf("\nMemory isn't enough.\n");
    }
    else
    {
        // First fit
        current = sBlock;
        while (current != NULL)
        {
            if (current->isfree && !check1)
            {
                if (size <= current->size) //process size < current memory block size
                {
                    check1 = 1;
                    allocateNextMemBlock(size, &s1, false, process);
                    allocateNextMemBlock(current->size - size, &s1, true, 0);   //vung nho con lai cua vung nho tro thanh khoang trong moi
                    current = current->next;
                    continue;
                }
            }
            allocateNextMemBlock(current->size, &s1, current->isfree, current->process);
            current = current->next;
        }
        if (check1)
        {
            printf("\n\nFirst-fit: New process was added.");
        }
        else
        {
            printf("\n\nNew process wasn't added.");
        }
        showTotalMemory(s1);
        releaseAllBlock(&s1);
        printf("\n-------------------------------------------------------------------------");

        // Best fit
        current = sBlock;
        int bestFit = MAX_MEM_SIZE, indBF, count = 0;
        while (current != NULL)
        {
            count++;
            if (current->isfree)
            {
                if (size <= current->size)
                {
                    check2 = 1;
                    if (bestFit > current->size) //best fit block
                    {
                        bestFit = current->size;
                        indBF = count;
                    }
                }
            }
            current = current->next;
        }
        if (check2)
        {
            printf("\n\nBest-fit: New process was added.");
            current = sBlock;
            while (--indBF)
            {
                allocateNextMemBlock(current->size, &s2, current->isfree, current->process);
                current = current->next;
            }
            allocateNextMemBlock(size, &s2, false, process);
            allocateNextMemBlock(current->size - size, &s2, true, 0);
            current = current->next;
            while (current != NULL)
            {
                allocateNextMemBlock(current->size, &s2, current->isfree, current->process);
                current = current->next;
            }
        }
        else
        {
            printf("\n\nNew process wasn't added.");
        }
        showTotalMemory(s2);
        releaseAllBlock(&s2);
        printf("\n-------------------------------------------------------------------------");

        // Worst fit
        current = sBlock;
        int worstFit = 0, indWF;
        count = 0;
        while (current != NULL)
        {
            count++;
            if (current->isfree)
            {
                if (size <= current->size)
                {
                    check3 = 1;
                    if (worstFit < current->size)  //worst fit block
                    {
                        worstFit = current->size;
                        indWF = count;
                    }
                }
            }
            current = current->next;
        }

        if (check3)
        {
            printf("\n\nWorst-fit: New process was added.");
            current = sBlock;
            while (--indWF)
            {
                allocateNextMemBlock(current->size, &s3, current->isfree, current->process);
                current = current->next;
            }
            allocateNextMemBlock(size, &s3, false, process);
            allocateNextMemBlock(current->size - size, &s3, true, 0);
            current = current->next;
            while (current != NULL)
            {
                allocateNextMemBlock(current->size, &s3, current->isfree, current->process);
                current = current->next;
            }
        }
        else
        {
            printf("\n\nNew process wasn't added.");
        }
        showTotalMemory(s3);
        releaseAllBlock(&s3);
        printf("\n-------------------------------------------------------------------------");
    }
}

int main()
{
    int Data[] = {0};
    int processEnd[] = {0};
    _PROCINTNODE *listData = NULL;
    _SBLOCK *sBlock = NULL;

    int numberProcess, numberProcessEnd, select, release = 0, sizeNP;
    printf("\nContiguousAllocation\n");

    while (true)
    {
        printf("\n1.Show current memory.\n2.End process.\n3.Add new process use First-fit/ Best-fit/ Worst-fit.\n4.Exit.\nSelect: ");
        scanf("%d", &select);
        if (select > 4 || select < 1)
            continue;
        if (select == 4)
            break;
        if (!release)
            release = 1;
        switch (select)
        {
        case 1:
            
            printf("\nEnter process number: ");
            scanf("%d", &numberProcess);

            for (int i = 0; i < numberProcess; i++)
            {
                printf("\nSize of process P%d:", i + 1);
                scanf("%d", &Data[i]);
            }

            listData = InsertData(Data, numberProcess, &listData);
            AllocateALLMemory(&sBlock, listData);
            break;

        case 2:
            
            printf("\nNumber process end: ");
            scanf("%d", &numberProcessEnd);
            for (int i = 0; i < numberProcessEnd; i++)
            {
                printf("\nProcess end: ");
                scanf("%d", &processEnd[i]);
            }

            for (int i = 0; i < numberProcessEnd; i++)
            {
                printf("\n\n+)Process %d end:", processEnd[i]);
                deleteSBlock(&sBlock, processEnd[i]);
                showTotalMemory(sBlock);
            }
            printf("\n\n-------------------------------------------------------------------------");
            break;

        case 3:
            printf("\nSize of new process: ");
            scanf("%d", &sizeNP);
            numberProcess++;
            searchAllMemory(sBlock, numberProcess, sizeNP);
            break;
        }
    }

    if (release)
    {
        releaseAllBlock(&sBlock);
    }

    return 0;
}
