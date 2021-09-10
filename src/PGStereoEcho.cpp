#include "Template.hpp"

#define HALF_BUFFER_SIZE 65536
#define BUFFER_SIZE HALF_BUFFER_SIZE * 2
#define BUFFER_MASK (BUFFER_SIZE - 1)

struct PGStereoEcho : Module
{
    enum ParamIds
    {
        TIME_PARAM,
        OFFSET_PARAM,
        FEEDBACK_PARAM,
        NUM_PARAMS
    };
    
    enum InputIds
    {
        LEFT_INPUT,
        RIGHT_INPUT,
        NUM_INPUTS
    };
    
    enum OutputIds
    {
        LEFT_OUTPUT,
        RIGHT_OUTPUT,
        NUM_OUTPUTS
    };

    int reader;
    int leftWriter;
    int rightWriter;
    int offset;
    int rightOffset;
    float leftBuffer[BUFFER_SIZE];
    float rightBuffer[BUFFER_SIZE];
    
    PGStereoEcho() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
        configParam(TIME_PARAM, 0.0f, 1.0f, 0.5f);
        configParam(FEEDBACK_PARAM, 0.0f, 1.0f, 0.5f);
        configParam(OFFSET_PARAM, 0.0f, 1.0f, 0.0f);

        reader = 0;
        offset = BUFFER_SIZE >> 1;
        rightOffset = 0;
        leftWriter = offset;
        rightWriter = offset;
        
        for(int i = 0; i < BUFFER_SIZE; i++)
        {
            leftBuffer[i] = 0.0f;
            rightBuffer[i] = 0.0f;
        }
    }
    
    void process(const ProcessArgs& args) override
    {
        int timeParam = (int)(params[TIME_PARAM].getValue() * BUFFER_SIZE);
        int offsetParam = (int)(params[OFFSET_PARAM].getValue() * HALF_BUFFER_SIZE);
        
        if (timeParam != offset || offsetParam != rightOffset)
        {
            offset = timeParam;
            rightOffset = offsetParam;
            leftWriter = (reader - offset) & BUFFER_MASK;
            rightWriter = (reader - offset - rightOffset) & BUFFER_MASK;
        }
        
        float input;

        input = inputs[LEFT_INPUT].getVoltage();
        outputs[LEFT_OUTPUT].setVoltage(input + leftBuffer[reader]);
        leftBuffer[leftWriter] = input + leftBuffer[leftWriter] * params[FEEDBACK_PARAM].getValue();
        
        input = inputs[RIGHT_INPUT].getVoltage();
        outputs[RIGHT_OUTPUT].setVoltage(input + rightBuffer[reader]);
        rightBuffer[rightWriter] = input + rightBuffer[rightWriter] * params[FEEDBACK_PARAM].getValue();
        
        reader++;
        leftWriter++;
        rightWriter++;
        
        reader &= BUFFER_MASK;
        leftWriter &= BUFFER_MASK;
        rightWriter &= BUFFER_MASK;
    }
};

struct PGStereoEchoWidget : ModuleWidget
{
    PGStereoEchoWidget(PGStereoEcho *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PGStereoEcho.svg")));
        
  		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 365)));

        addInput(createInput<PJ301MPort>(Vec(30, 100), module, PGStereoEcho::LEFT_INPUT));
        addInput(createInput<PJ301MPort>(Vec(30, 140), module, PGStereoEcho::RIGHT_INPUT));
        
        addParam(createParam<RoundBlackKnob>(Vec(70, 100), module, PGStereoEcho::TIME_PARAM));
        addParam(createParam<RoundBlackKnob>(Vec(110, 100), module, PGStereoEcho::FEEDBACK_PARAM));
        addParam(createParam<RoundBlackKnob>(Vec(70, 140), module, PGStereoEcho::OFFSET_PARAM));
        
        addOutput(createOutput<PJ301MPort>(Vec(150, 100), module, PGStereoEcho::LEFT_OUTPUT));
        addOutput(createOutput<PJ301MPort>(Vec(150, 140), module, PGStereoEcho::RIGHT_OUTPUT));
    }
};

Model *modelPGStereoEcho = createModel<PGStereoEcho, PGStereoEchoWidget>("PGStereoEcho");
