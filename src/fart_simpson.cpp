#include "fart_simpson.hpp"
#include <cmath>
#include <algorithm>

struct Voice {
    bool active = false;
    float t = 0.f;
    float phase = 0.f;
    float env = 0.f;
};

static float randomUniform() {
    return (float)rand() / (float)RAND_MAX;
}

int FartSimpson::frame = 0;

void FartSimpson::process(const ProcessArgs& args) {
    static Voice voice;

    static float age = 50.f, urgency = 0.5f, retention = 0.5f, shame = 0.f, surprise = 0.5f, food = 0.5f;
    static float ageCV, urgencyCV, retentionCV, shameCV, surpriseCV, foodCV;

    readKnobs(age, urgency, retention, shame, surprise, food);
    readInputs(ageCV, urgencyCV, retentionCV, shameCV, surpriseCV, foodCV);

    float ageCVc = std::clamp(ageCV, -1.f, 1.f);
    float urgencyCVc = std::clamp(urgencyCV, -1.f, 1.f);
    float retentionCVc = std::clamp(retentionCV, -1.f, 1.f);
    float shameCVc = std::clamp(shameCV, -1.f, 1.f);
    float surpriseCVc = std::clamp(surpriseCV, -1.f, 1.f);
    float foodCVc = std::clamp(foodCV, -1.f, 1.f);

    float ageFinal = std::clamp(age + ageCVc * 100.f, 0.f, 100.f);
    float urgencyFinal = std::clamp(urgency + urgencyCVc, 0.f, 1.f);
    float retentionFinal = std::clamp(retention + retentionCVc, 0.f, 1.f);
    float shameFinal = std::clamp(shame + shameCVc, 0.f, 1.f);
    float surpriseFinal = std::clamp(surprise + surpriseCVc, 0.f, 1.f);
    float foodFinal = std::clamp(food + foodCVc, 0.f, 1.f);

    bool gate = inputs[TRIGGER_INPUT].getVoltage() > 1.f || params[BUTTON_PARAM].getValue() > 0.5f || inputs[BUTTON_INPUT].getVoltage() > 1.f;
    static bool prevGate = false;

    float sampleRate = args.sampleTime;

    if (gate && !prevGate) {
        voice.active = true;
        voice.t = 0.f;
        voice.phase = 0.f;
        voice.env = 0.f;
    }
    prevGate = gate;

    float out = 0.f;
    float shameGain = 1.f - shameFinal;
    shameGain *= shameGain;

    if (voice.active) {
        voice.t += sampleRate;

        float baseFreq = 120.f - (ageFinal / 100.f) * 70.f;
        float attack = 0.02f + (1.f - urgencyFinal) * 0.08f;
        float decay = 0.25f + (1.f - urgencyFinal) * 0.3f;
        float env = 0.f;
        if (voice.t < attack) {
            env = (voice.t / attack) * (0.7f + urgencyFinal * 0.6f);
        } else {
            env = std::exp(-(voice.t - attack) / decay);
        }
        voice.env = env;

        float wideLFO = std::sin(2.f * (float)M_PI * voice.t * (0.5f + retentionFinal * 4.0f));
        float narrowLFO = std::sin(2.f * (float)M_PI * voice.t * (8.f + foodFinal * 32.f));

        float pitch = baseFreq * (1.f + wideLFO * retentionFinal * 0.2f + narrowLFO * foodFinal * 0.05f);
        voice.phase += pitch * sampleRate;
        if (voice.phase > 1.f) voice.phase -= 1.f;

        float core = std::tanh(std::sin(2.f * (float)M_PI * voice.phase));
        float noise = (randomUniform() * 2.f - 1.f) * (0.1f + surpriseFinal * 0.6f);
        float wet = std::sin(2.f * (float)M_PI * voice.phase * (2.5f + foodFinal * 2.f)) * (0.1f + foodFinal * 0.5f);

        out = (core + noise + wet) * env * shameGain * 5.f;

        if (env < 0.001f) {
            voice.active = false;
        }
    }

    outputs[AUDIO_OUTPUT].setVoltage(out);
}

