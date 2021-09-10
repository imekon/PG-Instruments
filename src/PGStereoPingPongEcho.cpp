#include "Template.hpp"

#define HALF_BUFFER_SIZE 65536
#define BUFFER_SIZE HALF_BUFFER_SIZE * 2
#define BUFFER_MASK (BUFFER_SIZE - 1)

struct PGStereoPingPongEcho : Module
{
    enum ParamIds
    {
        TIME_PARAM,
        FEEDBACK_PARAM,
        NUM_PARAMS
    };
    
    enum InputIds
    {
        INPUT,
        NUM_INPUTS
    };
    
    enum OutputIds
    {
        LEFT_OUTPUT,
        RIGHT_OUTPUT,
        NUM_OUTPUTS
    };

    int reader;
    int writer;
    int writer2;
    int offset;
    float buffer[BUFFER_SIZE];
    
    PGStereoPingPongEcho() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
        configParam(TIME_PARAM, 0.0f, 1.0f, 0.5f);
        configParam(FEEDBACK_PARAM, 0.0f, 1.0f, 0.5f);

        reader = 0;
        offset = BUFFER_SIZE >> 1;
        writer = offset;
        writer2 = BUFFER_SIZE - 1;
        
        for(int i = 0; i < BUFFER_SIZE; i++)
        {
            buffer[i] = 0.0f;
        }
    }
    
    void process(const ProcessArgs& args) override
    {
        int timeParam = (int)(params[TIME_PARAM].getValue() * BUFFER_SIZE);
        
        if (timeParam != offset)
        {
            offset = timeParam;
            writer = (reader - offset) & BUFFER_MASK;
            writer2 = (reader - offset * 2 - 1) & BUFFER_MASK;
        }
        
        float input;

        input = inputs[INPUT].getVoltage();
        outputs[LEFT_OUTPUT].setVoltage(input * 0.7f + buffer[reader] + buffer[writer]);
        outputs[RIGHT_OUTPUT].setVoltage(input * 0.7f + buffer[reader] + buffer[writer2]);
        
        buffer[writer] = input + buffer[writer] * params[FEEDBACK_PARAM].getValue();
        buffer[writer2] = input + buffer[writer2] * params[FEEDBACK_PARAM].getValue() / 2.0f;
        
        reader++;
        writer++;
        writer2++;
        
        reader &= BUFFER_MASK;
        writer &= BUFFER_MASK;
        writer2 &= BUFFER_MASK;
    }
};

struct PGStereoPingPongEchoWidget : ModuleWidget
{
    PGStereoPingPongEchoWidget(PGStereoPingPongEcho *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PGStereoPingPongEcho.svg")));
        
  		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 365)));

        addInput(createInput<PJ301MPort>(Vec(30, 100), module, PGStereoPingPongEcho::INPUT));
        
        addParam(createParam<RoundBlackKnob>(Vec(70, 100), module, PGStereoPingPongEcho::TIME_PARAM));
        addParam(createParam<RoundBlackKnob>(Vec(110, 100), module, PGStereoPingPongEcho::FEEDBACK_PARAM));
        
        addOutput(createOutput<PJ301MPort>(Vec(150, 100), module, PGStereoPingPongEcho::LEFT_OUTPUT));
        addOutput(createOutput<PJ301MPort>(Vec(150, 140), module, PGStereoPingPongEcho::RIGHT_OUTPUT));
    }
};

Model *modelPGStereoPingPongEcho = createModel<PGStereoPingPongEcho, PGStereoPingPongEchoWidget>("PGStereoPingPongEcho");
