#include "Template.hpp"

#define NUM_SEQ_STEPS 16

struct PGSEQ3 : Module 
{
	enum ParamIds 
    {
		CLOCK_PARAM,
		RUN_PARAM,
		RESET_PARAM,
		STEPS_PARAM,
		ENUMS(ROW1_PARAM, NUM_SEQ_STEPS),
		ENUMS(ROW2_PARAM, NUM_SEQ_STEPS),
		ENUMS(ROW3_PARAM, NUM_SEQ_STEPS),
		ENUMS(GATE_PARAM, NUM_SEQ_STEPS),
		NUM_PARAMS
	};
	
    enum InputIds 
    {
		CLOCK_INPUT,
		EXT_CLOCK_INPUT,
		RESET_INPUT,
		STEPS_INPUT,
		NUM_INPUTS
	};

	enum OutputIds 
    {
		GATES_OUTPUT,
		ROW1_OUTPUT,
		ROW2_OUTPUT,
		ROW3_OUTPUT,
		ENUMS(GATE_OUTPUT, NUM_SEQ_STEPS),
		NUM_OUTPUTS
	};

	enum LightIds 
    {
		RUNNING_LIGHT,
		RESET_LIGHT,
		GATES_LIGHT,
		ENUMS(ROW_LIGHTS, 3),
		ENUMS(GATE_LIGHTS, NUM_SEQ_STEPS),
		NUM_LIGHTS
	};

	bool running = true;
	dsp::SchmittTrigger clockTrigger;
	dsp::SchmittTrigger runningTrigger;
	dsp::SchmittTrigger resetTrigger;
	dsp::SchmittTrigger gateTriggers[NUM_SEQ_STEPS];
	/** Phase of internal LFO */
	float phase = 0.f;
	int index = 0;
	bool gates[NUM_SEQ_STEPS] = {};

	PGSEQ3() {
    	config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(CLOCK_PARAM, -2.0f, 6.0f, 2.0f);
		configParam(RUN_PARAM, 0.0f, 1.0f, 0.0f);
		configParam(RESET_PARAM, 0.0f, 1.0f, 0.0f);
		configParam(STEPS_PARAM, 1.0f, (float)NUM_SEQ_STEPS, (float)NUM_SEQ_STEPS);
		for (int i = 0; i < NUM_SEQ_STEPS; i++) 
        {
        	configParam(ROW1_PARAM + i, 0.0f, 10.0f, 1.0f);
        	configParam(ROW2_PARAM + i, 0.0f, 10.0f, 0.0f);
        	configParam(ROW3_PARAM + i, 0.0f, 10.0f, 0.0f);
        	configParam(GATE_PARAM + i, 0.0f, 1.0f, 0.0f);
        }
		onReset();
	}

	void onReset() override 
    {
		for (int i = 0; i < NUM_SEQ_STEPS; i++) 
        {
			gates[i] = true;
		}
	}

	void onRandomize() override 
    {
		for (int i = 0; i < NUM_SEQ_STEPS; i++)
        {
			gates[i] = (random::uniform() > 0.5f);
		}
	}

	json_t *dataToJson() override 
    {
		json_t *rootJ = json_object();

		// running
		json_object_set_new(rootJ, "running", json_boolean(running));

		// gates
		json_t *gatesJ = json_array();
		for (int i = 0; i < NUM_SEQ_STEPS; i++) 
        {
			json_array_insert_new(gatesJ, i, json_integer((int) gates[i]));
		}
		json_object_set_new(rootJ, "gates", gatesJ);

		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override 
    {
		// running
		json_t *runningJ = json_object_get(rootJ, "running");
		if (runningJ)
			running = json_is_true(runningJ);

		// gates
		json_t *gatesJ = json_object_get(rootJ, "gates");
		if (gatesJ) 
        {
			for (int i = 0; i < NUM_SEQ_STEPS; i++) 
            {
				json_t *gateJ = json_array_get(gatesJ, i);
				if (gateJ)
					gates[i] = !!json_integer_value(gateJ);
			}
		}
	}

	void setIndex(int index) 
    {
		int numSteps = (int) clamp(roundf(params[STEPS_PARAM].getValue() + inputs[STEPS_INPUT].getVoltage()), 1.0f, (float)NUM_SEQ_STEPS);
		phase = 0.f;
		this->index = index;
		if (this->index >= numSteps)
			this->index = 0;
	}

	void process(const ProcessArgs& args) override 
    {
		// Run
		if (runningTrigger.process(params[RUN_PARAM].getValue()))
        {
			running = !running;
		}

		bool gateIn = false;
		if (running) 
        {
			if (inputs[EXT_CLOCK_INPUT].isConnected()) 
            {
				// External clock
				if (clockTrigger.process(inputs[EXT_CLOCK_INPUT].getVoltage())) 
                {
					setIndex(index + 1);
				}
				gateIn = clockTrigger.isHigh();
			}
			else
            {
				// Internal clock
				float clockTime = powf(2.0f, params[CLOCK_PARAM].getValue() + inputs[CLOCK_INPUT].getVoltage());
				phase += clockTime * args.sampleTime;
				if (phase >= 1.0f) 
                {
					setIndex(index + 1);
				}
				gateIn = (phase < 0.5f);
			}
		}

		// Reset
		if (resetTrigger.process(params[RESET_PARAM].getValue() + inputs[RESET_INPUT].getVoltage())) 
        {
			setIndex(0);
		}

		// Gate buttons
		for (int i = 0; i < NUM_SEQ_STEPS; i++) 
        {
			if (gateTriggers[i].process(params[GATE_PARAM + i].getValue())) 
            {
				gates[i] = !gates[i];
			}
			outputs[GATE_OUTPUT + i].setVoltage((running && gateIn && i == index && gates[i]) ? 10.0f : 0.0f);
			lights[GATE_LIGHTS + i].setSmoothBrightness((gateIn && i == index) ? (gates[i] ? 1.f : 0.33) : (gates[i] ? 0.66 : 0.0), args.sampleTime);
		}

		// Outputs
		outputs[ROW1_OUTPUT].setVoltage(params[ROW1_PARAM + index].getValue());
		outputs[ROW2_OUTPUT].setVoltage(params[ROW2_PARAM + index].getValue());
		outputs[ROW3_OUTPUT].setVoltage(params[ROW3_PARAM + index].getValue());
		outputs[GATES_OUTPUT].setVoltage((gateIn && gates[index]) ? 10.0f : 0.0f);
		lights[RUNNING_LIGHT].value = (running);
		lights[RESET_LIGHT].setSmoothBrightness(resetTrigger.isHigh(), args.sampleTime);
		lights[GATES_LIGHT].setSmoothBrightness(gateIn, args.sampleTime);
		lights[ROW_LIGHTS].value = outputs[ROW1_OUTPUT].value / 10.0f;
		lights[ROW_LIGHTS + 1].value = outputs[ROW2_OUTPUT].value / 10.0f;
		lights[ROW_LIGHTS + 2].value = outputs[ROW3_OUTPUT].value / 10.0f;
	}
};


struct PGSEQ3Widget : ModuleWidget 
{
	PGSEQ3Widget(PGSEQ3 *module) {
    	setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/PGSEQ3.svg")));

		addChild(createWidget<ScrewSilver>(Vec(15, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 0)));
		addChild(createWidget<ScrewSilver>(Vec(15, 365)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x-30, 365)));

