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
 * Copyright (c) 2004-2006 The Regents of The University of Michigan
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
 * Authors: Kevin Lim
 *          Korey Sewell
 */

#ifndef __CPU_O3_ROB_IMPL_HH__
#define __CPU_O3_ROB_IMPL_HH__

#include <list>

#include "cpu/o3/rob.hh"
#include "debug/Fetch.hh"
#include "debug/ROB.hh"
#include "debug/DumpROB.hh"
#include "debug/DumpROB_show_addr.hh"
#include "debug/DumpROB_showSrcRegs.hh"
#include "params/DerivO3CPU.hh"

using namespace std;

template <class Impl>
ROB<Impl>::ROB(O3CPU *_cpu, DerivO3CPUParams *params)
    : cpu(_cpu),
      numEntries(params->numROBEntries),
      squashWidth(params->squashWidth),
      numInstsInROB(0),
      numThreads(params->numThreads)
{

    std::string policy = params->smtROBPolicy;

    //Convert string to lowercase
    std::transform(policy.begin(), policy.end(), policy.begin(),
                   (int(*)(int)) tolower);

    //Figure out rob policy
    if (policy == "dynamic") {
        robPolicy = Dynamic;

        //Set Max Entries to Total ROB Capacity
        for (ThreadID tid = 0; tid < numThreads; tid++) {
            maxEntries[tid] = numEntries;
        }

    } else if (policy == "partitioned") {
        robPolicy = Partitioned;
        DPRINTF(Fetch, "ROB sharing policy set to Partitioned\n");

        //@todo:make work if part_amt doesnt divide evenly.
        int part_amt = numEntries / numThreads;

        //Divide ROB up evenly
        for (ThreadID tid = 0; tid < numThreads; tid++) {
            maxEntries[tid] = part_amt;
        }

    } else if (policy == "threshold") {
        robPolicy = Threshold;
        DPRINTF(Fetch, "ROB sharing policy set to Threshold\n");

        int threshold =  params->smtROBThreshold;;

        //Divide up by threshold amount
        for (ThreadID tid = 0; tid < numThreads; tid++) {
            maxEntries[tid] = threshold;
        }
    } else {
        assert(0 && "Invalid ROB Sharing Policy.Options Are:{Dynamic,"
                    "Partitioned, Threshold}");
    }

    resetState();
}

template <class Impl>
void
ROB<Impl>::resetState()
{
    for (ThreadID tid = 0; tid  < numThreads; tid++) {
        doneSquashing[tid] = true;
        threadEntries[tid] = 0;
        squashIt[tid] = instList[tid].end();
        squashedSeqNum[tid] = 0;
    }
    numInstsInROB = 0;

    // Initialize the "universal" ROB head & tail point to invalid
    // pointers
    head = instList[0].end();
    tail = instList[0].end();
}

template <class Impl>
std::string
ROB<Impl>::name() const
{
    return cpu->name() + ".rob";
}

template <class Impl>
void
ROB<Impl>::setActiveThreads(list<ThreadID> *at_ptr)
{
    DPRINTF(ROB, "Setting active threads list pointer.\n");
    activeThreads = at_ptr;
}

template <class Impl>
void
ROB<Impl>::drainSanityCheck() const
{
    for (ThreadID tid = 0; tid  < numThreads; tid++)
        assert(instList[tid].empty());
    assert(isEmpty());
}

template <class Impl>
std::string 
ROB<Impl>::InstToPCStr(DynInstPtr& inst)
{
    std::ostringstream os;
    os << inst->pcState();
    return os.str();
}

template <class Impl>
void
ROB<Impl>::PrintROBContents( uint32_t tid, IQ& instQueue )
{
    int slotIdx = 0;
    DPRINTF( DumpROB, "=============== Dumping ROB contents =============== \n");


    for(InstIt it = instList[tid].begin(); it != instList[tid].end(); ++it) {
        DPRINTF( DumpROB, "%3d:%s [sn:%lli] %s/%s/%s/%s%7s" 
                            "%12s%i %s/%s/%s %3d%-35s %s%s\n",
                slotIdx++,
                DTRACE( DumpROB_show_addr ) ? InstToPCStr(*it) : "",
                (*it)->seqNum,
                /* (*it)->staticInst->getName(), */
                /* (*it)->threadNumber, */
                (*it)->readyToIssue() ? "RDY" : "---",
                (*it)->isIssued() ? "ISS" : "---",
                (*it)->isExecuted() ? "XED" : "---",
                (*it)->readyToCommit() ? "CMT" : "---",
                (*it)->isSquashed() ? "SQSHED" : "------",
                (*it)->isControlInducer(),
                (*it)->staticInst->disassemble( (*it)->instAddr() ),
                DTRACE( DumpROB_showSrcRegs ) ? instQueue.getSrcInstructionsStr(*it) : "",
                (*it)==(*head) ? " <= HEAD" : ((*it)==(*tail) ? " <= TAIL" : "" )
        );
    }
    (void) slotIdx;
    DPRINTF( DumpROB, "=============== End of ROB contents =============== \n" ); 
}

