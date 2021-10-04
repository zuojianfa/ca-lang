
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

uint64_t currenttime() {
  struct timeval tv;
  gettimeofday(&tv, NULL);

  return tv.tv_sec * 1000000L + tv.tv_usec;
}

