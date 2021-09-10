#include "Template.hpp"

struct PGStereoVCF : Module
{
    enum ParamIds
    {
        FREQUENCY_PARAM,
        RESONANCE_PARAM,
        NUM_PARAMS
    };
    
    enum InputIds
    {
        LEFT_INPUT,
        RIGHT_INPUT,
        CUTOFF,
        NUM_INPUTS
    };
    
    enum OutputIds
    {
        LEFT_LOWPASS_OUTPUT,
        RIGHT_LOWPASS_OUTPUT,
        
        LEFT_BANDPASS_OUTPUT,
        RIGHT_BANDPASS_OUTPUT,
        
        LEFT_HIGHPASS_OUTPUT,
        RIGHT_HIGHPASS_OUTPUT,

        NUM_OUTPUTS
    };
    
    float buf0[2];
    float buf1[2];
    float buf2[2];
    float buf3[2];
    float feedback;
    
    PGStereoVCF() {
        config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS);
        configParam(FREQUENCY_PARAM, 0.0f, 1.0f, 0.5f);
        configParam(RESONANCE_PARAM, 0.0f, 0.99f, 0.5f);

        for(int i = 0; i < 2; i++)
        {
            buf0[i] = buf1[i] = buf2[i] = buf3[i] = 0.0f;
        }
    }
    
    void process(const ProcessArgs& args) override
    {
        calculateFeedback();
        
        float calcCutoff = getCalculatedCutoff();
        
        for(int i = 0; i < 2; i++)
        {
            float input;
            
            if (i)
                input = inputs[RIGHT_INPUT].getVoltage();
            else
                input = inputs[LEFT_INPUT].getVoltage();
            
            buf0[i] += calcCutoff * (input - buf0[i] + feedback * (buf0[i] - buf1[i]));
            buf1[i] += calcCutoff * (buf0[i] - buf1[i]);
            buf2[i] += calcCutoff * (buf1[i] - buf2[i]);
            buf3[i] += calcCutoff * (buf2[i] - buf3[i]);
            
            if (i)
            {
                outputs[RIGHT_LOWPASS_OUTPUT].setVoltage(buf3[i]);
                outputs[RIGHT_BANDPASS_OUTPUT].setVoltage(buf0[i] - buf3[i]);
                outputs[RIGHT_HIGHPASS_OUTPUT].setVoltage(inputs[RIGHT_INPUT].getVoltage() - buf3[i]);
            }
            else
            {
                outputs[LEFT_LOWPASS_OUTPUT].setVoltage(buf3[i]);
                outputs[LEFT_BANDPASS_OUTPUT].setVoltage(buf0[i] - buf3[i]);
                outputs[LEFT_HIGHPASS_OUTPUT].setVoltage(inputs[LEFT_INPUT].getVoltage() - buf3[i]);
            }
        }
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

struct PGStereoVCFWidget : ModuleWidget
{
    PGStereoVCFWidget(PGStereoVCF *module) {
        setModule(module);
        setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PGStereoVCF.svg")));
        
  		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 365)));

        addInput(createInput<PJ301MPort>(Vec(20, 100), module, PGStereoVCF::LEFT_INPUT));
        addInput(createInput<PJ301MPort>(Vec(20, 140), module, PGStereoVCF::RIGHT_INPUT));
        
        addInput(createInput<PJ301MPort>(Vec(20, 180), module, PGStereoVCF::CUTOFF));

        addParam(createParam<RoundBlackKnob>(Vec(50, 100), module, PGStereoVCF::FREQUENCY_PARAM));
        addParam(createParam<RoundBlackKnob>(Vec(90, 100), module, PGStereoVCF::RESONANCE_PARAM));

        addOutput(createOutput<PJ301MPort>(Vec(130, 100), module, PGStereoVCF::LEFT_LOWPASS_OUTPUT));
        addOutput(createOutput<PJ301MPort>(Vec(130, 140), module, PGStereoVCF::LEFT_BANDPASS_OUTPUT));
        addOutput(createOutput<PJ301MPort>(Vec(130, 180), module, PGStereoVCF::LEFT_HIGHPASS_OUTPUT));

        addOutput(createOutput<PJ301MPort>(Vec(160, 100), module, PGStereoVCF::RIGHT_LOWPASS_OUTPUT));
        addOutput(createOutput<PJ301MPort>(Vec(160, 140), module, PGStereoVCF::RIGHT_BANDPASS_OUTPUT));
        addOutput(createOutput<PJ301MPort>(Vec(160, 180), module, PGStereoVCF::RIGHT_HIGHPASS_OUTPUT));
    }
};

Model *modelPGStereoVCF = createModel<PGStereoVCF, PGStereoVCFWidget>("PGStereoVCF");