template <class Impl>
void
ROB<Impl>::takeOverFrom()
{
    resetState();
}

template <class Impl>
void
ROB<Impl>::resetEntries()
{
    if (robPolicy != Dynamic || numThreads > 1) {
        int active_threads = activeThreads->size();

        list<ThreadID>::iterator threads = activeThreads->begin();
        list<ThreadID>::iterator end = activeThreads->end();

        while (threads != end) {
            ThreadID tid = *threads++;

            if (robPolicy == Partitioned) {
                maxEntries[tid] = numEntries / active_threads;
            } else if (robPolicy == Threshold && active_threads == 1) {
                maxEntries[tid] = numEntries;
            }
        }
    }
}

template <class Impl>
int
ROB<Impl>::entryAmount(ThreadID num_threads)
{
    if (robPolicy == Partitioned) {
        return numEntries / num_threads;
    } else {
        return 0;
    }
}

template <class Impl>
int
ROB<Impl>::countInsts()
{
    int total = 0;

    for (ThreadID tid = 0; tid < numThreads; tid++)
        total += countInsts(tid);

    return total;
}

template <class Impl>
int
ROB<Impl>::countInsts(ThreadID tid)
{
    return instList[tid].size();
}

template <class Impl>
void
ROB<Impl>::insertInst(DynInstPtr &inst, IQ& instQueue, LSQ& lsq)
{
    assert(inst);

    robWrites++;

    DPRINTF(ROB, "Adding inst [sn:%i] PC %s to the ROB.\n", inst->seqNum, inst->pcState());

    assert(numInstsInROB != numEntries);

    ThreadID tid = inst->threadNumber;

    instList[tid].push_back(inst);

    //Set Up head iterator if this is the 1st instruction in the ROB
    if (numInstsInROB == 0) {
        head = instList[tid].begin();
        assert((*head) == inst);
    }

    //Must Decrement for iterator to actually be valid  since __.end()
    //actually points to 1 after the last inst
    tail = instList[tid].end();
    tail--;

    inst->setInROB();

    // DOLMA: set control restriction/inducer logic (data logic handled during broadcast)
    if (cpu->isDolma() && !inst->isSquashed()) {

        InstIt end = instList[tid].end();

        if (!cpu->isDolmaMemOnly()) {
            for (InstIt it = instList[tid].begin(); it != end; ++it) {
                if ((*it)->isControlInducer()) {
                    inst->setControlRestricted();
                    break;
                }
            }
        }

        // DOLMA: now we must determine whether this inst is a control inducer.
        // All control instructions that depend on runtime args (i.e., indirect or conditional ctrl)
        // are control inducers. Put another way, direct-unconditional branches are not control
        // inducers, because (on a real processor) they can be resolved at decode (before younger
        // instructions can execute). Indeed, this logic exists for this processor in decode_impl.hh.
        // Unfortunately, gem5 seems to have a bug where direct/conditional info isn't provided for X86,
        // possibly because the branchTarget() function used in decode_impl is unimplemented for X86.
        // What follows is thus semi-complicated logic that determines whether an x86 ctrl
        // inst is a control inducer based on its mnemonic.

        if (inst->isControl()) {
            bool isControlInducer = false;

            bool hasCtrlInfo = (inst->isDirectCtrl() != inst->isIndirectCtrl()) &&
                (inst->isUncondCtrl() != inst->isCondCtrl());
            if (hasCtrlInfo) {
                // simple case: we have the needed ctrl info gem5 is supposed to provide
                isControlInducer = !(inst->isDirectCtrl() && inst->isUncondCtrl());
            }
            else {
                isControlInducer = true;
                // tougher case: we must determine it ourselves b/c of the gem5 bug
                const std::string &mnemonic_ = inst->staticInst->disassemble(inst->instAddr());
                std::string mnemonic = mnemonic_;
                while(mnemonic.size() && std::isspace(mnemonic.front())) {
                    mnemonic.erase(0,1);
                }

                std::string inst_name = mnemonic.substr(0, mnemonic.find(" "));

                // First, figure out direct/indirect
                // Besides returns, X86 direct jumps end with "_I" (immediate) on gem5
                std::string substr = "_I";

                if (!inst->isReturn()) {
                    if (inst_name.length() >= substr.length() &&
                        (0 == inst_name.compare(inst_name.length() - substr.length(), substr.length(), substr))) {
                        // now we know it's direct; determine if uncond
                        // Besides calls, X86 uncond jumps start with "JMP" on gem5
                        if (inst->isCall()) {
                            isControlInducer = false;
                        }
                        else {
                            substr = "JMP";
                            if (0 == inst_name.compare(0, substr.length(), substr)) {
                                isControlInducer = false;
                            }
                        }
                    }
                }
            }
            if (isControlInducer) {
                inst->setControlInducer();
            }
        }
    }

    ++numInstsInROB;
    ++threadEntries[tid];

    assert((*tail) == inst);

    DPRINTF(ROB, "[tid:%i] Now has %d instructions.\n", tid, threadEntries[tid]);
}

