#pragma once
#include "module.hpp"
#include <rack.hpp>

using namespace rack;

struct FartSynthsonWidget : ModuleWidget {
    FartSynthsonWidget(FartSynthson* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/layout.svg")));

    addParam(createParamCentered<Rogan2PWhite>(mm2px(Vec(10, 27)), module, FartSynthson::AGE_PARAM));
    addParam(createParamCentered<Rogan2PRed>(mm2px(Vec(10, 44)), module, FartSynthson::URGENCY_PARAM));
    addParam(createParamCentered<Rogan2PBlue>(mm2px(Vec(10, 61)), module, FartSynthson::RETENTION_PARAM));
    addParam(createParamCentered<Rogan2PRed>(mm2px(Vec(32, 27)), module, FartSynthson::SHAME_PARAM));
    addParam(createParamCentered<Rogan2PBlue>(mm2px(Vec(32, 44)), module, FartSynthson::SURPRISE_PARAM));    
    addParam(createParamCentered<Rogan2PGreen>(mm2px(Vec(32, 61)), module, FartSynthson::FOOD_PARAM));

    addParam(createParamCentered<CKD6>(mm2px(Vec(21, 81)), module, FartSynthson::BUTTON_PARAM));

    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(5, 100)), module, FartSynthson::RETENTION_CV_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(15, 100)), module, FartSynthson::TRIGGER_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(25, 100)), module, FartSynthson::SHAME_CV_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(35, 100)), module, FartSynthson::FOOD_CV_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(5, 110)), module, FartSynthson::URGENCY_CV_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(15, 110)), module, FartSynthson::AGE_CV_INPUT));
    addInput(createInputCentered<DarkPJ301MPort>(mm2px(Vec(25, 110)), module, FartSynthson::SURPRISE_CV_INPUT));
    
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(35, 110)), module, FartSynthson::AUDIO_OUTPUT));
}
};
