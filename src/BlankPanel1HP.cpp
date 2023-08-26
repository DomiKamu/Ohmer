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
	int Model; // 0 = Creamy, 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Titanium Signature.

	// Panel color (default is Creamy).
	NVGcolor panelBackgroundColor = nvgRGB(0xd2, 0xd2, 0xcd);

	OhmerBlank1() {
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

struct OhmerBlank1CreamyMenu : MenuItem {
	OhmerBlank1 *module;
	void onAction(const event::Action &e) override {
		module->Model = 0; // Model: Creamy.
	}
};

struct OhmerBlank1StageReproMenu : MenuItem {
	OhmerBlank1 *module;
	void onAction(const event::Action &e) override {
		module->Model = 1; // Model: Stage Repro.
	}
};

struct OhmerBlank1AbsoluteNightMenu : MenuItem {
	OhmerBlank1 *module;
	void onAction(const event::Action &e) override {
		module->Model = 2; // Model: Absolute Night.
	}
};

struct OhmerBlank1DarkSignatureMenu : MenuItem {
	OhmerBlank1 *module;
	void onAction(const event::Action &e) override {
		module->Model = 3; // Model: Dark Signature.
	}
};

struct OhmerBlank1DeepblueSignatureMenu : MenuItem {
	OhmerBlank1 *module;
	void onAction(const event::Action &e) override {
		module->Model = 4; // Model: Deepblue Signature.
	}
};

struct OhmerBlank1TitaniumSignatureMenu : MenuItem {
	OhmerBlank1 *module;
	void onAction(const event::Action &e) override {
		module->Model = 5; // Model: Titanium Signature.
	}
};

struct OhmerBlank1SubMenuItems : MenuItem {
	OhmerBlank1 *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		OhmerBlank1CreamyMenu *ohmerblank1creamymenu = new OhmerBlank1CreamyMenu;
		ohmerblank1creamymenu->text = "Creamy";
		ohmerblank1creamymenu->rightText = CHECKMARK(module->Model == 0);
		ohmerblank1creamymenu->module = module;
		menu->addChild(ohmerblank1creamymenu);

		OhmerBlank1StageReproMenu *ohmerblank1stagerepromenu = new OhmerBlank1StageReproMenu;
		ohmerblank1stagerepromenu->text = "Stage Repro";
		ohmerblank1stagerepromenu->rightText = CHECKMARK(module->Model == 1);
		ohmerblank1stagerepromenu->module = module;
		menu->addChild(ohmerblank1stagerepromenu);

		OhmerBlank1AbsoluteNightMenu *ohmerblank1absolutenightmenu = new OhmerBlank1AbsoluteNightMenu;
		ohmerblank1absolutenightmenu->text = "Absolute Night";
		ohmerblank1absolutenightmenu->rightText = CHECKMARK(module->Model == 2);
		ohmerblank1absolutenightmenu->module = module;
		menu->addChild(ohmerblank1absolutenightmenu);

		OhmerBlank1DarkSignatureMenu *ohmerblank1darksignaturemenu = new OhmerBlank1DarkSignatureMenu;
		ohmerblank1darksignaturemenu->text = "Dark \"Signature\"";
		ohmerblank1darksignaturemenu->rightText = CHECKMARK(module->Model == 3);
		ohmerblank1darksignaturemenu->module = module;
		menu->addChild(ohmerblank1darksignaturemenu);

		OhmerBlank1DeepblueSignatureMenu *ohmerblank1deepbluesignaturemenu = new OhmerBlank1DeepblueSignatureMenu;
		ohmerblank1deepbluesignaturemenu->text = "Deepblue \"Signature\"";
		ohmerblank1deepbluesignaturemenu->rightText = CHECKMARK(module->Model == 4);
		ohmerblank1deepbluesignaturemenu->module = module;
		menu->addChild(ohmerblank1deepbluesignaturemenu);

		OhmerBlank1TitaniumSignatureMenu *ohmerblank1titaniumsignaturemenu = new OhmerBlank1TitaniumSignatureMenu;
		ohmerblank1titaniumsignaturemenu->text = "Titanium \"Signature\"";
		ohmerblank1titaniumsignaturemenu->rightText = CHECKMARK(module->Model == 5);
		ohmerblank1titaniumsignaturemenu->module = module;
		menu->addChild(ohmerblank1titaniumsignaturemenu);

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
			else nvgFillColor(args.vg, rack::settings::preferDarkPanels ? nvgRGB(0x00, 0x00, 0x00) : nvgRGB(0xd2, 0xd2, 0xcd));
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
			// Silver Torx screws are visible only for non-"Signature" modules (Creamy, Stage Repro or Absolute Night).
			topScrewSilver->visible = (module->Model < 3);
			bottomScrewSilver->visible = (module->Model < 3);
			// Gold Torx screws are visible only for "Signature" modules (Dark Signature, Deepblue Signature or Titanium Signature).
			topScrewGold->visible = (module->Model > 2);
			bottomScrewGold->visible = (module->Model > 2);
		}
		else {
			// !module - probably from module browser.
			// By default, silver screws are visible for default Creamy or Absolute Night...
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
		menu->addChild(new MenuSeparator);
		OhmerBlank1SubMenuItems *ohmerblank1submenuitems = new OhmerBlank1SubMenuItems;
		ohmerblank1submenuitems->text = "Model";
		ohmerblank1submenuitems->rightText = RIGHT_ARROW;
		ohmerblank1submenuitems->module = module;
		menu->addChild(ohmerblank1submenuitems);
	}

};

Model *modelBlankPanel1 = createModel<OhmerBlank1, OhmerBlank1Widget>("OhmerBlank1");
