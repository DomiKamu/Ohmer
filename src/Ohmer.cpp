////////////////////////////////////////////////////////////////////////////////////////////////////
////// Ohmer Modules ///////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

Plugin *pluginInstance;

void init(rack::Plugin *p) {
	pluginInstance = p;
	p->addModel(modelKlokSpid); // KloSpid module.
	p->addModel(modelRKD); // RKD (Rotate Klok Divider) module.
	p->addModel(modelBRK); // BRK ("Break") expander module for RKD (Rotate Klok Divider).
	p->addModel(modelMetriks); // Metriks module.
	p->addModel(modelPolaritySwitch); // Polarity Switch module.
	p->addModel(modelSplitter); // Splitter 1x9 module.
	p->addModel(modelBlankPanel1); // 1 HP blank module.
	p->addModel(modelBlankPanel2); // 2 HP blank module.
	p->addModel(modelBlankPanel4); // 4 HP blank module.
	p->addModel(modelBlankPanel8); // 8 HP blank module.
	p->addModel(modelBlankPanel16); // 16 HP blank module.
	p->addModel(modelBlankPanel32); // 32 HP blank module.
}
