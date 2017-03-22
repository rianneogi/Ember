#pragma once

#include "PositionNN.h"

void train(Tensor inputs, Tensor outputs);
Tensor moveToTensor(Move m);