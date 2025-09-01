#pragma once
#include <rack.hpp>
#include <cmath>
#include <algorithm>
#include <numbers>
#include <cstdint>

using rack::componentlibrary::Rogan2PWhite;
using rack::componentlibrary::Rogan2PRed;
using rack::componentlibrary::Rogan2PBlue;
using rack::componentlibrary::Rogan2PGreen;
using rack::componentlibrary::CKD6;
using rack::componentlibrary::DarkPJ301MPort;
using rack::componentlibrary::PJ301MPort;

extern rack::Model* modelFartSynthson;
extern rack::plugin::Plugin* pluginInstance;

struct FartSynthson final : rack::Module {
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

private:
    static constexpr float pi = std::numbers::pi_v<float>;
    static constexpr float two_pi = 2.f * pi;
    uint32_t rngState = 0x12345678u;
    int frameCounter = 0;
    float t = 0.f;
    float phase = 0.f;
    float env = 0.f;
    float ampLFOPhase = 0.f;
    float pitchLFOPhase = 0.f;
    float ampLFO = 0.f;
    float pitchLFO = 0.f;
    float brown = 0.f;
    float bubbleEnv = 0.f;
    float bubblePhase = 0.f;
    bool gatePrev = false;
    float ageV = 50.f, urgencyV = 0.5f, retentionV = 0.5f, shameV = 0.f, surpriseV = 0.5f, foodV = 0.5f;
    float ageCV = 0.f, urgencyCV = 0.f, retentionCV = 0.f, shameCV = 0.f, surpriseCV = 0.f, foodCV = 0.f;

