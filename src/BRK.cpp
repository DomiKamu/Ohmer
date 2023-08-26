///////////////////////////////////////////////////////////////////////////////////////////////////////
////// BRK is a 4 HP expander, designed to setup jumpers for left-side RKD module.               //////
///////////////////////////////////////////////////////////////////////////////////////////////////////
////// Inspired from existing Eurorack hardware RCD Breakout companion module, by 4ms Company.   //////
////// Done with restricted 4ms Company permission (thank you, 4ms!).                            //////
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

	} // end of "process"...

}; // End of module (object) definition.

///////////////////////////////////////////////// MODULE WIDGET SECTION /////////////////////////////////////////////////

struct BRKWidget : ModuleWidget {
	// BRK panels (light, dark).
	SvgPanel *panelBRKlight;
	SvgPanel *panelBRKdark;

	BRKWidget(BRK *module) {
		setModule(module);
		box.size = Vec(4 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		// BRK light panel.
		panelBRKlight = new SvgPanel();
		panelBRKlight->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BRK_light.svg")));
		panelBRKlight->visible = !rack::settings::preferDarkPanels; // Light panel.
		addChild(panelBRKlight);
		// BRK dark panel.
		panelBRKdark = new SvgPanel();
		panelBRKdark->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/BRK_dark.svg")));
		panelBRKdark->visible = rack::settings::preferDarkPanels; // Dark panel.
		addChild(panelBRKdark);
		// Like original RCD Breakout module, we're using only two screws (top and bottom).
		// Top screw.
		addChild(createWidget<Torx_Silver>(Vec(RACK_GRID_WIDTH, 0)));
		// Bottom screw (it recovers a bit the Ohmer logo, it's normal!)
		addChild(createWidget<Torx_Silver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
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

	void step() override {
		// Depending "Use dark panels if available" option (from "View" menu), use the light or dark panel.
		panelBRKlight->visible = !rack::settings::preferDarkPanels;
		panelBRKdark->visible = rack::settings::preferDarkPanels;
		ModuleWidget::step();
	}

};

Model *modelBRK = createModel<BRK, BRKWidget>("BRK");
