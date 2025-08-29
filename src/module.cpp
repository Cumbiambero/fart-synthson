#include "module.hpp"
#include <cmath>
#include <algorithm>

static constexpr float PI = std::numbers::pi_v<float>;
static constexpr float TWO_PI = 2.0f * PI;
static constexpr float HALF = 0.5f;
static constexpr float ONE = 1.0f;
static constexpr float ZERO = 0.0f;
static constexpr float INV_PI = ONE / PI;

float t = 0.f;
float phase = 0.f;
float env = 0.f;
float age = 50.f, urgency = 0.5f, retention = 0.5f, shame = 0.f, surprise = 0.5f, food = 0.5f;
float ageCV, urgencyCV, retentionCV, shameCV, surpriseCV, foodCV;
float ampLFO = 0.f, ampLFOPhase = 0.f;
float pitchLFO = 0.f, pitchLFOPhase = 0.f;
float brownNoise = 0.f;
float bubbleEnv = 0.f, bubblePhase = 0.f;

[[nodiscard]] static inline float fastSin(float x) noexcept {
    x = x - TWO_PI * std::floor((x + PI) / TWO_PI);
    const float x2 = x * x;
    return x * (ONE - x2 * (ONE/6.0f - x2 * ONE/120.0f));
}

[[nodiscard]] static inline float fastTanh(float x) noexcept {
    if (x > 2.5f) return ONE;
    if (x < -2.5f) return -ONE;
    const float x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}

[[nodiscard]] static inline float normalizePhase(float phase) noexcept {
    if (phase >= TWO_PI) {
        return phase - TWO_PI;
    }
    if (phase < ZERO) {
        return phase + TWO_PI;
    }
    return phase;
}    

static float randomUniform() {
    return (float)rand() / (float)RAND_MAX;
}

int FartSynthson::frame = 0;

void FartSynthson::process(const ProcessArgs& args) {
    readKnobs(age, urgency, retention, shame, surprise, food);
    readInputs(ageCV, urgencyCV, retentionCV, shameCV, surpriseCV, foodCV);

    float ageFinal = std::clamp(age + std::clamp(ageCV, -1.f, 1.f) * 100.f, 0.f, 100.f);
    float urgencyFinal = std::clamp(urgency + std::clamp(urgencyCV, -1.f, 1.f), 0.f, 1.f);
    float retentionFinal = std::clamp(retention + std::clamp(retentionCV, -1.f, 1.f), 0.f, 1.f);
    float shameFinal = std::clamp(shame + std::clamp(shameCV, -1.f, 1.f), 0.f, 1.f);
    float surpriseFinal = std::clamp(surprise + std::clamp(surpriseCV, -1.f, 1.f), 0.f, 1.f);
    float foodFinal = std::clamp(food + std::clamp(foodCV, -1.f, 1.f), 0.f, 1.f);

    bool gate = inputs[TRIGGER_INPUT].getVoltage() > 1.f || params[BUTTON_PARAM].getValue() > 0.5f || inputs[BUTTON_INPUT].getVoltage() > 1.f;
    static bool prevGate = false;

    float sampleRate = args.sampleTime;

    if (gate && !prevGate) {
        t = 0.f;
        phase = 0.f;
        env = 0.f;
    }
    prevGate = gate;

    float out = 0.f;
    float shameGain = 1.f - shameFinal;
    shameGain *= shameGain;

    

    float baseFreq = 120.f - (ageFinal / 100.f) * 70.f;
    float attack = 0.08f + (1.f - urgencyFinal) * 0.15f;
    float decay = 0.5f + (1.f - urgencyFinal) * 0.5f;

    t += sampleRate;
    if (t < attack) {
        env = (t / attack) * (0.5f + urgencyFinal * 0.5f);
    } else {
        env = std::exp(-(t - attack) / decay);
    }

    ampLFOPhase += sampleRate * (0.2f + foodFinal * 0.3f);
    ampLFOPhase = normalizePhase(ampLFOPhase);
    ampLFO = 0.7f + 0.3f * fastSin(TWO_PI * ampLFOPhase + randomUniform() * 0.2f);

    pitchLFOPhase += sampleRate * (0.12f + retentionFinal * 0.18f);
    pitchLFOPhase = normalizePhase(pitchLFOPhase);
    pitchLFO = 1.f + 0.08f * fastSin(TWO_PI * pitchLFOPhase + randomUniform() * 0.3f);

    float wideLFO = fastSin(TWO_PI * t * (0.3f + retentionFinal * 2.0f));
    float narrowLFO = fastSin(TWO_PI * t * (3.f + foodFinal * 10.f));
    float pitch = baseFreq * pitchLFO * (1.f + wideLFO * retentionFinal * 0.25f + narrowLFO * foodFinal * 0.08f);
    phase += pitch * sampleRate;
    phase = normalizePhase(phase);

    float core = fastTanh(fastSin(TWO_PI * phase));

    float white = (randomUniform() * 2.f - 1.f);
    brownNoise = (brownNoise + 0.02f * white) * 0.98f;
    float noise = brownNoise * (0.18f + surpriseFinal * 0.7f);

    float wet = fastSin(TWO_PI * phase * (1.5f + foodFinal * 2.5f)) * (0.12f + foodFinal * 0.6f);

    if (bubbleEnv < 0.001f && randomUniform() < (0.008f + foodFinal * 0.03f + surpriseFinal * 0.01f)) {
        bubbleEnv = 1.f;
        bubblePhase = 0.f;
    }
    if (bubbleEnv > 0.001f) {
        bubblePhase += sampleRate * (8.f + randomUniform() * 8.f);
        float bubble = fastSin(TWO_PI * bubblePhase) * bubbleEnv * (0.2f + foodFinal * 0.5f);
        noise += bubble;
        bubbleEnv *= 0.93f - 0.04f * surpriseFinal;
    }

    out = (core + noise + wet) * env * shameGain * ampLFO * 5.f;

    outputs[AUDIO_OUTPUT].setVoltage(out);
}