// DOLMA: for updating micro-op safety status
template <class Impl>
typename Impl::DynInstPtr
ROB<Impl>::getResolvedRedirect(ThreadID tid)
{

    InstIt end = instList[tid].end();

    for (InstIt it = instList[tid].begin(); it != end; ++it) {
        DynInstPtr &inst = *it;
        if (!inst->isDolmaRestricted()) {
            if (inst->isPendingBranch()) {
                return inst;
            }
            // STT doesn't wait for the store to resolve
            else if (inst->isPendingMemOrder() && (inst->getColliderPC() || cpu->isSTT())) {
                return inst;
            }
        }
    }
    return NULL;
}

// DOLMA: for updating micro-op safety status
template <class Impl>
void
ROB<Impl>::updateSafeStatus(ThreadID tid)
{
    // can't clear control dependants once found unresolved branch
    bool foundUnresolvedBranch = false;
    bool foundUnresolvedStore = false;

    // can only clear data dependants whose inducers aren't in this list
    std::set<InstSeqNum> ydis;
    InstSeqNum oldestViolator = (InstSeqNum) -1;

    InstIt end = instList[tid].end();

    for (InstIt it = instList[tid].begin(); it != end; ++it) {
        DynInstPtr &inst = *it;
        
        if (inst->isDolmaRestricted()) {
            // control restrictions can be cleared when no unresolved branches precede inst
            if (inst->isControlRestricted() && !foundUnresolvedBranch) {
                inst->clearControlRestricted();
            }
            // data restrictions can be cleared when not dependent on unresolved data inducer
            if (inst->isDataRestricted() && ydis.find(inst->ydi) == ydis.end()) {
                inst->clearDataRestricted();
            }

            // both control and data dependency must be cleared for op to be safe
            if (!inst->isDolmaRestricted()) {
                // STT stalls, so doesn't need to do metadata update
                if (!cpu->isSTT() && !inst->isDolmaStalled() && inst->dolmaVirtualReq) {
                    inst->setPendingMetadata();
                }
                if (inst->isDolmaStalled()) {
                    inst->clearDolmaStalled();
                }
                else if (inst->isStore() && !cpu->isSTT()) {
                    DynInstPtr violator = inst->getViolator();
                    if (violator && !violator->isSquashed()) {
                        violator->setPendingMemOrder(inst);
                    }
                }
            }
        }

        if (inst->isDataInducer()) {
            if (cpu->isDolmaConservative() || inst->isPendingMemOrder()) {
                ydis.insert(inst->seqNum);
            }
            else if (cpu->isDolmaMemOnly() && foundUnresolvedBranch) {
                ydis.insert(inst->seqNum);
            }
            else if (!cpu->isSTT() && (foundUnresolvedStore || inst->seqNum >= oldestViolator)) {
                ydis.insert(inst->seqNum);
            }
            else {
                inst->clearDataInducer();
            }
        }
        if (inst->isControlInducer()) {
            foundUnresolvedBranch = true;
        }
        if (inst->isStore() && !inst->isSquashed() && !cpu->isSTT() && !cpu->isDolmaConservative()) {
            if (!inst->effAddrValid()) {
                foundUnresolvedStore = true;
            }
            else if (inst->violatorSeqNum && inst->violatorSeqNum < oldestViolator) {
                DynInstPtr violator = findInst(tid, inst->violatorSeqNum);
                if (violator) {
                    if (!violator->isSquashed()) {
                        oldestViolator = inst->violatorSeqNum;
                    }
                    else {
                        inst->violatorSeqNum = 0;
                    }
                }
                else {
                    inst->violatorSeqNum = 0;
                }
            }
        }
    }
}


