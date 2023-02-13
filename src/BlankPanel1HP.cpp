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
	int Theme = 0; // 0 = Classic (default), 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Carbon Signature.

	// Panel color (default is "Classic" beige model).
	NVGcolor panelBackgroundColor = nvgRGB(0xd2, 0xd2, 0xcd);

	OhmerBlank1() {
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

struct OhmerBlank1ClassicMenu : MenuItem {
	OhmerBlank1 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 0; // Model: default Classic (beige).
	}
};

struct OhmerBlank1StageReproMenu : MenuItem {
	OhmerBlank1 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 1; // Model: Stage Repro.
	}
};

struct OhmerBlank1AbsoluteNightMenu : MenuItem {
	OhmerBlank1 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 2; // Model: Absolute Night.
	}
};

struct OhmerBlank1DarkSignatureMenu : MenuItem {
	OhmerBlank1 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 3; // Model: Dark Signature.
	}
};

struct OhmerBlank1DeepblueSignatureMenu : MenuItem {
	OhmerBlank1 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 4; // Model: Deepblue Signature.
	}
};

struct OhmerBlank1CarbonSignatureMenu : MenuItem {
	OhmerBlank1 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 5; // Model: Carbon Signature.
	}
};

struct OhmerBlank1SubMenuItems : MenuItem {
	OhmerBlank1 *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		OhmerBlank1ClassicMenu *ohmerblank1menuitem1 = new OhmerBlank1ClassicMenu;
		ohmerblank1menuitem1->text = "Classic (default)";
		ohmerblank1menuitem1->rightText = CHECKMARK(module->Theme == 0);
		ohmerblank1menuitem1->module = module;
		menu->addChild(ohmerblank1menuitem1);

		OhmerBlank1StageReproMenu *ohmerblank1menuitem2 = new OhmerBlank1StageReproMenu;
		ohmerblank1menuitem2->text = "Stage Repro";
		ohmerblank1menuitem2->rightText = CHECKMARK(module->Theme == 1);
		ohmerblank1menuitem2->module = module;
		menu->addChild(ohmerblank1menuitem2);

		OhmerBlank1AbsoluteNightMenu *ohmerblank1menuitem3 = new OhmerBlank1AbsoluteNightMenu;
		ohmerblank1menuitem3->text = "Absolute Night";
		ohmerblank1menuitem3->rightText = CHECKMARK(module->Theme == 2);
		ohmerblank1menuitem3->module = module;
		menu->addChild(ohmerblank1menuitem3);

		OhmerBlank1DarkSignatureMenu *ohmerblank1menuitem4 = new OhmerBlank1DarkSignatureMenu;
		ohmerblank1menuitem4->text = "Dark \"Signature\"";
		ohmerblank1menuitem4->rightText = CHECKMARK(module->Theme == 3);
		ohmerblank1menuitem4->module = module;
		menu->addChild(ohmerblank1menuitem4);

		OhmerBlank1DeepblueSignatureMenu *ohmerblank1menuitem5 = new OhmerBlank1DeepblueSignatureMenu;
		ohmerblank1menuitem5->text = "Deepblue \"Signature\"";
		ohmerblank1menuitem5->rightText = CHECKMARK(module->Theme == 4);
		ohmerblank1menuitem5->module = module;
		menu->addChild(ohmerblank1menuitem5);

		OhmerBlank1CarbonSignatureMenu *ohmerblank1menuitem6 = new OhmerBlank1CarbonSignatureMenu;
		ohmerblank1menuitem6->text = "Carbon \"Signature\"";
		ohmerblank1menuitem6->rightText = CHECKMARK(module->Theme == 5);
		ohmerblank1menuitem6->module = module;
		menu->addChild(ohmerblank1menuitem6);

		return menu;
	}
};

///////////////////////////////////////////////// PANEL BACKGROUND COLOR /////////////////////////////////////////////////

struct OhmerBlank1Background : TransparentWidget {
	OhmerBlank1 *module;

	OhmerBlank1Background() {
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
		if (module) {
			// Torx screws metal (silver, gold) are visible or hidden, depending selected model (from module's context-menu).
			// Silver Torx screws are visible only for non-"Signature" modules (Classic, Stage Repro or Absolute Night).
			topScrewSilver->visible = (module->Theme < 3);
			bottomScrewSilver->visible = (module->Theme < 3);
			// Gold Torx screws are visible only for "Signature" modules (Dark Signature, Deepblue Signature or Carbon Signature).
			topScrewGold->visible = (module->Theme > 2);
			bottomScrewGold->visible = (module->Theme > 2);
		}
		else {
			// Default panel theme is always "Classic" (beige, using silver screws, using silver button, LCD).
			// Other panels are, of course, hidden.
			// By default, silver screws are visible for default beige Classic panel...
			// ...and, of course, golden screws are hidden.
			topScrewGold->visible = false;
			bottomScrewGold->visible = false;
			topScrewSilver->visible = true;
			bottomScrewSilver->visible = true;
		}
		ModuleWidget::step();
	}

	void appendContextMenu(Menu *menu) override {
		OhmerBlank1 *module = dynamic_cast<OhmerBlank1*>(this->module);
		menu->addChild(new MenuEntry);
		OhmerBlank1SubMenuItems *ohmerblank1submenuitems = new OhmerBlank1SubMenuItems;
		ohmerblank1submenuitems->text = "Model";
		ohmerblank1submenuitems->rightText = RIGHT_ARROW;
		ohmerblank1submenuitems->module = module;
		menu->addChild(ohmerblank1submenuitems);
	}

};

Model *modelBlankPanel1 = createModel<OhmerBlank1, OhmerBlank1Widget>("OhmerBlank1");
