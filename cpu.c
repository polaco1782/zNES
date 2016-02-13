#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "mem.h"

#define SET_Z(v) cpu.Z = !v;
#define SET_N(v) cpu.N = (v>>7)&1;
#define SET_ZN(v) SET_Z(v) SET_N(v)

enum
{
    Absolute         = 1,
    AbsoluteX        = 2,
    AbsoluteY        = 3,
    Accumulator      = 4,
    Immediate        = 5,
    Implied          = 6,
    IndexedIndirect  = 7,
    Indirect         = 8,
    IndirectIndexed  = 9,
    Relative         = 10,
    ZeroPage         = 11,
    ZeroPageX        = 12,
    ZeroPageY        = 13
};

// op size used to increment PC register
const uint8_t opsize[256] = 
{
    1, 2, 0, 0, 2, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
    2, 2, 0, 0, 2, 2, 2, 0, 1, 3, 1, 0, 3, 3, 3, 0,
    3, 2, 0, 0, 2, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
    2, 2, 0, 0, 2, 2, 2, 0, 1, 3, 1, 0, 3, 3, 3, 0,
    1, 2, 0, 0, 2, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
    2, 2, 0, 0, 2, 2, 2, 0, 1, 3, 1, 0, 3, 3, 3, 0,
    1, 2, 0, 0, 2, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
    2, 2, 0, 0, 2, 2, 2, 0, 1, 3, 1, 0, 3, 3, 3, 0,
    2, 2, 0, 0, 2, 2, 2, 0, 1, 0, 1, 0, 3, 3, 3, 0,
    2, 2, 0, 0, 2, 2, 2, 0, 1, 3, 1, 0, 0, 3, 0, 0,
    2, 2, 2, 0, 2, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
    2, 2, 0, 0, 2, 2, 2, 0, 1, 3, 1, 0, 3, 3, 3, 0,
    2, 2, 0, 0, 2, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
    2, 2, 0, 0, 2, 2, 2, 0, 1, 3, 1, 0, 3, 3, 3, 0,
    2, 2, 0, 0, 2, 2, 2, 0, 1, 2, 1, 0, 3, 3, 3, 0,
    2, 2, 0, 0, 2, 2, 2, 0, 1, 3, 1, 0, 3, 3, 3, 0
};

// op cpu cycles taken
const uint8_t opcycles[265] =
{
    7, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 4, 4, 6, 6,
    2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 4, 4, 6, 6,
    2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    6, 6, 2, 8, 3, 3, 5, 5, 3, 2, 2, 2, 3, 4, 6, 6,
    2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    6, 6, 2, 8, 3, 3, 5, 5, 4, 2, 2, 2, 5, 4, 6, 6,
    2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
    2, 6, 2, 6, 4, 4, 4, 4, 2, 5, 2, 5, 5, 5, 5, 5,
    2, 6, 2, 6, 3, 3, 3, 3, 2, 2, 2, 2, 4, 4, 4, 4,
    2, 5, 2, 5, 4, 4, 4, 4, 2, 4, 2, 4, 4, 4, 4, 4,
    2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
    2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
    2, 6, 2, 8, 3, 3, 5, 5, 2, 2, 2, 2, 4, 4, 6, 6,
    2, 5, 2, 8, 4, 4, 6, 6, 2, 4, 2, 7, 4, 4, 7, 7,
};

