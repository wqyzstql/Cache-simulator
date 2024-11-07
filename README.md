# 作业2报告

## 模拟器设计思想

基于MESI协议实现了一个2核CPU的cache模拟器，每个核只有一级cache，每个缓存行长度为64字节。考虑将内存中数据按照64字节的长度与缓存行对齐，由于只有一级cache，所以在缓存替换时如果遇到当前缓存不匹配直接进行替换即可，无需进一步考虑较为复杂的选择哪一个缓存行清空的情况。

在对内存的读写操作发生时，首先将被操作的内存进行处理，找到内存按照64字节对齐的这一段的开头作为识别标记。然后去CPU对应的核中寻找这段内存是否在缓存中，如果这段内存不在缓存中，那么就要把这段数据放入cache中。然后考虑两个核心的cache之间的一致性关系：如果另一个核心中的cache内不是目前操作的这段内存，那么另一个核心的状态便无需改变，只需要修改目前操作核心的状态即可。操作核心的状态转换有如下几种情况：

1. 之前缓存行为空，状态为$I$时:
   1. 当前操作为读操作，将当前缓存行的状态变为$E$,状态为$I->E$
   2. 当前操作为写操作，由于修改了缓存行状态，所以状态变为$M$.状态为$I->M$
2. 之前缓存行非空时，由于只有一级cache，则要将原来的数据状态设为$I$再进行和上文相同的状态修改，也就是将状态从原来的状态变为$E$或者$M$

当另一个核心的缓存中恰好存储的也是当前操作的这段内存时，就要考虑两个核心之间缓存行状态的关系。针对操作核心的状态转变和上文相似。同时，由于目前的内存操作一定是最新的，所以另一个核心如果是处于$M$状态，那么要先将缓存内容写入内存。针对两个核的操作如下:

1. 当目前的操作为读操作时，考虑另一个核心的状态为$M$或者$E$，在这两种状态下两个核心都要把状态转换成为$S$状态。
2. 当操作为写操作时，由于另一个核心保存的是之前的数据，那么另一个核心的状态应该被设为$I$状态，当前核心的状态应该被设为$M$状态。



## 测试

在测试中，假设所有操作轮流访问核心0和核心1，即trace0.txt中的操作和trace1.txt中的操作会交叉起来，多余的操作会按照顺序排到访问队列的最后。

首先考虑所有的访问都是不同内存段的情况，这里的测试数据如下

trace0.txt:

```
0 0000001A
1 0000005A
0 0000009A
```

trace1.txt:

```
0 0000011A
1 0000015A
1 000001AA
```

测试结果为:

```
--------------------------------------------------
Currently operating on core 0
Currently operating address is 0000001A, Operating type is read.
Core 0's cache save the memory from address 0 to address 64
There are no memory in Core 1's cache!
Core 0's status from I to E
Core 1's status from I to I
--------------------------------------------------
Currently operating on core 1
Currently operating address is 0000011A, Operating type is read.
Core 0's cache save the memory from address 0 to address 64
Core 1's cache save the memory from address 256 to address 320
Core 0's status from E to E
Core 1's status from I to E
--------------------------------------------------
Currently operating on core 0
Currently operating address is 0000005A, Operating type is write.
Core 0's cache save the memory from address 64 to address 128
Core 1's cache save the memory from address 256 to address 320
Core 0's status from E to M
Core 1's status from E to E
--------------------------------------------------
Currently operating on core 1
Currently operating address is 0000015A, Operating type is write.
Core 0's cache save the memory from address 64 to address 128
Core 1's cache save the memory from address 320 to address 384
Core 0's status from M to M
Core 1's status from E to M
--------------------------------------------------
Currently operating on core 0
Currently operating address is 0000009A, Operating type is read.
Core 0's cache save the memory from address 128 to address 192
Core 1's cache save the memory from address 320 to address 384
Core 0's status from M to E
Core 1's status from M to M
--------------------------------------------------
Currently operating on core 1
Currently operating address is 000001AA, Operating type is write.
Core 0's cache save the memory from address 128 to address 192
Core 1's cache save the memory from address 384 to address 448
Core 0's status from E to E
Core 1's status from M to M
--------------------------------
```

发现出现了$M->E,I->E,E->M$几种状态转变，还缺少转换到S和I这两种情况，这和上文分析是一致的，因为没有出现过两个核对相同内存的操作。

接下来测试两个核心对相同内存端的操作，测试数据如下:

trace0.txt

```
0 0000001A
1 0000002A
0 0000003A
```

trace1.txt

```
1 0000000A
1 0000002B
0 0000003B
```

得到的输出为

```
--------------------------------------------------
Currently operating on core 0
Currently operating address is 0000001A, Operating type is read.
Core 0's cache save the memory from address 0 to address 64
There are no memory in Core 1's cache!
Core 0's status from I to E
Core 1's status from I to I
--------------------------------------------------
Currently operating on core 1
Currently operating address is 0000000A, Operating type is write.
There are no memory in Core 0's cache!
Core 1's cache save the memory from address 0 to address 64
Core 0's status from E to I
Core 1's status from I to M
--------------------------------------------------
Currently operating on core 0
Currently operating address is 0000002A, Operating type is write.
Core 0's cache save the memory from address 0 to address 64
There are no memory in Core 1's cache!
Core 0's status from I to M
Core 1's status from M to I
--------------------------------------------------
Currently operating on core 1
Currently operating address is 0000002B, Operating type is write.
There are no memory in Core 0's cache!
Core 1's cache save the memory from address 0 to address 64
Core 0's status from M to I
Core 1's status from I to M
--------------------------------------------------
Currently operating on core 0
Currently operating address is 0000003A, Operating type is read.
Core 0's cache save the memory from address 0 to address 64
Core 1's cache save the memory from address 0 to address 64
Core 0's status from I to S
Core 1's status from M to S
--------------------------------------------------
Currently operating on core 1
Currently operating address is 0000003B, Operating type is read.
Core 0's cache save the memory from address 0 to address 64
Core 1's cache save the memory from address 0 to address 64
Core 0's status from S to S
Core 1's status from S to S
--------------------------------
```

出现了$I->E,I->M,E->I,M->I,I->S,M->S$的几种转移，符合上文中的分析。



## 感悟

在完成多核CPU缓存模拟器项目的这段时间里，我不仅深化了对计算机体系结构的理解，也对并行计算和性能优化有了新的认识。通过构建缓存模拟器，我对缓存的工作原理特别是缓存一致性协议有了更加深刻的认识。

