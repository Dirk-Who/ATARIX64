// Included after the current production MacXFS functions by run_safety_tests.py.
static unsigned filenameChecks=0;
#define FCHECK(c) do { ++filenameChecks; if(!(c)) { fprintf(stderr,"FAIL filename line %d: %s (errno %d)\n",__LINE__,#c,errno); abort(); } } while(0)
static std::string guestName(const std::string &host)
{
    char buf[MAXPATHNAMELEN];
    FCHECK(CTextConversion::HostFilenameToAtari(buf,host.c_str(),sizeof(buf)));
    return buf;
}
static std::string hostName(const std::string &guest)
{
    char buf[MAXPATHNAMELEN];
    FCHECK(CTextConversion::AtariFilenameToHost(buf,guest.c_str(),sizeof(buf)));
    return buf;
}
static CMacXFS::XfsCookie mountTest(CMacXFS &x,const std::string &root,int drive)
{
    auto node=new CMacXFS::XfsFsFile(x,"test",(root+"/").c_str());
    x.drives[drive].host_root=node; x.drives[drive].drv_valid=true;
    x.drives[drive].drv_flags=0;
    snprintf(x.drives[drive].mount_point,sizeof(x.drives[drive].mount_point),"%c:\\",'A'+drive);
    return CMacXFS::XfsCookie{(uint16_t)drive,node,&x.drives[drive]};
}
static CMacXFS::MAC_FD openTest(CMacXFS &x,CMacXFS::XfsCookie *fc,const std::string &name,unsigned flags)
{
    int32_t fd=x.xfs_fopen(fc,name.c_str(),flags,0);
    FCHECK(fd>=0);
    CMacXFS::MAC_FD f{};f.host_fd=be16_to_cpu((uint16_t)fd);f.fd.fd_refcnt=cpu_to_be16(1);f.fc=*fc;
    return f;
}
static CMacXFS::XfsCookie subdirTest(CMacXFS &x,CMacXFS::XfsCookie *fc,const std::string &name)
{
    char p[MAXPATHNAMELEN];strcpy(p,name.c_str());char *rest=nullptr,*link=nullptr;
    CMacXFS::MXFSDD base{int32_t(fc->index->mapped_value),0},dd{},ld{};uint16_t drive;
    FCHECK(x.xfs_path2DD(1,fc->dev,&base,p,&rest,&ld,&link,&dd,&drive)==0);
    FCHECK(rest && !*rest && !link);
    CMacXFS::XfsCookie result{};x.fetchXFSC(&result,fc->dev,&dd);
    FCHECK(result.index);return result;
}
static void copyTestTree(CMacXFS &x,CMacXFS::XfsCookie *source,CMacXFS::XfsCookie *target,unsigned &files,unsigned &dirs)
{
    CMacXFS::MAC_DIRHANDLE dh{};FCHECK(x.xfs_dopendir(&dh,source,0)==0);
    alignas(4) char buf[MAXPATHNAMELEN];XATTR attr{};int32_t xr=0,rc;
    while((rc=x.xfs_dreaddir(&dh,source->dev,sizeof(buf),buf,&attr,&xr))==0) {
        const std::string name=buf+4;
        if(name=="." || name=="..")continue;
        FCHECK(be32_to_cpu(xr)==0);
        XATTR lookup{};FCHECK(x.xfs_xattr(source,name.c_str(),&lookup,0)==0);
        FCHECK(lookup.index==attr.index && lookup.dev==attr.dev);
        if(be16_to_cpu(attr.attr)&F_SUBDIR) {
            FCHECK(x.xfs_dcreate(target,name.c_str())==0);++dirs;
            auto a=subdirTest(x,source,name),b=subdirTest(x,target,name);
            copyTestTree(x,&a,&b,files,dirs);
        } else {
            auto a=openTest(x,source,name,OM_RPERM);
            auto b=openTest(x,target,name,OM_WPERM|_ATARI_O_CREAT|_ATARI_O_EXCL);
            char data[16384];int32_t n;
            while((n=x.dev_read(&a,sizeof(data),data))>0)FCHECK(x.dev_write(&b,n,data)==n);
            FCHECK(n==0);FCHECK(x.dev_close(&a)==0);FCHECK(x.dev_close(&b)==0);++files;
        }
    }
    FCHECK(rc==TOS_ENMFIL);FCHECK(x.xfs_dclosedir(&dh,source->dev)==0);
}
static void test_filename_codec()
{
    std::vector<std::string> names={"ASCII.TXT","LONGNA~1.TXT","äöüÄÖÜß÷.txt","Font_∑ndern.PAP",
        "Einf˝hrkurs.PAP","H∑ufige_Fragen.PAP","Γêæ.PAP","י¥.PAP","☃😀.txt",
        "literal~{2211}.txt","~{007E}{2211}","bad~{xyz}","a\xcc\x88.txt","\xc3\xa4.txt",
        "Å.txt","Å.txt","μ.txt","µ.txt","~","~{","~~{2211}","∑.PAP"};
    std::map<std::string,std::string> seen;
    for(const auto &n:names) {
        const auto g=guestName(n);FCHECK(hostName(g)==n);
        FCHECK(seen.emplace(g,n).second);
        for(size_t cap=1;cap<=g.size()+2;++cap) {
            std::vector<char> b(cap+1,'!');
            bool ok=CTextConversion::HostFilenameToAtari(b.data(),n.c_str(),cap);
            FCHECK(ok==(cap>g.size()));FCHECK(b[cap]=='!');
            if(!ok) FCHECK(b[0]==0 && errno==ENAMETOOLONG);
        }
        for(size_t cap=1;cap<=n.size()+2;++cap) {
            std::vector<char> b(cap+1,'!');
            bool ok=CTextConversion::AtariFilenameToHost(b.data(),g.c_str(),cap);
            FCHECK(ok==(cap>n.size()));FCHECK(b[cap]=='!');
            if(!ok) FCHECK(b[0]==0 && errno==ENAMETOOLONG);
        }
    }
    FCHECK(guestName("Font_∑ndern.PAP")=="Font_~{2211}ndern.PAP");
    FCHECK(guestName("Γêæ.PAP")==std::string("\xe2\x88\x91.PAP"));
    FCHECK(hostName("\xe2\x88\x91.PAP")=="Γêæ.PAP"); // Never guess UTF-8.
    for(unsigned c=1;c<256;++c) {
        std::string a(1,char(c));char utf[8];
        FCHECK(CTextConversion::Atari2HostUtf8Copy(utf,a.c_str(),sizeof(utf)));
        FCHECK(hostName(a)==utf);FCHECK(guestName(utf)==a);
    }
    // Unicode scalar boundaries and a deterministic sample beyond the BMP.
    for(unsigned ch=128;ch<=0x10ffff;ch+=997) {
        if(ch>=0xd800 && ch<=0xdfff)continue;
        std::string u;
        if(ch<0x800)u+=char(0xc0|(ch>>6));
        else {
            if(ch<0x10000)u+=char(0xe0|(ch>>12));
            else {u+=char(0xf0|(ch>>18));u+=char(0x80|((ch>>12)&63));}
            u+=char(0x80|((ch>>6)&63));
        }
        u+=char(0x80|(ch&63));FCHECK(hostName(guestName(u))==u);
    }
    for(const char *bad:{"\xc0\xaf","\xc2","\xe2\x88","\xed\xa0\x80","\xf4\x90\x80\x80","\x80","\xf0\x9f\x98"}) {
        char b[64]="sentinel";FCHECK(!CTextConversion::HostFilenameToAtari(b,bad,sizeof(b)));
        FCHECK(b[0]==0 && errno==EILSEQ);
    }
    for(const char *bad:{"~{","~{22","~{2211","~{xyz}","~{D800}","~{110000}","~{0000}","~{002F}","~{002E}","~{005C}","~{000041}"}) {
        char b[64]="sentinel";FCHECK(!CTextConversion::AtariFilenameToHost(b,bad,sizeof(b)));
        FCHECK(b[0]==0 && errno==EILSEQ);
    }
    char b[16]="sentinel";FCHECK(!CTextConversion::HostFilenameToAtari(b,nullptr,sizeof(b)) && errno==EINVAL && !b[0]);
    FCHECK(!CTextConversion::AtariFilenameToHost(b,"x",0) && errno==EINVAL);
    std::string huge(MAXPATHNAMELEN,'x');
    FCHECK(!CTextConversion::AtariFilenameToHost(b,huge.c_str(),sizeof(b)) && errno==ENAMETOOLONG);
    puts("PASS filename codec: Atari bytes, Unicode, collisions, literal escapes, NFC/NFD, invalid input, capacities");
}
static void test_filename_regression(const std::string &work)
{
    test_filename_codec();
    const std::string root=work+"/names",target=work+"/names-copy";
    FCHECK(mkdir(root.c_str(),0700)==0);FCHECK(mkdir(target.c_str(),0700)==0);
    std::vector<std::string> names={"Font_∑ndern.PAP","Einf˝hrkurs.PAP","Γêæ.PAP","י¥.PAP",
        "literal~{2211}.txt","☃😀.txt","a\xcc\x88.txt","÷.TXT","∑.PAP","LONGNA~1.TXT"};
    for(const auto &n:names) {
        FILE *f=fopen((root+"/"+n).c_str(),"wb");FCHECK(f);fwrite(n.data(),1,n.size(),f);fclose(f);
    }
    CMacXFS x;auto fc=mountTest(x,root,2),tc=mountTest(x,target,3);
    auto dirname=guestName("Unter∑˝😀~{dir}");
    FCHECK(x.xfs_dcreate(&fc,dirname.c_str())==0);
    auto sub=subdirTest(x,&fc,dirname);
    char path[MAXPATHNAMELEN];FCHECK(x.xfs_DD2name(&sub,path,sizeof(path))==0);
    FCHECK(std::string(path)=="\\"+dirname);
    char tiny[2]={'!','!'};FCHECK(x.xfs_DD2name(&sub,tiny,1)==TOS_ERANGE && tiny[1]=='!');
    auto fd=openTest(x,&sub,guestName("child∑.txt"),OM_WPERM|_ATARI_O_CREAT|_ATARI_O_EXCL);
    char data[]="contents";FCHECK(x.dev_write(&fd,8,data)==8);FCHECK(x.dev_close(&fd)==0);
    unsigned files=0,dirs=0;copyTestTree(x,&fc,&tc,files,dirs);
    FCHECK(files==names.size()+1 && dirs==1);
    for(const auto &n:names) {
        char content[256]{};FILE *f=fopen((target+"/"+n).c_str(),"rb");FCHECK(f);
        size_t len=fread(content,1,sizeof(content),f);fclose(f);FCHECK(std::string(content,len)==n);
        auto old=guestName(n),renamed=guestName("renamed-"+n);
        FCHECK(x.xfs_link(&tc,&old[0],&tc,&renamed[0],0)==0);
        XATTR attr{};FCHECK(x.xfs_xattr(&tc,renamed.c_str(),&attr,0)==0);
        FCHECK(x.xfs_fdelete(&tc,renamed.c_str())==0);
    }
    // 8.3 listing must return complete, resolvable names only.
    fc.drv->drv_flags=M_DRV_DOSNAMES;
    CMacXFS::MAC_DIRHANDLE dh{};FCHECK(x.xfs_dopendir(&dh,&fc,1)==0);
    alignas(4) char b[1024];unsigned shortCount=0;int32_t rc;
    while((rc=x.xfs_dreaddir(&dh,2,sizeof(b),b,nullptr,nullptr))==0) {
        XATTR attr{};FCHECK(x.xfs_xattr(&fc,b,&attr,0)==0);++shortCount;
    }
    FCHECK(rc==TOS_ENMFIL && shortCount>=3);FCHECK(x.xfs_dclosedir(&dh,2)==0);
    CMacXFS::MAC_DTA dta{};char pattern[]="*.*";unsigned found=0;
    rc=x.xfs_sfirst(&fc,pattern,&dta,0);
    while(rc==0) {
        XATTR attr{};FCHECK(x.xfs_xattr(&fc,dta.mxdta.dta_name,&attr,0)==0);++found;
        rc=x.xfs_snext(2,&dta);
    }
    FCHECK(rc==TOS_ENMFIL && found>=3);
    fc.drv->drv_flags=0;
    auto ln=guestName("link∑"),to=guestName("Font_∑ndern.PAP");
    FCHECK(x.xfs_symlink(&fc,ln.c_str(),to.c_str())==0);
    char linkbuf[MAXPATHNAMELEN];
    FCHECK(x.xfs_readlink(&fc,ln.c_str(),linkbuf,sizeof(linkbuf))==0 && std::string(linkbuf)==to);
    FCHECK(x.xfs_readlink(&fc,ln.c_str(),linkbuf,4)==TOS_ERANGE);
    auto linked=openTest(x,&fc,ln,OM_RPERM);FCHECK(x.dev_close(&linked)==0);
    FCHECK(x.xfs_fdelete(&fc,ln.c_str())==0);
    FCHECK(x.xfs_dopendir(&dh,&fc,0)==0);char guard[2]={'!','!'};
    FCHECK(x.xfs_dreaddir(&dh,2,1,guard,nullptr,nullptr)==TOS_ERANGE && guard[1]=='!');
    FCHECK(x.xfs_dclosedir(&dh,2)==0);
    for(const char *invalid:{"~{2211","~{0000}","~{002F}etc"}) {
        XATTR attr{};FCHECK(x.xfs_xattr(&fc,invalid,&attr,0)==TOS_EINVAL);
        FCHECK(x.xfs_fopen(&fc,invalid,_ATARI_O_CREAT|OM_WPERM,0)==TOS_EINVAL);
        FCHECK(x.xfs_dcreate(&fc,invalid)==TOS_EINVAL);
        FCHECK(x.xfs_fdelete(&fc,invalid)==TOS_EINVAL);
    }
    auto ts=subdirTest(x,&tc,dirname);FCHECK(x.xfs_fdelete(&ts,guestName("child∑.txt").c_str())==0);
    FCHECK(x.xfs_ddelete(&ts)==0);
    if(const char *source=getenv("ATARIX_COPY_TREE")) {
        const std::string dest=work+"/xfs-copy-target";FCHECK(mkdir(dest.c_str(),0700)==0);
        auto a=mountTest(x,source,4),b=mountTest(x,dest,5);
        a.drv->drv_flags=M_DRV_READONLY;
        files=dirs=0;copyTestTree(x,&a,&b,files,dirs);
        printf("PASS real source tree through MacXFS: %u files, %u dirs -> %s\n",files,dirs,dest.c_str());
    }
    printf("PASS filename XFS: listing/xattr/open/copy/create/rename/delete, paths, 8.3, errors (%u checks)\n",filenameChecks);
}
