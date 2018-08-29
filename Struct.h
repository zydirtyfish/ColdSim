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

#ifndef EXPIREDHOUR
#define EXPIREDHOUR 168 /*7 days*/
#endif

#ifndef STARTTIME
#define STARTTIME 1514736000 /*2018-01-01 00:00:00*/
#endif

#ifndef BLOCKINFO
#define BLOCKINFO
struct BlockInfo
{
    int type;                 /*类别，0为冷数据，1为热数据*/
    long TOT_pos;            /*在TOT中的位置*/
    long read_timestamp;
    long write_timestamp;
    u_int64_t writesize;
    u_int64_t readsize;
    u_int64_t read_freq;
    u_int64_t write_freq;
    struct BlockInfo *next;
    struct BlockInfo *pre;
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
    unordered_map<long,struct BlockInfo *> *TOT;
    /*Index Map for TOT*/
    unordered_map<string, unordered_map<u_int64_t,struct BlockInfo *> *> *index_map;

    long lasthour,hishour;

public:

    MemStruct()
    {
        unordered_map<long, struct BlockInfo *> *TOT = new unordered_map<long, struct BlockInfo *>();
        unordered_map<string, unordered_map<u_int64_t, struct BlockInfo *>* > *index_map = new unordered_map<string, unordered_map<u_int64_t, struct BlockInfo *> *>();
        hishour = lasthour = 0;
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
        long hour = (bs->alloc_time - STARTTIME) / 3600;

        auto it1 = index_map->find(bs->disksn);
        struct BlockInfo *bio1 = NULL;
        if (it1 != index_map->end())
        {
            unordered_map<u_int64_t, long> *tmp = it1->second;
            auto it2 = tmp->find(bs->blockid);
            if (it2 != tmp->end())
            {
                struct BlockInfo *bio1 = it2->second;
            }
        }

        int flag = 0;
        if(bio1 == NULL)
        {
            bio1 = new BlockInfo;
            bio1->type = 1;
            bio1->read_timestamp = bio1->write_timestamp = 0;
            bio1->write_freq = bio1->writesize = bio1->read_freq = bio1->readsize = 0;
            bio1->next = bio1->pre = NULL;
            flag = 1;
        }

        if(bs->io_type == 0)
        {
            bio1->read_freq++;
            bio1->readsize += bs->size;
            bio1->read_timestamp = bs->alloc_time;
        }
        else
        {
            bio1->write_freq++;
            bio1->writesize += bs->io_type;
            bio1->write_timestamp = bs->alloc_time;
            flag = 1;
        }

        if(flag)
        {
            bio1->TOT_pos = hour;
            auto it = TOT->find(hour);
            struct BlockInfo *bio = it->second;
            if(bio1 != bio)
            {
                bio1->next = bio;
                bio1->pre = NULL;
                if(bio != NULL)
                {
                    bio->pre = bio1;
                }
            }
            it->second = bio1;
        }
        
    }

    bool isExpired(struct IoRecord *ir)
    {/*判断是否需要清理过期数据*/
        if (lasthour == 0)
        {
            hishour = lasthour = (ir->alloc_time - STARTTIME) / 3600;
            addNewHour(lasthour);
            return false;
        }

        long currenthour = (ir->alloc_time - STARTTIME) / 3600;
        long hourdiff = currenthour - lasthour;
        if (hourdiff > EXPIREDHOUR)
        {
            rmExpiredData();
            addNewHour(currenthour);
            return true;
        }
        else 
        {
            if(hourdiff < EXPIREDHOUR)
            {
                if(currenthour != hishour)
                {
                    addNewHour(currenthour);
                    hishour = currenthour;
                }
            }
            return false;
        }
    }

    void rmExpiredData()
    {
        /*remove expired data*/
        auto it = (*TOT).find(lasthour);
        struct BlockInfo *bio = it->second;
        struct BlockInfo *biotmp;
        while(bio != NULL)
        {
            biotmp = bio;
            bio = bio->next;
            //delete biotmp;
            biotmp->pre = biotmp->next = NULL;
            biotmp->TOT_pos = -1;
        }
        it->second = NULL;
        (*TOT).erase(it);
        lasthour++;
    }

    void addNewHour(long hour)
    {
        (*TOT)[hour] = NULL;
    }

    u_int64_t getToTSize()
    {
        u_int64_t result = 0;
        for(auto it = (*TOT).begin() ; it != (*TOT).end() ; it++)
        {
            struct BlockInfo *bio = it->second;
            while(bio != NULL)
            {
                bio = bio->next;
                result++;
            }
        }
        return result;
    }

    u_int64_t getTOTSize(long time)
    {
        u_int64_t result = 0;
        auto it = (*TOT).find(time);
        struct BlockInfo *bio = it->second;
        while (bio != NULL)
        {
            bio = bio->next;
            result++;
        }
        return result;
    }
};
#endif