template <class Impl>
void
ROB<Impl>::retireHead(  ThreadID tid, 
                        IEW *iewStage)
{
    robWrites++;

    assert(numInstsInROB > 0);

    // Get the head ROB instruction.
    InstIt head_it = instList[tid].begin();
    InstIt post_head_it = head_it;
    post_head_it++;

    DynInstPtr head_inst = (*head_it);

    assert(head_inst->readyToCommit());

    DPRINTF(ROB, "[tid:%u]: Retiring head %s instruction, "
            "instruction PC %s, [sn:%lli] %s\n", 
            tid, 
            head_inst->isSquashed() ? "SQUASHED" : "PENDING",
            head_inst->pcState(),
            head_inst->seqNum, 
            head_inst->staticInst->disassemble(head_inst->pcState().pc()));

    if (head_inst->isPendingMetadata()) {
        iewStage->updateMetadataIfNeeded(head_inst);
    }

    // data inducers resolve at retirement in dolma conservative
    if (head_inst->isDataInducer()) {
        assert(cpu->isDolmaConservative());
        head_inst->clearDataInducer();
        updateSafeStatus(tid);
    }

    --numInstsInROB;
    --threadEntries[tid];
    
    head_inst->clearInROB();
    head_inst->setCommitted();

    instList[tid].erase(head_it);

    //Update "Global" Head of ROB
    updateHead(iewStage->instQueue);

    // @todo: A special case is needed if the instruction being
    // retired is the only instruction in the ROB; otherwise the tail
    // iterator will become invalidated.
    cpu->removeFrontInst(head_inst);
}

template <class Impl>
bool
ROB<Impl>::isHeadReady(ThreadID tid)
{
    robReads++;
    if (threadEntries[tid] != 0) {
        return instList[tid].front()->readyToCommit();
    }

    return false;
}

template <class Impl>
bool
ROB<Impl>::canCommit()
{
    //@todo: set ActiveThreads through ROB or CPU
    list<ThreadID>::iterator threads = activeThreads->begin();
    list<ThreadID>::iterator end = activeThreads->end();

    while (threads != end) {
        ThreadID tid = *threads++;

        if (isHeadReady(tid)) {
            return true;
        }
    }

    return false;
}

template <class Impl>
unsigned
ROB<Impl>::numFreeEntries()
{
    return numEntries - numInstsInROB;
}

template <class Impl>
unsigned
ROB<Impl>::numFreeEntries(ThreadID tid)
{
    return maxEntries[tid] - threadEntries[tid];
}

template <class Impl>
void
ROB<Impl>::doSquash(ThreadID tid, IQ& instQueue, Scoreboard* scoreboard)
{
    robWrites++;

    DPRINTF(ROB, "[tid:%u]: Squashing instructions until [sn:%i].\n",
            tid, squashedSeqNum[tid]);

    assert(squashIt[tid] != instList[tid].end());

    if ((*squashIt[tid])->seqNum < squashedSeqNum[tid]) {
        DPRINTF(ROB, "[tid:%u]: Done squashing instructions.\n",
                tid);
        squashIt[tid] = instList[tid].end();

        doneSquashing[tid] = true;
        return;
    }

    bool robTailUpdate = false;

    for (int numSquashed = 0;
         numSquashed < squashWidth &&
         squashIt[tid] != instList[tid].end() &&
         (*squashIt[tid])->seqNum > squashedSeqNum[tid];
         ++numSquashed)
    {
        DPRINTF(ROB, "[tid:%u]: Squashing instruction PC %s, seq num %i.\n",
                (*squashIt[tid])->threadNumber,
                (*squashIt[tid])->pcState(),
                (*squashIt[tid])->seqNum);

        // Mark the instruction as squashed, and ready to commit so that
        // it can drain out of the pipeline.
        (*squashIt[tid])->setSquashed();
        (*squashIt[tid])->setCanCommit();

        if (squashIt[tid] == instList[tid].begin()) {
            DPRINTF(ROB, "Reached head of instruction list while "
                    "squashing.\n");

            squashIt[tid] = instList[tid].end();

            doneSquashing[tid] = true;

            return;
        }

        InstIt tail_thread = instList[tid].end();
        tail_thread--;

        if ((*squashIt[tid]) == (*tail_thread))
            robTailUpdate = true;

        squashIt[tid]--;
    }


    // Check if ROB is done squashing.
    if ((*squashIt[tid])->seqNum <= squashedSeqNum[tid]) {
        DPRINTF(ROB, "[tid:%u]: Done squashing instructions.\n",
                tid);

        squashIt[tid] = instList[tid].end();

        doneSquashing[tid] = true;
    }

    if (robTailUpdate) {
        updateTail();
    }
}


