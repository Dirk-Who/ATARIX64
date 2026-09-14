// Included by run_safety_tests.py after the current production functions.
static int checks = 0;
#define CHECK(x) do { ++checks; if (!(x)) { fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x); abort(); } } while (0)

static void test_memory()
{
    std::vector<uint8_t> ram(1040), video(80);
    OpcodeROM=ram.data(); HostVideoAddr=video.data();
    for (bool swizzle : {false,true}) {
        bAtariVideoRamHostEndian=swizzle;
        for (unsigned bytes : {1,2,4}) {
            for (uint32_t a : {0u,1u,1019u,1020u,1021u,1022u,1023u,1024u,1025u,
                               1083u,1084u,1085u,1086u,1087u,1088u,0xfffffffeu}) {
                std::fill(ram.begin(),ram.end(),0x55);
                std::fill(video.begin(),video.end(),0x55);
                busErrors=0;
                atomic_store_explicit(&changed, 0, memory_order_relaxed);
                const bool valid=a<1088 && bytes<=1088-a;
                uint32_t value=0x12345678;
                if(bytes==1) m68k_write_memory_8(a,value);
                if(bytes==2) m68k_write_memory_16(a,value);
                if(bytes==4) m68k_write_memory_32(a,value);
                CHECK(busErrors==(valid?0:1));
                CHECK(atomic_exchange(&changed, 0) == (valid && (uint64_t)a+bytes>Adr68kVideo ? 1:0));
                for(size_t i=1024;i<ram.size();++i) CHECK(ram[i]==0x55);
                for(size_t i=64;i<video.size();++i) CHECK(video[i]==0x55);
                if(valid) {
                    for(unsigned i=0;i<bytes;++i)
                        CHECK(m68k_read_memory_8(a+i)==((value>>(8*(bytes-1-i)))&255));
                    uint32_t actual=bytes==1?m68k_read_memory_8(a):bytes==2?m68k_read_memory_16(a):m68k_read_memory_32(a);
                    CHECK(actual==(bytes==1?value&255:bytes==2?value&65535:value));
                } else {
                    for(auto v:ram) CHECK(v==0x55);
                    for(auto v:video) CHECK(v==0x55);
                    busErrors=0;
                    if(bytes==1) (void)m68k_read_memory_8(a);
                    if(bytes==2) (void)m68k_read_memory_16(a);
                    if(bytes==4) (void)m68k_read_memory_32(a);
                    CHECK(busErrors==1);
                }
            }
        }
    }
    puts("PASS memory: endian modes, unaligned access, RAM/VRAM crossing, invalid spans");
}

static void test_pixels()
{
    for(int mode:{1,4,20,40,8,16}) for(int w:{320,321,327,335,640,1024}) {
        int bits=mode==20?2:mode==40?4:mode;
        int pitch=mode==20||mode==40?((w+15)/16)*bits*2:((w*bits+7)/8+3)&~3;
        std::vector<uint8_t> source(pitch*2);
        for(size_t i=0;i<source.size();++i) source[i]=uint8_t(i*37+29);
        std::vector<uint32_t> dest(w*2+16,0xdeadbeef);
        uint32_t palette[256];
        for(unsigned i=0;i<256;++i) palette[i]=0xff000000|i*0x010101;
        SDL_PixelFormat format{}; format.BitsPerPixel=bits;
        SDL_Surface s{},d{};
        s.pixels=source.data(); s.w=w; s.h=2; s.pitch=pitch; s.format=&format;
        s.userdata=mode==20||mode==40?(void*)1:nullptr;
        d.pixels=dest.data(); d.w=w; d.h=2; d.pitch=w*4;
        ConvertSurface(&s,&d,palette,false,false);
        for(int y=0;y<2;++y) for(int x=0;x<w;++x) {
            auto row=source.data()+pitch*y; unsigned index=0,expected=0;
            if(mode==1) index=(row[x/8]>>(7-x%8))&1;
            if(mode==4) index=(row[x/2]>>(x%2?0:4))&15;
            if(mode==8) index=row[x];
            if(mode==20||mode==40)
                for(int p=0;p<bits;++p)
                    index|=((row[(x/16)*bits*2+p*2+(x%16)/8]>>(7-x%8))&1)<<p;
            expected=palette[index];
            if(mode==16) {
                unsigned rgb=(row[x*2]<<8)|row[x*2+1];
                expected=0xff000000|((((rgb>>10)&31)*255/31)<<16)|
                    ((((rgb>>5)&31)*255/31)<<8)|((rgb&31)*255/31);
            }
            CHECK(dest[y*w+x]==expected);
        }
        for(size_t i=w*2;i<dest.size();++i) CHECK(dest[i]==0xdeadbeef);
        if(mode==20||mode==40) {
            std::fill(dest.begin(),dest.end(),0xdeadbeef);
            s.pitch=pitch-1;
            ConvertSurface(&s,&d,palette,false,false);
            for(auto p:dest) CHECK(p==0xdeadbeef);
        }
    }
    puts("PASS pixels: six formats, aligned/partial groups, exact source strides, guard pixels");
}

