# Fsw

fsw means from swoole but not all swoole. This project is the foundation of [study php extension](https://github.com/php-extension-research/study).

## Build

```shell
~/codeDir/cppCode/fsw # cmake .
~/codeDir/cppCode/fsw # make
~/codeDir/cppCode/fsw # make install
```

## Example

```cpp
#include <iostream>
#include "fsw/coroutine.h"

using namespace fsw;
using namespace std;

int main(int argc, char const *argv[])
{
    Coroutine::create([](void *arg)
    {
        cout << "co1" << endl;
    });

    Coroutine::create([](void *arg)
    {
        cout << "co2" << endl;
    });

    return 0;
}
```

## Pressure measurement

a Simple echo server:

```shell
~/codeDir/phpCode/test/swoole # ab -c 1000 -n 1000000 127.0.0.1:80/
This is ApacheBench, Version 2.3 <$Revision: 1843412 $>
Copyright 1996 Adam Twiss, Zeus Technology Ltd, http://www.zeustech.net/
Licensed to The Apache Software Foundation, http://www.apache.org/

Benchmarking 127.0.0.1 (be patient)
Completed 100000 requests
Completed 200000 requests
Completed 300000 requests
Completed 400000 requests
Completed 500000 requests
Completed 600000 requests
Completed 700000 requests
Completed 800000 requests
Completed 900000 requests
Completed 1000000 requests
Finished 1000000 requests


Server Software:
Server Hostname:        127.0.0.1
Server Port:            80

Document Path:          /
Document Length:        946 bytes

Concurrency Level:      1000
Time taken for tests:   74.984 seconds
Complete requests:      1000000
Failed requests:        0
Non-2xx responses:      1000000
Total transferred:      1023000000 bytes
HTML transferred:       946000000 bytes
Requests per second:    13336.26 [#/sec] (mean)
Time per request:       74.984 [ms] (mean)
Time per request:       0.075 [ms] (mean, across all concurrent requests)
Transfer rate:          13323.24 [Kbytes/sec] received

Connection Times (ms)
              min  mean[+/-sd] median   max
Connect:        0   35  43.4     33    1114
Processing:     0   39   8.8     37     110
Waiting:        0   29   8.2     26      96
Total:         35   75  44.3     71    1165

Percentage of the requests served within a certain time (ms)
  50%     71
  66%     74
  75%     76
  80%     78
  90%     84
  95%     93
  98%    111
  99%    123
 100%   1165 (longest request)
```

it consumes 1G of memory.
