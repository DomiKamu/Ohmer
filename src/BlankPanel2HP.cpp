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
	int Model; // 0 = Creamy, 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Titanium Signature.

	// Panel color (default is Creamy).
	NVGcolor panelBackgroundColor = nvgRGB(0xd2, 0xd2, 0xcd);

	// Screws disposal.
	int screwsDisposal = 0;

	OhmerBlank2() {
		// Module constructor.
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		Model = rack::settings::preferDarkPanels ? 2 : 0; // Model: assuming default is "Creamy" or "Absolute Night" (depending "Use dark panels if available" option, from "View" menu).
	}

	void process(const ProcessArgs &args) override {
		// DSP processing...
		// Depending current model, set the relevant background color for panel.
		panelBackgroundColor = tblPanelBackgroundColor[Model];
	}

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Model", json_integer(Model));
		json_object_set_new(rootJ, "screwsDisposal", json_integer(screwsDisposal));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		json_t *ModelJ = json_object_get(rootJ, "Model");
		if (ModelJ)
			Model = json_integer_value(ModelJ);
			else {
				// Used to migrate to "Model" (instead of "Theme") in json (compatibility).
				json_t *ModelJ = json_object_get(rootJ, "Theme");
				if (ModelJ)
					Model = json_integer_value(ModelJ);
			}
		json_t *screwsDisposalJ = json_object_get(rootJ, "screwsDisposal");
		if (screwsDisposalJ)
			screwsDisposal = json_integer_value(screwsDisposalJ);
	}

};

///////////////////////////////////////////////////// CONTEXT-MENU (MODEL) //////////////////////////////////////////////////////

struct OhmerBlank2CreamyMenu : MenuItem {
	OhmerBlank2 *module;
	void onAction(const event::Action &e) override {
		module->Model = 0; // Model: Creamy.
	}
};

struct OhmerBlank2StageReproMenu : MenuItem {
	OhmerBlank2 *module;
	void onAction(const event::Action &e) override {
		module->Model = 1; // Model: Stage Repro.
	}
};

struct OhmerBlank2AbsoluteNightMenu : MenuItem {
	OhmerBlank2 *module;
	void onAction(const event::Action &e) override {
		module->Model = 2; // Model: Absolute Night.
	}
};

struct OhmerBlank2DarkSignatureMenu : MenuItem {
	OhmerBlank2 *module;
	void onAction(const event::Action &e) override {
		module->Model = 3; // Model: Dark Signature.
	}
};

struct OhmerBlank2DeepblueSignatureMenu : MenuItem {
	OhmerBlank2 *module;
	void onAction(const event::Action &e) override {
		module->Model = 4; // Model: Deepblue Signature.
	}
};

struct OhmerBlank2TitaniumSignatureMenu : MenuItem {
	OhmerBlank2 *module;
	void onAction(const event::Action &e) override {
		module->Model = 5; // Model: Titanium Signature.
	}
};

struct OhmerBlank2SubMenuItems : MenuItem {
	OhmerBlank2 *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		OhmerBlank2CreamyMenu *ohmerblank2creamymenu = new OhmerBlank2CreamyMenu;
		ohmerblank2creamymenu->text = "Creamy";
		ohmerblank2creamymenu->rightText = CHECKMARK(module->Model == 0);
		ohmerblank2creamymenu->module = module;
		menu->addChild(ohmerblank2creamymenu);

		OhmerBlank2StageReproMenu *ohmerblank2stagerepromenu = new OhmerBlank2StageReproMenu;
		ohmerblank2stagerepromenu->text = "Stage Repro";
		ohmerblank2stagerepromenu->rightText = CHECKMARK(module->Model == 1);
		ohmerblank2stagerepromenu->module = module;
		menu->addChild(ohmerblank2stagerepromenu);

		OhmerBlank2AbsoluteNightMenu *ohmerblank2absolutenightmenu = new OhmerBlank2AbsoluteNightMenu;
		ohmerblank2absolutenightmenu->text = "Absolute Night";
		ohmerblank2absolutenightmenu->rightText = CHECKMARK(module->Model == 2);
		ohmerblank2absolutenightmenu->module = module;
		menu->addChild(ohmerblank2absolutenightmenu);

		OhmerBlank2DarkSignatureMenu *ohmerblank2darksignaturemenu = new OhmerBlank2DarkSignatureMenu;
		ohmerblank2darksignaturemenu->text = "Dark \"Signature\"";
		ohmerblank2darksignaturemenu->rightText = CHECKMARK(module->Model == 3);
		ohmerblank2darksignaturemenu->module = module;
		menu->addChild(ohmerblank2darksignaturemenu);

		OhmerBlank2DeepblueSignatureMenu *ohmerblank2deepbluesignaturemenu = new OhmerBlank2DeepblueSignatureMenu;
		ohmerblank2deepbluesignaturemenu->text = "Deepblue \"Signature\"";
		ohmerblank2deepbluesignaturemenu->rightText = CHECKMARK(module->Model == 4);
		ohmerblank2deepbluesignaturemenu->module = module;
		menu->addChild(ohmerblank2deepbluesignaturemenu);

		OhmerBlank2TitaniumSignatureMenu *ohmerblank2titaniumsignaturemenu = new OhmerBlank2TitaniumSignatureMenu;
		ohmerblank2titaniumsignaturemenu->text = "Titanium \"Signature\"";
		ohmerblank2titaniumsignaturemenu->rightText = CHECKMARK(module->Model == 5);
		ohmerblank2titaniumsignaturemenu->module = module;
		menu->addChild(ohmerblank2titaniumsignaturemenu);

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
		if (module) {
			// Torx screws metal (silver, gold) are visible or hidden, depending selected model (from module's context-menu).
			// Silver Torx screws are visible only for non-"Signature" modules (Creamy, Stage Repro or Absolute Night).
			topLeftScrewGold->visible = ((module->screwsDisposal == 0) || (module->screwsDisposal == 2)) && (module->Model > 2);
			topRightScrewGold->visible = ((module->screwsDisposal == 1) || (module->screwsDisposal == 2)) && (module->Model > 2);
			bottomLeftScrewGold->visible = ((module->screwsDisposal == 1) || (module->screwsDisposal == 2)) && (module->Model > 2);
			bottomRightScrewGold->visible = ((module->screwsDisposal == 0) || (module->screwsDisposal == 2)) && (module->Model > 2);
			// Silver Torx screws visible or hidden (depending screws disposal from module's context-menu).
			topLeftScrewSilver->visible = ((module->screwsDisposal == 0) || (module->screwsDisposal == 2)) && (module->Model < 3);
			topRightScrewSilver->visible = ((module->screwsDisposal == 1) || (module->screwsDisposal == 2)) && (module->Model < 3);
			bottomLeftScrewSilver->visible = ((module->screwsDisposal == 1) || (module->screwsDisposal == 2)) && (module->Model < 3);
			bottomRightScrewSilver->visible = ((module->screwsDisposal == 0) || (module->screwsDisposal == 2)) && (module->Model < 3);
		}
		else {
			// !module - probably from module browser.
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
		}
		ModuleWidget::step();
	}

	void appendContextMenu(Menu *menu) override {
		OhmerBlank2 *module = dynamic_cast<OhmerBlank2*>(this->module);

		menu->addChild(new MenuSeparator);

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
