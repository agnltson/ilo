#include <cstdint>
#include <fstream>
#include <iostream>

#include "machine.hpp"

#define MAGIC_CODE 0xD12EA2E2

#define OPCODE_SIZE 6
#define OPERAND_SIZE 5

// if your compiler doesn't use two's complement it won't work
inline int32_t i26_to_i32(int32_t val) {
    if ((val & 0x2000000) == 0) {
        return val;
    }
    // if negative on 26 bits then transfer the sign bit
    return (1 << 31) & (val & 0x1FFFFFF);
}

#define GET_OPCODE(inst) (inst & 0x3F)
#define GET_ARGA(inst) ((inst >> OPCODE_SIZE) & 0x1F)
#define GET_ARGB(inst) ((inst >> (OPCODE_SIZE + OPERAND_SIZE)) & 0x1F)
#define GET_IMMEDIATE(inst) (i26_to_i32((inst >> OPCODE_SIZE) & 0x3FFFFFF))

Machine::Machine() : _stack_frame(), _memory(0), _running(false) {
}

Machine::~Machine() {
}

uint32_t read_u32_le(std::ifstream& file) {
    uint8_t bytes[4];
    file.read(reinterpret_cast<char*>(bytes), 4);
    return static_cast<uint32_t>(bytes[0]) | static_cast<uint32_t>(bytes[1]) << 8 |
           static_cast<uint32_t>(bytes[2]) << 16 | static_cast<uint32_t>(bytes[3]) << 24;
}

int check_validity(std::ifstream& file) {
    if (file.peek() != EOF) {
        uint32_t magic_code = read_u32_le(file);
        if (magic_code == MAGIC_CODE) {
            return 0;
        }
    }
    return 1;
}

std::expected<void, IloError> Machine::load_program(std::string binary_path) {
    std::ifstream file(binary_path, std::ios::binary);
    if (!file) {
        return std::unexpected(IloError::NoInputFile);
    }

    size_t i = 0;
    int err = check_validity(file);
    if (err) {
        return std::unexpected(IloError::InvalidBinaryMetadata);
    }
    while (file.peek() != EOF) {
        uint32_t value = read_u32_le(file);
        if (file) {
            this->mem_write32(i, value);
            i += 4;
        }
    }
#ifdef DBG
    this->dump_mem("mem.html");
#endif // DBG
    return {};
}

void Machine::run() {
    this->_running = true;
    this->_stack_frame.push_back(new Frame(0));
#ifdef DBG
        this->debug();
#endif // DBG

    while (this->_running) {
        std::expected<void, IloError> res = this->process_instruction(this->mem_read32(this->_pc));
        if (!res) {
            log(res.error());
            return;
        }
        this->display();
#ifdef DBG
        this->debug();
#endif // DBG
    }
}

void Machine::display() {
    char c = (this->mem_read8(DISPLAY_DATA_ADDR));
    if (c) {
        std::cout << c;
        this->mem_write8(DISPLAY_DATA_ADDR, 0);
    }
}

int32_t Machine::frame_pop() {
    Frame* top_frame = this->_stack_frame.back();
    this->_stack_frame.pop_back();
    int32_t retval = top_frame->belt->get(0);
    return retval;
}

void Machine::frame_add() {
    Frame* new_frame = new Frame(this->_pc + 1, this->_stack_frame.back()->belt);
    this->_stack_frame.push_back(new_frame);
}

void Machine::inc_pc() {
    this->_pc += 4;
}

void Machine::push_belt(int32_t val) {
    this->_stack_frame.back()->belt->push(val);
}

int32_t Machine::get_belt(uint8_t idx) {
    return this->_stack_frame.back()->belt->get(idx);
}

void Machine::put_scratchpad(uint8_t idx, int32_t val) {
    this->_stack_frame.back()->scratchpad->put(idx, val);
}
int32_t Machine::get_scratchpad(uint8_t idx) {
    return this->_stack_frame.back()->scratchpad->get(idx);
}

