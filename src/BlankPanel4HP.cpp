////////////////////////////////////////////////////////////////////////////////////////////////////
////// Blank Panel 4 HP module /////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

struct OhmerBlank4 : Module {
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

	OhmerBlank4() {
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

struct OhmerBlank4ClassicMenu : MenuItem {
	OhmerBlank4 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 0; // Model: default Classic (beige).
	}
};

struct OhmerBlank4StageReproMenu : MenuItem {
	OhmerBlank4 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 1; // Model: Stage Repro.
	}
};

struct OhmerBlank4AbsoluteNightMenu : MenuItem {
	OhmerBlank4 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 2; // Model: Absolute Night.
	}
};

struct OhmerBlank4DarkSignatureMenu : MenuItem {
	OhmerBlank4 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 3; // Model: Dark Signature.
	}
};

struct OhmerBlank4DeepblueSignatureMenu : MenuItem {
	OhmerBlank4 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 4; // Model: Deepblue Signature.
	}
};

struct OhmerBlank4CarbonSignatureMenu : MenuItem {
	OhmerBlank4 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 5; // Model: Carbon Signature.
	}
};

struct OhmerBlank4SubMenuItems : MenuItem {
	OhmerBlank4 *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		OhmerBlank4ClassicMenu *ohmerblank4menuitem1 = new OhmerBlank4ClassicMenu;
		ohmerblank4menuitem1->text = "Classic (default)";
		ohmerblank4menuitem1->rightText = CHECKMARK(module->Theme == 0);
		ohmerblank4menuitem1->module = module;
		menu->addChild(ohmerblank4menuitem1);

		OhmerBlank4StageReproMenu *ohmerblank4menuitem2 = new OhmerBlank4StageReproMenu;
		ohmerblank4menuitem2->text = "Stage Repro";
		ohmerblank4menuitem2->rightText = CHECKMARK(module->Theme == 1);
		ohmerblank4menuitem2->module = module;
		menu->addChild(ohmerblank4menuitem2);

		OhmerBlank4AbsoluteNightMenu *ohmerblank4menuitem3 = new OhmerBlank4AbsoluteNightMenu;
		ohmerblank4menuitem3->text = "Absolute Night";
		ohmerblank4menuitem3->rightText = CHECKMARK(module->Theme == 2);
		ohmerblank4menuitem3->module = module;
		menu->addChild(ohmerblank4menuitem3);

		OhmerBlank4DarkSignatureMenu *ohmerblank4menuitem4 = new OhmerBlank4DarkSignatureMenu;
		ohmerblank4menuitem4->text = "Dark \"Signature\"";
		ohmerblank4menuitem4->rightText = CHECKMARK(module->Theme == 3);
		ohmerblank4menuitem4->module = module;
		menu->addChild(ohmerblank4menuitem4);

		OhmerBlank4DeepblueSignatureMenu *ohmerblank4menuitem5 = new OhmerBlank4DeepblueSignatureMenu;
		ohmerblank4menuitem5->text = "Deepblue \"Signature\"";
		ohmerblank4menuitem5->rightText = CHECKMARK(module->Theme == 4);
		ohmerblank4menuitem5->module = module;
		menu->addChild(ohmerblank4menuitem5);

		OhmerBlank4CarbonSignatureMenu *ohmerblank4menuitem6 = new OhmerBlank4CarbonSignatureMenu;
		ohmerblank4menuitem6->text = "Carbon \"Signature\"";
		ohmerblank4menuitem6->rightText = CHECKMARK(module->Theme == 5);
		ohmerblank4menuitem6->module = module;
		menu->addChild(ohmerblank4menuitem6);

		return menu;
	}
};

///////////////////////////////////////////////// PANEL BACKGROUND COLOR /////////////////////////////////////////////////

struct OhmerBlank4Background : TransparentWidget {
	OhmerBlank4 *module;

	OhmerBlank4Background() {
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

struct OhmerBlank4Widget : ModuleWidget {
	// Panel (transparent widget).
	OhmerBlank4Background *blankPanel;
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

	OhmerBlank4Widget(OhmerBlank4 *module) {
		setModule(module);
		// 4 HP module, no SVG panel loaded, but using transparent widget instead.
		box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
    {
			blankPanel = new OhmerBlank4Background();
			blankPanel->box.size = box.size;
			blankPanel->module = module;
			addChild(blankPanel);
		}
		// This 4 HP module uses 4 screws (may are silver or gold).
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
	}

	void step() override {
		OhmerBlank4 *module = dynamic_cast<OhmerBlank4*>(this->module);
		if (module) {
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
		OhmerBlank4 *module = dynamic_cast<OhmerBlank4*>(this->module);
		menu->addChild(new MenuEntry);
		OhmerBlank4SubMenuItems *ohmerblank4submenuitems = new OhmerBlank4SubMenuItems;
		ohmerblank4submenuitems->text = "Model";
		ohmerblank4submenuitems->rightText = RIGHT_ARROW;
		ohmerblank4submenuitems->module = module;
		menu->addChild(ohmerblank4submenuitems);
	}

};

Model *modelBlankPanel4 = createModel<OhmerBlank4, OhmerBlank4Widget>("OhmerBlank4");
