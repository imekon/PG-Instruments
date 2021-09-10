#include "Template.hpp"

struct PGPanner : Module
{
    enum ParamIds
    {
        PAN_PARAM,
        NUM_PARAMS
    };
    
    enum InputIds
    {
        INPUT,
        PAN_INPUT,
        NUM_INPUTS
    };
    
    enum OutputIds
    {
        LEFT_OUTPUT,
        RIGHT_OUTPUT,
        NUM_OUTPUTS
    };
    
    enum LightIds
    {
        RUNNING_LIGHT,
        NUM_LIGHTS
    };
    
    float panning = 0.5f;
    
    PGPanner() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
        configParam(PAN_PARAM, 0.0f, 1.0, 0.5f);
        onReset();
    }
    
    void onReset() override
    {
    }
    
    void onRandomize() override
    {
        
    }
    
    void process(const ProcessArgs& args) override
    {
        panning = params[PAN_PARAM].getValue();
        
        float mono = inputs[INPUT].getVoltage();
        float panInput = inputs[PAN_INPUT].getVoltage();
        float pan = panning + panInput;
        
        float leftGain = cosf(pan * M_PI / 2.0f) * mono;
        float rightGain = sinf(pan * M_PI / 2.0f) * mono;
        
        outputs[LEFT_OUTPUT].setVoltage(leftGain);
        outputs[RIGHT_OUTPUT].setVoltage(rightGain);
    }
};

struct PGPannerWidget : ModuleWidget
{
    PGPannerWidget(PGPanner *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PGPanner.svg")));
        
  		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 365)));

        addParam(createParam<RoundLargeBlackKnob>(Vec(20, 40), module, PGPanner::PAN_PARAM));
        
        addInput(createInput<PJ301MPort>(Vec(26, 100), module, PGPanner::INPUT));
        addInput(createInput<PJ301MPort>(Vec(26, 160), module, PGPanner::PAN_INPUT));
        addOutput(createOutput<PJ301MPort>(Vec(12, 220), module, PGPanner::LEFT_OUTPUT));
        addOutput(createOutput<PJ301MPort>(Vec(42, 220), module, PGPanner::RIGHT_OUTPUT));
    }
};

Model *modelPGPanner = createModel<PGPanner, PGPannerWidget>("PGPanner");
