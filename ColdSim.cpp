#include "run.h"

int main(int argc, char *argv[])
{
    RUN *run = new RUN("/data0/app/hivelog/");
    //run->exec(argv[1]);
    run->exec("3");
    delete run;
    return 0;
}