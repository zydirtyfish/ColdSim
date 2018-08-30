#include "ColdSim.h"
#include "Struct.h"
#include "get_trace_tool.h"
#include "log.h"

class RUN
{
private:
    get_trace_tool *gtt;
    struct IoRecord *ior;
    MemStruct *ms;

  public:
    RUN()
    {
        LogClear();
        ms = new MemStruct();
    }

    ~RUN()
    {
        delete ms;
        ms = NULL;
    }

    void exec(char *depotid)
    {
        char ds1[40];
        char ds2[40];
        char logfile[200];
        long time_stamp = get_today();

        for (int i = 0; i < 2160; i++)
        {
            get_ds1(ds1, time_stamp, i * 3600);
            get_ds2(ds2, time_stamp, i * 3600);
            long row_cnt = get_data(ds1, ds2, depotid);
            strcpy(logfile, "/data0/app/hivelog/");
            strcat(logfile, depotid);
            strcat(logfile, "/");
            strcat(logfile, ds2);
            if (row_cnt == 0)
            {
                LogWrite(2, "file %s doesn't exist!!!", logfile); //error info
                continue;
            }
            LogWrite(1, "file %s start!!!", logfile); //start info

            gtt = new get_trace_tool(logfile);
            ior = gtt->get_ti(true);
            while (ior != NULL)
            {
                process(ior);
                ior = gtt->get_ti(true);
            }
            delete gtt;

            if (i % 24 == 0)
            {
                //output_info(io_log);
            }
        }
        /*ior = new IoRecord;
        ior->alloc_time = 1514746000;
        strcpy(ior->disksn, "12312aaaa");
        ior->offset = 33435564;
        ior->size = 16;
        ior->io_type = 1;
        process(ior);*/
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

    long get_today()
    {
        //time_t curtime = time(NULL);
        return 1515772800; /*2018.01.13-00:00:00*/
    }

    long get_max()
    {
        return 1523199600; /*2018.04.08-23:00:00*/
    }

    char *get_ds1(char *ds1, long time_tmp, int diff)
    {
        time_tmp += diff;
        struct tm *ttime;
        ttime = localtime(&time_tmp);
        strftime(ds1, 64, "%Y-%m-%d %H", ttime);
        return ds1;
    }

    char *get_ds2(char *ds1, long time_tmp, int diff)
    {
        time_tmp += diff;
        struct tm *ttime;
        ttime = localtime(&time_tmp);
        strftime(ds1, 64, "%Y-%m-%d-%H", ttime);
        return ds1;
    }

    long get_data(char *ds1, char *ds2, char *para1)
    {
        char cmd_sh[200];
        strcpy(cmd_sh, "sh get_data.sh ");
        strcat(cmd_sh, para1);
        strcat(cmd_sh, " '");
        strcat(cmd_sh, ds1);
        strcat(cmd_sh, "' ");
        strcat(cmd_sh, ds2);
        cout << cmd_sh << endl;
        system(cmd_sh);

        strcpy(cmd_sh, "/data0/app/hivelog/");
        strcat(cmd_sh, para1);
        strcat(cmd_sh, "/");
        strcat(cmd_sh, ds2);
        FILE *fh = fopen(cmd_sh, "r");
        if (fh == NULL)
        {
            printf("%s doesn't exist\n", cmd_sh);
            return 0;
        }
        fclose(fh);

        strcpy(cmd_sh, "wc -l /data0/app/hivelog/");
        strcat(cmd_sh, para1);
        strcat(cmd_sh, "/");
        strcat(cmd_sh, ds2);
        strcat(cmd_sh, " | awk '{print $1}' > tmp");
        cout << cmd_sh << endl;
        long result;
        system(cmd_sh);
        FILE *fp;
        fp = fopen("tmp", "r");
        fscanf(fp, "%ld", &result);
        fclose(fp);
        return result;
    }

};