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

	// Current selected model (GUI theme variation).
	int Theme = 0; // 0 = Creamy, 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Titanium Signature.
	int prevTheme = 0xff; // Used to detect model (theme) change.
	int portMetal = 0; // Used to select silver or golden jacks.

	// Output voltage settings for upper module and lower module.
	int UpperVoltage = 0; // 0 means unaltered IN voltage, 1 means output voltage(s) is/are forced to +5V, 2 means output voltage(s) is/are forced to +10V.
	int LowerVoltage = 0; // 0 means unaltered IN voltage, 1 means output voltage(s) is/are forced to +5V, 2 means output voltage(s) is/are forced to +10V.

	// Module constructor.
	PolaritySwitchModule() {
		Theme = rack::settings::preferDarkPanels ? 2 : 0; // Assuming default is "Creamy" or "Absolute Night" (depending "Use dark panels if available" option, from "View" menu).
		prevTheme = 0xff; // To force change theme as soon as possible.
		portMetal = (Theme > 2) ? 1 : 0;
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configInput(INPUT_1, "IN1 voltage");
		configOutput(OUTPUT_P1, "IN2 >= 0: sent to this P1");
		configOutput(OUTPUT_N1, "IN2 < 0: sent (absolute) to this N1");
		configInput(INPUT_2, "IN2 voltage");
		configOutput(OUTPUT_P2, "IN2 >= 0: sent to this P2");
		configOutput(OUTPUT_N2, "IN2 < 0: sent (absolute) to this N2");
		UpperVoltage = 0;
		LowerVoltage = 0;
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// JSON DATAS SERIALIZATION /////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// These datas are saved to .vcv patch files (including "autosave/patch.json"), also they're used to duplicate the module, or via Copy/Paste feature between Vektor modules.
	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Model", json_integer(Theme)); // Save current theme (model).
		json_object_set_new(rootJ, "UpperVoltage", json_integer(UpperVoltage));
		json_object_set_new(rootJ, "LowerVoltage", json_integer(LowerVoltage));
		return rootJ;
	}

	/////////////////////////////////////////////
	//// RETRIEVE SERIALIZED VARIABLES/FLAGS ////
	/////////////////////////////////////////////

	void dataFromJson(json_t *rootJ) override {
		// Retrieving saved theme (Model).
		json_t *ThemeJ = json_object_get(rootJ, "Model");
		if (ThemeJ) {
			Theme = json_integer_value(ThemeJ);
			portMetal = (Theme > 2) ? 1 : 0; // first three models use silver connectors (0), last three ("Signature"-line panels) use golden (1).
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

///////////////////////////////////////////////// MODULE WIDGET SECTION /////////////////////////////////////////////////

struct PolaritySwitchWidget : ModuleWidget {
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
		if (!module) {
			// The module isn't instantiated: probably from module browser...
			// Default presented model is "Creamy" or "Absolute Night" (depending "Use dark panels if available" VCV Rack 2's global option, from "View" menu).
			if (rack::settings::preferDarkPanels)
				setPanel(createPanel(asset::plugin(pluginInstance, "res/Polarity_Switch_Absolute_Night.svg")));
				else setPanel(createPanel(asset::plugin(pluginInstance, "res/Polarity_Switch_Creamy.svg")));
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
			// To avoid potential crash...
			return;
		}
		// Current panel.
		if (module->Theme != module->prevTheme) {
			// Switching to different panel.
			switch (module->Theme) {
				case 0:
					// Creamy.
					setPanel(createPanel(asset::plugin(pluginInstance, "res/Polarity_Switch_Creamy.svg")));
					break;
				case 1:
					// Stage Repro.
					setPanel(createPanel(asset::plugin(pluginInstance, "res/Polarity_Switch_Stage_Repro.svg")));
					break;
				case 2:
					// Absolute Night.
					setPanel(createPanel(asset::plugin(pluginInstance, "res/Polarity_Switch_Absolute_Night.svg")));
					break;
				case 3:
					// Dark "Signature".
					setPanel(createPanel(asset::plugin(pluginInstance, "res/Polarity_Switch_Dark_Signature.svg")));
					break;
				case 4:
					// Deepblue "Signature".
					setPanel(createPanel(asset::plugin(pluginInstance, "res/Polarity_Switch_Deepblue_Signature.svg")));
					break;
				case 5:
					// Titanium "Signature".
					setPanel(createPanel(asset::plugin(pluginInstance, "res/Polarity_Switch_Titanium_Signature.svg")));
			}
			// Screws metal texture.
			if (module->portMetal == 1) {
				// "Signature"-line panels.
				// Screws: golden are visible...
				topLeftScrewGold->visible = true;
				topRightScrewGold->visible = true;
				bottomLeftScrewGold->visible = true;
				bottomRightScrewGold->visible = true;
				// ...silver are hidden.
				topLeftScrewSilver->visible = false;
				topRightScrewSilver->visible = false;
				bottomLeftScrewSilver->visible = false;
				bottomRightScrewSilver->visible = false;
			}
			else {
				// Non-"Signature" panels.
				// Screws: silver are visible...
				topLeftScrewSilver->visible = true;
				topRightScrewSilver->visible = true;
				bottomLeftScrewSilver->visible = true;
				bottomRightScrewSilver->visible = true;
				// ...golden are hidden.
				topLeftScrewGold->visible = false;
				topRightScrewGold->visible = false;
				bottomLeftScrewGold->visible = false;
				bottomRightScrewGold->visible = false;
			}
			// Align prevTheme variable along new theme.
			module->prevTheme = module->Theme;
		}
		//
		ModuleWidget::step();
	}

	///////////////////////////////////////////////////// CONTEXTUAL MENU - THEME //////////////////////////////////////////////////////

	struct PSThemeCreamyMenuItem : MenuItem {
		PolaritySwitchModule *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 0; // Creamy.
			module->portMetal = 0; // Silver connectors.
		}
	};

	struct PSThemeStageReproMenuItem : MenuItem {
		PolaritySwitchModule *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 1; // Stage Repro.
			module->portMetal = 0; // Silver connectors.
		}
	};

	struct PSThemeAbsoluteNightMenuItem : MenuItem {
		PolaritySwitchModule *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 2; // Absolute Night.
			module->portMetal = 0; // Silver connectors.
		}
	};

	struct PSThemeDarkSignatureMenuItem : MenuItem {
		PolaritySwitchModule *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 3; // Dark Signature.
			module->portMetal = 1; // Golden connectors.
		}
	};

	struct PSThemeDeepblueSignatureMenuItem : MenuItem {
		PolaritySwitchModule *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 4; // Deepblue Signature.
			module->portMetal = 1; // Golden connectors.
		}
	};

	struct PSThemeTitaniumSignatureMenuItem : MenuItem {
		PolaritySwitchModule *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 5; // Titanium Signature.
			module->portMetal = 1; // Golden connectors.
		}
	};

	struct PSThemeSubmenuItems : MenuItem {
		PolaritySwitchModule *module;
		Menu *createChildMenu() override {
			Menu *menu = new Menu;

			PSThemeCreamyMenuItem *psthemecreamymenuitem = new PSThemeCreamyMenuItem;
			psthemecreamymenuitem->text = "Creamy";
			psthemecreamymenuitem->rightText = CHECKMARK(module->Theme == 0);
			psthemecreamymenuitem->module = module;
			menu->addChild(psthemecreamymenuitem);

			PSThemeStageReproMenuItem *psthemestagerepromenuitem = new PSThemeStageReproMenuItem;
			psthemestagerepromenuitem->text = "Stage Repro";
			psthemestagerepromenuitem->rightText = CHECKMARK(module->Theme == 1);
			psthemestagerepromenuitem->module = module;
			menu->addChild(psthemestagerepromenuitem);

			PSThemeAbsoluteNightMenuItem *psthemeabsolutenightmenuitem = new PSThemeAbsoluteNightMenuItem;
			psthemeabsolutenightmenuitem->text = "Absolute Night";
			psthemeabsolutenightmenuitem->rightText = CHECKMARK(module->Theme == 2);
			psthemeabsolutenightmenuitem->module = module;
			menu->addChild(psthemeabsolutenightmenuitem);

			PSThemeDarkSignatureMenuItem *psthemedarksignaturemenuitem = new PSThemeDarkSignatureMenuItem;
			psthemedarksignaturemenuitem->text = "Dark \"Signature\"";
			psthemedarksignaturemenuitem->rightText = CHECKMARK(module->Theme == 3);
			psthemedarksignaturemenuitem->module = module;
			menu->addChild(psthemedarksignaturemenuitem);

			PSThemeDeepblueSignatureMenuItem *psthemedeepbluesignaturemenuitem = new PSThemeDeepblueSignatureMenuItem;
			psthemedeepbluesignaturemenuitem->text = "Deepblue \"Signature\"";
			psthemedeepbluesignaturemenuitem->rightText = CHECKMARK(module->Theme == 4);
			psthemedeepbluesignaturemenuitem->module = module;
			menu->addChild(psthemedeepbluesignaturemenuitem);

			PSThemeTitaniumSignatureMenuItem *psthemetitaniumsignaturemenuitem = new PSThemeTitaniumSignatureMenuItem;
			psthemetitaniumsignaturemenuitem->text = "Titanium \"Signature\"";
			psthemetitaniumsignaturemenuitem->rightText = CHECKMARK(module->Theme == 5);
			psthemetitaniumsignaturemenuitem->module = module;
			menu->addChild(psthemetitaniumsignaturemenuitem);

			return menu;
		}

	};

	/////////////////////////////////////////// CONTEXTUAL MENU - UPPER MODULE //////////////////////////////////////////////////

	struct UpperKeepVoltage : MenuItem {
		PolaritySwitchModule *module;
		void onAction(const ActionEvent& e) override {
			module->UpperVoltage = 0; // Keep voltage.
		}
	};

	struct UpperForce5V : MenuItem {
		PolaritySwitchModule *module;
		void onAction(const ActionEvent& e) override {
			module->UpperVoltage = 1; // Force all outputs to +5V.
		}
	};

	struct UpperForce10V : MenuItem {
		PolaritySwitchModule *module;
		void onAction(const ActionEvent& e) override {
			module->UpperVoltage = 2; // Force all outputs to +10V.
		}
	};

	/////////////////////////////////////////// CONTEXTUAL MENU - LOWER MODULE //////////////////////////////////////////////////

	struct LowerKeepVoltage : MenuItem {
		PolaritySwitchModule *module;
		void onAction(const ActionEvent& e) override {
			module->LowerVoltage = 0; // Keep voltage.
		}
	};

	struct LowerForce5V : MenuItem {
		PolaritySwitchModule *module;
		void onAction(const ActionEvent& e) override {
			module->LowerVoltage = 1; // Force all outputs to +5V.
		}
	};

	struct LowerForce10V : MenuItem {
		PolaritySwitchModule *module;
		void onAction(const ActionEvent& e) override {
			module->LowerVoltage = 2; // Force all outputs to +10V.
		}
	};

	void appendContextMenu(Menu *menu) override {
		PolaritySwitchModule *module = dynamic_cast<PolaritySwitchModule*>(this->module);

		if (!module)
			return;

		menu->addChild(new MenuSeparator);

		PSThemeSubmenuItems *psthemesubmenuitems = new PSThemeSubmenuItems;
		psthemesubmenuitems->text = "Model";
		psthemesubmenuitems->rightText = RIGHT_ARROW;
		psthemesubmenuitems->module = module;
		menu->addChild(psthemesubmenuitems);

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
