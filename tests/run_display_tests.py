#!/usr/bin/env python3
"""Exercise the production display scheduler and extracted SDL timer under ASan/UBSan.
No GUI, guest filesystem, or real-time performance claims. Requires macOS clang++.
"""
from pathlib import Path
import os
import subprocess
import tempfile
repo = Path(__file__).resolve().parents[1]
src = repo/'src/AtariX-MT/AtariX'
text = (src/'EmulationRunner.cpp').read_text()
start = text.index('Uint32 EmulationRunner::LoopTimer(')
end = text.index('\n}', start) + 2
timer = text[start:end]
prefix = r'''
#include "DisplayUpdateScheduler.h"
#include <atomic>
#include <climits>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <vector>
#include <stdatomic.h>
using Uint32 = unsigned;
const int SDL_USEREVENT = 123, RUN_EMULATOR_WINDOW_UPDATE = 1;
struct SDL_Event { int type; struct {int code; void *data1, *data2;} user; };
unsigned checks=0, queued=0, pushCalls=0;
int pushResult=1;
#define CHECK(x) do { ++checks; if (!(x)) { std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x); std::abort(); } } while(0)
int SDL_PushEvent(SDL_Event *event) {
    ++pushCalls;
    CHECK(event->type==SDL_USEREVENT && event->user.code==RUN_EMULATOR_WINDOW_UPDATE);
    CHECK(event->user.data1==nullptr && event->user.data2==nullptr);
    if(pushResult>0) ++queued;
    return pushResult;
}
struct EmulatorStub {
    atomic_char bVideoBufChanged=0;
    unsigned hz200=0, vbl=0;
    void SendHz200() { ++hz200; }
    void SendVBL() { ++vbl; }
};
struct EmulationRunner {
    bool m_EmulatorRunning=true;
    unsigned m_200HzCnt=0;
    EmulatorStub m_Emulator;
    DisplayUpdateScheduler m_displayUpdates;
    static Uint32 LoopTimer(Uint32, void *);
};
'''
tests = r'''
int main() {
    DisplayUpdateScheduler scheduler;
    CHECK(scheduler.GetRefreshRate()==50);
    for(unsigned rate : {0u,1u,24u,26u,49u,51u,UINT_MAX}) {
        scheduler.SetRefreshRate(rate); CHECK(scheduler.GetRefreshRate()==50);
    }
    for(unsigned rate : {25u,50u}) {
        EmulationRunner runner;
        runner.m_displayUpdates.SetRefreshRate(rate);
        queued=0; pushCalls=0; pushResult=1;
        for(unsigned tick=1; tick<=200; ++tick) {
            atomic_store(&runner.m_Emulator.bVideoBufChanged,1);
            CHECK(EmulationRunner::LoopTimer(5,&runner)==5);
            if(tick%(200/rate)==0) {
                CHECK(queued==1); --queued;
                CHECK(atomic_exchange(&runner.m_Emulator.bVideoBufChanged,0)==1);
                runner.m_displayUpdates.FinishUpdate();
            } else CHECK(queued==0);
        }
        CHECK(pushCalls==rate); CHECK(runner.m_Emulator.hz200==200); CHECK(runner.m_Emulator.vbl==50);
        // An idle display does not enqueue, guest clocks continue.
        unsigned before=pushCalls;
        for(int tick=0;tick<200;++tick) EmulationRunner::LoopTimer(5,&runner);
        CHECK(pushCalls==before); CHECK(runner.m_Emulator.hz200==400); CHECK(runner.m_Emulator.vbl==100);
        // Slow/blocked rendering can occupy only one queue slot.
        atomic_store(&runner.m_Emulator.bVideoBufChanged,1);
        for(int tick=0;tick<200;++tick) EmulationRunner::LoopTimer(5,&runner);
        CHECK(queued==1); CHECK(pushCalls==before+1);
        // Writes during rendering remain dirty and cause the next update.
        atomic_exchange(&runner.m_Emulator.bVideoBufChanged,0);
        atomic_store_explicit(&runner.m_Emulator.bVideoBufChanged,1,memory_order_release);
        runner.m_displayUpdates.FinishUpdate(); --queued;
        for(unsigned tick=0;tick<200/rate;++tick) EmulationRunner::LoopTimer(5,&runner);
        CHECK(queued==1); CHECK(atomic_load(&runner.m_Emulator.bVideoBufChanged)==1);
        runner.m_displayUpdates.FinishUpdate(); --queued;
        // Both SDL errors and event filters must release the slot for retry.
        for(int result : {-1,0}) {
            pushResult=result; before=pushCalls;
            for(unsigned tick=0;tick<400/rate;++tick) EmulationRunner::LoopTimer(5,&runner);
            CHECK(queued==0); CHECK(pushCalls==before+2);
        }
        pushResult=1;
        for(unsigned tick=0;tick<200/rate;++tick) EmulationRunner::LoopTimer(5,&runner);
        CHECK(queued==1);
        // Stop leaves both guest counters unchanged.
        runner.m_EmulatorRunning=false;
        auto hz=runner.m_Emulator.hz200, vbl=runner.m_Emulator.vbl;
        for(int tick=0;tick<200;++tick) EmulationRunner::LoopTimer(5,&runner);
        CHECK(runner.m_Emulator.hz200==hz && runner.m_Emulator.vbl==vbl);
    }
    // Rate changes cannot overwrite an outstanding slot; wraparound still schedules.
    scheduler.SetRefreshRate(25); CHECK(scheduler.TrySchedule(8,true));
    scheduler.SetRefreshRate(50); CHECK(!scheduler.TrySchedule(12,true));
    scheduler.FinishUpdate(); CHECK(scheduler.TrySchedule(12,true));
    scheduler.FinishUpdate(); CHECK(!scheduler.TrySchedule(UINT_MAX,true));
    CHECK(scheduler.TrySchedule(0,true)); scheduler.FinishUpdate();
    // Concurrent claims have exactly one winner, even during preference changes.
    for(unsigned round=0;round<100;++round) {
        std::atomic<unsigned> winners{0}; std::vector<std::thread> threads;
        for(unsigned i=0;i<8;++i) threads.emplace_back([&,i] {
            scheduler.SetRefreshRate(i%2 ? 25:50);
            if(scheduler.TrySchedule(8,true)) winners.fetch_add(1);
        });
        for(auto &thread:threads) thread.join();
        CHECK(winners.load()==1); scheduler.FinishUpdate();
    }
    std::printf("PASS: %u display/timer assertions (ASan + UBSan)\n", checks);
}
'''
with tempfile.TemporaryDirectory(prefix='atarix-display-tests-') as tmp:
    path = Path(tmp)
    (path/'tests.cpp').write_text(prefix+'\n'+timer+'\n'+tests)
    subprocess.run(['clang++','-std=c++17','-O1','-g','-fsanitize=address,undefined',
                    '-fno-omit-frame-pointer','-pthread','-I'+str(src),str(path/'tests.cpp'),
                    '-o',str(path/'tests')],check=True)
    subprocess.run([str(path/'tests')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
