////////////////////////////////////////////////////////////////////////////////////////////////////
////// Blank Panel 8 HP module /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

struct OhmerBlank8 : Module {
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
	OhmerBlank8() {
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

struct OhmerBlank8Background : TransparentWidget {
	OhmerBlank8 *module;

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

struct OhmerBlank8Widget : ModuleWidget {
	// Panel (transparent widget).
	OhmerBlank8Background *blankPanel;
	// Gold Torx screws.
	SvgScrew *topLeftScrewGold;
	SvgScrew *topRightScrewGold;
	SvgScrew *bottomLeftScrewGold;
	SvgScrew *bottomRightScrewGold;
	// Silver Torx screws.
	SvgScrew *topLeftScrewSilver;
	SvgScrew *topRightScrewSilver;
	SvgScrew *bottomLeftScrewSilver;
	SvgScrew *bottomRightScrewSilver;

	OhmerBlank8Widget(OhmerBlank8 *module) {
		setModule(module);
		// 8 HP module, no SVG panel loaded, but using transparent widget instead.
		box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
			blankPanel = new OhmerBlank8Background();
			blankPanel->box.size = box.size;
			blankPanel->module = module;
			addChild(blankPanel);
		}
		// This 8 HP module uses 4 screws (may are silver or gold).
		// Top-left gold screw.
		topLeftScrewGold = createWidget<Torx_Gold>(Vec(RACK_GRID_WIDTH, 0));
		addChild(topLeftScrewGold);
		// Top-right gold screw.
		topRightScrewGold = createWidget<Torx_Gold>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0));
		addChild(topRightScrewGold);
		// Bottom-left gold screw.
		bottomLeftScrewGold = createWidget<Torx_Gold>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomLeftScrewGold);
		// Bottom-right gold screw.
		bottomRightScrewGold = createWidget<Torx_Gold>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomRightScrewGold);
		// Top-left silver screw.
		topLeftScrewSilver = createWidget<Torx_Silver>(Vec(RACK_GRID_WIDTH, 0));
		addChild(topLeftScrewSilver);
		// Top-right silver screw.
		topRightScrewSilver = createWidget<Torx_Silver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0));
		addChild(topRightScrewSilver);
		// Bottom-left silver screw.
		bottomLeftScrewSilver = createWidget<Torx_Silver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomLeftScrewSilver);
		// Bottom-right silver screw.
		bottomRightScrewSilver = createWidget<Torx_Silver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomRightScrewSilver);
	}

	void step() override {
		OhmerBlank8 *module = dynamic_cast<OhmerBlank8*>(this->module);
		if (!module) {
			// Probably from module browser...
			// By default, silver screws are visible for default Creamy or Absolute Night...
			// ...and, of course, golden screws are hidden.
			topLeftScrewGold->visible = false;
			topRightScrewGold->visible = false;
			bottomLeftScrewGold->visible = false;
			bottomRightScrewGold->visible = false;
			// ...and silver screws are visible.
			topLeftScrewSilver->visible = true;
			topRightScrewSilver->visible = true;
			bottomLeftScrewSilver->visible = true;
			bottomRightScrewSilver->visible = true;
			return;
		}
		else {
			// Torx screws metal (silver, gold) are visible or hidden, depending selected model (from module's context-menu).
			// Gold Torx screws are visible only for "Signature" modules (Dark Signature, Deepblue Signature or Titanium Signature).
			topLeftScrewGold->visible = (module->Theme > 2);
			topRightScrewGold->visible = (module->Theme > 2);
			bottomLeftScrewGold->visible = (module->Theme > 2);
			bottomRightScrewGold->visible = (module->Theme > 2);
			// Silver Torx screws are visible only for non-"Signature" modules (Creamy, Stage Repro or Absolute Night).
			topLeftScrewSilver->visible = (module->Theme < 3);
			topRightScrewSilver->visible = (module->Theme < 3);
			bottomLeftScrewSilver->visible = (module->Theme < 3);
			bottomRightScrewSilver->visible = (module->Theme < 3);
		}
		ModuleWidget::step();
	}

	///////////////////////////////////////////////////// CONTEXTUAL MENU - THEME //////////////////////////////////////////////////////

	struct OB8ThemeCreamyMenuItem : MenuItem {
		OhmerBlank8 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 0; // Creamy.
			module->panelBgColor = tblpanelBgColor[0];
		}
	};

	struct OB8ThemeStageReproMenuItem : MenuItem {
		OhmerBlank8 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 1; // Stage Repro.
			module->panelBgColor = tblpanelBgColor[1];
		}
	};

	struct OB8ThemeAbsoluteNightMenuItem : MenuItem {
		OhmerBlank8 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 2; // Absolute Night.
			module->panelBgColor = tblpanelBgColor[2];
		}
	};

	struct OB8ThemeDarkSignatureMenuItem : MenuItem {
		OhmerBlank8 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 3; // Dark Signature.
			module->panelBgColor = tblpanelBgColor[3];
		}
	};

	struct OB8ThemeDeepblueSignatureMenuItem : MenuItem {
		OhmerBlank8 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 4; // Deepblue Signature.
			module->panelBgColor = tblpanelBgColor[4];
		}
	};

	struct OB8ThemeTitaniumSignatureMenuItem : MenuItem {
		OhmerBlank8 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 5; // Titanium Signature.
			module->panelBgColor = tblpanelBgColor[5];
		}
	};

	struct OB8ThemeMenuItems : MenuItem {
		OhmerBlank8 *module;
		Menu *createChildMenu() override {
			Menu *menu = new Menu;

			OB8ThemeCreamyMenuItem *ob8themecreamymenuitem = new OB8ThemeCreamyMenuItem;
			ob8themecreamymenuitem->text = "Creamy";
			ob8themecreamymenuitem->rightText = CHECKMARK(module->Theme == 0);
			ob8themecreamymenuitem->module = module;
			menu->addChild(ob8themecreamymenuitem);

			OB8ThemeStageReproMenuItem *ob8themestagerepromenuitem = new OB8ThemeStageReproMenuItem;
			ob8themestagerepromenuitem->text = "Stage Repro";
			ob8themestagerepromenuitem->rightText = CHECKMARK(module->Theme == 1);
			ob8themestagerepromenuitem->module = module;
			menu->addChild(ob8themestagerepromenuitem);

			OB8ThemeAbsoluteNightMenuItem *ob8themeabsolutenightmenuitem = new OB8ThemeAbsoluteNightMenuItem;
			ob8themeabsolutenightmenuitem->text = "Absolute Night";
			ob8themeabsolutenightmenuitem->rightText = CHECKMARK(module->Theme == 2);
			ob8themeabsolutenightmenuitem->module = module;
			menu->addChild(ob8themeabsolutenightmenuitem);

			OB8ThemeDarkSignatureMenuItem *ob8themedarksignaturemenuitem = new OB8ThemeDarkSignatureMenuItem;
			ob8themedarksignaturemenuitem->text = "Dark \"Signature\"";
			ob8themedarksignaturemenuitem->rightText = CHECKMARK(module->Theme == 3);
			ob8themedarksignaturemenuitem->module = module;
			menu->addChild(ob8themedarksignaturemenuitem);

			OB8ThemeDeepblueSignatureMenuItem *ob8themedeepbluesignaturemenuitem = new OB8ThemeDeepblueSignatureMenuItem;
			ob8themedeepbluesignaturemenuitem->text = "Deepblue \"Signature\"";
			ob8themedeepbluesignaturemenuitem->rightText = CHECKMARK(module->Theme == 4);
			ob8themedeepbluesignaturemenuitem->module = module;
			menu->addChild(ob8themedeepbluesignaturemenuitem);

			OB8ThemeTitaniumSignatureMenuItem *ob8themetitaniumsignaturemenuitem = new OB8ThemeTitaniumSignatureMenuItem;
			ob8themetitaniumsignaturemenuitem->text = "Titanium \"Signature\"";
			ob8themetitaniumsignaturemenuitem->rightText = CHECKMARK(module->Theme == 5);
			ob8themetitaniumsignaturemenuitem->module = module;
			menu->addChild(ob8themetitaniumsignaturemenuitem);

			return menu;
		}
	};

	void appendContextMenu(Menu *menu) override {
		OhmerBlank8 *module = dynamic_cast<OhmerBlank8*>(this->module);
		if (!module)
			return;
		menu->addChild(new MenuSeparator);
		OB8ThemeMenuItems *ob8thememenuitems = new OB8ThemeMenuItems;
		ob8thememenuitems->text = "Model";
		ob8thememenuitems->rightText = RIGHT_ARROW;
		ob8thememenuitems->module = module;
		menu->addChild(ob8thememenuitems);
	}

};

Model *modelBlankPanel8 = createModel<OhmerBlank8, OhmerBlank8Widget>("OhmerBlank8");
