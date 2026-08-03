#include "rack.hpp"
#include "OhmerWidgets.hpp"

using namespace rack;

extern Plugin *pluginInstance;

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
extern Model *modelKlokSpid; // (DEPRECATED MODULE)

//// COLOR TABLE USED FOR DOT-MATRIX DISPLAY (REGARDLING SELECTED MODEL) - METRIKS MODULES.

static const NVGcolor tblDMDtextColor[6] = {
	nvgRGB(0x08, 0x08, 0x08), // LCD-like for Creamy.
	nvgRGB(0x08, 0x08, 0x08), // LCD-like for Stage Repro.
	nvgRGB(0x08, 0x08, 0x08), // LCD-like for Absolute Night.
	nvgRGB(0xe0, 0xe0, 0xff), // Blue plasma-like for Dark "Signature".
	nvgRGB(0xff, 0x8a, 0x00), // Orange plasma-like for Deepblue "Signature".
	nvgRGB(0xb0, 0xff, 0xff) // Light cyan plasma-like for Titanium "Signature".
};

//// BACKGROUND COLOR TABLES USED FOR BLANK PANELS.

static const NVGcolor tblpanelBgColor[6] = {
	nvgRGB(0xd2, 0xd2, 0xcd), // Creamy blank panel.
	nvgRGB(0x70, 0x00, 0x00), // Stage Repro blank panel.
	nvgRGB(0x00, 0x00, 0x00), // Absolute Night blank panel.
	nvgRGB(0x0a, 0x0a, 0x1e), // Dark "Signature" blank panel.
	nvgRGB(0x1a, 0x1a, 0x57), // Deepblue "Signature" blank panels
	nvgRGB(0x30, 0x30, 0x30) // Titanium "Signature" blank panel.
};

//// CUSTOM COMPONENTS (SCREWS, JACKS, KNOBS, ENCODERS, BUTTONS, LEDS).

// Custom silver Torx screw (used for non-"Signature" line modules).
struct Torx_Silver : SvgScrew {
	Torx_Silver() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Torx_Silver.svg")));
	}
};

// Custom gold Torx screw (used for "Signature" line modules).
struct Torx_Gold : SvgScrew {
	Torx_Gold() {
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/Torx_Gold.svg")));
	}
};

// Silver momentary button (used for non-"Signature" line modules).
struct Ohmer_ButtonSilver : SvgSwitch {
	Ohmer_ButtonSilver() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/XB_Button_Up_Silver.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/XB_Button_Down_Silver.svg")));
	}

	void onEnter(const EnterEvent& e) override {
		destroyTooltip(); // Destroy possible default tooltip.
	}

	void onButton(const ButtonEvent& e) override {
		if (e.action == GLFW_PRESS) {
			if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
				// Avoid to display value/ID when right-mouse click on it.
				e.consume(this); // Avoid context menu over button.
			}
			else SvgSwitch::onButton(e);
		}
	}

};

