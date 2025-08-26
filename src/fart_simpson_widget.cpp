
#include "fart_simpson_widget.hpp"
#include "fart_simpson.hpp"
#include "plugin.hpp"
#include <rack.hpp>

using namespace rack;

FartSimpsonWidget::FartSimpsonWidget(FartSimpson* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/layout.svg")));

    addParam(createParamCentered<Rogan2PWhite>(mm2px(Vec(10, 27)), module, FartSimpson::AGE_PARAM));
    addParam(createParamCentered<Rogan2PRed>(mm2px(Vec(10, 44)), module, FartSimpson::URGENCY_PARAM));
    addParam(createParamCentered<Rogan2PBlue>(mm2px(Vec(10, 61)), module, FartSimpson::RETENTION_PARAM));
    addParam(createParamCentered<Rogan2PRed>(mm2px(Vec(32, 27)), module, FartSimpson::SHAME_PARAM));
    addParam(createParamCentered<Rogan2PBlue>(mm2px(Vec(32, 44)), module, FartSimpson::SURPRISE_PARAM));    
    addParam(createParamCentered<Rogan2PGreen>(mm2px(Vec(32, 61)), module, FartSimpson::FOOD_PARAM));

    addParam(createParamCentered<CKD6>(mm2px(Vec(21, 81)), module, FartSimpson::BUTTON_PARAM));

    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(5, 100)), module, FartSimpson::RETENTION_CV_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(15, 100)), module, FartSimpson::TRIGGER_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(25, 100)), module, FartSimpson::SHAME_CV_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(35, 100)), module, FartSimpson::FOOD_CV_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(5, 110)), module, FartSimpson::URGENCY_CV_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(15, 110)), module, FartSimpson::AGE_CV_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(25, 110)), module, FartSimpson::SURPRISE_CV_INPUT));
    
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35, 110)), module, FartSimpson::AUDIO_OUTPUT));
}
