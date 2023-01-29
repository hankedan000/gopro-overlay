# GoPro Overlay
Tools for overlaying telemetry data onto GoPro footage.

## Building
Pretty standard CMake build procedure.

```bash
mkdir build
cd build
cmake ..
make
```

## Testing
### Unit Tests
The repo contains a few cppunit tests. You can run them all after a build with `make test`. You should get output similar to this.

``` bash
user@dev-pa:~/git/gopro-overlay/build$ make test
Running tests...
Test project /home/user/git/gopro-overlay/build
    Start 1: LineSegmentUtilsTest
1/4 Test #1: LineSegmentUtilsTest .............   Passed    0.00 sec
    Start 2: SeekerTest
2/4 Test #2: SeekerTest .......................   Passed    0.10 sec
    Start 3: TrackDataObjectsTest
3/4 Test #3: TrackDataObjectsTest .............   Passed    0.09 sec
    Start 4: DataProcessingUtilsTest
4/4 Test #4: DataProcessingUtilsTest ..........   Passed    0.10 sec

100% tests passed, 0 tests failed out of 4

Total Test time (real) =   0.30 sec
```

### Memory Checks
To run memory-related tests (access violations, memory leaks, etc), use `ctest -T memcheck` with the build root. You should get output similar to this.

``` bash
user@dev-pc:~/git/gopro-overlay/build$ ctest -T memcheck
   Site: dev-pc
   Build name: Linux-c++
Memory check project /home/user/git/gopro-overlay/build
    Start 1: LineSegmentUtilsTest
1/4 MemCheck #1: LineSegmentUtilsTest .............   Passed    0.88 sec
    Start 2: SeekerTest
2/4 MemCheck #2: SeekerTest .......................   Passed    4.35 sec
    Start 3: TrackDataObjectsTest
3/4 MemCheck #3: TrackDataObjectsTest .............   Passed    4.49 sec
    Start 4: DataProcessingUtilsTest
4/4 MemCheck #4: DataProcessingUtilsTest ..........   Passed    4.85 sec

100% tests passed, 0 tests failed out of 4

Total Test time (real) =  14.58 sec
-- Processing memory checking output:
2/4 MemCheck: #2: SeekerTest .......................   Defects: 293
3/4 MemCheck: #3: TrackDataObjectsTest .............   Defects: 293
4/4 MemCheck: #4: DataProcessingUtilsTest ..........   Defects: 293
MemCheck log files can be found here: (<#> corresponds to test number)
/home/user/git/gopro-overlay/build/Testing/Temporary/MemoryChecker.<#>.log
Memory checking results:
Potential Memory Leak - 879
```