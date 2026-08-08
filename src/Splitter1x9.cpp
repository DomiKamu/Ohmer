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

	// Current selected model (GUI theme variation).
	int Theme = 0; // 0 = Creamy, 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Titanium Signature.
	int prevTheme = 0xff; // Used to detect model (theme) change.
	int portMetal = 0; // Used to select silver or golden jacks.

	// Module constructor.
	SplitterModule() {
		Theme = rack::settings::preferDarkPanels ? 2 : 0; // Assuming default is "Creamy" or "Absolute Night" (depending "Use dark panels if available" option, from "View" menu).
		prevTheme = 0xff; // To force change theme as soon as possible.
		portMetal = (Theme > 2) ? 1 : 0;
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
	}

	void process(const ProcessArgs &args) override {
		if (inputs[MAIN_INPUT].isConnected()) {
			int nChannels = inputs[MAIN_INPUT].getChannels(); // Added for polyphonic.
			for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
				// Per output port.
				for (int c = 0;  c < nChannels; c++) {
				// then per polyphonic channel (1 channel if monophonic cable on input).
					float raw_input_voltage = inputs[MAIN_INPUT].getVoltage(c);
					float splitted_out_voltage = clamp(raw_input_voltage, -10.f, 10.f); // -10V/+10V range voltage on Eurorack.
					outputs[i].setVoltage(splitted_out_voltage, c);
				}
				outputs[i].setChannels(nChannels);
			}
		}
		else {
			// If input jack isn't connected, assuming it's a monophonic module instead. Also, no voltage to output jacks, and unlit LED.
			for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
				outputs[i].setVoltage(0.f);
				outputs[i].setChannels(1);
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// JSON DATAS SERIALIZATION /////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// These datas are saved to .vcv patch files (including "autosave/patch.json"), also they're used to duplicate the module, or via Copy/Paste feature between Vektor modules.
	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Model", json_integer(Theme)); // Save current theme (model).
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
	}

};

///////////////////////////////////////////////// MODULE WIDGET SECTION /////////////////////////////////////////////////

