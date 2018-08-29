#include "ColdSim.h"
#include "Struct.h"

int main(int argc, char *argv[])
{
    MemStruct *ms = new MemStruct();
    struct IoRecord *ior = new IoRecord;
    ior->alloc_time = 1514746000;
    strcpy(ior->disksn, "12312aaaa");
    ior->offset = 33435564;
    ior->size =16;
    ior->io_type = 1;

    /*处理流程示例*/
    ms->isExpired(ior);
    struct BlockStruct *bs = ms->ioToBlock(ior);
    struct BlockStruct *bshead = bs;
    while(bs != NULL)
    {
        ms->updateTOT(bs);
        bs = bs->next;   
    }
    ms->freeBlockList(bshead);
    
    //cout << "1234" << endl;
    return 0;
}