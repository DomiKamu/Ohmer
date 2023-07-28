/////////////////////////////////////////////////////////////////////////////////////////////////
// Polarity Switch                                                                             //
// 3 HP module.                                                                                //
// - Input signal is routed to "P" (upper output) if its voltage is positive.                  //
// - Input signal is routed to "N" (lower output) if its voltage is negative, after conversion //
//   to positive unipolar equivalent voltage (aka... absolute value), or +5V, or +10V.         //
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

	// Output voltage settings for upper module and lower module.
	int UpperVoltage = 0; // 0 means unaltered IN voltage, 1 means output voltage(s) is/are forced to +5V, 2 means output voltage(s) is/are forced to +10V.
	int LowerVoltage = 0; // 0 means unaltered IN voltage, 1 means output voltage(s) is/are forced to +5V, 2 means output voltage(s) is/are forced to +10V.

	// Sample rate (from Rack engine).
	float sampleRate = 0.0f;

	PolaritySwitchModule() {
		// Module's constructor...
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configInput(INPUT_1, "Signal");
		configOutput(OUTPUT_P1, "P (if IN1 > 0)");
		configOutput(OUTPUT_N1, "N (if IN1 < 0)");
		configInput(INPUT_2, "Signal");
		configOutput(OUTPUT_P2, "P (if IN2 > 0)");
		configOutput(OUTPUT_N2, "N (if IN2 < 0)");
		UpperVoltage = 0;
		LowerVoltage = 0;
		onSampleRateChange();
	}

	void onSampleRateChange() override {
		sampleRate = APP->engine->getSampleRate();
	}		

	void process(const ProcessArgs &args) override {
		float out_voltage;
		// First input (upper part of the module).
		out_voltage = clamp(inputs[INPUT_1].getVoltage(), -10.f, 10.f);
		if (out_voltage >= 0.f) {
			// Voltage is positive: routing to "P" output jack.
			switch (UpperVoltage) {
				case 0:
					outputs[OUTPUT_P1].setVoltage(out_voltage); // IN voltage is kept as is, routed to "P" output jack.
					break;
				case 1:
					outputs[OUTPUT_P1].setVoltage(5.f); // Forced +5V routed to "P" output jack.
					break;
				case 2:
					outputs[OUTPUT_P1].setVoltage(10.f); // Forced +10V routed to "P" output jack.
			}
			// Voltage is positive: "N" output jack is set to 0V.
			outputs[OUTPUT_N1].setVoltage(0.f);
		}
		else {
			// Voltage is negative: routing to "N" output jack (but as absolute value).
			switch (UpperVoltage) {
				case 0:
					outputs[OUTPUT_N1].setVoltage(-1.f * out_voltage); // Convert the negative voltage applied on "IN" jack to positive equivalent (absolute value) before sending it to "N" jack!
					break;
				case 1:
					outputs[OUTPUT_N1].setVoltage(5.f); // Forced +5V routed to "N" output jack.
					break;
				case 2:
					outputs[OUTPUT_N1].setVoltage(10.f); // Forced +10V routed to "N" output jack.
			}
			// Voltage is negative: "P" output jack is set to 0V.
			outputs[OUTPUT_P1].setVoltage(0.f);
		}

		// Second input (lower part of the module).
		out_voltage = clamp(inputs[INPUT_2].getVoltage(), -10.f, 10.f);
		if (out_voltage >= 0.f) {
			// Voltage is positive: routing to "P" output jack.
			switch (LowerVoltage) {
				case 0:
					outputs[OUTPUT_P2].setVoltage(out_voltage); // IN voltage is kept as is, routed to "P" output jack.
					break;
				case 1:
					outputs[OUTPUT_P2].setVoltage(5.f); // Forced +5V routed to "P" output jack.
					break;
				case 2:
					outputs[OUTPUT_P2].setVoltage(10.f); // Forced +10V routed to "P" output jack.
			}
			// Voltage is positive: "N" output jack is set to 0V.
			outputs[OUTPUT_N2].setVoltage(0.f);
		}
		else {
			// Voltage is negative: routing to "N" output jack (but as absolute value).
			switch (LowerVoltage) {
				case 0:
					outputs[OUTPUT_N2].setVoltage(-1.f * out_voltage); // Convert the negative voltage applied on "IN" jack to positive equivalent (absolute value) before sending it to "N" jack!
					break;
				case 1:
					outputs[OUTPUT_N2].setVoltage(5.f); // Forced +5V routed to "N" output jack.
					break;
				case 2:
					outputs[OUTPUT_N2].setVoltage(10.f); // Forced +10V routed to "N" output jack.
			}
			// Voltage is negative: "P" output jack is set to 0V.
			outputs[OUTPUT_P2].setVoltage(0.f);
		}

	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Theme", json_integer(Theme));
		json_object_set_new(rootJ, "UpperVoltage", json_integer(UpperVoltage));
		json_object_set_new(rootJ, "LowerVoltage", json_integer(LowerVoltage));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		// Retrieving module theme/variation (when loading .vcv and cloning module).
		json_t *ThemeJ = json_object_get(rootJ, "Theme");
		if (ThemeJ) {
			Theme = json_integer_value(ThemeJ);
			portMetal = Theme / 3; // first three use silver (0), last three use gold (1) - the int division by 3 is useful ;)
		}

		// Retrieving upper module voltage behavior ("P" and "N" outputs).
		json_t *UpperVoltageJ = json_object_get(rootJ, "UpperVoltage");
		if (UpperVoltageJ)
			UpperVoltage = json_integer_value(UpperVoltageJ);

		// Retrieving lower module voltage behavior ("P" and "N" outputs).
		json_t *LowerVoltageJ = json_object_get(rootJ, "LowerVoltage");
		if (LowerVoltageJ)
			LowerVoltage = json_integer_value(LowerVoltageJ);

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

/////////////////////////////////////////// CONTEXT-MENU: UPPER MODULE //////////////////////////////////////////////////

struct UpperKeepVoltage : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->UpperVoltage = 0; // Keep voltage.
	}
};

