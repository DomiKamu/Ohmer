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
	int Theme = 0; // 0 = Classic (default), 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Carbon Signature.

	// Panel color (default is "Classic" beige model).
	NVGcolor panelBackgroundColor = nvgRGB(0xd2, 0xd2, 0xcd);

	// Screws disposal.
	int screwsDisposal = 0;

	OhmerBlank2() {
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
		json_object_set_new(rootJ, "screwsDisposal", json_integer(screwsDisposal));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t *ThemeJ = json_object_get(rootJ, "Theme");
		if (ThemeJ)
			Theme = json_integer_value(ThemeJ);
		json_t *screwsDisposalJ = json_object_get(rootJ, "screwsDisposal");
		if (screwsDisposalJ)
			screwsDisposal = json_integer_value(screwsDisposalJ);
	}

};

///////////////////////////////////////////////////// CONTEXT-MENU (MODEL) //////////////////////////////////////////////////////

struct OhmerBlank2ClassicMenu : MenuItem {
	OhmerBlank2 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 0; // Model: default Classic (beige).
	}
};

struct OhmerBlank2StageReproMenu : MenuItem {
	OhmerBlank2 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 1; // Model: Stage Repro.
	}
};

struct OhmerBlank2AbsoluteNightMenu : MenuItem {
	OhmerBlank2 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 2; // Model: Absolute Night.
	}
};

struct OhmerBlank2DarkSignatureMenu : MenuItem {
	OhmerBlank2 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 3; // Model: Dark Signature.
	}
};

struct OhmerBlank2DeepblueSignatureMenu : MenuItem {
	OhmerBlank2 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 4; // Model: Deepblue Signature.
	}
};

struct OhmerBlank2CarbonSignatureMenu : MenuItem {
	OhmerBlank2 *module;
	void onAction(const event::Action &e) override {
		module->Theme = 5; // Model: Carbon Signature.
	}
};

struct OhmerBlank2SubMenuItems : MenuItem {
	OhmerBlank2 *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		OhmerBlank2ClassicMenu *ohmerblank2menuitem1 = new OhmerBlank2ClassicMenu;
		ohmerblank2menuitem1->text = "Classic (default)";
		ohmerblank2menuitem1->rightText = CHECKMARK(module->Theme == 0);
		ohmerblank2menuitem1->module = module;
		menu->addChild(ohmerblank2menuitem1);

		OhmerBlank2StageReproMenu *ohmerblank2menuitem2 = new OhmerBlank2StageReproMenu;
		ohmerblank2menuitem2->text = "Stage Repro";
		ohmerblank2menuitem2->rightText = CHECKMARK(module->Theme == 1);
		ohmerblank2menuitem2->module = module;
		menu->addChild(ohmerblank2menuitem2);

		OhmerBlank2AbsoluteNightMenu *ohmerblank2menuitem3 = new OhmerBlank2AbsoluteNightMenu;
		ohmerblank2menuitem3->text = "Absolute Night";
		ohmerblank2menuitem3->rightText = CHECKMARK(module->Theme == 2);
		ohmerblank2menuitem3->module = module;
		menu->addChild(ohmerblank2menuitem3);

		OhmerBlank2DarkSignatureMenu *ohmerblank2menuitem4 = new OhmerBlank2DarkSignatureMenu;
		ohmerblank2menuitem4->text = "Dark \"Signature\"";
		ohmerblank2menuitem4->rightText = CHECKMARK(module->Theme == 3);
		ohmerblank2menuitem4->module = module;
		menu->addChild(ohmerblank2menuitem4);

		OhmerBlank2DeepblueSignatureMenu *ohmerblank2menuitem5 = new OhmerBlank2DeepblueSignatureMenu;
		ohmerblank2menuitem5->text = "Deepblue \"Signature\"";
		ohmerblank2menuitem5->rightText = CHECKMARK(module->Theme == 4);
		ohmerblank2menuitem5->module = module;
		menu->addChild(ohmerblank2menuitem5);

		OhmerBlank2CarbonSignatureMenu *ohmerblank2menuitem6 = new OhmerBlank2CarbonSignatureMenu;
		ohmerblank2menuitem6->text = "Carbon \"Signature\"";
		ohmerblank2menuitem6->rightText = CHECKMARK(module->Theme == 5);
		ohmerblank2menuitem6->module = module;
		menu->addChild(ohmerblank2menuitem6);

