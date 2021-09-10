#include "Template.hpp"

#define NUM_PANNERS 4
#define LEFT_MARGIN 20
#define TOP_MARGIN 100
#define SPACING 36

struct PGQuadPanner : Module
{
    enum ParamIds
    {
        ENUMS(LEVEL_PARAM, NUM_PANNERS),
        ENUMS(PAN_PARAM, NUM_PANNERS),
        NUM_PARAMS
    };
    
    enum InputIds
    {
        ENUMS(INPUT, NUM_PANNERS),
        NUM_INPUTS
    };
    
    enum OutputIds
    {
        LEFT_OUTPUT,
        RIGHT_OUTPUT,
        NUM_OUTPUTS
    };
    
    //float panning[NUM_PANNERS] = { 0.5f, 0.5f, 0.5f, 0.5f };
    
    PGQuadPanner() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
        for(int i = 0; i < NUM_PANNERS; i++)
        {
            configParam(PAN_PARAM + i, 0.0f, 1.0, 0.5f);
            configParam(LEVEL_PARAM + i, 0.0f, 1.0f, 0.7f);
        }
    }
    
    void process(const ProcessArgs& args) override
    {
        float left = 0.0f;
        float right = 0.0f;
        
        for(int i = 0; i < NUM_PANNERS; i++)
        {
            float input = inputs[INPUT + i].getVoltage();
            float panning = params[PAN_PARAM + i].getValue();
            float level = params[LEVEL_PARAM + i].getValue();
            
            float l, r;
            
            pan(input, panning, level, l, r);
            
            left += l;
            right += r;
        }
        
        outputs[LEFT_OUTPUT].setVoltage(left);
        outputs[RIGHT_OUTPUT].setVoltage(right);
    }
    
    void pan(float input, float panning, float level, float &left, float &right)
    {
        left = cosf(panning * M_PI / 2.0f) * input * level;
        right = sinf(panning * M_PI / 2.0f) * input * level;
    }
};

struct PGQuadPannerWidget : ModuleWidget
{
    PGQuadPannerWidget(PGQuadPanner *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PGQuadPanner.svg")));
        
  		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 365)));

        for(int i = 0; i < NUM_PANNERS; i++)
        {
            addInput(createInput<PJ301MPort>(Vec(LEFT_MARGIN, TOP_MARGIN + SPACING * i), module, PGQuadPanner::INPUT + i));
            addParam(createParam<RoundBlackKnob>(Vec(LEFT_MARGIN + 40, TOP_MARGIN + SPACING * i), module, PGQuadPanner::PAN_PARAM + i));
            addParam(createParam<RoundBlackKnob>(Vec(LEFT_MARGIN + 80, TOP_MARGIN + SPACING * i), module, PGQuadPanner::LEVEL_PARAM + i));
        }
        
        addOutput(createOutput<PJ301MPort>(Vec(LEFT_MARGIN + 120, TOP_MARGIN + SPACING), module, PGQuadPanner::LEFT_OUTPUT));
        addOutput(createOutput<PJ301MPort>(Vec(LEFT_MARGIN + 120, TOP_MARGIN + SPACING * 2), module, PGQuadPanner::RIGHT_OUTPUT));
    }
};

Model *modelPGQuadPanner = createModel<PGQuadPanner, PGQuadPannerWidget>("PGQuadPanner");