#ifdef DBG

void Machine::debug() const {
    std::cout << "#### DEBUG ###" << std::endl;
    size_t count = 0;
    for (Frame* frame : this->_stack_frame) {
        std::cout << "--- Frame " << count << " ---" << std::endl;
        frame->debug();
        count++;
    }
}
#endif // DBG

void Machine::dump_mem(std::string path) const {
    FILE* f = fopen(path.c_str(), "w");
    if (!f) return;

    const int COLS = 16;

    fprintf(f, "<!DOCTYPE html><html><head><meta charset='utf-8'>"
               "<title>Memory Dump</title><style>"
               "body{font-family:monospace;background:#1e1e1e;color:#d4d4d4;padding:1rem}"
               "h1{color:#9cdcfe;font-size:1rem;margin-bottom:1rem}"
               "table{border-collapse:collapse;font-size:0.85rem}"
               "th{color:#569cd6;text-align:left;padding:2px 8px;border-bottom:1px solid #333}"
               "td{padding:2px 8px;white-space:pre}"
               "tr:hover td{background:#2d2d2d}"
               ".addr{color:#569cd6}"
               ".hex span:hover{background:#264f78;cursor:default}"
               ".nul{color:#444}"
               ".ascii{color:#ce9178;border-left:1px solid #333;padding-left:12px}"
               "</style></head><body>"
               "<h1>Memory Dump &mdash; %zu bytes</h1>"
               "<table><thead><tr>"
               "<th>Address</th><th colspan='16'>Hex</th><th>ASCII</th>"
               "</tr></thead><tbody>\n",
               sizeof(_memory));

    for (size_t i = 0; i < sizeof(_memory); i += COLS) {
        fprintf(f, "<tr><td class='addr'>%08zX</td><td class='hex'>", i);

        for (int j = 0; j < COLS; j++) {
            size_t idx = i + j;
            if (idx < sizeof(_memory)) {
                uint8_t b = _memory[idx];
                const char* cls = (b == 0) ? "nul" : "";
                fprintf(f, "<span class='%s'>%02X </span>", cls, b);
            } else {
                fprintf(f, "   ");
            }
        }

        fprintf(f, "</td><td class='ascii'>");
        for (int j = 0; j < COLS; j++) {
            size_t idx = i + j;
            if (idx < sizeof(_memory)) {
                uint8_t b = _memory[idx];
                fputc((b >= 0x20 && b < 0x7F) ? (char)b : '.', f);
            }
        }

        fprintf(f, "</td></tr>\n");
    }

    fprintf(f, "</tbody></table></body></html>\n");
    fclose(f);
}

