#include "Template.hpp"

#define BUFFER_SIZE 65536 * 2
#define BUFFER_MASK (BUFFER_SIZE - 1)

struct PGEcho : Module
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
        OUTPUT,
        NUM_OUTPUTS
    };

    int reader;
    int writer;
    int offset;
    float buffer[BUFFER_SIZE];
    
    PGEcho() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);

        configParam(TIME_PARAM, 0.0f, 1.0f, 0.5f);
        configParam(FEEDBACK_PARAM, 0.0f, 1.0f, 0.5f);

        reader = 0;
        offset = BUFFER_SIZE >> 1;
        writer = offset;
        
        for(int i = 0; i < BUFFER_SIZE; i++)
            buffer[i] = 0.0f;
    }
    
    void process(const ProcessArgs& args) override
    {
        int echoOffset = (int)(params[TIME_PARAM].getValue() * BUFFER_SIZE);
        
        if (echoOffset != offset)
        {
            offset = echoOffset;
            writer = (reader - offset) & BUFFER_MASK;
        }
        
        float input = inputs[INPUT].getVoltage();

        outputs[OUTPUT].setVoltage(input + buffer[reader]);
        buffer[writer] = input + buffer[writer] * params[FEEDBACK_PARAM].getValue();
        
        reader++;
        writer++;
        
        reader &= BUFFER_MASK;
        writer &= BUFFER_MASK;
    }
};

struct PGEchoWidget : ModuleWidget
{
    PGEchoWidget(PGEcho *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PGEcho.svg")));
        
  		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 365)));

        addInput(createInput<PJ301MPort>(Vec(30, 100), module, PGEcho::INPUT));
        addParam(createParam<RoundBlackKnob>(Vec(70, 100), module, PGEcho::TIME_PARAM));
        addParam(createParam<RoundBlackKnob>(Vec(110, 100), module, PGEcho::FEEDBACK_PARAM));
        addOutput(createOutput<PJ301MPort>(Vec(150, 100), module, PGEcho::OUTPUT));
    }
};

Model *modelPGEcho = createModel<PGEcho, PGEchoWidget>("PGEcho");
