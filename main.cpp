#include <stdio.h>
#include <iostream>
#include <stdlib.h>

#include <zlib.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>


#include <list>

// Local includes
#include "nbt.h"

#define getBlock(blockThing, x,y,z) blockThing[ y + ( z * 128 + ( x * 128 * 16) ) ]

// forwards
void updateBlockCounts(z_streamp dat, char *table);


typedef struct gz_stream {
    z_stream stream;
    int      z_err;   /* error code for last stream operation */
    int      z_eof;   /* set if end of input file */
    FILE     *file;   /* .gz file */
    Byte     *inbuf;  /* input buffer */
    Byte     *outbuf; /* output buffer */
    uLong    crc;     /* crc32 of uncompressed data */
    char     *msg;    /* error message */
    char     *path;   /* path name for debugging only */
    int      transparent; /* 1 if input file is not a .gz file */
    char     mode;    /* 'w' or 'r' */
#ifdef _LARGEFILE64_SOURCE
    off64_t  start;   /* start of compressed data in file (header skipped) */
    off64_t  in;      /* bytes into deflate or inflate */
    off64_t  out;     /* bytes out of deflate or inflate */
#else
    z_off_t  start;   /* start of compressed data in file (header skipped) */
    z_off_t  in;      /* bytes into deflate or inflate */
    z_off_t  out;     /* bytes out of deflate or inflate */
#endif
    int      back;    /* one character push-back */
    int      last;    /* true if push-back is last character */
} gz_stream;


void readRegion(const char *path, char *table) {

    FILE *f = fopen(path, "rb");
    if (!f) {
        perror("fopen");
        exit(1);
    }



    for (int x = 0; x < 32; x++) {
        for (int z = 0; z < 32; z++) {
            // TODO speed this up by reading linearly and deriving x and z from offset
            unsigned int offset = 4 * ((x % 32) + (z % 32) * 32);
            //  seek into the locations field
            fseek(f, offset, SEEK_SET);

            uint32_t s=0;
            // read 3 bytes
            fread(((char*)&s)+1, 1, 3, f);
            uint32_t chunk_loc = ntohl(s);

            char sec_count;
            fread(&sec_count, 1, 1, f);

            if (chunk_loc > 0 && sec_count > 0) {
                //fprintf(stderr, "location offset for %d,%d: %u\tsec_count: %hhu\n", x, z, chunk_loc, sec_count);
                fseek(f, 4096 * chunk_loc, SEEK_SET);
                // read in 4 bytes for the size:
                fread((char*)&s, 1, 4, f);
                uint32_t chunk_size = ntohl(s);

                // read in 1 byte for the compression type
                char compress_type;
                fread(&compress_type, 1, 1, f);
                assert(compress_type == 1 || compress_type == 2);
                //fprintf(stderr, "  chunk has size %d bytes \t compressed with %hhu\n", chunk_size, compress_type);
                
                // read in compressed data
                unsigned char *data = (unsigned char*)malloc(chunk_size);
                fread(data, 1, chunk_size, f);

                z_streamp dfs = (z_streamp)malloc(sizeof(z_stream));
                memset(dfs, 0, sizeof(z_stream));
                dfs->next_in = data;
                dfs->avail_in = chunk_size;
                assert(inflateInit(dfs) == Z_OK);
                
                unsigned char abyte;
                dfs->next_out = &abyte;
                dfs->avail_out = 1;
                assert(dfs->total_out == 0);

                updateBlockCounts(dfs, table);

                assert(inflateEnd(dfs) == Z_OK);
                free(dfs);
                free(data);



            }



        }
    }
    fclose(f);
    return;
}


void updateBlockCounts(z_streamp dat, char *table) {

    
    NBT_Compound root(dat); 
    
    NBT_Byte_Array *blocks = (NBT_Byte_Array*)root.findByName("Blocks", 1);
    assert(blocks != NULL);

    // print the first level
    for (int y = 0; y < 128; y++)  {

        // get the table for this level        
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


}


void printReport(const char *path, char*table) {


    // material distribution by level, for each material

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


    // material distribution by material, for each level
    f = fopen("data2.js", "w");
    fprintf(f, "var leveldata=[];\n\n");

    for (int y = 0; y < 128; y++) {
        uint64_t (*blockCounts)[256] = (uint64_t(*)[256])(table + (y * 256 * sizeof(uint64_t)));
        // for this level, produce sums for each material
        //uint64_t sums[256] = {0};

        fprintf(f, "leveldata[%d] = [", y);
        for(unsigned int blockType = 0; blockType < 100; blockType++) {
            fprintf(f, "{blockType: %u, count: %lld}, ", blockType, (*blockCounts)[blockType]);
        }
        fprintf(f, "];\n");

    }
    

    fclose(f);

}

void usage() {
    printf("\n");
}

void walkDir(const char*path, char*table) {
    // find 
    DIR *root = opendir(path);
    char nextdir[256];
    char fullname[256];
    struct dirent *entry;
    if (root == NULL) {
        fprintf(stderr, "Failed to opendir(%s): %s\n\n", path, strerror(errno));
        exit(1);
    }
    while((entry = readdir(root)) != NULL) {
        if (entry->d_type == 4 && entry->d_name[0] != '.') {
            sprintf(nextdir, "%s/%s", path, entry->d_name);
            walkDir(nextdir, table);
        } else if (entry->d_type == 8 && entry->d_name[0] == 'r' && entry->d_name[1] == '.') {
            sprintf(fullname, "%s/%s", path, entry->d_name);
            fprintf(stderr, ".");
            fflush(stderr);
            readRegion(fullname, table);

        }

    }
    closedir(root);

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

    walkDir(argv[1], blockTable);


    fprintf(stderr, "\n-- REPORT --\n");
    //fprintf(stderr, "Read %u chunks\n", numChunks);

    printReport("data.js", blockTable); 

    free(blockTable);
    
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
