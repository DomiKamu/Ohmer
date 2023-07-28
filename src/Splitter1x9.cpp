////////////////////////////////////////////////////////////////////////////////////////////////////
////// Splitter 1x9                                                                              ///
////// 2 HP module, having 1 input sent "splitted" to 9 outputs, but limited voltages must stay  ///
////// in -11.7 V / +11.7 V bounds to every output ("hard clipping").                            ///
////// This module supports polyphonic cables.                                                   ///
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

struct SplitterModule : Module {

	enum ParamIds {
		NUM_PARAMS
	};

	enum InputIds {
		MAIN_INPUT,
		NUM_INPUTS
	};

	enum OutputIds {
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3,
		OUTPUT_4,
		OUTPUT_5,
		OUTPUT_6,
		OUTPUT_7,
		OUTPUT_8,
		OUTPUT_9,
		NUM_OUTPUTS
	};

	enum LightIds {
		NUM_LIGHTS
	};

	// Current selected Splitter 1x9 model (GUI theme).
	int Theme = 0; // 0 = Classic (default), 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Carbon Signature.
	int portMetal = 0; // 0 = silver connector (default), 1 = gold connector used by "Signature"-line models only.

	// Sample rate (from Rack engine).
	float sampleRate = 0.0f;

	SplitterModule() {
		// Constructor...
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configInput(MAIN_INPUT, "Signal");
		configOutput(OUTPUT_1, "1st");
		configOutput(OUTPUT_2, "2nd");
		configOutput(OUTPUT_3, "3rd");
		configOutput(OUTPUT_4, "4th");
		configOutput(OUTPUT_5, "5th");
		configOutput(OUTPUT_6, "6th");
		configOutput(OUTPUT_7, "7th");
		configOutput(OUTPUT_8, "8th");
		configOutput(OUTPUT_9, "8th");
		onSampleRateChange();
	}

	void onSampleRateChange() override {
		sampleRate = APP->engine->getSampleRate();
	}		

	void process(const ProcessArgs &args) override {
		if (inputs[MAIN_INPUT].isConnected()) {
			int nChannels = inputs[MAIN_INPUT].getChannels(); // Added for polyphonic.
			for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
				// Per output port.
				for (int c = 0;  c < nChannels; c++) {
				// then per polyphonic channel (1 channel if monophonic cable on input).
					float raw_input_voltage = inputs[MAIN_INPUT].getVoltage(c);
					float splitted_out_voltage = clamp(raw_input_voltage, -11.7f, 11.7f); // These -11.7 V / +11.7 V limits are max. possible voltage on Eurorack.
					outputs[i].setVoltage(splitted_out_voltage, c);
				}
				outputs[i].setChannels(nChannels);
			}
		}
		else {
			// If input jack isn't connected, assuming it's a monophonic module instead. Also, no voltage to output jacks, and unlit LED.
			for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
				outputs[i].setVoltage(0.0f);
				outputs[i].setChannels(1);
			}
		}
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Theme", json_integer(Theme));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		// Retrieving module theme/variation (when loading .vcv and cloning module).
		json_t *ThemeJ = json_object_get(rootJ, "Theme");
		if (ThemeJ) {
			Theme = json_integer_value(ThemeJ);
			portMetal = Theme / 3; // first three use silver (0), last three use gold (1) - the int division by 3 is useful ;)
		}
	}

};

///////////////////////////////////////////////////// CONTEXT-MENU //////////////////////////////////////////////////////

struct SplitterClassicMenu : MenuItem {
	SplitterModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 0; // Model: default Classic (beige).
		module->portMetal = 0; // Silver connectors for Classic.
	}
};

struct SplitterStageReproMenu : MenuItem {
	SplitterModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 1; // Model: Stage Repro.
		module->portMetal = 0; // Silver connectors for Stage Repro.
	}
};

struct SplitterAbsoluteNightMenu : MenuItem {
	SplitterModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 2; // Model: Absolute Night.
		module->portMetal = 0; // Silver connectors for Absolute Night.
	}
};

