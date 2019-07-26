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
	int Theme = 0; // 0 = Classic (default), 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Carbon Signature.

	// Panel color (default is "Classic" beige model).
	NVGcolor panelBackgroundColor = nvgRGB(0xd2, 0xd2, 0xcd);

	OhmerBlank16() {
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

struct OhmerBlank16ClassicMenu : MenuItem {
	OhmerBlank16 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 0; // Model: default Classic (beige).
	}
};

struct OhmerBlank16StageReproMenu : MenuItem {
	OhmerBlank16 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 1; // Model: Stage Repro.
	}
};

struct OhmerBlank16AbsoluteNightMenu : MenuItem {
	OhmerBlank16 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 2; // Model: Absolute Night.
	}
};

struct OhmerBlank16DarkSignatureMenu : MenuItem {
	OhmerBlank16 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 3; // Model: Dark Signature.
	}
};

struct OhmerBlank16DeepblueSignatureMenu : MenuItem {
	OhmerBlank16 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 4; // Model: Deepblue Signature.
	}
};

struct OhmerBlank16CarbonSignatureMenu : MenuItem {
	OhmerBlank16 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 5; // Model: Carbon Signature.
	}
};

struct OhmerBlank16SubMenuItems : MenuItem {
	OhmerBlank16 *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		OhmerBlank16ClassicMenu *ohmerblank16menuitem1 = new OhmerBlank16ClassicMenu;
		ohmerblank16menuitem1->text = "Classic (default)";
		ohmerblank16menuitem1->rightText = CHECKMARK(module->Theme == 0);
		ohmerblank16menuitem1->module = module;
		menu->addChild(ohmerblank16menuitem1);

		OhmerBlank16StageReproMenu *ohmerblank16menuitem2 = new OhmerBlank16StageReproMenu;
		ohmerblank16menuitem2->text = "Stage Repro";
		ohmerblank16menuitem2->rightText = CHECKMARK(module->Theme == 1);
		ohmerblank16menuitem2->module = module;
		menu->addChild(ohmerblank16menuitem2);

		OhmerBlank16AbsoluteNightMenu *ohmerblank16menuitem3 = new OhmerBlank16AbsoluteNightMenu;
		ohmerblank16menuitem3->text = "Absolute Night";
		ohmerblank16menuitem3->rightText = CHECKMARK(module->Theme == 2);
		ohmerblank16menuitem3->module = module;
		menu->addChild(ohmerblank16menuitem3);

		OhmerBlank16DarkSignatureMenu *ohmerblank16menuitem4 = new OhmerBlank16DarkSignatureMenu;
		ohmerblank16menuitem4->text = "Dark \"Signature\"";
		ohmerblank16menuitem4->rightText = CHECKMARK(module->Theme == 3);
		ohmerblank16menuitem4->module = module;
		menu->addChild(ohmerblank16menuitem4);

		OhmerBlank16DeepblueSignatureMenu *ohmerblank16menuitem5 = new OhmerBlank16DeepblueSignatureMenu;
		ohmerblank16menuitem5->text = "Deepblue \"Signature\"";
		ohmerblank16menuitem5->rightText = CHECKMARK(module->Theme == 4);
		ohmerblank16menuitem5->module = module;
		menu->addChild(ohmerblank16menuitem5);

		OhmerBlank16CarbonSignatureMenu *ohmerblank16menuitem6 = new OhmerBlank16CarbonSignatureMenu;
		ohmerblank16menuitem6->text = "Carbon \"Signature\"";
		ohmerblank16menuitem6->rightText = CHECKMARK(module->Theme == 5);
		ohmerblank16menuitem6->module = module;
		menu->addChild(ohmerblank16menuitem6);

		return menu;
	}
};

///////////////////////////////////////////////// PANEL BACKGROUND COLOR /////////////////////////////////////////////////

struct OhmerBlank16Background : TransparentWidget {
	OhmerBlank16 *module;

	OhmerBlank16Background() {
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

struct OhmerBlank16Widget : ModuleWidget {
	// Panel (transparent widget).
	OhmerBlank16Background *blankPanel;
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
	}

	void step() override {
		OhmerBlank16 *module = dynamic_cast<OhmerBlank16*>(this->module);
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
		OhmerBlank16 *module = dynamic_cast<OhmerBlank16*>(this->module);
		menu->addChild(new MenuEntry);
		OhmerBlank16SubMenuItems *ohmerblank16submenuitems = new OhmerBlank16SubMenuItems;
		ohmerblank16submenuitems->text = "Model";
		ohmerblank16submenuitems->rightText = RIGHT_ARROW;
		ohmerblank16submenuitems->module = module;
		menu->addChild(ohmerblank16submenuitems);
	}

};

Model *modelBlankPanel16 = createModel<OhmerBlank16, OhmerBlank16Widget>("OhmerBlank16");
