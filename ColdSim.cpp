#include "ColdSim.h"

int main(int argc, char *argv[])
{
    MemStruct *ms = new MemStruct();
    cout << ms->hasBlock("abc",123) << endl;;
    cout << "1234" << endl;
    return 0;
}