struct UpperForce5V : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->UpperVoltage = 1; // Force all outputs to +5V.
	}
};

struct UpperForce10V : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->UpperVoltage = 2; // Force all outputs to +10V.
	}
};

/////////////////////////////////////////// CONTEXT-MENU: LOWER MODULE //////////////////////////////////////////////////

struct LowerKeepVoltage : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->LowerVoltage = 0; // Keep voltage.
	}
};

struct LowerForce5V : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->LowerVoltage = 1; // Force all outputs to +5V.
	}
};

struct LowerForce10V : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->LowerVoltage = 2; // Force all outputs to +10V.
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

		menu->addChild(new MenuSeparator);

		PolaritySwitchModelSubMenuItems *polswModelSubMenuItems = new PolaritySwitchModelSubMenuItems;
		polswModelSubMenuItems->text = "Model";
		polswModelSubMenuItems->rightText = RIGHT_ARROW;
		polswModelSubMenuItems->module = module;
		menu->addChild(polswModelSubMenuItems);

		menu->addChild(new MenuSeparator);

		MenuLabel *upperPolaritySwitchLabel = new MenuLabel();
		upperPolaritySwitchLabel->text = "UPPER PART";
		menu->addChild(upperPolaritySwitchLabel);

		UpperKeepVoltage *upperKeepVoltage = new UpperKeepVoltage;
		upperKeepVoltage->text = "Keep IN voltage (default)";
		upperKeepVoltage->rightText = CHECKMARK(module->UpperVoltage == 0);
		upperKeepVoltage->module = module;
		menu->addChild(upperKeepVoltage);

		UpperForce5V *upperforce5v = new UpperForce5V;
		upperforce5v->text = "Force outputs to +5V";
		upperforce5v->rightText = CHECKMARK(module->UpperVoltage == 1);
		upperforce5v->module = module;
		menu->addChild(upperforce5v);

		UpperForce10V *upperforce10v = new UpperForce10V;
		upperforce10v->text = "Force outputs to +10V";
		upperforce10v->rightText = CHECKMARK(module->UpperVoltage == 2);
		upperforce10v->module = module;
		menu->addChild(upperforce10v);

		menu->addChild(new MenuSeparator);

		MenuLabel *lowerPolaritySwitchLabel = new MenuLabel();
		lowerPolaritySwitchLabel->text = "LOWER PART";
		menu->addChild(lowerPolaritySwitchLabel);

		LowerKeepVoltage *lowerKeepVoltage = new LowerKeepVoltage;
		lowerKeepVoltage->text = "Keep IN voltage (default)";
		lowerKeepVoltage->rightText = CHECKMARK(module->LowerVoltage == 0);
		lowerKeepVoltage->module = module;
		menu->addChild(lowerKeepVoltage);

		LowerForce5V *lowerforce5v = new LowerForce5V;
		lowerforce5v->text = "Force outputs to +5V";
		lowerforce5v->rightText = CHECKMARK(module->LowerVoltage == 1);
		lowerforce5v->module = module;
		menu->addChild(lowerforce5v);

		LowerForce10V *lowerforce10v = new LowerForce10V;
		lowerforce10v->text = "Force outputs to +10V";
		lowerforce10v->rightText = CHECKMARK(module->LowerVoltage == 2);
		lowerforce10v->module = module;
		menu->addChild(lowerforce10v);

	}

};

Model *modelPolaritySwitch = createModel<PolaritySwitchModule, PolaritySwitchWidget>("PolaritySwitch");
