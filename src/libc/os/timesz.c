/*
  Copyright (c) 2013-2014 Dong Fang. All rights reserved.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom
  the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included
  in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/

#include <time.h>
#include <sys/time.h>
#include "timesz.h"

int64_t rt_mstime() {
    struct timeval tv;
    int64_t ct;

    gettimeofday(&tv, NULL);
    ct = (int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;

    return ct;
}

int64_t rt_ustime() {
    struct timeval tv;
    int64_t ct;

    gettimeofday(&tv, NULL);
    ct = (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;

    return ct;
}

int64_t rt_nstime() {
    struct timeval tv;
    int64_t ct;

    gettimeofday(&tv, NULL);
    ct = (int64_t)tv.tv_sec * 1000000000 + (int64_t)tv.tv_usec * 1000;

    return ct;
}
    

int rt_usleep(int64_t usec) {
    struct timespec tv;
    tv.tv_sec = usec / 1000000;
    tv.tv_nsec = (usec % 1000000) * 1000;
    return nanosleep(&tv, NULL);
}
