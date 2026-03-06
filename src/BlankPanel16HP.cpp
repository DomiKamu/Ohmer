////////////////////////////////////////////////////////////////////////////////////////////////////
////// Blank Panel 16 HP module ////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

struct OhmerBlank16 : Module {
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

	OhmerBlank16() {
		// Module constructor.
		Theme = rack::settings::preferDarkPanels ? 2 : 0; // Assuming default is "Creamy" or "Absolute Night" (depending "Use dark panels if available" option, from "View" menu).
		panelBgColor = tblpanelBgColor[Theme];
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Model", json_integer(Theme));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t *ThemeJ = json_object_get(rootJ, "Model");
		if (ThemeJ)
			Theme = json_integer_value(ThemeJ);
		panelBgColor = tblpanelBgColor[Theme];
	}

};

///////////////////////////////////////////////// PANEL BACKGROUND COLOR /////////////////////////////////////////////////

struct OhmerBlank16Background : TransparentWidget {
	OhmerBlank16 *module;

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

struct OhmerBlank16Widget : ModuleWidget {
	// Panel (transparent widget).
	OhmerBlank16Background *blankPanel;
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

	OhmerBlank16Widget(OhmerBlank16 *module) {
		setModule(module);
		// 16 HP module, no SVG panel loaded, but using transparent widget instead.
		box.size = Vec(16 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
			blankPanel = new OhmerBlank16Background();
			blankPanel->box.size = box.size;
			blankPanel->module = module;
			addChild(blankPanel);
		}
		// This 16 HP module uses 4 screws (may are silver or gold).
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
		OhmerBlank16 *module = dynamic_cast<OhmerBlank16*>(this->module);
		if (!module) {
			// !module: the module isn't instanciated (probably as preview from module browser).
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
			// Torx screws metal (silver, gold) are visible or hidden, depending selected model (from module's contextual menu).
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

	struct OB16ThemeCreamyMenuItem : MenuItem {
		OhmerBlank16 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 0; // Creamy.
			module->panelBgColor = tblpanelBgColor[0];
		}
	};

	struct OB16ThemeStageReproMenuItem : MenuItem {
		OhmerBlank16 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 1; // Stage Repro.
			module->panelBgColor = tblpanelBgColor[1];
		}
	};

	struct OB16ThemeAbsoluteNightMenuItem : MenuItem {
		OhmerBlank16 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 2; // Absolute Night.
			module->panelBgColor = tblpanelBgColor[2];
		}
	};

	struct OB16ThemeDarkSignatureMenuItem : MenuItem {
		OhmerBlank16 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 3; // Dark Signature.
			module->panelBgColor = tblpanelBgColor[3];
		}
	};

	struct OB16ThemeDeepblueSignatureMenuItem : MenuItem {
		OhmerBlank16 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 4; // Deepblue Signature.
			module->panelBgColor = tblpanelBgColor[4];
		}
	};

	struct OB16ThemeTitaniumSignatureMenuItem : MenuItem {
		OhmerBlank16 *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 5; // Titanium Signature.
			module->panelBgColor = tblpanelBgColor[5];
		}
	};

	struct OB16ThemeMenuItems : MenuItem {
		OhmerBlank16 *module;
		Menu *createChildMenu() override {
			Menu *menu = new Menu;

			OB16ThemeCreamyMenuItem *ob16themecreamymenuitem = new OB16ThemeCreamyMenuItem;
			ob16themecreamymenuitem->text = "Creamy";
			ob16themecreamymenuitem->rightText = CHECKMARK(module->Theme == 0);
			ob16themecreamymenuitem->module = module;
			menu->addChild(ob16themecreamymenuitem);

			OB16ThemeStageReproMenuItem *ob16themestagerepromenuitem = new OB16ThemeStageReproMenuItem;
			ob16themestagerepromenuitem->text = "Stage Repro";
			ob16themestagerepromenuitem->rightText = CHECKMARK(module->Theme == 1);
			ob16themestagerepromenuitem->module = module;
			menu->addChild(ob16themestagerepromenuitem);

			OB16ThemeAbsoluteNightMenuItem *ob16themeabsolutenightmenuitem = new OB16ThemeAbsoluteNightMenuItem;
			ob16themeabsolutenightmenuitem->text = "Absolute Night";
			ob16themeabsolutenightmenuitem->rightText = CHECKMARK(module->Theme == 2);
			ob16themeabsolutenightmenuitem->module = module;
			menu->addChild(ob16themeabsolutenightmenuitem);

			OB16ThemeDarkSignatureMenuItem *ob16themedarksignaturemenuitem = new OB16ThemeDarkSignatureMenuItem;
			ob16themedarksignaturemenuitem->text = "Dark \"Signature\"";
			ob16themedarksignaturemenuitem->rightText = CHECKMARK(module->Theme == 3);
			ob16themedarksignaturemenuitem->module = module;
			menu->addChild(ob16themedarksignaturemenuitem);

			OB16ThemeDeepblueSignatureMenuItem *ob16themedeepbluesignaturemenuitem = new OB16ThemeDeepblueSignatureMenuItem;
			ob16themedeepbluesignaturemenuitem->text = "Deepblue \"Signature\"";
			ob16themedeepbluesignaturemenuitem->rightText = CHECKMARK(module->Theme == 4);
			ob16themedeepbluesignaturemenuitem->module = module;
			menu->addChild(ob16themedeepbluesignaturemenuitem);

			OB16ThemeTitaniumSignatureMenuItem *ob16themetitaniumsignaturemenuitem = new OB16ThemeTitaniumSignatureMenuItem;
			ob16themetitaniumsignaturemenuitem->text = "Titanium \"Signature\"";
			ob16themetitaniumsignaturemenuitem->rightText = CHECKMARK(module->Theme == 5);
			ob16themetitaniumsignaturemenuitem->module = module;
			menu->addChild(ob16themetitaniumsignaturemenuitem);

			return menu;
		}
	};

	void appendContextMenu(Menu *menu) override {
		OhmerBlank16 *module = dynamic_cast<OhmerBlank16*>(this->module);
		menu->addChild(new MenuSeparator);
		OB16ThemeMenuItems *ob16thememenuitems = new OB16ThemeMenuItems;
		ob16thememenuitems->text = "Model";
		ob16thememenuitems->rightText = RIGHT_ARROW;
		ob16thememenuitems->module = module;
		menu->addChild(ob16thememenuitems);
	}

};

Model *modelBlankPanel16 = createModel<OhmerBlank16, OhmerBlank16Widget>("OhmerBlank16");
