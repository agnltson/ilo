#include "error.hpp"

void log(IloError err) {
    switch (err) {
    case IloError::NoInputFile:
        std::cout << "fatal: Given file doesn't exist" << std::endl;
        break;
    case IloError::InvalidBinaryMetadata:
        std::cout << "fatal: Invalid binary metadata" << std::endl;
        break;
    case IloError::InvalidOpcode:
        std::cout << "fatal: Invalid Opcode" << std::endl;
        break;
    default:
        std::cout << "Something wrong happend" << std::endl;
    }
}
