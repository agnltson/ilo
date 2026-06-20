#include "machine.hpp"

Machine::Machine():_stack_frame(), _ram({0}), _running(false) {}

Machine::~Machine() {}

void Machine::run() {
    this->_running = true;
    while (this->_running) {

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
