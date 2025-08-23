
#include "plugin.hpp"
#include "fart_simpson.hpp"

rack::plugin::Plugin* pluginInstance;


Model* modelFartSimpson = nullptr;

#include "fart_simpson_widget.hpp"

void init(rack::plugin::Plugin* p) {
    pluginInstance = p;
    modelFartSimpson = createModel<FartSimpson, FartSimpsonWidget>("FartSimpson");
    p->addModel(modelFartSimpson);
}
