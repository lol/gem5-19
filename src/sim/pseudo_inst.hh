/*
 * Copyright (c) 2012 ARM Limited
 * All rights reserved
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2003-2006 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Authors: Nathan Binkert
 */

#ifndef __SIM_PSEUDO_INST_HH__
#define __SIM_PSEUDO_INST_HH__

#include <gem5/asm/generic/m5ops.h>

class ThreadContext;

#include "arch/pseudo_inst.hh"
#include "arch/utility.hh"
#include "base/types.hh" // For Tick and Addr data types.
#include "debug/PseudoInst.hh"
#include "sim/guest_abi.hh"

struct PseudoInstABI
{
    using Position = int;
};

namespace GuestABI
{

template <typename T>
struct Result<PseudoInstABI, T>
{
    static void
    store(ThreadContext *tc, const T &ret)
    {
        // Don't do anything with the pseudo inst results by default.
    }
};

template <>
struct Argument<PseudoInstABI, uint64_t>
{
    static uint64_t
    get(ThreadContext *tc, PseudoInstABI::Position &position)
    {
        uint64_t result = TheISA::getArgument(tc, position, sizeof(uint64_t),
                                              false);
        position++;
        return result;
    }
};

} // namespace GuestABI

namespace PseudoInst
{

static inline void
decodeAddrOffset(Addr offset, uint8_t &func)
{
    func = bits(offset, 15, 8);
}

void arm(ThreadContext *tc);
void quiesce(ThreadContext *tc);
void quiesceSkip(ThreadContext *tc);
void quiesceNs(ThreadContext *tc, uint64_t ns);
void quiesceCycles(ThreadContext *tc, uint64_t cycles);
uint64_t quiesceTime(ThreadContext *tc);
uint64_t readfile(ThreadContext *tc, Addr vaddr, uint64_t len,
    uint64_t offset);
uint64_t writefile(ThreadContext *tc, Addr vaddr, uint64_t len,
    uint64_t offset, Addr filenameAddr);
void loadsymbol(ThreadContext *xc);
void addsymbol(ThreadContext *tc, Addr addr, Addr symbolAddr);
uint64_t initParam(ThreadContext *xc, uint64_t key_str1, uint64_t key_str2);
uint64_t rpns(ThreadContext *tc);
void wakeCPU(ThreadContext *tc, uint64_t cpuid);
void m5exit(ThreadContext *tc, Tick delay);
void m5fail(ThreadContext *tc, Tick delay, uint64_t code);
void resetstats(ThreadContext *tc, Tick delay, Tick period);
void dumpstats(ThreadContext *tc, Tick delay, Tick period);
void dumpresetstats(ThreadContext *tc, Tick delay, Tick period);
void m5checkpoint(ThreadContext *tc, Tick delay, Tick period);
void debugbreak(ThreadContext *tc);
void switchcpu(ThreadContext *tc);
void workbegin(ThreadContext *tc, uint64_t workid, uint64_t threadid);
void workend(ThreadContext *tc, uint64_t workid, uint64_t threadid);
void m5Syscall(ThreadContext *tc);
void togglesync(ThreadContext *tc);

/**
 * Execute a decoded M5 pseudo instruction
 *
 * The ISA-specific code is responsible to decode the pseudo inst
 * function number and subfunction number. After that has been done,
 * the rest of the instruction can be implemented in an ISA-agnostic
 * manner using the ISA-specific getArguments functions.
 *
 * @param func M5 pseudo op major function number (see utility/m5/m5ops.h)
 */

template <typename ABI>
uint64_t
pseudoInst(ThreadContext *tc, uint8_t func)
{
    DPRINTF(PseudoInst, "PseudoInst::pseudoInst(%i)\n", func);

    switch (func) {
      case M5OP_ARM:
        invokeSimcall<ABI>(tc, arm);
        break;

      case M5OP_QUIESCE:
        invokeSimcall<ABI>(tc, quiesce);
        break;

      case M5OP_QUIESCE_NS:
        invokeSimcall<ABI>(tc, quiesceNs);
        break;

      case M5OP_QUIESCE_CYCLE:
        invokeSimcall<ABI>(tc, quiesceCycles);
        break;

      case M5OP_QUIESCE_TIME:
        return invokeSimcall<ABI>(tc, quiesceTime);

      case M5OP_RPNS:
        return invokeSimcall<ABI>(tc, rpns);

      case M5OP_WAKE_CPU:
        invokeSimcall<ABI>(tc, wakeCPU);
        break;

      case M5OP_EXIT:
        invokeSimcall<ABI>(tc, m5exit);
        break;

      case M5OP_FAIL:
        invokeSimcall<ABI>(tc, m5fail);
        break;

      case M5OP_INIT_PARAM:
        return invokeSimcall<ABI>(tc, initParam);

      case M5OP_LOAD_SYMBOL:
        invokeSimcall<ABI>(tc, loadsymbol);
        break;

      case M5OP_RESET_STATS:
        invokeSimcall<ABI>(tc, resetstats);
        break;

      case M5OP_DUMP_STATS:
        invokeSimcall<ABI>(tc, dumpstats);
        break;

      case M5OP_DUMP_RESET_STATS:
        invokeSimcall<ABI>(tc, dumpresetstats);
        break;

      case M5OP_CHECKPOINT:
        invokeSimcall<ABI>(tc, m5checkpoint);
        break;

      case M5OP_WRITE_FILE:
        return invokeSimcall<ABI>(tc, writefile);

      case M5OP_READ_FILE:
        return invokeSimcall<ABI>(tc, readfile);

      case M5OP_DEBUG_BREAK:
        invokeSimcall<ABI>(tc, debugbreak);
        break;

      case M5OP_SWITCH_CPU:
        invokeSimcall<ABI>(tc, switchcpu);
        break;

      case M5OP_ADD_SYMBOL:
        invokeSimcall<ABI>(tc, addsymbol);
        break;

      case M5OP_PANIC:
        panic("M5 panic instruction called at %s\n", tc->pcState());

      case M5OP_WORK_BEGIN:
        invokeSimcall<ABI>(tc, workbegin);
        break;

      case M5OP_WORK_END:
        invokeSimcall<ABI>(tc, workend);
        break;

      case M5OP_ANNOTATE:
      case M5OP_RESERVED2:
      case M5OP_RESERVED3:
      case M5OP_RESERVED4:
      case M5OP_RESERVED5:
        warn("Unimplemented m5 op (%#x)\n", func);
        break;

      /* SE mode functions */
      case M5OP_SE_SYSCALL:
        invokeSimcall<ABI>(tc, m5Syscall);
        break;

      case M5OP_SE_PAGE_FAULT:
        invokeSimcall<ABI>(tc, TheISA::m5PageFault);
        break;

      /* dist-gem5 functions */
      case M5OP_DIST_TOGGLE_SYNC:
        invokeSimcall<ABI>(tc, togglesync);
        break;

      default:
        warn("Unhandled m5 op: %#x\n", func);
        break;
    }

    return 0;
}

} // namespace PseudoInst

#endif // __SIM_PSEUDO_INST_HH__