// op addressing modes
const uint8_t opmodes[256] =
{
    6, 7, 6, 7, 11, 11, 11, 11, 6, 5, 4, 5, 1, 1, 1, 1,
    10, 9, 6, 9, 12, 12, 12, 12, 6, 3, 6, 3, 2, 2, 2, 2,
    1, 7, 6, 7, 11, 11, 11, 11, 6, 5, 4, 5, 1, 1, 1, 1,
    10, 9, 6, 9, 12, 12, 12, 12, 6, 3, 6, 3, 2, 2, 2, 2,
    6, 7, 6, 7, 11, 11, 11, 11, 6, 5, 4, 5, 1, 1, 1, 1,
    10, 9, 6, 9, 12, 12, 12, 12, 6, 3, 6, 3, 2, 2, 2, 2,
    6, 7, 6, 7, 11, 11, 11, 11, 6, 5, 4, 5, 8, 1, 1, 1,
    10, 9, 6, 9, 12, 12, 12, 12, 6, 3, 6, 3, 2, 2, 2, 2,
    5, 7, 5, 7, 11, 11, 11, 11, 6, 5, 6, 5, 1, 1, 1, 1,
    10, 9, 6, 9, 12, 12, 13, 13, 6, 3, 6, 3, 2, 2, 3, 3,
    5, 7, 5, 7, 11, 11, 11, 11, 6, 5, 6, 5, 1, 1, 1, 1,
    10, 9, 6, 9, 12, 12, 13, 13, 6, 3, 6, 3, 2, 2, 3, 3,
    5, 7, 5, 7, 11, 11, 11, 11, 6, 5, 6, 5, 1, 1, 1, 1,
    10, 9, 6, 9, 12, 12, 12, 12, 6, 3, 6, 3, 2, 2, 2, 2,
    5, 7, 5, 7, 11, 11, 11, 11, 6, 5, 6, 5, 1, 1, 1, 1,
    10, 9, 6, 9, 12, 12, 12, 12, 6, 3, 6, 3, 2, 2, 2, 2
};

// extra cycles added when page is crossed
const uint8_t pagecrosscyles[256] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0,
};

typedef struct
{
    uint8_t A;
    uint8_t X;
    uint8_t Y;

    union
    {
        // bit access to state register
        struct
        {
            uint8_t C:1;
            uint8_t Z:1;
            uint8_t I:1;
            uint8_t D:1;
            uint8_t B:1;
            uint8_t R:1;
            uint8_t V:1;
            uint8_t N:1;
        };
        uint8_t regs;
    };

    bool NMI;
    bool IRQ;

    uint8_t OP;     // opcode
    uint8_t SP;     // stack pointer
    uint16_t PC;    // program counter

    uint64_t cycles;
    uint32_t stall;

    uint16_t memaddr;
} Cpu;

Cpu cpu;

void cpu_reset()
{
    cpu.PC = mem_read16(0xfffc);
    cpu.SP = 0xff;
    cpu.NMI = 0;
    cpu.IRQ = 0;
    cpu.A = 0;
    cpu.X = 0;
    cpu.Y = 0;
    cpu.Z = 1;
    cpu.R = 1;
    cpu.cycles = 0;
    cpu.stall = 0;
}

static inline void push(uint8_t value)
{
    // push a byte into stack
    mem_write(0x100+cpu.SP, value);
    cpu.SP--;
}

void push16(uint16_t value)
{
    push(value>>8);
    push(value&0xff);
}

static inline uint8_t pull()
{
    // pull a byte from stack
    cpu.SP++;
    return mem_read(0x100+cpu.SP);
}

static uint16_t pull16()
{
    return pull()|pull()<<8;
}

static uint16_t read16wrap(uint16_t addr)
{
    // TODO: simulate wrap around lower nibble bug
    //uint8_t hi = (addr&0xff00)>>8;
    //uint8_t lo = (addr&0xff)+1;

    //return mem_read(hi)<<8 | mem_read(lo);
    return mem_read16(addr);
}

static inline bool pagecrossed(uint16_t a, uint16_t b)
{
    return ((a>>8)&0xff!=(b>>8)&0xff);
}

static void branchaddcycle()
{
    cpu.cycles++;

    // add another cycle on crossed page
    if(pagecrossed(cpu.PC, cpu.memaddr))
        cpu.cycles++;
}

void cpu_debug()
{
    printf("PC: %04x, A: %02x, X: %02x, Y: %02x, SP: %02x\n", cpu.PC, cpu.A, cpu.X, cpu.Y, cpu.SP);
    printf("C-Z-I-D-B-R-V-N\n");
    printf("%d %d %d %d %d %d %d %d\n\n", cpu.C, cpu.Z, cpu.I, cpu.D, cpu.B, cpu.R, cpu.V, cpu.N);
}

