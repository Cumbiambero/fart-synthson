
#include "plugin.hpp"
#include "module.hpp"

rack::plugin::Plugin* pluginInstance;

Model* modelFartSynthson = nullptr;

#include "widget.hpp"

void init(rack::plugin::Plugin* p) {
    pluginInstance = p;
    modelFartSynthson = createModel<FartSynthson, FartSynthsonWidget>("FartSynthson");
    p->addModel(modelFartSynthson);
}