std::expected<void, IloError> Machine::process_instruction(uint32_t instruction) {
    switch (GET_OPCODE(instruction)) {
    case 0b000001: // add
        this->add(GET_ARGA(instruction), GET_ARGB(instruction));
        this->inc_pc();
        break;
    case 0b000011: // sub
        this->sub(GET_ARGA(instruction), GET_ARGB(instruction));
        this->inc_pc();
        break;
    case 0b000101: // mul
        this->mul(GET_ARGA(instruction), GET_ARGB(instruction));
        this->inc_pc();
        break;
    case 0b000111: // div
        this->div(GET_ARGA(instruction), GET_ARGB(instruction));
        this->inc_pc();
        break;
    case 0b001001: // and
        this->andd(GET_ARGA(instruction), GET_ARGB(instruction));
        this->inc_pc();
        break;
    case 0b001011: // or
        this->orr(GET_ARGA(instruction), GET_ARGB(instruction));
        this->inc_pc();
        break;
    case 0b001101: // xor
        this->xorr(GET_ARGA(instruction), GET_ARGB(instruction));
        this->inc_pc();
        break;
    case 0b001111: // sll
        this->sll(GET_ARGA(instruction), GET_ARGB(instruction));
        this->inc_pc();
        break;
    case 0b010001: // srl
        this->srl(GET_ARGA(instruction), GET_ARGB(instruction));
        this->inc_pc();
        break;
    case 0b010011: // sra
        this->sra(GET_ARGA(instruction), GET_ARGB(instruction));
        this->inc_pc();
        break;
    case 0b010101: // eq
        this->eq(GET_ARGA(instruction), GET_ARGB(instruction));
        this->inc_pc();
        break;
    case 0b010111: // lt
        this->lt(GET_ARGA(instruction), GET_ARGB(instruction));
        this->inc_pc();
        break;
    case 0b011001: // put
        this->put(GET_ARGA(instruction), GET_ARGB(instruction));
        this->inc_pc();
        break;
    case 0b011011: // pick
        this->pick(GET_ARGA(instruction));
        this->inc_pc();
        break;
    case 0b100001: // load8
        this->load8(GET_ARGA(instruction));
        this->inc_pc();
        break;
    case 0b100011: // store8
        this->store8(GET_ARGA(instruction), GET_ARGB(instruction));
        this->inc_pc();
        break;
    case 0b100101: // load16
        this->load16(GET_ARGA(instruction));
        this->inc_pc();
        break;
    case 0b100111: // store16
        this->store16(GET_ARGA(instruction), GET_ARGB(instruction));
        this->inc_pc();
        break;
    case 0b101001: // load32
        this->load32(GET_ARGA(instruction));
        this->inc_pc();
        break;
    case 0b101011: // store32
        this->store32(GET_ARGA(instruction), GET_ARGB(instruction));
        this->inc_pc();
        break;
    case 0b000010: // immh
        this->immh(GET_IMMEDIATE(instruction));
        this->inc_pc();
        break;
    case 0b000100: // imml
        this->imml(GET_IMMEDIATE(instruction));
        this->inc_pc();
        break;
    case 0b000110: // jmp
        this->jmp(GET_IMMEDIATE(instruction));
        break;
    case 0b001000: // jmpif
        this->jmpif(GET_IMMEDIATE(instruction));
        break;
    case 0b001010: // call
        this->call(GET_IMMEDIATE(instruction));
        break;
    case 0b001100: // ret
        this->ret();
        break;
    case 0b111110: // halt
        this->halt();
        break;
    case 0b000000: // nop
        break;
    default:
        this->_running = false;
        return std::unexpected(IloError::InvalidOpcode);
    }
    return {};
}

inline uint32_t Machine::mem_read8(uint32_t addr) const {
    return this->_memory[addr];
}
inline void Machine::mem_write8(uint32_t addr, uint32_t val) {
    this->_memory[addr] = val & 0xFF;
}

inline uint32_t Machine::mem_read16(uint32_t addr) const {
    return this->_memory[addr]
        | (this->_memory[addr+1]<<8);
}
inline void Machine::mem_write16(uint32_t addr, uint32_t val) {
    this->_memory[addr] = val & 0xFF;
    this->_memory[addr+1] = (val>>8) & 0xFF;
}

inline uint32_t Machine::mem_read32(uint32_t addr) const {
    return this->_memory[addr]
        | (this->_memory[addr+1]<<8)
        | (this->_memory[addr+2]<<16)
        | (this->_memory[addr+3]<<24);
}
inline void Machine::mem_write32(uint32_t addr, uint32_t val) {
    this->_memory[addr] = val & 0xFF;
    this->_memory[addr+1] = (val>>8) & 0xFF;
    this->_memory[addr+2] = (val>>16) & 0xFF;
    this->_memory[addr+3] = (val>>24) & 0xFF;
}

#define PUSH_BINARY(op) (this->push_belt(this->get_belt(idxa) op this->get_belt(idxb)))

void Machine::add(uint8_t idxa, uint8_t idxb) {
    PUSH_BINARY(+);
}

