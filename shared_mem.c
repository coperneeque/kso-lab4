/************************************************************************
 * Mikolaj Panka                                                        *
 * KSO 2021-z                                                           *
 * lab3                                                                 *
 ************************************************************************/
#include "shared_mem.h"

#include <errno.h>
#include <stdio.h>
#include <sys/shm.h>

#include "test_flags.h"

int getMemBlock(char* path, int proj_id, size_t size)
{
    key_t blockKey = ftok(path, proj_id);

    if (blockKey == -1)
    {
            #ifdef MP_DEBUG
        perror("[ error ] getMemBlock(): ftok() failed on mem.txt");
            #endif
        return -1;
    }
    
    int blockId = shmget(blockKey, size, 0644 | IPC_CREAT);
    if (blockId == -1)
    {
            #ifdef MP_DEBUG
        perror("[ error ] getMemBlock(): shmget() failed");
            #endif
        return -1;
    }

    return blockId;
}

void* attachMemBlock(int block_id)
{
    void* ret = (void*)0;

    ret = shmat(block_id, (void*)0, 0);
        #ifdef MP_DEBUG
    if (ret == (void*)0)
    {
        perror("[ error ] attachMemBlock(): shmat() failed");
    }
        #endif

    return ret;
}
