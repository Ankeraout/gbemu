#include <string.h>

#include <gb/gb.h>

static inline void op_push(uint16_t value);
static inline uint8_t op_rl_base(uint8_t value);
static inline uint8_t op_rlc_base(uint8_t value);
static inline uint8_t op_rr_base(uint8_t value);
static inline uint8_t op_rrc_base(uint8_t value);

static const uint16_t gb_cpu_interruptJumpTable[32] = {
	0x00, // .....
	0x40, // ....1
	0x48, // ...1.
	0x40, // ...-1

	0x50, // ..1..
	0x40, // ..-.1
	0x48, // ..-1.
	0x40, // ..--1

	0x58, // .1...
	0x40, // .-..1
	0x48, // .-.1.
	0x40, // .-.-1

	0x50, // .-1..
	0x40, // .--.1
	0x48, // .--1.
	0x40, // .---1

	0x60, // 1....
	0x40, // -...1
	0x48, // -..1.
	0x40, // -..-1

	0x50, // -.1..
	0x40, // -.-.1
	0x48, // -.-1.
	0x40, // -.--1

	0x58, // -1...
	0x40, // --..1
	0x48, // --.1.
	0x40, // --.-1

	0x50, // --1..
	0x40, // ---.1
	0x48, // ---1.
	0x40  // ----1
};

static const uint8_t gb_cpu_interruptFlagClearTable[32] = {
	~0x00,
	~0x01,
	~0x02,
	~0x01,

	~0x04,
	~0x01,
	~0x02,
	~0x01,

	~0x08,
	~0x01,
	~0x02,
	~0x01,

	~0x04,
	~0x01,
	~0x02,
	~0x01,

	~0x10,
	~0x01,
	~0x02,
	~0x01,

	~0x04,
	~0x01,
	~0x02,
	~0x01,

	~0x08,
	~0x01,
	~0x02,
	~0x01,

	~0x04,
	~0x01,
	~0x02,
	~0x01,
};

/*
static const uint8_t gb_cpu_interruptFlagSetTable[32] = {
	0x00,
	0x01,
	0x02,
	0x01,

	0x04,
	0x01,
	0x02,
	0x01,

	0x08,
	0x01,
	0x02,
	0x01,

	0x04,
	0x01,
	0x02,
	0x01,

	0x10,
	0x01,
	0x02,
	0x01,

	0x04,
	0x01,
	0x02,
	0x01,

	0x08,
	0x01,
	0x02,
	0x01,

	0x04,
	0x01,
	0x02,
	0x01,
};
*/

static inline uint8_t fetch8() {
	return gb_mmu_read8_cycle(GB_CPU.pc++);
}

static inline uint16_t fetch16() {
	uint8_t low = fetch8();
	uint8_t high = fetch8();

	return (high << 8) | low;
}

static inline void op_adc_a(uint8_t value) {
	int carry = GB_CPU.flagC ? 1 : 0;

	GB_CPU.flagN = false;
	GB_CPU.flagH = (((GB_CPU.a & 0x0f) + (value & 0x0f) + carry) >= 0x10);
	GB_CPU.flagC = ((GB_CPU.a + value + carry) >= 0x100);

	GB_CPU.a += value + carry;

	GB_CPU.flagZ = (GB_CPU.a == 0);
}

static inline void op_add_a(uint8_t value) {
	GB_CPU.flagN = false;
	GB_CPU.flagH = (((GB_CPU.a & 0x0f) + (value & 0x0f)) >= 0x10);
	GB_CPU.flagC = ((GB_CPU.a + value) >= 0x100);

	GB_CPU.a += value;

	GB_CPU.flagZ = (GB_CPU.a == 0);
}

static inline void op_add_hl_r16(uint16_t value) {
	GB_CPU.flagN = false;
	GB_CPU.flagH = (GB_CPU.hl & 0xfff) + (value & 0xfff) >= 0x1000;
	GB_CPU.flagC = GB_CPU.hl + value >= 0x10000;

	GB_CPU.hl += value;

	gb_cycleNoCPU();
}

static inline void op_add_sp(int8_t value) {
	GB_CPU.flagZ = false;
	GB_CPU.flagN = false;
	GB_CPU.flagH = (((GB_CPU.sp & 0x0f) + (value & 0x0f)) >= 0x10);
	GB_CPU.flagC = (((GB_CPU.sp & 0xff) + (uint8_t)value) >= 0x100);

	GB_CPU.sp += value;

	gb_cycleNoCPU();
	gb_cycleNoCPU();
}

static inline void op_and_a(uint8_t value) {
	GB_CPU.a &= value;

	GB_CPU.flagZ = (GB_CPU.a == 0);
	GB_CPU.flagN = false;
	GB_CPU.flagH = true;
	GB_CPU.flagC = false;
}

static inline void op_bit(int bit, uint8_t value) {
	GB_CPU.flagZ = (((1 << bit) & value) == 0);
	GB_CPU.flagN = false;
	GB_CPU.flagH = true;
}

static inline void op_call_u16() {
	uint16_t address = fetch16();

	gb_cycleNoCPU();
	gb_mmu_write8_cycle(--GB_CPU.sp, GB_CPU.pc >> 8);
	gb_mmu_write8_cycle(--GB_CPU.sp, GB_CPU.pc);

	GB_CPU.pc = address;
}

static inline void op_call_u16_cond(bool cond) {
	uint16_t address = fetch16();

	if(cond) {
		gb_cycleNoCPU();
		gb_mmu_write8_cycle(--GB_CPU.sp, GB_CPU.pc >> 8);
		gb_mmu_write8_cycle(--GB_CPU.sp, GB_CPU.pc);

		GB_CPU.pc = address;
	}
}

static inline void op_cp_a(uint8_t value) {
	uint8_t tmp = GB_CPU.a;
	
	GB_CPU.flagN = true;
	GB_CPU.flagH = (((tmp & 0x0f) - (value & 0x0f)) < 0);
	GB_CPU.flagC = ((tmp - value) < 0);

	tmp -= value;

	GB_CPU.flagZ = (tmp == 0);
}

static inline void op_daa() {
	if(GB_CPU.flagN) {
		if(GB_CPU.flagC) {
			GB_CPU.a -= 0x60;
		} 

		if(GB_CPU.flagH) {
			GB_CPU.a -= 0x06;
		}
	} else {
		if(GB_CPU.flagC || GB_CPU.a > 0x99) {
			GB_CPU.a += 0x60;
			GB_CPU.flagC = true;
		}

		if(GB_CPU.flagH || (GB_CPU.a & 0x0f) > 0x09) {
			GB_CPU.a += 0x06;
		}
	}

	GB_CPU.flagZ = (GB_CPU.a == 0);
	GB_CPU.flagH = false;
}

static inline uint16_t op_dec16(uint16_t value) {
	gb_cycleNoCPU();

	return value - 1;
}

static inline uint8_t op_dec8(uint8_t value) {
	GB_CPU.flagZ = value == 0x01;
	GB_CPU.flagN = true;
	GB_CPU.flagH = (value & 0x0f) == 0x00;

	return value - 1;
}

static inline void op_di() {
	GB_CPU.ime = false;
	GB_CPU.ime_enableNextCycle = false;
}

static inline void op_ei() {
	GB_CPU.ime = false;
	GB_CPU.ime_enableNextCycle = true;
}

static inline uint16_t op_inc16(uint16_t value) {
	gb_cycleNoCPU();

	return value + 1;
}

static inline uint8_t op_inc8(uint8_t value) {
	GB_CPU.flagZ = value == 0xff;
	GB_CPU.flagN = false;
	GB_CPU.flagH = (value & 0x0f) == 0x0f;

	return value + 1;
}

static inline void op_jp_u16() {
	GB_CPU.pc = fetch16();

	gb_cycleNoCPU();
}

static inline void op_jp_u16_cond(bool cond) {
	uint16_t address = fetch16();

	if(cond) {
		GB_CPU.pc = address;
		gb_cycleNoCPU();
	}
}

static inline void op_jr8(int8_t value) {
	gb_cycleNoCPU();
	GB_CPU.pc += value;
}

static inline void op_jr8_cond(bool cond) {
	int8_t value = fetch8();

	if(cond) {
		op_jr8(value);
	}
}

static inline void op_ld_hl_sp_i8() {
	int8_t value = fetch8();

	GB_CPU.flagZ = false;
	GB_CPU.flagN = false;
	GB_CPU.flagH = (((GB_CPU.sp & 0x0f) + (value & 0x0f)) >= 0x10);
	GB_CPU.flagC = (((GB_CPU.sp & 0xff) + (uint8_t)value) >= 0x100);

	GB_CPU.hl = GB_CPU.sp + value;

	gb_cycleNoCPU();
}

