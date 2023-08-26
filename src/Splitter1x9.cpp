//////////////////////////////////////////////////////////////////////////////////////////////
// Splitter 1x9                                                                             //
// 2 HP module, having 1 input sent "splitted" to 9 outputs, but limited voltages must stay //
// into -11.7 V / +11.7 V bounds to every output ("hard clipping").                         //
// This module is polyphonic.                                                               //
//////////////////////////////////////////////////////////////////////////////////////////////

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
	int Model; // 0 = Creamy, 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Titanium Signature.
	int portMetal = 0; // 0 = silver connector (default), 1 = gold connector used by "Signature"-line models only.

	// Sample rate (from Rack engine).
	float sampleRate = 0.0f;

	SplitterModule() {
		// Module constructor.
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
		// Model.
		Model = rack::settings::preferDarkPanels ? 2 : 0; // Model: assuming default is "Creamy" or "Absolute Night" (depending "Use dark panels if available" option, from "View" menu).
		// Get current engine sample rate.
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
		json_object_set_new(rootJ, "Model", json_integer(Model));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		// Retrieving module theme/variation (when loading .vcv and cloning module).
		json_t *ModelJ = json_object_get(rootJ, "Model");
		if (ModelJ)
			Model = json_integer_value(ModelJ);
			else {
				// Used to migrate to "Model" (instead of "Theme") in json (compatibility).
				json_t *ModelJ = json_object_get(rootJ, "Theme");
				if (ModelJ)
					Model = json_integer_value(ModelJ);
			}
		portMetal = Model / 3; // first three use silver (0), last three use gold (1) - the int division by 3 is useful ;)
	}

};

///////////////////////////////////////////////////// CONTEXT-MENU //////////////////////////////////////////////////////

struct SplitterCreamyMenu : MenuItem {
	SplitterModule *module;
	void onAction(const event::Action &e) override {
		module->Model = 0; // Model: Creamy.
		module->portMetal = 0; // Silver connectors for Creamy.
	}
};

struct SplitterStageReproMenu : MenuItem {
	SplitterModule *module;
	void onAction(const event::Action &e) override {
		module->Model = 1; // Model: Stage Repro.
		module->portMetal = 0; // Silver connectors for Stage Repro.
	}
};

struct SplitterAbsoluteNightMenu : MenuItem {
	SplitterModule *module;
	void onAction(const event::Action &e) override {
		module->Model = 2; // Model: Absolute Night.
		module->portMetal = 0; // Silver connectors for Absolute Night.
	}
};

struct SplitterDarkSignatureMenu : MenuItem {
	SplitterModule *module;
	void onAction(const event::Action &e) override {
		module->Model = 3; // Model: Dark Signature.
		module->portMetal = 1; // Gold connectors for Dark Signature.
	}
};

struct SplitterDeepblueSignatureMenu : MenuItem {
	SplitterModule *module;
	void onAction(const event::Action &e) override {
		module->Model = 4; // Model: Deepblue Signature.
		module->portMetal = 1; // Gold connectors for Deepblue Signature.
	}
};

struct SplitterTitaniumSignatureMenu : MenuItem {
	SplitterModule *module;
	void onAction(const event::Action &e) override {
		module->Model = 5; // Model: Titanium Signature.
		module->portMetal = 1; // Gold connectors for Titanium Signature.
	}
};

struct SplitterSubMenuItems : MenuItem {
	SplitterModule *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		SplitterCreamyMenu *splittercreamymenu = new SplitterCreamyMenu;
		splittercreamymenu->text = "Creamy";
		splittercreamymenu->rightText = CHECKMARK(module->Model == 0);
		splittercreamymenu->module = module;
		menu->addChild(splittercreamymenu);

		SplitterStageReproMenu *splitterstagerepromenu = new SplitterStageReproMenu;
		splitterstagerepromenu->text = "Stage Repro";
		splitterstagerepromenu->rightText = CHECKMARK(module->Model == 1);
		splitterstagerepromenu->module = module;
		menu->addChild(splitterstagerepromenu);

		SplitterAbsoluteNightMenu *splitterabsolutenightmenu = new SplitterAbsoluteNightMenu;
		splitterabsolutenightmenu->text = "Absolute Night";
		splitterabsolutenightmenu->rightText = CHECKMARK(module->Model == 2);
		splitterabsolutenightmenu->module = module;
		menu->addChild(splitterabsolutenightmenu);

