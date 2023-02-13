/////////////////////////////////////////////////////////////////////////////////////////////////
// Polarity Switch                                                                             //
// 3 HP module.                                                                                //
// - Input signal is routed to "P" (upper output) if its voltage is positive.                  //
// - Input signal is routed to "N" (lower output) if its voltage is negative, after conversion //
//   to positive unipolar equivalent voltage (aka... absolute value).                          //
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

struct PolaritySwitchModule : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_1,
		INPUT_2,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_P1,
		OUTPUT_N1,
		OUTPUT_P2,
		OUTPUT_N2,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	// Current selected model (GUI theme).
	int Theme = 0; // 0 = Classic (default), 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Carbon Signature.
	int portMetal = 0; // 0 = silver connector (default), 1 = gold connector used by "Signature"-line models only.
	bool Bipolar = false; // default false: N outputs are converted to positive voltage, true: voltage is kept as is.
	bool Force10V = false; // default false: absolute value of voltage stays unchanged, true: voltage is forced to +10V or to -10V.

	// Sample rate (from Rack engine).
	float sampleRate = 0.0f;

	PolaritySwitchModule() {
		// Constructor...
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configInput(INPUT_1, "Signal");
		configOutput(OUTPUT_P1, "P (if IN1 > 0)");
		configOutput(OUTPUT_N1, "N (if IN1 < 0)");
		configInput(INPUT_2, "Signal");
		configOutput(OUTPUT_P2, "P (if IN2 > 0)");
		configOutput(OUTPUT_N2, "N (if IN2 < 0)");
		onSampleRateChange();
	}

	void onSampleRateChange() override {
		sampleRate = APP->engine->getSampleRate();
	}		

	void process(const ProcessArgs &args) override {
		float bipol;
		float out_voltage;
		// First input (top part of module).
		if (inputs[INPUT_1].isConnected()) {
			out_voltage = clamp(inputs[INPUT_1].getVoltage(), -10.f, 10.f);
			bipol = Bipolar ? 1.f : -1.f;
			if (out_voltage > 0.f) {
				outputs[OUTPUT_P1].setVoltage(Force10V ? 10.f : out_voltage);
				outputs[OUTPUT_N1].setVoltage(0.f);
			}
			else if (out_voltage < 0.f) {
				outputs[OUTPUT_P1].setVoltage(0.f);
				outputs[OUTPUT_N1].setVoltage(Force10V ? -10.f * bipol : out_voltage * bipol);
			}
			else {
				outputs[OUTPUT_P1].setVoltage(0.f);
				outputs[OUTPUT_N1].setVoltage(0.f);
			}
		}
		else {
			// If input jack isn't connected, sets outputs at 0V.
			outputs[OUTPUT_P1].setVoltage(0.f);
			outputs[OUTPUT_N1].setVoltage(0.f);
		}

		// Second input (bottom part of module).
		if (inputs[INPUT_2].isConnected()) {
			out_voltage = clamp(inputs[INPUT_2].getVoltage(), -10.f, 10.f);
			bipol = Bipolar ? 1.f : -1.f;
			if (out_voltage > 0.f) {
				outputs[OUTPUT_P2].setVoltage(Force10V ? 10.f : out_voltage);
				outputs[OUTPUT_N2].setVoltage(0.f);
			}
			else if (out_voltage < 0.f) {
				outputs[OUTPUT_P2].setVoltage(0.f);
				outputs[OUTPUT_N2].setVoltage(Force10V ? -10.f * bipol : out_voltage * bipol);
			}
			else {
				outputs[OUTPUT_P2].setVoltage(0.f);
				outputs[OUTPUT_N2].setVoltage(0.f);
			}
		}
		else {
			// If input jack isn't connected, sets outputs at 0V.
			outputs[OUTPUT_P2].setVoltage(0.f);
			outputs[OUTPUT_N2].setVoltage(0.f);
		}
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Theme", json_integer(Theme));
		json_object_set_new(rootJ, "Bipolar", json_boolean(Bipolar));
		json_object_set_new(rootJ, "Force10V", json_boolean(Force10V));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		// Retrieving module theme/variation (when loading .vcv and cloning module).
		json_t *ThemeJ = json_object_get(rootJ, "Theme");
		if (ThemeJ) {
			Theme = json_integer_value(ThemeJ);
			portMetal = Theme / 3; // first three use silver (0), last three use gold (1) - the int division by 3 is useful ;)
		}
		// Retrieving unipolar/bipolar (when loading .vcv and cloning module).
		json_t *BipolarJ = json_object_get(rootJ, "Bipolar");
		if (BipolarJ)
			Bipolar = json_boolean_value(BipolarJ);

		// Retrieving forcing to 10V (when loading .vcv and cloning module).
		json_t *Force10VJ = json_object_get(rootJ, "Force10V");
		if (Force10VJ)
			Force10V = json_boolean_value(Force10VJ);
	}

};

///////////////////////////////////////////////////// CONTEXT-MENU: MODELS //////////////////////////////////////////////////////

