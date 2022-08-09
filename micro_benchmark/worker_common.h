#ifndef __WORKER_COMMON_H__
#define __WORKER_COMMON_H__

#define LOOP4(x)   x x x x
#define LOOP16(x)  LOOP4(x) LOOP4(x) LOOP4(x) LOOP4(x)
#define LOOP64(x)  LOOP16(x) LOOP16(x) LOOP16(x) LOOP16(x)
#define LOOP128(x) LOOP64(x) LOOP64(x)
#define LOOP256(x) LOOP128(x) LOOP128(x)

#define MY_NOP1()    asm("nop")
#define MY_NOP2()    MY_NOP1(); MY_NOP1()
#define MY_NOP4()    MY_NOP2(); MY_NOP2()
#define MY_NOP8()    MY_NOP4(); MY_NOP4()
#define MY_NOP16()   MY_NOP8(); MY_NOP8()
#define MY_NOP32()   MY_NOP16(); MY_NOP16()
#define MY_NOP48()   MY_NOP16(); MY_NOP32()
#define MY_NOP64()   MY_NOP32(); MY_NOP32()
#define MY_NOP96()   MY_NOP32(); MY_NOP64()
#define MY_NOP128()  MY_NOP64(); MY_NOP64()
#define MY_NOP160()  MY_NOP64(); MY_NOP96()
#define MY_NOP192()  MY_NOP64(); MY_NOP128()
#define MY_NOP224()  MY_NOP64(); MY_NOP160()
#define MY_NOP256()  MY_NOP128(); MY_NOP128()
#define MY_NOP384()  MY_NOP128(); MY_NOP256()
#define MY_NOP512()  MY_NOP256(); MY_NOP256()
#define MY_NOP768()  MY_NOP256(); MY_NOP512()
#define MY_NOP1024() MY_NOP512(); MY_NOP512()

#endif
