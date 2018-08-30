#!/bin/bash

#ts=`date -d -1day +%s`
depot_id=$1
#ts=$2
ds=$2
ds_tmp=$3

#su hadoop

mkdir -p /data0/app/hivelog/$depot_id
mkdir -p /data0/app/hivelog/tmp/$depot_id
#ds=`date -d @$ts +'%Y-%m-%d %H'`
#ds_tmp=`date -d @$ts +'%Y-%m-%d-%H'`
echo $ds 
hive -e "use hcbs_iotrace;insert overwrite local directory '/data0/app/hivelog/tmp/$depot_id/' row format delimited fields terminated by ',' select alloc_time,offset,reqlen,optype,disk_sn from trace_table where depot_id=$depot_id and ds='$ds' order by alloc_time"

for((j=0;j<20;j++))
do
 filename="/data0/app/hivelog/tmp/$depot_id/0000"
 if [ $j -lt 10 ];then
  filename=$filename'0'$j'_0'
 else
  filename=$filename$j'_0'
 fi
 if [ -f "$filename" ];then
  if [ $j -eq 0 ];then
   mv $filename /data0/app/hivelog/$depot_id/$ds_tmp
  else
   cat $filename >> /data0/app/hivelog/$depot_id/$ds_tmp
  fi
 fi
done