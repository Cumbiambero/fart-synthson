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


void FartSimpson::process(const ProcessArgs& args) {
  static Voice voice;
  static int frame = 0;
  static float age = 50.f, urgency = 0.5f, retention = 0.5f, shame = 0.f, surprise = 0.5f, food = 0.5f;

  readParams(frame, age, urgency, retention, shame, surprise, food);

  bool gate = inputs[TRIGGER_INPUT].getVoltage() > 1.f || params[BUTTON_PARAM].getValue() > 0.5f || inputs[BUTTON_INPUT].getVoltage() > 1.f;
  static bool prevGate = false;

  float sampleRate = args.sampleRate;
  float dt = args.sampleTime;

  if (gate && !prevGate) {
    voice.active = true;
    voice.t = 0.f;
    voice.phase = 0.f;
    voice.env = 0.f;
  }
  prevGate = gate;

  float out = 0.f;
  float shameGain = 1.f - shame;
  shameGain *= shameGain;

  if (voice.active) {
    voice.t += dt;

    float baseFreq = 120.f - (age / 100.f) * 70.f;
    float attack = 0.02f + (1.f - urgency) * 0.08f;
    float decay = 0.25f + (1.f - urgency) * 0.3f;
    float env = 0.f;
    if (voice.t < attack) {
      env = (voice.t / attack) * (0.7f + urgency * 0.6f);
    } else {
      env = std::exp(-(voice.t - attack) / decay);
    }
    voice.env = env;

    float wideLFO = std::sin(2.f * (float)M_PI * voice.t * (0.5f + retention * 4.0f));
    float narrowLFO = std::sin(2.f * (float)M_PI * voice.t * (8.f + food * 32.f));

    float pitch = baseFreq * (1.f + wideLFO * retention * 0.2f + narrowLFO * food * 0.05f);
    voice.phase += pitch * dt;
    if (voice.phase > 1.f) voice.phase -= 1.f;

    float core = std::tanh(std::sin(2.f * (float)M_PI * voice.phase));
    float noise = (randomUniform() * 2.f - 1.f) * (0.1f + surprise * 0.6f);
    float wet = std::sin(2.f * (float)M_PI * voice.phase * (2.5f + food * 2.f)) * (0.1f + food * 0.5f);

    out = (core + noise + wet) * env * shameGain * 5.f;

    if (env < 0.001f) {
      voice.active = false;
    }
  }

  outputs[AUDIO_OUTPUT].setVoltage(out);
}

void FartSimpson::readParams(int &frame, float &age, float &urgency, float &retention, float &shame, float &surprise, float &food)
{
if ((frame++ & 63) == 0) {
    age = std::clamp(params[AGE_PARAM].getValue() + std::clamp(inputs[AGE_CV_INPUT].getVoltage() / 10.f, -1.f, 1.f) * 100.f, 0.f, 100.f);
    urgency = std::clamp(params[URGENCY_PARAM].getValue() + std::clamp(inputs[URGENCY_CV_INPUT].getVoltage() / 10.f, -1.f, 1.f), 0.f, 1.f);
    retention = std::clamp(params[RETENTION_PARAM].getValue() + std::clamp(inputs[RETENTION_CV_INPUT].getVoltage() / 10.f, -1.f, 1.f), 0.f, 1.f);
    shame = std::clamp(params[SHAME_PARAM].getValue() + std::clamp(inputs[SHAME_CV_INPUT].getVoltage() / 10.f, -1.f, 1.f), 0.f, 1.f);
    surprise = std::clamp(params[SURPRISE_PARAM].getValue() + std::clamp(inputs[SURPRISE_CV_INPUT].getVoltage() / 10.f, -1.f, 1.f), 0.f, 1.f);
    food = std::clamp(params[FOOD_PARAM].getValue() + std::clamp(inputs[FOOD_CV_INPUT].getVoltage() / 10.f, -1.f, 1.f), 0.f, 1.f);
  }
}