void Machine::sub(uint8_t idxa, uint8_t idxb) {
    PUSH_BINARY(-);
}

void Machine::mul(uint8_t idxa, uint8_t idxb) {
    PUSH_BINARY(*);
}

void Machine::div(uint8_t idxa, uint8_t idxb) {
    PUSH_BINARY(/);
}

void Machine::andd(uint8_t idxa, uint8_t idxb) {
    PUSH_BINARY(&);
}

void Machine::orr(uint8_t idxa, uint8_t idxb) {
    PUSH_BINARY(|);
}

void Machine::xorr(uint8_t idxa, uint8_t idxb) {
    PUSH_BINARY(^);
}

void Machine::sll(uint8_t idxa, uint8_t idxb) {
    this->push_belt(this->get_belt(idxa) << (this->get_belt(idxb) & 0x1F));
}

void Machine::srl(uint8_t idxa, uint8_t idxb) {
    uint32_t a = static_cast<uint32_t>(this->get_belt(idxa));
    uint32_t sh = static_cast<uint32_t>(this->get_belt(idxb)) & 0x1F;
    this->push_belt(static_cast<int32_t>(a >> sh));
}

void Machine::sra(uint8_t idxa, uint8_t idxb) {
    int32_t a = this->get_belt(idxa);
    uint32_t sh = static_cast<uint32_t>(this->get_belt(idxb)) & 0x1F;
    this->push_belt(a >> sh);
}

void Machine::eq(uint8_t idxa, uint8_t idxb) {
    this->push_belt(this->get_belt(idxa) == this->get_belt(idxb) ? 1 : 0);
}

void Machine::lt(uint8_t idxa, uint8_t idxb) {
    this->push_belt(this->get_belt(idxa) < this->get_belt(idxb) ? 1 : 0);
}

void Machine::load8(uint8_t idxa) {
    this->push_belt(static_cast<int32_t>(mem_read8(this->get_belt(idxa))));
}

void Machine::store8(uint8_t idxa, uint8_t idxb) {
    mem_write8(this->get_belt(idxa), static_cast<uint32_t>(this->get_belt(idxb)));
}

void Machine::load16(uint8_t idxa) {
    this->push_belt(static_cast<int32_t>(mem_read16(this->get_belt(idxa))));
}

void Machine::store16(uint8_t idxa, uint8_t idxb) {
    mem_write16(this->get_belt(idxa), static_cast<uint32_t>(this->get_belt(idxb)));
}

void Machine::load32(uint8_t idxa) {
    this->push_belt(static_cast<int32_t>(mem_read32(this->get_belt(idxa))));
}

void Machine::store32(uint8_t idxa, uint8_t idxb) {
    mem_write32(this->get_belt(idxa), static_cast<uint32_t>(this->get_belt(idxb)));
}

void Machine::put(uint8_t idxa, uint8_t idxb) {
    this->put_scratchpad(idxa, this->get_belt(idxb));
}

void Machine::pick(uint8_t idxa) {
    this->push_belt(this->get_scratchpad(idxa));
}

void Machine::immh(int32_t imm) {
    this->push_belt(imm << 12);
}

void Machine::imml(int32_t imm) {
    this->push_belt(imm & 0xFFF);
}

void Machine::jmp(int32_t imm) {
    this->_pc = this->_pc + 1 + imm;
}

void Machine::jmpif(int32_t imm) {
    if (this->get_belt(0) == 1) {
        this->_pc = this->_pc + 1 + imm;
    } else {
        this->_pc += 4;
    }
}

void Machine::call(int32_t imm) {
    this->frame_add();
    this->_pc = this->_pc + 1 + imm;
}

void Machine::ret() {
    this->_pc = this->_stack_frame.back()->return_pc;
    this->push_belt(this->frame_pop());
}

void Machine::halt() {
    this->_running = false;
}