struct SplitterWidget : ModuleWidget {
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
		if (!module) {
			// The module isn't instantiated: probably from module browser...
			// Default presented model is "Creamy" or "Absolute Night" (depending "Use dark panels if available" VCV Rack 2's global option, from "View" menu).
			if (rack::settings::preferDarkPanels)
				setPanel(createPanel(asset::plugin(pluginInstance, "res/Splitter1x9_Absolute_Night.svg")));
				else setPanel(createPanel(asset::plugin(pluginInstance, "res/Splitter1x9_Creamy.svg")));
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
					setPanel(createPanel(asset::plugin(pluginInstance, "res/Splitter1x9_Creamy.svg")));
					break;
				case 1:
					// Stage Repro.
					setPanel(createPanel(asset::plugin(pluginInstance, "res/Splitter1x9_Stage_Repro.svg")));
					break;
				case 2:
					// Absolute Night.
					setPanel(createPanel(asset::plugin(pluginInstance, "res/Splitter1x9_Absolute_Night.svg")));
					break;
				case 3:
					// Dark "Signature".
					setPanel(createPanel(asset::plugin(pluginInstance, "res/Splitter1x9_Dark_Signature.svg")));
					break;
				case 4:
					// Deepblue "Signature".
					setPanel(createPanel(asset::plugin(pluginInstance, "res/Splitter1x9_Deepblue_Signature.svg")));
					break;
				case 5:
					// Titanium "Signature".
					setPanel(createPanel(asset::plugin(pluginInstance, "res/Splitter1x9_Titanium_Signature.svg")));
			}
			// Screws metal texture.
			bool b_MetalIsGold = (module->portMetal == 1);
			// Metal for screws.
			// Golden screws.
			topLeftScrewGold->visible = b_MetalIsGold;
			topRightScrewGold->visible = b_MetalIsGold;
			bottomLeftScrewGold->visible = b_MetalIsGold;
			bottomRightScrewGold->visible = b_MetalIsGold;
			// Silver screws.
			topLeftScrewSilver->visible = !b_MetalIsGold;
			topRightScrewSilver->visible = !b_MetalIsGold;
			bottomLeftScrewSilver->visible = !b_MetalIsGold;
			bottomRightScrewSilver->visible = !b_MetalIsGold;
			// Align "prevTheme" variable along new selected theme.
			module->prevTheme = module->Theme;
		}
		//
		ModuleWidget::step();
	}

	///////////////////////////////////////////////////// CONTEXTUAL MENU - THEME //////////////////////////////////////////////////////

	struct SplitterCreamyMenu : MenuItem {
		SplitterModule *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 0; // Creamy.
			module->portMetal = 0; // Silver connectors.
		}
	};

	struct SplitterStageReproMenu : MenuItem {
		SplitterModule *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 1; // Stage Repro.
			module->portMetal = 0; // Silver connectors.
		}
	};

	struct SplitterAbsoluteNightMenu : MenuItem {
		SplitterModule *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 2; // Absolute Night.
			module->portMetal = 0; // Silver connectors.
		}
	};

	struct SplitterDarkSignatureMenu : MenuItem {
		SplitterModule *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 3; // Dark Signature.
			module->portMetal = 1; // Golden connectors.
		}
	};

	struct SplitterDeepblueSignatureMenu : MenuItem {
		SplitterModule *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 4; // Deepblue Signature.
			module->portMetal = 1; // Golden connectors.
		}
	};

	struct SplitterTitaniumSignatureMenu : MenuItem {
		SplitterModule *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 5; // Titanium Signature.
			module->portMetal = 1; // Golden connectors.
		}
	};

	struct SplitterSubMenuItems : MenuItem {
		SplitterModule *module;
		Menu *createChildMenu() override {
			Menu *menu = new Menu;

			SplitterCreamyMenu *splittercreamymenu = new SplitterCreamyMenu;
			splittercreamymenu->text = "Creamy";
			splittercreamymenu->rightText = CHECKMARK(module->Theme == 0);
			splittercreamymenu->module = module;
			menu->addChild(splittercreamymenu);

			SplitterStageReproMenu *splitterstagerepromenu = new SplitterStageReproMenu;
			splitterstagerepromenu->text = "Stage Repro";
			splitterstagerepromenu->rightText = CHECKMARK(module->Theme == 1);
			splitterstagerepromenu->module = module;
			menu->addChild(splitterstagerepromenu);

			SplitterAbsoluteNightMenu *splitterabsolutenightmenu = new SplitterAbsoluteNightMenu;
			splitterabsolutenightmenu->text = "Absolute Night";
			splitterabsolutenightmenu->rightText = CHECKMARK(module->Theme == 2);
			splitterabsolutenightmenu->module = module;
			menu->addChild(splitterabsolutenightmenu);

			SplitterDarkSignatureMenu *splitterdarksignaturemenu = new SplitterDarkSignatureMenu;
			splitterdarksignaturemenu->text = "Dark \"Signature\"";
			splitterdarksignaturemenu->rightText = CHECKMARK(module->Theme == 3);
			splitterdarksignaturemenu->module = module;
			menu->addChild(splitterdarksignaturemenu);

			SplitterDeepblueSignatureMenu *splitterdeepbluesignaturemenu = new SplitterDeepblueSignatureMenu;
			splitterdeepbluesignaturemenu->text = "Deepblue \"Signature\"";
			splitterdeepbluesignaturemenu->rightText = CHECKMARK(module->Theme == 4);
			splitterdeepbluesignaturemenu->module = module;
			menu->addChild(splitterdeepbluesignaturemenu);

			SplitterTitaniumSignatureMenu *splittertitaniumsignaturemenu = new SplitterTitaniumSignatureMenu;
			splittertitaniumsignaturemenu->text = "Titanium \"Signature\"";
			splittertitaniumsignaturemenu->rightText = CHECKMARK(module->Theme == 5);
			splittertitaniumsignaturemenu->module = module;
			menu->addChild(splittertitaniumsignaturemenu);

			return menu;
		}
	};

	void appendContextMenu(Menu *menu) override {
		SplitterModule *module = dynamic_cast<SplitterModule*>(this->module);

		if (!module)
			return;

		menu->addChild(new MenuSeparator);

		SplitterSubMenuItems *spltrSubMenuItems = new SplitterSubMenuItems;
		spltrSubMenuItems->text = "Model";
		spltrSubMenuItems->rightText = RIGHT_ARROW;
		spltrSubMenuItems->module = module;
		menu->addChild(spltrSubMenuItems);
	}

};

Model *modelSplitter = createModel<SplitterModule, SplitterWidget>("SplitterModule");
