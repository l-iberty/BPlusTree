# C++ implementation of B+Tree
- Reference
    - Book *Database System Concepts 7th Edition-2019*
    - [CMU 15.445 2017](https://15445.courses.cs.cmu.edu/fall2017//assignments.html)
- API
    - [x] Insert
    - [x] Lookup
    - [ ] Delete
- Test results
```
[==========] Running 5 tests from 1 test case.
[----------] Global test environment set-up.
[----------] 5 tests from BPlusTree
[ RUN      ] BPlusTree.KeyComparatorTest
[       OK ] BPlusTree.KeyComparatorTest (0 ms)
[ RUN      ] BPlusTree.SimpleTest1
dumping B+Tree to dot files "test1_1.dot", "test1_2"...
[       OK ] BPlusTree.SimpleTest1 (7 ms)
[ RUN      ] BPlusTree.SimpleTest2
dumping B+Tree to dot files "test2_1.dot", "test2_2"...
[       OK ] BPlusTree.SimpleTest2 (13 ms)
[ RUN      ] BPlusTree.SimpleTest3
number of kv inserted: 1000
dumping B+Tree to dot files "test3_1.dot", "test3_2"...
[       OK ] BPlusTree.SimpleTest3 (9078 ms)
[ RUN      ] BPlusTree.Compare1
stl map INSERT test begins...
stl map INSERT test ends: 0.482
B+Tree INSERT test begins...
B+Tree INSERT test ends: 5.506
stl map FIND test begins...
stl map FIND test ends: 0.478
B+Tree FIND test begins...
B+Tree FIND test ends: 3.992
[       OK ] BPlusTree.Compare1 (10495 ms)
[----------] 5 tests from BPlusTree (19600 ms total)

[----------] Global test environment tear-down
[==========] 5 tests from 1 test case ran. (19604 ms total)
[  PASSED  ] 5 tests.
```
- Using *dot* to visualize. Here is an example:

![](code/test1_1.png)

In order to validate the *parent* pointer of each node, another picture is drawn as follow:

![](code/test1_2.png)