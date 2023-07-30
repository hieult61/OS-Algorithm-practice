#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

//maximum memory size 1MB
#define MAX_MEM_SIZE (1024 * 1024)
#define MAX_SEGMENT_SIZE 256000 //256 KB
#define BLOCK_SIZE sizeof(_SBLOCK)
#define MEM_SEGMENT_BLOCK_SIZE sizeof(_MEMSEGMENTBLOCKS)


int segmentnumber = 0;

//structure for memory block
typedef struct __s_block
{
    struct __s_block *next;
    struct __s_block *prev;
    bool isfree;
    size_t size;
} _SBLOCK;

//structure for allocate memory using segmentation
typedef struct __memsegmentblocks
{
    struct __memsegmentblocks *next;
    bool isfree;
    size_t size;
    void *memoryaddress;
} _MEMSEGMENTBLOCKS;

//structure to store process
typedef struct __procintnode
{
    int process;
    int size;
    struct __procintnode *next;
} _PROCINTNODE;

/////////////////////////////////////////////////////////////////
/*process function*/
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

_PROCINTNODE *_procintinsertAtEnd(int process, int size, _PROCINTNODE **head)
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

_PROCINTNODE *InsertData(int Data[], int length, _PROCINTNODE **listData)
{
    for (int i = 0; i < length; i++)
    {
        *listData = _procintinsertAtEnd(i + 1, Data[i], &(*listData));
    }
    return *listData;
}

int _countPROCINTNodes(_PROCINTNODE *prochead)
{
    int count = 0;
    while (prochead != NULL)
    {
        count++;
        prochead = prochead->next;
    }
    return count;
}

//////////////////////////////////////////////////////////////////
void _deleteProcINTNode_ZeroData(_PROCINTNODE **procinthead) //delete node size 0
{
    _PROCINTNODE *current = *procinthead;
    _PROCINTNODE *temp = *procinthead;

    if (current == NULL)
    {
    }
    else
    {
        if (_countPROCINTNodes(current) == 1)
        {
            if (current->size == 0)
            {
                *procinthead = NULL;
                free(current);
            }
        }
        else
        {
            while (current != NULL)
            {
                if (current->size == 0)
                {
                    temp->next = current->next;
                    free(current);
                    current = temp->next;
                }
                else
                {
                    temp = current;
                    current = current->next;
                }
            }
        }
    }
    current = *procinthead;
    if (current->size == 0)
    {
        *procinthead = current->next;
    }
}
//////////////////////////////////////////////////////////////////
//allocate memory block for Segmentation
_MEMSEGMENTBLOCKS *allocateMemSegmentBlock(size_t size)
{
    _MEMSEGMENTBLOCKS *block = sbrk(0);
    if (sbrk(MEM_SEGMENT_BLOCK_SIZE + size) == (void *)-1)
    {
        return NULL;
    }
    else
    {
        sbrk(MEM_SEGMENT_BLOCK_SIZE + size);
        block->next = NULL;
        block->isfree = false;
        block->size = size;
        block->memoryaddress = sbrk(0);
        return block;
    }
}