struct PolaritySwitchClassicMenu : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 0; // Model: default Classic (beige).
		module->portMetal = 0; // Silver connectors for Classic.
	}
};

struct PolaritySwitchStageReproMenu : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 1; // Model: Stage Repro.
		module->portMetal = 0; // Silver connectors for Stage Repro.
	}
};

struct PolaritySwitchAbsoluteNightMenu : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 2; // Model: Absolute Night.
		module->portMetal = 0; // Silver connectors for Absolute Night.
	}
};

struct PolaritySwitchDarkSignatureMenu : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 3; // Model: Dark Signature.
		module->portMetal = 1; // Gold connectors for Dark Signature.
	}
};

struct PolaritySwitchDeepblueSignatureMenu : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 4; // Model: Deepblue Signature.
		module->portMetal = 1; // Gold connectors for Deepblue Signature.
	}
};

struct PolaritySwitchCarbonSignatureMenu : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 5; // Model: Carbon Signature.
		module->portMetal = 1; // Gold connectors for Carbon Signature.
	}
};

struct PolaritySwitchModelSubMenuItems : MenuItem {
	PolaritySwitchModule *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		PolaritySwitchClassicMenu *polswmenuitem1 = new PolaritySwitchClassicMenu;
		polswmenuitem1->text = "Classic (default)";
		polswmenuitem1->rightText = CHECKMARK(module->Theme == 0);
		polswmenuitem1->module = module;
		menu->addChild(polswmenuitem1);

		PolaritySwitchStageReproMenu *polswmenuitem2 = new PolaritySwitchStageReproMenu;
		polswmenuitem2->text = "Stage Repro";
		polswmenuitem2->rightText = CHECKMARK(module->Theme == 1);
		polswmenuitem2->module = module;
		menu->addChild(polswmenuitem2);

		PolaritySwitchAbsoluteNightMenu *polswmenuitem3 = new PolaritySwitchAbsoluteNightMenu;
		polswmenuitem3->text = "Absolute Night";
		polswmenuitem3->rightText = CHECKMARK(module->Theme == 2);
		polswmenuitem3->module = module;
		menu->addChild(polswmenuitem3);

		PolaritySwitchDarkSignatureMenu *polswmenuitem4 = new PolaritySwitchDarkSignatureMenu;
		polswmenuitem4->text = "Dark \"Signature\"";
		polswmenuitem4->rightText = CHECKMARK(module->Theme == 3);
		polswmenuitem4->module = module;
		menu->addChild(polswmenuitem4);

		PolaritySwitchDeepblueSignatureMenu *polswmenuitem5 = new PolaritySwitchDeepblueSignatureMenu;
		polswmenuitem5->text = "Deepblue \"Signature\"";
		polswmenuitem5->rightText = CHECKMARK(module->Theme == 4);
		polswmenuitem5->module = module;
		menu->addChild(polswmenuitem5);

		PolaritySwitchCarbonSignatureMenu *polswmenuitem6 = new PolaritySwitchCarbonSignatureMenu;
		polswmenuitem6->text = "Carbon \"Signature\"";
		polswmenuitem6->rightText = CHECKMARK(module->Theme == 5);
		polswmenuitem6->module = module;
		menu->addChild(polswmenuitem6);

		return menu;
	}
};


///////////////////////////////////////////////////// CONTEXT-MENU: BIPOLAR /////////////////////////////////////////////

struct PolaritySwitchBipolarMenu : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->Bipolar = !(module->Bipolar);
	}
};

///////////////////////////////////////////////////// CONTEXT-MENU: BIPOLAR /////////////////////////////////////////////

struct PolaritySwitchForceTenVoltsMenu : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->Force10V = !(module->Force10V);
	}
};

///////////////////////////////////////////////// MODULE WIDGET SECTION /////////////////////////////////////////////////

struct PolaritySwitchWidget : ModuleWidget {
	// Panels (one per "Theme").
	SvgPanel *panelPolaritySwitchClassic;
	SvgPanel *panelPolaritySwitchStageRepro;
	SvgPanel *panelPolaritySwitchAbsoluteNight;
	SvgPanel *panelPolaritySwitchDarkSignature;
	SvgPanel *panelPolaritySwitchDeepBlueSignature;
	SvgPanel *panelPolaritySwitchCarbonSignature;
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

