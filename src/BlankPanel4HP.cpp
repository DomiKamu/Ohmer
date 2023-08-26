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
	int Model; // 0 = Creamy, 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Titanium Signature.

	// Panel color (default is Creamy).
	NVGcolor panelBackgroundColor = nvgRGB(0xd2, 0xd2, 0xcd);

	OhmerBlank4() {
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

struct OhmerBlank4CreamyMenu : MenuItem {
	OhmerBlank4 *module;
	void onAction(const event::Action &e) override {
		module->Model = 0; // Model: Creamy.
	}
};

struct OhmerBlank4StageReproMenu : MenuItem {
	OhmerBlank4 *module;
	void onAction(const event::Action &e) override {
		module->Model = 1; // Model: Stage Repro.
	}
};

struct OhmerBlank4AbsoluteNightMenu : MenuItem {
	OhmerBlank4 *module;
	void onAction(const event::Action &e) override {
		module->Model = 2; // Model: Absolute Night.
	}
};

struct OhmerBlank4DarkSignatureMenu : MenuItem {
	OhmerBlank4 *module;
	void onAction(const event::Action &e) override {
		module->Model = 3; // Model: Dark Signature.
	}
};

struct OhmerBlank4DeepblueSignatureMenu : MenuItem {
	OhmerBlank4 *module;
	void onAction(const event::Action &e) override {
		module->Model = 4; // Model: Deepblue Signature.
	}
};

struct OhmerBlank4TitaniumSignatureMenu : MenuItem {
	OhmerBlank4 *module;
	void onAction(const event::Action &e) override {
		module->Model = 5; // Model: Titanium Signature.
	}
};

struct OhmerBlank4SubMenuItems : MenuItem {
	OhmerBlank4 *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		OhmerBlank4CreamyMenu *ohmerblank4creamymenu = new OhmerBlank4CreamyMenu;
		ohmerblank4creamymenu->text = "Creamy";
		ohmerblank4creamymenu->rightText = CHECKMARK(module->Model == 0);
		ohmerblank4creamymenu->module = module;
		menu->addChild(ohmerblank4creamymenu);

		OhmerBlank4StageReproMenu *ohmerblank4stagerepromenu = new OhmerBlank4StageReproMenu;
		ohmerblank4stagerepromenu->text = "Stage Repro";
		ohmerblank4stagerepromenu->rightText = CHECKMARK(module->Model == 1);
		ohmerblank4stagerepromenu->module = module;
		menu->addChild(ohmerblank4stagerepromenu);

		OhmerBlank4AbsoluteNightMenu *ohmerblank4absolutenightmenu = new OhmerBlank4AbsoluteNightMenu;
		ohmerblank4absolutenightmenu->text = "Absolute Night";
		ohmerblank4absolutenightmenu->rightText = CHECKMARK(module->Model == 2);
		ohmerblank4absolutenightmenu->module = module;
		menu->addChild(ohmerblank4absolutenightmenu);

		OhmerBlank4DarkSignatureMenu *ohmerblank4darksignaturemenu = new OhmerBlank4DarkSignatureMenu;
		ohmerblank4darksignaturemenu->text = "Dark \"Signature\"";
		ohmerblank4darksignaturemenu->rightText = CHECKMARK(module->Model == 3);
		ohmerblank4darksignaturemenu->module = module;
		menu->addChild(ohmerblank4darksignaturemenu);

		OhmerBlank4DeepblueSignatureMenu *ohmerblank4deepbluesignaturemenu = new OhmerBlank4DeepblueSignatureMenu;
		ohmerblank4deepbluesignaturemenu->text = "Deepblue \"Signature\"";
		ohmerblank4deepbluesignaturemenu->rightText = CHECKMARK(module->Model == 4);
		ohmerblank4deepbluesignaturemenu->module = module;
		menu->addChild(ohmerblank4deepbluesignaturemenu);

		OhmerBlank4TitaniumSignatureMenu *ohmerblank4titaniumsignaturemenu = new OhmerBlank4TitaniumSignatureMenu;
		ohmerblank4titaniumsignaturemenu->text = "Titanium \"Signature\"";
		ohmerblank4titaniumsignaturemenu->rightText = CHECKMARK(module->Model == 5);
		ohmerblank4titaniumsignaturemenu->module = module;
		menu->addChild(ohmerblank4titaniumsignaturemenu);

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
			else nvgFillColor(args.vg, rack::settings::preferDarkPanels ? nvgRGB(0x00, 0x00, 0x00) : nvgRGB(0xd2, 0xd2, 0xcd));
		nvgFill(args.vg);
	}

};


///////////////////////////////////////////////// MODULE WIDGET SECTION /////////////////////////////////////////////////

struct OhmerBlank4Widget : ModuleWidget {
	// Panel (transparent widget).
	OhmerBlank4Background *blankPanel;
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
		OhmerBlank4 *module = dynamic_cast<OhmerBlank4*>(this->module);
		if (module) {
			// Torx screws metal (silver, gold) are visible or hidden, depending selected model (from module's context-menu).
			// Silver Torx screws are visible only for non-"Signature" modules (Creamy, Stage Repro or Absolute Night).
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
		OhmerBlank4 *module = dynamic_cast<OhmerBlank4*>(this->module);

		menu->addChild(new MenuSeparator);

		OhmerBlank4SubMenuItems *ohmerblank4submenuitems = new OhmerBlank4SubMenuItems;
		ohmerblank4submenuitems->text = "Model";
		ohmerblank4submenuitems->rightText = RIGHT_ARROW;
		ohmerblank4submenuitems->module = module;
		menu->addChild(ohmerblank4submenuitems);
	}

};

Model *modelBlankPanel4 = createModel<OhmerBlank4, OhmerBlank4Widget>("OhmerBlank4");
