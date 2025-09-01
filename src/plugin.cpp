#include <rack.hpp>
#include "module.hpp"

rack::Model* modelFartSynthson = nullptr;
rack::plugin::Plugin* pluginInstance = nullptr;

extern "C" void init(rack::plugin::Plugin* p) { initFartSynthson(p); }