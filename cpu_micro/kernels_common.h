#ifndef __KERNELS_COMMON_H__
#define __KERNELS_COMMON_H__

namespace mm_worker {

#define RD32     ret += *p; p += 4;
#define WR32     *p = ret;  p += 4;

#define LP1(x)   x
#define LP2(x)   x x
#define LP3(x)   x x x
#define LP4(x)   LP2(x) LP2(x)
#define LP5(x)   LP3(x) LP2(x)
#define LP6(x)   LP4(x) LP2(x)
#define LP8(x)   LP4(x) LP4(x)
#define LP16(x)  LP8(x) LP8(x)
#define LP32(x)  LP16(x) LP16(x)
#define LP42(x)  LP32(x) LP8(x) LP2(x)
#define LP64(x)  LP32(x) LP32(x)
#define LP128(x) LP64(x) LP64(x)
#define LP256(x) LP128(x) LP128(x)

#define MN1    asm("nop");
#define MN2    MN1 MN1
#define MN4    MN2 MN2
#define MN8    MN4 MN4
#define MN16   MN8 MN8
#define MN32   MN16 MN16
#define MN48   MN16 MN32
#define MN64   MN32 MN32
#define MN80   MN32 MN48
#define MN88   MN32 MN48 MN8
#define MN96   MN32 MN64
#define MN104  MN64 MN48 MN8
#define MN112  MN64 MN48
#define MN128  MN64 MN64
#define MN160  MN64 MN96
#define MN192  MN64 MN128
#define MN224  MN64 MN160
#define MN256  MN128 MN128
#define MN320  MN128 MN192
#define MN384  MN128 MN256
#define MN448  MN256 MN192
#define MN512  MN256 MN256
#define MN640  MN256 MN384
#define MN768  MN256 MN512
#define MN1024 MN512 MN512
#define MN1536 MN512 MN1024
#define MN2048 MN1024 MN1024

}

#endif
