////////////////////////////////////////////////////////////////////////////////////////////////
// Polarity Switch                                                                            //
// 3 HP module, polyphonic.                                                                   //
// - Input signal is routed to "P" (upper output) if voltage is positive.                     //
// - Input signal is routed to "N" (lower output) if voltage is negative, after conversion to //
//   positive equivalent (absolute value) voltage, or +5V, or +10V.                           //
////////////////////////////////////////////////////////////////////////////////////////////////

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
	int Model; // 0 = Creamy, 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Titanium Signature.
	int portMetal = 0; // 0 = silver connector (default), 1 = gold connector used by "Signature"-line models only.

	// Output voltage settings for upper module and lower module.
	int UpperVoltage = 0; // 0 means unaltered IN voltage, 1 means output voltage(s) is/are forced to +5V, 2 means output voltage(s) is/are forced to +10V.
	int LowerVoltage = 0; // 0 means unaltered IN voltage, 1 means output voltage(s) is/are forced to +5V, 2 means output voltage(s) is/are forced to +10V.

	// Sample rate (from Rack engine).
	float sampleRate = 0.0f;

	PolaritySwitchModule() {
		// Module constructor.
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configInput(INPUT_1, "IN1 signal");
		configOutput(OUTPUT_P1, "If IN1 positive: sent to this P1");
		configOutput(OUTPUT_N1, "If IN1 negative: sent to this N1");
		configInput(INPUT_2, "IN2 signal");
		configOutput(OUTPUT_P2, "If IN2 positive: sent to this P2");
		configOutput(OUTPUT_N2, "If IN2 negative: sent to this N2");
		UpperVoltage = 0;
		LowerVoltage = 0;
		// Model.
		Model = rack::settings::preferDarkPanels ? 2 : 0; // Model: assuming default is "Creamy" or "Absolute Night" (depending "Use dark panels if available" option, from "View" menu).
		// Get current engine sample rate.
		onSampleRateChange();
	}

	void onSampleRateChange() override {
		sampleRate = APP->engine->getSampleRate();
	}		

	void process(const ProcessArgs &args) override {
		int nChannels; // Used for number of channels (monophonic or polyphonic).
		float out_voltage;

		if (inputs[INPUT_1].isConnected()) {
			// Upper input jack (IN1).
			nChannels = std::max(1, inputs[INPUT_1].getChannels()); // Get number of polyphonic channels for IN input (1 if monophonic cable).
			// Proceeding all channels.
			for (int chan = 0;  chan < nChannels; chan++) {
				out_voltage = clamp(inputs[INPUT_1].getVoltage(chan), -10.f, 10.f);
				if (out_voltage >= 0.f) {
					// Voltage is positive: routing to "P" output jack.
					switch (UpperVoltage) {
						case 0:
							outputs[OUTPUT_P1].setVoltage(out_voltage, chan); // IN voltage is kept as is, routed to "P" output jack (related polyphony channel).
							break;
						case 1:
							outputs[OUTPUT_P1].setVoltage(5.f, chan); // Forced +5V routed to "P" output jack (related polyphony channel).
							break;
						case 2:
							outputs[OUTPUT_P1].setVoltage(10.f, chan); // Forced +10V routed to "P" output jack (related polyphony channel).
					}
					// Voltage is positive: "N" output jack is set to 0V (related polyphony channel).
					outputs[OUTPUT_N1].setVoltage(0.f, chan);
				}
				else {
					// Voltage is negative: routing to "N" output jack (but as absolute value).
					switch (UpperVoltage) {
						case 0:
							outputs[OUTPUT_N1].setVoltage(std::abs(out_voltage), chan); // Convert the negative voltage to positive (aka "absolute value") before sending it to "N" jack (related polyphony channel).
							break;
						case 1:
							outputs[OUTPUT_N1].setVoltage(5.f, chan); // Forced +5V routed to "N" output jack (related polyphony channel).
							break;
						case 2:
							outputs[OUTPUT_N1].setVoltage(10.f, chan); // Forced +10V routed to "N" output jack (related polyphony channel).
					}
					// Voltage is negative: "P" output jack is set to 0V (related polyphony channel).
					outputs[OUTPUT_P1].setVoltage(0.f, chan);
				}
			}
			outputs[OUTPUT_P1].setChannels(nChannels);
			outputs[OUTPUT_N1].setChannels(nChannels);
		}
		else {
			// IN input jack isn't connected: send 0V to both "P" and "N" jacks.
			outputs[OUTPUT_P1].setChannels(1);
			outputs[OUTPUT_P1].setVoltage(0.f);
			outputs[OUTPUT_N1].setChannels(1);
			outputs[OUTPUT_N1].setVoltage(0.f);
		}

		if (inputs[INPUT_2].isConnected()) {
			// Lower input jack (IN2).
			nChannels = std::max(1, inputs[INPUT_2].getChannels()); // Get number of polyphonic channels for IN input (1 if monophonic cable).
			// Proceeding all channels.
			for (int chan = 0;  chan < nChannels; chan++) {
				out_voltage = clamp(inputs[INPUT_2].getVoltage(chan), -10.f, 10.f);
				if (out_voltage >= 0.f) {
					// Voltage is positive: routing to "P" output jack.
					switch (LowerVoltage) {
						case 0:
							outputs[OUTPUT_P2].setVoltage(out_voltage, chan); // IN voltage is kept as is, routed to "P" output jack (related polyphony channel).
							break;
						case 1:
							outputs[OUTPUT_P2].setVoltage(5.f, chan); // Forced +5V routed to "P" output jack (related polyphony channel).
							break;
						case 2:
							outputs[OUTPUT_P2].setVoltage(10.f, chan); // Forced +10V routed to "P" output jack (related polyphony channel).
					}
					// Voltage is positive: "N" output jack is set to 0V (related polyphony channel).
					outputs[OUTPUT_N2].setVoltage(0.f, chan);
				}
				else {
					// Voltage is negative: routing to "N" output jack (but as absolute value).
					switch (LowerVoltage) {
						case 0:
							outputs[OUTPUT_N2].setVoltage(std::abs(out_voltage), chan); // Convert the negative voltage to positive (aka "absolute value") before sending it to "N" jack (related polyphony channel).
							break;
						case 1:
							outputs[OUTPUT_N2].setVoltage(5.f, chan); // Forced +5V routed to "N" output jack (related polyphony channel).
							break;
						case 2:
							outputs[OUTPUT_N2].setVoltage(10.f, chan); // Forced +10V routed to "N" output jack (related polyphony channel).
					}
					// Voltage is negative: "P" output jack is set to 0V (related polyphony channel).
					outputs[OUTPUT_P2].setVoltage(0.f, chan);
				}
			}
			outputs[OUTPUT_P2].setChannels(nChannels);
			outputs[OUTPUT_N2].setChannels(nChannels);
		}
		else {
			// IN input jack isn't connected: send 0V to both "P" and "N" jacks.
			outputs[OUTPUT_P2].setChannels(1);
			outputs[OUTPUT_P2].setVoltage(0.f);
			outputs[OUTPUT_N2].setChannels(1);
			outputs[OUTPUT_N2].setVoltage(0.f);
		}

	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Model", json_integer(Model));
		json_object_set_new(rootJ, "UpperVoltage", json_integer(UpperVoltage));
		json_object_set_new(rootJ, "LowerVoltage", json_integer(LowerVoltage));
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

struct PolaritySwitchCreamyMenu : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->Model = 0; // Model: Creamy.
		module->portMetal = 0; // Silver connectors for Creamy.
	}
};

