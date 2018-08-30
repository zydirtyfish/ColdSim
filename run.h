#include "ColdSim.h"
#include "Struct.h"
#include "get_trace_tool.h"

class RUN
{
private:
    get_trace_tool *gtt;
    struct IoRecord *ior;

public:
    RUN()
    {

    }
    void exec()
    {
        gtt = new get_trace_tool("filename");
        ior = gtt->get_ti(true);
        while(ior != NULL)
        {
            ior = gtt->get_ti(true);
        }
        delete gtt;
    }

};