#include "Template.hpp"


Plugin *pluginInstance;


void init(Plugin *p) 
{
	pluginInstance = p;

	// Add all Models defined throughout the pluginInstance
    p->addModel(modelPGEcho);
    p->addModel(modelPGOctPanner);
    p->addModel(modelPGPanner);
    p->addModel(modelPGQuadPanner);
    p->addModel(modelPGSEQ3);
    p->addModel(modelPGStereoEcho);
    p->addModel(modelPGStereoPingPongEcho);
    p->addModel(modelPGStereoVCF);
    p->addModel(modelPGVCF);

	// Any other pluginInstance initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
