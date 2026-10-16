#ifndef MM_MEM_KERNELS_BRANCH_H
#define MM_MEM_KERNELS_BRANCH_H

#include <stdint.h>

namespace mm_worker {

// Each trip executes Branches forward taken branches and one loop branch.
// Padding is skipped, never executed. The final loop branch is not taken.
// Keeping the loop in volatile asm prevents compiler unrolling or elimination.
template <unsigned Branches, unsigned Alignment, bool Conditional>
__attribute__((noinline)) void branch_chain(uint64_t rounds) {
    if (rounds == 0) {
        return;
    }
#if defined(__x86_64__)
    asm volatile(
        ".p2align 6\n"
        "1:\n"
        ".if %c[conditional]\n"
        "test %[rounds], %[rounds]\n"
        ".endif\n"
        ".rept %c[count]\n"
        ".if %c[conditional]\n"
        "jnz 2f\n"
        ".else\n"
        "jmp 2f\n"
        ".endif\n"
        ".p2align %c[alignment]\n"
        "2:\n"
        ".endr\n"
        "sub $1, %[rounds]\n"
        "jnz 1b\n"
        : [rounds] "+r" (rounds)
        : [count] "i" (Branches), [alignment] "i" (Alignment),
          [conditional] "i" (Conditional)
        : "cc", "memory");
#elif defined(__aarch64__)
    asm volatile(
        ".p2align 6\n"
        "1:\n"
        ".rept %c[count]\n"
        ".if %c[conditional]\n"
        "cbnz %[rounds], 2f\n"
        ".else\n"
        "b 2f\n"
        ".endif\n"
        ".p2align %c[alignment]\n"
        "2:\n"
        ".endr\n"
        "sub %[rounds], %[rounds], #1\n"
        "cbnz %[rounds], 1b\n"
        : [rounds] "+r" (rounds)
        : [count] "i" (Branches), [alignment] "i" (Alignment),
          [conditional] "i" (Conditional)
        : "cc", "memory");
#else
#error "Branch throughput kernels require x86-64 or AArch64"
#endif
}

struct BranchCase {
    const char* type;
    unsigned branches;
    unsigned spacing;
    void (*run)(uint64_t);
};

inline const BranchCase* branch_cases() {
#define BRANCH_CASE(N, A, C) {C ? "conditional" : "unconditional", N, 1u << A, branch_chain<N, A, C>}
    static const BranchCase cases[] = {
        BRANCH_CASE(8, 4, false), BRANCH_CASE(32, 4, false), BRANCH_CASE(128, 4, false),
        BRANCH_CASE(8, 6, false), BRANCH_CASE(32, 6, false), BRANCH_CASE(128, 6, false),
        BRANCH_CASE(8, 4, true), BRANCH_CASE(32, 4, true), BRANCH_CASE(128, 4, true),
        BRANCH_CASE(8, 6, true), BRANCH_CASE(32, 6, true), BRANCH_CASE(128, 6, true),
    };
#undef BRANCH_CASE
    return cases;
}

constexpr unsigned branch_case_count = 12;

inline uint64_t taken_branches(unsigned branches, uint64_t rounds) {
    return rounds ? rounds * (branches + 1) - 1 : 0;
}

} // namespace mm_worker
#endif
