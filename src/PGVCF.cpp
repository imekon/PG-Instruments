#include "Template.hpp"

struct PGVCF : Module
{
    enum ParamIds
    {
        FREQUENCY_PARAM,
        RESONANCE_PARAM,
        NUM_PARAMS
    };
    
    enum InputIds
    {
        INPUT,
        CUTOFF,
        NUM_INPUTS
    };
    
    enum OutputIds
    {
        LOWPASS_OUTPUT,
        BANDPASS_OUTPUT,
        HIGHPASS_OUTPUT,
        NUM_OUTPUTS
    };
    
    float buf0;
    float buf1;
    float buf2;
    float buf3;
    float feedback;
    
    PGVCF() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
        configParam(FREQUENCY_PARAM, 0.0f, 1.0f, 0.5f);
        configParam(RESONANCE_PARAM, 0.0f, 0.99f, 0.5f);

        buf0 = buf1 = buf2 = buf3 = 0.0f;
    }
    
    void process(const ProcessArgs& args) override
    {
        calculateFeedback();
        
        float calcCutoff = getCalculatedCutoff();
        
        buf0 += calcCutoff * (inputs[INPUT].getVoltage() - buf0 + feedback * (buf0 - buf1));
        buf1 += calcCutoff * (buf0 - buf1);
        buf2 += calcCutoff * (buf1 - buf2);
        buf3 += calcCutoff * (buf2 - buf3);
        
        outputs[LOWPASS_OUTPUT].setVoltage(buf3);
        outputs[BANDPASS_OUTPUT].setVoltage(buf0 - buf3);
        outputs[HIGHPASS_OUTPUT].setVoltage(inputs[INPUT].getVoltage() - buf3);
    }
    
    void calculateFeedback()
    {
        feedback = params[RESONANCE_PARAM].getValue() + params[RESONANCE_PARAM].getValue() / (1.0f - getCalculatedCutoff());
    }
    
    float getCalculatedCutoff()
    {
        return fmax(fmin(params[FREQUENCY_PARAM].getValue() + inputs[CUTOFF].getVoltage(), 0.99f), 0.01f);
    }
};

struct PGVCFWidget : ModuleWidget
{
    PGVCFWidget(PGVCF *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PGVCF.svg")));
        
  		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 365)));

        addInput(createInput<PJ301MPort>(Vec(30, 100), module, PGVCF::INPUT));
        addInput(createInput<PJ301MPort>(Vec(30, 140), module, PGVCF::CUTOFF));
        addParam(createParam<RoundBlackKnob>(Vec(70, 100), module, PGVCF::FREQUENCY_PARAM));
        addParam(createParam<RoundBlackKnob>(Vec(110, 100), module, PGVCF::RESONANCE_PARAM));
        addOutput(createOutput<PJ301MPort>(Vec(150, 100), module, PGVCF::LOWPASS_OUTPUT));
        addOutput(createOutput<PJ301MPort>(Vec(150, 140), module, PGVCF::BANDPASS_OUTPUT));
        addOutput(createOutput<PJ301MPort>(Vec(150, 180), module, PGVCF::HIGHPASS_OUTPUT));
    }
};

Model *modelPGVCF = createModel<PGVCF, PGVCFWidget>("PGVCF");