static inline void op_ld_u16_sp() 
{
	uint16_t addr = fetch16();

	gb_mmu_write8_cycle(addr++, (uint8_t)(GB_CPU.sp));
	gb_mmu_write8_cycle(addr, (uint8_t)(GB_CPU.sp >> 8));
}

static inline void op_or_a(uint8_t value) {
	GB_CPU.a |= value;

	GB_CPU.flagZ = (GB_CPU.a == 0);
	GB_CPU.flagN = false;
	GB_CPU.flagH = false;
	GB_CPU.flagC = false;
}

static inline uint16_t op_pop() {
	uint8_t low = gb_mmu_read8_cycle(GB_CPU.sp++);
	uint8_t high = gb_mmu_read8_cycle(GB_CPU.sp++);

	return (high << 8) | low;
}

static inline void op_pop_af() {
	uint16_t value = op_pop();

	GB_CPU.a = value >> 8;
	
	GB_CPU.flagZ = ((value & 0x80) != 0);
	GB_CPU.flagN = ((value & 0x40) != 0);
	GB_CPU.flagH = ((value & 0x20) != 0);
	GB_CPU.flagC = ((value & 0x10) != 0);
}

static inline void op_push(uint16_t value) {
	gb_cycleNoCPU();

	gb_mmu_write8_cycle(--GB_CPU.sp, (uint8_t)(value >> 8));
	gb_mmu_write8_cycle(--GB_CPU.sp, (uint8_t)value);
}

static inline void op_push_af() {
	uint16_t tmp = GB_CPU.a << 8;

	if(GB_CPU.flagZ) tmp |= 0x80;
	if(GB_CPU.flagN) tmp |= 0x40;
	if(GB_CPU.flagH) tmp |= 0x20;
	if(GB_CPU.flagC) tmp |= 0x10;

	op_push(tmp);
}

static inline uint8_t op_res(int bit, uint8_t value) {
	return value & ~(1 << bit);
}

static inline void op_ret() {
	uint8_t low = gb_mmu_read8_cycle(GB_CPU.sp++);
	uint8_t high = gb_mmu_read8_cycle(GB_CPU.sp++);

	GB_CPU.pc = (high << 8) | low;

	gb_cycleNoCPU();
}

static inline void op_reti() {
	op_ret();
	GB_CPU.ime = true;
}

static inline void op_ret_cond(bool cond) {
	gb_cycleNoCPU();

	if(cond) {
		op_ret();
	}
}

static inline uint8_t op_rl(uint8_t value) {
	value = op_rl_base(value);

	GB_CPU.flagZ = value == 0x00;

	return value;
}

static inline uint8_t op_rl_base(uint8_t value) {
	uint8_t savedBit = GB_CPU.flagC;

	GB_CPU.flagN = false;
	GB_CPU.flagH = false;
	GB_CPU.flagC = value >> 7;

	return (value << 1) | savedBit;
}

static inline uint8_t op_rlc(uint8_t value) {
	value = op_rlc_base(value);

	GB_CPU.flagZ = value == 0x00;

	return value;
}

static inline uint8_t op_rlc_base(uint8_t value) {
	GB_CPU.flagN = false;
	GB_CPU.flagH = false;
	GB_CPU.flagC = value >> 7;
	
	return (value << 1) | GB_CPU.flagC;
}

static inline uint8_t op_rr(uint8_t value) {
	value = op_rr_base(value);

	GB_CPU.flagZ = value == 0x00;

	return value;
}

static inline uint8_t op_rr_base(uint8_t value) {
	uint8_t savedBit = GB_CPU.flagC << 7;

	GB_CPU.flagN = false;
	GB_CPU.flagH = false;
	GB_CPU.flagC = value & 0x01;

	return (value >> 1) | savedBit;
}

static inline uint8_t op_rrc(uint8_t value) {
	value = op_rrc_base(value);

	GB_CPU.flagZ = value == 0x00;

	return value;
}

static inline uint8_t op_rrc_base(uint8_t value) {
	GB_CPU.flagN = false;
	GB_CPU.flagH = false;
	GB_CPU.flagC = value & 0x01;

	return (value >> 1) | (GB_CPU.flagC << 7);
}

static inline void op_rst(uint16_t address) {
	gb_cycleNoCPU();

	gb_mmu_write8_cycle(--GB_CPU.sp, (uint8_t)(GB_CPU.pc >> 8));
	gb_mmu_write8_cycle(--GB_CPU.sp, (uint8_t)GB_CPU.pc);

	GB_CPU.pc = address;
}

static inline void op_sbc_a(uint8_t value) {
	int carry = GB_CPU.flagC ? 1 : 0;

	GB_CPU.flagN = true;
	GB_CPU.flagH = (((GB_CPU.a & 0x0f) - (value & 0x0f) - carry) < 0);
	GB_CPU.flagC = ((GB_CPU.a - value - carry) < 0);

	GB_CPU.a -= value + carry;

	GB_CPU.flagZ = (GB_CPU.a == 0);
}

static inline uint8_t op_set(int bit, uint8_t value) {
	return value | (1 << bit);
}

static inline uint8_t op_sla(uint8_t value) {
	GB_CPU.flagN = false;
	GB_CPU.flagH = false;
	GB_CPU.flagC = value >> 7;

	value <<= 1;

	GB_CPU.flagZ = (value == 0);

	return value;
}

static inline uint8_t op_sra(uint8_t value) {
	uint8_t savedBit = value & 0x80;

	GB_CPU.flagN = false;
	GB_CPU.flagH = false;
	GB_CPU.flagC = (value & 0x01) != 0;

	value >>= 1;
	value |= savedBit;

	GB_CPU.flagZ = (value == 0);

	return value;
}

static inline uint8_t op_srl(uint8_t value) {
	GB_CPU.flagN = false;
	GB_CPU.flagH = false;
	GB_CPU.flagC = (value & 0x01) != 0;

	value >>= 1;

	GB_CPU.flagZ = (value == 0);

	return value;
}

static inline void op_stop() {
	if(fetch8() == 0x00) {
		GB_CPU.stopped = true;
	} else {
		gbemu_error("Unknown opcode");
	}
}

static inline void op_sub_a(uint8_t value) {
	GB_CPU.flagN = true;
	GB_CPU.flagH = (((GB_CPU.a & 0x0f) - (value & 0x0f)) < 0);
	GB_CPU.flagC = ((GB_CPU.a - value) < 0);

	GB_CPU.a -= value;

	GB_CPU.flagZ = (GB_CPU.a == 0);
}

static inline uint8_t op_swap(uint8_t value) {
	GB_CPU.flagZ = (value == 0);
	GB_CPU.flagN = false;
	GB_CPU.flagH = false;
	GB_CPU.flagC = false;

	uint8_t high = value & 0x0f;
	uint8_t low = value >> 4;

	return (high << 4) | low;
}

static inline void op_xor_a(uint8_t value) {
	GB_CPU.a ^= value;

	GB_CPU.flagZ = (GB_CPU.a == 0);
	GB_CPU.flagN = false;
	GB_CPU.flagH = false;
	GB_CPU.flagC = false;
}

void gb_cpu_init() {
	memset(&GB_CPU, 0, sizeof(GB_CPU));
}

