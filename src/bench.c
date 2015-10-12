#include <string.h>
#include <sys/time.h>

#include "bench.h"

void bench_start(struct bench* b, const char* name) {
        strcpy(b->name, name);
        gettimeofday(&b->start, NULL);
}

void bench_end(struct bench* b) {
        gettimeofday(&b->stop, NULL);
}

int bench_diff_us(const struct bench* b) {
        int start = (b->stop.tv_sec - b->start.tv_sec) * 1000000
                  + (b->stop.tv_usec - b->start.tv_usec);
        return start;
}

float bench_diff_s(const struct bench* b) {
        float start = (b->stop.tv_sec - b->start.tv_sec)
                    + (b->stop.tv_usec - b->start.tv_usec) * 1E-6;
        return start;
}

