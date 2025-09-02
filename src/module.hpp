#pragma once
#include <rack.hpp>
#include <cmath>
#include <algorithm>
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
        configParam(SHAME_PARAM, 0.f, 1.f, 0.f, "Shame");
        configParam(SURPRISE_PARAM, 0.f, 1.f, 0.5f, "Surprise");
        configParam(FOOD_PARAM, 0.f, 1.f, 0.5f, "Food");
        configParam(BUTTON_PARAM, 0.f, 1.f, 0.f, "Let it out");
        configInput(AGE_CV_INPUT,  "Age (CV)");
        configInput(URGENCY_CV_INPUT, "Urgency (CV)");
        configInput(RETENTION_CV_INPUT, "Retention (CV)");
        configInput(SHAME_CV_INPUT, "Shame (CV)");
        configInput(SURPRISE_CV_INPUT, "Surprise (CV)");
        configInput(FOOD_CV_INPUT, "Food (CV)");
        configInput(TRIGGER_INPUT, "Let it out (Clock)");
        configOutput(AUDIO_OUTPUT, "Audio Output");
    }

    void process(const ProcessArgs &args) override;

private:
    static constexpr float pi = 3.14159265358979323846f;
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
  float bubbleEnvTarget = 0.f;
  float pink1 = 0.f, pink2 = 0.f, pink3 = 0.f;
  float pressureEnv = 0.f;
  float burstEnv = 0.f, burstEnvTarget = 0.f;
  float formantEnv = 0.f, formantEnvTarget = 0.f;
  float formantPhase = 0.f;
  float ampRand = 0.f, ampRandTarget = 0.f;
  float pitchRand = 0.f, pitchRandTarget = 0.f;
  float randTimer = 0.f, randInterval = 0.03f;
  float noiseLP = 0.f;
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
  randTimer += dt;
  if (randTimer >= randInterval) {
    randTimer -= randInterval;
    randInterval = 0.02f + frand() * 0.06f;
    ampRandTarget = (frand() * 2.f - 1.f) * (0.2f + foodF * 0.5f);
    pitchRandTarget = (frand() * 2.f - 1.f) * (0.4f + retentionF * 0.6f);
  }
  ampRand += (ampRandTarget - ampRand) * 0.0025f;
  pitchRand += (pitchRandTarget - pitchRand) * 0.0025f;
  float baseFreq = 120.f - (ageF * 0.7f);
  float attack = 0.08f + (1.f - urgencyF) * 0.15f;
  float decay = 0.5f + (1.f - urgencyF) * 0.5f;
  env = t < attack ? (t / attack) * (0.5f + urgencyF * 0.5f)
                   : std::exp(-(t - attack) / decay);
  pressureEnv += ((gate ? 1.f : 0.f) - pressureEnv) * (gate ? 0.002f + urgencyF * 0.004f : 0.0008f + (1.f - retentionF) * 0.0012f);
  ampLFOPhase += dt * (0.2f + foodF * 0.3f);
  wrap(ampLFOPhase);
  ampLFO = 0.7f + 0.3f * fastSin(two_pi * ampLFOPhase);
  pitchLFOPhase += dt * (0.12f + retentionF * 0.18f);
  wrap(pitchLFOPhase);
  pitchLFO = 1.f + 0.08f * fastSin(two_pi * pitchLFOPhase);
  float wideLFO = fastSin(two_pi * t * (0.3f + retentionF * 2.f));
  float narrowLFO = fastSin(two_pi * t * (3.f + foodF * 10.f));
  float pitch = baseFreq * pitchLFO * (1.f + wideLFO * retentionF * 0.25f + narrowLFO * foodF * 0.08f);
  phase += (pitch + pitchRand * pitch * 0.1f) * dt;
  wrap(phase);
  float core = fastTanh(fastSin(two_pi * phase));
  float white = frand() * 2.f - 1.f;
  brown = (brown + 0.02f * white) * 0.98f;
  pink1 = 0.997f * pink1 + 0.003f * white;
  pink2 = 0.985f * pink2 + 0.015f * white;
  pink3 = 0.95f * pink3 + 0.05f * white;
  float pink = (pink1 + pink2 + pink3) * 0.3333f;
  float noiseColor = foodF * 0.6f + 0.2f;
  float noise = (brown * (1.f - noiseColor) + pink * noiseColor) * (0.18f + surpriseF * 0.7f);
  float wetResGain = 0.4f + urgencyF * 0.6f;
  float wetFreq = 1.5f + foodF * (2.5f + urgencyF * 1.2f);
  float wet = fastSin(two_pi * phase * wetFreq) * (0.12f + foodF * 0.6f) * wetResGain;
  if (bubbleEnv < 0.001f && frand() < (0.008f + foodF * 0.03f + surpriseF * 0.01f)) {
    bubbleEnvTarget = 0.35f + foodF * 0.65f;
    bubblePhase = 0.f;
    formantEnvTarget = 0.7f + foodF * 0.3f;
    formantPhase = 0.f;
    burstEnvTarget = 0.6f + surpriseF * 0.4f;
  }
  if (bubbleEnvTarget > 0.f) {
    bubbleEnv += (bubbleEnvTarget - bubbleEnv) * 0.15f;
  }
  if (bubbleEnv > 0.001f) {
    bubblePhase += dt * (8.f + frand() * 8.f);
    wrap(bubblePhase);
    float bubble =
        fastSin(two_pi * bubblePhase) * bubbleEnv * (0.2f + foodF * 0.5f);
    noise += bubble;
    bubbleEnv *= 0.94f - 0.05f * surpriseF;
    if (bubbleEnv < 0.002f) { bubbleEnv = 0.f; bubbleEnvTarget = 0.f; }
  }
  if (formantEnvTarget > 0.f) {
    formantEnv += (formantEnvTarget - formantEnv) * 0.08f;
  }
  if (formantEnv > 0.0005f) {
    float formantFreq = 90.f + foodF * 220.f + retentionF * 70.f;
    formantPhase += formantFreq * dt;
    wrap(formantPhase);
    float ring = fastSin(two_pi * formantPhase) * formantEnv;
    noise += ring * (0.25f + foodF * 0.35f);
    formantEnv *= 0.995f - 0.04f * (1.f - retentionF);
    if (formantEnv < 0.001f) { formantEnv = 0.f; formantEnvTarget = 0.f; }
  }
  if (burstEnvTarget > 0.f) {
    burstEnv += (burstEnvTarget - burstEnv) * 0.2f;
  }
  if (burstEnv > 0.0005f) {
    float hiss = white * burstEnv * (0.4f + foodF * 0.4f);
    noise += hiss;
    burstEnv *= 0.90f - 0.1f * foodF;
    if (burstEnv < 0.001f) { burstEnv = 0.f; burstEnvTarget = 0.f; }
  }
  float lpA = std::clamp(8000.f * dt, 0.f, 1.f);
  noiseLP += (noise - noiseLP) * lpA;
  noise = noiseLP;
  float sg = 1.f - shameF;
  sg *= sg;
  float dyn = 0.6f + ampRand * 0.4f;
  float mix = (core + noise + wet) * env * sg * ampLFO * dyn * (0.7f + pressureEnv * 0.6f);
  float sat = fastTanh(mix * 1.4f) * 5.2f;
  outputs[AUDIO_OUTPUT].setVoltage(sat);
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
