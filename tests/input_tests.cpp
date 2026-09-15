// Included after current production methods by run_input_tests.py.
static void test_wheel()
{
    // Exhaust every ring position and capacity, in all four directions.
    const int directions[4][3]={{0,1,0x48},{0,-1,0x50},{1,0,0x4d},{-1,0,0x4b}};
    for(unsigned offset=0;offset<KEYBOARDBUFLEN;++offset)
    for(unsigned free=0;free<KEYBOARDBUFLEN;++free)
    for(auto &direction:directions) {
        CMagiC c; c.fill(free,offset);
        WheelFixture{c}.wheel(direction[0],direction[1]);
        auto bytes=drain(c); unsigned old=KEYBOARDBUFLEN-1-free;
        CHECK(bytes.size()==old+(free>=2?2:0));
        for(unsigned i=0;i<old;++i) CHECK(bytes[i]==0x1e);
        if(free>=2) {
            CHECK(bytes[old]==unsigned(direction[2]));
            CHECK(bytes[old+1]==unsigned(direction[2]|0x80));
        }
        CHECK(c.GetKbBufferFree()==KEYBOARDBUFLEN-1);
    }
    // Bursts stay bounded; large negative values must not overflow on flip/abs.
    for(int delta:{INT_MIN,-100,-8,-1,0,1,8,100,INT_MAX})
    for(bool flipped:{false,true})
    for(bool horizontal:{false,true}) {
        CMagiC c; WheelFixture{c}.wheel(horizontal?delta:0,horizontal?0:delta,flipped);
        auto bytes=drain(c);
        unsigned turns=delta< -8||delta>8?8:unsigned(delta<0?-delta:delta);
        CHECK(bytes.size()==turns*2);
        bool positive=(delta>0)!=flipped;
        unsigned key=horizontal?(positive?0x4d:0x4b):(positive?0x48:0x50);
        for(unsigned i=0;i<bytes.size();i+=2) { CHECK(bytes[i]==key); CHECK(bytes[i+1]==(key|0x80)); }
    }
    CMagiC c; WheelFixture wheel{c};
    wheel.wheel(0,1); wheel.wheel(0,-1); wheel.wheel(1,0); wheel.wheel(-1,0);
    CHECK(drain(c)==std::vector<unsigned>({0x48,0xc8,0x50,0xd0,0x4d,0xcd,0x4b,0xcb}));
    wheel.wheel(1,-1); CHECK(drain(c)==std::vector<unsigned>({0x50,0xd0}));
    CHECK(c.SendSdlKeyboardPair(SDL_SCANCODE_UNKNOWN)==0); CHECK(drain(c).empty());
    c.m_bEmulatorIsRunning=false; wheel.wheel(0,8); CHECK(drain(c).empty());
    std::puts("PASS wheel: all capacities/wraparound, pair integrity, burst limit, directions, flipped and INT_MIN");
}

