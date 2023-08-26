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
	int Model; // 0 = Creamy, 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Titanium Signature.

	// Panel color (default is Creamy).
	NVGcolor panelBackgroundColor = nvgRGB(0xd2, 0xd2, 0xcd);

	OhmerBlank32() {
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
	}

};

///////////////////////////////////////////////////// CONTEXT-MENU //////////////////////////////////////////////////////

struct OhmerBlank32CreamyMenu : MenuItem {
	OhmerBlank32 *module;
	void onAction(const event::Action &e) override {
		module->Model = 0; // Model: Creamy.
	}
};

struct OhmerBlank32StageReproMenu : MenuItem {
	OhmerBlank32 *module;
	void onAction(const event::Action &e) override {
		module->Model = 1; // Model: Stage Repro.
	}
};

struct OhmerBlank32AbsoluteNightMenu : MenuItem {
	OhmerBlank32 *module;
	void onAction(const event::Action &e) override {
		module->Model = 2; // Model: Absolute Night.
	}
};

struct OhmerBlank32DarkSignatureMenu : MenuItem {
	OhmerBlank32 *module;
	void onAction(const event::Action &e) override {
		module->Model = 3; // Model: Dark Signature.
	}
};

struct OhmerBlank32DeepblueSignatureMenu : MenuItem {
	OhmerBlank32 *module;
	void onAction(const event::Action &e) override {
		module->Model = 4; // Model: Deepblue Signature.
	}
};

struct OhmerBlank32TitaniumSignatureMenu : MenuItem {
	OhmerBlank32 *module;
	void onAction(const event::Action &e) override {
		module->Model = 5; // Model: Titanium Signature.
	}
};

struct OhmerBlank32SubMenuItems : MenuItem {
	OhmerBlank32 *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		OhmerBlank32CreamyMenu *ohmerblank32creamymenu = new OhmerBlank32CreamyMenu;
		ohmerblank32creamymenu->text = "Creamy";
		ohmerblank32creamymenu->rightText = CHECKMARK(module->Model == 0);
		ohmerblank32creamymenu->module = module;
		menu->addChild(ohmerblank32creamymenu);

		OhmerBlank32StageReproMenu *ohmerblank32stagerepromenu = new OhmerBlank32StageReproMenu;
		ohmerblank32stagerepromenu->text = "Stage Repro";
		ohmerblank32stagerepromenu->rightText = CHECKMARK(module->Model == 1);
		ohmerblank32stagerepromenu->module = module;
		menu->addChild(ohmerblank32stagerepromenu);

		OhmerBlank32AbsoluteNightMenu *ohmerblank32absolutenightmenu = new OhmerBlank32AbsoluteNightMenu;
		ohmerblank32absolutenightmenu->text = "Absolute Night";
		ohmerblank32absolutenightmenu->rightText = CHECKMARK(module->Model == 2);
		ohmerblank32absolutenightmenu->module = module;
		menu->addChild(ohmerblank32absolutenightmenu);

		OhmerBlank32DarkSignatureMenu *ohmerblank32darksignaturemenu = new OhmerBlank32DarkSignatureMenu;
		ohmerblank32darksignaturemenu->text = "Dark \"Signature\"";
		ohmerblank32darksignaturemenu->rightText = CHECKMARK(module->Model == 3);
		ohmerblank32darksignaturemenu->module = module;
		menu->addChild(ohmerblank32darksignaturemenu);

		OhmerBlank32DeepblueSignatureMenu *ohmerblank32deepbluesignaturemenu = new OhmerBlank32DeepblueSignatureMenu;
		ohmerblank32deepbluesignaturemenu->text = "Deepblue \"Signature\"";
		ohmerblank32deepbluesignaturemenu->rightText = CHECKMARK(module->Model == 4);
		ohmerblank32deepbluesignaturemenu->module = module;
		menu->addChild(ohmerblank32deepbluesignaturemenu);

		OhmerBlank32TitaniumSignatureMenu *ohmerblank32titaniumsignaturemenu = new OhmerBlank32TitaniumSignatureMenu;
		ohmerblank32titaniumsignaturemenu->text = "Titanium \"Signature\"";
		ohmerblank32titaniumsignaturemenu->rightText = CHECKMARK(module->Model == 5);
		ohmerblank32titaniumsignaturemenu->module = module;
		menu->addChild(ohmerblank32titaniumsignaturemenu);

		return menu;
	}
};

///////////////////////////////////////////////// PANEL BACKGROUND COLOR /////////////////////////////////////////////////

struct OhmerBlank32Background : TransparentWidget {
	OhmerBlank32 *module;

	OhmerBlank32Background() {
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
		if (module) {
			// Torx screws metal (silver, gold) are visible or hidden, depending selected model (from module's context-menu).
			// Gold Torx screws are visible only for "Signature" modules (Dark Signature, Deepblue Signature or Titanium Signature).
			topLeftScrewGold->visible = (module->Model > 2);
			topRightScrewGold->visible = (module->Model > 2);
			bottomLeftScrewGold->visible = (module->Model > 2);
			bottomRightScrewGold->visible = (module->Model > 2);
			// Silver Torx screws are visible only for non-"Signature" modules (Creamy, Stage Repro or Absolute Night).
			topLeftScrewSilver->visible = (module->Model < 3);
			topRightScrewSilver->visible = (module->Model < 3);
			bottomLeftScrewSilver->visible = (module->Model < 3);
			bottomRightScrewSilver->visible = (module->Model < 3);
		}
		else {
			// !module - probably from module browser.
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
		}
		ModuleWidget::step();
	}

	void appendContextMenu(Menu *menu) override {
		OhmerBlank32 *module = dynamic_cast<OhmerBlank32*>(this->module);

		menu->addChild(new MenuSeparator);

		OhmerBlank32SubMenuItems *ohmerblank32submenuitems = new OhmerBlank32SubMenuItems;
		ohmerblank32submenuitems->text = "Model";
		ohmerblank32submenuitems->rightText = RIGHT_ARROW;
		ohmerblank32submenuitems->module = module;
		menu->addChild(ohmerblank32submenuitems);
	}

};

Model *modelBlankPanel32 = createModel<OhmerBlank32, OhmerBlank32Widget>("OhmerBlank32");
