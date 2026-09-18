#!/usr/bin/env python3
"""Test production motion handler with real SDL logical event filtering, no GUI.

--baseline-zip checks that the released source fails the same edge regression.
"""
from pathlib import Path
import argparse
import os
import subprocess
import tempfile
import zipfile

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--baseline-zip', type=Path)
args = parser.parse_args()
repo = Path(__file__).resolve().parents[1]
src = repo / 'src/AtariX-MT/AtariX'
runner = (src / 'EmulationRunner.cpp').read_text()
if args.baseline_zip:
    with zipfile.ZipFile(args.baseline_zip) as z:
        runner = z.read('ATARIX-0.7.3/src/AtariX-MT/AtariX/EmulationRunner.cpp').decode()
motion = runner[runner.index('\t\tcase SDL_MOUSEMOTION:'):runner.index('\t\tcase SDL_MOUSEBUTTONDOWN:')]
code = r'''
#include <SDL2/SDL.h>
#include <cstdio>
#include <cstdlib>
int failures=0, checks=0;
#define CHECK(c) do { ++checks; if(!(c)) { ++failures; fprintf(stderr,"FAIL %d: %s\n",__LINE__,#c); } } while(0)
struct Guest { int x=-1,y=-1; void SendMousePosition(int a,int b) { x=a; y=b; } };
struct Fixture {
    SDL_Renderer *m_sdl_renderer;
    bool m_relativeMouseMode=false, m_atariScreenStretchX=false, m_atariScreenStretchY=false;
    float m_virtualMouseX=0, m_virtualMouseY=0;
    int m_atariScreenW=640, m_atariScreenH=480, m_lastSentMouseX=-1, m_lastSentMouseY=-1;
    Guest m_Emulator;
    void _UpdateHostCursorVisibility() {}
    void handle(SDL_Event event) { switch(event.type) {
''' + motion + r'''
    default: break;
    } }
};
int main() {
    SDL_SetHint(SDL_HINT_VIDEODRIVER,"dummy");
    SDL_SetHint(SDL_HINT_MOUSE_RELATIVE_SCALING,"0");
    if(SDL_Init(SDL_INIT_VIDEO)!=0) { fprintf(stderr,"%s\n",SDL_GetError()); return 2; }
    // 660x495 is a 3.125% enlarged 640x480 image: old code loses 15 bottom pixels.
    const int sizes[][2]={{640,480},{660,495},{1280,960},{800,600},{800,640},{900,600}};
    for(auto &size:sizes) for(int stretch=0;stretch<4;++stretch) {
        SDL_Window *w=SDL_CreateWindow("test",0,0,size[0],size[1],SDL_WINDOW_HIDDEN);
        SDL_Renderer *r=SDL_CreateRenderer(w,-1,SDL_RENDERER_SOFTWARE);
        if(!w || !r) { fprintf(stderr,"%s\n",SDL_GetError()); return 2; }
        Fixture f; f.m_sdl_renderer=r;
        f.m_atariScreenStretchX=(stretch&1)!=0; f.m_atariScreenStretchY=(stretch&2)!=0;
        CHECK(SDL_RenderSetLogicalSize(r,640*(f.m_atariScreenStretchX?2:1),480*(f.m_atariScreenStretchY?2:1))==0);
        // Translate target guest points to window coordinates, then send real SDL events.
        const int points[][2]={{0,0},{639,479},{0,479},{639,0},{320,240}};
        for(auto &point:points) {
            int wx,wy;
            SDL_RenderLogicalToWindow(r,point[0]*(f.m_atariScreenStretchX?2:1),point[1]*(f.m_atariScreenStretchY?2:1),&wx,&wy);
            // Ceil to the first physical pixel inside the target logical pixel.
            float lx,ly; SDL_RenderWindowToLogical(r,wx,wy,&lx,&ly);
            if(lx+0.001f<point[0]*(f.m_atariScreenStretchX?2:1)) ++wx;
            if(ly+0.001f<point[1]*(f.m_atariScreenStretchY?2:1)) ++wy;
            CHECK(wx>=0 && wx<size[0] && wy>=0 && wy<size[1]);
            SDL_FlushEvents(SDL_FIRSTEVENT,SDL_LASTEVENT);
            SDL_Event e{}; e.type=SDL_MOUSEMOTION; e.motion.windowID=SDL_GetWindowID(w); e.motion.x=wx; e.motion.y=wy;
            CHECK(SDL_PushEvent(&e)==1);
            CHECK(SDL_PeepEvents(&e,1,SDL_GETEVENT,SDL_MOUSEMOTION,SDL_MOUSEMOTION)==1);
            f.handle(e);
            // Downscaled images cannot represent every interior guest pixel.
            // All four corners must still be reachable exactly.
            const int tolerance=(point[0]==320 && point[1]==240)?1:0;
            if(abs(f.m_Emulator.x-point[0])>tolerance || abs(f.m_Emulator.y-point[1])>tolerance)
                fprintf(stderr,"window=%dx%d stretch=%d target=%d,%d physical=%d,%d SDL=%d,%d guest=%d,%d\n",size[0],size[1],stretch,point[0],point[1],wx,wy,e.motion.x,e.motion.y,f.m_Emulator.x,f.m_Emulator.y);
            CHECK(abs(f.m_Emulator.x-point[0])<=tolerance); CHECK(abs(f.m_Emulator.y-point[1])<=tolerance);
        }
        // Relative mode still converts unscaled deltas and clamps at both edges.
        f.m_relativeMouseMode=true;
        SDL_Event e{}; e.type=SDL_MOUSEMOTION; e.motion.xrel=10000; e.motion.yrel=10000;
        f.handle(e); CHECK(f.m_Emulator.x==639 && f.m_Emulator.y==479);
        e.motion.xrel=-10000; e.motion.yrel=-10000;
        f.handle(e); CHECK(f.m_Emulator.x==0 && f.m_Emulator.y==0);
        SDL_DestroyRenderer(r); SDL_DestroyWindow(w);
    }
    SDL_Quit(); printf("Mouse coordinates: %d checks, %d failures\n",checks,failures);
    return failures?1:0;
}
'''
with tempfile.TemporaryDirectory(prefix='atarix-mouse-test-') as tmp:
    tmp = Path(tmp)
    (tmp/'mouse.cpp').write_text(code)
    subprocess.run(['clang++','-std=c++17','-fsanitize=address,undefined','-F'+str(src),
                    str(tmp/'mouse.cpp'),'-framework','SDL2','-Wl,-rpath,'+str(src),
                    '-o',str(tmp/'mouse')],check=True)
    result=subprocess.run([str(tmp/'mouse')],env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0'))
    if args.baseline_zip:
        if result.returncode != 1:
            raise SystemExit('Expected coordinate assertion failures in released baseline')
        print('Released baseline reproduces coordinate regression.')
    else:
        result.check_returncode()
