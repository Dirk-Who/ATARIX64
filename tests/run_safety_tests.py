#!/usr/bin/env python3
"""Compile current production functions into a headless ASan regression executable.

Needs macOS, clang++, and SDL headers (scripts/bootstrap-sdl2.sh).
The emulated memory bus uses test buffers and a bus-error recorder. XFS uses
its real class definition, pointer mapper, methods and text conversion.
No emulator GUI or user MAGIC_C directory is opened. Fixtures stay in a
new temporary directory. Generated sources retain the project's GPL notice.
"""
from pathlib import Path
import os
import subprocess
import tempfile

repo = Path(__file__).resolve().parents[1]
src = repo / 'src/AtariX-MT/AtariX'
work = Path(tempfile.mkdtemp(prefix='atarix-regressions-'))

def extract(file, signature):
    text = (src/file).read_text()
    start = text.index(signature)
    end = text.index('\n}', text.index('{', start))+2
    line = text[:start].count('\n')+1
    return f'#line {line} "{src/file}"\n'+text[start:end]+'\n'

code = (src/'MacXFS.cpp').read_text().split('*/', 1)[0]+'*/\n'
code += r'''
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <atomic>
#include <cassert>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cerrno>
#include <cctype>
#include <limits.h>
#include <CoreFoundation/CoreFoundation.h>
#include "SDL.h"
#include "Globals.h"
#include "s_endian.h"
#include "maptab.h"
#include "TextConversion.h"
// Test access only; the production class layout is unchanged.
#define private public
#include "MacXFS.h"
#undef private
#define DebugError(...) ((void)0)
#define DebugWarning(...) ((void)0)
#define DebugInfo(...) ((void)0)
#define DIRSEPARATOR "/"
#define IS_DIR_SEP(c) ((c) == '\\' || (c) == '/')
#define HAVE_REALPATH
static char const lowers[] = { '\x84','\x94','\x81','\x87','\x82','\x86','\x91','\xb4','\x85','\xc0','\xa4','\xb0','\xb3','\xb1',0 };
static char const uppers[] = { '\x8e','\x99','\x9a','\x80','\x90','\x8f','\x92','\xb5','\xb6','\xc1','\xa5','\xb7','\xb2','\xb8',0 };
using m68k_addr_type = uint32_t;
using m68k_data_type = uint32_t;
uint32_t Adr68kVideo=1024, Adr68kVideoEnd=1088;
uint8_t *OpcodeROM, *HostVideoAddr;
bool bAtariVideoRamHostEndian=false;
std::atomic<int> changed{0};
std::atomic<int> *p_bVideoBufChanged=&changed;
using std::atomic_exchange;
int busErrors=0;
struct CMagiC {
  static void GetActAtariPrg(const char **name,uint32_t *pd) { *name="test"; *pd=0; }
  void SendBusError(uint32_t,const char *) { ++busErrors; }
} magic, *pTheMagiC=&magic;
const char *AtariAddr2Description(uint32_t) { return "test"; }
'''
for bits in [8,16,32]:
    code += extract('MagiC.cpp',f'm68k_addr_type m68k_read_memory_{bits}(')
for bits in [8,16,32]:
    code += extract('MagiC.cpp',f'void m68k_write_memory_{bits}(')
code += extract('EmulationRunner.cpp','static void ConvertSurface\n')
code += extract('EmulationRunner.cpp','static unsigned AtariX_CompatibleScreenWidth(')
for sig in [
    'CMacXFS::XfsFsFile::XfsFsFile(', 'CMacXFS::XfsFsFile::~XfsFsFile()',
    'CMacXFS::XfsFsFile *CMacXFS::XfsFsFile::insert(',
    'CMacXFS::CMacXFS()', 'CMacXFS::~CMacXFS()',
    'unsigned char CMacXFS::ToUpper(', 'unsigned char CMacXFS::ToLower(',
    'int32_t CMacXFS::errnoHost2Mint(', 'bool CMacXFS::nameto_8_3(',
    'int CMacXFS::fname_is_invalid(',
    'char *CMacXFS::cookie2Pathname(struct mount_info',
    'char *CMacXFS::cookie2Pathname(XfsCookie',
    'bool CMacXFS::getHostFileName(', 'DIR *CMacXFS::host_opendir(',
    'char *CMacXFS::my_canonicalize_file_name(', 'char *CMacXFS::host_readlink(',
    'void CMacXFS::fetchXFSC(', 'int32_t CMacXFS::xfs_path2DD\n',
    'int32_t CMacXFS::xfs_readlink(', 'int32_t CMacXFS::xfs_dcreate(',
]:
    code += extract('MacXFS.cpp',sig)
code += '#line 1 "safety_tests.cpp"\n'+(repo/'tests/safety_tests.cpp').read_text()
(work/'generated.cpp').write_text(code)
cmd=['clang++','-x','c++','-std=c++17','-O1','-g','-fsanitize=address',
     '-fno-omit-frame-pointer','-Wno-deprecated-declarations',
     '-I'+str(src),'-F'+str(src),'-I'+str(src/'SDL2.framework/Headers'),
     str(work/'generated.cpp'),str(src/'TextConversion.cpp'),str(src/'maptab.c'),
     '-framework','CoreFoundation','-o',str(work/'safety_tests')]
subprocess.run(cmd,check=True)
env=dict(os.environ,ASAN_OPTIONS='detect_leaks=0') # legacy XFS tree lifetime is not part of this patch
subprocess.run([str(work/'safety_tests'),str(work)],check=True,env=env)
print('Generated test executable and fixtures:',work)