    inline float frand() noexcept {
        uint32_t x = rngState;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        rngState = x;
        return static_cast<float>(x) * (1.f / 4294967295.f);
    }
    static inline float fastSin(float x) noexcept {
        x -= two_pi * std::floor((x + pi) / two_pi);
        float x2 = x * x;
        return x * (1.f - x2 * (1.f/6.f - x2 * (1.f/120.f)));
    }
    static inline float fastTanh(float x) noexcept {
        if (x > 2.5f) return 1.f;
        if (x < -2.5f) return -1.f;
        float x2 = x * x;
        return x * (27.f + x2) / (27.f + 9.f * x2);
    }
    inline void wrap(float &p) noexcept { if (p >= 1.f) p -= 1.f; }
    void readKnobs();
    void readInputs();
};
inline void FartSynthson::process(const ProcessArgs& args) {
  readKnobs();
  readInputs();
  float ageF =
      std::clamp(ageV + std::clamp(ageCV, -1.f, 1.f) * 100.f, 0.f, 100.f);
  float urgencyF =
      std::clamp(urgencyV + std::clamp(urgencyCV, -1.f, 1.f), 0.f, 1.f);
  float retentionF =
      std::clamp(retentionV + std::clamp(retentionCV, -1.f, 1.f), 0.f, 1.f);
  float shameF = std::clamp(shameV + std::clamp(shameCV, -1.f, 1.f), 0.f, 1.f);
  float surpriseF =
      std::clamp(surpriseV + std::clamp(surpriseCV, -1.f, 1.f), 0.f, 1.f);
  float foodF = std::clamp(foodV + std::clamp(foodCV, -1.f, 1.f), 0.f, 1.f);
  bool gate = inputs[TRIGGER_INPUT].getVoltage() > 1.f ||
              params[BUTTON_PARAM].getValue() > 0.5f ||
              inputs[BUTTON_INPUT].getVoltage() > 1.f;
  if (gate && !gatePrev) {
    t = 0.f;
    phase = 0.f;
    env = 0.f;
  }
  gatePrev = gate;
  float dt = args.sampleTime;
  t += dt;
  float baseFreq = 120.f - (ageF * 0.7f);
  float attack = 0.08f + (1.f - urgencyF) * 0.15f;
  float decay = 0.5f + (1.f - urgencyF) * 0.5f;
  env = t < attack ? (t / attack) * (0.5f + urgencyF * 0.5f)
                   : std::exp(-(t - attack) / decay);
  ampLFOPhase += dt * (0.2f + foodF * 0.3f);
  wrap(ampLFOPhase);
  ampLFO = 0.7f + 0.3f * fastSin(two_pi * ampLFOPhase + frand() * 0.2f);
  pitchLFOPhase += dt * (0.12f + retentionF * 0.18f);
  wrap(pitchLFOPhase);
  pitchLFO = 1.f + 0.08f * fastSin(two_pi * pitchLFOPhase + frand() * 0.3f);
  float wideLFO = fastSin(two_pi * t * (0.3f + retentionF * 2.f));
  float narrowLFO = fastSin(two_pi * t * (3.f + foodF * 10.f));
  float pitch =
      baseFreq * pitchLFO *
      (1.f + wideLFO * retentionF * 0.25f + narrowLFO * foodF * 0.08f);
  phase += pitch * dt;
  wrap(phase);
  float core = fastTanh(fastSin(two_pi * phase));
  float white = frand() * 2.f - 1.f;
  brown = (brown + 0.02f * white) * 0.98f;
  float noise = brown * (0.18f + surpriseF * 0.7f);
  float wet =
      fastSin(two_pi * phase * (1.5f + foodF * 2.5f)) * (0.12f + foodF * 0.6f);
  if (bubbleEnv < 0.001f &&
      frand() < (0.008f + foodF * 0.03f + surpriseF * 0.01f)) {
    bubbleEnv = 1.f;
    bubblePhase = 0.f;
  }
  if (bubbleEnv > 0.001f) {
    bubblePhase += dt * (8.f + frand() * 8.f);
    wrap(bubblePhase);
    float bubble =
        fastSin(two_pi * bubblePhase) * bubbleEnv * (0.2f + foodF * 0.5f);
    noise += bubble;
    bubbleEnv *= 0.93f - 0.04f * surpriseF;
  }
  float sg = 1.f - shameF;
  sg *= sg;
  float out = (core + noise + wet) * env * sg * ampLFO * 5.f;
  outputs[AUDIO_OUTPUT].setVoltage(out);
}
inline void FartSynthson::readKnobs() {
  if (++frameCounter == 64) {
    frameCounter = 0;
    ageV = params[AGE_PARAM].getValue();
    urgencyV = params[URGENCY_PARAM].getValue();
    retentionV = params[RETENTION_PARAM].getValue();
    shameV = params[SHAME_PARAM].getValue();
    surpriseV = params[SURPRISE_PARAM].getValue();
    foodV = params[FOOD_PARAM].getValue();
  }
}
inline void FartSynthson::readInputs() {
  if (inputs[AGE_CV_INPUT].isConnected()) {
    float v = inputs[AGE_CV_INPUT].getVoltage();
    float mapped = std::clamp(((v + 10.f) * 0.05f) * 100.f, 0.f, 100.f);
    getParamQuantity(AGE_PARAM)->setDisplayValue(mapped);
    ageCV = (mapped - params[AGE_PARAM].getValue()) * 0.01f;
  } else
    ageCV = 0.f;
  if (inputs[URGENCY_CV_INPUT].isConnected()) {
    float v = inputs[URGENCY_CV_INPUT].getVoltage();
    float mapped = std::clamp((v + 10.f) * 0.05f, 0.f, 1.f);
    getParamQuantity(URGENCY_PARAM)->setDisplayValue(mapped);
    urgencyCV = mapped - params[URGENCY_PARAM].getValue();
  } else
    urgencyCV = 0.f;
  if (inputs[RETENTION_CV_INPUT].isConnected()) {
    float v = inputs[RETENTION_CV_INPUT].getVoltage();
    float mapped = std::clamp((v + 10.f) * 0.05f, 0.f, 1.f);
    getParamQuantity(RETENTION_PARAM)->setDisplayValue(mapped);
    retentionCV = mapped - params[RETENTION_PARAM].getValue();
  } else
    retentionCV = 0.f;
  if (inputs[SHAME_CV_INPUT].isConnected()) {
    float v = inputs[SHAME_CV_INPUT].getVoltage();
    float mapped = std::clamp((v + 10.f) * 0.05f, 0.f, 1.f);
    getParamQuantity(SHAME_PARAM)->setDisplayValue(mapped);
    shameCV = mapped - params[SHAME_PARAM].getValue();
  } else
    shameCV = 0.f;
  if (inputs[SURPRISE_CV_INPUT].isConnected()) {
    float v = inputs[SURPRISE_CV_INPUT].getVoltage();
    float mapped = std::clamp((v + 10.f) * 0.05f, 0.f, 1.f);
    getParamQuantity(SURPRISE_PARAM)->setDisplayValue(mapped);
    surpriseCV = mapped - params[SURPRISE_PARAM].getValue();
  } else
    surpriseCV = 0.f;
  if (inputs[FOOD_CV_INPUT].isConnected()) {
    float v = inputs[FOOD_CV_INPUT].getVoltage();
    float mapped = std::clamp((v + 10.f) * 0.05f, 0.f, 1.f);
    getParamQuantity(FOOD_PARAM)->setDisplayValue(mapped);
    foodCV = mapped - params[FOOD_PARAM].getValue();
  } else
    foodCV = 0.f;
}
struct FartSynthsonWidget final : rack::ModuleWidget {
    explicit FartSynthsonWidget(FartSynthson* module) {
        setModule(module);
        setPanel(rack::createPanel(rack::asset::plugin(pluginInstance, "res/layout.svg")));
        addParam(rack::createParamCentered<Rogan2PWhite>(rack::mm2px(rack::Vec(10, 27)), module, FartSynthson::AGE_PARAM));
        addParam(rack::createParamCentered<Rogan2PRed>(rack::mm2px(rack::Vec(10, 44)), module, FartSynthson::URGENCY_PARAM));
        addParam(rack::createParamCentered<Rogan2PBlue>(rack::mm2px(rack::Vec(10, 61)), module, FartSynthson::RETENTION_PARAM));
        addParam(rack::createParamCentered<Rogan2PRed>(rack::mm2px(rack::Vec(32, 27)), module, FartSynthson::SHAME_PARAM));
        addParam(rack::createParamCentered<Rogan2PBlue>(rack::mm2px(rack::Vec(32, 44)), module, FartSynthson::SURPRISE_PARAM));
        addParam(rack::createParamCentered<Rogan2PGreen>(rack::mm2px(rack::Vec(32, 61)), module, FartSynthson::FOOD_PARAM));
        addParam(rack::createParamCentered<CKD6>(rack::mm2px(rack::Vec(21, 81)), module, FartSynthson::BUTTON_PARAM));
        addInput(rack::createInputCentered<DarkPJ301MPort>(rack::mm2px(rack::Vec(5, 100)), module, FartSynthson::RETENTION_CV_INPUT));
        addInput(rack::createInputCentered<DarkPJ301MPort>(rack::mm2px(rack::Vec(15, 100)), module, FartSynthson::TRIGGER_INPUT));
        addInput(rack::createInputCentered<DarkPJ301MPort>(rack::mm2px(rack::Vec(25, 100)), module, FartSynthson::SHAME_CV_INPUT));
        addInput(rack::createInputCentered<DarkPJ301MPort>(rack::mm2px(rack::Vec(35, 100)), module, FartSynthson::FOOD_CV_INPUT));
        addInput(rack::createInputCentered<DarkPJ301MPort>(rack::mm2px(rack::Vec(5, 110)), module, FartSynthson::URGENCY_CV_INPUT));
        addInput(rack::createInputCentered<DarkPJ301MPort>(rack::mm2px(rack::Vec(15, 110)), module, FartSynthson::AGE_CV_INPUT));
        addInput(rack::createInputCentered<DarkPJ301MPort>(rack::mm2px(rack::Vec(25, 110)), module, FartSynthson::SURPRISE_CV_INPUT));
        addOutput(rack::createOutputCentered<PJ301MPort>(rack::mm2px(rack::Vec(35, 110)), module, FartSynthson::AUDIO_OUTPUT));
    }
};
inline void initFartSynthson(rack::plugin::Plugin* p) { pluginInstance = p; modelFartSynthson = rack::createModel<FartSynthson, FartSynthsonWidget>("FartSynthson"); p->addModel(modelFartSynthson); }
