./Dockerfile contains requirements needed to build and run unit tests.
./Makefile contains helper targets

To build and run tests:
 - docker build . -t heap
 - docker run heap:latest


To build and run tests:
 - docker build . -f ./Dockerfile.arm32v7 -t heap_arm32v7
 - docker run heap_arm32v7:latest