	PolaritySwitchWidget(PolaritySwitchModule *module) {
		setModule(module);
		box.size = Vec(3 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		// Classic (default) beige panel.
		panelPolaritySwitchClassic = new SvgPanel();
		panelPolaritySwitchClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Polarity_Switch_Classic.svg")));
		panelPolaritySwitchClassic->visible = true;
		addChild(panelPolaritySwitchClassic);
		// Stage Repro panel.
		panelPolaritySwitchStageRepro = new SvgPanel();
		panelPolaritySwitchStageRepro->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Polarity_Switch_Stage_Repro.svg")));
		panelPolaritySwitchStageRepro->visible = false;
		addChild(panelPolaritySwitchStageRepro);
		// Absolute Night panel.
		panelPolaritySwitchAbsoluteNight = new SvgPanel();
		panelPolaritySwitchAbsoluteNight->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Polarity_Switch_Absolute_Night.svg")));
		panelPolaritySwitchAbsoluteNight->visible = false;
		addChild(panelPolaritySwitchAbsoluteNight);
		// Dark Signature panel.
		panelPolaritySwitchDarkSignature = new SvgPanel();
		panelPolaritySwitchDarkSignature->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Polarity_Switch_Dark_Signature.svg")));
		panelPolaritySwitchDarkSignature->visible = false;
		addChild(panelPolaritySwitchDarkSignature);
		// Deepblue Signature panel.
		panelPolaritySwitchDeepBlueSignature = new SvgPanel();
		panelPolaritySwitchDeepBlueSignature->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Polarity_Switch_Deepblue_Signature.svg")));
		panelPolaritySwitchDeepBlueSignature->visible = false;
		addChild(panelPolaritySwitchDeepBlueSignature);
		// Deepblue Signature panel.
		panelPolaritySwitchCarbonSignature = new SvgPanel();
		panelPolaritySwitchCarbonSignature->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Polarity_Switch_Carbon_Signature.svg")));
		panelPolaritySwitchCarbonSignature->visible = false;
		addChild(panelPolaritySwitchCarbonSignature);
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
		addInput(createDynamicPort<DynSVGPort>(Vec(10.f, 32.f), true, module, PolaritySwitchModule::INPUT_1, module ? &module->portMetal : NULL));
		addInput(createDynamicPort<DynSVGPort>(Vec(10.f, 205.5f), true, module, PolaritySwitchModule::INPUT_2, module ? &module->portMetal : NULL));
		// Output ports (ports are using "dynamic ports" to change connector metal - silver or gold).
		addOutput(createDynamicPort<DynSVGPort>(Vec(10.f, 96.f), false, module, PolaritySwitchModule::OUTPUT_P1, module ? &module->portMetal : NULL));
		addOutput(createDynamicPort<DynSVGPort>(Vec(10.f, 139.f), false, module, PolaritySwitchModule::OUTPUT_N1, module ? &module->portMetal : NULL));
		addOutput(createDynamicPort<DynSVGPort>(Vec(10.f, 269.5f), false, module, PolaritySwitchModule::OUTPUT_P2, module ? &module->portMetal : NULL));
		addOutput(createDynamicPort<DynSVGPort>(Vec(10.f, 312.5f), false, module, PolaritySwitchModule::OUTPUT_N2, module ? &module->portMetal : NULL));
	}

	void step() override {
		PolaritySwitchModule *module = dynamic_cast<PolaritySwitchModule*>(this->module);
		if (module) {
			// Possible alternate panel themes (GUIs).
			panelPolaritySwitchClassic->visible = (module->Theme == 0);
			panelPolaritySwitchStageRepro->visible = (module->Theme == 1);
			panelPolaritySwitchAbsoluteNight->visible = (module->Theme == 2);
			panelPolaritySwitchDarkSignature->visible = (module->Theme == 3);
			panelPolaritySwitchDeepBlueSignature->visible = (module->Theme == 4);
			panelPolaritySwitchCarbonSignature->visible = (module->Theme == 5);
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
			panelPolaritySwitchClassic->visible = true;
			panelPolaritySwitchStageRepro->visible = false;
			panelPolaritySwitchAbsoluteNight->visible = false;
			panelPolaritySwitchDarkSignature->visible = false;
			panelPolaritySwitchDeepBlueSignature->visible = false;
			panelPolaritySwitchCarbonSignature->visible = false;
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
		PolaritySwitchModule *module = dynamic_cast<PolaritySwitchModule*>(this->module);
		menu->addChild(new MenuEntry);

		PolaritySwitchModelSubMenuItems *polswModelSubMenuItems = new PolaritySwitchModelSubMenuItems;
		polswModelSubMenuItems->text = "Model";
		polswModelSubMenuItems->rightText = RIGHT_ARROW;
		polswModelSubMenuItems->module = module;
		menu->addChild(polswModelSubMenuItems);

		PolaritySwitchBipolarMenu *polswmenubipolar = new PolaritySwitchBipolarMenu;
		polswmenubipolar->text = "\"N\" outs: keep negative V";
		polswmenubipolar->rightText = CHECKMARK(module->Bipolar == true);
		polswmenubipolar->module = module;
		menu->addChild(polswmenubipolar);

		PolaritySwitchForceTenVoltsMenu *polswmenuforce = new PolaritySwitchForceTenVoltsMenu;
		polswmenuforce->text = "\"N\" outs: force to 10V";
		polswmenuforce->rightText = CHECKMARK(module->Force10V == true);
		polswmenuforce->module = module;
		menu->addChild(polswmenuforce);
	}

};

Model *modelPolaritySwitch = createModel<PolaritySwitchModule, PolaritySwitchWidget>("PolaritySwitch");
