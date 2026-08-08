///////////////////////////////////////////////////////////////////////////////////////////////////////
////// BRK is a 4 HP expander, designed to setup jumpers for left-side RKD module.               //////
///////////////////////////////////////////////////////////////////////////////////////////////////////
////// Inspired from existing Eurorack hardware RCD Breakout companion module, by 4ms Company.   //////
////// Made with restricted 4ms Company permission (thank you, 4ms Company!).                    //////
////// This module uses its own algorithm (no original part of firmware code was used).          //////
////// 4ms Company name, logo, RCD, RCDBO, Rotating Clock Divider & RCD Breakout as TRADEMARKED! //////
///////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

struct BRK : Module {

	enum ParamIds {
		SWITCH_COUNTINGDOWN,
		SWITCH_GATE,
		SWITCH_MAXDIVRANGE16,
		SWITCH_MAXDIVRANGE32,
		SWITCH_SPREAD,
		SWITCH_AUTORESET,
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

	// Messages to left module (RKD) - this is the default.
	bool leftMessages[2][NUM_PARAMS] = {};

	// Messages to right module (RKD) - valid only if BRK is left, and no other BRK at right side (otherwise ignored).
	bool rightMessages[2][NUM_PARAMS] = {};

	// This flag indicates the panel must be updated.
	bool b_PanelUpdate = true;
	// This flag indicates if the panel is dark, or not.
	bool b_DarkPanel = false;

	// MODULE CONSTRUCTOR.

	BRK() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(SWITCH_COUNTINGDOWN, 0.0, 1.0, 0.0, "Counting Up/Dn"); // Off by default;
		configParam(SWITCH_GATE, 0.0, 1.0, 0.0, "Trig./Gate"); // Off by default;
		configParam(SWITCH_MAXDIVRANGE32, 0.0, 1.0, 1.0, "Max Div 32"); // On by default;
		configParam(SWITCH_MAXDIVRANGE16, 0.0, 1.0, 1.0, "Max Div 16"); // On by default;
		configParam(SWITCH_SPREAD, 0.0, 1.0, 0.0, "Spread"); // Off by default;
		configParam(SWITCH_AUTORESET, 0.0, 1.0, 0.0, "Auto-Reset"); // Off by default;
		leftExpander.producerMessage = leftMessages[0];
		leftExpander.consumerMessage = leftMessages[1];
		rightExpander.producerMessage = rightMessages[0];
		rightExpander.consumerMessage = rightMessages[1];
		b_PanelUpdate = true; // Be sure the panel will be updated!
		b_DarkPanel = rack::settings::preferDarkPanels;
	}

	void processBypass(const ProcessArgs &args) override {
		// DSP processing while the module is bypassed...

		// Panel change detection.
		if (b_DarkPanel != rack::settings::preferDarkPanels) {
			b_PanelUpdate = true; // Be sure the panel will be updated!
			b_DarkPanel = rack::settings::preferDarkPanels;
		}

	}

	void process(const ProcessArgs &args) override {
		// DSP processing...
		if (leftExpander.module && leftExpander.module->model == modelRKD) {
			// BRK expander is connected to RKD (right-side - take priority): sending switches positions to relevant jumpers to adjacent (left) RKD module.
			bool *message = (bool*)leftExpander.module->rightExpander.producerMessage;
			message[SWITCH_COUNTINGDOWN] = (params[SWITCH_COUNTINGDOWN].getValue() == 1.0);
			message[SWITCH_GATE] = (params[SWITCH_GATE].getValue() == 1.0);
			message[SWITCH_MAXDIVRANGE16] = (params[SWITCH_MAXDIVRANGE32].getValue() == 1.0);
			message[SWITCH_MAXDIVRANGE32] = (params[SWITCH_MAXDIVRANGE16].getValue() == 1.0);
			message[SWITCH_SPREAD] = (params[SWITCH_SPREAD].getValue() == 1.0);
			message[SWITCH_AUTORESET] = (params[SWITCH_AUTORESET].getValue() == 1.0);
			// Flip messages.
			leftExpander.module->rightExpander.messageFlipRequested = true;
		}
		else if (rightExpander.module && rightExpander.module->model == modelRKD) {
			// BRK expander is connected to RKD (left-side): sending switches positions to relevant jumpers to adjacent (right) RKD module.
			bool *message = (bool*)rightExpander.module->leftExpander.producerMessage;
			message[SWITCH_COUNTINGDOWN] = (params[SWITCH_COUNTINGDOWN].getValue() == 1.0);
			message[SWITCH_GATE] = (params[SWITCH_GATE].getValue() == 1.0);
			message[SWITCH_MAXDIVRANGE16] = (params[SWITCH_MAXDIVRANGE32].getValue() == 1.0);
			message[SWITCH_MAXDIVRANGE32] = (params[SWITCH_MAXDIVRANGE16].getValue() == 1.0);
			message[SWITCH_SPREAD] = (params[SWITCH_SPREAD].getValue() == 1.0);
			message[SWITCH_AUTORESET] = (params[SWITCH_AUTORESET].getValue() == 1.0);
			// Flip messages.
			rightExpander.module->leftExpander.messageFlipRequested = true;
		}

		// Panel change detection.
		if (b_DarkPanel != rack::settings::preferDarkPanels) {
			b_PanelUpdate = true; // Be sure the panel will be updated!
			b_DarkPanel = rack::settings::preferDarkPanels;
		}

	} // End of "process"...

}; // End of module (object) definition.

