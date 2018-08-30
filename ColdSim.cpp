#include "run.h"

int main(int argc, char *argv[])
{
    RUN *run = new RUN();
    run->exec();
    delete run;
    return 0;
}