void allocateNextMemSegmentBlock(size_t size, _MEMSEGMENTBLOCKS **head)
{
    _MEMSEGMENTBLOCKS *current = *head;
    void *allocate_mem = NULL;
    if (current == NULL)
    {
        *head = allocateMemSegmentBlock(size);
    }
    else
    {
        while (current->next != NULL)
        {
            current = current->next;
        }
        _MEMSEGMENTBLOCKS *newblock = sbrk(0);

        allocate_mem = sbrk(MEM_SEGMENT_BLOCK_SIZE + size);
        if (allocate_mem == (void *)-1)
        {
        }
        else
        {
            sbrk(MEM_SEGMENT_BLOCK_SIZE + size);
            newblock->next = NULL;
            newblock->isfree = false;
            newblock->size = size;
            newblock->memoryaddress = sbrk(0);
            current->next = newblock;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
// lay start address cua bo nho trong cho phan doan tiep theo cua process
void getFreeMemoryAddress(_MEMSEGMENTBLOCKS *memSegBlockshead, unsigned int size, void **start_address) //start address
{
    _MEMSEGMENTBLOCKS *current = memSegBlockshead;

    int size_temp = current->size;

    while (current != NULL)
    {
        if (current->isfree)
        {
            if (size_temp >= size)
            {
                *start_address = current->memoryaddress;
                size_temp = 0;
                break;
            }
            size_temp += current->size;
            current = current->next;
        }
        else
        {
            current = current->next;
        }
    }
}
// cap phat bo nho cho phan doan con lai
void allocateMemory_using_Segment_remain(_PROCINTNODE **procinthead, _MEMSEGMENTBLOCKS **memSegBlockshead)
{
    _PROCINTNODE *current = *procinthead;
    int total_size = 0;
    _MEMSEGMENTBLOCKS *segcurrent = *memSegBlockshead;

    //free the used memory
    while (segcurrent != NULL)
    {
        segcurrent->isfree = true;
        segcurrent = segcurrent->next;
    }

    segcurrent = *memSegBlockshead;
    void *start_address;

    while (current != NULL)
    {
        if (current->size <= MAX_SEGMENT_SIZE)
        {
            if (total_size + current->size < MAX_MEM_SIZE)
            {

                getFreeMemoryAddress(*memSegBlockshead, current->size, &start_address);

                void *end_address = start_address + current->size;

                total_size += current->size;

                printf("\n|     P%d     | %p         | %p        |       %d       |",
                       current->process, start_address, end_address, segmentnumber);

                printf("     %d      \n", total_size);

                current->size = 0;
                current = current->next;
                segmentnumber++;
            }
            else
            {
                total_size = 0;
                break;
            }
        }
        else
        {
            if (total_size + MAX_SEGMENT_SIZE < MAX_MEM_SIZE)
            {

                if (segcurrent->next == NULL)
                    segcurrent = *memSegBlockshead;
                else
                    segcurrent = segcurrent->next;

                getFreeMemoryAddress(segcurrent, MAX_SEGMENT_SIZE, &start_address);

                void *end_address = start_address + MAX_SEGMENT_SIZE;

                printf("\n|     P%d     | %p         | %p        |       %d       |",
                       current->process, start_address, end_address, segmentnumber);

                total_size += MAX_SEGMENT_SIZE;

                printf("     %d      \n", total_size);

                current->size = current->size - MAX_SEGMENT_SIZE;
                segmentnumber++;
            }
            else
            {
                total_size = 0;
                break;
            }
        }
    }
}
// cap phat bo nho cho phan doan
void allocateMemory_using_Segmentation(_PROCINTNODE **procinthead, _MEMSEGMENTBLOCKS **memSegBlockshead)
{
    _PROCINTNODE *current = *procinthead;
    int total_size = 0; // total size mem for allocated by seg

    printf("\nsbrk(0) = %p\n", sbrk(0)); // allocate memory for proc - sbrk

    printf("-------------------------------------------------------------------------------------------------");
    printf("\n|  Process   |     Start Address      |      End Address      | Segment Number|    Total Size   \n");
    printf("-------------------------------------------------------------------------------------------------\n");

    while (current != NULL)
    {
        if (current->size <= MAX_SEGMENT_SIZE) // if size proc <= max   
        {
            if (total_size + current->size < MAX_MEM_SIZE)
            {
                void *start_address = sbrk(0) + 1;
                allocateNextMemSegmentBlock(current->size + 1, &(*memSegBlockshead));
                void *end_address = sbrk(0);

                total_size += current->size;

                printf("\n|     P%d     | %p         | %p        |       %d       |",
                       current->process, start_address, end_address, segmentnumber);

                printf("     %d      \n", total_size);

                current->size = 0;
                current = current->next;
                segmentnumber++;
            }
            else
            {
                total_size = 0;
                break;
            }
        }
        else
        {
            if (total_size + MAX_SEGMENT_SIZE < MAX_MEM_SIZE)
            {
                void *start_address = sbrk(0) + 1;
                allocateNextMemSegmentBlock(MAX_SEGMENT_SIZE, &(*memSegBlockshead));
                void *end_address = sbrk(0);
                total_size += MAX_SEGMENT_SIZE;

                printf("\n|     P%d     | %p         | %p        |       %d       |",
                       current->process, start_address, end_address, segmentnumber);

                printf("     %d      \n", total_size);

                current->size = current->size - MAX_SEGMENT_SIZE;
                segmentnumber++;
            }
            else
            {
                total_size = 0;
                break;
            }
        }
    }

    //delete memory nodes of size 0 or used
    current = *procinthead;
    _deleteProcINTNode_ZeroData(&current);
    current = *procinthead;

    if (_countPROCINTNodes(current) == 1)
    {
        if (current->size == 0)
        {
            *procinthead = NULL;
        }
    }
    else
    {
        *procinthead = (*procinthead)->next;
    }

    //call function to continue process to all processes
    allocateMemory_using_Segment_remain(&(*procinthead), &(*memSegBlockshead));
}

void AllocateSEGMENTATION(_SBLOCK *s_blockHead, _MEMSEGMENTBLOCKS *memSegBlocksHead, _PROCINTNODE *procintHead)
{

    printf("\n\n\t[ Allocate memory using Segmentation ]\n");
    printf("____________________________________________________________________________");
    do
    {
        if (procintHead->next == NULL)
        {
            if (procintHead->size == 0)
                break;
        }
        else
        {
            _deleteProcINTNode_ZeroData(&procintHead);
            allocateMemory_using_Segmentation(&procintHead, &memSegBlocksHead);
        }
    } while (procintHead != NULL);
}


int main()
{
    int proc_num, i = 0;
    int *Data = NULL;
    printf("Enter process number you want : ");
    scanf("%d", &proc_num);
    Data = (int *)calloc(proc_num, sizeof(int));
    if (Data == NULL)
    {
        printf("No more memory to allocate!!!");
        exit(0);
    }
    else
    {
        for (i = 0; i < proc_num; i++)
        {
            printf("Size of process %d : ", i + 1);
            scanf("%d", &Data[i]);
        }
        _PROCINTNODE *listData = NULL;
        _SBLOCK *s_blockHead = NULL;
        _MEMSEGMENTBLOCKS *memSegBlocksHead = NULL;
        
        listData = InsertData(Data, proc_num, &listData);
        AllocateSEGMENTATION(s_blockHead, memSegBlocksHead, listData);
    }
    free(Data);
    return 0;
}
