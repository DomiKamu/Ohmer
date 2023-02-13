#include "rack.hpp"
#include "OhmerWidgets.hpp"

using namespace rack;

extern Plugin *pluginInstance;

extern Model *modelKlokSpid;
extern Model *modelRKD;
extern Model *modelBRK;
extern Model *modelMetriks;
extern Model *modelPolaritySwitch;
extern Model *modelSplitter;
extern Model *modelBlankPanel1;
extern Model *modelBlankPanel2;
extern Model *modelBlankPanel4;
extern Model *modelBlankPanel8;
extern Model *modelBlankPanel16;
extern Model *modelBlankPanel32;


//// COLOR TABLE USED FOR DOT-MATRIX DISPLAY (REGARDLING SELECTED MODEL) - KLOKSPID & METRIKS MODULES.

static const NVGcolor tblDMDtextColor[6] = {
	nvgRGB(0x08, 0x08, 0x08), // LCD-like for Classic.
	nvgRGB(0x08, 0x08, 0x08), // LCD-like for Stage Repro.
	nvgRGB(0x08, 0x08, 0x08), // LCD-like for Absolute Night.
	nvgRGB(0xe0, 0xe0, 0xff), // Blue plasma-like for Dark "Signature".
	nvgRGB(0xff, 0x8a, 0x00), // Orange plasma-like for Deepblue "Signature".
	nvgRGB(0xb0, 0xff, 0xff) // Light cyan plasma-like for Carbon "Signature".
};

//// BACKGROUND COLOR TABLES USED FOR BLANK PANELS.

static const NVGcolor tblPanelBackgroundColor[6] = {
	nvgRGB(0xd2, 0xd2, 0xcd), // Classic blank panel.
	nvgRGB(0x70, 0x00, 0x00), // Stage Repro blank panel.
	nvgRGB(0x00, 0x00, 0x00), // Absolute Night blank panel.
	nvgRGB(0x0a, 0x0a, 0x1e), // Dark "Signature" blank panel.
	nvgRGB(0x1a, 0x1a, 0x57), // Deepblue "Signature" blank panels
	nvgRGB(0x30, 0x30, 0x30) // Carbon "Signature" blank panel.
};

//// CUSTOM COMPONENTS (SCREWS, JACKS, KNOBS, ENCODERS, BUTTONS, LEDS).

// Custom silver Torx screw (used for classic Ohmer).
struct Torx_Silver : SvgScrew {
	Torx_Silver() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Torx_Silver.svg")));
	}
};

// Custom gold Torx screw (used for "Dark Signature" line).
struct Torx_Gold : SvgScrew {
	Torx_Gold() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Torx_Gold.svg")));
	}
};

// Silver momentary button (used by standard-line KlokSpid modules).
// This button is used for:
// - BPM start/stop toggle (KlokSpid module acting as standalone BPM clock generator).
// - entering Setup (by holding this button).
// - advance to next Setup parameter (and exit Setup).
struct KS_ButtonSilver : SvgSwitch {
	KS_ButtonSilver() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/XB_Button_Up_Silver.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/XB_Button_Down_Silver.svg")));
	}
};

// Gold momentary button (used by Signature-line KlokSpid modules only).
struct KS_ButtonGold : SvgSwitch {
	KS_ButtonGold() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/XB_Button_Up_Gold.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/XB_Button_Down_Gold.svg")));
	}
};

// RKD jumper shunts (working as toggle switch).
struct RKD_Jumper : SvgSwitch {
	RKD_Jumper() {
		momentary = false;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RKD_PCB_BJ_Off.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RKD_PCB_BJ_On.svg")));
	}
};

// RKDBRK toggle switch (working as toggle switch).
struct RKDBRK_Switch : SvgSwitch {
	RKDBRK_Switch() {
		momentary = false;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RKDBRK_NKKH_0.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/RKDBRK_NKKH_1.svg")));
	}
};

struct DynSVGPort : DynamicSVGPort {
	DynSVGPort() {
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/XB_Port_Silver.svg")));
		addFrameAlt(asset::plugin(pluginInstance, "res/components/XB_Port_Gold.svg"));
		shadow->blurRadius = 1.0f;
	}
};

// Custom nickel metal port, with red in-ring (input port), used only by RKD & RKD-BRK modules (CLK jack). Derived from default CL1362.svg
struct CL1362_In : SvgPort {
	CL1362_In() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/CL1362_In.svg")));
	}
};

// Custom nickel metal port, with red in-ring (input port), used only by RKD & RKD-BRK modules (90° rotated for... ROTATE and RESET input ports). Derived from default CL1362.svg
struct CL1362_In_RR : SvgPort {
	CL1362_In_RR() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/CL1362_In_RR.svg")));
	}
};

// Custom nickel metal port, with green in-ring (output port), used only by RKD & RKD-BRK modules. Derived from default CL1362.svg
struct CL1362_Out : SvgPort {
	CL1362_Out() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/CL1362_Out.svg")));
	}
};

// Freeware "Moog-style" continuous encoder, used by KlokSpid module.
struct KS_Encoder : SvgKnob {
	KS_Encoder() {
		minAngle = -1.0 * M_PI;
		maxAngle = M_PI;
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/KS_Encoder.svg")));
	}

	void onEnter(const event::Enter &e) override {
	}

	void onLeave(const event::Leave &e) override {
	}

	void step() override {
		ParamWidget::step();
	}

};

// Custom orange color used by two small LEDs (CV-RATIO, start/stop), KlokSpid module.
// Also, this color is used for medium LED located below CV/TRIG port (KlokSpid module).
struct KlokSpidOrangeLight : GrayModuleLightWidget {
	KlokSpidOrangeLight() {
		addBaseColor(nvgRGB(0xe8, 0xad, 0x10));
	}
};

// White LED color for RKD & RKD-BRK modules (used for CLK and output 8).
struct RKDWhiteLight : GrayModuleLightWidget {
	RKDWhiteLight() {
		addBaseColor(nvgRGB(0xff, 0xff, 0xff));
	}
};

// White LED color for RKD & RKD-BRK modules(used for output 2).
struct RKDOrangeLight : GrayModuleLightWidget {
	RKDOrangeLight() {
		addBaseColor(nvgRGB(0xf2, 0xb1, 0x20));
	}
};

// White LED color for RKD & RKD-BRK modules (used for output 7).
struct RKDPurpleLight : GrayModuleLightWidget {
	RKDPurpleLight() {
		addBaseColor(nvgRGB(0xd5, 0x2b, 0xed));
	}
};

// Tri-colored red/orange/blue LED for RKD & RKD-BRK modules (used by "RESET").
struct RedOrangeBlueLight : GrayModuleLightWidget {
	RedOrangeBlueLight() {
		addBaseColor(nvgRGB(0xed, 0x2c, 0x24));
		addBaseColor(nvgRGB(0xe8, 0xad, 0x10)); // Orange (same used by KlokSpid module).
		addBaseColor(nvgRGB(0x29, 0xb2, 0xef));
	}
};
