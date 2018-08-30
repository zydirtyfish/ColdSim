#include "ColdSim.h"
#include "Struct.h"
#include "get_trace_tool.h"

class RUN
{
private:
    get_trace_tool *gtt;
    struct IoRecord *ior;
    MemStruct *ms;

  public:
    RUN()
    {
        ms = new MemStruct();
    }

    ~RUN()
    {
        delete ms;
        ms = NULL;
    }
    void exec()
    {
        /*gtt = new get_trace_tool("filename");
        ior = gtt->get_ti(true);
        while(ior != NULL)
        {
            process(ior);
            ior = gtt->get_ti(true);
        }
        delete gtt;*/

        ior = new IoRecord;
        ior->alloc_time = 1514746000;
        strcpy(ior->disksn, "12312aaaa");
        ior->offset = 33435564;
        ior->size = 16;
        ior->io_type = 1;
        process(ior);
    }

    void process(struct IoRecord *ior)
    {
        /*处理流程示例*/
        ms->isExpired(ior);
        struct BlockStruct *bs = ms->ioToBlock(ior);
        struct BlockStruct *bshead = bs;
        while (bs != NULL)
        {
            ms->updateTOT(bs);
            bs = bs->next;
        }
        ms->freeBlockList(bshead);
    }
};