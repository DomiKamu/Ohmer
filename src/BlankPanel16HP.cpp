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
	int Model; // 0 = Creamy, 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Titanium Signature.

	// Panel color (default is Creamy).
	NVGcolor panelBackgroundColor = nvgRGB(0xd2, 0xd2, 0xcd);

	OhmerBlank16() {
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

struct OhmerBlank16CreamyMenu : MenuItem {
	OhmerBlank16 *module;
	void onAction(const event::Action &e) override {
		module->Model = 0; // Model: default Classic (beige).
	}
};

struct OhmerBlank16StageReproMenu : MenuItem {
	OhmerBlank16 *module;
	void onAction(const event::Action &e) override {
		module->Model = 1; // Model: Stage Repro.
	}
};

struct OhmerBlank16AbsoluteNightMenu : MenuItem {
	OhmerBlank16 *module;
	void onAction(const event::Action &e) override {
		module->Model = 2; // Model: Absolute Night.
	}
};

struct OhmerBlank16DarkSignatureMenu : MenuItem {
	OhmerBlank16 *module;
	void onAction(const event::Action &e) override {
		module->Model = 3; // Model: Dark Signature.
	}
};

struct OhmerBlank16DeepblueSignatureMenu : MenuItem {
	OhmerBlank16 *module;
	void onAction(const event::Action &e) override {
		module->Model = 4; // Model: Deepblue Signature.
	}
};

struct OhmerBlank16TitaniumSignatureMenu : MenuItem {
	OhmerBlank16 *module;
	void onAction(const event::Action &e) override {
		module->Model = 5; // Model: Titanium Signature.
	}
};

struct OhmerBlank16SubMenuItems : MenuItem {
	OhmerBlank16 *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		OhmerBlank16CreamyMenu *ohmerblank16creamymenu = new OhmerBlank16CreamyMenu;
		ohmerblank16creamymenu->text = "Creamy";
		ohmerblank16creamymenu->rightText = CHECKMARK(module->Model == 0);
		ohmerblank16creamymenu->module = module;
		menu->addChild(ohmerblank16creamymenu);

		OhmerBlank16StageReproMenu *ohmerblank16stagerepromenu = new OhmerBlank16StageReproMenu;
		ohmerblank16stagerepromenu->text = "Stage Repro";
		ohmerblank16stagerepromenu->rightText = CHECKMARK(module->Model == 1);
		ohmerblank16stagerepromenu->module = module;
		menu->addChild(ohmerblank16stagerepromenu);

		OhmerBlank16AbsoluteNightMenu *ohmerblank16absolutenightmenu = new OhmerBlank16AbsoluteNightMenu;
		ohmerblank16absolutenightmenu->text = "Absolute Night";
		ohmerblank16absolutenightmenu->rightText = CHECKMARK(module->Model == 2);
		ohmerblank16absolutenightmenu->module = module;
		menu->addChild(ohmerblank16absolutenightmenu);

		OhmerBlank16DarkSignatureMenu *ohmerblank16darksignaturemenu = new OhmerBlank16DarkSignatureMenu;
		ohmerblank16darksignaturemenu->text = "Dark \"Signature\"";
		ohmerblank16darksignaturemenu->rightText = CHECKMARK(module->Model == 3);
		ohmerblank16darksignaturemenu->module = module;
		menu->addChild(ohmerblank16darksignaturemenu);

		OhmerBlank16DeepblueSignatureMenu *ohmerblank16deepbluesignaturemenu = new OhmerBlank16DeepblueSignatureMenu;
		ohmerblank16deepbluesignaturemenu->text = "Deepblue \"Signature\"";
		ohmerblank16deepbluesignaturemenu->rightText = CHECKMARK(module->Model == 4);
		ohmerblank16deepbluesignaturemenu->module = module;
		menu->addChild(ohmerblank16deepbluesignaturemenu);

		OhmerBlank16TitaniumSignatureMenu *ohmerblank16titaniumsignaturemenu = new OhmerBlank16TitaniumSignatureMenu;
		ohmerblank16titaniumsignaturemenu->text = "Titanium \"Signature\"";
		ohmerblank16titaniumsignaturemenu->rightText = CHECKMARK(module->Model == 5);
		ohmerblank16titaniumsignaturemenu->module = module;
		menu->addChild(ohmerblank16titaniumsignaturemenu);

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
		OhmerBlank16 *module = dynamic_cast<OhmerBlank16*>(this->module);

		menu->addChild(new MenuSeparator);

		OhmerBlank16SubMenuItems *ohmerblank16submenuitems = new OhmerBlank16SubMenuItems;
		ohmerblank16submenuitems->text = "Model";
		ohmerblank16submenuitems->rightText = RIGHT_ARROW;
		ohmerblank16submenuitems->module = module;
		menu->addChild(ohmerblank16submenuitems);
	}

};

Model *modelBlankPanel16 = createModel<OhmerBlank16, OhmerBlank16Widget>("OhmerBlank16");