		return menu;
	}
};

///////////////////////////////////////////////////// CONTEXT-MENU (SCREWS DISPOSAL) //////////////////////////////////////////////////////

struct ScrewsTLBR : MenuItem {
	OhmerBlank2 *module;
	void onAction(const event::Action &e) override {
		module->screwsDisposal = 0;
	}
};

struct ScrewsTRBL : MenuItem {
	OhmerBlank2 *module;
	void onAction(const event::Action &e) override {
		module->screwsDisposal = 1;
	}
};

struct ScrewsAll : MenuItem {
	OhmerBlank2 *module;
	void onAction(const event::Action &e) override {
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

///////////////////////////////////////////////// PANEL BACKGROUND COLOR /////////////////////////////////////////////////

struct OhmerBlank2Background : TransparentWidget {
	OhmerBlank2 *module;

	OhmerBlank2Background() {
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

struct OhmerBlank2Widget : ModuleWidget {
	// Panel (transparent widget).
	OhmerBlank2Background *blankPanel;
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
		OhmerBlank2 *module = dynamic_cast<OhmerBlank2*>(this->module);
		if (module) {
			// Silver Torx screws visible or hidden (depending screws disposal from module's context-menu).
			topLeftScrewSilver->visible = ((module->screwsDisposal == 0) || (module->screwsDisposal == 2)) && (module->Theme < 3);
			topRightScrewSilver->visible = ((module->screwsDisposal == 1) || (module->screwsDisposal == 2)) && (module->Theme < 3);
			bottomLeftScrewSilver->visible = ((module->screwsDisposal == 1) || (module->screwsDisposal == 2)) && (module->Theme < 3);
			bottomRightScrewSilver->visible = ((module->screwsDisposal == 0) || (module->screwsDisposal == 2)) && (module->Theme < 3);
			// Gold Torx screws visible or hidden (depending screws disposal from module's context-menu).
			topLeftScrewGold->visible = ((module->screwsDisposal == 0) || (module->screwsDisposal == 2)) && (module->Theme > 2);
			topRightScrewGold->visible = ((module->screwsDisposal == 1) || (module->screwsDisposal == 2)) && (module->Theme > 2);
			bottomLeftScrewGold->visible = ((module->screwsDisposal == 1) || (module->screwsDisposal == 2)) && (module->Theme > 2);
			bottomRightScrewGold->visible = ((module->screwsDisposal == 0) || (module->screwsDisposal == 2)) && (module->Theme > 2);
		}
		else {
			// By default, only top-left and bottom right, silver.
			topLeftScrewSilver->visible = true;
			topRightScrewSilver->visible = false;
			bottomLeftScrewSilver->visible = false;
			bottomRightScrewSilver->visible = true;
			// By default, all gold are hidden for Classic blank plate.
			topLeftScrewGold->visible = false;
			topRightScrewGold->visible = false;
			bottomLeftScrewGold->visible = false;
			bottomRightScrewGold->visible = false;
		}
		ModuleWidget::step();
	}

	void appendContextMenu(Menu *menu) override {
		OhmerBlank2 *module = dynamic_cast<OhmerBlank2*>(this->module);
		menu->addChild(new MenuEntry);

		OhmerBlank2SubMenuItems *ohmerblank2submenuitems = new OhmerBlank2SubMenuItems;
		ohmerblank2submenuitems->text = "Model";
		ohmerblank2submenuitems->rightText = RIGHT_ARROW;
		ohmerblank2submenuitems->module = module;
		menu->addChild(ohmerblank2submenuitems);

		BP2HPSubMenuItems *bp2hpsubmenuitems = new BP2HPSubMenuItems;
		bp2hpsubmenuitems->text = "Screws disposal";
		bp2hpsubmenuitems->rightText = RIGHT_ARROW;
		bp2hpsubmenuitems->module = module;
		menu->addChild(bp2hpsubmenuitems);
	}

};

Model *modelBlankPanel2 = createModel<OhmerBlank2, OhmerBlank2Widget>("OhmerBlank2");
