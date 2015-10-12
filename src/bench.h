#ifndef _bench_h_
#define _bench_h_

typedef struct bench {
        char name[64];
        struct timeval start;
        struct timeval stop;
} bench_t;

void bench_start(struct bench* b, const char* name);

void bench_end(struct bench* b);

int bench_diff_us(const struct bench* b);

float bench_diff_s(const struct bench* b);

#endif

