
# 环境安装

1. 安装CUnit 
```shell
[root@xphobia cunit]# yum install -y CUnit-devel
```

2. 安装lcov（用于生成测试覆盖率报告）
```shell
[root@xphobia cunit]# yum install -y lcov
```

# 测试

1. 编译测试用例
```shell
[root@xphobia cunit]# gcc -fprofile-arcs -ftest-coverage -o example example.c -lcunit
```

2. 执行测试
```shell
[root@xphobia cunit]# ./example 


     CUnit - A unit testing framework for C - Version 2.1-2
     http://cunit.sourceforge.net/


Suite: Suite_1
  Test: test of fprintf() ...passed
  Test: test of fread() ...passed

Run Summary:    Type  Total    Ran Passed Failed Inactive
              suites      1      1    n/a      0        0
               tests      2      2      2      0        0
             asserts      5      5      5      0      n/a

Elapsed time =    0.000 seconds
```

3. 生成覆盖率中间数据文件
```shell
[root@xphobia cunit]# gcov example.c
File 'example.c'
Lines executed:81.58% of 38
Creating 'example.c.gcov'
```

4. 生成xml形式的覆盖率报告文件至app.info文件
```shell
[root@xphobia cunit]# lcov --directory . --capture --output-file app.info
Capturing coverage data from .
Found gcov version: 4.8.5
Scanning . for .gcda files ...
Found 1 data files in .
Processing example.gcda
Finished .info-file creation
```

5. 生成html形式的覆盖率报告文件至results文件夹，数据来源文件为app.info
```shell
[root@xphobia cunit]# genhtml -o results app.info
Reading data file app.info
Found 1 entries.
Found common filename prefix "/root/linuxC"
Writing .css and .png files.
Generating output.
Processing file cunit/example.c
Writing directory view page.
Overall coverage rate:
  lines......: 81.6% (31 of 38 lines)
  functions..: 100.0% (5 of 5 functions)

```
查看results目录
```shell
[root@xphobia cunit]# ls
app.info  doc.md  example  example.c  example.gcda  example.gcno  results  temp.txt
[root@xphobia cunit]# 
[root@xphobia cunit]# ls results/
amber.png  cunit  emerald.png  gcov.css  glass.png  index.html  index-sort-f.html  index-sort-l.html  ruby.png  snow.png  updown.png
```

6. 把results目录拷贝出来，在浏览器中打开index.html，可直观的查看测试覆盖率


# 参考资料

1. CUnit官网：http://cunit.sourceforge.net/
2. 