#include <unordered_map>
#include <string.h>
#include <cstring>
using namespace std;

#ifndef MAXSCSI
#define MAXSCSI 24
#endif

#ifndef BLOCKSIZE
#define BLOCKSIZE 2048 /*1MB, 2048sectors*/
#endif

#ifndef EXPIREDAY
#define EXPIREDAY 7
#endif

#ifndef STARTTIME
#define STARTTIME 1514736000 /*2018-01-01 00:00:00*/
#endif

#ifndef BLOCKINFO
#define BLOCKINFO
struct BlockInfo
{
    u_int64_t readcnt;
};
#endif

#ifndef BLOCKSTRUCT
#define BLOCKSTRUCT
struct BlockStruct
{
    int io_type;
    int size;
    long alloc_time;
    char disksn[MAXSCSI];
    u_int64_t blockid;
    struct BlockStruct *next;
};
#endif

#ifndef RECORD
#define RECORD
struct IoRecord
{
    int io_type;
    int size;
    long alloc_time;
    char disksn[MAXSCSI];
    u_int64_t offset;
};
#endif

#ifndef MEMSTRUCT
#define MEMSTRUCT
class MemStruct
{
private:
    /*Time Oriented Table, i.e. TOT*/
    unordered_map<long,unordered_map<string,unordered_map<u_int64_t,struct BlockInfo * > * > * > *TOT;
    /*Index Map for TOT*/
    unordered_map<string, unordered_map<u_int64_t,long> *> *index_map;

    long currentday;
    long lastday;

public:

    MemStruct()
    {
        unordered_map<long, unordered_map<string, unordered_map<u_int64_t, struct BlockInfo *>* >* > *TOT = new unordered_map<long, unordered_map<string, unordered_map<u_int64_t, struct BlockInfo *>* >* >();
        unordered_map<string, unordered_map<u_int64_t, long>* > *index_map = new unordered_map<string, unordered_map<u_int64_t, long> *>();
        currentday = lastday = 0;
    }

    long hasBlock(struct BlockStruct *bs)
    {/*to judge whether the block is in the TOT*/ 
        auto it = (*index_map).find(bs->disksn);
        if(it != (*index_map).end())
        {
            unordered_map<u_int64_t,long> *tmp = &(it->second);
            auto it2= (*tmp).find(bs->blockid);
        }
        return 0;
    }

    struct BlockStruct * ioToBlock(struct IoRecord *ir)
    {/*将io记录转化为block链表，产生的block链表用freeBlockList进行回收*/
        struct BlockStruct *bstmp,*result,*curr;

        u_int64_t start = ir->offset / BLOCKSIZE;

        bstmp = (struct BlockStruct *)sizeof(struct BlockStruct);
        bstmp->blockid = start;
        bstmp->alloc_time = ir->alloc_time;
        bstmp->io_type = ir->io_type;
        bstmp->size = ir->size;
        strcpy(bstmp->disksn,ir->disksn);
        bstmp->next = NULL;
        curr = result = bstmp;
        u_int64_t end = (ir->offset + ir->size - 1) / BLOCKSIZE;

        if(start == end)
        {
            return result;
        }

        for(u_int64_t i = start+1 ; i <= end; i++)
        {
            bstmp = (struct BlockStruct *)sizeof(struct BlockStruct);
            bstmp->blockid = i;
            bstmp->alloc_time = ir->alloc_time;
            bstmp->io_type = ir->io_type;
            bstmp->size = ir->size;
            strcpy(bstmp->disksn, ir->disksn);
            bstmp->next = NULL;
            curr->next = bstmp;
            curr = bstmp;
        }
        return result;
    }

    void freeBlockList(struct BlockStruct *head)
    {/*回收block list*/
        struct BlockStruct *tmp = head;
        while(tmp != NULL)
        {
            head = tmp->next;
            free(tmp);
            tmp = head;
        }
    }

    void updateTOT(struct BlockStruct *bs)
    {
        long hourtmp = hasBlock(bs);
        if(hourtmp == 0)
        {/*this block is not exist in the TOT*/
            long hour = (bs->alloc_time - STARTTIME) / 3600;
        }
        else
        {

        }
        
    }

    void test()
    {

    }

    bool isExpired(struct IoRecord *ir)
    {/*判断是否需要清理过期数据*/
        long daydiff = (ir->alloc_time - lastday) / 86400;
        if(daydiff > EXPIREDAY)
        {
            return true;
        }
        else return false;
    }

    void rmExpiredData()
    {
        auto it = (*TOT).find(lastday);
        /*remove expired data*/
        unordered_map<string,unordered_map<u_int64_t, struct BlockInfo *> *> *diskmap = it->second;
        for(auto it2 = (*diskmap).begin() ; it2 != (*diskmap).end() ; it2++)
        {
            unordered_map<u_int64_t,struct BlockInfo *> *blockmap = it2->second;
            for(auto it3 = (*blockmap).begin() ; it3 != (*blockmap).end() ; it3++)
            {
                struct BlockInfo *bio = it3->second;
                free(bio);
                it3->second = NULL;
                (*blockmap).erase(it3);
            }
            delete blockmap;
            it2->second = NULL;
            (*diskmap).erase(it2);
        }
        delete diskmap;
        it->second = NULL;
        (*TOT).erase(it);
        lastday++;
    }

};
#endif