void FartSimpson::readKnobs(float &age, float &urgency, float &retention, float &shame, float &surprise, float &food) {
    if (frame++ == 63) {
        age = params[AGE_PARAM].getValue();
        urgency = params[URGENCY_PARAM].getValue();
        retention = params[RETENTION_PARAM].getValue();
        shame = params[SHAME_PARAM].getValue();
        surprise = params[SURPRISE_PARAM].getValue();
        food = params[FOOD_PARAM].getValue();
        frame = 0;
    }
}

void FartSimpson::readInputs(float &ageCV, float &urgencyCV, float &retentionCV, float &shameCV, float &surpriseCV, float &foodCV) {
    if (inputs[AGE_CV_INPUT].isConnected()) {
        float v = inputs[AGE_CV_INPUT].getVoltage();
        float mapped = std::clamp(((v + 10.f) / 20.f) * 100.f, 0.f, 100.f);
        getParamQuantity(AGE_PARAM)->setDisplayValue(mapped);
        ageCV = (mapped - params[AGE_PARAM].getValue()) / 100.f;
    } else {
        getParamQuantity(AGE_PARAM)->setDisplayValue(params[AGE_PARAM].getValue());
        ageCV = 0.f;
    }
    if (inputs[URGENCY_CV_INPUT].isConnected()) {
        float v = inputs[URGENCY_CV_INPUT].getVoltage();
        float mapped = std::clamp((v + 10.f) / 20.f, 0.f, 1.f);
        getParamQuantity(URGENCY_PARAM)->setDisplayValue(mapped);
        urgencyCV = mapped - params[URGENCY_PARAM].getValue();
    } else {
        getParamQuantity(URGENCY_PARAM)->setDisplayValue(params[URGENCY_PARAM].getValue());
        urgencyCV = 0.f;
    }
    if (inputs[RETENTION_CV_INPUT].isConnected()) {
        float v = inputs[RETENTION_CV_INPUT].getVoltage();
        float mapped = std::clamp((v + 10.f) / 20.f, 0.f, 1.f);
        getParamQuantity(RETENTION_PARAM)->setDisplayValue(mapped);
        retentionCV = mapped - params[RETENTION_PARAM].getValue();
    } else {
        getParamQuantity(RETENTION_PARAM)->setDisplayValue(params[RETENTION_PARAM].getValue());
        retentionCV = 0.f;
    }
    if (inputs[SHAME_CV_INPUT].isConnected()) {
        float v = inputs[SHAME_CV_INPUT].getVoltage();
        float mapped = std::clamp((v + 10.f) / 20.f, 0.f, 1.f);
        getParamQuantity(SHAME_PARAM)->setDisplayValue(mapped);
        shameCV = mapped - params[SHAME_PARAM].getValue();
    } else {
        getParamQuantity(SHAME_PARAM)->setDisplayValue(params[SHAME_PARAM].getValue());
        shameCV = 0.f;
    }
    if (inputs[SURPRISE_CV_INPUT].isConnected()) {
        float v = inputs[SURPRISE_CV_INPUT].getVoltage();
        float mapped = std::clamp((v + 10.f) / 20.f, 0.f, 1.f);
        getParamQuantity(SURPRISE_PARAM)->setDisplayValue(mapped);
        surpriseCV = mapped - params[SURPRISE_PARAM].getValue();
    } else {
        getParamQuantity(SURPRISE_PARAM)->setDisplayValue(params[SURPRISE_PARAM].getValue());
        surpriseCV = 0.f;
    }
    if (inputs[FOOD_CV_INPUT].isConnected()) {
        float v = inputs[FOOD_CV_INPUT].getVoltage();
        float mapped = std::clamp((v + 10.f) / 20.f, 0.f, 1.f);
        getParamQuantity(FOOD_PARAM)->setDisplayValue(mapped);
        foodCV = mapped - params[FOOD_PARAM].getValue();
    } else {
        getParamQuantity(FOOD_PARAM)->setDisplayValue(params[FOOD_PARAM].getValue());
        foodCV = 0.f;
    }
}