////////////////////////////////////////////////////////////////////////////////////////////////////
////// Blank Panel 1 HP module /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

struct OhmerBlank1 : Module {
	enum ParamIds {
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	// Current selected plate model (color).
	uint8_t Theme; // 0 = Creamy, 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Titanium Signature.

	// Panel color (default is Creamy).
	NVGcolor panelBgColor = nvgRGB(0xd2, 0xd2, 0xcd);

	// Module constructor.
	OhmerBlank1() {
		Theme = rack::settings::preferDarkPanels ? 2 : 0; // Assuming default is "Creamy" or "Absolute Night" (depending "Use dark panels if available" option, from "View" menu).
		panelBgColor = tblpanelBgColor[Theme];
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////// JSON DATAS SERIALIZATION /////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// These datas are saved to .vcv patch files (including "autosave/patch.json"), also they're used to duplicate the module, or via Copy/Paste feature between Vektor modules.
	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Model", json_integer(Theme));
		return rootJ;
	}

	/////////////////////////////////////////////
	//// RETRIEVE SERIALIZED VARIABLES/FLAGS ////
	/////////////////////////////////////////////

	void dataFromJson(json_t *rootJ) override {
		json_t *ThemeJ = json_object_get(rootJ, "Model");
		if (ThemeJ) {
			Theme = json_integer_value(ThemeJ);
			panelBgColor = tblpanelBgColor[Theme];
		}
	}

};

///////////////////////////////////////////////// PANEL BACKGROUND COLOR /////////////////////////////////////////////////

struct OhmerBlank1Background : TransparentWidget {
	OhmerBlank1 *module;

	void draw(const DrawArgs &args) override {
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
		if (module)
			nvgFillColor(args.vg, module->panelBgColor);
			else nvgFillColor(args.vg, rack::settings::preferDarkPanels ? nvgRGB(0x00, 0x00, 0x00) : nvgRGB(0xd2, 0xd2, 0xcd));
		nvgFill(args.vg);
	}

};

///////////////////////////////////////////////// MODULE WIDGET SECTION /////////////////////////////////////////////////

struct OhmerBlank1Widget : ModuleWidget {
	// Panel (transparent widget).
	OhmerBlank1Background *blankPanel;
	// Silver Torx screws.
	SvgScrew *bottomScrewSilver;
	SvgScrew *topScrewSilver;
	// Gold Torx screws.
	SvgScrew *bottomScrewGold;
	SvgScrew *topScrewGold;

