#pragma once
#include "fart_simpson.hpp"
#include <rack.hpp>

using namespace rack;

struct FartSimpsonWidget : ModuleWidget {
    FartSimpsonWidget(FartSimpson* module);
};