struct PolaritySwitchStageReproMenu : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->Model = 1; // Model: Stage Repro.
		module->portMetal = 0; // Silver connectors for Stage Repro.
	}
};

struct PolaritySwitchAbsoluteNightMenu : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->Model = 2; // Model: Absolute Night.
		module->portMetal = 0; // Silver connectors for Absolute Night.
	}
};

struct PolaritySwitchDarkSignatureMenu : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->Model = 3; // Model: Dark Signature.
		module->portMetal = 1; // Gold connectors for Dark Signature.
	}
};

struct PolaritySwitchDeepblueSignatureMenu : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->Model = 4; // Model: Deepblue Signature.
		module->portMetal = 1; // Gold connectors for Deepblue Signature.
	}
};

struct PolaritySwitchTitaniumSignatureMenu : MenuItem {
	PolaritySwitchModule *module;
	void onAction(const event::Action &e) override {
		module->Model = 5; // Model: Titanium Signature.
		module->portMetal = 1; // Gold connectors for Titanium Signature.
	}
};

struct PolaritySwitchModelSubMenuItems : MenuItem {
	PolaritySwitchModule *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		PolaritySwitchCreamyMenu *polarityswitchcreamymenu = new PolaritySwitchCreamyMenu;
		polarityswitchcreamymenu->text = "Creamy";
		polarityswitchcreamymenu->rightText = CHECKMARK(module->Model == 0);
		polarityswitchcreamymenu->module = module;
		menu->addChild(polarityswitchcreamymenu);

		PolaritySwitchStageReproMenu *polarityswitchstagerepromenu = new PolaritySwitchStageReproMenu;
		polarityswitchstagerepromenu->text = "Stage Repro";
		polarityswitchstagerepromenu->rightText = CHECKMARK(module->Model == 1);
		polarityswitchstagerepromenu->module = module;
		menu->addChild(polarityswitchstagerepromenu);

		PolaritySwitchAbsoluteNightMenu *polarityswitchabsolutenightmenu = new PolaritySwitchAbsoluteNightMenu;
		polarityswitchabsolutenightmenu->text = "Absolute Night";
		polarityswitchabsolutenightmenu->rightText = CHECKMARK(module->Model == 2);
		polarityswitchabsolutenightmenu->module = module;
		menu->addChild(polarityswitchabsolutenightmenu);

