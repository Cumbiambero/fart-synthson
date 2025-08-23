#include "fart_simpson.hpp"
#include <cmath>

using namespace rack;

void FartSimpson::process(const ProcessArgs& args) {
    static int paramCounter = 0;
    static struct ParamCache {
        float age, urgency, alimentation, loudness, wetness, length, pitch, flutter;
    } cache;
    if (paramCounter++ % 64 == 0) {
        cache.age = std::clamp(params[AGE_PARAM].getValue() + inputs[AGE_CV_INPUT].getVoltage() / 10.f, 0.f, 1.f);
        cache.urgency = std::clamp(params[URGENCY_PARAM].getValue() + inputs[URGENCY_CV_INPUT].getVoltage() / 10.f, 0.f, 1.f);
        cache.alimentation = std::clamp(params[FOOD_PARAM].getValue() + inputs[ALIMENTATION_CV_INPUT].getVoltage() / 10.f, 0.f, 1.f);
        cache.loudness = std::clamp(params[LOUDNESS_PARAM].getValue() + inputs[LOUDNESS_CV_INPUT].getVoltage() / 10.f, 0.f, 1.f);
        cache.wetness = std::clamp(params[WETNESS_PARAM].getValue() + inputs[WETNESS_CV_INPUT].getVoltage() / 10.f, 0.f, 1.f);
        cache.length = std::clamp(params[LENGTH_PARAM].getValue() + inputs[LENGTH_CV_INPUT].getVoltage() / 10.f, 0.f, 1.f);
        cache.pitch = std::clamp(params[INTENSION_PARAM].getValue() + inputs[PITCH_CV_INPUT].getVoltage() / 10.f, 0.f, 1.f);
        cache.flutter = std::clamp(params[FLUTTER_PARAM].getValue() + inputs[FLUTTER_CV_INPUT].getVoltage() / 10.f, 0.f, 1.f);
    }
    const auto& [age, urgency, alimentation, loudness, wetness, length, pitch, flutter] = cache;

    bool gate = inputs[GATE_INPUT].getVoltage() > 1.f;
    bool trig = inputs[TRIGGER_INPUT].getVoltage() > 1.f;
    bool button = params[BUTTON_PARAM].getValue() > 0.5f || inputs[BUTTON_INPUT].getVoltage() > 1.f;
    static bool farting = false;
    static float fartPhase = 0.f;
    static float fartLength = 0.f;
    static float lastOut = 0.f;

    if ((gate || trig || button) && !farting) {
        farting = true;
        fartPhase = 0.f;
        fartLength = 0.2f + length * 1.8f;
        lastOut = 0.f;
    }
    if (!gate && !trig && !button && farting && fartPhase > fartLength) {
        farting = false;
    }

    float out = 0.f;
    if (farting) {
        float t = fartPhase;
        constexpr float pi = static_cast<float>(M_PI);
        float basePitch = 40.f + pitch * 120.f;
        float foodLfo = std::sin(2.f * pi * t * (0.2f + alimentation * 2.5f));
        float flutterLfo = std::sin(2.f * pi * t * (0.05f + flutter * 1.5f));
        float freq = basePitch * (1.f - 0.5f * age) + foodLfo * 8.f + flutterLfo * 16.f;
        float envAttack = std::min(1.f, t / (0.01f + 0.09f * (1.f - urgency)));
        float envDecay = std::exp(-t / (0.2f + 0.6f * (1.f - urgency)));
        float env = envAttack * envDecay;
        float squelch = wetness * std::sin(2.f * pi * t * (1.2f + wetness * 3.f)) * (0.7f + wetness * 0.6f);
        float liquid = wetness * (std::sin(2.f * pi * t * (8.f + wetness * 32.f + alimentation * 10.f)) * 0.5f + std::sin(2.f * pi * t * (13.f + wetness * 20.f)) * 0.3f);
        float noise = (random::uniform() * 2.f - 1.f) * (0.15f + wetness * 0.7f) * (1.f - age * 0.7f);
        float osc = std::sin(2.f * pi * freq * t) * (1.f - age * 0.8f);
        float raw = (osc + squelch + noise + liquid) * env * loudness * 5.f;
        float cutoff = 800.f + wetness * 1200.f - age * 600.f;
        float RC = 1.f / (2.f * pi * cutoff);
        float alpha = args.sampleTime / (RC + args.sampleTime);
        out = lastOut + alpha * (raw - lastOut);
        lastOut = out;
        fartPhase += args.sampleTime;
    } else {
        lastOut = 0.f;
    }
    outputs[AUDIO_OUTPUT].setVoltage(out);
}
