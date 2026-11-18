// M163-M164 (2701-2744): a scoped deterministic 6502/NROM CPU-bus oracle.
// NOT a full NES: missing most opcodes, PPU rendering, APU, cycle-accurate
// bus traffic, DMC/real DMA arbitration, all other mappers and ROM timings.
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace allmight::nes {
using U8=std::uint8_t;
using U16=std::uint16_t;
void check(bool condition,const char* what) {
    if(!condition) throw std::runtime_error(what);
}
struct Cartridge {
    std::vector<U8> prg;
    std::vector<U8> chr;
    bool chr_is_ram{};
    bool vertical_mirroring{};
};

// iNES 1.0/NROM subset: explicitly reject unsupported variants and trainers.
Cartridge parse_nrom_ines(const std::vector<U8>& file) {
    check(file.size()>=16,"truncated iNES header");
    check(file[0]=='N'&&file[1]=='E'&&file[2]=='S'&&file[3]==0x1A,
          "invalid iNES magic");
    check((file[7]&0x0C)!=0x08,"NES 2.0 unsupported by iNES1 parser");
    const unsigned mapper=(file[6]>>4)|(file[7]&0xF0);
    check(mapper==0,"this teaching oracle supports only mapper 0");
    check((file[6]&0x04)==0,"trainer must be handled explicitly");
    check(file[4]==1||file[4]==2,"NROM requires 16 or 32 KiB PRG");
    check(file[5]<=1,"NROM CHR ROM size must be 0 or 8 KiB");
    const std::size_t prg_size=16384u*file[4];
    const std::size_t chr_size=8192u*file[5];
    check(file.size()>=16+prg_size+chr_size,"truncated PRG or CHR");
    Cartridge result;
    result.prg.assign(file.begin()+16,file.begin()+16+prg_size);
    result.chr_is_ram=(chr_size==0);
    result.chr.assign(chr_size?file.begin()+16+prg_size:file.begin()+16+prg_size,
                      chr_size?file.begin()+16+prg_size+chr_size:
                               file.begin()+16+prg_size);
    if(result.chr_is_ram) result.chr.resize(8192,0);
    result.vertical_mirroring=(file[6]&0x01)!=0;
    return result;
}

struct Bus {
    std::array<U8,2048> ram{};
    std::array<U8,256> oam{};
    Cartridge cartridge{};
    std::array<U8,8> controller_bits{};
    U8 controller_shift{};
    U8 controller_index{};
    bool controller_strobe{};
    U8 last_ppu_register_write{};
    std::uint64_t cpu_cycles{};
    std::uint64_t ppu_dots{}; // Approximate scheduler, not an accurate PPU.
    unsigned dma_requests{};

    U8 read(U16 address) {
        if(address<0x2000) return ram[address&0x07FF];
        if(address>=0x2000 && address<0x4000) {
            // A real PPU has different semantics for each 8-register mirror.
            return 0;
        }
        if(address==0x4016) {
            if(controller_strobe) return controller_bits[0];
            const U8 answer=controller_index<8 ?
                controller_bits[controller_index]:1;
            if(controller_index<8) ++controller_index;
            return answer;
        }
        if(address>=0x8000) {
            check(cartridge.prg.size()==16384||cartridge.prg.size()==32768,
                  "cartridge PRG missing");
            const auto offset=static_cast<std::size_t>(address-0x8000);
            return cartridge.prg[offset%cartridge.prg.size()];
        }
        // Open-bus behavior is not emulated. Zero is a deliberate sentinel.
        return 0;
    }
    void write(U16 address,U8 value) {
        if(address<0x2000) { ram[address&0x07FF]=value;return; }
        if(address>=0x2000&&address<0x4000) {
            last_ppu_register_write=static_cast<U8>((address-0x2000)&7);
            return;
        }
        if(address==0x4016) {
            const bool previous=controller_strobe;
            controller_strobe=(value&1)!=0;
            if(previous && !controller_strobe) controller_index=0;
            if(controller_strobe) controller_index=0;
            return;
        }
        if(address==0x4014) {
            // Illustrative page copy only. A real OAM DMA requires 513/514
            // cycles and interacts with CPU/APU arbitration and bus phases.
            for(unsigned i=0;i<256;++i)
                oam[i]=read(static_cast<U16>((U16(value)<<8)|i));
            ++dma_requests;
            return;
        }
        // Mapper writes and APU registers beyond controller/DMA: unsupported.
    }
    void tick(unsigned cpu_count) {
        cpu_cycles+=cpu_count;
        ppu_dots+=static_cast<std::uint64_t>(cpu_count)*3;
    }
};

struct Cpu {
    Bus& bus;
    U8 a{},x{},y{},sp{0xFD},p{0x24};
    U16 pc{};

    enum Flag : U8 {
        Carry=0x01, Zero=0x02, InterruptDisable=0x04, Decimal=0x08,
        Overflow=0x40, Negative=0x80
    };
    bool flag(Flag bit) const { return (p&bit)!=0; }
    void set(Flag bit,bool yes) {
        if(yes) p|=bit;
        else p=static_cast<U8>(p&~bit);
    }
    void set_zn(U8 result) {
        set(Zero,result==0);
        set(Negative,(result&0x80)!=0);
    }
    U8 fetch() {const U8 value=bus.read(pc);++pc;return value;}
    U16 fetch16() {
        const U16 lo=fetch(),hi=fetch();
        return static_cast<U16>(lo|(hi<<8));
    }
    void reset() {
        a=x=y=0;sp=0xFD;p=0x24;
        pc=static_cast<U16>(bus.read(0xFFFC)|(U16(bus.read(0xFFFD))<<8));
        // Real NES startup has additional CPU/PPU internal timing behavior.
        bus.tick(7);
    }
    void adc(U8 value) {
        const unsigned sum=unsigned(a)+value+(flag(Carry)?1u:0u);
        const U8 result=static_cast<U8>(sum);
        set(Carry,sum>255);
        set(Overflow,((~(a^value)&(a^result)&0x80)!=0));
        a=result;
        set_zn(a);
        // NES 2A03 disables decimal arithmetic even if D flag is set.
    }
    void push(U8 value) {
        bus.write(static_cast<U16>(0x0100|sp),value);
        --sp;
    }
    U8 pull() {
        ++sp;
        return bus.read(static_cast<U16>(0x0100|sp));
    }
    // An instruction returns the nominal CPU cycles consumed. This is not
    // a bus-cycle microsequencer: MMIO accesses occur instantaneously here.
    unsigned step() {
        const U8 op=fetch();
        unsigned cycles{};
        switch(op) {
            case 0xA9: a=fetch();set_zn(a);cycles=2;break; // LDA #imm
            case 0xA5: a=bus.read(fetch());set_zn(a);cycles=3;break;
            case 0xAD: a=bus.read(fetch16());set_zn(a);cycles=4;break;
            case 0x69: adc(fetch());cycles=2;break; // ADC #imm
            case 0x85: bus.write(fetch(),a);cycles=3;break; // STA zp
            case 0x8D: {const U16 addr=fetch16();bus.write(addr,a);
                         cycles=4;break;} // STA abs
            case 0xE8: ++x;set_zn(x);cycles=2;break; // INX
            case 0x18: set(Carry,false);cycles=2;break; // CLC
            case 0x38: set(Carry,true);cycles=2;break; // SEC
            case 0xC9: {const U8 v=fetch();const U8 difference=U8(a-v);
                         set(Carry,a>=v);set_zn(difference);cycles=2;break;}
            case 0xD0: { // BNE signed offset relative to next opcode.
                const auto offset=static_cast<std::int8_t>(fetch());
                cycles=2;
                if(!flag(Zero)) {
                    const U16 before=pc;
                    pc=static_cast<U16>(pc+offset);
                    ++cycles;
                    if((before&0xFF00)!=(pc&0xFF00)) ++cycles;
                }
                break;
            }
            case 0x4C: pc=fetch16();cycles=3;break; // JMP abs
            case 0x20: { // JSR pushes return address = address of final operand.
                const U8 low=fetch();
                const U16 ret=pc;
                push(static_cast<U8>(ret>>8));
                push(static_cast<U8>(ret&255));
                const U8 high=fetch();
                pc=static_cast<U16>((U16(high)<<8)|low);
                cycles=6;break;
            }
            case 0x60: { // RTS pulls return address then adds one.
                const U16 lo=pull(),hi=pull();
                pc=static_cast<U16>((hi<<8)|lo);
                ++pc;cycles=6;break;
            }
            case 0xEA: cycles=2;break; // NOP
            default:
                throw std::runtime_error("unimplemented opcode in CPU subset");
        }
        bus.tick(cycles);
        return cycles;
    }
};

// Store endian-independent, versioned snapshots, not native C++ struct bytes.
// Save state includes console memory, CPU registers and counters; it does not
// serialize a working PPU/APU/mapper because those devices are not modeled.
void append64(std::vector<U8>& out,std::uint64_t v) {
    for(unsigned i=0;i<8;++i) out.push_back(U8(v>>(i*8)));
}
std::uint64_t read64(const std::vector<U8>& v,std::size_t at) {
    check(at+8<=v.size(),"truncated state integer");
    std::uint64_t result{};
    for(unsigned i=0;i<8;++i) result|=(std::uint64_t(v[at+i])<<(i*8));
    return result;
}
std::vector<U8> save_state(const Cpu& cpu) {
    const Bus& bus=cpu.bus;
    std::vector<U8> out{'A','M','N','S',1};
    out.push_back(cpu.a);out.push_back(cpu.x);out.push_back(cpu.y);
    out.push_back(cpu.sp);out.push_back(cpu.p);
    out.push_back(U8(cpu.pc));out.push_back(U8(cpu.pc>>8));
    append64(out,bus.cpu_cycles);
    append64(out,bus.ppu_dots);
    out.insert(out.end(),bus.ram.begin(),bus.ram.end());
    out.insert(out.end(),bus.oam.begin(),bus.oam.end());
    return out;
}
void load_state(Cpu& cpu,const std::vector<U8>& input) {
    constexpr std::size_t total=5+7+8+8+2048+256;
    check(input.size()==total,"state size/version does not match");
    check(input[0]=='A'&&input[1]=='M'&&input[2]=='N'&&
          input[3]=='S'&&input[4]==1,"state signature or version");
    std::size_t at=5;
    const U8 a=input[at++],x=input[at++],y=input[at++];
    const U8 sp=input[at++],p=input[at++];
    const U16 pc=static_cast<U16>(input[at]|(U16(input[at+1])<<8));
    at+=2;
    const auto cycles=read64(input,at);at+=8;
    const auto dots=read64(input,at);at+=8;
    // All validation finished before mutating the CPU/bus.
    cpu.a=a;cpu.x=x;cpu.y=y;cpu.sp=sp;cpu.p=p;cpu.pc=pc;
    cpu.bus.cpu_cycles=cycles;cpu.bus.ppu_dots=dots;
    std::copy_n(input.begin()+static_cast<std::ptrdiff_t>(at),2048,
                cpu.bus.ram.begin());
    at+=2048;
    std::copy_n(input.begin()+static_cast<std::ptrdiff_t>(at),256,
                cpu.bus.oam.begin());
}

std::vector<U8> make_nrom(const std::vector<U8>& program) {
    std::vector<U8> image(16+16384,0);
    image[0]='N';image[1]='E';image[2]='S';image[3]=0x1A;
    image[4]=1;image[5]=0; // CHR RAM.
    check(program.size()<16384-6,"fixture program too large");
    std::copy(program.begin(),program.end(),image.begin()+16);
    // CPU $FFFC/FFFD mirror the final 16 KB PRG bank at offset $3FFC.
    image[16+0x3FFC]=0;
    image[16+0x3FFD]=0x80;
    return image;
}
void test_ines_bus_reset() {
    const auto image=make_nrom({0xEA});
    const auto cart=parse_nrom_ines(image);
    check(cart.prg.size()==16384 && cart.chr.size()==8192 &&
          cart.chr_is_ram,"iNES NROM mapping");
    Bus bus;bus.cartridge=cart;
    bus.write(0x0000,0x47);
    check(bus.read(0x0800)==0x47&&bus.read(0x1800)==0x47,
          "2KiB RAM mirrors");
    Cpu cpu{bus};cpu.reset();
    check(cpu.pc==0x8000&&bus.read(0xC000)==0xEA,"reset vector and PRG mirror");
    bus.controller_bits={1,0,1,0,0,1,0,1};
    bus.write(0x4016,1);
    check(bus.read(0x4016)==1,"strobe reads A repeatedly");
    bus.write(0x4016,0);
    for(const U8 bit:bus.controller_bits)
        check(bus.read(0x4016)==bit,"latched controller shifts eight bits");
    check(bus.read(0x4016)==1,"controller past eighth bit yields one");
    bus.write(0x4014,0);
    check(bus.oam[0]==0x47 && bus.dma_requests==1,"illustrative OAM page transfer");
    auto malformed=image;malformed[6]=0x10;
    bool unsupported=false;
    try { (void)parse_nrom_ines(malformed); }
    catch(const std::runtime_error&) {unsupported=true;}
    check(unsupported,"unsupported mapper rejected");
}

void test_6502_flags_cycles_and_replay() {
    // LDA #$7F; CLC; ADC #$01; STA $0020; INX; BNE +2;
    // skipped: LDA #$00; final: NOP; JMP $800C (loop).
    const std::vector<U8> program{
        0xA9,0x7F,0x18,0x69,0x01,0x8D,0x20,0x00,
        0xE8,0xD0,0x02,0xA9,0x00,0xEA,0x4C,0x0D,0x80
    };
    Bus bus;bus.cartridge=parse_nrom_ines(make_nrom(program));
    Cpu cpu{bus};cpu.reset();
    check(cpu.step()==2 && cpu.a==0x7F,"LDA immediate");
    check(cpu.step()==2,"CLC");
    check(cpu.step()==2 && cpu.a==0x80,"ADC immediate");
    check(!cpu.flag(Cpu::Carry) && cpu.flag(Cpu::Overflow) &&
          cpu.flag(Cpu::Negative),"6502 carry vs signed overflow");
    check(cpu.step()==4 && bus.read(0x20)==0x80,"STA absolute");
    check(cpu.step()==2 && cpu.x==1,"INX");
    check(cpu.step()==3 && cpu.pc==0x800D,"taken BNE 3 cycles");
    const auto snapshot=save_state(cpu);
    const U16 pc_before=cpu.pc;
    check(cpu.step()==2 && cpu.pc==pc_before+1,"NOP");
    const auto cycles_before=bus.cpu_cycles;
    load_state(cpu,snapshot);
    check(cpu.pc==pc_before && save_state(cpu)==snapshot,
          "save state replay and canonical bytes");
    check(cpu.step()==2 && bus.cpu_cycles==cycles_before,
          "restored cycle counter deterministic replay");
    auto invalid=snapshot;invalid[4]=2;
    bool rejected=false;
    try {load_state(cpu,invalid);} catch(const std::runtime_error&){rejected=true;}
    check(rejected && cpu.pc==0x800E,"version rejected without CPU mutation");

    // Zero/carry wrapping: $FF + $01 = $00 with carry, zero set, overflow clear.
    cpu.a=0xFF;cpu.set(Cpu::Carry,false);cpu.adc(1);
    check(cpu.a==0 && cpu.flag(Cpu::Carry) && cpu.flag(Cpu::Zero) &&
          !cpu.flag(Cpu::Overflow),"unsigned carry is not signed overflow");
}
} // namespace allmight::nes

int main(){
    try {
        allmight::nes::test_ines_bus_reset();
        allmight::nes::test_6502_flags_cycles_and_replay();
        std::cout<<"phase28_emulator_core_lab: all checks passed\n";
    }catch(const std::exception& e){
        std::cerr<<"phase28_emulator_core_lab: FAIL: "<<e.what()<<"\n";
        return 1;
    }
    return 0;
}
