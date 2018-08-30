#include "ColdSim.h"
#include "Struct.h"

class get_trace_tool
{
  private:
    char filename[200];
    FILE *fp;
    struct IoRecord *ti;
    u_int64_t total_rec;
    u_int64_t curr_rec;

  public:
    //get_trace_tool init
    get_trace_tool(const char *fn)
    {
        strcpy(filename, fn);
        total_rec = get_rec_cnt();
        fp = NULL;
        fp = open_file(filename);
        ti = new IoRecord;
        curr_rec = 0;
    }

    ~get_trace_tool()
    {
        close_file(fp);
        fp = NULL;
        delete ti;
        ti = NULL;
    }

    //open a file
    FILE *open_file(char *fn)
    {
        return fopen(fn, "r");
    }

    //close a file
    void close_file(FILE *fp)
    {
        fclose(fp);
        fp = NULL;
        remove(filename);
    }

    //get next IoRecord
    struct IoRecord *get_ti(bool output)
    {
        if (!feof(fp))
        {
            get_ti_function(output);
            return ti;
        }
        else
        {
            return NULL;
        }
    }

    void get_ti_function(bool output)
    {
        curr_rec++;
        fscanf(fp, "%ld,%llu,%d,%d,%s\n", &(ti->alloc_time), &(ti->offset), &(ti->size), &(ti->io_type), ti->disksn);
        if (output)
        {
            get_progress_ratio();
        }
    }

    u_int64_t get_rec_cnt()
    {
        string tmp;
        u_int64_t n = 0;
        ifstream ifs1;
        ifs1.open(filename, ios::in); // ios::in 表示以只读方式读取文件
        while (getline(ifs1, tmp))
        {
            n++;
        }
        ifs1.close();
        return n;
    }

    void get_progress_ratio()
    {
        if (curr_rec % 100000 == 0)
        {
            printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b");
            cout << "progress ratio:" << (u_int64_t)(curr_rec * 100.0) / total_rec << "%";
        }
    }

    u_int64_t get_total_rec()
    {
        return total_rec;
    }
};