#pragma once

#include <iostream>

enum class IloError {
    NoInputFile,
    InvalidBinaryMetadata,
    InvalidOpcode,
};

void log(IloError err);
