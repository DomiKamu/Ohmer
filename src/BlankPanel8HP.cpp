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
	int Theme = 0; // 0 = Classic (default), 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Carbon Signature.

	// Panel color (default is "Classic" beige model).
	NVGcolor panelBackgroundColor = nvgRGB(0xd2, 0xd2, 0xcd);

	OhmerBlank8() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
	}

	void process(const ProcessArgs &args) override {
		// DSP processing...
		// Depending current model (theme), set the relevant background color for panel.
		panelBackgroundColor = tblPanelBackgroundColor[Theme];
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Theme", json_integer(Theme));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t *ThemeJ = json_object_get(rootJ, "Theme");
		if (ThemeJ)
			Theme = json_integer_value(ThemeJ);
	}

};

///////////////////////////////////////////////////// CONTEXT-MENU //////////////////////////////////////////////////////

struct OhmerBlank8ClassicMenu : MenuItem {
	OhmerBlank8 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 0; // Model: default Classic (beige).
	}
};

struct OhmerBlank8StageReproMenu : MenuItem {
	OhmerBlank8 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 1; // Model: Stage Repro.
	}
};

struct OhmerBlank8AbsoluteNightMenu : MenuItem {
	OhmerBlank8 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 2; // Model: Absolute Night.
	}
};

struct OhmerBlank8DarkSignatureMenu : MenuItem {
	OhmerBlank8 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 3; // Model: Dark Signature.
	}
};

struct OhmerBlank8DeepblueSignatureMenu : MenuItem {
	OhmerBlank8 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 4; // Model: Deepblue Signature.
	}
};

struct OhmerBlank8CarbonSignatureMenu : MenuItem {
	OhmerBlank8 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 5; // Model: Carbon Signature.
	}
};

struct OhmerBlank8SubMenuItems : MenuItem {
	OhmerBlank8 *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		OhmerBlank8ClassicMenu *ohmerblank8menuitem1 = new OhmerBlank8ClassicMenu;
		ohmerblank8menuitem1->text = "Classic (default)";
		ohmerblank8menuitem1->rightText = CHECKMARK(module->Theme == 0);
		ohmerblank8menuitem1->module = module;
		menu->addChild(ohmerblank8menuitem1);

		OhmerBlank8StageReproMenu *ohmerblank8menuitem2 = new OhmerBlank8StageReproMenu;
		ohmerblank8menuitem2->text = "Stage Repro";
		ohmerblank8menuitem2->rightText = CHECKMARK(module->Theme == 1);
		ohmerblank8menuitem2->module = module;
		menu->addChild(ohmerblank8menuitem2);

		OhmerBlank8AbsoluteNightMenu *ohmerblank8menuitem3 = new OhmerBlank8AbsoluteNightMenu;
		ohmerblank8menuitem3->text = "Absolute Night";
		ohmerblank8menuitem3->rightText = CHECKMARK(module->Theme == 2);
		ohmerblank8menuitem3->module = module;
		menu->addChild(ohmerblank8menuitem3);

		OhmerBlank8DarkSignatureMenu *ohmerblank8menuitem4 = new OhmerBlank8DarkSignatureMenu;
		ohmerblank8menuitem4->text = "Dark \"Signature\"";
		ohmerblank8menuitem4->rightText = CHECKMARK(module->Theme == 3);
		ohmerblank8menuitem4->module = module;
		menu->addChild(ohmerblank8menuitem4);

		OhmerBlank8DeepblueSignatureMenu *ohmerblank8menuitem5 = new OhmerBlank8DeepblueSignatureMenu;
		ohmerblank8menuitem5->text = "Deepblue \"Signature\"";
		ohmerblank8menuitem5->rightText = CHECKMARK(module->Theme == 4);
		ohmerblank8menuitem5->module = module;
		menu->addChild(ohmerblank8menuitem5);

		OhmerBlank8CarbonSignatureMenu *ohmerblank8menuitem6 = new OhmerBlank8CarbonSignatureMenu;
		ohmerblank8menuitem6->text = "Carbon \"Signature\"";
		ohmerblank8menuitem6->rightText = CHECKMARK(module->Theme == 5);
		ohmerblank8menuitem6->module = module;
		menu->addChild(ohmerblank8menuitem6);

		return menu;
	}
};

///////////////////////////////////////////////// PANEL BACKGROUND COLOR /////////////////////////////////////////////////

struct OhmerBlank8Background : TransparentWidget {
	OhmerBlank8 *module;

	OhmerBlank8Background() {
	}

	void draw(const DrawArgs &args) override {
		nvgBeginPath(args.vg);
		nvgRect(args.vg, 0.0, 0.0, box.size.x, box.size.y);
		if (module)
			nvgFillColor(args.vg, module->panelBackgroundColor);
			else nvgFillColor(args.vg, nvgRGB(0xd2, 0xd2, 0xcd));
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
		if (module) {
			// Torx screws metal (silver, gold) are visible or hidden, depending selected model (from module's context-menu).
			// Gold Torx screws are visible only for "Signature" modules (Dark Signature, Deepblue Signature or Carbon Signature).
			topLeftScrewGold->visible = (module->Theme > 2);
			topRightScrewGold->visible = (module->Theme > 2);
			bottomLeftScrewGold->visible = (module->Theme > 2);
			bottomRightScrewGold->visible = (module->Theme > 2);
			// Silver Torx screws are visible only for non-"Signature" modules (Classic, Stage Repro or Absolute Night).
			topLeftScrewSilver->visible = (module->Theme < 3);
			topRightScrewSilver->visible = (module->Theme < 3);
			bottomLeftScrewSilver->visible = (module->Theme < 3);
			bottomRightScrewSilver->visible = (module->Theme < 3);
		}
		else {
			// Default panel theme is always "Classic" (beige, using silver screws, using silver button, LCD).
			// Other panels are, of course, hidden.
			// By default, golden screws are hidden...
			topLeftScrewGold->visible = false;
			topRightScrewGold->visible = false;
			bottomLeftScrewGold->visible = false;
			bottomRightScrewGold->visible = false;
			// ...and silver screws are visible (for default beige "Classic" model, in module browser)...
			topLeftScrewSilver->visible = true;
			topRightScrewSilver->visible = true;
			bottomLeftScrewSilver->visible = true;
			bottomRightScrewSilver->visible = true;
		}
		ModuleWidget::step();
	}

	void appendContextMenu(Menu *menu) override {
		OhmerBlank8 *module = dynamic_cast<OhmerBlank8*>(this->module);
		menu->addChild(new MenuEntry);
		OhmerBlank8SubMenuItems *ohmerblank8submenuitems = new OhmerBlank8SubMenuItems;
		ohmerblank8submenuitems->text = "Model";
		ohmerblank8submenuitems->rightText = RIGHT_ARROW;
		ohmerblank8submenuitems->module = module;
		menu->addChild(ohmerblank8submenuitems);
	}

};

Model *modelBlankPanel8 = createModel<OhmerBlank8, OhmerBlank8Widget>("OhmerBlank8");
