#include <fstream>
#include <cstdint>
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

Machine::Machine():_stack_frame(), _memory({0}), _running(false) {}

Machine::~Machine() {}

uint32_t read_u32_le(std::ifstream& file) {
    uint8_t bytes[4];
    file.read(reinterpret_cast<char*>(bytes), 4);
    return static_cast<uint32_t>(bytes[0])
         | static_cast<uint32_t>(bytes[1]) << 8
         | static_cast<uint32_t>(bytes[2]) << 16
         | static_cast<uint32_t>(bytes[3]) << 24;
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
            this->_memory[i] = value;
            i++;
        }
    }
    return {};
}

void Machine::run() {
    this->_running = true;
    this->_stack_frame.push_back(new Frame(0));

    while (this->_running) {
        std::expected<void, IloError> res = this->process_instruction(this->_memory[this->_pc]);
        if (!res) {
            log(res.error());
            return;
        }
#ifdef DBG
        this->debug();
#endif // DBG
    }
}

int32_t Machine::frame_pop() {
    Frame* top_frame = this->_stack_frame.back();
    this->_stack_frame.pop_back();
    int32_t retval = top_frame->belt->get(0);
    return retval;
}

void Machine::frame_add() {
    Frame* new_frame = new Frame(this->_pc+1, this->_stack_frame.back()->belt);
    this->_stack_frame.push_back(new_frame);
}

void Machine::inc_pc() {
    ++this->_pc;
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
    for (Frame* frame: this->_stack_frame) {
        std::cout << "--- Frame " << count << " ---" << std::endl;
        frame->debug();
        count++;
    }
}
#endif // DBG

std::expected<void, IloError> Machine::process_instruction(uint32_t instruction) {
    switch (GET_OPCODE(instruction)) {
        case 0b110001: // add
            this->add(GET_ARGA(instruction), GET_ARGB(instruction));
            this->_pc++;
            break;
        case 0b101001: // sub
            this->sub(GET_ARGA(instruction), GET_ARGB(instruction));
            this->_pc++;
            break;
        case 0b100101: // mul
            this->mul(GET_ARGA(instruction), GET_ARGB(instruction));
            this->_pc++;
            break;
        case 0b111001: // div
            this->div(GET_ARGA(instruction), GET_ARGB(instruction));
            this->_pc++;
            break;
        case 0b110101: // and
            this->andd(GET_ARGA(instruction), GET_ARGB(instruction));
            this->_pc++;
            break;
        case 0b101101: // or
            this->orr(GET_ARGA(instruction), GET_ARGB(instruction));
            this->_pc++;
            break;
        case 0b111101: // xor
            this->xorr(GET_ARGA(instruction), GET_ARGB(instruction));
            this->_pc++;
            break;
        case 0b100011: // sll
            this->sll(GET_ARGA(instruction), GET_ARGB(instruction));
            this->_pc++;
            break;
        case 0b110011: // srl
            this->srl(GET_ARGA(instruction), GET_ARGB(instruction));
            this->_pc++;
            break;
        case 0b010001: // sra
            this->sra(GET_ARGA(instruction), GET_ARGB(instruction));
            this->_pc++;
            break;
        case 0b101011: // eq
            this->eq(GET_ARGA(instruction), GET_ARGB(instruction));
            this->_pc++;
            break;
        case 0b111011: // lt
            this->lt(GET_ARGA(instruction), GET_ARGB(instruction));
            this->_pc++;
            break;
        case 0b100111: // load
            this->load(GET_ARGA(instruction));
            this->_pc++;
            break;
        case 0b110111: // store
            this->store(GET_ARGA(instruction), GET_ARGB(instruction));
            this->_pc++;
            break;
        case 0b101111: // put
            this->put(GET_ARGA(instruction), GET_ARGB(instruction));
            this->_pc++;
            break;
        case 0b100001: // pick
            this->pick(GET_ARGA(instruction));
            this->_pc++;
            break;
        case 0b010000: // immh
            this->immh(GET_IMMEDIATE(instruction));
            this->_pc++;
            break;
        case 0b001000: // imml
            this->imml(GET_IMMEDIATE(instruction));
            this->_pc++;
            break;
        case 0b011000: // jmp
            this->jmp(GET_IMMEDIATE(instruction));
            break;
        case 0b000100: // jmpif
            this->jmpif(GET_IMMEDIATE(instruction));
            break;
        case 0b010100: // call
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

void Machine::load(uint8_t idxa) {
    this->push_belt(this->_memory[this->get_belt(idxa)]);
}

void Machine::store(uint8_t idxa, uint8_t idxb) {
    this->_memory[this->get_belt(idxa)] = this->get_belt(idxb);
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