static void nullop()
{
    printf("ERROR: invalid opcode 0x%02x (%d)\n", cpu.OP, cpu.OP);
    exit(0);
}

static void cpu_and() { cpu.A &= mem_read(cpu.memaddr); SET_ZN(cpu.A) };
static void cpu_bcc() { if(!cpu.C) { branchaddcycle(); cpu.PC += (int8_t)mem_read(cpu.memaddr); } };
static void cpu_bcs() { if(cpu.C) { branchaddcycle(); cpu.PC += (int8_t)mem_read(cpu.memaddr); } };
static void cpu_beq() { if(cpu.Z) { branchaddcycle(); cpu.PC += (int8_t)mem_read(cpu.memaddr); } };
static void cpu_bit() { uint8_t v = mem_read(cpu.memaddr); cpu.V = (v>>6)&1; SET_Z((v&cpu.A)) SET_N(v)};
static void cpu_bmi() { if(cpu.N) { branchaddcycle(); cpu.PC += (int8_t)mem_read(cpu.memaddr); } };
static void cpu_bne() { if(!cpu.Z) { branchaddcycle(); cpu.PC += (int8_t)mem_read(cpu.memaddr); } };
static void cpu_bpl() { if(!cpu.N) { branchaddcycle(); cpu.PC += (int8_t)mem_read(cpu.memaddr); } };
static void cpu_brk() { push16(cpu.PC); push(cpu.regs); cpu.I = 1; cpu.PC = mem_read16(0xfffe); };
static void cpu_bvc() { if(!cpu.V) { branchaddcycle(); cpu.PC += (int8_t)mem_read(cpu.memaddr); } };
static void cpu_bvs() { if(cpu.V) { branchaddcycle(); cpu.PC += (int8_t)mem_read(cpu.memaddr); } };
static void cpu_clc() { cpu.C = 0; } // 24
static void cpu_cld() { cpu.D = 0; } // 216
static void cpu_cli() { cpu.I = 0; } // 88
static void cpu_clv() { cpu.V = 0; } // 184
static void cpu_cmp() { uint8_t b = mem_read(cpu.memaddr), v = cpu.A - b; cpu.C = (cpu.A >= b); SET_ZN(v); };
static void cpu_cpx() { uint8_t b = mem_read(cpu.memaddr), v = cpu.X - b; cpu.C = (cpu.X >= b); SET_ZN(v); };
static void cpu_cpy() { uint8_t b = mem_read(cpu.memaddr), v = cpu.Y - b; cpu.C = (cpu.Y >= b); SET_ZN(v); };
static void cpu_dec() { uint8_t v = mem_read(cpu.memaddr)-1; mem_write(cpu.memaddr, v); SET_ZN(v) };
static void cpu_dex() { cpu.X--; SET_ZN(cpu.X) };
static void cpu_dey() { cpu.Y--; SET_ZN(cpu.Y) };
static void cpu_eor() { cpu.A ^= mem_read(cpu.memaddr); SET_ZN(cpu.A) };
static void cpu_inc() { uint8_t v = mem_read(cpu.memaddr)+1; mem_write(cpu.memaddr, v); SET_ZN(v) };
static void cpu_inx() { cpu.X++; SET_ZN(cpu.X) };
static void cpu_iny() { cpu.Y++; SET_ZN(cpu.Y) };
static void cpu_jmp() { cpu.PC = cpu.memaddr; };
static void cpu_jsr() { push16(cpu.PC-1); cpu.PC = cpu.memaddr; };
static void cpu_lda() { cpu.A = mem_read(cpu.memaddr); SET_ZN(cpu.A) };
static void cpu_ldx() { cpu.X = mem_read(cpu.memaddr); SET_ZN(cpu.X) };
static void cpu_ldy() { cpu.Y = mem_read(cpu.memaddr); SET_ZN(cpu.Y) };
static void cpu_nop() { };
static void cpu_ora() { cpu.A |= mem_read(cpu.memaddr); SET_ZN(cpu.A) };
static void cpu_pha() { push(cpu.A); };
static void cpu_php() { push(cpu.regs); };
static void cpu_pla() { cpu.A = pull(); SET_ZN(cpu.A) };
static void cpu_plp() { cpu.regs = pull(); };
static void cpu_rti() { cpu.regs = pull(); cpu.PC = pull16(); cpu.R = 1; };
static void cpu_rts() { cpu.PC = pull16()+1; };
static void cpu_sec() { cpu.C = 1; };
static void cpu_sed() { cpu.D = 1; };
static void cpu_sei() { cpu.I = 1; };
static void cpu_sta() { mem_write(cpu.memaddr, cpu.A); };
static void cpu_stx() { mem_write(cpu.memaddr, cpu.X); };
static void cpu_sty() { mem_write(cpu.memaddr, cpu.Y); };
static void cpu_tax() { cpu.X = cpu.A; SET_ZN(cpu.X) };
static void cpu_tay() { cpu.Y = cpu.A; SET_ZN(cpu.Y) };
static void cpu_tsx() { cpu.X = cpu.SP; SET_ZN(cpu.X) };
static void cpu_txa() { cpu.A = cpu.X; SET_ZN(cpu.A) };
static void cpu_txs() { cpu.SP = cpu.X; };
static void cpu_tya() { cpu.A = cpu.Y; SET_ZN(cpu.A) };
static void cpu_adc()
{
    uint8_t a,b,c;
    uint16_t sum;

    a = cpu.A;
    b = mem_read(cpu.memaddr);
    c = cpu.C;

    sum = a+b+c;
    cpu.C = sum>0xff;
    cpu.V = ~(a^b)&(a^sum)&0x80?1:0;
    cpu.A = sum&0xff;
    SET_ZN(cpu.A);
};
static void cpu_asl()
{
    uint8_t v;

    if(opmodes[cpu.OP]==Accumulator)
    {
        cpu.C = (cpu.A>>7)&1;
        cpu.A <<= 1;
        SET_ZN(cpu.A);
    }
    else
    {
        v = mem_read(cpu.memaddr);
        cpu.C = (v>>7)&1;
        v <<= 1;
        mem_write(cpu.memaddr, v);
        SET_ZN(v);
    }
};
static void cpu_lsr()
{
    uint8_t v;

    if(opmodes[cpu.OP]==Accumulator)
    {
        cpu.C = cpu.A&1;
        cpu.A >>= 1;
        SET_ZN(cpu.A);
    }
    else
    {
        v = mem_read(cpu.memaddr);
        cpu.C = v&1;
        v >>= 1;
        mem_write(cpu.memaddr, v);
        SET_ZN(v);
    }
};
static void cpu_rol()
{
    uint8_t v1,v2;

    if(opmodes[cpu.OP]==Accumulator)
    {
        v1 = cpu.C;
        cpu.C = (cpu.A>>7)&1;
        cpu.A = (cpu.A<<1)|v1;
        SET_ZN(cpu.A);
    }
    else
    {
        v1 = cpu.C;
        v2 = mem_read(cpu.memaddr);
        cpu.C = (v2>>7)&1;
        v2 = (v2<<1)|v1;
        mem_write(cpu.memaddr, v2);
        SET_ZN(v2);
    }
};
static void cpu_ror()
{
    uint8_t v1,v2;

    if(opmodes[cpu.OP]==Accumulator)
    {
        v1 = cpu.C;
        cpu.C = cpu.A&1;
        cpu.A = (cpu.A>>1)|v1<<7;
        SET_ZN(cpu.A);
    }
    else
    {
        v1 = cpu.C;
        v2 = mem_read(cpu.memaddr);
        cpu.C = v2&1;
        v2 = (v2>>1)|v1<<7;
        mem_write(cpu.memaddr, v2);
        SET_ZN(v2);
    }
};
static void cpu_sbc()
{
    uint8_t a,b,c;
    uint16_t sum;

    a = cpu.A;
    b = mem_read(cpu.memaddr);
    c = cpu.C;

    sum = a-b-(!c);
    cpu.C = !(sum&0x100);
    cpu.V = (a^b)&(a^sum)&0x80?1:0;
    cpu.A = sum&0xff;
    SET_ZN(cpu.A);
};
void cpu_nmi()
{
    push16(cpu.PC);
    push(cpu.regs);
    cpu.PC = mem_read16(0xfffa);
    cpu.I = 1;
    cpu.cycles += 7;
}
void cpu_irq()
{
    push16(cpu.PC);
    push(cpu.regs);
    cpu.PC = mem_read16(0xfffe);
    cpu.I = 1;
    cpu.cycles += 7;
}

