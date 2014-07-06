数据库这边我把情况跟你们说一下
首先是要建的表

---------
首先是一个存储主机地址信息的表
hosts (#id, ip, hostname )

---------
存储主机信息的表
hostinfo(id, 内存总量, CPU个数, 磁盘总量，磁盘用量, ...)
...代表可能还有其他的，我暂时想不起来，你也想想

--------
存储资源占用情况的表，这里我设计了一下，我觉得我们可以将时间来分表，
存储精度为十秒的表，存储精度为分钟的表，存储精度为小时的表，精度为天
的表，我们采集的精度是十秒，然后每过一分钟我们将这一分钟里面的采集
的数据（存储于十 秒精度的表中）求平均插入精度为分钟的表里面，每过一
小时整理一下数据插入到小时表里面，等等。然后过一段时间删除精度为十秒
钟的表里面的数据，可以30 天删一次，过更长时间删除精度为小时的表里面的
数据，依次类推。这是我设计的数据整理的一个方法，这样的话我那边写前端的
时候可以依据不同精度来查询数据，减轻了用户的访问资源占用情况。

cpu_second( timestamp/*时间unix时间戳*/, coreid/*核的id*/, hostid, user,
     nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice)

cpu_minute( timestamp/*时间unix时间戳*/, coreid/*核的id*/, hostid, user,
     nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice)

cpu_hour( timestamp/*时间unix时间戳*/, coreid/*核的id*/, hostid, user,
     nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice)

user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice
这几个值是cpu占用的情况，所有占用信息我都用的64bit无符号整形存储的，mysql
对应的存储类型应该是unsigned bigint。

----------
下面是内存占用情况
mem_second(timestamp, hostid, usage, buffers, cached, swap_usage)
usage, buffers, cached, swap_usage这几个是资源占用情况，64bit无符号整形
minute和hour这俩表我就不写了，下面几个也同样的。

----------
磁盘io
磁盘这里多了一个表，用于保存每个设备的名称，比如sda，hda这些的字符
disk_devinfo(#devid, name)
disk_second(timestamp, hostid, devid, read, write)
read, write是读写速率，64bit无符号整形

----------
网络io
net_devinfo(#devid, name)
net_second(timestamp, hostid, devid, rc_kB, rc_pkt, rc_err, rc_drp, 
        rc_fifo, rc_frame, rc_cmprs, rc_multi, sd_kB, sd_pkt, sd_err,
        sd_drp, sd_fifo, sd_coll, sd_carrier, sd_cmprs)
这个信息略多，因为相应的proc文件下的信息就有很多，后面都是资源，
64bit整形

----------
这样我们的资源表都完了，然后是我们系统的配置信息表，这个表里面的内容
我暂时还没想到要存什么，但是我觉得我们软件的配置信息可以存里面
这个先留着不动，如果有什么配置信息的话我们讨论一下

----------
用户表
user(name, pass)