struct SplitterDarkSignatureMenu : MenuItem {
	SplitterModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 3; // Model: Dark Signature.
		module->portMetal = 1; // Gold connectors for Dark Signature.
	}
};

struct SplitterDeepblueSignatureMenu : MenuItem {
	SplitterModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 4; // Model: Deepblue Signature.
		module->portMetal = 1; // Gold connectors for Deepblue Signature.
	}
};

struct SplitterCarbonSignatureMenu : MenuItem {
	SplitterModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 5; // Model: Carbon Signature.
		module->portMetal = 1; // Gold connectors for Carbon Signature.
	}
};

struct SplitterSubMenuItems : MenuItem {
	SplitterModule *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		SplitterClassicMenu *splittrmenuitem1 = new SplitterClassicMenu;
		splittrmenuitem1->text = "Classic (default)";
		splittrmenuitem1->rightText = CHECKMARK(module->Theme == 0);
		splittrmenuitem1->module = module;
		menu->addChild(splittrmenuitem1);

		SplitterStageReproMenu *splittrmenuitem2 = new SplitterStageReproMenu;
		splittrmenuitem2->text = "Stage Repro";
		splittrmenuitem2->rightText = CHECKMARK(module->Theme == 1);
		splittrmenuitem2->module = module;
		menu->addChild(splittrmenuitem2);

		SplitterAbsoluteNightMenu *splittrmenuitem3 = new SplitterAbsoluteNightMenu;
		splittrmenuitem3->text = "Absolute Night";
		splittrmenuitem3->rightText = CHECKMARK(module->Theme == 2);
		splittrmenuitem3->module = module;
		menu->addChild(splittrmenuitem3);

		SplitterDarkSignatureMenu *splittrmenuitem4 = new SplitterDarkSignatureMenu;
		splittrmenuitem4->text = "Dark \"Signature\"";
		splittrmenuitem4->rightText = CHECKMARK(module->Theme == 3);
		splittrmenuitem4->module = module;
		menu->addChild(splittrmenuitem4);

		SplitterDeepblueSignatureMenu *splittrmenuitem5 = new SplitterDeepblueSignatureMenu;
		splittrmenuitem5->text = "Deepblue \"Signature\"";
		splittrmenuitem5->rightText = CHECKMARK(module->Theme == 4);
		splittrmenuitem5->module = module;
		menu->addChild(splittrmenuitem5);

		SplitterCarbonSignatureMenu *splittrmenuitem6 = new SplitterCarbonSignatureMenu;
		splittrmenuitem6->text = "Carbon \"Signature\"";
		splittrmenuitem6->rightText = CHECKMARK(module->Theme == 5);
		splittrmenuitem6->module = module;
		menu->addChild(splittrmenuitem6);

		return menu;
	}
};

///////////////////////////////////////////////// MODULE WIDGET SECTION /////////////////////////////////////////////////

struct SplitterWidget : ModuleWidget {
	// Panels (one per "Theme").
	SvgPanel *panelSplitterClassic;
	SvgPanel *panelSplitterStageRepro;
	SvgPanel *panelSplitterAbsoluteNight;
	SvgPanel *panelSplitterDarkSignature;
	SvgPanel *panelSplitterDeepBlueSignature;
	SvgPanel *panelSplitterCarbonSignature;
	// Silver Torx screws.
	SvgScrew *topLeftScrewSilver;
	SvgScrew *topRightScrewSilver;
	SvgScrew *bottomLeftScrewSilver;
	SvgScrew *bottomRightScrewSilver;
	// Gold Torx screws.
	SvgScrew *topLeftScrewGold;
	SvgScrew *topRightScrewGold;
	SvgScrew *bottomLeftScrewGold;
	SvgScrew *bottomRightScrewGold;

