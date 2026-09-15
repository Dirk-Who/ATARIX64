#!/usr/bin/env python3
"""ASan/UBSan regressions for actual AtariX wheel, ring, IRQ and mouse code.

Extracts production methods into a controlled emulator fixture; uses the real
recursive mutex, SDL scancodes, scancode conversion and CMagiCMouse. Only the
68k CPU/IRQ execution is stubbed. No application or guest filesystem is opened.
Use --repo PATH --characterize-baseline to reproduce both 0.7.2 bugs.
"""
from pathlib import Path
import argparse, os, re, subprocess, tempfile
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--repo',type=Path,default=Path(__file__).resolve().parents[1])
parser.add_argument('--characterize-baseline',action='store_true')
args=parser.parse_args(); repo=args.repo.resolve(); src=repo/'src/AtariX-MT/AtariX'
magic=(src/'MagiC.cpp').read_text(); runner=(src/'EmulationRunner.cpp').read_text()
def extract(name,signature):
    text=(src/name).read_text();start=text.index(signature);end=text.index('\n}',text.index('{',start))+2
    return text[start:end]+'\n'
code=r'''
#include <atomic>
#include <cassert>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <vector>
#include <CoreFoundation/CoreFoundation.h>
#include <SDL2/SDL.h>
#include "Atari.h"
#include "ModernSync.h"
#include "MagiCMouse.h"
#include "MagiCKeyboard.h"
#include "s_endian.h"
#define DebugError(...) ((void)0)
#define DebugWarning(...) ((void)0)
#define DebugInfo(...) ((void)0)
#define M68K_IRQ_6 6
#define EMU_INTPENDING_KBMOUSE 1
std::atomic<unsigned> checks{0}, irqCalls{0}, exitCalls{0};
#define CHECK(x) do { ++checks; if(!(x)) { std::fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x); std::abort(); } } while(0)
void m68k_set_irq(int irq) { CHECK(irq==6); ++irqCalls; }
void m68k_end_timeslice() { ++exitCalls; }
CMagiCKeyboard::CMagiCKeyboard() {}
CMagiCKeyboard::~CMagiCKeyboard() {}
'''
code+=re.search(r'^#define KEYBOARDBUFLEN\s+\d+', (src/'MagiC.h').read_text(), re.M).group()+'\n'
for sig in ['static inline void OS_EnterCriticalRegion(', 'static inline void OS_ExitCriticalRegion(', 'static inline void OS_SetEvent(']: code+=extract('MagiC.cpp',sig)
code+=r'''
struct CMagiC {
    unsigned char m_cKeyboardOrMouseData[KEYBOARDBUFLEN]{};
    unsigned char *m_pKbRead=m_cKeyboardOrMouseData, *m_pKbWrite=m_cKeyboardOrMouseData;
    std::atomic_bool m_bEmulatorIsRunning{true}, m_bInterruptMouseKeyboardPending{false};
    bool m_bInterruptPending=false, m_bInterruptMouseButton[2]={false,false};
    AtariPoint m_InterruptMouseWhere{0,0};
    AtariRecursiveMutex m_KbCriticalRegion;
    AtariEvent m_InterruptEvent;
    CMagiCMouse m_MagiCMouse;
    CMagiCKeyboard m_MagiCKeyboard;
    alignas(8) unsigned char lineMemory[2048]{};
    unsigned char *m_LineAVars=lineMemory+1024;
    CMagiC() { m_MagiCMouse.Init(m_LineAVars,{0,0}); m_MagiCMouse.SetNewPosition({0,0}); }
    int GetKbBufferFree(void);
    void PutKeyToBuffer(unsigned char);
    int SendSdlKeyboard(int,bool);
    int SendSdlKeyboardPair(int);
    int SendMousePosition(int,int);
    int SendMouseButton(unsigned,bool);
    void PrepareMouseKeyboardInterrupt(void);
    uint32_t AtariGetKeyboardOrMouseData(uint32_t,unsigned char*);
    void fill(unsigned free,unsigned offset=0) {
        m_pKbRead=m_pKbWrite=m_cKeyboardOrMouseData+offset;
        for(unsigned i=0;i<KEYBOARDBUFLEN-1-free;++i) PutKeyToBuffer(0x1e);
    }
};
'''
for sig in ['int CMagiC::GetKbBufferFree(', 'void CMagiC::PutKeyToBuffer(', 'int CMagiC::SendSdlKeyboard(', 'int CMagiC::SendMousePosition(', 'int CMagiC::SendMouseButton(', 'uint32_t CMagiC::AtariGetKeyboardOrMouseData(']: code+=extract('MagiC.cpp',sig)
code+=extract('MagiCKeyboard.cpp','unsigned char CMagiCKeyboard::SdlScanCode2AtariScanCode(')
if args.characterize_baseline:
    start=magic.index('if\t(m_bInterruptMouseKeyboardPending)',magic.index('void CMagiC::EmuThread'))
    start=magic.index('{',start)+1; end=magic.index('m_bWaitEmulatorForIRQCallback = true;',start)
    code+='void CMagiC::PrepareMouseKeyboardInterrupt() { bool bNewBstate[2], bNewMpos, bNewKey;\n'+magic[start:end]+'}\n'