	OhmerBlank1Widget(OhmerBlank1 *module) {
		setModule(module);
		// 1 HP module, no SVG panel loaded, but using transparent widget instead.
		box.size = Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
			blankPanel = new OhmerBlank1Background();
			blankPanel->box.size = box.size;
			blankPanel->module = module;
			addChild(blankPanel);
		}
		// This 1 HP module uses two screws only.
		// Top gold screw.
		topScrewGold = createWidget<Torx_Gold>(Vec(0, 0));
		addChild(topScrewGold);
		// Top silver screw.
		topScrewSilver = createWidget<Torx_Silver>(Vec(0, 0));
		addChild(topScrewSilver);
		// Bottom gold screw.
		bottomScrewGold = createWidget<Torx_Gold>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomScrewGold);
		// Bottom silver screw.
		bottomScrewSilver = createWidget<Torx_Silver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomScrewSilver);
	}

	void step() override {
		OhmerBlank1 *module = dynamic_cast<OhmerBlank1*>(this->module);
		if (!module) {
			// Probably from module browser...
			// By default, silver screws are visible for default Creamy or Absolute Night...
			// ...and, of course, golden screws are hidden.
			topScrewGold->visible = false;
			bottomScrewGold->visible = false;
			topScrewSilver->visible = true;
			bottomScrewSilver->visible = true;
			return;
		}
		else {
			// Torx screws metal (silver, gold) are visible or hidden, depending selected model (from module's context-menu).
			// Silver Torx screws are visible only for non-"Signature" modules (Creamy, Stage Repro or Absolute Night).
			topScrewSilver->visible = (module->Theme < 3);
			bottomScrewSilver->visible = (module->Theme < 3);
			// Gold Torx screws are visible only for "Signature" modules (Dark Signature, Deepblue Signature or Titanium Signature).
			topScrewGold->visible = (module->Theme > 2);
			bottomScrewGold->visible = (module->Theme > 2);
		}
		ModuleWidget::step();
	}

	///////////////////////////////////////////////////// CONTEXTUAL MENU - THEME //////////////////////////////////////////////////////

	struct OB1ThemeCreamyMenuItem : MenuItem {
		OhmerBlank1 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 0; // Creamy.
			module->panelBgColor = tblpanelBgColor[0];
		}
	};

	struct OB1ThemeStageReproMenuItem : MenuItem {
		OhmerBlank1 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 1; // Stage Repro.
			module->panelBgColor = tblpanelBgColor[1];
		}
	};

	struct OB1ThemeAbsoluteNightMenuItem : MenuItem {
		OhmerBlank1 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 2; // Absolute Night.
			module->panelBgColor = tblpanelBgColor[2];
		}
	};

	struct OB1ThemeDarkSignatureMenuItem : MenuItem {
		OhmerBlank1 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 3; // Dark Signature.
			module->panelBgColor = tblpanelBgColor[3];
		}
	};

	struct OB1ThemeDeepblueSignatureMenuItem : MenuItem {
		OhmerBlank1 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 4; // Deepblue Signature.
			module->panelBgColor = tblpanelBgColor[4];
		}
	};

	struct OB1ThemeTitaniumSignatureMenuItem : MenuItem {
		OhmerBlank1 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 5; // Titanium Signature.
			module->panelBgColor = tblpanelBgColor[5];
		}
	};

	struct OB1ThemeMenuItems : MenuItem {
		OhmerBlank1 *module;
		Menu *createChildMenu() override {
			Menu *menu = new Menu;

			OB1ThemeCreamyMenuItem *ob1themecreamymenuitem = new OB1ThemeCreamyMenuItem;
			ob1themecreamymenuitem->text = "Creamy";
			ob1themecreamymenuitem->rightText = CHECKMARK(module->Theme == 0);
			ob1themecreamymenuitem->module = module;
			menu->addChild(ob1themecreamymenuitem);

			OB1ThemeStageReproMenuItem *ob1themestagerepromenuitem = new OB1ThemeStageReproMenuItem;
			ob1themestagerepromenuitem->text = "Stage Repro";
			ob1themestagerepromenuitem->rightText = CHECKMARK(module->Theme == 1);
			ob1themestagerepromenuitem->module = module;
			menu->addChild(ob1themestagerepromenuitem);

			OB1ThemeAbsoluteNightMenuItem *ob1themeabsolutenightmenuitem = new OB1ThemeAbsoluteNightMenuItem;
			ob1themeabsolutenightmenuitem->text = "Absolute Night";
			ob1themeabsolutenightmenuitem->rightText = CHECKMARK(module->Theme == 2);
			ob1themeabsolutenightmenuitem->module = module;
			menu->addChild(ob1themeabsolutenightmenuitem);

			OB1ThemeDarkSignatureMenuItem *ob1themedarksignaturemenuitem = new OB1ThemeDarkSignatureMenuItem;
			ob1themedarksignaturemenuitem->text = "Dark \"Signature\"";
			ob1themedarksignaturemenuitem->rightText = CHECKMARK(module->Theme == 3);
			ob1themedarksignaturemenuitem->module = module;
			menu->addChild(ob1themedarksignaturemenuitem);

			OB1ThemeDeepblueSignatureMenuItem *ob1themedeepbluesignaturemenuitem = new OB1ThemeDeepblueSignatureMenuItem;
			ob1themedeepbluesignaturemenuitem->text = "Deepblue \"Signature\"";
			ob1themedeepbluesignaturemenuitem->rightText = CHECKMARK(module->Theme == 4);
			ob1themedeepbluesignaturemenuitem->module = module;
			menu->addChild(ob1themedeepbluesignaturemenuitem);

			OB1ThemeTitaniumSignatureMenuItem *ob1themetitaniumsignaturemenuitem = new OB1ThemeTitaniumSignatureMenuItem;
			ob1themetitaniumsignaturemenuitem->text = "Titanium \"Signature\"";
			ob1themetitaniumsignaturemenuitem->rightText = CHECKMARK(module->Theme == 5);
			ob1themetitaniumsignaturemenuitem->module = module;
			menu->addChild(ob1themetitaniumsignaturemenuitem);

			return menu;
		}
	};

	void appendContextMenu(Menu *menu) override {
		OhmerBlank1 *module = dynamic_cast<OhmerBlank1*>(this->module);
		if (!module)
			return;
		menu->addChild(new MenuSeparator);
		OB1ThemeMenuItems *ob1thememenuitems = new OB1ThemeMenuItems;
		ob1thememenuitems->text = "Model";
		ob1thememenuitems->rightText = RIGHT_ARROW;
		ob1thememenuitems->module = module;
		menu->addChild(ob1thememenuitems);
	}

};

Model *modelBlankPanel1 = createModel<OhmerBlank1, OhmerBlank1Widget>("OhmerBlank1");