	SplitterWidget(SplitterModule *module) {
		setModule(module);
		box.size = Vec(2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		// Classic (default) beige panel.
		panelSplitterClassic = new SvgPanel();
		panelSplitterClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Splitter1x9_Classic.svg")));
		panelSplitterClassic->visible = true;
		addChild(panelSplitterClassic);
		// Stage Repro panel.
		panelSplitterStageRepro = new SvgPanel();
		panelSplitterStageRepro->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Splitter1x9_Stage_Repro.svg")));
		panelSplitterStageRepro->visible = false;
		addChild(panelSplitterStageRepro);
		// Absolute Night panel.
		panelSplitterAbsoluteNight = new SvgPanel();
		panelSplitterAbsoluteNight->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Splitter1x9_Absolute_Night.svg")));
		panelSplitterAbsoluteNight->visible = false;
		addChild(panelSplitterAbsoluteNight);
		// Dark Signature panel.
		panelSplitterDarkSignature = new SvgPanel();
		panelSplitterDarkSignature->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Splitter1x9_Dark_Signature.svg")));
		panelSplitterDarkSignature->visible = false;
		addChild(panelSplitterDarkSignature);
		// Deepblue Signature panel.
		panelSplitterDeepBlueSignature = new SvgPanel();
		panelSplitterDeepBlueSignature->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Splitter1x9_Deepblue_Signature.svg")));
		panelSplitterDeepBlueSignature->visible = false;
		addChild(panelSplitterDeepBlueSignature);
		// Deepblue Signature panel.
		panelSplitterCarbonSignature = new SvgPanel();
		panelSplitterCarbonSignature->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Splitter1x9_Carbon_Signature.svg")));
		panelSplitterCarbonSignature->visible = false;
		addChild(panelSplitterCarbonSignature);
		// Using four screws configuration (even for 2 HP module), due to mechanical constraints on connectors... :-)
		// Top-left golden screw.
		topLeftScrewGold = createWidget<Torx_Gold>(Vec(0, 0));
		addChild(topLeftScrewGold);
		// Top-left silver screw.
		topLeftScrewSilver = createWidget<Torx_Silver>(Vec(0, 0));
		addChild(topLeftScrewSilver);
		// Top-right golden screw.
		topRightScrewGold = createWidget<Torx_Gold>(Vec(box.size.x - RACK_GRID_WIDTH, 0));
		addChild(topRightScrewGold);
		// Top-right silver screw.
		topRightScrewSilver = createWidget<Torx_Silver>(Vec(box.size.x - RACK_GRID_WIDTH, 0));
		addChild(topRightScrewSilver);
		// Bottom-left golden screw.
		bottomLeftScrewGold = createWidget<Torx_Gold>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomLeftScrewGold);
		// Bottom-left silver screw.
		bottomLeftScrewSilver = createWidget<Torx_Silver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomLeftScrewSilver);
		// Bottom-right golden screw.
		bottomRightScrewGold = createWidget<Torx_Gold>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomRightScrewGold);
		// Bottom-right silver screw.
		bottomRightScrewSilver = createWidget<Torx_Silver>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomRightScrewSilver);
		// Input ports (ports are using "dynamic ports" to change connector metal - silver or gold: big thanks to Marc Boulé!).
		addInput(createDynamicPort<DynSVGPort>(Vec(2.5, 22), true, module, SplitterModule::MAIN_INPUT, module ? &module->portMetal : NULL));
		// Output ports (ports are using "dynamic ports" to change connector metal - silver or gold).
		addOutput(createDynamicPort<DynSVGPort>(Vec(2.5, 70), false, module, SplitterModule::OUTPUT_1, module ? &module->portMetal : NULL));
		addOutput(createDynamicPort<DynSVGPort>(Vec(2.5, 100), false, module, SplitterModule::OUTPUT_2, module ? &module->portMetal : NULL));
		addOutput(createDynamicPort<DynSVGPort>(Vec(2.5, 130), false, module, SplitterModule::OUTPUT_3, module ? &module->portMetal : NULL));
		addOutput(createDynamicPort<DynSVGPort>(Vec(2.5, 160), false, module, SplitterModule::OUTPUT_4, module ? &module->portMetal : NULL));
		addOutput(createDynamicPort<DynSVGPort>(Vec(2.5, 190), false, module, SplitterModule::OUTPUT_5, module ? &module->portMetal : NULL));
		addOutput(createDynamicPort<DynSVGPort>(Vec(2.5, 220), false, module, SplitterModule::OUTPUT_6, module ? &module->portMetal : NULL));
		addOutput(createDynamicPort<DynSVGPort>(Vec(2.5, 250), false, module, SplitterModule::OUTPUT_7, module ? &module->portMetal : NULL));
		addOutput(createDynamicPort<DynSVGPort>(Vec(2.5, 280), false, module, SplitterModule::OUTPUT_8, module ? &module->portMetal : NULL));
		addOutput(createDynamicPort<DynSVGPort>(Vec(2.5, 310), false, module, SplitterModule::OUTPUT_9, module ? &module->portMetal : NULL));
	}

	void step() override {
		SplitterModule *module = dynamic_cast<SplitterModule*>(this->module);
		if (module) {
			// Possible alternate panel themes (GUIs).
			panelSplitterClassic->visible = (module->Theme == 0);
			panelSplitterStageRepro->visible = (module->Theme == 1);
			panelSplitterAbsoluteNight->visible = (module->Theme == 2);
			panelSplitterDarkSignature->visible = (module->Theme == 3);
			panelSplitterDeepBlueSignature->visible = (module->Theme == 4);
			panelSplitterCarbonSignature->visible = (module->Theme == 5);
			// Torx screws metal (silver, gold) are visible or hidden, depending selected model (from module's context-menu).
			// Silver Torx screws are visible only for non-"Signature" modules (Classic, Stage Repro or Absolute Night).
			topLeftScrewSilver->visible = (module->Theme < 3);
			topRightScrewSilver->visible = (module->Theme < 3);
			bottomLeftScrewSilver->visible = (module->Theme < 3);
			bottomRightScrewSilver->visible = (module->Theme < 3);
			// Gold Torx screws are visible only for "Signature" modules (Dark Signature, Deepblue Signature or Carbon Signature).
			topLeftScrewGold->visible = (module->Theme > 2);
			topRightScrewGold->visible = (module->Theme > 2);
			bottomLeftScrewGold->visible = (module->Theme > 2);
			bottomRightScrewGold->visible = (module->Theme > 2);
		}
		else {
			// Default panel theme is always "Classic" (beige, using silver screws, using silver button, LCD).
			// Other panels are, of course, hidden.
			panelSplitterClassic->visible = true;
			panelSplitterStageRepro->visible = false;
			panelSplitterAbsoluteNight->visible = false;
			panelSplitterDarkSignature->visible = false;
			panelSplitterDeepBlueSignature->visible = false;
			panelSplitterCarbonSignature->visible = false;
			// By default, silver screws are visible for default beige Classic panel...
			topLeftScrewSilver->visible = true;
			topRightScrewSilver->visible = true;
			bottomLeftScrewSilver->visible = true;
			bottomRightScrewSilver->visible = true;
			// ...and, of course, golden screws are hidden.
			topLeftScrewGold->visible = false;
			topRightScrewGold->visible = false;
			bottomLeftScrewGold->visible = false;
			bottomRightScrewGold->visible = false;
		}
		ModuleWidget::step();
	}

	void appendContextMenu(Menu *menu) override {
		SplitterModule *module = dynamic_cast<SplitterModule*>(this->module);

		menu->addChild(new MenuSeparator);

		SplitterSubMenuItems *spltrSubMenuItems = new SplitterSubMenuItems;
		spltrSubMenuItems->text = "Model";
		spltrSubMenuItems->rightText = RIGHT_ARROW;
		spltrSubMenuItems->module = module;
		menu->addChild(spltrSubMenuItems);
	}

};

Model *modelSplitter = createModel<SplitterModule, SplitterWidget>("SplitterModule");
