////////////////////////////////////////////////////////////////////////////////////////////////////
////// Blank Panel 2 HP module /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

struct OhmerBlank2 : Module {
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

	// Screws disposal.
	uint8_t screwsDisposal = 0;

	// Module constructor.
	OhmerBlank2() {
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
		json_object_set_new(rootJ, "screwsDisposal", json_integer(screwsDisposal));
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
		json_t *screwsDisposalJ = json_object_get(rootJ, "screwsDisposal");
		if (screwsDisposalJ)
			screwsDisposal = json_integer_value(screwsDisposalJ);
	}

};

///////////////////////////////////////////////// PANEL BACKGROUND COLOR /////////////////////////////////////////////////

struct OhmerBlank2Background : TransparentWidget {
	OhmerBlank2 *module;

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

struct OhmerBlank2Widget : ModuleWidget {
	// Panel (transparent widget).
	OhmerBlank2Background *blankPanel;
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

	OhmerBlank2Widget(OhmerBlank2 *module) {
		setModule(module);
		// 2 HP module, no SVG panel loaded, but using transparent widget instead.
		box.size = Vec(2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
			blankPanel = new OhmerBlank2Background();
			blankPanel->box.size = box.size;
			blankPanel->module = module;
			addChild(blankPanel);
		}
		// Top-left gold screw.
		topLeftScrewGold = createWidget<Torx_Gold>(Vec(0, 0));
		addChild(topLeftScrewGold);
		// Top-right gold screw.
		topRightScrewGold = createWidget<Torx_Gold>(Vec(box.size.x - RACK_GRID_WIDTH, 0));
		addChild(topRightScrewGold);
		// Bottom-left gold screw.
		bottomLeftScrewGold = createWidget<Torx_Gold>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomLeftScrewGold);
		// Bottom-right gold screw.
		bottomRightScrewGold = createWidget<Torx_Gold>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomRightScrewGold);
		// Top-left silver screw.
		topLeftScrewSilver = createWidget<Torx_Silver>(Vec(0, 0));
		addChild(topLeftScrewSilver);
		// Top-right silver screw.
		topRightScrewSilver = createWidget<Torx_Silver>(Vec(box.size.x - RACK_GRID_WIDTH, 0));
		addChild(topRightScrewSilver);
		// Bottom-left silver screw.
		bottomLeftScrewSilver = createWidget<Torx_Silver>(Vec(0, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomLeftScrewSilver);
		// Bottom-right silver screw.
		bottomRightScrewSilver = createWidget<Torx_Silver>(Vec(box.size.x - RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomRightScrewSilver);
	}

	void step() override {
		OhmerBlank2 *module = dynamic_cast<OhmerBlank2*>(this->module);
		if (!module) {
			// Probably from module browser...
			// By default, silver screws are visible for default Creamy or Absolute Night...
			// ...and, of course, golden screws are hidden.
			topLeftScrewGold->visible = false;
			topRightScrewGold->visible = false;
			bottomLeftScrewGold->visible = false;
			bottomRightScrewGold->visible = false;
			// By default, only top-left and bottom right, silver.
			topLeftScrewSilver->visible = true;
			topRightScrewSilver->visible = false;
			bottomLeftScrewSilver->visible = false;
			bottomRightScrewSilver->visible = true;
			return;
		}
		else {
			// Torx screws metal (silver, gold) are visible or hidden, depending selected model (from module's context-menu).
			// Silver Torx screws are visible only for non-"Signature" modules (Creamy, Stage Repro or Absolute Night).
			topLeftScrewGold->visible = ((module->screwsDisposal == 0) || (module->screwsDisposal == 2)) && (module->Theme > 2);
			topRightScrewGold->visible = ((module->screwsDisposal == 1) || (module->screwsDisposal == 2)) && (module->Theme > 2);
			bottomLeftScrewGold->visible = ((module->screwsDisposal == 1) || (module->screwsDisposal == 2)) && (module->Theme > 2);
			bottomRightScrewGold->visible = ((module->screwsDisposal == 0) || (module->screwsDisposal == 2)) && (module->Theme > 2);
			// Silver Torx screws visible or hidden (depending screws disposal from module's context-menu).
			topLeftScrewSilver->visible = ((module->screwsDisposal == 0) || (module->screwsDisposal == 2)) && (module->Theme < 3);
			topRightScrewSilver->visible = ((module->screwsDisposal == 1) || (module->screwsDisposal == 2)) && (module->Theme < 3);
			bottomLeftScrewSilver->visible = ((module->screwsDisposal == 1) || (module->screwsDisposal == 2)) && (module->Theme < 3);
			bottomRightScrewSilver->visible = ((module->screwsDisposal == 0) || (module->screwsDisposal == 2)) && (module->Theme < 3);
		}
		ModuleWidget::step();
	}

	///////////////////////////////////////////////////// CONTEXTUAL MENU - THEME //////////////////////////////////////////////////////

	struct OB2ThemeCreamyMenuItem : MenuItem {
		OhmerBlank2 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 0; // Creamy.
			module->panelBgColor = tblpanelBgColor[0];
		}
	};

