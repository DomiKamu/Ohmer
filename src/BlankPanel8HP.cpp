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
	int Model; // 0 = Creamy, 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Titanium Signature.

	// Panel color (default is Creamy).
	NVGcolor panelBackgroundColor = nvgRGB(0xd2, 0xd2, 0xcd);

	OhmerBlank8() {
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

struct OhmerBlank8CreamyMenu : MenuItem {
	OhmerBlank8 *module;
	void onAction(const event::Action &e) override {
		module->Model = 0; // Model: Creamy.
	}
};

struct OhmerBlank8StageReproMenu : MenuItem {
	OhmerBlank8 *module;
	void onAction(const event::Action &e) override {
		module->Model = 1; // Model: Stage Repro.
	}
};

struct OhmerBlank8AbsoluteNightMenu : MenuItem {
	OhmerBlank8 *module;
	void onAction(const event::Action &e) override {
		module->Model = 2; // Model: Absolute Night.
	}
};

struct OhmerBlank8DarkSignatureMenu : MenuItem {
	OhmerBlank8 *module;
	void onAction(const event::Action &e) override {
		module->Model = 3; // Model: Dark Signature.
	}
};

struct OhmerBlank8DeepblueSignatureMenu : MenuItem {
	OhmerBlank8 *module;
	void onAction(const event::Action &e) override {
		module->Model = 4; // Model: Deepblue Signature.
	}
};

struct OhmerBlank8TitaniumSignatureMenu : MenuItem {
	OhmerBlank8 *module;
	void onAction(const event::Action &e) override {
		module->Model = 5; // Model: Titanium Signature.
	}
};

struct OhmerBlank8SubMenuItems : MenuItem {
	OhmerBlank8 *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		OhmerBlank8CreamyMenu *ohmerblank8creamymenu = new OhmerBlank8CreamyMenu;
		ohmerblank8creamymenu->text = "Creamy";
		ohmerblank8creamymenu->rightText = CHECKMARK(module->Model == 0);
		ohmerblank8creamymenu->module = module;
		menu->addChild(ohmerblank8creamymenu);

		OhmerBlank8StageReproMenu *ohmerblank8stagerepromenu = new OhmerBlank8StageReproMenu;
		ohmerblank8stagerepromenu->text = "Stage Repro";
		ohmerblank8stagerepromenu->rightText = CHECKMARK(module->Model == 1);
		ohmerblank8stagerepromenu->module = module;
		menu->addChild(ohmerblank8stagerepromenu);

		OhmerBlank8AbsoluteNightMenu *ohmerblank8absolutenightmenu = new OhmerBlank8AbsoluteNightMenu;
		ohmerblank8absolutenightmenu->text = "Absolute Night";
		ohmerblank8absolutenightmenu->rightText = CHECKMARK(module->Model == 2);
		ohmerblank8absolutenightmenu->module = module;
		menu->addChild(ohmerblank8absolutenightmenu);

		OhmerBlank8DarkSignatureMenu *ohmerblank8darksignaturemenu = new OhmerBlank8DarkSignatureMenu;
		ohmerblank8darksignaturemenu->text = "Dark \"Signature\"";
		ohmerblank8darksignaturemenu->rightText = CHECKMARK(module->Model == 3);
		ohmerblank8darksignaturemenu->module = module;
		menu->addChild(ohmerblank8darksignaturemenu);

		OhmerBlank8DeepblueSignatureMenu *ohmerblank8deepbluesignaturemenu = new OhmerBlank8DeepblueSignatureMenu;
		ohmerblank8deepbluesignaturemenu->text = "Deepblue \"Signature\"";
		ohmerblank8deepbluesignaturemenu->rightText = CHECKMARK(module->Model == 4);
		ohmerblank8deepbluesignaturemenu->module = module;
		menu->addChild(ohmerblank8deepbluesignaturemenu);

		OhmerBlank8TitaniumSignatureMenu *ohmerblank8titaniumsignaturemenu = new OhmerBlank8TitaniumSignatureMenu;
		ohmerblank8titaniumsignaturemenu->text = "Titanium \"Signature\"";
		ohmerblank8titaniumsignaturemenu->rightText = CHECKMARK(module->Model == 5);
		ohmerblank8titaniumsignaturemenu->module = module;
		menu->addChild(ohmerblank8titaniumsignaturemenu);

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
			else nvgFillColor(args.vg, rack::settings::preferDarkPanels ? nvgRGB(0x00, 0x00, 0x00) : nvgRGB(0xd2, 0xd2, 0xcd));
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
		OhmerBlank8 *module = dynamic_cast<OhmerBlank8*>(this->module);

		menu->addChild(new MenuSeparator);

		OhmerBlank8SubMenuItems *ohmerblank8submenuitems = new OhmerBlank8SubMenuItems;
		ohmerblank8submenuitems->text = "Model";
		ohmerblank8submenuitems->rightText = RIGHT_ARROW;
		ohmerblank8submenuitems->module = module;
		menu->addChild(ohmerblank8submenuitems);
	}

};

Model *modelBlankPanel8 = createModel<OhmerBlank8, OhmerBlank8Widget>("OhmerBlank8");