		addParam(createParam<RoundBlackKnob>(Vec(18, 56), module, PGSEQ3::CLOCK_PARAM));
		addParam(createParam<LEDButton>(Vec(60, 61-1), module, PGSEQ3::RUN_PARAM));
		addParam(createParam<LEDButton>(Vec(99, 61-1), module, PGSEQ3::RESET_PARAM));
		addParam(createParam<RoundBlackSnapKnob>(Vec(132, 56), module, PGSEQ3::STEPS_PARAM));

		addChild(createLight<MediumLight<GreenLight>>(Vec(64.4f, 64.4f), module, PGSEQ3::RUNNING_LIGHT));
		addChild(createLight<MediumLight<GreenLight>>(Vec(103.4f, 64.4f), module, PGSEQ3::RESET_LIGHT));
		addChild(createLight<MediumLight<GreenLight>>(Vec(179.4f, 64.4f), module, PGSEQ3::GATES_LIGHT));
		addChild(createLight<MediumLight<GreenLight>>(Vec(218.4f, 64.4f), module, PGSEQ3::ROW_LIGHTS));
		addChild(createLight<MediumLight<GreenLight>>(Vec(256.4f, 64.4f), module, PGSEQ3::ROW_LIGHTS + 1));
		addChild(createLight<MediumLight<GreenLight>>(Vec(295.4f, 64.4f), module, PGSEQ3::ROW_LIGHTS + 2));

		static const float portX[NUM_SEQ_STEPS] = 
        {
            20, 58, 96, 135, 
            173, 212, 250, 288,
            327, 366, 404, 442,
            480, 519, 557, 596
        };
        
		addInput(createInput<PJ301MPort>(Vec(portX[0]-1, 98), module, PGSEQ3::CLOCK_INPUT));
		addInput(createInput<PJ301MPort>(Vec(portX[1]-1, 98), module, PGSEQ3::EXT_CLOCK_INPUT));
		addInput(createInput<PJ301MPort>(Vec(portX[2]-1, 98), module, PGSEQ3::RESET_INPUT));
		addInput(createInput<PJ301MPort>(Vec(portX[3]-1, 98), module, PGSEQ3::STEPS_INPUT));
		addOutput(createOutput<PJ301MPort>(Vec(portX[4]-1, 98), module, PGSEQ3::GATES_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(portX[5]-1, 98), module, PGSEQ3::ROW1_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(portX[6]-1, 98), module, PGSEQ3::ROW2_OUTPUT));
		addOutput(createOutput<PJ301MPort>(Vec(portX[7]-1, 98), module, PGSEQ3::ROW3_OUTPUT));

		for (int i = 0; i < NUM_SEQ_STEPS; i++) 
        {
			addParam(createParam<RoundBlackKnob>(Vec(portX[i]-2, 157), module, PGSEQ3::ROW1_PARAM + i));
			addParam(createParam<RoundBlackKnob>(Vec(portX[i]-2, 198), module, PGSEQ3::ROW2_PARAM + i));
			addParam(createParam<RoundBlackKnob>(Vec(portX[i]-2, 240), module, PGSEQ3::ROW3_PARAM + i));
			addParam(createParam<LEDButton>(Vec(portX[i]+2, 278-1), module, PGSEQ3::GATE_PARAM + i));
			addChild(createLight<MediumLight<GreenLight>>(Vec(portX[i]+6.4f, 281.4f), module, PGSEQ3::GATE_LIGHTS + i));
			addOutput(createOutput<PJ301MPort>(Vec(portX[i]-1, 307), module, PGSEQ3::GATE_OUTPUT + i));
		}
	}
};

Model *modelPGSEQ3 = createModel<PGSEQ3, PGSEQ3Widget>("PGSEQ3");