static void test_files(const std::string &work)
{
    const std::string root=work+"/drive", child=root+"/child";
    CHECK(mkdir(root.c_str(),0700)==0); CHECK(mkdir(child.c_str(),0700)==0);
    CHECK(symlink(root.c_str(),(root+"/rootlink").c_str())==0);
    CHECK(symlink("child",(root+"/relative").c_str())==0);
    CHECK(symlink(child.c_str(),(root+"/absolute").c_str())==0);
    CHECK(symlink("cycle",(root+"/cycle").c_str())==0);
    CHECK(symlink("missing",(root+"/broken").c_str())==0);
    CMacXFS x;
    auto node=new CMacXFS::XfsFsFile(x,"C",(root+"/").c_str());
    x.drives[2].host_root=node; x.drives[2].drv_valid=true; x.drives[2].drv_flags=0;
    strcpy(x.drives[2].mount_point,"C:\\");
    char buf[PATH_MAX];
    CHECK(x.host_readlink((root+"/rootlink").c_str(),buf,sizeof(buf)) && !strcmp(buf,"C:\\"));
    CHECK(x.host_readlink((root+"/relative").c_str(),buf,sizeof(buf)) && !strcmp(buf,"child"));
    CHECK(x.host_readlink((root+"/absolute").c_str(),buf,sizeof(buf)) && !strcmp(buf,"C:\\child"));
    for(const char *name:{"relative","absolute","rootlink"}) {
        DIR *d=x.host_opendir((root+"/"+name).c_str()); CHECK(d); closedir(d);
    }
    CHECK(!x.host_opendir((root+"/cycle").c_str()) && errno==ELOOP);
    CHECK(!x.host_opendir((root+"/broken").c_str()) && errno==ENOENT);
    char small[5]={'x','x','x','x','!'};
    CHECK(!x.host_readlink((root+"/relative").c_str(),small,4));
    CHECK(errno==ENAMETOOLONG && small[4]=='!');
    CMacXFS::XfsCookie fc{2,node,&x.drives[2]};
    CHECK(x.xfs_dcreate(&fc,"created")==TOS_E_OK);
    struct stat createdStat{};
    CHECK(stat((root+"/created").c_str(),&createdStat)==0 && S_ISDIR(createdStat.st_mode));
    CHECK(x.xfs_dcreate(&fc,"created")!=TOS_E_OK);
    x.drives[2].drv_flags=M_DRV_READONLY;
    CHECK(x.xfs_dcreate(&fc,"readonly")==TOS_EWRPRO);
    x.drives[2].drv_flags=0;
    CHECK(x.xfs_readlink(&fc,"relative",buf,2)==TOS_ERANGE);
    CHECK(x.xfs_readlink(&fc,"relative",buf,sizeof(buf))==TOS_E_OK && !strcmp(buf,"child"));
    auto nested=new CMacXFS::XfsFsFile(x,"D",(child+"/").c_str());
    x.drives[3].host_root=nested; x.drives[3].drv_valid=true;
    strcpy(x.drives[3].mount_point,"D:\\");
    CHECK(x.host_readlink((root+"/absolute").c_str(),buf,sizeof(buf)) && !strcmp(buf,"D:\\"));
    // A sibling sharing the root's string prefix must not match the mount.
    std::string sibling=root+"-other";
    CHECK(mkdir(sibling.c_str(),0700)==0);
    CHECK(symlink(sibling.c_str(),(root+"/sibling").c_str())==0);
    CHECK(x.host_readlink((root+"/sibling").c_str(),buf,sizeof(buf)) && std::string(buf)==sibling);
    auto rel=[&](const char *path,uint16_t mode,const char *expectedRest,CMacXFS::XfsFsFile *expected,int32_t code) {
        char p[PATH_MAX]; strcpy(p,path); char *rest=nullptr,*link=nullptr;
        CMacXFS::MXFSDD base{int32_t(node->mapped_value),0},dd{},linkdd{};
        uint16_t drive=0;
        CHECK(x.xfs_path2DD(mode,2,&base,p,&rest,&linkdd,&link,&dd,&drive)==code);
        if(expectedRest) CHECK(rest && !strcmp(rest,expectedRest));
        CHECK(link==nullptr);
        if(expected) CHECK(dd.dirID==int32_t(expected->mapped_value));
    };
    rel("child/../file",0,"file",node,TOS_E_OK);
    rel("./child/..",1,"",node,TOS_E_OK);
    rel("../file",0,"/file",nullptr,ELINK);
    rel("missing/file",0,nullptr,nullptr,TOS_EPTHNF);
    rel("relative",1,"",nullptr,TOS_E_OK);
    CHECK(x.cookie2Pathname(&fc,"rootlink",buf,sizeof(buf),true));
    CHECK(std::string(buf)==root+"/rootlink");
    char tiny[4]; CHECK(!x.cookie2Pathname(&fc,"x",tiny,sizeof(tiny),true) && errno==ENAMETOOLONG);
    std::vector<CMacXFS::XfsFsFile*> nodes;
    auto last=node;
    for(int i=0;i<5;++i) {
        auto n=new CMacXFS::XfsFsFile(x,"long",std::string(250,'a').c_str());
        n->parent=last; nodes.push_back(n); last=n;
    }
    CHECK(!x.cookie2Pathname(&x.drives[2],last,"file",buf,sizeof(buf),true) && errno==ENAMETOOLONG);
    for(auto n:nodes) delete n;
    puts("PASS XFS: long paths, root/relative/nested links, loops, missing paths, dot/parent handling");
}