void gb_cpu_instruction() {
	// Check for interrupts
	uint8_t intFlags = GB_CPU.ioreg_if & GB.hram[0x7f] & 0x1f;

	// Check if the GameBoy should exit HALT state
	if(GB_CPU.halted && intFlags) {
		GB_CPU.halted = false;
		gb_cycleNoCPU();
		return;
	}

	// Do nothing if the CPU is halted
	if(GB_CPU.halted || GB_CPU.stopped) {
		gb_cycleNoCPU();
		return;
	}

	// If interrupts are enabled
	if(GB_CPU.ime) {
		// If at least one interrupt that is requested is enabled
		if(intFlags) {
			gb_cycleNoCPU();
			gb_cycleNoCPU();
			gb_cycleNoCPU();
			gb_mmu_write8_cycle(--GB_CPU.sp, GB_CPU.pc >> 8);
			gb_mmu_write8_cycle(--GB_CPU.sp, GB_CPU.pc);

			// Jump to the address of the interrupt handler
			GB_CPU.pc = gb_cpu_interruptJumpTable[intFlags];

			// Disable IME to prevent nested interrupts
			GB_CPU.ime = false;

			// Clear the interrupt bit
			GB_CPU.ioreg_if &= gb_cpu_interruptFlagClearTable[intFlags];

			return;
		}
	}

	// If IME should be enabled on next cycle, we enable it now that we've checked for interrupts.
	if(GB_CPU.ime_enableNextCycle) {
		GB_CPU.ime = true;
		GB_CPU.ime_enableNextCycle = false;
	}

	// Fetch the opcode
	uint8_t opcode = fetch8();

	// Decode the opcode
	switch(opcode) {
		case 0x00: // NOP
			break;

		case 0x01: // LD BC, u16
			GB_CPU.c = fetch8();
			GB_CPU.b = fetch8();
			break;

		case 0x02: // LD (BC), A
			gb_mmu_write8_cycle(GB_CPU.bc, GB_CPU.a);
			break;

		case 0x03: // INC BC
			GB_CPU.bc = op_inc16(GB_CPU.bc);
			break;

		case 0x04: // INC B
			GB_CPU.b = op_inc8(GB_CPU.b);
			break;

		case 0x05: // DEC B
			GB_CPU.b = op_dec8(GB_CPU.b);
			break;

		case 0x06: // LD B, u8
			GB_CPU.b = fetch8();
			break;

		case 0x07: // RLCA
			GB_CPU.a = op_rlc_base(GB_CPU.a);
			GB_CPU.flagZ = false;
			break;

		case 0x08: // LD (u16), SP
			op_ld_u16_sp();
			break;
		
		case 0x09: // ADD HL, BC
			op_add_hl_r16(GB_CPU.bc);
			break;

		case 0x0a: // LD A, (BC)
			GB_CPU.a = gb_mmu_read8_cycle(GB_CPU.bc);
			break;

		case 0x0b: // DEC BC
			GB_CPU.bc = op_dec16(GB_CPU.bc);
			break;

		case 0x0c: // INC C
			GB_CPU.c = op_inc8(GB_CPU.c);
			break;

		case 0x0d: // DEC C
			GB_CPU.c = op_dec8(GB_CPU.c);
			break;

		case 0x0e: // LD C, u8
			GB_CPU.c = fetch8();
			break;
		
		case 0x0f: // RRCA
			GB_CPU.a = op_rrc_base(GB_CPU.a);
			GB_CPU.flagZ = false;
			break;

		case 0x10: // STOP
			op_stop();
			break;

		case 0x11: // LD DE, u16
			GB_CPU.de = fetch16();
			break;

		case 0x12: // LD (DE), A
			gb_mmu_write8_cycle(GB_CPU.de, GB_CPU.a);
			break;

		case 0x13: // INC DE
			GB_CPU.de = op_inc16(GB_CPU.de);
			break;

		case 0x14: // INC D
			GB_CPU.d = op_inc8(GB_CPU.d);
			break;

		case 0x15: // DEC D
			GB_CPU.d = op_dec8(GB_CPU.d);
			break;

		case 0x16: // LD D, u8
			GB_CPU.d = fetch8();
			break;

		case 0x17: // RLA
			GB_CPU.a = op_rl_base(GB_CPU.a);
			GB_CPU.flagZ = false;
			break;

		case 0x18: // JR i8
			op_jr8(fetch8());
			break;

		case 0x19: // ADD HL, DE
			op_add_hl_r16(GB_CPU.de);
			break;

		case 0x1a: // LD A, (DE)
			GB_CPU.a = gb_mmu_read8_cycle(GB_CPU.de);
			break;

		case 0x1b: // DEC DE
			GB_CPU.de = op_dec16(GB_CPU.de);
			break;

		case 0x1c: // INC E
			GB_CPU.e = op_inc8(GB_CPU.e);
			break;

		case 0x1d: // DEC E
			GB_CPU.e = op_dec8(GB_CPU.e);
			break;

		case 0x1e: // LD E, u8
			GB_CPU.e = fetch8();
			break;

		case 0x1f: // RRA
			GB_CPU.a = op_rr_base(GB_CPU.a);
			GB_CPU.flagZ = false;
			break;

		case 0x20: // JR NZ, i8
			op_jr8_cond(!GB_CPU.flagZ);
			break;

		case 0x21: // LD HL, u16
			GB_CPU.hl = fetch16();
			break;

		case 0x22: // LD (HL+), A
			gb_mmu_write8_cycle(GB_CPU.hl++, GB_CPU.a);
			break;

		case 0x23: // INC HL
			GB_CPU.hl = op_inc16(GB_CPU.hl);
			break;

		case 0x24: // INC H
			GB_CPU.h = op_inc8(GB_CPU.h);
			break;

		case 0x25: // DEC H
			GB_CPU.h = op_dec8(GB_CPU.h);
			break;

		case 0x26: // LD H, u8
			GB_CPU.h = fetch8();
			break;

		case 0x27: // DAA
			op_daa();
			break;

		case 0x28: // JR Z, i8
			op_jr8_cond(GB_CPU.flagZ);
			break;

		case 0x29: // ADD HL, HL
			op_add_hl_r16(GB_CPU.hl);
			break;

		case 0x2a: // LD A, (HL+)
			GB_CPU.a = gb_mmu_read8_cycle(GB_CPU.hl++);
			break;

		case 0x2b: // DEC HL
			GB_CPU.hl = op_dec16(GB_CPU.hl);
			break;

		case 0x2c: // INC L
			GB_CPU.l = op_inc8(GB_CPU.l);
			break;

		case 0x2d: // DEC L
			GB_CPU.l = op_dec8(GB_CPU.l);
			break;

		case 0x2e: // LD L, u8
			GB_CPU.l = fetch8();
			break;

		case 0x2f: // CPL
			GB_CPU.flagN = true;
			GB_CPU.flagH = true;
			GB_CPU.a = ~GB_CPU.a;
			break;

		case 0x30: // JR NC, i8
			op_jr8_cond(!GB_CPU.flagC);
			break;

		case 0x31: // LD SP, u16
			GB_CPU.sp = fetch16();
			break;

		case 0x32: // LD (HL-), A
			gb_mmu_write8_cycle(GB_CPU.hl--, GB_CPU.a);
			break;

		case 0x33: // INC SP
			GB_CPU.sp = op_inc16(GB_CPU.sp);
			break;

		case 0x34: // INC (HL)
			gb_mmu_write8_cycle(GB_CPU.hl, op_inc8(gb_mmu_read8_cycle(GB_CPU.hl)));
			break;

		case 0x35: // DEC (HL)
			gb_mmu_write8_cycle(GB_CPU.hl, op_dec8(gb_mmu_read8_cycle(GB_CPU.hl)));
			break;

		case 0x36: // LD (HL), u8
			gb_mmu_write8_cycle(GB_CPU.hl, fetch8());
			break;

		case 0x37: // SCF
			GB_CPU.flagN = false;
			GB_CPU.flagH = false;
			GB_CPU.flagC = true;
			break;

		case 0x38: //JR C, i8
			op_jr8_cond(GB_CPU.flagC);
			break;

		case 0x39: // ADD HL, SP
			op_add_hl_r16(GB_CPU.sp);
			break;

		case 0x3a: // LD A, (HL-)
			GB_CPU.a = gb_mmu_read8_cycle(GB_CPU.hl--);
			break;

		case 0x3b: // DEC SP
			GB_CPU.sp = op_dec16(GB_CPU.sp);
			break;

		case 0x3c: // INC A
			GB_CPU.a = op_inc8(GB_CPU.a);
			break;

		case 0x3d: // DEC A
			GB_CPU.a = op_dec8(GB_CPU.a);
			break;

		case 0x3e: // LD A, u8
			GB_CPU.a = fetch8();
			break;

		case 0x3f: // CCF
			GB_CPU.flagN = false;
			GB_CPU.flagH = false;
			GB_CPU.flagC = !GB_CPU.flagC;
			break;
		
		case 0x40: // LD B, B
			GB_CPU.b = GB_CPU.b;
			break;

		case 0x41: // LD B, C
			GB_CPU.b = GB_CPU.c;
			break;

		case 0x42: // LD B, D
			GB_CPU.b = GB_CPU.d;
			break;

		case 0x43: // LD B, E
			GB_CPU.b = GB_CPU.e;
			break;

		case 0x44: // LD B, H
			GB_CPU.b = GB_CPU.h;
			break;

		case 0x45: // LD B, L
			GB_CPU.b = GB_CPU.l;
			break;

		case 0x46: // LD B, (HL)
			GB_CPU.b = gb_mmu_read8_cycle(GB_CPU.hl);
			break;

		case 0x47: // LD B, A
			GB_CPU.b = GB_CPU.a;
			break;

		case 0x48: // LD C, B
			GB_CPU.c = GB_CPU.b;
			break;

		case 0x49: // LD C, C
			GB_CPU.c = GB_CPU.c;
			break;

		case 0x4a: // LD C, D
			GB_CPU.c = GB_CPU.d;
			break;

		case 0x4b: // LD C, E
			GB_CPU.c = GB_CPU.e;
			break;

		case 0x4c: // LD C, H
			GB_CPU.c = GB_CPU.h;
			break;

		case 0x4d: // LD C, L
			GB_CPU.c = GB_CPU.l;
			break;

		case 0x4e: // LD C, (HL)
			GB_CPU.c = gb_mmu_read8_cycle(GB_CPU.hl);
			break;

		case 0x4f: // LD C, A
			GB_CPU.c = GB_CPU.a;
			break;

		case 0x50: // LD D, B
			GB_CPU.d = GB_CPU.b;
			break;

		case 0x51: // LD D, C
			GB_CPU.d = GB_CPU.c;
			break;

		case 0x52: // LD D, D
			GB_CPU.d = GB_CPU.d;
			break;

		case 0x53: // LD D, E
			GB_CPU.d = GB_CPU.e;
			break;

		case 0x54: // LD D, H
			GB_CPU.d = GB_CPU.h;
			break;

		case 0x55: // LD D, L
			GB_CPU.d = GB_CPU.l;
			break;

		case 0x56: // LD D, (HL)
			GB_CPU.d = gb_mmu_read8_cycle(GB_CPU.hl);
			break;

		case 0x57: // LD D, A
			GB_CPU.d = GB_CPU.a;
			break;

		case 0x58: // LD E, B
			GB_CPU.e = GB_CPU.b;
			break;

		case 0x59: // LD E, C
			GB_CPU.e = GB_CPU.c;
			break;

		case 0x5a: // LD E, D
			GB_CPU.e = GB_CPU.d;
			break;

		case 0x5b: // LD E, E
			GB_CPU.e = GB_CPU.e;
			break;

		case 0x5c: // LD E, H
			GB_CPU.e = GB_CPU.h;
			break;

		case 0x5d: // LD E, L
			GB_CPU.e = GB_CPU.l;
			break;

		case 0x5e: // LD E, (HL)
			GB_CPU.e = gb_mmu_read8_cycle(GB_CPU.hl);
			break;

		case 0x5f: // LD E, A
			GB_CPU.e = GB_CPU.a;
			break;

		case 0x60: // LD H, B
			GB_CPU.h = GB_CPU.b;
			break;

		case 0x61: // LD H, C
			GB_CPU.h = GB_CPU.c;
			break;

		case 0x62: // LD H, D
			GB_CPU.h = GB_CPU.d;
			break;

		case 0x63: // LD H, E
			GB_CPU.h = GB_CPU.e;
			break;

		case 0x64: // LD H, H
			GB_CPU.h = GB_CPU.h;
			break;

		case 0x65: // LD H, L
			GB_CPU.h = GB_CPU.l;
			break;

		case 0x66: // LD H, (HL)
			GB_CPU.h = gb_mmu_read8_cycle(GB_CPU.hl);
			break;

		case 0x67: // LD H, A
			GB_CPU.h = GB_CPU.a;
			break;

		case 0x68: // LD L, B
			GB_CPU.l = GB_CPU.b;
			break;

		case 0x69: // LD L, C
			GB_CPU.l = GB_CPU.c;
			break;

		case 0x6a: // LD L, D
			GB_CPU.l = GB_CPU.d;
			break;

		case 0x6b: // LD L, E
			GB_CPU.l = GB_CPU.e;
			break;

		case 0x6c: // LD L, H
			GB_CPU.l = GB_CPU.h;
			break;

		case 0x6d: // LD L, L
			GB_CPU.l = GB_CPU.l;
			break;

		case 0x6e: // LD L, (HL)
			GB_CPU.l = gb_mmu_read8_cycle(GB_CPU.hl);
			break;

		case 0x6f: // LD L, A
			GB_CPU.l = GB_CPU.a;
			break;

		case 0x70: // LD (HL), B
			gb_mmu_write8_cycle(GB_CPU.hl, GB_CPU.b);
			break;

		case 0x71: // LD (HL), C
			gb_mmu_write8_cycle(GB_CPU.hl, GB_CPU.c);
			break;

		case 0x72: // LD (HL), D
			gb_mmu_write8_cycle(GB_CPU.hl, GB_CPU.d);
			break;

		case 0x73: // LD (HL), E
			gb_mmu_write8_cycle(GB_CPU.hl, GB_CPU.e);
			break;

		case 0x74: // LD (HL), H
			gb_mmu_write8_cycle(GB_CPU.hl, GB_CPU.h);
			break;

		case 0x75: // LD (HL), L
			gb_mmu_write8_cycle(GB_CPU.hl, GB_CPU.l);
			break;

		case 0x76: // HALT
			GB_CPU.halted = true;
			break;

		case 0x77: // LD (HL), A
			gb_mmu_write8_cycle(GB_CPU.hl, GB_CPU.a);
			break;

		case 0x78: // LD A, B
			GB_CPU.a = GB_CPU.b;
			break;

		case 0x79: // LD A, C
			GB_CPU.a = GB_CPU.c;
			break;

		case 0x7a: // LD A, D
			GB_CPU.a = GB_CPU.d;
			break;

		case 0x7b: // LD A, E
			GB_CPU.a = GB_CPU.e;
			break;

		case 0x7c: // LD A, H
			GB_CPU.a = GB_CPU.h;
			break;

		case 0x7d: // LD A, L
			GB_CPU.a = GB_CPU.l;
			break;

		case 0x7e: // LD A, (HL)
			GB_CPU.a = gb_mmu_read8_cycle(GB_CPU.hl);
			break;

		case 0x7f: // LD A, A
			GB_CPU.a = GB_CPU.a;
			break;
		
		case 0x80: // ADD A, B
			op_add_a(GB_CPU.b);
			break;

		case 0x81: // ADD A, C
			op_add_a(GB_CPU.c);
			break;

		case 0x82: // ADD A, D
			op_add_a(GB_CPU.d);
			break;

		case 0x83: // ADD A, E
			op_add_a(GB_CPU.e);
			break;

		case 0x84: // ADD A, H
			op_add_a(GB_CPU.h);
			break;

		case 0x85: // ADD A, L
			op_add_a(GB_CPU.l);
			break;

		case 0x86: // ADD A, (HL)
			op_add_a(gb_mmu_read8_cycle(GB_CPU.hl));
			break;

		case 0x87: // ADD A, A
			op_add_a(GB_CPU.a);
			break;

		case 0x88: // ADC A, B
			op_adc_a(GB_CPU.b);
			break;

		case 0x89: // ADC A, C
			op_adc_a(GB_CPU.c);
			break;

		case 0x8a: // ADC A, D
			op_adc_a(GB_CPU.d);
			break;

		case 0x8b: // ADC A, E
			op_adc_a(GB_CPU.e);
			break;

		case 0x8c: // ADC A, H
			op_adc_a(GB_CPU.h);
			break;

		case 0x8d: // ADC A, L
			op_adc_a(GB_CPU.l);
			break;

		case 0x8e: // ADC A, (HL)
			op_adc_a(gb_mmu_read8_cycle(GB_CPU.hl));
			break;

		case 0x8f: // ADC A, A
			op_adc_a(GB_CPU.a);
			break;

		case 0x90: // SUB A, B
			op_sub_a(GB_CPU.b);
			break;

		case 0x91: // SUB A, C
			op_sub_a(GB_CPU.c);
			break;

		case 0x92: // SUB A, D
			op_sub_a(GB_CPU.d);
			break;

		case 0x93: // SUB A, E
			op_sub_a(GB_CPU.e);
			break;

		case 0x94: // SUB A, H
			op_sub_a(GB_CPU.h);
			break;

		case 0x95: // SUB A, L
			op_sub_a(GB_CPU.l);
			break;

		case 0x96: // SUB A, (HL)
			op_sub_a(gb_mmu_read8_cycle(GB_CPU.hl));
			break;

		case 0x97: // SUB A, A
			op_sub_a(GB_CPU.a);
			break;

		case 0x98: // SBC A, B
			op_sbc_a(GB_CPU.b);
			break;

		case 0x99: // SBC A, C
			op_sbc_a(GB_CPU.c);
			break;

		case 0x9a: // SBC A, D
			op_sbc_a(GB_CPU.d);
			break;

		case 0x9b: // SBC A, E
			op_sbc_a(GB_CPU.e);
			break;

		case 0x9c: // SBC A, H
			op_sbc_a(GB_CPU.h);
			break;

		case 0x9d: // SBC A, L
			op_sbc_a(GB_CPU.l);
			break;

		case 0x9e: // SBC A, (HL)
			op_sbc_a(gb_mmu_read8_cycle(GB_CPU.hl));
			break;

		case 0x9f: // SBC A, A
			op_sbc_a(GB_CPU.a);
			break;

		case 0xa0: // AND A, B
			op_and_a(GB_CPU.b);
			break;

		case 0xa1: // AND A, C
			op_and_a(GB_CPU.c);
			break;

		case 0xa2: // AND A, D
			op_and_a(GB_CPU.d);
			break;

		case 0xa3: // AND A, E
			op_and_a(GB_CPU.e);
			break;

		case 0xa4: // AND A, H
			op_and_a(GB_CPU.h);
			break;

		case 0xa5: // AND A, L
			op_and_a(GB_CPU.l);
			break;

		case 0xa6: // AND A, (HL)
			op_and_a(gb_mmu_read8_cycle(GB_CPU.hl));
			break;

		case 0xa7: // AND A, A
			op_and_a(GB_CPU.a);
			break;

		case 0xa8: // XOR A, B
			op_xor_a(GB_CPU.b);
			break;

		case 0xa9: // XOR A, C
			op_xor_a(GB_CPU.c);
			break;

		case 0xaa: // XOR A, D
			op_xor_a(GB_CPU.d);
			break;

		case 0xab: // XOR A, E
			op_xor_a(GB_CPU.e);
			break;

		case 0xac: // XOR A, H
			op_xor_a(GB_CPU.h);
			break;

		case 0xad: // XOR A, L
			op_xor_a(GB_CPU.l);
			break;

		case 0xae: // XOR A, (HL)
			op_xor_a(gb_mmu_read8_cycle(GB_CPU.hl));
			break;

		case 0xaf: // XOR A, A
			op_xor_a(GB_CPU.a);
			break;

		case 0xb0: // OR A, B
			op_or_a(GB_CPU.b);
			break;

		case 0xb1: // OR A, C
			op_or_a(GB_CPU.c);
			break;

		case 0xb2: // OR A, D
			op_or_a(GB_CPU.d);
			break;

		case 0xb3: // OR A, E
			op_or_a(GB_CPU.e);
			break;

		case 0xb4: // OR A, H
			op_or_a(GB_CPU.h);
			break;

		case 0xb5: // OR A, L
			op_or_a(GB_CPU.l);
			break;

		case 0xb6: // OR A, (HL)
			op_or_a(gb_mmu_read8_cycle(GB_CPU.hl));
			break;

		case 0xb7: // OR A, A
			op_or_a(GB_CPU.a);
			break;

		case 0xb8: // CP A, B
			op_cp_a(GB_CPU.b);
			break;

		case 0xb9: // CP A, C
			op_cp_a(GB_CPU.c);
			break;

		case 0xba: // CP A, D
			op_cp_a(GB_CPU.d);
			break;

		case 0xbb: // CP A, E
			op_cp_a(GB_CPU.e);
			break;

		case 0xbc: // CP A, H
			op_cp_a(GB_CPU.h);
			break;

		case 0xbd: // CP A, L
			op_cp_a(GB_CPU.l);
			break;

		case 0xbe: // CP A, (HL)
			op_cp_a(gb_mmu_read8_cycle(GB_CPU.hl));
			break;

		case 0xbf: // CP A, A
			op_cp_a(GB_CPU.a);
			break;
		
		case 0xc0: // RET NZ
			op_ret_cond(!GB_CPU.flagZ);
			break;

		case 0xc1: // POP BC
			GB_CPU.bc = op_pop();
			break;

		case 0xc2: // JP NZ, u16
			op_jp_u16_cond(!GB_CPU.flagZ);
			break;

		case 0xc3: // JP u16
			op_jp_u16();
			break;

		case 0xc4: // CALL NZ, u16
			op_call_u16_cond(!GB_CPU.flagZ);
			break;

		case 0xc5: // PUSH BC
			op_push(GB_CPU.bc);
			break;

		case 0xc6: // ADD A, u8
			op_add_a(fetch8());
			break;

		case 0xc7: // RST 0x00
			op_rst(0x00);
			break;

		case 0xc8: // RET Z
			op_ret_cond(GB_CPU.flagZ);
			break;

		case 0xc9: // RET
			op_ret();
			break;

		case 0xca: // JP Z, u16
			op_jp_u16_cond(GB_CPU.flagZ);
			break;

		case 0xcb: // CB prefix
			switch(fetch8()) {
				case 0x00: // RLC B
					GB_CPU.b = op_rlc(GB_CPU.b);
					break;

				case 0x01: // RLC C
					GB_CPU.c = op_rlc(GB_CPU.c);
					break;

				case 0x02: // RLC D
					GB_CPU.d = op_rlc(GB_CPU.d);
					break;

				case 0x03: // RLC E
					GB_CPU.e = op_rlc(GB_CPU.e);
					break;

				case 0x04: // RLC H
					GB_CPU.h = op_rlc(GB_CPU.h);
					break;

				case 0x05: // RLC L
					GB_CPU.l = op_rlc(GB_CPU.l);
					break;

				case 0x06: // RLC (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_rlc(gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0x07: // RLC A
					GB_CPU.a = op_rlc(GB_CPU.a);
					break;

				case 0x08: // RRC B
					GB_CPU.b = op_rrc(GB_CPU.b);
					break;

				case 0x09: // RRC C
					GB_CPU.c = op_rrc(GB_CPU.c);
					break;

				case 0x0a: // RRC D
					GB_CPU.d = op_rrc(GB_CPU.d);
					break;

				case 0x0b: // RRC E
					GB_CPU.e = op_rrc(GB_CPU.e);
					break;

				case 0x0c: // RRC H
					GB_CPU.h = op_rrc(GB_CPU.h);
					break;

				case 0x0d: // RRC L
					GB_CPU.l = op_rrc(GB_CPU.l);
					break;

				case 0x0e: // RRC (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_rrc(gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0x0f: // RRC A
					GB_CPU.a = op_rrc(GB_CPU.a);
					break;

				case 0x10: // RL B
					GB_CPU.b = op_rl(GB_CPU.b);
					break;

				case 0x11: // RL C
					GB_CPU.c = op_rl(GB_CPU.c);
					break;

				case 0x12: // RL D
					GB_CPU.d = op_rl(GB_CPU.d);
					break;

				case 0x13: // RL E
					GB_CPU.e = op_rl(GB_CPU.e);
					break;

				case 0x14: // RL H
					GB_CPU.h = op_rl(GB_CPU.h);
					break;

				case 0x15: // RL L
					GB_CPU.l = op_rl(GB_CPU.l);
					break;

				case 0x16: // RL (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_rl(gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0x17: // RL A
					GB_CPU.a = op_rl(GB_CPU.a);
					break;

				case 0x18: // RR B
					GB_CPU.b = op_rr(GB_CPU.b);
					break;

				case 0x19: // RR C
					GB_CPU.c = op_rr(GB_CPU.c);
					break;

				case 0x1a: // RR D
					GB_CPU.d = op_rr(GB_CPU.d);
					break;

				case 0x1b: // RR E
					GB_CPU.e = op_rr(GB_CPU.e);
					break;

				case 0x1c: // RR H
					GB_CPU.h = op_rr(GB_CPU.h);
					break;

				case 0x1d: // RR L
					GB_CPU.l = op_rr(GB_CPU.l);
					break;

				case 0x1e: // RR (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_rr(gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0x1f: // RR A
					GB_CPU.a = op_rr(GB_CPU.a);
					break;

				case 0x20: // SLA B
					GB_CPU.b = op_sla(GB_CPU.b);
					break;

				case 0x21: // SLA C
					GB_CPU.c = op_sla(GB_CPU.c);
					break;

				case 0x22: // SLA D
					GB_CPU.d = op_sla(GB_CPU.d);
					break;

				case 0x23: // SLA E
					GB_CPU.e = op_sla(GB_CPU.e);
					break;

				case 0x24: // SLA H
					GB_CPU.h = op_sla(GB_CPU.h);
					break;

				case 0x25: // SLA L
					GB_CPU.l = op_sla(GB_CPU.l);
					break;

				case 0x26: // SLA (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_sla(gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0x27: // SLA A
					GB_CPU.a = op_sla(GB_CPU.a);
					break;

				case 0x28: // SRA B
					GB_CPU.b = op_sra(GB_CPU.b);
					break;

				case 0x29: // SRA C
					GB_CPU.c = op_sra(GB_CPU.c);
					break;

				case 0x2a: // SRA D
					GB_CPU.d = op_sra(GB_CPU.d);
					break;

				case 0x2b: // SRA E
					GB_CPU.e = op_sra(GB_CPU.e);
					break;

				case 0x2c: // SRA H
					GB_CPU.h = op_sra(GB_CPU.h);
					break;

				case 0x2d: // SRA L
					GB_CPU.l = op_sra(GB_CPU.l);
					break;

				case 0x2e: // SRA (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_sra(gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0x2f: // SRA A
					GB_CPU.a = op_sra(GB_CPU.a);
					break;

				case 0x30: // SWAP B
					GB_CPU.b = op_swap(GB_CPU.b);
					break;

				case 0x31: // SWAP C
					GB_CPU.c = op_swap(GB_CPU.c);
					break;

				case 0x32: // SWAP D
					GB_CPU.d = op_swap(GB_CPU.d);
					break;

				case 0x33: // SWAP E
					GB_CPU.e = op_swap(GB_CPU.e);
					break;

				case 0x34: // SWAP H
					GB_CPU.h = op_swap(GB_CPU.h);
					break;

				case 0x35: // SWAP L
					GB_CPU.l = op_swap(GB_CPU.l);
					break;

				case 0x36: // SWAP (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_swap(gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0x37: // SWAP A
					GB_CPU.a = op_swap(GB_CPU.a);
					break;

				case 0x38: // SRL B
					GB_CPU.b = op_srl(GB_CPU.b);
					break;

				case 0x39: // SRL C
					GB_CPU.c = op_srl(GB_CPU.c);
					break;

				case 0x3a: // SRL D
					GB_CPU.d = op_srl(GB_CPU.d);
					break;

				case 0x3b: // SRL E
					GB_CPU.e = op_srl(GB_CPU.e);
					break;

				case 0x3c: // SRL H
					GB_CPU.h = op_srl(GB_CPU.h);
					break;

				case 0x3d: // SRL L
					GB_CPU.l = op_srl(GB_CPU.l);
					break;

				case 0x3e: // SRL (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_srl(gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0x3f: // SRL A
					GB_CPU.a = op_srl(GB_CPU.a);
					break;

				case 0x40: // BIT 0, B
					op_bit(0, GB_CPU.b);
					break;

				case 0x41: // BIT 0, C
					op_bit(0, GB_CPU.c);
					break;

				case 0x42: // BIT 0, D
					op_bit(0, GB_CPU.d);
					break;

				case 0x43: // BIT 0, E
					op_bit(0, GB_CPU.e);
					break;

				case 0x44: // BIT 0, H
					op_bit(0, GB_CPU.h);
					break;

				case 0x45: // BIT 0, L
					op_bit(0, GB_CPU.l);
					break;

				case 0x46: // BIT 0, (HL)
					op_bit(0, gb_mmu_read8_cycle(GB_CPU.hl));
					break;

				case 0x47: // BIT 0, A
					op_bit(0, GB_CPU.a);
					break;

				case 0x48: // BIT 1, B
					op_bit(1, GB_CPU.b);
					break;

				case 0x49: // BIT 1, C
					op_bit(1, GB_CPU.c);
					break;

				case 0x4a: // BIT 1, D
					op_bit(1, GB_CPU.d);
					break;

				case 0x4b: // BIT 1, E
					op_bit(1, GB_CPU.e);
					break;

				case 0x4c: // BIT 1, H
					op_bit(1, GB_CPU.h);
					break;

				case 0x4d: // BIT 1, L
					op_bit(1, GB_CPU.l);
					break;

				case 0x4e: // BIT 1, (HL)
					op_bit(1, gb_mmu_read8_cycle(GB_CPU.hl));
					break;

				case 0x4f: // BIT 1, A
					op_bit(1, GB_CPU.a);
					break;

				case 0x50: // BIT 2, B
					op_bit(2, GB_CPU.b);
					break;

				case 0x51: // BIT 2, C
					op_bit(2, GB_CPU.c);
					break;

				case 0x52: // BIT 2, D
					op_bit(2, GB_CPU.d);
					break;

				case 0x53: // BIT 2, E
					op_bit(2, GB_CPU.e);
					break;

				case 0x54: // BIT 2, H
					op_bit(2, GB_CPU.h);
					break;

				case 0x55: // BIT 2, L
					op_bit(2, GB_CPU.l);
					break;

				case 0x56: // BIT 2, (HL)
					op_bit(2, gb_mmu_read8_cycle(GB_CPU.hl));
					break;

				case 0x57: // BIT 2, A
					op_bit(2, GB_CPU.a);
					break;

				case 0x58: // BIT 3, B
					op_bit(3, GB_CPU.b);
					break;

				case 0x59: // BIT 3, C
					op_bit(3, GB_CPU.c);
					break;

				case 0x5a: // BIT 3, D
					op_bit(3, GB_CPU.d);
					break;

				case 0x5b: // BIT 3, E
					op_bit(3, GB_CPU.e);
					break;

				case 0x5c: // BIT 3, H
					op_bit(3, GB_CPU.h);
					break;

				case 0x5d: // BIT 3, L
					op_bit(3, GB_CPU.l);
					break;

				case 0x5e: // BIT 3, (HL)
					op_bit(3, gb_mmu_read8_cycle(GB_CPU.hl));
					break;

				case 0x5f: // BIT 3, A
					op_bit(3, GB_CPU.a);
					break;

				case 0x60: // BIT 4, B
					op_bit(4, GB_CPU.b);
					break;

				case 0x61: // BIT 4, C
					op_bit(4, GB_CPU.c);
					break;

				case 0x62: // BIT 4, D
					op_bit(4, GB_CPU.d);
					break;

				case 0x63: // BIT 4, E
					op_bit(4, GB_CPU.e);
					break;

				case 0x64: // BIT 4, H
					op_bit(4, GB_CPU.h);
					break;

				case 0x65: // BIT 4, L
					op_bit(4, GB_CPU.l);
					break;

				case 0x66: // BIT 4, (HL)
					op_bit(4, gb_mmu_read8_cycle(GB_CPU.hl));
					break;

				case 0x67: // BIT 4, A
					op_bit(4, GB_CPU.a);
					break;

				case 0x68: // BIT 5, B
					op_bit(5, GB_CPU.b);
					break;

				case 0x69: // BIT 5, C
					op_bit(5, GB_CPU.c);
					break;

				case 0x6a: // BIT 5, D
					op_bit(5, GB_CPU.d);
					break;

				case 0x6b: // BIT 5, E
					op_bit(5, GB_CPU.e);
					break;

				case 0x6c: // BIT 5, H
					op_bit(5, GB_CPU.h);
					break;

				case 0x6d: // BIT 5, L
					op_bit(5, GB_CPU.l);
					break;

				case 0x6e: // BIT 5, (HL)
					op_bit(5, gb_mmu_read8_cycle(GB_CPU.hl));
					break;

				case 0x6f: // BIT 5, A
					op_bit(5, GB_CPU.a);
					break;

				case 0x70: // BIT 6, B
					op_bit(6, GB_CPU.b);
					break;

				case 0x71: // BIT 6, C
					op_bit(6, GB_CPU.c);
					break;

				case 0x72: // BIT 6, D
					op_bit(6, GB_CPU.d);
					break;

				case 0x73: // BIT 6, E
					op_bit(6, GB_CPU.e);
					break;

				case 0x74: // BIT 6, H
					op_bit(6, GB_CPU.h);
					break;

				case 0x75: // BIT 6, L
					op_bit(6, GB_CPU.l);
					break;

				case 0x76: // BIT 6, (HL)
					op_bit(6, gb_mmu_read8_cycle(GB_CPU.hl));
					break;

				case 0x77: // BIT 6, A
					op_bit(6, GB_CPU.a);
					break;

				case 0x78: // BIT 7, B
					op_bit(7, GB_CPU.b);
					break;

				case 0x79: // BIT 7, C
					op_bit(7, GB_CPU.c);
					break;

				case 0x7a: // BIT 7, D
					op_bit(7, GB_CPU.d);
					break;

				case 0x7b: // BIT 7, E
					op_bit(7, GB_CPU.e);
					break;

				case 0x7c: // BIT 7, H
					op_bit(7, GB_CPU.h);
					break;

				case 0x7d: // BIT 7, L
					op_bit(7, GB_CPU.l);
					break;

				case 0x7e: // BIT 7, (HL)
					op_bit(7, gb_mmu_read8_cycle(GB_CPU.hl));
					break;

				case 0x7f: // BIT 7, A
					op_bit(7, GB_CPU.a);
					break;

				case 0x80: // RES 0, B
					GB_CPU.b = op_res(0, GB_CPU.b);
					break;

				case 0x81: // RES 0, C
					GB_CPU.c = op_res(0, GB_CPU.c);
					break;

				case 0x82: // RES 0, D
					GB_CPU.d = op_res(0, GB_CPU.d);
					break;

				case 0x83: // RES 0, E
					GB_CPU.e = op_res(0, GB_CPU.e);
					break;

				case 0x84: // RES 0, H
					GB_CPU.h = op_res(0, GB_CPU.h);
					break;

				case 0x85: // RES 0, L
					GB_CPU.l = op_res(0, GB_CPU.l);
					break;

				case 0x86: // RES 0, (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_res(0, gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0x87: // RES 0, A
					GB_CPU.a = op_res(0, GB_CPU.a);
					break;

				case 0x88: // RES 1, B
					GB_CPU.b = op_res(1, GB_CPU.b);
					break;

				case 0x89: // RES 1, C
					GB_CPU.c = op_res(1, GB_CPU.c);
					break;

				case 0x8a: // RES 1, D
					GB_CPU.d = op_res(1, GB_CPU.d);
					break;

				case 0x8b: // RES 1, E
					GB_CPU.e = op_res(1, GB_CPU.e);
					break;

				case 0x8c: // RES 1, H
					GB_CPU.h = op_res(1, GB_CPU.h);
					break;

				case 0x8d: // RES 1, L
					GB_CPU.l = op_res(1, GB_CPU.l);
					break;

				case 0x8e: // RES 1, (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_res(1, gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0x8f: // RES 1, A
					GB_CPU.a = op_res(1, GB_CPU.a);
					break;

				case 0x90: // RES 2, B
					GB_CPU.b = op_res(2, GB_CPU.b);
					break;

				case 0x91: // RES 2, C
					GB_CPU.c = op_res(2, GB_CPU.c);
					break;

				case 0x92: // RES 2, D
					GB_CPU.d = op_res(2, GB_CPU.d);
					break;

				case 0x93: // RES 2, E
					GB_CPU.e = op_res(2, GB_CPU.e);
					break;

				case 0x94: // RES 2, H
					GB_CPU.h = op_res(2, GB_CPU.h);
					break;

				case 0x95: // RES 2, L
					GB_CPU.l = op_res(2, GB_CPU.l);
					break;

				case 0x96: // RES 2, (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_res(2, gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0x97: // RES 2, A
					GB_CPU.a = op_res(2, GB_CPU.a);
					break;

				case 0x98: // RES 3, B
					GB_CPU.b = op_res(3, GB_CPU.b);
					break;

				case 0x99: // RES 3, C
					GB_CPU.c = op_res(3, GB_CPU.c);
					break;

				case 0x9a: // RES 3, D
					GB_CPU.d = op_res(3, GB_CPU.d);
					break;

				case 0x9b: // RES 3, E
					GB_CPU.e = op_res(3, GB_CPU.e);
					break;

				case 0x9c: // RES 3, H
					GB_CPU.h = op_res(3, GB_CPU.h);
					break;

				case 0x9d: // RES 3, L
					GB_CPU.l = op_res(3, GB_CPU.l);
					break;

				case 0x9e: // RES 3, (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_res(3, gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0x9f: // RES 3, A
					GB_CPU.a = op_res(3, GB_CPU.a);
					break;

				case 0xa0: // RES 4, B
					GB_CPU.b = op_res(4, GB_CPU.b);
					break;

				case 0xa1: // RES 4, C
					GB_CPU.c = op_res(4, GB_CPU.c);
					break;

				case 0xa2: // RES 4, D
					GB_CPU.d = op_res(4, GB_CPU.d);
					break;

				case 0xa3: // RES 4, E
					GB_CPU.e = op_res(4, GB_CPU.e);
					break;

				case 0xa4: // RES 4, H
					GB_CPU.h = op_res(4, GB_CPU.h);
					break;

				case 0xa5: // RES 4, L
					GB_CPU.l = op_res(4, GB_CPU.l);
					break;

				case 0xa6: // RES 4, (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_res(4, gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0xa7: // RES 4, A
					GB_CPU.a = op_res(4, GB_CPU.a);
					break;

				case 0xa8: // RES 5, B
					GB_CPU.b = op_res(5, GB_CPU.b);
					break;

				case 0xa9: // RES 5, C
					GB_CPU.c = op_res(5, GB_CPU.c);
					break;

				case 0xaa: // RES 5, D
					GB_CPU.d = op_res(5, GB_CPU.d);
					break;

				case 0xab: // RES 5, E
					GB_CPU.e = op_res(5, GB_CPU.e);
					break;

				case 0xac: // RES 5, H
					GB_CPU.h = op_res(5, GB_CPU.h);
					break;

				case 0xad: // RES 5, L
					GB_CPU.l = op_res(5, GB_CPU.l);
					break;

				case 0xae: // RES 5, (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_res(5, gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0xaf: // RES 5, A
					GB_CPU.a = op_res(5, GB_CPU.a);
					break;

				case 0xb0: // RES 6, B
					GB_CPU.b = op_res(6, GB_CPU.b);
					break;

				case 0xb1: // RES 6, C
					GB_CPU.c = op_res(6, GB_CPU.c);
					break;

				case 0xb2: // RES 6, D
					GB_CPU.d = op_res(6, GB_CPU.d);
					break;

				case 0xb3: // RES 6, E
					GB_CPU.e = op_res(6, GB_CPU.e);
					break;

				case 0xb4: // RES 6, H
					GB_CPU.h = op_res(6, GB_CPU.h);
					break;

				case 0xb5: // RES 6, L
					GB_CPU.l = op_res(6, GB_CPU.l);
					break;

				case 0xb6: // RES 6, (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_res(6, gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0xb7: // RES 6, A
					GB_CPU.a = op_res(6, GB_CPU.a);
					break;

				case 0xb8: // RES 7, B
					GB_CPU.b = op_res(7, GB_CPU.b);
					break;

				case 0xb9: // RES 7, C
					GB_CPU.c = op_res(7, GB_CPU.c);
					break;

				case 0xba: // RES 7, D
					GB_CPU.d = op_res(7, GB_CPU.d);
					break;

				case 0xbb: // RES 7, E
					GB_CPU.e = op_res(7, GB_CPU.e);
					break;

				case 0xbc: // RES 7, H
					GB_CPU.h = op_res(7, GB_CPU.h);
					break;

				case 0xbd: // RES 7, L
					GB_CPU.l = op_res(7, GB_CPU.l);
					break;

				case 0xbe: // RES 7, (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_res(7, gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0xbf: // RES 7, A
					GB_CPU.a = op_res(7, GB_CPU.a);
					break;

				case 0xc0: // SET 0, B
					GB_CPU.b = op_set(0, GB_CPU.b);
					break;

				case 0xc1: // SET 0, C
					GB_CPU.c = op_set(0, GB_CPU.c);
					break;

				case 0xc2: // SET 0, D
					GB_CPU.d = op_set(0, GB_CPU.d);
					break;

				case 0xc3: // SET 0, E
					GB_CPU.e = op_set(0, GB_CPU.e);
					break;

				case 0xc4: // SET 0, H
					GB_CPU.h = op_set(0, GB_CPU.h);
					break;

				case 0xc5: // SET 0, L
					GB_CPU.l = op_set(0, GB_CPU.l);
					break;

				case 0xc6: // SET 0, (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_set(0, gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0xc7: // SET 0, A
					GB_CPU.a = op_set(0, GB_CPU.a);
					break;

				case 0xc8: // SET 1, B
					GB_CPU.b = op_set(1, GB_CPU.b);
					break;

				case 0xc9: // SET 1, C
					GB_CPU.c = op_set(1, GB_CPU.c);
					break;

				case 0xca: // SET 1, D
					GB_CPU.d = op_set(1, GB_CPU.d);
					break;

				case 0xcb: // SET 1, E
					GB_CPU.e = op_set(1, GB_CPU.e);
					break;

				case 0xcc: // SET 1, H
					GB_CPU.h = op_set(1, GB_CPU.h);
					break;

				case 0xcd: // SET 1, L
					GB_CPU.l = op_set(1, GB_CPU.l);
					break;

				case 0xce: // SET 1, (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_set(1, gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0xcf: // SET 1, A
					GB_CPU.a = op_set(1, GB_CPU.a);
					break;

				case 0xd0: // SET 2, B
					GB_CPU.b = op_set(2, GB_CPU.b);
					break;

				case 0xd1: // SET 2, C
					GB_CPU.c = op_set(2, GB_CPU.c);
					break;

				case 0xd2: // SET 2, D
					GB_CPU.d = op_set(2, GB_CPU.d);
					break;

				case 0xd3: // SET 2, E
					GB_CPU.e = op_set(2, GB_CPU.e);
					break;

				case 0xd4: // SET 2, H
					GB_CPU.h = op_set(2, GB_CPU.h);
					break;

				case 0xd5: // SET 2, L
					GB_CPU.l = op_set(2, GB_CPU.l);
					break;

				case 0xd6: // SET 2, (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_set(2, gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0xd7: // SET 2, A
					GB_CPU.a = op_set(2, GB_CPU.a);
					break;

				case 0xd8: // SET 3, B
					GB_CPU.b = op_set(3, GB_CPU.b);
					break;

				case 0xd9: // SET 3, C
					GB_CPU.c = op_set(3, GB_CPU.c);
					break;

				case 0xda: // SET 3, D
					GB_CPU.d = op_set(3, GB_CPU.d);
					break;

				case 0xdb: // SET 3, E
					GB_CPU.e = op_set(3, GB_CPU.e);
					break;

				case 0xdc: // SET 3, H
					GB_CPU.h = op_set(3, GB_CPU.h);
					break;

				case 0xdd: // SET 3, L
					GB_CPU.l = op_set(3, GB_CPU.l);
					break;

				case 0xde: // SET 3, (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_set(3, gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0xdf: // SET 3, A
					GB_CPU.a = op_set(3, GB_CPU.a);
					break;

				case 0xe0: // SET 4, B
					GB_CPU.b = op_set(4, GB_CPU.b);
					break;

				case 0xe1: // SET 4, C
					GB_CPU.c = op_set(4, GB_CPU.c);
					break;

				case 0xe2: // SET 4, D
					GB_CPU.d = op_set(4, GB_CPU.d);
					break;

				case 0xe3: // SET 4, E
					GB_CPU.e = op_set(4, GB_CPU.e);
					break;

				case 0xe4: // SET 4, H
					GB_CPU.h = op_set(4, GB_CPU.h);
					break;

				case 0xe5: // SET 4, L
					GB_CPU.l = op_set(4, GB_CPU.l);
					break;

				case 0xe6: // SET 4, (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_set(4, gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0xe7: // SET 4, A
					GB_CPU.a = op_set(4, GB_CPU.a);
					break;

				case 0xe8: // SET 5, B
					GB_CPU.b = op_set(5, GB_CPU.b);
					break;

				case 0xe9: // SET 5, C
					GB_CPU.c = op_set(5, GB_CPU.c);
					break;

				case 0xea: // SET 5, D
					GB_CPU.d = op_set(5, GB_CPU.d);
					break;

				case 0xeb: // SET 5, E
					GB_CPU.e = op_set(5, GB_CPU.e);
					break;

				case 0xec: // SET 5, H
					GB_CPU.h = op_set(5, GB_CPU.h);
					break;

				case 0xed: // SET 5, L
					GB_CPU.l = op_set(5, GB_CPU.l);
					break;

				case 0xee: // SET 5, (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_set(5, gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0xef: // SET 5, A
					GB_CPU.a = op_set(5, GB_CPU.a);
					break;

				case 0xf0: // SET 6, B
					GB_CPU.b = op_set(6, GB_CPU.b);
					break;

				case 0xf1: // SET 6, C
					GB_CPU.c = op_set(6, GB_CPU.c);
					break;

				case 0xf2: // SET 6, D
					GB_CPU.d = op_set(6, GB_CPU.d);
					break;

				case 0xf3: // SET 6, E
					GB_CPU.e = op_set(6, GB_CPU.e);
					break;

				case 0xf4: // SET 6, H
					GB_CPU.h = op_set(6, GB_CPU.h);
					break;

				case 0xf5: // SET 6, L
					GB_CPU.l = op_set(6, GB_CPU.l);
					break;

				case 0xf6: // SET 6, (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_set(6, gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0xf7: // SET 6, A
					GB_CPU.a = op_set(6, GB_CPU.a);
					break;

				case 0xf8: // SET 7, B
					GB_CPU.b = op_set(7, GB_CPU.b);
					break;

				case 0xf9: // SET 7, C
					GB_CPU.c = op_set(7, GB_CPU.c);
					break;

				case 0xfa: // SET 7, D
					GB_CPU.d = op_set(7, GB_CPU.d);
					break;

				case 0xfb: // SET 7, E
					GB_CPU.e = op_set(7, GB_CPU.e);
					break;

				case 0xfc: // SET 7, H
					GB_CPU.h = op_set(7, GB_CPU.h);
					break;

				case 0xfd: // SET 7, L
					GB_CPU.l = op_set(7, GB_CPU.l);
					break;

				case 0xfe: // SET 7, (HL)
					gb_mmu_write8_cycle(GB_CPU.hl, op_set(7, gb_mmu_read8_cycle(GB_CPU.hl)));
					break;

				case 0xff: // SET 7, A
					GB_CPU.a = op_set(7, GB_CPU.a);
					break;
			}
			break;

		case 0xcc: // CALL Z, u16
			op_call_u16_cond(GB_CPU.flagZ);
			break;

		case 0xcd: // CALL u16
			op_call_u16();
			break;

		case 0xce: // ADC A, u8
			op_adc_a(fetch8());
			break;

		case 0xcf: // RST 0x08
			op_rst(0x08);
			break;

		case 0xd0: // RET NC
			op_ret_cond(!GB_CPU.flagC);
			break;

		case 0xd1: // POP DE
			GB_CPU.de = op_pop();
			break;

		case 0xd2: // JP NC, u16
			op_jp_u16_cond(!GB_CPU.flagC);
			break;

		case 0xd4: // CALL NC, u16
			op_call_u16_cond(!GB_CPU.flagC);
			break;

		case 0xd5: // PUSH DE
			op_push(GB_CPU.de);
			break;

		case 0xd6: // SUB A, u8
			op_sub_a(fetch8());
			break;

		case 0xd7: // RST 0x10
			op_rst(0x10);
			break;

		case 0xd8: // RET C
			op_ret_cond(GB_CPU.flagC);
			break;

		case 0xd9: // RETI
			op_reti();
			break;

		case 0xda: // JP C, u16
			op_jp_u16_cond(GB_CPU.flagC);
			break;

		case 0xdc: // CALL C, u16
			op_call_u16_cond(GB_CPU.flagC);
			break;

		case 0xde: // SBC A, u8
			op_sbc_a(fetch8());
			break;

		case 0xdf: // RST 0x18
			op_rst(0x18);
			break;

		case 0xe0: // LD (0xff00 + u8), A
			gb_mmu_write8_cycle(0xff00 + fetch8(), GB_CPU.a);
			break;

		case 0xe1: // POP HL
			GB_CPU.hl = op_pop();
			break;

		case 0xe2: // LD (0xff00 + c), A
			gb_mmu_write8_cycle(0xff00 + GB_CPU.c, GB_CPU.a);
			break;

		case 0xe5: // PUSH HL
			op_push(GB_CPU.hl);
			break;

		case 0xe6: // AND A, u8
			op_and_a(fetch8());
			break;

		case 0xe7: // RST 0x20
			op_rst(0x20);
			break;

		case 0xe8: // ADD SP, i8
			op_add_sp(fetch8());
			break;

		case 0xe9: // JP HL
			GB_CPU.pc = GB_CPU.hl;
			break;

		case 0xea: // LD (u16), A
			gb_mmu_write8_cycle(fetch16(), GB_CPU.a);
			break;

		case 0xee: // XOR A, u8
			op_xor_a(fetch8());
			break;

		case 0xef: // RST 0x28
			op_rst(0x28);
			break;

		case 0xf0: // LD A, (0xff00 + u8)
			GB_CPU.a = gb_mmu_read8_cycle(0xff00 + fetch8());
			break;

		case 0xf1: // POP AF
			op_pop_af();
			break;

		case 0xf2: // LD A, (0xff00 + C)
			GB_CPU.a = gb_mmu_read8_cycle(0xff00 + GB_CPU.c);
			break;

		case 0xf3: // DI
			op_di();
			break;

		case 0xf5: // PUSH AF
			op_push_af();
			break;

		case 0xf6: // OR A, u8
			op_or_a(fetch8());
			break;

		case 0xf7: // RST 0x30
			op_rst(0x30);
			break;

		case 0xf8: // LD HL, SP + i8
			op_ld_hl_sp_i8();
			break;

		case 0xf9: // LD SP, HL
			GB_CPU.sp = GB_CPU.hl;
			gb_cycleNoCPU();
			break;

		case 0xfa: // LD A, (u16)
			GB_CPU.a = gb_mmu_read8_cycle(fetch16());
			break;

		case 0xfb: // EI
			GB_CPU.ime = false;
			GB_CPU.ime_enableNextCycle = true;
			break;

		case 0xfe: // CP A, u8
			op_cp_a(fetch8());
			break;

		case 0xff: // RST 0x38
			op_rst(0x38);
			break;

		default:
			gbemu_error("Unknown opcode");
			break;
	}
}

uint8_t gb_cpu_read_ff0f() {
	return GB_CPU.ioreg_if;
}

void gb_cpu_write_ff0f(uint8_t value) {
	GB_CPU.ioreg_if = value | 0xe0;
}
