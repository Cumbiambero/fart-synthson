#pragma once
#include <rack.hpp>

using namespace rack;

extern Model* modelFartSimpson;

struct FartSimpson : Module {
    enum ParamIds {
        AGE_PARAM,
        URGENCY_PARAM,
        FOOD_PARAM,
        LOUDNESS_PARAM,
        WETNESS_PARAM,
        LENGTH_PARAM,
        INTENSION_PARAM,
        FLUTTER_PARAM,
        BUTTON_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        AGE_CV_INPUT,
        URGENCY_CV_INPUT,
        ALIMENTATION_CV_INPUT,
        LOUDNESS_CV_INPUT,
        WETNESS_CV_INPUT,
        LENGTH_CV_INPUT,
        PITCH_CV_INPUT,
        FLUTTER_CV_INPUT,
        GATE_INPUT,
        TRIGGER_INPUT,
        BUTTON_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        AUDIO_OUTPUT,
        NUM_OUTPUTS
    };
    enum LightIds {
        NUM_LIGHTS
    };

    FartSimpson() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(AGE_PARAM, 0.f, 1.f, 0.5f, "Age");
        configParam(URGENCY_PARAM, 0.f, 1.f, 0.5f, "Urgency");
        configParam(FOOD_PARAM, 0.f, 1.f, 0.5f, "Food");
        configParam(LOUDNESS_PARAM, 0.f, 1.f, 0.5f, "Loudness");
        configParam(WETNESS_PARAM, 0.f, 1.f, 0.5f, "Wetness");
        configParam(LENGTH_PARAM, 0.f, 1.f, 0.5f, "Length");
        configParam(INTENSION_PARAM, 0.f, 1.f, 0.5f, "Intension");
        configParam(FLUTTER_PARAM, 0.f, 1.f, 0.5f, "Flutter");
        configParam(BUTTON_PARAM, 0.f, 1.f, 0.f, "Manual Trigger");
    }

    void process(const ProcessArgs& args) override;
};