void cpu_nmi_notify()
{
    cpu.NMI = true;
}

void cpu_stall_notify(uint32_t value)
{
    cpu.stall += value;

    if(cpu.cycles%2==1)
        cpu.stall++;
}

typedef void (*optable_t)(void);
optable_t optable[] =
{
    cpu_brk, cpu_ora, nullop, nullop, cpu_nop, cpu_ora, cpu_asl, nullop, 
    cpu_php, cpu_ora, cpu_asl, nullop, cpu_nop, cpu_ora, cpu_asl, nullop, 
    cpu_bpl, cpu_ora, nullop, nullop, cpu_nop, cpu_ora, cpu_asl, nullop, 
    cpu_clc, cpu_ora, cpu_nop, nullop, cpu_nop, cpu_ora, cpu_asl, nullop, 
    cpu_jsr, cpu_and, nullop, nullop, cpu_bit, cpu_and, cpu_rol, nullop, 
    cpu_plp, cpu_and, cpu_rol, nullop, cpu_bit, cpu_and, cpu_rol, nullop, 
    cpu_bmi, cpu_and, nullop, nullop, cpu_nop, cpu_and, cpu_rol, nullop, 
    cpu_sec, cpu_and, cpu_nop, nullop, cpu_nop, cpu_and, cpu_rol, nullop, 
    cpu_rti, cpu_eor, nullop, nullop, cpu_nop, cpu_eor, cpu_lsr, nullop, 
    cpu_pha, cpu_eor, cpu_lsr, nullop, cpu_jmp, cpu_eor, cpu_lsr, nullop, 
    cpu_bvc, cpu_eor, nullop, nullop, cpu_nop, cpu_eor, cpu_lsr, nullop, 
    cpu_cli, cpu_eor, cpu_nop, nullop, cpu_nop, cpu_eor, cpu_lsr, nullop, 
    cpu_rts, cpu_adc, nullop, nullop, cpu_nop, cpu_adc, cpu_ror, nullop, 
    cpu_pla, cpu_adc, cpu_ror, nullop, cpu_jmp, cpu_adc, cpu_ror, nullop, 
    cpu_bvs, cpu_adc, nullop, nullop, cpu_nop, cpu_adc, cpu_ror, nullop, 
    cpu_sei, cpu_adc, cpu_nop, nullop, cpu_nop, cpu_adc, cpu_ror, nullop, 
    cpu_nop, cpu_sta, cpu_nop, nullop, cpu_sty, cpu_sta, cpu_stx, nullop, 
    cpu_dey, cpu_nop, cpu_txa, nullop, cpu_sty, cpu_sta, cpu_stx, nullop, 
    cpu_bcc, cpu_sta, nullop, nullop, cpu_sty, cpu_sta, cpu_stx, nullop, 
    cpu_tya, cpu_sta, cpu_txs, nullop, nullop, cpu_sta, nullop, nullop,
    cpu_ldy, cpu_lda, cpu_ldx, nullop, cpu_ldy, cpu_lda, cpu_ldx, nullop, 
    cpu_tay, cpu_lda, cpu_tax, nullop, cpu_ldy, cpu_lda, cpu_ldx, nullop, 
    cpu_bcs, cpu_lda, nullop, nullop, cpu_ldy, cpu_lda, cpu_ldx, nullop, 
    cpu_clv, cpu_lda, cpu_tsx, nullop, cpu_ldy, cpu_lda, cpu_ldx, nullop, 
    cpu_cpy, cpu_cmp, cpu_nop, nullop, cpu_cpy, cpu_cmp, cpu_dec, nullop,
    cpu_iny, cpu_cmp, cpu_dex, nullop, cpu_cpy, cpu_cmp, cpu_dec, nullop,
    cpu_bne, cpu_cmp, nullop, nullop, cpu_nop, cpu_cmp, cpu_dec, nullop,
    cpu_cld, cpu_cmp, cpu_nop, nullop, cpu_nop, cpu_cmp, cpu_dec, nullop,
    cpu_cpx, cpu_sbc, cpu_nop, nullop, cpu_cpx, cpu_sbc, cpu_inc, nullop,
    cpu_inx, cpu_sbc, cpu_nop, cpu_sbc, cpu_cpx, cpu_sbc, cpu_inc, nullop,
    cpu_beq, cpu_sbc, nullop, nullop, cpu_nop, cpu_sbc, cpu_inc, nullop,
    cpu_sed, cpu_sbc, cpu_nop, nullop, cpu_nop, cpu_sbc, cpu_inc, nullop
};

