#pragma once
#include <rack.hpp>

using namespace rack;

extern Model* modelFartSynthson;

struct FartSynthson : Module {
    enum ParamIds {
        AGE_PARAM,
        URGENCY_PARAM,
        RETENTION_PARAM,
        SHAME_PARAM,
        SURPRISE_PARAM,
        FOOD_PARAM,
        BUTTON_PARAM,
        NUM_PARAMS
    };
    enum InputIds {
        AGE_CV_INPUT,
        URGENCY_CV_INPUT,
        RETENTION_CV_INPUT,
        SHAME_CV_INPUT,
        SURPRISE_CV_INPUT,
        FOOD_CV_INPUT,
        TRIGGER_INPUT,
        BUTTON_INPUT,
        NUM_INPUTS
    };
    enum OutputIds {
        AUDIO_OUTPUT,
        NUM_OUTPUTS
    };

    static int frame;

    FartSynthson() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, 0);
        configParam(AGE_PARAM, 0.f, 100.f, 50.f, "Age");
        configParam(URGENCY_PARAM, 0.f, 1.f, 0.5f, "Urgency");
        configParam(RETENTION_PARAM, 0.f, 1.f, 0.5f, "Retention");
        configParam(SHAME_PARAM, 0.f, 1.f, 0.f, "Shame (0 loud, 1 silent)");
        configParam(SURPRISE_PARAM, 0.f, 1.f, 0.5f, "Surprise");
        configParam(FOOD_PARAM, 0.f, 1.f, 0.5f, "Food (wetness)");
        configParam(BUTTON_PARAM, 0.f, 1.f, 0.f, "Let it out");

        configInput(AGE_CV_INPUT,  "Age (CV)");
        configInput(URGENCY_CV_INPUT, "Urgency (CV)");
        configInput(RETENTION_CV_INPUT, "Retention (CV)");
        configInput(SHAME_CV_INPUT, "Shame (0 loud, 1 silent) (CV)");
        configInput(SURPRISE_CV_INPUT, "Surprise (CV)");
        configInput(FOOD_CV_INPUT, "Food (CV)");
        configInput(TRIGGER_INPUT, "Let it out (Clock)");

        configOutput(AUDIO_OUTPUT, "Audio Output");
    }

    void process(const ProcessArgs &args) override;
    void readKnobs(float &age, float &urgency, float &retention,float &shame, float &surprise, float &food);
    void readInputs(float &ageCV, float &urgencyCV, float &retentionCV, float &shameCV, float &surpriseCV, float &foodCV);
};