template <class Impl>
void
ROB<Impl>::updateHead(IQ& instQueue)
{
    InstSeqNum lowest_num = 0;
    bool first_valid = true;

    // @todo: set ActiveThreads through ROB or CPU
    list<ThreadID>::iterator threads = activeThreads->begin();
    list<ThreadID>::iterator end = activeThreads->end();

    while (threads != end) {
        ThreadID tid = *threads++;

        if (instList[tid].empty())
            continue;

        InstIt head_thread = instList[tid].begin();

        DynInstPtr head_inst = (*head_thread);

        assert(head_inst != 0);

        if (first_valid) {
            head = instList[tid].begin();
            lowest_num = (*head)->seqNum;
            first_valid = false;
            continue;
        }

        if (head_inst->seqNum < lowest_num) {
            head = head_thread;
            lowest_num = head_inst->seqNum;
        }
    }

    if (first_valid) {
        head = instList[0].end();
    }
}

template <class Impl>
void
ROB<Impl>::updateTail()
{
    tail = instList[0].end();
    bool first_valid = true;

    list<ThreadID>::iterator threads = activeThreads->begin();
    list<ThreadID>::iterator end = activeThreads->end();

    while (threads != end) {
        ThreadID tid = *threads++;

        if (instList[tid].empty()) {
            continue;
        }

        // If this is the first valid then assign w/out
        // comparison
        if (first_valid) {
            tail = instList[tid].end();
            tail--;
            first_valid = false;
            continue;
        }

        // Assign new tail if this thread's tail is younger
        // than our current "tail high"
        InstIt tail_thread = instList[tid].end();
        tail_thread--;

        if ((*tail_thread)->seqNum > (*tail)->seqNum) {
            tail = tail_thread;
        }
    }
}


template <class Impl>
void
ROB<Impl>::squash(InstSeqNum    squash_num, 
                  ThreadID      tid, 
                  IQ&           instQueue, 
                  Scoreboard*   scoreboard)
{
    if (isEmpty(tid)) {
        DPRINTF(ROB, "Does not need to squash due to being empty "
                "[sn:%i]\n",
                squash_num);

        return;
    }

    DPRINTF(ROB, "Starting to squash within the ROB.\n");

    robStatus[tid] = ROBSquashing;

    doneSquashing[tid] = false;

    squashedSeqNum[tid] = squash_num;

    if (!instList[tid].empty()) {
        InstIt tail_thread = instList[tid].end();
        tail_thread--;

        squashIt[tid] = tail_thread;

        doSquash(tid, instQueue, scoreboard);
    }
}

template <class Impl>
typename Impl::DynInstPtr
ROB<Impl>::readHeadInst(ThreadID tid)
{
    if (threadEntries[tid] != 0) {
        InstIt head_thread = instList[tid].begin();

        assert((*head_thread)->isInROB());

        return *head_thread;
    } else {
        return dummyInst;
    }
}

template <class Impl>
typename Impl::DynInstPtr
ROB<Impl>::readTailInst(ThreadID tid)
{
    InstIt tail_thread = instList[tid].end();
    tail_thread--;

    return *tail_thread;
}

template <class Impl>
void
ROB<Impl>::regStats()
{
    using namespace Stats;
    robReads
        .name(name() + ".rob_reads")
        .desc("The number of ROB reads");

    robWrites
        .name(name() + ".rob_writes")
        .desc("The number of ROB writes");
    
    robNumEntries_accumulator
        .name(name() + ".robNumEntries_accumulator")
        .desc("An accumulator of number of entries in ROB. divide this number in "
                "number of ticks to get an average.");
}

template <class Impl>
typename Impl::DynInstPtr
ROB<Impl>::findInst(ThreadID tid, InstSeqNum squash_inst)
{
    for (InstIt it = instList[tid].begin(); it != instList[tid].end(); it++) {
        if ((*it)->seqNum == squash_inst) {
            return *it;
        }
    }
    return NULL;
}

#endif//__CPU_O3_ROB_IMPL_HH__