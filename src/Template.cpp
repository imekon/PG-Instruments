#include "Template.hpp"


Plugin *plugin;


void init(Plugin *p) 
{
	plugin = p;
	p->slug = TOSTRING(SLUG);
	p->version = TOSTRING(VERSION);

	// Add all Models defined throughout the plugin
	p->addModel(modelPGSEQ3);
    p->addModel(modelPGPanner);
    p->addModel(modelPGQuadPanner);
    p->addModel(modelPGOctPanner);
    p->addModel(modelPGVCF);
    p->addModel(modelPGStereoVCF);
    p->addModel(modelPGEcho);
    p->addModel(modelPGStereoEcho);
    p->addModel(modelPGStereoPingPongEcho);

	// Any other plugin initialization may go here.
	// As an alternative, consider lazy-loading assets and lookup tables when your module is created to reduce startup times of Rack.
}