	struct OB2ThemeStageReproMenuItem : MenuItem {
		OhmerBlank2 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 1; // Stage Repro.
			module->panelBgColor = tblpanelBgColor[1];
		}
	};

	struct OB2ThemeAbsoluteNightMenuItem : MenuItem {
		OhmerBlank2 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 2; // Absolute Night.
			module->panelBgColor = tblpanelBgColor[2];
		}
	};

	struct OB2ThemeDarkSignatureMenuItem : MenuItem {
		OhmerBlank2 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 3; // Dark Signature.
			module->panelBgColor = tblpanelBgColor[3];
		}
	};

	struct OB2ThemeDeepblueSignatureMenuItem : MenuItem {
		OhmerBlank2 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 4; // Deepblue Signature.
			module->panelBgColor = tblpanelBgColor[4];
		}
	};

	struct OB2ThemeTitaniumSignatureMenuItem : MenuItem {
		OhmerBlank2 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 5; // Titanium Signature.
			module->panelBgColor = tblpanelBgColor[5];
		}
	};

	struct OB2ThemeMenuItems : MenuItem {
		OhmerBlank2 *module;
		Menu *createChildMenu() override {
			Menu *menu = new Menu;

			OB2ThemeCreamyMenuItem *ob2themecreamymenuitem = new OB2ThemeCreamyMenuItem;
			ob2themecreamymenuitem->text = "Creamy";
			ob2themecreamymenuitem->rightText = CHECKMARK(module->Theme == 0);
			ob2themecreamymenuitem->module = module;
			menu->addChild(ob2themecreamymenuitem);

			OB2ThemeStageReproMenuItem *ob2themestagerepromenuitem = new OB2ThemeStageReproMenuItem;
			ob2themestagerepromenuitem->text = "Stage Repro";
			ob2themestagerepromenuitem->rightText = CHECKMARK(module->Theme == 1);
			ob2themestagerepromenuitem->module = module;
			menu->addChild(ob2themestagerepromenuitem);

			OB2ThemeAbsoluteNightMenuItem *ob2themeabsolutenightmenuitem = new OB2ThemeAbsoluteNightMenuItem;
			ob2themeabsolutenightmenuitem->text = "Absolute Night";
			ob2themeabsolutenightmenuitem->rightText = CHECKMARK(module->Theme == 2);
			ob2themeabsolutenightmenuitem->module = module;
			menu->addChild(ob2themeabsolutenightmenuitem);

			OB2ThemeDarkSignatureMenuItem *ob2themedarksignaturemenuitem = new OB2ThemeDarkSignatureMenuItem;
			ob2themedarksignaturemenuitem->text = "Dark \"Signature\"";
			ob2themedarksignaturemenuitem->rightText = CHECKMARK(module->Theme == 3);
			ob2themedarksignaturemenuitem->module = module;
			menu->addChild(ob2themedarksignaturemenuitem);

			OB2ThemeDeepblueSignatureMenuItem *ob2themedeepbluesignaturemenuitem = new OB2ThemeDeepblueSignatureMenuItem;
			ob2themedeepbluesignaturemenuitem->text = "Deepblue \"Signature\"";
			ob2themedeepbluesignaturemenuitem->rightText = CHECKMARK(module->Theme == 4);
			ob2themedeepbluesignaturemenuitem->module = module;
			menu->addChild(ob2themedeepbluesignaturemenuitem);

			OB2ThemeTitaniumSignatureMenuItem *ob2themetitaniumsignaturemenuitem = new OB2ThemeTitaniumSignatureMenuItem;
			ob2themetitaniumsignaturemenuitem->text = "Titanium \"Signature\"";
			ob2themetitaniumsignaturemenuitem->rightText = CHECKMARK(module->Theme == 5);
			ob2themetitaniumsignaturemenuitem->module = module;
			menu->addChild(ob2themetitaniumsignaturemenuitem);

			return menu;
		}
	};

	///////////////////////////////////////////////////// CONTEXTUAL MENU - SCREWS DISPOSAL //////////////////////////////////////////////////////

	struct ScrewsTLBR : MenuItem {
		OhmerBlank2 *module;
		void onAction(const ActionEvent& e) override {
			module->screwsDisposal = 0;
		}
	};

	struct ScrewsTRBL : MenuItem {
		OhmerBlank2 *module;
		void onAction(const ActionEvent& e) override {
			module->screwsDisposal = 1;
		}
	};

	struct ScrewsAll : MenuItem {
		OhmerBlank2 *module;
		void onAction(const ActionEvent& e) override {
			module->screwsDisposal = 2;
		}
	};

	struct BP2HPSubMenuItems : MenuItem {
		OhmerBlank2 *module;
		Menu *createChildMenu() override {
			Menu *menu = new Menu;
			ScrewsTLBR *bp2hpmenuitem1 = new ScrewsTLBR;
			bp2hpmenuitem1->text = "2: top-left & bottom-right";
			bp2hpmenuitem1->rightText = CHECKMARK(module->screwsDisposal == 0);
			bp2hpmenuitem1->module = module;
			menu->addChild(bp2hpmenuitem1);
			ScrewsTRBL *bp2hpmenuitem2 = new ScrewsTRBL;
			bp2hpmenuitem2->text = "2: top-right & bottom-left";
			bp2hpmenuitem2->rightText = CHECKMARK(module->screwsDisposal == 1);
			bp2hpmenuitem2->module = module;
			menu->addChild(bp2hpmenuitem2);
			ScrewsAll *bp2hpmenuitem3 = new ScrewsAll;
			bp2hpmenuitem3->text = "Four screws";
			bp2hpmenuitem3->rightText = CHECKMARK(module->screwsDisposal == 2);
			bp2hpmenuitem3->module = module;
			menu->addChild(bp2hpmenuitem3);
			return menu;
		}
	};

	void appendContextMenu(Menu *menu) override {
		OhmerBlank2 *module = dynamic_cast<OhmerBlank2*>(this->module);

		if (!module)
			return;

		menu->addChild(new MenuSeparator);

		OB2ThemeMenuItems *ob2thememenuitems = new OB2ThemeMenuItems;
		ob2thememenuitems->text = "Model";
		ob2thememenuitems->rightText = RIGHT_ARROW;
		ob2thememenuitems->module = module;
		menu->addChild(ob2thememenuitems);

		BP2HPSubMenuItems *bp2hpsubmenuitems = new BP2HPSubMenuItems;
		bp2hpsubmenuitems->text = "Screws disposal";
		bp2hpsubmenuitems->rightText = RIGHT_ARROW;
		bp2hpsubmenuitems->module = module;
		menu->addChild(bp2hpsubmenuitems);
	}

};

Model *modelBlankPanel2 = createModel<OhmerBlank2, OhmerBlank2Widget>("OhmerBlank2");