uint32_t cpu_execop()
{
    uint64_t cycles;
    uint16_t addr;

    if(cpu.stall>0)
    {
        cpu.stall--;
        return 1;
    }

    if(cpu.NMI) cpu_nmi();

    cpu.NMI = 0;
    cpu.IRQ = 0;

    cpu.OP = mem_read(cpu.PC);

    switch(opmodes[cpu.OP])
    {
        case Absolute:
            cpu.memaddr = mem_read16(cpu.PC+1);
            break;
        case AbsoluteX:
            addr = mem_read16(cpu.PC+1) + cpu.X;
            if(pagecrossed(addr-cpu.X, addr))
                cpu.cycles += pagecrosscyles[cpu.OP];
            cpu.memaddr = addr;
            break;
        case AbsoluteY:
            addr = mem_read16(cpu.PC+1) + cpu.Y;
            if(pagecrossed(addr-cpu.Y, addr))
                cpu.cycles += pagecrosscyles[cpu.OP];
            cpu.memaddr = addr;
            break;
        case Accumulator:
            cpu.memaddr = 0;
            break;
        case Immediate:
            cpu.memaddr = cpu.PC+1;
            break;
        case Implied:
            cpu.memaddr = 0;
            break;
        case IndexedIndirect:
            cpu.memaddr = read16wrap((uint16_t)mem_read(cpu.PC+1)+cpu.X);
            break;
        case Indirect:
            cpu.memaddr = read16wrap(mem_read16(cpu.PC+1));
            break;
        case IndirectIndexed:
            addr = read16wrap((uint16_t)mem_read(cpu.PC+1))+cpu.Y;
            if(pagecrossed(addr-cpu.Y, addr))
                cpu.cycles += pagecrosscyles[cpu.OP];
            cpu.memaddr = addr;
            break;
        case Relative:
            cpu.memaddr = cpu.PC+1;
            break;
        case ZeroPage:
            cpu.memaddr = mem_read(cpu.PC+1);
            break;
        case ZeroPageX:
            cpu.memaddr = mem_read(cpu.PC+1)+cpu.X;
            break;
        case ZeroPageY:
            cpu.memaddr = mem_read(cpu.PC+1)+cpu.Y;
            break;
    }

    cycles = cpu.cycles;

    cpu.PC += opsize[cpu.OP];
    cpu.cycles += opcycles[cpu.OP];

    //if(cpu.PC == 0x8972)
    //    printf("BUG\n");

    // execute
    optable[cpu.OP]();

    //cpu_debug();

    // return per opcode cycles
    return cpu.cycles-cycles;
}