// Golden momentary button (used for "Signature" line modules).
struct Ohmer_ButtonGold : SvgSwitch {
	Ohmer_ButtonGold() {
		momentary = true;
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/XB_Button_Up_Gold.svg")));
		addFrame(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/XB_Button_Down_Gold.svg")));
	}

	void onEnter(const EnterEvent& e) override {
		destroyTooltip(); // Destroy possible default tooltip.
	}

	void onButton(const ButtonEvent& e) override {
		if (e.action == GLFW_PRESS) {
			if (e.button == GLFW_MOUSE_BUTTON_RIGHT) {
			// Avoid to display value/ID when right-mouse click on it.
				e.consume(this); // Avoid context menu over button.
			}
			else SvgSwitch::onButton(e);
		}
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

// Metal-based dynamic port (thanks to Marc Boulé for C++ code, and Xavier Belmont for SVGs).
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

// Freeware "Moog-style" continuous encoder.
struct Ohmer_Encoder : SvgKnob {
	Ohmer_Encoder() {
		minAngle = -1.0 * M_PI;
		maxAngle = M_PI;
		setSvg(APP->window->loadSvg(asset::plugin(pluginInstance, "res/components/KS_Encoder.svg")));
	}

	void onEnter(const EnterEvent& e) override {
		destroyTooltip(); // Destroy possible default tooltip.
	}

	void onButton(const ButtonEvent& e) override {
		if ((e.action == GLFW_PRESS) && (e.button == GLFW_MOUSE_BUTTON_RIGHT)) {
			// Avoid to display value/ID when right-mouse click on it.
			e.consume(this); // Avoid contextual menu over continuous encoder.
		}
		else SvgKnob::onButton(e);
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

// Custom white LED color for RKD module (used for CLK input jack, and 8+R output jack).
template <typename TBase = GrayModuleLightWidget>
struct TRKDWhiteLight : TBase {
	TRKDWhiteLight() {
	  this->addBaseColor(nvgRGB(0xff, 0xff, 0xff));
	}
};
using RKDWhiteLight = TRKDWhiteLight<>;

// Custom red LED color for RKD module (used for 1+R output jack).
template <typename TBase = GrayModuleLightWidget>
struct TRKDRedLight : TBase {
	TRKDRedLight() {
	  this->addBaseColor(nvgRGB(0xff, 0x00, 0x00));
	}
};
using RKDRedLight = TRKDRedLight<>;

// Custom orange LED color for RKD module (used for 2+R output jack).
template <typename TBase = GrayModuleLightWidget>
struct TRKDOrangeLight : TBase {
	TRKDOrangeLight() {
	  this->addBaseColor(nvgRGB(0xf2, 0xb1, 0x20));
	}
};
using RKDOrangeLight = TRKDOrangeLight<>;

// Custom yellow LED color for RKD module (used for 3+R output jack).
template <typename TBase = GrayModuleLightWidget>
struct TRKDYellowLight : TBase {
	TRKDYellowLight() {
	  this->addBaseColor(nvgRGB(0xff, 0xff, 0x00));
	}
};
using RKDYellowLight = TRKDYellowLight<>;

// Custom lime LED color for RKD module (used for 4+R output jack).
template <typename TBase = GrayModuleLightWidget>
struct TRKDLimeLight : TBase {
	TRKDLimeLight() {
	  this->addBaseColor(nvgRGB(0x00, 0xff, 0x00));
	}
};
using RKDLimeLight = TRKDLimeLight<>;

// Custom green LED color for RKD module (used for 5+R output jack).
template <typename TBase = GrayModuleLightWidget>
struct TRKDGreenLight : TBase {
	TRKDGreenLight() {
	  this->addBaseColor(nvgRGB(0x20, 0xc0, 0x20));
	}
};
using RKDGreenLight = TRKDGreenLight<>;

// Custom blue LED color for RKD module (used for 6+R output jack).
template <typename TBase = GrayModuleLightWidget>
struct TRKDBlueLight : TBase {
	TRKDBlueLight() {
	  this->addBaseColor(nvgRGB(0x00, 0x80, 0xff));
	}
};
using RKDBlueLight = TRKDBlueLight<>;

// Custom purple LED color for RKD module (used for 7+R output jack).
template <typename TBase = GrayModuleLightWidget>
struct TRKDPurpleLight : TBase {
	TRKDPurpleLight() {
	  this->addBaseColor(nvgRGB(0xd5, 0x2b, 0xed));
	}
};
using RKDPurpleLight = TRKDPurpleLight<>;

// Custom tri-colored RESET LED.
template <typename TBase = GrayModuleLightWidget>
struct TRKDResetLight : TBase {
	TRKDResetLight() {
	  this->addBaseColor(nvgRGB(0xed, 0x2c, 0x24)); // Red component.
	  this->addBaseColor(nvgRGB(0xe8, 0xad, 0x10)); // Orange component.
	  this->addBaseColor(nvgRGB(0xa0, 0x2b, 0xff)); // Purple component.
	}
};
using RKDResetLight = TRKDResetLight<>;
