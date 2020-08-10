//
//  main.cpp
//  cacheProject
//
//  Created by Khalid A. Mohamed on 7/12/19.
//  Copyright © 2019 Khalid A. Mohamed. All rights reserved.
//

#include <iostream>
#include  <iomanip>
#include <math.h>

using namespace std;

#define        DBG                1
#define        DRAM_SIZE        (64*1024*1024)
#define        CACHE_SIZE        (64*1024)

enum cacheResType {MISS=0, HIT=1};

/* The following implements a random number generator */
unsigned int m_w = 0xABCCAB55;    /* must not be zero, nor 0x464fffff */
unsigned int m_z = 0x05786902;    /* must not be zero, nor 0x9068ffff */
unsigned int rand_()
{
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;  /* 32-bit result */
}

unsigned int memGenA()
{
    static unsigned int addr=0;
    return (addr++)%(DRAM_SIZE/4);
}

unsigned int memGenB()
{
    static unsigned int addr=0;
    return  rand_()%(64*1024);
}

unsigned int memGenC()
{
    static unsigned int a1=0, a0=0;
    a0++;
    if(a0 == 512) { a1++; a0=0; }
    if(a1 == 128) a1=0;
    return(a1 + a0*128);
}

unsigned int memGenD()
{
    static unsigned int addr=0;
    return (addr++)%(1024*4);
}

unsigned int memGenE()
{
    static unsigned int addr=0;
    return (addr++)%(1024*16);
}

unsigned int memGenF()
{
    static unsigned int addr=0;
    return (addr+=64)%(128*1024);
}

unsigned int test1()
{
    static unsigned int addr;
    return (addr += 50) % DRAM_SIZE/14;
}

unsigned int test2()
{
    static unsigned int addr;
    return ((addr += rand_() + 321) % DRAM_SIZE/321);
}






// Direct Mapped Cache Simulator
cacheResType cacheSimDM(unsigned int addr, int blockSize, int TAG_addr, int index_addr, int cache[2 * 6][99999], int blocksCount, unsigned int &capacityMissCount, unsigned int &compulsoryMissCount, unsigned int &conflictMissCount, int iter)
{

    bool capacityMiss = true;

    
    // If the content in the index's address of the Array == the TAG address from the line, return HIT
    if ((cache[2*iter] [index_addr] == TAG_addr) && (cache[2*iter+1][index_addr] == 1))
        return HIT;
    
    // Else replace the current address with the index's address, check which type is the miss and return MISS
    else
    {
        cache[2*iter][index_addr] = TAG_addr;                       // Replacing the current line

        
        for (int i=0; i < blocksCount; i++)                         // If
        {
            if (cache[2*iter+1][i] != 1)
            {
                capacityMiss = false;
                i = blocksCount;
            }
        }

        if (capacityMiss)
            capacityMissCount++;
        else
        {
            if(cache[2*iter+1][index_addr] == 1)
                conflictMissCount++;
            else
            {
                compulsoryMissCount++;
            }
        }
        
        cache[2*iter+1][index_addr] = 1;
        return MISS;
    }

}


char *msg[2] = {"Miss","Hit"};

#define        function             memGenA()                  // Change for different tests
#define        NO_OF_Iterations      1000000


int main()
{
    // Using this iterator within arrays to access the 6 line sizes simutaneously (4, 8, 16, 32, 64, 128)
    int iter = 0;
    
    unsigned int capacityMissCount[6] = {0}, compulsoryMissCount[6] = {0}, conflictMissCount[6] = {0};
    unsigned int hit[6] = {0}, miss[6] = {0};
    float hitratio[6] = {0};                                  // To calculate the hit ratios
    
    
    cacheResType r;
    
    // The address, TAG, index, Amount of offset and index bits to be shifted and filtered.
    unsigned int addr, TAG_addr, index_addr, offsetBits, indexBits, blocksCount[6];
    
    cout << "Direct Mapped Cache Simulator\n";
    
    // Initializing the block size to start with 2 * 2 = 4, then blockSize *= 2
    int blockSize = 2;
    
    /* For simplicity, The Array containing the cache values (more optimum to be of pointers or a vector).
     Each row for a single line size contains only two columns, the first one for the address and the second one for the valid bit
     There is no data replacement/check in this project hence there is no need to reserve a space of the array for it.*/
    
    int cache[2 * 6] [99999];
    
    for (int i=0; i < 1; i++)                                  // Initializing the cache with -1
        for (int j=0; j < 99999; j++)
            cache[i][j]=-1;
    
    
        
        for(int inst=0; inst<NO_OF_Iterations; inst++)
        {
            // Resetting the block size to start each output line with blocksize = 4
            blockSize = 2;
            
            addr = function;                                    // The function to to generate the addresses (simulate an application)
            
            
            for (iter = 0; iter < 6; iter++)
            {
                
                blockSize *= 2;
                blocksCount[iter] = (CACHE_SIZE)/blockSize;
                
                offsetBits = log2(blockSize);                   // Determining the amount of the offset bits and index bits ...
                indexBits = log2(blocksCount[iter]);
                
                index_addr= (addr >> offsetBits) % blocksCount[iter];      // ... to shift in order to get the index & TAG addresses
                TAG_addr= addr >> (offsetBits + indexBits);
                
                r = cacheSimDM(addr, blockSize, TAG_addr, index_addr, cache, blocksCount[iter], capacityMissCount[iter], compulsoryMissCount[iter], conflictMissCount[iter], iter);
                
                if(r == HIT) hit[iter]++;
                else miss[iter]++;
                
                cout << "\t0x" << setfill('0') << setw(8) << hex << addr <<" ("<< msg[r] <<")\t" << '\t';
            }
        
        cout << endl;
    }
    cout << endl;
    
    
    for (iter = 0; iter < 6; iter++)
    {
        hitratio[iter] = 100*hit[iter]/NO_OF_Iterations;
        cout << setw(16) << setfill(' ')<< "Hit ratio = " << hitratio[iter] << '%' << "\t\t\t";
    }
    
    cout << "\n\n\n";
    
    blockSize = 2;
    for (iter = 0; iter < 6; iter++)
    {
        int lineSize = blockSize*=2;
        cout << "Line Size = " << lineSize << " Bytes" << endl;
        hitratio[iter] = 100*hit[iter]/NO_OF_Iterations;
        cout << "\tHit ratio = " << hitratio[iter] << '%' << endl;
        cout << "\tMiss ratio = " << 100 - hitratio[iter] << '%' << endl;
        cout << "\tHits = " << dec << hit[iter] << endl;
        cout << "\tMisses = " << dec << miss[iter] << endl;
        cout << "\tCompulsory Misses = " << dec << compulsoryMissCount[iter] << endl;
        cout << "\tCapcity Misses = " << dec << capacityMissCount[iter] << endl;
        cout << "\tConflict = " << dec << conflictMissCount[iter] << endl;
        cout << endl << endl;
        
    }
    
    return 0;
}
