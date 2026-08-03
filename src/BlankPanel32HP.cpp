////////////////////////////////////////////////////////////////////////////////////////////////////
////// Blank Panel 32 HP module ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

struct OhmerBlank32 : Module {
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
	OhmerBlank32() {
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

struct OhmerBlank32Background : TransparentWidget {
	OhmerBlank32 *module;

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

struct OhmerBlank32Widget : ModuleWidget {
	// Panel (transparent widget).
	OhmerBlank32Background *blankPanel;
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

	OhmerBlank32Widget(OhmerBlank32 *module) {
		setModule(module);
		// 32 HP module, no SVG panel loaded, but using transparent widget instead.
		box.size = Vec(32 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
			blankPanel = new OhmerBlank32Background();
			blankPanel->box.size = box.size;
			blankPanel->module = module;
			addChild(blankPanel);
		}
		// This 32 HP module uses 4 screws (may are silver or gold).
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
		OhmerBlank32 *module = dynamic_cast<OhmerBlank32*>(this->module);
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

	struct OB32ThemeCreamyMenuItem : MenuItem {
		OhmerBlank32 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 0; // Creamy.
			module->panelBgColor = tblpanelBgColor[0];
		}
	};

	struct OB32ThemeStageReproMenuItem : MenuItem {
		OhmerBlank32 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 1; // Stage Repro.
			module->panelBgColor = tblpanelBgColor[1];
		}
	};

	struct OB32ThemeAbsoluteNightMenuItem : MenuItem {
		OhmerBlank32 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 2; // Absolute Night.
			module->panelBgColor = tblpanelBgColor[2];
		}
	};

	struct OB32ThemeDarkSignatureMenuItem : MenuItem {
		OhmerBlank32 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 3; // Dark Signature.
			module->panelBgColor = tblpanelBgColor[3];
		}
	};

	struct OB32ThemeDeepblueSignatureMenuItem : MenuItem {
		OhmerBlank32 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 4; // Deepblue Signature.
			module->panelBgColor = tblpanelBgColor[4];
		}
	};

	struct OB32ThemeTitaniumSignatureMenuItem : MenuItem {
		OhmerBlank32 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 5; // Titanium Signature.
			module->panelBgColor = tblpanelBgColor[5];
		}
	};

	struct OB32ThemeMenuItems : MenuItem {
		OhmerBlank32 *module;
		Menu *createChildMenu() override {
			Menu *menu = new Menu;

			OB32ThemeCreamyMenuItem *ob32themecreamymenuitem = new OB32ThemeCreamyMenuItem;
			ob32themecreamymenuitem->text = "Creamy";
			ob32themecreamymenuitem->rightText = CHECKMARK(module->Theme == 0);
			ob32themecreamymenuitem->module = module;
			menu->addChild(ob32themecreamymenuitem);

			OB32ThemeStageReproMenuItem *ob32themestagerepromenuitem = new OB32ThemeStageReproMenuItem;
			ob32themestagerepromenuitem->text = "Stage Repro";
			ob32themestagerepromenuitem->rightText = CHECKMARK(module->Theme == 1);
			ob32themestagerepromenuitem->module = module;
			menu->addChild(ob32themestagerepromenuitem);

			OB32ThemeAbsoluteNightMenuItem *ob32themeabsolutenightmenuitem = new OB32ThemeAbsoluteNightMenuItem;
			ob32themeabsolutenightmenuitem->text = "Absolute Night";
			ob32themeabsolutenightmenuitem->rightText = CHECKMARK(module->Theme == 2);
			ob32themeabsolutenightmenuitem->module = module;
			menu->addChild(ob32themeabsolutenightmenuitem);

			OB32ThemeDarkSignatureMenuItem *ob32themedarksignaturemenuitem = new OB32ThemeDarkSignatureMenuItem;
			ob32themedarksignaturemenuitem->text = "Dark \"Signature\"";
			ob32themedarksignaturemenuitem->rightText = CHECKMARK(module->Theme == 3);
			ob32themedarksignaturemenuitem->module = module;
			menu->addChild(ob32themedarksignaturemenuitem);

			OB32ThemeDeepblueSignatureMenuItem *ob32themedeepbluesignaturemenuitem = new OB32ThemeDeepblueSignatureMenuItem;
			ob32themedeepbluesignaturemenuitem->text = "Deepblue \"Signature\"";
			ob32themedeepbluesignaturemenuitem->rightText = CHECKMARK(module->Theme == 4);
			ob32themedeepbluesignaturemenuitem->module = module;
			menu->addChild(ob32themedeepbluesignaturemenuitem);

			OB32ThemeTitaniumSignatureMenuItem *ob32themetitaniumsignaturemenuitem = new OB32ThemeTitaniumSignatureMenuItem;
			ob32themetitaniumsignaturemenuitem->text = "Titanium \"Signature\"";
			ob32themetitaniumsignaturemenuitem->rightText = CHECKMARK(module->Theme == 5);
			ob32themetitaniumsignaturemenuitem->module = module;
			menu->addChild(ob32themetitaniumsignaturemenuitem);

			return menu;
		}
	};

	void appendContextMenu(Menu *menu) override {
		OhmerBlank32 *module = dynamic_cast<OhmerBlank32*>(this->module);
		if (!module)
			return;
		menu->addChild(new MenuSeparator);
		OB32ThemeMenuItems *ob32thememenuitems = new OB32ThemeMenuItems;
		ob32thememenuitems->text = "Model";
		ob32thememenuitems->rightText = RIGHT_ARROW;
		ob32thememenuitems->module = module;
		menu->addChild(ob32thememenuitems);
	}

};

Model *modelBlankPanel32 = createModel<OhmerBlank32, OhmerBlank32Widget>("OhmerBlank32");