void FartSynthson::readKnobs(float &age, float &urgency, float &retention, float &shame, float &surprise, float &food) {
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

void FartSynthson::readInputs(float &ageCV, float &urgencyCV, float &retentionCV, float &shameCV, float &surpriseCV, float &foodCV) {
    if (inputs[AGE_CV_INPUT].isConnected()) {
        float v = inputs[AGE_CV_INPUT].getVoltage();
        float mapped = std::clamp(((v + 10.f) / 20.f) * 100.f, 0.f, 100.f);
        getParamQuantity(AGE_PARAM)->setDisplayValue(mapped);
        ageCV = (mapped - params[AGE_PARAM].getValue()) / 100.f;
    } else {
        ageCV = 0.f;
    }
    if (inputs[URGENCY_CV_INPUT].isConnected()) {
        float v = inputs[URGENCY_CV_INPUT].getVoltage();
        float mapped = std::clamp((v + 10.f) / 20.f, 0.f, 1.f);
        getParamQuantity(URGENCY_PARAM)->setDisplayValue(mapped);
        urgencyCV = mapped - params[URGENCY_PARAM].getValue();
    } else {
        urgencyCV = 0.f;
    }
    if (inputs[RETENTION_CV_INPUT].isConnected()) {
        float v = inputs[RETENTION_CV_INPUT].getVoltage();
        float mapped = std::clamp((v + 10.f) / 20.f, 0.f, 1.f);
        getParamQuantity(RETENTION_PARAM)->setDisplayValue(mapped);
        retentionCV = mapped - params[RETENTION_PARAM].getValue();
    } else {
        retentionCV = 0.f;
    }
    if (inputs[SHAME_CV_INPUT].isConnected()) {
        float v = inputs[SHAME_CV_INPUT].getVoltage();
        float mapped = std::clamp((v + 10.f) / 20.f, 0.f, 1.f);
        getParamQuantity(SHAME_PARAM)->setDisplayValue(mapped);
        shameCV = mapped - params[SHAME_PARAM].getValue();
    } else {
        shameCV = 0.f;
    }
    if (inputs[SURPRISE_CV_INPUT].isConnected()) {
        float v = inputs[SURPRISE_CV_INPUT].getVoltage();
        float mapped = std::clamp((v + 10.f) / 20.f, 0.f, 1.f);
        getParamQuantity(SURPRISE_PARAM)->setDisplayValue(mapped);
        surpriseCV = mapped - params[SURPRISE_PARAM].getValue();
    } else {
        surpriseCV = 0.f;
    }
    if (inputs[FOOD_CV_INPUT].isConnected()) {
        float v = inputs[FOOD_CV_INPUT].getVoltage();
        float mapped = std::clamp((v + 10.f) / 20.f, 0.f, 1.f);
        getParamQuantity(FOOD_PARAM)->setDisplayValue(mapped);
        foodCV = mapped - params[FOOD_PARAM].getValue();
    } else {
        foodCV = 0.f;
    }
}