///////////////////////////////////////////////// MODULE WIDGET SECTION /////////////////////////////////////////////////

struct BRKWidget : ModuleWidget {
	// Silver Torx screws.
	SvgScrew *topScrewSilver;
	SvgScrew *bottomScrewSilver;
	// Golden Torx screws.
	SvgScrew *topScrewGold;
	SvgScrew *bottomScrewGold;

	BRKWidget(BRK *module) {
		setModule(module);
		box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		// Like original hardware module, we're using only two screws (top and bottom).
		// Top screw - silver (removed while PCB is shown).
		topScrewSilver = createWidget<Torx_Silver>(Vec(RACK_GRID_WIDTH, 0));
		addChild(topScrewSilver);
		// Bottom screw - silver (removed while PCB is shown).
		bottomScrewSilver = createWidget<Torx_Silver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomScrewSilver);
		// Top screw - gold (removed while PCB is shown).
		topScrewGold = createWidget<Torx_Gold>(Vec(RACK_GRID_WIDTH, 0));
		addChild(topScrewGold);
		// Bottom screw - gold (removed while PCB is shown).
		bottomScrewGold = createWidget<Torx_Gold>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomScrewGold);
		// Switch "Counting Up/Down". By default Off (counting up).
		addParam(createParam<RKDBRK_Switch>(Vec(10.3, 64.2), module, BRK::SWITCH_COUNTINGDOWN));
		// Switch "Trig/Gate". By default Off (trigger).
		addParam(createParam<RKDBRK_Switch>(Vec(10.3, 112.2), module, BRK::SWITCH_GATE));
		// Switch "Max-Div-Range 32".
		addParam(createParam<RKDBRK_Switch>(Vec(10.3, 160.2), module, BRK::SWITCH_MAXDIVRANGE32));
		// Switch "Max-Div-Range 16".
		addParam(createParam<RKDBRK_Switch>(Vec(10.3, 206.2), module, BRK::SWITCH_MAXDIVRANGE16));
		// Switch "Spread Off/On". By default Off (spread off).
		addParam(createParam<RKDBRK_Switch>(Vec(10.3, 256.2), module, BRK::SWITCH_SPREAD));
		// Switch "Auto-Reset Off/On". By default Off (auto-reset is disabled).
		addParam(createParam<RKDBRK_Switch>(Vec(10.3, 304.2), module, BRK::SWITCH_AUTORESET));
	};

	void selectPanel() {
		if (rack::settings::preferDarkPanels)
			setPanel(createPanel(asset::plugin(pluginInstance, "res/BRK_dark.svg")));
			else setPanel(createPanel(asset::plugin(pluginInstance, "res/BRK_light.svg")));
		// Golden screws (dark panel).
		topScrewGold->visible = rack::settings::preferDarkPanels;
		bottomScrewGold->visible = rack::settings::preferDarkPanels;
		// Silver screws (light panel).
		topScrewSilver->visible = !rack::settings::preferDarkPanels;
		bottomScrewSilver->visible = !rack::settings::preferDarkPanels;
	}

	void step() override {
		BRK *module = dynamic_cast<BRK*>(this->module);
		// The module isn't instantiated: probably from module browser...
		// Default presented model is light or dark, depending "Use dark panels if available" VCV Rack 2's global option (from "View" menu).
		if (!module) {
			// Select the relevant panel (view from module browser).
			selectPanel();
			// Required to avoid crash!
			if (!module)
				return;
		}
		if (module->b_PanelUpdate) {
			// Select the relevant panel.
			selectPanel();
			// Reset the panel update flag.
			module->b_PanelUpdate = false;
		}
		//
		ModuleWidget::step();
	}

};

Model *modelBRK = createModel<BRK, BRKWidget>("BRK");
