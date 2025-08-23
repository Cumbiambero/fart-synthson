
#include "fart_simpson_widget.hpp"
#include "fart_simpson.hpp"
#include "plugin.hpp"
#include <rack.hpp>

using namespace rack;

FartSimpsonWidget::FartSimpsonWidget(FartSimpson* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/layout.svg")));

    addParam(createParamCentered<Rogan2PBlue>(mm2px(Vec(11, 25)), module, FartSimpson::AGE_PARAM));
    addParam(createParamCentered<Rogan2PBlue>(mm2px(Vec(11, 42)), module, FartSimpson::URGENCY_PARAM));
    addParam(createParamCentered<Rogan2PBlue>(mm2px(Vec(11, 60)), module, FartSimpson::FOOD_PARAM));
    addParam(createParamCentered<Rogan2PBlue>(mm2px(Vec(11, 78)), module, FartSimpson::WETNESS_PARAM));
    addParam(createParamCentered<Rogan2PBlue>(mm2px(Vec(31, 25)), module, FartSimpson::LOUDNESS_PARAM));
    addParam(createParamCentered<Rogan2PBlue>(mm2px(Vec(31, 42)), module, FartSimpson::LENGTH_PARAM));
    addParam(createParamCentered<Rogan2PBlue>(mm2px(Vec(31, 60)), module, FartSimpson::INTENSION_PARAM));
    addParam(createParamCentered<Rogan2PBlue>(mm2px(Vec(31, 78)), module, FartSimpson::FLUTTER_PARAM));

    addParam(createParamCentered<CKD6>(mm2px(Vec(22, 95)), module, FartSimpson::BUTTON_PARAM));

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 100)), module, FartSimpson::BUTTON_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 110)), module, FartSimpson::AGE_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18, 110)), module, FartSimpson::URGENCY_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28, 110)), module, FartSimpson::ALIMENTATION_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38, 110)), module, FartSimpson::LOUDNESS_CV_INPUT));
    // addInput(createInputCentered<PJ301MPort>(mm2px(Vec(48, 110)), module, FartSimpson::WETNESS_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(8, 120)), module, FartSimpson::LENGTH_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18, 120)), module, FartSimpson::PITCH_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(28, 120)), module, FartSimpson::FLUTTER_CV_INPUT));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(38, 120)), module, FartSimpson::TRIGGER_INPUT));
    // addInput(createInputCentered<PJ301MPort>(mm2px(Vec(48, 120)), module, FartSimpson::GATE_INPUT));
    
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38, 100)), module, FartSimpson::AUDIO_OUTPUT));
}