else:
    code+=extract('MagiC.cpp','void CMagiC::PrepareMouseKeyboardInterrupt(')
    code+=extract('MagiC.cpp','int CMagiC::SendSdlKeyboardPair(')
    assert 'PrepareMouseKeyboardInterrupt();' in extract('MagiC.cpp','void CMagiC::EmuThread(')
start=runner.index('\t\tcase SDL_MOUSEWHEEL:'); end=runner.index('\t\tcase SDL_TEXTEDITING:',start)
code+='struct WheelFixture { CMagiC &m_Emulator; void wheel(int x,int y,bool flipped=false) { SDL_Event event{}; event.type=SDL_MOUSEWHEEL; event.wheel.x=x; event.wheel.y=y; event.wheel.direction=flipped?SDL_MOUSEWHEEL_FLIPPED:SDL_MOUSEWHEEL_NORMAL; switch(event.type) {\n'+runner[start:end]+' default: break; } } };\n'
code+=r'''
std::vector<unsigned> drain(CMagiC &c) {
    std::vector<unsigned> bytes;
    while(c.AtariGetKeyboardOrMouseData(1,nullptr)) {
        CHECK(bytes.size()<1024);
        bytes.push_back(c.AtariGetKeyboardOrMouseData(0,nullptr));
    }
    return bytes;
}
'''
if args.characterize_baseline:
    code+=r'''
int main() {
    CMagiC wheel; wheel.fill(1); WheelFixture{wheel}.wheel(0,-1);
    auto bytes=drain(wheel); CHECK(bytes.size()==31); CHECK(bytes.back()==0x50);
    CHECK(wheel.GetKbBufferFree()==31);
    for(unsigned free: {0u,1u,2u}) {
        CMagiC c; c.fill(free); c.SendMousePosition(120,55); c.SendMouseButton(0,true);
        auto before=irqCalls.load(); c.PrepareMouseKeyboardInterrupt();
        CHECK(irqCalls==before); CHECK(!c.m_bInterruptMouseKeyboardPending);
        CHECK(c.m_pKbRead!=c.m_pKbWrite);
    }
    std::printf("PASS baseline characterization: orphan wheel press and suppressed full-buffer IRQ reproduced (%u checks)\n",checks.load());
}
'''
else:
    code+=(Path(__file__).parent/'input_tests.cpp').read_text()
with tempfile.TemporaryDirectory(prefix='atarix-input-tests-') as tmp:
    tmp=Path(tmp); (tmp/'input.cpp').write_text(code)
    subprocess.run(['clang++','-std=c++17','-O1','-g','-pthread','-fsanitize=address,undefined',
                    '-fno-omit-frame-pointer','-Wno-deprecated-declarations','-I'+str(src),'-F'+str(src),
                    str(tmp/'input.cpp'),str(src/'MagiCMouse.cpp'),'-framework','CoreFoundation',
                    '-o',str(tmp/'input')],check=True)
    subprocess.run([str(tmp/'input')],check=True,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
