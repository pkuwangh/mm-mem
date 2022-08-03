#ifndef __WORKER_COMMON_H__
#define __WORKER_COMMON_H__

#define LOOP4(x)   x x x x
#define LOOP16(x)  LOOP4(x) LOOP4(x) LOOP4(x) LOOP4(x)
#define LOOP64(x)  LOOP16(x) LOOP16(x) LOOP16(x) LOOP16(x)
#define LOOP128(x) LOOP64(x) LOOP64(x)
#define LOOP256(x) LOOP128(x) LOOP128(x)


#endif
