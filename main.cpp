#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include <zlib.h>
#include <assert.h>

#include <list>

// Local includes
#include "nbt.h"

#define getBlock(blockThing, x,y,z) blockThing[ y + ( z * 128 + ( x * 16 * 16) ) ]


void updateBlockCounts(const char *path, char *table) {
    gzFile dat = gzopen(path,"rb");
    if (dat == NULL) {
        printf("failed to open file: %s\n", path);
        perror("gzopen");
        exit(1);
    }
    
    NBT_Compound root(dat); 
    
    NBT_Byte_Array *blocks = (NBT_Byte_Array*)root.findByName("Blocks", 1);
    assert(blocks != NULL);

    // print the first level
    for (int y = 0; y < 128; y++)  {
        
        uint64_t (*blockCounts)[256] = (uint64_t(*)[256])(table + (y * 256 * sizeof(uint64_t)));

        for (int x = 0; x < 16; x++) {
            for (int z = 0; z < 16; z++) {
                //printf("%02hhx ", getBlock(blocks->buff, x, y, z));

                unsigned char blockID = getBlock(blocks->buff, x, y, z);
                //printf("%d, %d, %d  has type/%d\n", x, y, z, blockID);
                (*blockCounts)[blockID]++;
                }
        }
    }

    gzclose(dat);

}


void printReport(const char *path, char*table) {

    FILE *f = fopen(path, "w");
    fprintf(f, "var rawdata=[];\n\n");

    for (int blockType = 0; blockType < 100; blockType++) {

        fprintf(f, "rawdata[%d] = [", blockType);
        for (int y = 0;  y < 128; y++) {
            uint64_t (*blockCounts)[256] = (uint64_t(*)[256])(table + (y * 256 * sizeof(uint64_t)));
            uint64_t count = (*blockCounts)[blockType];
            //fprintf(stderr, "%u: %lld\n", y, count);
            fprintf(f, "%lld, ", count);
        }
        fprintf(f,"];\n");

    }
    fclose(f);
}


int main(int argc, char **argv) {

    fprintf(stderr, "hello world\n");

    // this can hold blockcounts:
    //    uint64_t blockCounts[256];
    // We need 128 of these, hold block counts for each level
    const size_t toMalloc = 256*sizeof(uint64_t) * 128;
    //                      [ table size       ] * [num tables]
    fprintf(stderr, "mallocing %d bytes of memory to block tables... ", toMalloc);
    char * blockTable = (char*)malloc(toMalloc);
    fprintf(stderr, "ok.\n");
    fprintf(stderr, "clearing... ");
    memset(blockTable, 0, toMalloc);
    fprintf(stderr, "ok.\n");

   

    unsigned int numChunks = 0;
    for (int x = 1; x < argc; x++)  {
        fprintf(stderr,".");
        updateBlockCounts(argv[x], blockTable);
        numChunks++;
        fflush(stderr);
    }
    //updateBlockCounts(argv[2], &blockCounts);

    fprintf(stderr, "\n-- REPORT --\n");
    fprintf(stderr, "Read %u chunks\n", numChunks);

    printReport("data.js", blockTable); 
    
    /*  Test code:  TODO, move this elsewhere

    printf("read in %d bytes: %d\n", r, (unsigned int)(*buf));

    char t[256] = "\x00\x00\x00\x01\x00\x09\x00\x05hello\x00\x00\x00\x06\x01\x02\x03\x04\x05\x06\x12"; 
    char *test = t;

    NBT_Int i(test);
    printf("int val=%d\n", i.getValue());
    assert(i.getValue() == 1);
    test += i.size;

    NBT_Short s(test);
    printf("short val=%d\n", s.getValue());
    assert(s.getValue() == 9);
    test+=s.size;

    NBT_String str(test);
    printf("str val=%s\n", str.getString().c_str());
    assert(strncmp(str.getString().c_str(), "hello", 5) == 0);
    test += str.size;
    assert(str.size == 7);

    NBT_Byte_Array ba(test);
    assert(ba.size == (4+6));
    assert(ba.length() == 6);
    assert(memcmp(ba.buff, "\x01\x02\x03\x04\x05\x06", 6) == 0);
    test += ba.size;
        
    NBT_Byte b(test);
    assert(b.getByte() == 0x12);
    assert(b.size == 1);
    test += b.size;


    char t2[256] = "\x09\x00\x06mylist\x02\x00\x00\x00\x04\x00\x01\x00\x02\x00\x03\x00\x04";

    test = t2;
    test += 1; 
    NBT_String t_name(test);
    test += t_name.size;

    printf("name: %s\n", t_name.getString().c_str());

    NBT_List l(test);
    assert(l.getListType() == TAG_Short);
    assert(l.getList().size() == 4);

    printf("start Compounttest:\n");

    char t3[256] = "\x01\x00\x04\x42yte\xab\x03\x00\x05myint\x00\x00\x00\x08\x02\x00\x07myshort\x00\xff\x00";
    test = t3;
    NBT_Compound cmp(test);


    printf("real data test\n----\n");

    test = buf;
    test++;
    NBT_String buf_string(test);
    test += buf_string.size;
    printf("compound name: %s\n", buf_string.getString().c_str());

    NBT_Compound buf_compound(test);
    buf_compound.name = buf_string.getString();
    printf("\n\n");

    buf_compound.print(0);

    list<NBT_Tag*> listlist = buf_compound.getList();


    //t->print();
    //free(testbuf);
    */
    return 0;
}