static void test_irq_and_mouse()
{
    for(unsigned offset=0;offset<KEYBOARDBUFLEN;++offset)
    for(unsigned free:{0u,1u,2u,3u,31u}) {
        CMagiC c; c.fill(free,offset);
        c.SendMousePosition(120,55); c.SendMouseButton(0,true);
        unsigned before=irqCalls.load(); c.PrepareMouseKeyboardInterrupt();
        CHECK(irqCalls==before+1); CHECK(c.m_bInterruptPending);
        CHECK(!c.m_bInterruptMouseKeyboardPending);
        // No new mouse bytes were inserted into the full/nearly full ring.
        CHECK(c.GetKbBufferFree()==int(free));
        unsigned old=KEYBOARDBUFLEN-1-free;
        // Each one-byte guest read frees capacity; no further host event is needed.
        for(unsigned i=0;i<old;++i) {
            CHECK(c.AtariGetKeyboardOrMouseData(1,nullptr));
            CHECK(c.AtariGetKeyboardOrMouseData(0,nullptr)==0x1e);
        }
        CHECK(drain(c)==std::vector<unsigned>({0xfa,120,55}));
        // Simulate the guest updating its real cursor coordinates.
        auto x=cpu_to_be16(uint16_t(120)),y=cpu_to_be16(uint16_t(55));
        std::memcpy(c.m_LineAVars-0x158,&x,2); std::memcpy(c.m_LineAVars-0x156,&y,2);
        // Release reaches the guest even if it first arrives behind a full ring.
        c.fill(0,offset); c.SendMouseButton(0,false); c.PrepareMouseKeyboardInterrupt();
        auto bytes=drain(c); CHECK(bytes.size()==34);
        CHECK(bytes[31]==0xf8 && bytes[32]==0 && bytes[33]==0);
    }
    // Keyboard-only traffic also raises IRQ at capacities 0,1,2,3.
    for(unsigned free:{0u,1u,2u,3u}) {
        CMagiC c; c.fill(free); c.m_bInterruptMouseKeyboardPending=true;
        unsigned before=irqCalls.load(); c.PrepareMouseKeyboardInterrupt();
        CHECK(irqCalls==before+1); CHECK(drain(c).size()==31-free);
    }
    CMagiC idle; auto before=irqCalls.load(); idle.PrepareMouseKeyboardInterrupt(); CHECK(irqCalls==before);
    // New host state after staging remains pending and supersedes the old target.
    CMagiC latest; latest.fill(0); latest.SendMousePosition(40,20); latest.PrepareMouseKeyboardInterrupt();
    latest.SendMousePosition(80,60); latest.SendMouseButton(1,true);
    CHECK(latest.m_bInterruptMouseKeyboardPending); latest.PrepareMouseKeyboardInterrupt();
    auto bytes=drain(latest); CHECK(bytes.size()==34); CHECK(bytes[31]==0xf9&&bytes[32]==80&&bytes[33]==60);
    // Large moves produce multiple complete packets until the target is reached.
    CMagiC large; large.fill(0); large.SendMousePosition(300,260); large.PrepareMouseKeyboardInterrupt();
    bytes=drain(large); CHECK(bytes.size()==40);
    CHECK(std::vector<unsigned>(bytes.begin()+31,bytes.end())==std::vector<unsigned>({0xf8,127,127,0xf8,127,127,0xf8,46,6}));
    std::puts("PASS IRQ/mouse: full-ring delivery, deferred motion/buttons, releases, partial draining, latest state and multi-packet motion");
}

static void test_mixed_and_concurrent()
{
    CMagiC c;
    // Repeated mixed traffic never truncates accepted wheel pairs.
    for(unsigned round=0;round<1000;++round) {
        CHECK(c.SendSdlKeyboard(SDL_SCANCODE_A,false)==0);
        CHECK(c.SendSdlKeyboard(SDL_SCANCODE_A,true)==0);
        WheelFixture{c}.wheel(0,round%2?100:-100);
        c.SendMouseButton(0,round%2!=0); c.PrepareMouseKeyboardInterrupt();
        auto bytes=drain(c); CHECK(bytes[0]==0x1e && bytes[1]==0x9e);
        unsigned key=round%2?0x48:0x50;
        for(unsigned i=2;i<18;i+=2) { CHECK(bytes[i]==key); CHECK(bytes[i+1]==(key|0x80)); }
        CHECK(bytes.size()==(round?21u:18u));
        if(round) CHECK(bytes[18]==(round%2?0xfau:0xf8u));
    }
    // Real recursive input mutex, multiple producers, consumer only observes
    // adjacent make/break bytes. Dropped pairs are counted, never half queued.
    CMagiC concurrent; std::atomic<unsigned> done{0},accepted{0};
    std::vector<std::thread> producers;
    for(int i=0;i<3;++i) producers.emplace_back([&,i]{
        for(int n=0;n<4000;++n) if(concurrent.SendSdlKeyboardPair(i%2?SDL_SCANCODE_UP:SDL_SCANCODE_DOWN)==0) ++accepted;
        ++done;
    });
    std::vector<unsigned> bytes;
    while(done.load()!=3 || concurrent.AtariGetKeyboardOrMouseData(1,nullptr)) {
        if(concurrent.AtariGetKeyboardOrMouseData(1,nullptr)) bytes.push_back(concurrent.AtariGetKeyboardOrMouseData(0,nullptr));
        else std::this_thread::yield();
    }
    for(auto &producer:producers) producer.join();
    CHECK(bytes.size()==2*accepted.load()); CHECK(bytes.size()%2==0);
    for(unsigned i=0;i<bytes.size();i+=2) { CHECK(bytes[i]==0x48||bytes[i]==0x50); CHECK(bytes[i+1]==(bytes[i]|0x80)); }
    CHECK(concurrent.GetKbBufferFree()==31);
    std::puts("PASS mixed input and concurrent pair producers/consumer (accepted count varies with scheduling)");
}

int main()
{
    test_wheel(); test_irq_and_mouse(); test_mixed_and_concurrent();
    std::printf("PASS: %u input assertions (ASan + UBSan; count includes concurrent accepted pairs)\n",checks.load());
}