		PolaritySwitchDarkSignatureMenu *polarityswitchdarksignaturemenu = new PolaritySwitchDarkSignatureMenu;
		polarityswitchdarksignaturemenu->text = "Dark \"Signature\"";
		polarityswitchdarksignaturemenu->rightText = CHECKMARK(module->Model == 3);
		polarityswitchdarksignaturemenu->module = module;
		menu->addChild(polarityswitchdarksignaturemenu);

		PolaritySwitchDeepblueSignatureMenu *polarityswitchdeepbluesignaturemenu = new PolaritySwitchDeepblueSignatureMenu;
		polarityswitchdeepbluesignaturemenu->text = "Deepblue \"Signature\"";
		polarityswitchdeepbluesignaturemenu->rightText = CHECKMARK(module->Model == 4);
		polarityswitchdeepbluesignaturemenu->module = module;
		menu->addChild(polarityswitchdeepbluesignaturemenu);

		PolaritySwitchTitaniumSignatureMenu *polarityswitchtitaniumsignaturemenu = new PolaritySwitchTitaniumSignatureMenu;
		polarityswitchtitaniumsignaturemenu->text = "Titanium \"Signature\"";
		polarityswitchtitaniumsignaturemenu->rightText = CHECKMARK(module->Model == 5);
		polarityswitchtitaniumsignaturemenu->module = module;
		menu->addChild(polarityswitchtitaniumsignaturemenu);

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
	// Panels.
	SvgPanel *panelPolaritySwitchCreamy;
	SvgPanel *panelPolaritySwitchStageRepro;
	SvgPanel *panelPolaritySwitchAbsoluteNight;
	SvgPanel *panelPolaritySwitchDarkSignature;
	SvgPanel *panelPolaritySwitchDeepBlueSignature;
	SvgPanel *panelPolaritySwitchTitaniumSignature;
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
		// Creamy panel.
		panelPolaritySwitchCreamy = new SvgPanel();
		panelPolaritySwitchCreamy->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Polarity_Switch_Creamy.svg")));
		panelPolaritySwitchCreamy->visible = !rack::settings::preferDarkPanels;
		addChild(panelPolaritySwitchCreamy);
		// Stage Repro panel.
		panelPolaritySwitchStageRepro = new SvgPanel();
		panelPolaritySwitchStageRepro->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Polarity_Switch_Stage_Repro.svg")));
		panelPolaritySwitchStageRepro->visible = false;
		addChild(panelPolaritySwitchStageRepro);
		// Absolute Night panel.
		panelPolaritySwitchAbsoluteNight = new SvgPanel();
		panelPolaritySwitchAbsoluteNight->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Polarity_Switch_Absolute_Night.svg")));
		panelPolaritySwitchAbsoluteNight->visible = rack::settings::preferDarkPanels;
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
		// Titanium Signature panel.
		panelPolaritySwitchTitaniumSignature = new SvgPanel();
		panelPolaritySwitchTitaniumSignature->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Polarity_Switch_Titanium_Signature.svg")));
		panelPolaritySwitchTitaniumSignature->visible = false;
		addChild(panelPolaritySwitchTitaniumSignature);
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
			// Possible panels.
			panelPolaritySwitchCreamy->visible = (module->Model == 0);
			panelPolaritySwitchStageRepro->visible = (module->Model == 1);
			panelPolaritySwitchAbsoluteNight->visible = (module->Model == 2);
			panelPolaritySwitchDarkSignature->visible = (module->Model == 3);
			panelPolaritySwitchDeepBlueSignature->visible = (module->Model == 4);
			panelPolaritySwitchTitaniumSignature->visible = (module->Model == 5);
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
			panelPolaritySwitchCreamy->visible = !rack::settings::preferDarkPanels;
			panelPolaritySwitchStageRepro->visible = false;
			panelPolaritySwitchAbsoluteNight->visible = rack::settings::preferDarkPanels;
			panelPolaritySwitchDarkSignature->visible = false;
			panelPolaritySwitchDeepBlueSignature->visible = false;
			panelPolaritySwitchTitaniumSignature->visible = false;
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
		PolaritySwitchModule *module = dynamic_cast<PolaritySwitchModule*>(this->module);

		menu->addChild(new MenuSeparator);

		PolaritySwitchModelSubMenuItems *polswModelSubMenuItems = new PolaritySwitchModelSubMenuItems;
		polswModelSubMenuItems->text = "Model";
		polswModelSubMenuItems->rightText = RIGHT_ARROW;
		polswModelSubMenuItems->module = module;
		menu->addChild(polswModelSubMenuItems);

		menu->addChild(new MenuSeparator);

		MenuLabel *upperPolaritySwitchLabel = new MenuLabel();
		upperPolaritySwitchLabel->text = "UPPER PART:";
		menu->addChild(upperPolaritySwitchLabel);

		UpperKeepVoltage *upperKeepVoltage = new UpperKeepVoltage;
		upperKeepVoltage->text = "Keep IN1 voltage (default)";
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
		lowerPolaritySwitchLabel->text = "LOWER PART:";
		menu->addChild(lowerPolaritySwitchLabel);

		LowerKeepVoltage *lowerKeepVoltage = new LowerKeepVoltage;
		lowerKeepVoltage->text = "Keep IN2 voltage (default)";
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