		SplitterDarkSignatureMenu *splitterdarksignaturemenu = new SplitterDarkSignatureMenu;
		splitterdarksignaturemenu->text = "Dark \"Signature\"";
		splitterdarksignaturemenu->rightText = CHECKMARK(module->Model == 3);
		splitterdarksignaturemenu->module = module;
		menu->addChild(splitterdarksignaturemenu);

		SplitterDeepblueSignatureMenu *splitterdeepbluesignaturemenu = new SplitterDeepblueSignatureMenu;
		splitterdeepbluesignaturemenu->text = "Deepblue \"Signature\"";
		splitterdeepbluesignaturemenu->rightText = CHECKMARK(module->Model == 4);
		splitterdeepbluesignaturemenu->module = module;
		menu->addChild(splitterdeepbluesignaturemenu);

		SplitterTitaniumSignatureMenu *splittertitaniumsignaturemenu = new SplitterTitaniumSignatureMenu;
		splittertitaniumsignaturemenu->text = "Titanium \"Signature\"";
		splittertitaniumsignaturemenu->rightText = CHECKMARK(module->Model == 5);
		splittertitaniumsignaturemenu->module = module;
		menu->addChild(splittertitaniumsignaturemenu);

		return menu;
	}
};

///////////////////////////////////////////////// MODULE WIDGET SECTION /////////////////////////////////////////////////

struct SplitterWidget : ModuleWidget {
	// Panels.
	SvgPanel *panelSplitterCreamy;
	SvgPanel *panelSplitterStageRepro;
	SvgPanel *panelSplitterAbsoluteNight;
	SvgPanel *panelSplitterDarkSignature;
	SvgPanel *panelSplitterDeepBlueSignature;
	SvgPanel *panelSplitterTitaniumSignature;
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
		// Creamy panel.
		panelSplitterCreamy = new SvgPanel();
		panelSplitterCreamy->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Splitter1x9_Creamy.svg")));
		panelSplitterCreamy->visible = true;
		addChild(panelSplitterCreamy);
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
		panelSplitterTitaniumSignature = new SvgPanel();
		panelSplitterTitaniumSignature->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Splitter1x9_Titanium_Signature.svg")));
		panelSplitterTitaniumSignature->visible = false;
		addChild(panelSplitterTitaniumSignature);
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
			// Possible panels.
			panelSplitterCreamy->visible = (module->Model == 0);
			panelSplitterStageRepro->visible = (module->Model == 1);
			panelSplitterAbsoluteNight->visible = (module->Model == 2);
			panelSplitterDarkSignature->visible = (module->Model == 3);
			panelSplitterDeepBlueSignature->visible = (module->Model == 4);
			panelSplitterTitaniumSignature->visible = (module->Model == 5);
			// Torx screws metal (silver, gold) are visible or hidden, depending selected model (from module's context-menu).
			// Silver Torx screws are visible only for non-"Signature" modules (Creamy, Stage Repro or Absolute Night).
			topLeftScrewSilver->visible = (module->Model < 3);
			topRightScrewSilver->visible = (module->Model < 3);
			bottomLeftScrewSilver->visible = (module->Model < 3);
			bottomRightScrewSilver->visible = (module->Model < 3);
			// Gold Torx screws are visible only for "Signature" modules (Dark Signature, Deepblue Signature or Titanium Signature).
			topLeftScrewGold->visible = (module->Model > 2);
			topRightScrewGold->visible = (module->Model > 2);
			bottomLeftScrewGold->visible = (module->Model > 2);
			bottomRightScrewGold->visible = (module->Model > 2);
		}
		else {
			// !module - probably from module browser.
			// Default model is always "Creamy" or "Absolute Night" (depending "Use dark panels if available" option, from "View" menu).
			// Other panels are, of course, hidden.
			panelSplitterCreamy->visible = !rack::settings::preferDarkPanels;
			panelSplitterStageRepro->visible = false;
			panelSplitterAbsoluteNight->visible = rack::settings::preferDarkPanels;
			panelSplitterDarkSignature->visible = false;
			panelSplitterDeepBlueSignature->visible = false;
			panelSplitterTitaniumSignature->visible = false;
			// By default, silver screws are visible by default ("Creamy" or "Absolute Night" panel).
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