static void test_text()
{
    const char umlaut[]={char(0x84),0};
    for(size_t capacity=1;capacity<5;++capacity) {
        char text[6]; memset(text,'!',sizeof(text));
        bool ok=CTextConversion::Atari2HostUtf8Copy(text,umlaut,capacity);
        CHECK(ok==(capacity>=3));
        CHECK(memchr(text,0,capacity)); CHECK(text[capacity]=='!');
        if(ok) CHECK(!strcmp(text,"\xc3\xa4"));
    }
    char text[5]; CHECK(CTextConversion::Atari2HostUtf8Copy(text,"abcd",5));
    CHECK(!CTextConversion::Atari2HostUtf8Copy(text,"abcde",5));
    CHECK(!strcmp(text,"abcd"));
    for(size_t capacity=1;capacity<5;++capacity) {
        char result[6]; memset(result,'!',sizeof(result));
        bool ok=CTextConversion::Host2AtariUtf8Copy(result,"\xe2\x98\x83",capacity);
        CHECK(ok==(capacity>=4));
        CHECK(memchr(result,0,capacity)); CHECK(result[capacity]=='!');
    }
    puts("PASS UTF-8: complete sequences, NUL termination, truncation status");
}

static void test_geometry()
{
    for(unsigned mode=0;mode<=6;++mode) for(unsigned request:{0u,320u,321u,1920u,2048u,3840u,4096u,99999u}) {
        unsigned width=AtariX_CompatibleScreenWidth(request,mode);
        CHECK(width>=320 && width<=4096);
        unsigned bits=mode==0?32:mode==1?16:mode==2?8:mode==5?2:mode==6?1:4;
        unsigned pitch=((width*bits+7)/8+3)&~3u;
        CHECK(pitch<0x2000);
        CHECK(((pitch|0x8000)&0x1fff)==pitch);
        if(mode==4||mode==5) CHECK(width%16==0);
        if(mode==0 && request>=2048) CHECK(width==2032);
        if(mode==1 && request>=4096) CHECK(width==4080);
    }
    puts("PASS geometry: all colour modes stay within bundled-driver pitch limits");
}

int main(int argc,char **argv)
{
    CHECK(argc==2);
    test_filename_regression(argv[1]);
    test_memory(); test_pixels(); test_files(argv[1]); test_text(); test_geometry();
    printf("PASS: %d assertions\n",checks);
}
