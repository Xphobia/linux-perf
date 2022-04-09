
## 编译highcpu.c
```
[root@localhost ~]# gcc -g -o highcpu highcpu.c -lpthread
[root@localhost ~]# ./highcpu 
140476148422400 : threadA starts working...
140476140029696 : threadB starts working...
```

## top命令查找cpu占用最高进程

1. 查到占用cpu高的进程pid是19152
```
[root@localhost ~]# top -c -b | head -10
top - 01:54:51 up 1 day,  8:01,  3 users,  load average: 0.77, 1.28, 1.01
Tasks: 194 total,   1 running, 193 sleeping,   0 stopped,   0 zombie
%Cpu(s): 26.6 us,  1.6 sy,  0.0 ni, 68.8 id,  3.1 wa,  0.0 hi,  0.0 si,  0.0 st
KiB Mem :  8009192 total,  4943380 free,  1307968 used,  1757844 buff/cache
KiB Swap:        0 total,        0 free,        0 used.  6393948 avail Mem 

  PID USER      PR  NI    VIRT    RES    SHR S  %CPU %MEM     TIME+ COMMAND
19152 root      20   0   22900    396    312 S 106.7  0.0   0:58.32 ./highcpu
    1 root      20   0  191000   3900   2572 S   0.0  0.0   0:01.99 /usr/lib/systemd/systemd --switched-root --system --deserialize 22
    2 root      20   0       0      0      0 S   0.0  0.0   0:00.00 [kthreadd]
```

2. 查看其线程cpu使用情况，可以看出19153号线程占用cpu很高
命令：top -H -p 19152
```
top - 01:55:46 up 1 day,  8:02,  3 users,  load average: 0.91, 1.23, 1.01
Threads:   3 total,   1 running,   2 sleeping,   0 stopped,   0 zombie
%Cpu(s): 25.0 us,  0.1 sy,  0.0 ni, 74.9 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
KiB Mem :  8009192 total,  4943340 free,  1307912 used,  1757940 buff/cache
KiB Swap:        0 total,        0 free,        0 used.  6393996 avail Mem 

  PID USER      PR  NI    VIRT    RES    SHR S %CPU %MEM     TIME+ COMMAND                                                                                                                                                                                                                                                 
19153 root      20   0   22900    396    312 R 99.9  0.0   1:53.13 highcpu                                                                                                                                                                                                                                                 
19152 root      20   0   22900    396    312 S  0.0  0.0   0:00.00 highcpu                                                                                                                                                                                                                                                 
19154 root      20   0   22900    396    312 S  0.0  0.0   0:00.00 highcpu
```

3. 查看这个线程所有系统调用
   （1）命令：strace -p 19099



## ps命令查找cpu占用最高进程

```
[root@localhost ~]# ps -eo pid,ppid,%mem,%cpu,cmd --sort=-%cpu | head
  PID  PPID %MEM %CPU CMD
19152 19060  0.0 99.7 ./highcpu
 8786  7036  1.7  0.4 /root/.vscode-server/bin/e18005f0f1b33c29e81d732535d8c0e47cafb0b5/node /root/.vscode-server/bin/e18005f0f1b33c29e81d732535d8c0e47cafb0b5/out/bootstrap-fork --type=extensionHost --transformURIs --useHostProxy=false
 7036  7024  3.0  0.2 /root/.vscode-server/bin/e18005f0f1b33c29e81d732535d8c0e47cafb0b5/node /root/.vscode-server/bin/e18005f0f1b33c29e81d732535d8c0e47cafb0b5/out/server-main.js --start-server --host=127.0.0.1 --accept-server-license-terms --enable-remote-auto-shutdown --port=0 --telemetry-level all --connection-token-file /root/.vscode-server/.e18005f0f1b33c29e81d732535d8c0e47cafb0b5.token
 7118  7036  2.1  0.2 /root/.vscode-server/bin/e18005f0f1b33c29e81d732535d8c0e47cafb0b5/node /root/.vscode-server/bin/e18005f0f1b33c29e81d732535d8c0e47cafb0b5/out/bootstrap-fork --type=extensionHost --transformURIs --useHostProxy=false
 7092  7036  0.6  0.1 /root/.vscode-server/bin/e18005f0f1b33c29e81d732535d8c0e47cafb0b5/node /root/.vscode-server/bin/e18005f0f1b33c29e81d732535d8c0e47cafb0b5/out/bootstrap-fork --type=ptyHost
 7208  7118  1.6  0.1 /root/.vscode-server/extensions/ms-vscode.cpptools-1.9.7/bin/cpptools
 8957  8786  2.5  0.1 /root/.vscode-server/extensions/ms-vscode.cpptools-1.9.7/bin/cpptools
    1     0  0.0  0.0 /usr/lib/systemd/systemd --switched-root --system --deserialize 22
    2     0  0.0  0.0 [kthreadd]

[root@localhost ~]# pstree -p 19152
highcpu(19152)─┬─{highcpu}(19153)
               └─{highcpu}(19154)
```



