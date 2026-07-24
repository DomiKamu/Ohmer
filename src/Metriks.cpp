////////////////////////////////////////////////////////////////////////
//// Metriks is a 8 HP measuring/visual module:                    /////
//// - Voltmeter.                                                  /////
//// - CV Pitch.                                                   /////
//// - BPM Meter.                                                  /////
//// - Peak Counter (aka pulse counter).                           /////
////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"
#include <dsp/digital.hpp>
#include <string>

// Module structure.
struct MetriksModule : Module {
	enum ParamIds {
		PARAM_ENCODER,
		BUTTON_OPTIONS,
		BUTTON_PLAYPAUSE,
		BUTTON_RESET,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_SOURCE,
		INPUT_PLAYPAUSE,
		INPUT_RESET,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_THRU,
		NUM_OUTPUTS
	};
	enum LightIds {
		LED_PLAY_GREEN,
		LED_PLAY_RED,
		LED_OPTIONS,
		NUM_LIGHTS
	};

	// SAMPLE RATE / SAMPLE TIME.
	float sampleRate = 48000.f; // Would be redefined by onSampleRateChange() event. Defined as default @ 48kHz.

	bool b_dspProcessing = false; // Will be set true as soon as DSP is processing.

	// Current selected Metriks model (GUI theme variation).
	int Theme = 0; // 0 = Creamy, 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Titanium Signature.
	int portMetal = 0; // Used to select silver or golden jacks.

	// Mode (0: voltmeter, 1: CV Pitch, 2: BPM meter, 3: peak counter).
	bool b_ChangingMode = false; // true during mode transition, false otherwise.
	int Mode = 0; // Current mode.
	int _Mode = 0; // Its old/previous state (required for Preset management).
	int ct_SwitchedMode = 0; // Counter used as "timer" when switching to next/previous mode.

	// Counter used when input isn't connected to blink "? INPUT ?" message on DMD (second line).
	int ct_NoInputTimer = 0;

	// Option to change (depending mode).
	bool b_ChangingOption = false;
	int currentOptionID = 0; // Option identifier (0 is first).
	int ct_OptionTimeout = 0; // Counter used as "timer" when editing any option.
	int ct_OptionBlinkTimer = 0; // Used to relevant LED blink.

	// Tables (arrays) used for options/parameters.
	enum en_Modes {
		METRIKS_VOLTMETER,
		METRIKS_CVPITCH,
		METRIKS_BPMMETER,
		METRIKS_PEAKCOUNTER,
		METRIKS_NUM_MODES
	};

	const int tb_OptionNumPerMode[METRIKS_NUM_MODES] = {2, 2, 0, 1}; // For each mode, number of possible option(s). BPM meter doesn't have option.
	std::string tb_OptionID[METRIKS_NUM_MODES][4]; // Will be initialized later (from module constructor).
	int tb_ParamNumPerOpt[METRIKS_NUM_MODES][4]; // Will be initialized later (from module constructor).
	std::string tb_OptParameter[METRIKS_NUM_MODES][4][5]; // Will be initialized later (from module constructor).
	float tb_OptParameterXPos[METRIKS_NUM_MODES][4][5]; // Message positions (on line 2 of DMD). Will be initialized later (from module constructor).
	int currentParameter[METRIKS_NUM_MODES][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {1, 0, 0, 0}}; // Must be initialized here, to avoid potential crash on instanciate!
	int _currentParameter[METRIKS_NUM_MODES][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {1, 0, 0, 0}}; // Must be initialized here, to avoid potential crash on instanciate!

	// Frequencies tables used by CV Pitch feature. Will are initialized later (from module constructor).
	double tb_FreqNote_Center[132] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0};

	double tb_FreqNote_LP_LimL[132] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0};

	double tb_FreqNote_LP_LimH[132] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0};

	double tb_FreqNote_MP_LimL[132] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0};

	double tb_FreqNote_MP_LimH[132] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0};

	double tb_FreqNote_HP_LimL[132] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0};

	double tb_FreqNote_HP_LimH[132] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
																		0.0, 0.0, 0.0, 0.0};

	// CV Pitch variables.
	std::string pitchBaseNoteName[12] = {"", "", "", "", "", "", "", "", "", "", "", ""}; // Base note names, for now empty, filled later...
	std::string cvpitchNote[132];
	float dmdCVPitchMarkerPos = 0.f;
	char dmdCVPitchMarker[3] = ""; // CV Pitch only, to display the below/above marker(s).
	bool b_cvpitchMarkerVisible = false;

	// Messages displayed on DMD (dot-matrix display), using two lines..
	char dmdTextMain1[20] = ""; // 20 chars for upper (1st) line.
	char dmdTextMain2[20] = ""; // 20 chars for lower (2nd) line.
	float dmdOffsetTextMain2 = 0.f; // Horizontal offset on DMD to display for lower (2nd) line.

	// Encoder (registered position to be used on next step for relative move).
	int encoderParam = 0; // Encoder parameter.
	int _encoderParam = 0; // Previous encoder parameter.

	// OPT. (options) button.
	dsp::SchmittTrigger optButton;

	// IN (INput) jack.
	bool b_ActiveINjack = false;
	bool _b_ActiveINjack = false; // Old/previous IN jack state.
	float f_InVoltage = 0.f;
	float _f_InVoltage = -1.f; // Old/previous voltage on IN jack.
	// Used for mix, max, and median.
	float f_VoltageMin = 99999.f;
	float f_VoltageMax = -99999.f;
	float f_VoltageMed = 0.f;

	// PLAY/STOP button and related (trigger) port (PLAY/STOP is used for "Pulse Counter" mode only).
	dsp::SchmittTrigger playButton;
	dsp::SchmittTrigger playPort;
	bool b_PeakCounterIsPlaying = false;

	// RESET (RST) button and related (trigger) port.
	dsp::SchmittTrigger resetButton;
	dsp::SchmittTrigger resetPort;

	// Number of decimals can be displayed by voltmeter.
	int vltmDecimals = 4; // Number of decimals used by Voltmeter (default is 4 = "Auto").
	int _vltmDecimals = -1; // Old/previous number of decimals (to detect value change).

	// Schmitt trigger used to determine frequency. Also used for peak counter mode.
	dsp::SchmittTrigger inputPort;

	// Used by Peak Counter mode, for peak detection at treshold voltage.
	int pcntTresholdVoltage = 1;
	float f_pcntTresholdVoltage = .1f; // Low threshold is 0.1V.

	// Dummy string (used for std::string to char * conversions).
	std::string _tmpString; // Dummy string.

	// TEMPORARY - used for inoperative mode(s) - MUST BE REMOVED WHEN ALL MODES WORK.
	bool b_InopMode = false;
	int ct_InopModeTimer = 0;
	int i_InopMsgCycling = 0;

	MetriksModule() {
		// Module constructor.
		Theme = rack::settings::preferDarkPanels ? 2 : 0; // Assuming default is "Creamy" or "Absolute Night" (depending "Use dark panels if available" option, from "View" menu).
		portMetal = (Theme > 2) ? 1 : 0;
		b_dspProcessing = false; // Will be set true as soon as DSP is running.
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PARAM_ENCODER, -INFINITY, INFINITY, 0.f, "");
		configButton(BUTTON_OPTIONS, "OPTIONS");
		configButton(BUTTON_PLAYPAUSE, "PLAY/PAUSE");
		configButton(BUTTON_RESET, "RESET");
		configInput(INPUT_SOURCE, "Signal to meter");
		configInput(INPUT_PLAYPAUSE, "Play/Pause");
		configInput(INPUT_RESET, "Reset");
		configOutput(OUTPUT_THRU, "INput thru");
		configBypass(INPUT_SOURCE, OUTPUT_THRU);
		b_InopMode = false; // TEMPORARY - false means the mode is operational (totally or partially) - MUST BE REMOVED WHEN ALL MODES WORK.
		b_ChangingMode = false;
		ct_SwitchedMode = 0;
		b_ChangingOption = false;
		currentOptionID = -1; // Option ID (-1 while not option edit).
		ct_OptionTimeout = 0; // Used as timer for current option (as time out).
		ct_OptionBlinkTimer = 0;
		b_PeakCounterIsPlaying = false;
		for (int i = 0; i < METRIKS_NUM_MODES; i++)
			for (int j = 0; j < 4; j++)
				for (int k = 0; k < 4; k++)
					tb_OptParameterXPos[i][j][k] = 0.f; // Horizontal positions (display on line 2 of DMD). Default 0.f, used will are set just below.
		// Tables used by Voltmeter mode.
		tb_OptionID[METRIKS_VOLTMETER][0] = "Metering";
		tb_ParamNumPerOpt[METRIKS_VOLTMETER][0] = 4;
		tb_OptParameter[METRIKS_VOLTMETER][0][0] = "Realtime";
		tb_OptParameterXPos[METRIKS_VOLTMETER][0][0] = 9.17f;
		tb_OptParameter[METRIKS_VOLTMETER][0][1] = "Minimum";
		tb_OptParameterXPos[METRIKS_VOLTMETER][0][1] = 12.94f;
		tb_OptParameter[METRIKS_VOLTMETER][0][2] = "Maximum";
		tb_OptParameterXPos[METRIKS_VOLTMETER][0][2] = 12.f;
		tb_OptParameter[METRIKS_VOLTMETER][0][3] = "Median";
		tb_OptParameterXPos[METRIKS_VOLTMETER][0][3] = 18.6f;
		tb_OptionID[METRIKS_VOLTMETER][1] = "Decimals";
		tb_ParamNumPerOpt[METRIKS_VOLTMETER][1] = 5;
		tb_OptParameter[METRIKS_VOLTMETER][1][0] = "Auto";
		tb_OptParameterXPos[METRIKS_VOLTMETER][1][0] = 29.5f;
		tb_OptParameter[METRIKS_VOLTMETER][1][1] = "2";
		tb_OptParameterXPos[METRIKS_VOLTMETER][1][1] = 41.19f;
		tb_OptParameter[METRIKS_VOLTMETER][1][2] = "3";
		tb_OptParameterXPos[METRIKS_VOLTMETER][1][2] = 41.19f;
		tb_OptParameter[METRIKS_VOLTMETER][1][3] = "0";
		tb_OptParameterXPos[METRIKS_VOLTMETER][1][3] = 41.19f;
		tb_OptParameter[METRIKS_VOLTMETER][1][4] = "1";
		tb_OptParameterXPos[METRIKS_VOLTMETER][1][4] = 41.19f;
		tb_OptionID[METRIKS_VOLTMETER][2] = ""; // Not used.
		tb_ParamNumPerOpt[METRIKS_VOLTMETER][2] = 0;
		tb_OptParameter[METRIKS_VOLTMETER][2][0] = ""; // Not used.
		tb_OptParameter[METRIKS_VOLTMETER][2][1] = ""; // Not used.
		tb_OptParameter[METRIKS_VOLTMETER][2][2] = ""; // Not used.
		tb_OptParameter[METRIKS_VOLTMETER][2][3] = ""; // Not used.
		tb_OptionID[METRIKS_VOLTMETER][3] = ""; // Not used.
		tb_ParamNumPerOpt[METRIKS_VOLTMETER][3] = 0;
		tb_OptParameter[METRIKS_VOLTMETER][3][0] = ""; // Not used.
		tb_OptParameter[METRIKS_VOLTMETER][3][1] = ""; // Not used.
		tb_OptParameter[METRIKS_VOLTMETER][3][2] = ""; // Not used.
		tb_OptParameter[METRIKS_VOLTMETER][3][3] = ""; // Not used.
		// Tables used by CV Pitch mode.
		tb_OptionID[METRIKS_CVPITCH][0] = "Notation";
		tb_ParamNumPerOpt[METRIKS_CVPITCH][0] = 2;
		tb_OptParameter[METRIKS_CVPITCH][0][0] = "C-D-E...B";
		tb_OptParameterXPos[METRIKS_CVPITCH][0][0] = 4.466f;
		tb_OptParameter[METRIKS_CVPITCH][0][1] = "Do-Re-Mi";
		tb_OptParameterXPos[METRIKS_CVPITCH][0][1] = 5.408f;
		tb_OptParameter[METRIKS_CVPITCH][0][2] = ""; // Not used.
		tb_OptParameter[METRIKS_CVPITCH][0][3] = ""; // Not used.
		tb_OptionID[METRIKS_CVPITCH][1] = "Sharps/Flats";
		tb_ParamNumPerOpt[METRIKS_CVPITCH][1] = 2;
		tb_OptParameter[METRIKS_CVPITCH][1][0] = "Sharps #";
		tb_OptParameterXPos[METRIKS_CVPITCH][1][0] = 6.35f;
		tb_OptParameter[METRIKS_CVPITCH][1][1] = "Flats b";
		tb_OptParameterXPos[METRIKS_CVPITCH][1][1] = 12.94f;
		tb_OptParameter[METRIKS_CVPITCH][1][2] = ""; // Not used.
		tb_OptParameter[METRIKS_CVPITCH][1][3] = ""; // Not used.
		tb_OptionID[METRIKS_CVPITCH][2] = ""; // Not used.
		tb_ParamNumPerOpt[METRIKS_CVPITCH][2] = 1; // Not used.
		tb_OptParameter[METRIKS_CVPITCH][2][0] = ""; // Not used.
		tb_OptParameter[METRIKS_CVPITCH][2][1] = ""; // Not used.
		tb_OptParameter[METRIKS_CVPITCH][2][2] = ""; // Not used.
		tb_OptParameter[METRIKS_CVPITCH][2][3] = ""; // Not used.
		tb_OptionID[METRIKS_CVPITCH][3] = ""; // Not used.
		tb_ParamNumPerOpt[METRIKS_CVPITCH][3] = 0;
		tb_OptParameter[METRIKS_CVPITCH][3][0] = ""; // Not used.
		tb_OptParameter[METRIKS_CVPITCH][3][1] = ""; // Not used.
		tb_OptParameter[METRIKS_CVPITCH][3][2] = ""; // Not used.
		tb_OptParameter[METRIKS_CVPITCH][3][3] = ""; // Not used.
		// Tables used by BPM Meter mode (for now, BPM Meter mode doesn't have options).
		tb_OptionID[METRIKS_BPMMETER][0] = ""; // Not used.
		tb_ParamNumPerOpt[METRIKS_BPMMETER][0] = 0;
		tb_OptParameter[METRIKS_BPMMETER][0][0] = ""; // Not used.
		tb_OptParameter[METRIKS_BPMMETER][0][1] = ""; // Not used.
		tb_OptParameter[METRIKS_BPMMETER][0][2] = ""; // Not used.
		tb_OptParameter[METRIKS_BPMMETER][0][3] = ""; // Not used.
		tb_OptionID[METRIKS_BPMMETER][1] = ""; // Not used.
		tb_ParamNumPerOpt[METRIKS_BPMMETER][1] = 0;
		tb_OptParameter[METRIKS_BPMMETER][1][0] = ""; // Not used.
		tb_OptParameter[METRIKS_BPMMETER][1][1] = ""; // Not used.
		tb_OptParameter[METRIKS_BPMMETER][1][2] = ""; // Not used.
		tb_OptParameter[METRIKS_BPMMETER][1][3] = ""; // Not used.
		tb_OptionID[METRIKS_BPMMETER][2] = ""; // Not used.
		tb_ParamNumPerOpt[METRIKS_BPMMETER][2] = 0;
		tb_OptParameter[METRIKS_BPMMETER][2][0] = ""; // Not used.
		tb_OptParameter[METRIKS_BPMMETER][2][1] = ""; // Not used.
		tb_OptParameter[METRIKS_BPMMETER][2][2] = ""; // Not used.
		tb_OptParameter[METRIKS_BPMMETER][2][3] = ""; // Not used.
		tb_OptionID[METRIKS_BPMMETER][3] = ""; // Not used.
		tb_ParamNumPerOpt[METRIKS_BPMMETER][3] = 0;
		tb_OptParameter[METRIKS_BPMMETER][3][0] = ""; // Not used.
		tb_OptParameter[METRIKS_BPMMETER][3][1] = ""; // Not used.
		tb_OptParameter[METRIKS_BPMMETER][3][2] = ""; // Not used.
		tb_OptParameter[METRIKS_BPMMETER][3][3] = ""; // Not used.
		// Tables used by Peak Counter mode.
		tb_OptionID[METRIKS_PEAKCOUNTER][0] = "Threshold";
		tb_ParamNumPerOpt[METRIKS_PEAKCOUNTER][0] = 1; // NOTE: threshold voltage is set directly via continuous encoder (by +/- 0.1V steps).
		tb_OptParameter[METRIKS_PEAKCOUNTER][0][0] = ""; // Not used.
		tb_OptParameter[METRIKS_PEAKCOUNTER][0][1] = ""; // Not used.
		tb_OptParameter[METRIKS_PEAKCOUNTER][0][2] = ""; // Not used.
		tb_OptParameter[METRIKS_PEAKCOUNTER][0][3] = ""; // Not used.
		tb_OptionID[METRIKS_PEAKCOUNTER][1] = ""; // Not used.
		tb_ParamNumPerOpt[METRIKS_PEAKCOUNTER][1] = 0;
		tb_OptParameter[METRIKS_PEAKCOUNTER][1][0] = ""; // Not used.
		tb_OptParameter[METRIKS_PEAKCOUNTER][1][1] = ""; // Not used.
		tb_OptParameter[METRIKS_PEAKCOUNTER][1][2] = ""; // Not used.
		tb_OptParameter[METRIKS_PEAKCOUNTER][1][3] = ""; // Not used.
		tb_OptionID[METRIKS_PEAKCOUNTER][2] = ""; // Not used.
		tb_ParamNumPerOpt[METRIKS_PEAKCOUNTER][2] = 0;
		tb_OptParameter[METRIKS_PEAKCOUNTER][2][0] = ""; // Not used.
		tb_OptParameter[METRIKS_PEAKCOUNTER][2][1] = ""; // Not used.
		tb_OptParameter[METRIKS_PEAKCOUNTER][2][2] = ""; // Not used.
		tb_OptParameter[METRIKS_PEAKCOUNTER][2][3] = ""; // Not used.
		tb_OptionID[METRIKS_PEAKCOUNTER][3] = ""; // Not used.
		tb_ParamNumPerOpt[METRIKS_PEAKCOUNTER][3] = 0;
		tb_OptParameter[METRIKS_PEAKCOUNTER][3][0] = ""; // Not used.
		tb_OptParameter[METRIKS_PEAKCOUNTER][3][1] = ""; // Not used.
		tb_OptParameter[METRIKS_PEAKCOUNTER][3][2] = ""; // Not used.
		tb_OptParameter[METRIKS_PEAKCOUNTER][3][3] = ""; // Not used.
		// Get engine sample rate.
		sampleRate = APP->engine->getSampleRate();
		// Set up frequencies tables for CV Pitch mode (precomputed frequencies tables, used by CV Pitch mode).
		for (int i = 0; i < 132; i++) {
			// Central frequencies (frequency of each note).
			tb_FreqNote_Center[i] = 440.0 * (double)(pow(2, ((i - 69.0) / 12)));
			// Low precision ranges tables, also used for initial note detection.
			tb_FreqNote_LP_LimL[i] = 440.0 * (double)(pow(2, ((i - 69.5) / 12)));
			tb_FreqNote_LP_LimH[i] = 440.0 * (double)(pow(2, ((i - 68.5) / 12)));
			// Medium precision ranges tables.
			tb_FreqNote_MP_LimL[i] = 440.0 * (double)(pow(2, ((i - 69.2) / 12)));
			tb_FreqNote_MP_LimH[i] = 440.0 * (double)(pow(2, ((i - 68.8) / 12)));
			// High precision ranges tables.
			tb_FreqNote_HP_LimL[i] = 440.0 * (double)(pow(2, ((i - 69.02) / 12)));
			tb_FreqNote_HP_LimH[i] = 440.0 * (double)(pow(2, ((i - 68.98) / 12)));
		}
	}

	// Invoked (as event) from Initialize command via module's context menu (also Ctrl+I, Command+I on Macinthosh) to reset the module.
	void onReset(const ResetEvent& e) override {
		// Current parameters (and their old/previous states) reset to default values (0).
		for (int i = 0; i < METRIKS_NUM_MODES; i++)
			for (int j = 0; j < 4; j++) {
				if ((i == METRIKS_PEAKCOUNTER) && (j == 0)) {
					_currentParameter[METRIKS_PEAKCOUNTER][0] = 1; // Set treshold voltage to default 0.1V (for Peak Counter mode).
					currentParameter[METRIKS_PEAKCOUNTER][0] = 1; // Set treshold voltage to default 0.1V (for Peak Counter mode).
				}
				else {
					_currentParameter[i][j] = 0; // All options to default.
					currentParameter[i][j] = 0; // All options to default.
				}
				setMetriksParameters(i, j);
			}
		_Mode = Mode;
		b_InopMode = false; // TEMPORARY - false means the mode is operational (totally or partially) - MUST BE REMOVED WHEN ALL MODES WORK.
		b_ChangingMode = false;
		ct_SwitchedMode = 0;
		b_ChangingOption = false;
		currentOptionID = -1; // Option ID (-1 while not option edit).
		ct_OptionTimeout = 0; // Used as timer for current option (as time out).
		ct_OptionBlinkTimer = 0;
		lights[LED_OPTIONS].setBrightness(0.f);
		// Peak Counter isn't running.
		b_PeakCounterIsPlaying = false;
		// Reset minimum, maximum and median voltages (voltmeter mode).
		f_VoltageMin = f_InVoltage;
		f_VoltageMax = f_InVoltage;
		f_VoltageMed = f_InVoltage;
		// By doing this, the second line of DMD will be refreshed.
		_f_InVoltage += 1.f;
	}

	// Invoked (as event) when Engine's Sample rate is changed from VCV Rack menu.
	void onSampleRateChange(const SampleRateChangeEvent& e) override {
		sampleRate = APP->engine->getSampleRate();
	}

	// Custom function to round a float-type at given decimals. Using internally "double", for best precision!
	inline float roundp(double f, int prec) {
		return round(f * (double)(pow(10, prec))) / (double)(pow(10, prec));
	}

	void setMetriksParameters(int i_Mode, int i_Opt) {
		if (currentParameter[i_Mode][i_Opt] != _currentParameter[i_Mode][i_Opt]) {
			// Suddently (uncontroled) changed parameter, for example via Preset load or copy/paste accross Metriks modules.
			_Mode = Mode;
			b_InopMode = false; // TEMPORARY - false means the mode is operational (totally or partially) - MUST BE REMOVED WHEN ALL MODES WORK.
			b_ChangingMode = false;
			ct_SwitchedMode = 0;
			b_ChangingOption = false;
			currentOptionID = -1; // Option ID (-1 while not option edit).
			ct_OptionTimeout = 0; // Used as timer for current option (as time out).
			ct_OptionBlinkTimer = 0;
			lights[LED_OPTIONS].setBrightness(0.f);
			// Peak Counter isn't running.
			b_PeakCounterIsPlaying = false;
			// By doing this, the second line of DMD will be refreshed.
			_f_InVoltage += 1.f;
			// Mirror current parameter to its backup.
			_currentParameter[i_Mode][i_Opt] = currentParameter[i_Mode][i_Opt];
		}
		switch (i_Mode) {
			case METRIKS_VOLTMETER:
				// Voltmeter mode.
				if (i_Opt == 1) {
					// Decimals...
					switch (currentParameter[METRIKS_VOLTMETER][1]) {
						case 0:
							 // Auto (default).
							 vltmDecimals = 4;
							break;
						case 1:
							 // 2 decimals.
							 vltmDecimals = 2;
							break;
						case 2:
							 // 3 decimals.
							 vltmDecimals = 3;
							break;
						case 3:
							 // No decimal.
							 vltmDecimals = 0;
							break;
						case 4:
							 // 1 decimal.
							 vltmDecimals = 1;
					}
				}
				break;
			case METRIKS_CVPITCH:
				// CV Pitch mode.
				// Notation or sharps/flats: update notes tables.
				makeNotesTables();
				break;
			case METRIKS_BPMMETER:
				break;
			case METRIKS_PEAKCOUNTER:
				// Peak Counter mode.
				pcntTresholdVoltage = currentParameter[METRIKS_PEAKCOUNTER][0];
				if (pcntTresholdVoltage < 1)
					pcntTresholdVoltage = 1; // Set to minimum treshold voltage (0.1V), if below.
					else if (pcntTresholdVoltage > 100)
						pcntTresholdVoltage = 100; // Set to maximum allowed treshold voltage (10V), if above.
				f_pcntTresholdVoltage = (float)(pcntTresholdVoltage) / 10.f;
		}
	}

	// Custom method to make current notes table, depending "Notation" and "Sharps/Flats" parameters (CV Pitch mode).
	void makeNotesTables() {
		if (currentParameter[METRIKS_CVPITCH][0] == 0) {
			// English (international) notation (C, D, E,...B).
			tb_OptionID[METRIKS_CVPITCH][2] = "A4 Pitch";
			if (currentParameter[METRIKS_CVPITCH][1] == 0) {
				// Sharps.
				pitchBaseNoteName[0] = "C";
				pitchBaseNoteName[1] = "C#";
				pitchBaseNoteName[2] = "D";
				pitchBaseNoteName[3] = "D#";
				pitchBaseNoteName[4] = "E";
				pitchBaseNoteName[5] = "F";
				pitchBaseNoteName[6] = "F#";
				pitchBaseNoteName[7] = "G";
				pitchBaseNoteName[8] = "G#";
				pitchBaseNoteName[9] = "A";
				pitchBaseNoteName[10] = "A#";
				pitchBaseNoteName[11] = "B";
			}
			else {
				// Flats.
				pitchBaseNoteName[0] = "C";
				pitchBaseNoteName[1] = "Db";
				pitchBaseNoteName[2] = "D";
				pitchBaseNoteName[3] = "Eb";
				pitchBaseNoteName[4] = "E";
				pitchBaseNoteName[5] = "F";
				pitchBaseNoteName[6] = "Gb";
				pitchBaseNoteName[7] = "G";
				pitchBaseNoteName[8] = "Ab";
				pitchBaseNoteName[9] = "A";
				pitchBaseNoteName[10] = "Bb";
				pitchBaseNoteName[11] = "B";
			}
		}
		else {
			// Do-Re-Mi (French/Italian) notation.
			tb_OptionID[METRIKS_CVPITCH][2] = "La4 Pitch";
			if (currentParameter[METRIKS_CVPITCH][1] == 0) {
				// Sharps.
				pitchBaseNoteName[0] = "Do";
				pitchBaseNoteName[1] = "Do#";
				pitchBaseNoteName[2] = "Re";
				pitchBaseNoteName[3] = "Re#";
				pitchBaseNoteName[4] = "Mi";
				pitchBaseNoteName[5] = "Fa";
				pitchBaseNoteName[6] = "Fa#";
				pitchBaseNoteName[7] = "Sol";
				pitchBaseNoteName[8] = "Sol#";
				pitchBaseNoteName[9] = "La";
				pitchBaseNoteName[10] = "La#";
				pitchBaseNoteName[11] = "Si";
			}
			else {
				// Flats.
				pitchBaseNoteName[0] = "Do";
				pitchBaseNoteName[1] = "Reb";
				pitchBaseNoteName[2] = "Re";
				pitchBaseNoteName[3] = "Mib";
				pitchBaseNoteName[4] = "Mi";
				pitchBaseNoteName[5] = "Fa";
				pitchBaseNoteName[6] = "Solb";
				pitchBaseNoteName[7] = "Sol";
				pitchBaseNoteName[8] = "Lab";
				pitchBaseNoteName[9] = "La";
				pitchBaseNoteName[10] = "Sib";
				pitchBaseNoteName[11] = "Si";
			}
		}
		// Final table construction, including octave.
		for (int i = 0; i < 132; i++)
			cvpitchNote[i] = pitchBaseNoteName[i % 12] + std::to_string((i / 12) - 1);
	}

	// Custom method to prepare threshold voltage for display (2nd line).
	void setDisplayThresholdVoltage() {
		f_pcntTresholdVoltage = (float)(pcntTresholdVoltage / 10.f);
		if (pcntTresholdVoltage < 100)
			dmdOffsetTextMain2 = 36.f;
			else dmdOffsetTextMain2 = 24.f;
		snprintf(dmdTextMain2, sizeof(dmdTextMain2), "%2.1fV", f_pcntTresholdVoltage);
	}

	// This function will search accurate note, from given frequency, from precomputed tables (best for CPU save, tables are computed on Ref. A4/La4 tuning change only).
	int getNotebyFreq(double freq) {
		int aPos = -1;
		int startPos = 0;
		bool b_IsAbove = false;
		if (freq > 369.f)
			startPos = 66; // Start the table scans from... middle (F#4 / Gb4).
		if ((freq >= tb_FreqNote_LP_LimL[0]) && (freq < tb_FreqNote_LP_LimH[131])) {
			// Doing search only if input frequency is in allowed limits (C-1 to B9).
			for (int i = startPos; i < 132; i++) {
				// Finding note (from LP - low-precision - tables).
				if ((freq >= tb_FreqNote_LP_LimL[i]) && (freq < tb_FreqNote_LP_LimH[i])) {
					// Note is found.
					aPos = i;
					b_IsAbove = (freq >= tb_FreqNote_Center[aPos]); // true if the frequency is above "center" frequency/note...
					if ((freq >= tb_FreqNote_HP_LimL[aPos]) && (freq < tb_FreqNote_HP_LimH[aPos])) {
 						// High precision: don't display left/right marker(s).
						b_cvpitchMarkerVisible = false;
						dmdCVPitchMarkerPos = 0.f;
						strcpy(dmdCVPitchMarker, " ");
					}
					else if ((freq >= tb_FreqNote_MP_LimL[aPos]) && (freq < tb_FreqNote_MP_LimH[aPos])) {
 						// Medium precision: display one marker only, either "<" or ">".
						b_cvpitchMarkerVisible = true;
						if (b_IsAbove) {
							dmdCVPitchMarkerPos = 2.6f;
							strcpy(dmdCVPitchMarker, "<");
						}
						else {
							dmdCVPitchMarkerPos = 90.f;
							strcpy(dmdCVPitchMarker, ">");
						}
					}
					else {
 						// Near bounds (aka bad precision): display three markers, either "<<<" or ">>>", because frequency is near bound.
						b_cvpitchMarkerVisible = true;
						if (b_IsAbove) {
							dmdCVPitchMarkerPos = 2.6f;
							strcpy(dmdCVPitchMarker, "<<");
						}
						else {
							dmdCVPitchMarkerPos = 84.f;
							strcpy(dmdCVPitchMarker, ">>");
						}
					}
					i = 132; // Exit note scan loop.
				}
			}
		}
		else if (freq < tb_FreqNote_LP_LimL[0]) {
			// Input frequency is too low (below C-1).
			b_cvpitchMarkerVisible = true;
			dmdCVPitchMarkerPos = 84.f;
			strcpy(dmdCVPitchMarker, ">>");
		}
		else {
			// Input frequency is too high (above B9).
			b_cvpitchMarkerVisible = true;
			dmdCVPitchMarkerPos = 2.6f;
			strcpy(dmdCVPitchMarker, "<<");
		}
		return aPos;
	}

	// This function returns, for a given string, the number of px (for second line on the DMD).
	// Useful for centered display (line 2 only!).
	float getCenteredDMD(std::string sStr) {
		std::string pxChar;
		std::string searchInto;
		int px;
		bool b_found;
		px = 0;
		for (unsigned int i = 0; i < sStr.length(); i++) {
			pxChar = sStr.at(i);
			b_found = false;
			// 5 px chars...
			searchInto = "abcdefghknopqrstuxyz";
			if (searchInto.find(pxChar) != std::string::npos) {
				b_found = true;
				px = px + 5;
			}
			// 4 px chars...
			searchInto = "ijl<>[]\"";
			if (!b_found)
				if (searchInto.find(pxChar) != std::string::npos) {
					b_found = true;
					px = px + 4;
				}
			// 3 px chars...
			searchInto = "():.,";
			if (!b_found)
				if (searchInto.find(pxChar) != std::string::npos) {
					b_found = true;
					px = px + 3;
				}
			// 2 px chars...
			searchInto = "'!|";
			if (!b_found)
				if (searchInto.find(pxChar) != std::string::npos) {
					b_found = true;
					px = px + 2;
				}
			// Other else are 6 px...
			if (!b_found)
				px = px + 6;
		}
		px--;
		return ((105.8f - (11.3f * (float)(px) / 6.f)) / 2.f) - 7.f;
	}

	// TEMPORARY - used for inoperative mode(s) - MUST BE REMOVED WHEN ALL MODES WORK.
	void setInopMode() {
		if (b_InopMode)
			return;
		b_InopMode = true;
		ct_InopModeTimer = (int)(1.5f * sampleRate); // This timer is used to cycle inop. messages every 1.5 second.
		i_InopMsgCycling = 0; // Index to displayed message.
	}

	// TEMPORARY - used for inoperative mode(s) - MUST BE REMOVED WHEN ALL MODES WORK.
	void runInopMode() {
		if (ct_InopModeTimer > 0)
			ct_InopModeTimer--;
			else {
				ct_InopModeTimer = (int)(1.5f * sampleRate); // This timer is used to cycle inop. messages every 1.5 second.
				i_InopMsgCycling++; // Point to next message to display (or restart if necessary).
				if (i_InopMsgCycling > 5)
					i_InopMsgCycling = 0;
			}
		// Display inoperative message...
		dmdOffsetTextMain2 = 1.f;
		switch (i_InopMsgCycling) {
			case 0:
				strcpy(dmdTextMain2, "This mode");
				break;
			case 1:
				strcpy(dmdTextMain2, "can't be");
				break;
			case 2:
				strcpy(dmdTextMain2, "used yet.");
				break;
			case 3:
				strcpy(dmdTextMain2, "Still in");
				break;
			case 4:
				strcpy(dmdTextMain2, "developm!");
				break;
			case 5:
				strcpy(dmdTextMain2, "");
		}
	}

	// Module's DSP.
	void process(const ProcessArgs &args) override {
		// DSP processing...

		if (!b_dspProcessing) {
			for (int i = 0; i < METRIKS_NUM_MODES; i++)
				for (int j = 0; j < 4; j++) {
					// Set up relevant mode/option.
					setMetriksParameters(i, j);
					_currentParameter[i][j] = currentParameter[i][j];
				}
			// Reset all momentary buttons.
			optButton.reset();
			playButton.reset();
			resetButton.reset();
			// By doing this, the second line of DMD will be refreshed.
			_f_InVoltage += 1.f;
			b_dspProcessing = true; // Yes, DSP is running...
		}

		// TEMPORARY - used for inoperative mode(s) - MUST BE REMOVED WHEN ALL MODES WORK.
		if (b_InopMode)
			if (!b_ChangingMode && !b_ChangingOption)
				runInopMode();

		// Is IN (INput) jack connected?
		b_ActiveINjack = inputs[INPUT_SOURCE].isConnected();

		// Transmit (as passthrough/daisy chain) current voltage on IN jack, to OUT jack. 0V will be sent as long as IN jack remains disconnected!
		if (b_ActiveINjack)
			outputs[OUTPUT_THRU].setVoltage(inputs[INPUT_SOURCE].getVoltage());
			else outputs[OUTPUT_THRU].setVoltage(0.f);

		// Read if the continuous encoder is moved...
		encoderParam = (int)roundf(10.f * params[PARAM_ENCODER].getValue());
		if (encoderParam != _encoderParam) {
			if (abs(encoderParam - _encoderParam) <= 2) {
				if (encoderParam > _encoderParam) {
					// Clockwise move (increment).
					if (!b_ChangingOption) {
						// Can change current mode.
						Mode++;
						if (Mode == METRIKS_NUM_MODES)
							Mode = 0; // Returning to first mode (aka "Voltmeter").
						_Mode = Mode; // Done by encoder: backup to old/previous state variable.
						ct_SwitchedMode = (int)(1.f * sampleRate);
						b_ChangingMode = true;
					}
					else {
						ct_OptionTimeout = (int)(10.f * sampleRate); // Restart timeout for another 5 seconds when encoder is moved.
						if (Mode == METRIKS_PEAKCOUNTER) {
							// Incrementing treshold voltage by 0.1...
							pcntTresholdVoltage++;
							if (pcntTresholdVoltage > 100)
								pcntTresholdVoltage = 100; // Maximum treshold voltage: 10V.
							_currentParameter[METRIKS_PEAKCOUNTER][0] = pcntTresholdVoltage; // Done by encoder: to backup first...
							currentParameter[METRIKS_PEAKCOUNTER][0] = pcntTresholdVoltage; // ...then regular.
							// Threshold voltage (Peak Counter mode only).
							setDisplayThresholdVoltage();
						}
						else {
							// any other parameter.
							currentParameter[Mode][currentOptionID]++;
							if (currentParameter[Mode][currentOptionID] > (tb_ParamNumPerOpt[Mode][currentOptionID] - 1))
								currentParameter[Mode][currentOptionID] = 0; // Return to first parameter.
							_currentParameter[Mode][currentOptionID] = currentParameter[Mode][currentOptionID];
							_tmpString = tb_OptParameter[Mode][currentOptionID][currentParameter[Mode][currentOptionID]];
							dmdOffsetTextMain2 = tb_OptParameterXPos[Mode][currentOptionID][currentParameter[Mode][currentOptionID]]; // Centered display on second line of DMD.
							strcpy(dmdTextMain2, _tmpString.c_str());
						}
						setMetriksParameters(Mode, currentOptionID);
					}
				}
				else {
					// Counter-clockwise move (decrement).
					if (!b_ChangingOption) {
						// Can change current mode.
						Mode--;
						if (Mode < 0)
							Mode = (METRIKS_NUM_MODES - 1); // Returning to last possible mode.
						_Mode = Mode; // Done by encoder: backup to old/previous state variable.
						ct_SwitchedMode = (int)(1.f * sampleRate);
						b_ChangingMode = true;
					}
					else {
						ct_OptionTimeout = (int)(10.f * sampleRate); // Restart timeout for another 5 seconds when encoder is moved.
						if (Mode == METRIKS_PEAKCOUNTER) {
							// Decrementing treshold voltage by 0.1...
							pcntTresholdVoltage--;
							if (pcntTresholdVoltage < 1)
								pcntTresholdVoltage = 1; // Minimum treshold voltage: 0.1V.
							_currentParameter[METRIKS_PEAKCOUNTER][0] = pcntTresholdVoltage; // Done by encoder: to backup first...
							currentParameter[METRIKS_PEAKCOUNTER][0] = pcntTresholdVoltage; // ...then regular.
							// Threshold voltage (Peak Counter mode only).
							setDisplayThresholdVoltage();
						}
						else {
							// any other parameter.
							currentParameter[Mode][currentOptionID]--;
							if (currentParameter[Mode][currentOptionID] < 0)
								currentParameter[Mode][currentOptionID] = tb_ParamNumPerOpt[Mode][currentOptionID] - 1; // Return to last parameter.
							_currentParameter[Mode][currentOptionID] = currentParameter[Mode][currentOptionID];
							_tmpString = tb_OptParameter[Mode][currentOptionID][currentParameter[Mode][currentOptionID]];
							dmdOffsetTextMain2 = tb_OptParameterXPos[Mode][currentOptionID][currentParameter[Mode][currentOptionID]]; // Centered display on second line of DMD.
							strcpy(dmdTextMain2, _tmpString.c_str());
						}
						setMetriksParameters(Mode, currentOptionID);
					}
				}
			}
			// Save current encoder position to become previous, for next check.
			_encoderParam = encoderParam;
		}

		if (b_ChangingMode) {
			if (ct_SwitchedMode > 0) {
				ct_SwitchedMode--;
				b_ChangingOption = false;
				ct_OptionTimeout = 0;
				ct_OptionBlinkTimer = 0;
				currentOptionID = 0;
				b_cvpitchMarkerVisible = false; // To avoid "marker(s)" displayed on DMD!
				// Display new mode for a given delay.
				strcpy(dmdTextMain1, "Switch to...");
				switch (Mode) {
					case METRIKS_VOLTMETER:
						// Voltmeter.
						dmdOffsetTextMain2 = 3.52f; // Centered "Voltmeter" message on second line of DMD.
						strcpy(dmdTextMain2, "Voltmeter");
						break;
					case METRIKS_CVPITCH:
						// CV Pitch.
						dmdOffsetTextMain2 = 5.408f; // Centered "CV Tun." message on second line of DMD.
						strcpy(dmdTextMain2, "CV Pitch");
						break;
					case METRIKS_BPMMETER:
						// BPM Meter.
						dmdOffsetTextMain2 = -0.24f;
						strcpy(dmdTextMain2, "BPM Meter"); // Centered "BPM Meter" message on second line of DMD.
						break;
					case METRIKS_PEAKCOUNTER:
						// Peak counter.
						dmdOffsetTextMain2 = 3.52f; // Centered "Peak Cnt." message on second line of DMD.
						strcpy(dmdTextMain2, "Peak Cnt.");
						break;
				}
				return; // while changing mode, do nothing else!
			}
			else {
				// Exit point for changing mode.
				b_cvpitchMarkerVisible = false; // To avoid "marker(s)" displayed on DMD!
				_f_InVoltage += 1.f; // By doing this, the second line of DMD will be refreshed.
				b_InopMode = false; // TEMPORARY - false means the mode is operational (totally or partially) - MUST BE REMOVED WHEN ALL MODES WORK.
				b_ChangingMode = false;
			}
		}

		// OPT. (options) button trigger: enter option for current mode, or select next option, or quit option after last option.
		if (optButton.process(params[BUTTON_OPTIONS].getValue())) {
			if (!b_ChangingMode) {
				ct_OptionTimeout = (int)(10.f * sampleRate); // Arming option timer for 5 seconds (or give additional 5 seconds).
				if (b_ChangingOption) {
					currentOptionID++;
					if (currentOptionID > tb_OptionNumPerMode[Mode] - 1) {
						// It was the last option, now exit option(s) for current mode.
						currentOptionID = 0;
						lights[LED_OPTIONS].setBrightness(0.f);
						b_ChangingOption = false; // Exit options.
						b_InopMode = false; // TEMPORARY - false means the mode is operational (totally or partially) - MUST BE REMOVED WHEN ALL MODES WORK.
						_f_InVoltage += 1.f; // By doing this, the second line of DMD will be refreshed.
					}
					else {
						_tmpString = tb_OptParameter[Mode][currentOptionID][currentParameter[Mode][currentOptionID]];
						dmdOffsetTextMain2 = tb_OptParameterXPos[Mode][currentOptionID][currentParameter[Mode][currentOptionID]]; // Centered display on second line of DMD.
						strcpy(dmdTextMain2, _tmpString.c_str());
					}
				}
				else {
					if (tb_OptionNumPerMode[Mode] > 0) {
						// Consider option(s) only if current mode have option(s).
						currentOptionID = 0;
						ct_OptionBlinkTimer = (int)(sampleRate); // For LED blink.
						b_ChangingOption = true;
						b_InopMode = false; // TEMPORARY - false means the mode is operational (totally or partially) - MUST BE REMOVED WHEN ALL MODES WORK.
						lights[LED_OPTIONS].setBrightness(1.f);
						_tmpString = tb_OptionID[Mode][0];
						strcpy(dmdTextMain1, _tmpString.c_str());
						if (Mode == METRIKS_PEAKCOUNTER) {
							// Threshold voltage (Peak Counter mode only).
							_currentParameter[METRIKS_PEAKCOUNTER][0] = pcntTresholdVoltage; // Done by encoder: to backup first...
							currentParameter[METRIKS_PEAKCOUNTER][0] = pcntTresholdVoltage; // ...then regular.
							setDisplayThresholdVoltage();
						}
						else {
							// any other parameter.
							_tmpString = tb_OptParameter[Mode][currentOptionID][0];
							dmdOffsetTextMain2 = tb_OptParameterXPos[Mode][currentOptionID][0]; // Centered display on second line of DMD.
							strcpy(dmdTextMain2, _tmpString.c_str());
						}
					}
				}
			}
			optButton.reset();
		}

		// RESET button and/or input jack:
		// - Voltmeter mode: reset Min, Max and Med voltages.
		// - Peak Counter: reset the counter. 
		if (resetButton.process(params[BUTTON_RESET].getValue()) || resetPort.process(rescale(inputs[INPUT_RESET].getVoltage(), 0.2f, 1.f, 0.0f, 1.0f))) {
			switch (Mode) {
				case METRIKS_VOLTMETER:
					// Reset Min, Max and median registered voltages.
					f_VoltageMin = f_InVoltage;
					f_VoltageMax = f_InVoltage;
					f_VoltageMed = f_InVoltage;
					_f_InVoltage += 1.f; // By doing this, the second line of DMD will be refreshed.
//					break;
//				case METRIKS_PEAKCOUNTER:
					// ToDo...
//					break;
			}
			resetButton.reset();
		}


		if (b_ChangingOption) {
			if (ct_OptionTimeout > 0) {
				int kBlinkSpeedFactor = 1;
				ct_OptionTimeout--;
				if (ct_OptionBlinkTimer > 0)
					ct_OptionBlinkTimer--;
					else ct_OptionBlinkTimer = (int)(sampleRate / kBlinkSpeedFactor);
				if (ct_OptionTimeout < (int)(sampleRate * 2.f))
					kBlinkSpeedFactor = 5;
				_tmpString = tb_OptionID[Mode][currentOptionID];
				strcpy(dmdTextMain1, _tmpString.c_str());
				if ((ct_OptionBlinkTimer % (int)(sampleRate / kBlinkSpeedFactor)) < (int)(sampleRate / kBlinkSpeedFactor / 2)) {
					lights[LED_OPTIONS].setBrightness(0.f);
					strcpy(dmdTextMain2, "");
				}
				else {
					lights[LED_OPTIONS].setBrightness(1.f);
					if (Mode == METRIKS_PEAKCOUNTER) {
						// Threshold voltage (Peak Counter mode only).
						_currentParameter[METRIKS_PEAKCOUNTER][0] = pcntTresholdVoltage; // Done by encoder: to backup first...
						currentParameter[METRIKS_PEAKCOUNTER][0] = pcntTresholdVoltage; // ...then regular.
						setDisplayThresholdVoltage();
					}
					else {
						// any other parameters are displayed as-is, from array of strings.
						_tmpString = tb_OptParameter[Mode][currentOptionID][currentParameter[Mode][currentOptionID]];
						dmdOffsetTextMain2 = tb_OptParameterXPos[Mode][currentOptionID][currentParameter[Mode][currentOptionID]]; // Centered display on second line of DMD.
						strcpy(dmdTextMain2, _tmpString.c_str());
					}
				}
			}
			else {
				lights[LED_OPTIONS].setBrightness(0.f);
				b_cvpitchMarkerVisible = false; // To avoid "marker(s)" displayed on DMD!
				_f_InVoltage += 1.f; // By doing this, the second line of DMD will be refreshed.
				currentOptionID = 0;
				ct_OptionBlinkTimer = 0;
				b_ChangingOption = false;
				b_InopMode = false; // TEMPORARY - false means the mode is operational (totally or partially) - MUST BE REMOVED WHEN ALL MODES WORK.
			}
			return;
		}
		else {
		// Display current mode on first line of DMD (if not currently changing an option for current mode).
			switch (Mode) {
				case METRIKS_VOLTMETER:
					// Voltmeter mode.
					switch (currentParameter[METRIKS_VOLTMETER][0]) {
						case 0:
							// Voltmeter, realtime (default metering option).
							strcpy(dmdTextMain1, "Voltmeter");
							break;
						case 1:
							// Voltmeter, minimum.
							strcpy(dmdTextMain1, "Voltm. Min.");
							break;
						case 2:
							// Voltmeter, maximum.
							strcpy(dmdTextMain1, "Voltm. Max.");
							break;
						case 3:
							// Voltmeter, median.
							strcpy(dmdTextMain1, "Voltm. Medn.");
					}
					break;
				case METRIKS_CVPITCH:
					strcpy(dmdTextMain1, "CV Pitch");
					break;
				case METRIKS_BPMMETER:
					strcpy(dmdTextMain1, "BPM Meter");
					break;
				case METRIKS_PEAKCOUNTER:
					strcpy(dmdTextMain1, "Peak Counter");
			}
		}

		if (b_ActiveINjack != _b_ActiveINjack) {
			// Input jack state was changed from connected to disconnected, and vice-versa).
			b_cvpitchMarkerVisible = false; // To avoid "marker(s)" displayed on DMD!
			_f_InVoltage += 1.f; // By doing this, the second line of DMD will be refreshed.
			_b_ActiveINjack = b_ActiveINjack;
		}

		if (b_ActiveINjack) {
			f_InVoltage = inputs[INPUT_SOURCE].getVoltage();
			if (f_InVoltage > f_VoltageMax)
				f_VoltageMax = f_InVoltage;
				else if (f_InVoltage < f_VoltageMin)
					f_VoltageMin = f_InVoltage;
			f_VoltageMed = (f_VoltageMax + f_VoltageMin) / 2.f;
			ct_NoInputTimer = 0;
		}
		else {
			// Input jack isn't connected...
			f_InVoltage = 0.f;
			_f_InVoltage = 1.f;
			f_VoltageMin = 99999.f;
			f_VoltageMax = -99999.f;
			f_VoltageMed = 0.f;
			// Be sure Peak Counter is stopped. Unlit PLAY/PAUSE bi-colored LED.
			lights[LED_PLAY_GREEN].setBrightness(0.f);
			lights[LED_PLAY_RED].setBrightness(0.f);
			b_PeakCounterIsPlaying = false;
			// Blinking "? Input ?" message on second line of the DMD.
			if (ct_NoInputTimer > 0)
				ct_NoInputTimer--;
				else ct_NoInputTimer = (int)(sampleRate / 5.f);
			dmdOffsetTextMain2 = -1.f;
			if (ct_NoInputTimer % (int)(sampleRate / 5.f) < (int)(sampleRate / 10.f))
				strcpy(dmdTextMain2, "? Input ?");
				else strcpy(dmdTextMain2, "");
		}

		if (b_ActiveINjack) {
			// Do possible measurement while IN jack is connected!
			switch (Mode) {
				case METRIKS_VOLTMETER:
					// Voltmeter mode implementation.
					b_InopMode = false; // TEMPORARY - false means the mode is operational (totally or partially) - MUST BE REMOVED WHEN ALL MODES WORK.
					b_cvpitchMarkerVisible = false; // To avoid "marker(s)" displayed on DMD!
					// Be sure Peak Counter is stopped. Unlit PLAY/PAUSE bi-colored LED.
					lights[LED_PLAY_GREEN].setBrightness(0.f);
					lights[LED_PLAY_RED].setBrightness(0.f);
					b_PeakCounterIsPlaying = false;
					if (f_InVoltage != _f_InVoltage) {
						// Display voltage, but if it was changed only! Also, if number of decimal(s) option was changed.
						float vFloor = -99999.f;
						float vCeiling = 99999.f;
						std::string vSign = "+";
						std::string vMask = "";
						float currentVoltage = 0.f;
						_f_InVoltage = f_InVoltage;
						switch (currentParameter[METRIKS_VOLTMETER][0]) {
							case 0:
								// Voltmeter, realtime (default metering option).
								currentVoltage = roundp((double)f_InVoltage, vltmDecimals);
								break;
							case 1:
								// Voltmeter, minimum.
								currentVoltage = roundp((double)f_VoltageMin, vltmDecimals);
								break;
							case 2:
								// Voltmeter, maximum.
								currentVoltage = roundp((double)f_VoltageMax, vltmDecimals);
								break;
							case 3:
								// Voltmeter, median.
								currentVoltage = roundp((double)f_VoltageMed, vltmDecimals);
						}
						if (currentVoltage < 0.f)
							vSign = "-";
						currentVoltage = abs(currentVoltage);
						switch (vltmDecimals) {
							case 0:
								vFloor = -99999.f;
								vCeiling = 99999.f;
								break;
							case 1:
								vFloor = -9999.99f;
								vCeiling = 9999.99f;
								break;
							case 2:
								vFloor = -999.999f;
								vCeiling = 999.999f;
								break;
							case 3:
							case 4:
								vFloor = -99.9999f;
								vCeiling = 99.9999f;
						}
						if ((currentVoltage >= vFloor) && (currentVoltage <= vCeiling)) {
							_tmpString = std::to_string(currentVoltage);
							int vPos = _tmpString.find(".");
							if (vltmDecimals == 4) {
								// Decimals: Auto
								if (_tmpString.length() > 6)
									_tmpString = _tmpString.substr(0, 6);
								while (_tmpString.at(_tmpString.length() - 1) == 48)
									_tmpString = _tmpString.substr(0, _tmpString.length() - 1);
								if (_tmpString.at(_tmpString.length() - 1) == 0x2e)
									_tmpString = _tmpString.substr(0, _tmpString.length() - 1);
								_tmpString = vSign + _tmpString + "V";
								vPos = _tmpString.find(".");
								if (vPos > 0)
									dmdOffsetTextMain2 = 102.f - (_tmpString.length() * 12.f);
									else dmdOffsetTextMain2 = 96.f - (_tmpString.length() * 12.f);
							}
							else {
								if (vltmDecimals == 0)
									vPos--;
								_tmpString = _tmpString.substr(0, vPos + vltmDecimals + 1);
								_tmpString = vSign + _tmpString + "V";
								if (vltmDecimals == 0)
									dmdOffsetTextMain2 = 96.f - (_tmpString.length() * 12.f);
									else dmdOffsetTextMain2 = 102.f - (_tmpString.length() * 12.f);
							}
							strcpy(dmdTextMain2, _tmpString.c_str());
						}
						else {
							// Voltage is out of range (overflow).
							dmdOffsetTextMain2 = 2.583f;
							strcpy(dmdTextMain2, "!Out.Rang.!");
						}
						// Restore backuped voltage...
						f_InVoltage = _f_InVoltage;
					}
					break;
				case METRIKS_CVPITCH:
					// CV Pitch mode implementation.
					b_InopMode = false; // TEMPORARY - false means the mode is operational (totally or partially) - MUST BE REMOVED WHEN ALL MODES WORK.
					// Be sure Peak Counter is stopped. Unlit PLAY/PAUSE bi-colored LED.
					lights[LED_PLAY_GREEN].setBrightness(0.f);
					lights[LED_PLAY_RED].setBrightness(0.f);
					b_PeakCounterIsPlaying = false;
					if (f_InVoltage != _f_InVoltage) {
						// Doing note search by voltage (CV), but only if voltage has changed!
						_f_InVoltage = f_InVoltage;
						int x = getNotebyFreq(dsp::FREQ_C4 * (double)(pow(2.0, f_InVoltage)));
						if (x != -1)
							_tmpString = cvpitchNote[x];
							else _tmpString = "?";
						dmdOffsetTextMain2 = getCenteredDMD(_tmpString); // Centered display on second line.
						strcpy(dmdTextMain2, _tmpString.c_str());
					}
					break;
				case METRIKS_BPMMETER:
					// BPM meter mode implementation.
					b_cvpitchMarkerVisible = false; // To avoid "marker(s)" displayed on DMD!
					// Be sure Peak Counter is stopped. Unlit PLAY/PAUSE bi-colored LED.
					lights[LED_PLAY_GREEN].setBrightness(0.f);
					lights[LED_PLAY_RED].setBrightness(0.f);
					b_PeakCounterIsPlaying = false;
					// TEMPORARY - used for inoperative mode(s) - MUST BE REMOVED WHEN ALL MODES WORK.
					setInopMode();
					break;
				case METRIKS_PEAKCOUNTER:
					// Peak counter mode implementation.
					b_cvpitchMarkerVisible = false; // To avoid "marker(s)" displayed on DMD!
					// TEMPORARY - used for inoperative mode(s) - MUST BE REMOVED WHEN ALL MODES WORK.
					setInopMode();
					break;
			}
		}

	} // end of "process"...

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Model", json_integer(Theme)); // Save current theme (model).
		json_object_set_new(rootJ, "Mode", json_integer(Mode)); // Save current mode.
		json_object_set_new(rootJ, "lastVMin", json_real(f_VoltageMin)); // Save lastest min. voltage.
		json_object_set_new(rootJ, "lastVMax", json_real(f_VoltageMax)); // Save lastest max. voltage.
		// Preset management: check if mode have suddently changed (without encoder usage).
		if (b_dspProcessing)
			if (Mode != _Mode) {
				_Mode = Mode;
			}
		// All parameters per mode and, per option (two-dimension array of integers).
		json_t *optionsJ = json_array();
		for (int i = 0; i < METRIKS_NUM_MODES; i++)
			for (int j = 0; j < 4; j++) {
				json_array_insert_new(optionsJ, (4 * i) + j, json_integer(currentParameter[i][j]));
				if (b_dspProcessing) {
					if (_currentParameter[i][j] != currentParameter[i][j]) {
						// Set up relevant mode/option.
						setMetriksParameters(i, j);
					}
				}
				_currentParameter[i][j] = currentParameter[i][j];
			}
		json_object_set_new(rootJ, "MtrxOptions", optionsJ);
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		// Retrieving saved theme (Model).
		json_t *ThemeJ = json_object_get(rootJ, "Model");
		if (ThemeJ) {
			Theme = json_integer_value(ThemeJ);
			portMetal = (Theme > 2) ? 1 : 0; // first three models are using silver connectors (0), last three as "Signature" are using golden connectors (1), instead.
		}
		// Retrieving saved measuring mode.
		json_t *ModeJ = json_object_get(rootJ, "Mode");
		if (ModeJ) {
			Mode = json_integer_value(ModeJ);
			if ((Mode < 0) || (Mode > 4))
				Mode = 0; // Set mode to Voltmeter if not compliant.
		}
		else Mode = 0; // Default voltmeter.
		// Retrieving registered last minimum voltage.
		json_t *lastVMinJ = json_object_get(rootJ, "lastVMin");
		if (lastVMinJ) {
			f_VoltageMin = json_real_value(lastVMinJ);
		}
		else f_VoltageMin = 99999.f;
		// Retrieving registered last maximum voltage.
		json_t *lastVMaxJ = json_object_get(rootJ, "lastVMax");
		if (lastVMaxJ) {
			f_VoltageMax = json_real_value(lastVMaxJ);
		}
		else f_VoltageMax = -99999.f;
		// Now we can define last median voltage (never stored, always computed).
		f_VoltageMed = (f_VoltageMax + f_VoltageMin) / 2.f;
		// Retrieving all saved options/parameters (per mode) (two-dimension array of integers).
		json_t *optionsJ = json_object_get(rootJ, "MtrxOptions");
		if (optionsJ) {
			for (int i = 0; i < METRIKS_NUM_MODES; i++) {
				for (int j = 0; j < 4; j++) {
					json_t *optionJ = json_array_get(optionsJ, (4 * i) + j);
					if (optionJ)
						currentParameter[i][j] = json_integer_value(optionJ);
						else {
							if ((i == METRIKS_PEAKCOUNTER) && (j == 0))
								currentParameter[i][j] = 1; // Default treshold voltage (Peak Counter mode) set to 0.1V, stored as integer 1 (value x 10).
								else currentParameter[i][j] = 0; // Default 0 for others.
						}
				}
			}
		}
		else {
			for (int i = 0; i < METRIKS_NUM_MODES; i++)
				for (int j = 0; j < 4; j++) {
					if ((i == METRIKS_PEAKCOUNTER) && (j == 0))
						currentParameter[i][j] = 1; // Default treshold voltage (Peak Counter mode) set to 0.1V.
						else currentParameter[i][j] = 0; // Default 0 for others.
					_currentParameter[i][j] = currentParameter[i][j];
				}
		}
	}

};

// Dot-matrix display (DMD) handler. Mainly hardcoded for best performances.
struct MetriksDMD : TransparentWidget {
	MetriksModule *module;
	std::shared_ptr<Font> font;
	std::string fontPath;

	MetriksDMD() {
		fontPath = std::string(asset::plugin(pluginInstance, "res/fonts/LEDCounter7.ttf"));
	}

	void draw(const DrawArgs &args) override {
		if (!(font = APP->window->loadFont(fontPath)))
			return;
		if (!module) {
			Vec textPos = Vec(14, box.size.y - 174);
			// Required as "module preview" (from VCV Rack modules browser).
			// Default message on DMD (LCD).
			nvgFillColor(args.vg, nvgTransRGBA(nvgRGB(0x08, 0x08, 0x08), 0xff)); // Using default black LCD.
			nvgText(args.vg, textPos.x, textPos.y, "Voltmeter", NULL); // Default message on first line (Voltmeter, the default mode).
			// Main DMD, lower line.
			nvgFontSize(args.vg, 20);
			nvgTextLetterSpacing(args.vg, -1);
			textPos = Vec(12, box.size.y - 152);
			nvgText(args.vg, textPos.x + 26, textPos.y, "+3.14V", NULL); // Default message on second line (display fictious voltage).
		}
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (!(font = APP->window->loadFont(fontPath)))
			return;
		if (!module)
			return;
		if (layer == 1) {
			if ((module->Theme == 2) && (!module->isBypassed())) {
				// Yellow rounded rectangle to simulate yellow backlit of (LCD) dot-matrix display ("Absolute Night" model only, and not bypassed).
				// Main DMD.
				nvgBeginPath(args.vg);
				nvgRoundedRect(args.vg, 7.16f, 43.6f, 105.7f, 45.48f, 6.5f);
				nvgFillColor(args.vg, nvgRGBA(0xc0, 0xe9, 0x10, 0xff));
				nvgFill(args.vg);
				nvgClosePath(args.vg);
			}
			// Main DMD, upper line.
			nvgFontSize(args.vg, 16);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, -2);
			Vec textPos = Vec(14, box.size.y - 174);
			nvgFillColor(args.vg, nvgTransRGBA(tblDMDtextColor[module->Theme], 0xff)); // Using current color for DMD.
			if (!module->isBypassed())
				nvgText(args.vg, textPos.x, textPos.y, module->dmdTextMain1, NULL); // Proceeding module->dmdTextMain2 string (second line).
			// Main DMD, lower line.
			nvgFontSize(args.vg, 20);
			nvgTextLetterSpacing(args.vg, -1);
			textPos = Vec(12, box.size.y - 152);
			if (!module->isBypassed())
				nvgText(args.vg, textPos.x + module->dmdOffsetTextMain2, textPos.y, module->dmdTextMain2, NULL); // Displaying module->dmdTextMain2 string (second line). The second line may have an horizontal offset.
			// CV Tuner (Mode = 1) only from this point.
			if (module->Mode != 1)
				return; // Exit immediatly (code below will be ignored) if current mode isn't "CV Tuner".
			if (module->b_ChangingMode || module->b_ChangingOption)
				return; // Exit immediatly if currently changing mode or changing option.
			if (!module->b_cvpitchMarkerVisible)
				return; // Flag "module->b_cvpitchMarkerVisible" is false, don't display marker(s): exit method immediatly.
			// Display marker(s) on the DMD.
			nvgFontSize(args.vg, 14);
			nvgTextLetterSpacing(args.vg, -1);
			textPos = Vec(12.f, box.size.y - 154);
			if (!module->isBypassed())
				nvgText(args.vg, textPos.x + module->dmdCVPitchMarkerPos, textPos.y, module->dmdCVPitchMarker, NULL);
		}
		Widget::drawLayer(args, layer);
	}

};

///////////////////////////////////////////////// MODULE WIDGET SECTION /////////////////////////////////////////////////

struct MetriksWidget : ModuleWidget {
	// Panels.
	SvgPanel *panelMetriksCreamy;
	SvgPanel *panelMetriksStageRepro;
	SvgPanel *panelMetriksAbsoluteNight;
	SvgPanel *panelMetriksDarkSignature;
	SvgPanel *panelMetriksDeepBlueSignature;
	SvgPanel *panelMetriksTitaniumSignature;
	// Silver Torx screws.
	SvgScrew *topLeftScrewSilver;
	SvgScrew *topRightScrewSilver;
	SvgScrew *bottomLeftScrewSilver;
	SvgScrew *bottomRightScrewSilver;
	// Gold Torx screws.
	SvgScrew *topLeftScrewGold;
	SvgScrew *topRightScrewGold;
	SvgScrew *bottomLeftScrewGold;
	SvgScrew *bottomRightScrewGold;
	// Silver buttons.
	SvgSwitch *buttonOptionsSilver;
	SvgSwitch *buttonPlayPauseSilver;
	SvgSwitch *buttonResetSilver;
	// Gold buttons.
	SvgSwitch *buttonOptionsGold;
	SvgSwitch *buttonPlayPauseGold;
	SvgSwitch *buttonResetGold;

	MetriksWidget(MetriksModule *module) {
		setModule(module);
		box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		// Creamy panel.
		panelMetriksCreamy = new SvgPanel();
		panelMetriksCreamy->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Metriks_Creamy.svg")));
		panelMetriksCreamy->visible = !rack::settings::preferDarkPanels;
		addChild(panelMetriksCreamy);
		// Stage Repro panel.
		panelMetriksStageRepro = new SvgPanel();
		panelMetriksStageRepro->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Metriks_Stage_Repro.svg")));
		panelMetriksStageRepro->visible = false;
		addChild(panelMetriksStageRepro);
		// Absolute Night panel.
		panelMetriksAbsoluteNight = new SvgPanel();
		panelMetriksAbsoluteNight->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Metriks_Absolute_Night.svg")));
		panelMetriksAbsoluteNight->visible = rack::settings::preferDarkPanels;
		addChild(panelMetriksAbsoluteNight);
		// Dark Signature panel.
		panelMetriksDarkSignature = new SvgPanel();
		panelMetriksDarkSignature->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Metriks_Dark_Signature.svg")));
		panelMetriksDarkSignature->visible = false;
		addChild(panelMetriksDarkSignature);
		// Deepblue Signature panel.
		panelMetriksDeepBlueSignature = new SvgPanel();
		panelMetriksDeepBlueSignature->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Metriks_Deepblue_Signature.svg")));
		panelMetriksDeepBlueSignature->visible = false;
		addChild(panelMetriksDeepBlueSignature);
		// Titanium Signature panel.
		panelMetriksTitaniumSignature = new SvgPanel();
		panelMetriksTitaniumSignature->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Metriks_Titanium_Signature.svg")));
		panelMetriksTitaniumSignature->visible = false;
		addChild(panelMetriksTitaniumSignature);
		// The DMD display.
		{
			MetriksDMD *display = new MetriksDMD();
			display->box.pos = Vec(0, 0);
			display->box.size = Vec(box.size.x, 234);
			display->module = module;
			addChild(display);
		}
		// Top-left golden screw.
		topLeftScrewGold = createWidget<Torx_Gold>(Vec(RACK_GRID_WIDTH, 0));
		addChild(topLeftScrewGold);
		// Top-left silver screw.
		topLeftScrewSilver = createWidget<Torx_Silver>(Vec(RACK_GRID_WIDTH, 0));
		addChild(topLeftScrewSilver);
		// Top-right golden screw.
		topRightScrewGold = createWidget<Torx_Gold>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0));
		addChild(topRightScrewGold);
		// Top-right silver screw.
		topRightScrewSilver = createWidget<Torx_Silver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0));
		addChild(topRightScrewSilver);
		// Bottom-left golden screw.
		bottomLeftScrewGold = createWidget<Torx_Gold>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomLeftScrewGold);
		// Bottom-left silver screw.
		bottomLeftScrewSilver = createWidget<Torx_Silver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomLeftScrewSilver);
		// Bottom-right golden screw.
		bottomRightScrewGold = createWidget<Torx_Gold>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomRightScrewGold);
		// Bottom-right silver screw.
		bottomRightScrewSilver = createWidget<Torx_Silver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH));
		addChild(bottomRightScrewSilver);
		// Input ports (ports are using "dynamic ports" to change connector metal - silver or gold: big thanks to Marc BoulÃ©!).
		addInput(createDynamicPort<DynSVGPort>(Vec(24, 304), true, module, MetriksModule::INPUT_SOURCE, module ? &module->portMetal : NULL));
		addInput(createDynamicPort<DynSVGPort>(Vec(24, 262), true, module, MetriksModule::INPUT_PLAYPAUSE, module ? &module->portMetal : NULL));
		addInput(createDynamicPort<DynSVGPort>(Vec(72, 262), true, module, MetriksModule::INPUT_RESET, module ? &module->portMetal : NULL));
		// Output port (ports are using "dynamic ports" to change connector metal - silver or gold: big thanks to Marc BoulÃ©!).
		addOutput(createDynamicPort<DynSVGPort>(Vec(72, 304), false, module, MetriksModule::OUTPUT_THRU, module ? &module->portMetal : NULL));
		// Multipurpose continuous encoder (used to select BPM, modulator ratio or setup items).
		addParam(createParam<Ohmer_Encoder>(Vec(20, 106), module, MetriksModule::PARAM_ENCODER));
		// Gold push button to select option.
		buttonOptionsGold = createParam<Ohmer_ButtonGold>(Vec(94, 178), module, MetriksModule::BUTTON_OPTIONS);
		addParam(buttonOptionsGold);
		// Silver push button to select option.
		buttonOptionsSilver = createParam<Ohmer_ButtonSilver>(Vec(94, 178), module, MetriksModule::BUTTON_OPTIONS);
		addParam(buttonOptionsSilver);
		// Gold push button to toggle PLAY/PAUSE.
		buttonPlayPauseGold = createParam<Ohmer_ButtonGold>(Vec(27.4, 240), module, MetriksModule::BUTTON_PLAYPAUSE);
		addParam(buttonPlayPauseGold);
		// Silver push button to toggle PLAY/PAUSE.
		buttonPlayPauseSilver = createParam<Ohmer_ButtonSilver>(Vec(27.4, 240), module, MetriksModule::BUTTON_PLAYPAUSE);
		addParam(buttonPlayPauseSilver);
		// Gold push button to toggle PLAY/PAUSE.
		buttonResetGold = createParam<Ohmer_ButtonGold>(Vec(75.4, 240), module, MetriksModule::BUTTON_RESET);
		addParam(buttonResetGold);
		// Silver push button to toggle PLAY/PAUSE.
		buttonResetSilver = createParam<Ohmer_ButtonSilver>(Vec(75.4, 240), module, MetriksModule::BUTTON_RESET);
		addParam(buttonResetSilver);
		// PLAY/STOP (bicolored green/red) LED.
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(18, 252), module, MetriksModule::LED_PLAY_GREEN)); // Unified PLAY/STOP LED (green/red).
		// OPTIONS (OPT.) red LED.
		addChild(createLight<MediumLight<RedLight>>(Vec(83, 200.5), module, MetriksModule::LED_OPTIONS)); // Unified PLAY/STOP LED (green/red).
	}

	void step() override {
		MetriksModule *module = dynamic_cast<MetriksModule*>(this->module);
		if (!module) {
			// Probably from module browser...
			// Default model is always "Creamy" or "Absolute Night" (depending "Use dark panels if available" option, from "View" menu).
			// Other panels are, of course, hidden.
			panelMetriksCreamy->visible = !rack::settings::preferDarkPanels;
			panelMetriksStageRepro->visible = false;
			panelMetriksAbsoluteNight->visible = rack::settings::preferDarkPanels;
			panelMetriksDarkSignature->visible = false;
			panelMetriksDeepBlueSignature->visible = false;
			panelMetriksTitaniumSignature->visible = false;
			// By default, silver screws are always visible by default ("Creamy" or "Absolute Night" panel).
			topLeftScrewSilver->visible = true;
			topRightScrewSilver->visible = true;
			bottomLeftScrewSilver->visible = true;
			bottomRightScrewSilver->visible = true;
			// ...and, of course, golden screws are hidden.
			topLeftScrewGold->visible = false;
			topRightScrewGold->visible = false;
			bottomLeftScrewGold->visible = false;
			bottomRightScrewGold->visible = false;
			// By default, silver buttons are always visible...
			buttonOptionsSilver->visible = true;
			buttonPlayPauseSilver->visible = true;
			buttonResetSilver->visible = true;
			// ...and, of course, golden buttons are hidden...
			buttonOptionsGold->visible = false;
			buttonPlayPauseGold->visible = false;
			buttonResetGold->visible = false;
			return;
		}
		else {
			// The six possible panels.
			panelMetriksCreamy->visible = (module->Theme == 0);
			panelMetriksStageRepro->visible = (module->Theme == 1);
			panelMetriksAbsoluteNight->visible = (module->Theme == 2);
			panelMetriksDarkSignature->visible = (module->Theme == 3);
			panelMetriksDeepBlueSignature->visible = (module->Theme == 4);
			panelMetriksTitaniumSignature->visible = (module->Theme == 5);
			// Torx screws metal (silver, golden) are visible or hidden, depending selected model (from module's contextual menu).
			// Silver Torx screws are visible only for non-"Signature" modules (Creamy, Stage Repro or Absolute Night).
			topLeftScrewSilver->visible = (module->Theme < 3);
			topRightScrewSilver->visible = (module->Theme < 3);
			bottomLeftScrewSilver->visible = (module->Theme < 3);
			bottomRightScrewSilver->visible = (module->Theme < 3);
			// Golden Torx screws are visible only for "Signature"-line modules (Dark "Signature", Deepblue "Signature" or Titanium "Signature").
			topLeftScrewGold->visible = (module->Theme > 2);
			topRightScrewGold->visible = (module->Theme > 2);
			bottomLeftScrewGold->visible = (module->Theme > 2);
			bottomRightScrewGold->visible = (module->Theme > 2);
			// Silver buttons are visible for first three models (themes) as non-Signature modules.
			buttonOptionsSilver->visible = (module->Theme < 3);
			buttonPlayPauseSilver->visible = (module->Theme < 3);
			buttonResetSilver->visible = (module->Theme < 3);
			// Golden buttons are visible for last three models (themes) aka "Signature"-line modules.
			buttonOptionsGold->visible = (module->Theme > 2);
			buttonPlayPauseGold->visible = (module->Theme > 2);
			buttonResetGold->visible = (module->Theme > 2);
		}
		ModuleWidget::step();
	}

	///////////////////////////////////////////////////////// CONTEXTUAL MENU //////////////////////////////////////////////////////////

	///////////////////////////////////////////////////// CONTEXTUAL MENU - THEME //////////////////////////////////////////////////////

	struct MetriksThemeCreamyMenuItem : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 0; // Creamy.
			module->portMetal = 0; // Silver connectors.
		}
	};

	struct MetriksThemeStageReproMenuItem : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 1; // Stage Repro.
			module->portMetal = 0; // Silver connectors.
		}
	};

	struct MetriksThemeAbsoluteNightMenuItem : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 2; // Absolute Night.
			module->portMetal = 0; // Silver connectors.
		}
	};

	struct MetriksThemeDarkSignatureMenuItem : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 3; // Dark Signature.
			module->portMetal = 1; // Golden connectors.
		}
	};

	struct MetriksThemeDeepblueSignatureMenuItem : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 4; // Deepblue Signature.
			module->portMetal = 1; // Golden connectors.
		}
	};

	struct MetriksThemeTitaniumSignatureMenuItem : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			module->Theme = 5; // Titanium Signature.
			module->portMetal = 1; // Golden connectors.
		}
	};

	struct MetriksThemeSubmenuItems : MenuItem {
		MetriksModule *module;
		Menu *createChildMenu() override {
			Menu *menu = new Menu;

			MetriksThemeCreamyMenuItem *metriksthemecreamymenuitem = new MetriksThemeCreamyMenuItem;
			metriksthemecreamymenuitem->text = "Creamy";
			metriksthemecreamymenuitem->rightText = CHECKMARK(module->Theme == 0);
			metriksthemecreamymenuitem->module = module;
			menu->addChild(metriksthemecreamymenuitem);

			MetriksThemeStageReproMenuItem *metriksthemestagerepromenuitem = new MetriksThemeStageReproMenuItem;
			metriksthemestagerepromenuitem->text = "Stage Repro";
			metriksthemestagerepromenuitem->rightText = CHECKMARK(module->Theme == 1);
			metriksthemestagerepromenuitem->module = module;
			menu->addChild(metriksthemestagerepromenuitem);

			MetriksThemeAbsoluteNightMenuItem *metriksthemeabsolutenightmenuitem = new MetriksThemeAbsoluteNightMenuItem;
			metriksthemeabsolutenightmenuitem->text = "Absolute Night";
			metriksthemeabsolutenightmenuitem->rightText = CHECKMARK(module->Theme == 2);
			metriksthemeabsolutenightmenuitem->module = module;
			menu->addChild(metriksthemeabsolutenightmenuitem);

			MetriksThemeDarkSignatureMenuItem *metriksthemedarksignaturemenuitem = new MetriksThemeDarkSignatureMenuItem;
			metriksthemedarksignaturemenuitem->text = "Dark \"Signature\"";
			metriksthemedarksignaturemenuitem->rightText = CHECKMARK(module->Theme == 3);
			metriksthemedarksignaturemenuitem->module = module;
			menu->addChild(metriksthemedarksignaturemenuitem);

			MetriksThemeDeepblueSignatureMenuItem *metriksthemedeepbluesignaturemenuitem = new MetriksThemeDeepblueSignatureMenuItem;
			metriksthemedeepbluesignaturemenuitem->text = "Deepblue \"Signature\"";
			metriksthemedeepbluesignaturemenuitem->rightText = CHECKMARK(module->Theme == 4);
			metriksthemedeepbluesignaturemenuitem->module = module;
			menu->addChild(metriksthemedeepbluesignaturemenuitem);

			MetriksThemeTitaniumSignatureMenuItem *metriksthemetitaniumsignaturemenuitem = new MetriksThemeTitaniumSignatureMenuItem;
			metriksthemetitaniumsignaturemenuitem->text = "Titanium \"Signature\"";
			metriksthemetitaniumsignaturemenuitem->rightText = CHECKMARK(module->Theme == 5);
			metriksthemetitaniumsignaturemenuitem->module = module;
			menu->addChild(metriksthemetitaniumsignaturemenuitem);

			return menu;
		}

	};

	/////////////////////////////////////////////////////// VOLTMETER - METERING SUBMENU //////////////////////////////////////////////////////

	struct MetriksVoltmeterMeteringRT : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			module->currentParameter[module->METRIKS_VOLTMETER][0] = 0; // Metering: Realtime.
			// By doing this, the second line of DMD will be refreshed.
			module->_f_InVoltage += 1.f;
		}
	};

	struct MetriksVoltmeterMeteringMin : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			module->currentParameter[module->METRIKS_VOLTMETER][0] = 1; // Metering: Min.
			// By doing this, the second line of DMD will be refreshed.
			module->_f_InVoltage += 1.f;
		}
	};

	struct MetriksVoltmeterMeteringMax : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			module->currentParameter[module->METRIKS_VOLTMETER][0] = 2; // Metering: Max.
			// By doing this, the second line of DMD will be refreshed.
			module->_f_InVoltage += 1.f;
		}
	};

	struct MetriksVoltmeterMeteringMedian : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			module->currentParameter[module->METRIKS_VOLTMETER][0] = 3; // Metering: Median.
			// By doing this, the second line of DMD will be refreshed.
			module->_f_InVoltage += 1.f;
		}
	};

	struct MetriksVoltmeterMeteringMenuItems : MenuItem {
		MetriksModule *module;
		Menu *createChildMenu() override {
			Menu *menu = new Menu;

			MetriksVoltmeterMeteringRT *metriksvoltmetermeteringrt = new MetriksVoltmeterMeteringRT;
			metriksvoltmetermeteringrt->text = "Realtime";
			metriksvoltmetermeteringrt->rightText = CHECKMARK(module->currentParameter[module->METRIKS_VOLTMETER][0] == 0);
			metriksvoltmetermeteringrt->module = module;
			menu->addChild(metriksvoltmetermeteringrt);

			MetriksVoltmeterMeteringMin *metriksvoltmetermeteringmin = new MetriksVoltmeterMeteringMin;
			metriksvoltmetermeteringmin->text = "Minimum";
			metriksvoltmetermeteringmin->rightText = CHECKMARK(module->currentParameter[module->METRIKS_VOLTMETER][0] == 1);
			metriksvoltmetermeteringmin->module = module;
			menu->addChild(metriksvoltmetermeteringmin);

			MetriksVoltmeterMeteringMax *metriksvoltmetermeteringmax = new MetriksVoltmeterMeteringMax;
			metriksvoltmetermeteringmax->text = "Maximum";
			metriksvoltmetermeteringmax->rightText = CHECKMARK(module->currentParameter[module->METRIKS_VOLTMETER][0] == 2);
			metriksvoltmetermeteringmax->module = module;
			menu->addChild(metriksvoltmetermeteringmax);

			MetriksVoltmeterMeteringMedian *metriksvoltmetermeteringmedian = new MetriksVoltmeterMeteringMedian;
			metriksvoltmetermeteringmedian->text = "Median";
			metriksvoltmetermeteringmedian->rightText = CHECKMARK(module->currentParameter[module->METRIKS_VOLTMETER][0] == 3);
			metriksvoltmetermeteringmedian->module = module;
			menu->addChild(metriksvoltmetermeteringmedian);

			return menu;
		}

	};

	/////////////////////////////////////////////////////// VOLTMETER - DECIMALS SUBMENU //////////////////////////////////////////////////////

	struct MetriksVoltmeterAutoDecimals : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			module->vltmDecimals = 4; // Decimals: Auto.
			// By doing this, the second line of DMD will be refreshed.
			module->_f_InVoltage += 1.f;
		}
	};

	struct MetriksVoltmeterZeroDecimal : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			module->vltmDecimals = 0; // Decimals: 0.
			// By doing this, the second line of DMD will be refreshed.
			module->_f_InVoltage += 1.f;
		}
	};

	struct MetriksVoltmeterOneDecimal : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			module->vltmDecimals = 1; // Decimals: 1.
			// By doing this, the second line of DMD will be refreshed.
			module->_f_InVoltage += 1.f;
		}
	};

	struct MetriksVoltmeterTwoDecimals : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			module->vltmDecimals = 2; // Decimals: 2.
			// By doing this, the second line of DMD will be refreshed.
			module->_f_InVoltage += 1.f;
		}
	};

	struct MetriksVoltmeterThreeDecimals : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			module->vltmDecimals = 3; // Decimals: 3.
			// By doing this, the second line of DMD will be refreshed.
			module->_f_InVoltage += 1.f;
		}
	};

	struct MetriksVoltmeterDecimalsMenuItems : MenuItem {
		MetriksModule *module;
		Menu *createChildMenu() override {
			Menu *menu = new Menu;

			MetriksVoltmeterAutoDecimals *metriksvoltmeterautodecimals = new MetriksVoltmeterAutoDecimals;
			metriksvoltmeterautodecimals->text = "Auto";
			metriksvoltmeterautodecimals->rightText = CHECKMARK(module->vltmDecimals == 4);
			metriksvoltmeterautodecimals->module = module;
			menu->addChild(metriksvoltmeterautodecimals);

			MetriksVoltmeterZeroDecimal *metriksvoltmeterzerodecimal = new MetriksVoltmeterZeroDecimal;
			metriksvoltmeterzerodecimal->text = "0";
			metriksvoltmeterzerodecimal->rightText = CHECKMARK(module->vltmDecimals == 0);
			metriksvoltmeterzerodecimal->module = module;
			menu->addChild(metriksvoltmeterzerodecimal);

			MetriksVoltmeterOneDecimal *metriksvoltmeteronedecimal = new MetriksVoltmeterOneDecimal;
			metriksvoltmeteronedecimal->text = "1";
			metriksvoltmeteronedecimal->rightText = CHECKMARK(module->vltmDecimals == 1);
			metriksvoltmeteronedecimal->module = module;
			menu->addChild(metriksvoltmeteronedecimal);

			MetriksVoltmeterTwoDecimals *metriksvoltmetertwodecimals = new MetriksVoltmeterTwoDecimals;
			metriksvoltmetertwodecimals->text = "2";
			metriksvoltmetertwodecimals->rightText = CHECKMARK(module->vltmDecimals == 2);
			metriksvoltmetertwodecimals->module = module;
			menu->addChild(metriksvoltmetertwodecimals);

			MetriksVoltmeterThreeDecimals *metriksvoltmeterthreedecimals = new MetriksVoltmeterThreeDecimals;
			metriksvoltmeterthreedecimals->text = "3";
			metriksvoltmeterthreedecimals->rightText = CHECKMARK(module->vltmDecimals == 3);
			metriksvoltmeterthreedecimals->module = module;
			menu->addChild(metriksvoltmeterthreedecimals);

			return menu;
		}

	};

	/////////////////////////////////////////////////////// CV PITCH - NOTATION SUBMENU //////////////////////////////////////////////////////

	struct MetriksCVPitchInternational : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			if (module->currentParameter[module->METRIKS_CVPITCH][0] != 0) {
				// Only changed if necessary!
				module->currentParameter[module->METRIKS_CVPITCH][0] = 0; // Notation: International (C/D/E.../B).
				module->makeNotesTables(); // Notes tables must be refreshed!
				// By doing this, the second line of DMD will be refreshed.
				module->_f_InVoltage += 1.f;
			}
		}
	};

	struct MetriksCVPitchLatin : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			if (module->currentParameter[module->METRIKS_CVPITCH][0] != 1) {
				// Only changed if necessary!
				module->currentParameter[module->METRIKS_CVPITCH][0] = 1; // Notation: Latin (Do/Ré/Mi.../Si).
				module->makeNotesTables(); // Notes tables must be refreshed!
				// By doing this, the second line of DMD will be refreshed.
				module->_f_InVoltage += 1.f;
			}
		}
	};

	struct MetriksCVPitchNotationMenuItems : MenuItem {
		MetriksModule *module;
		Menu *createChildMenu() override {
			Menu *menu = new Menu;

			MetriksCVPitchInternational *metrikscvpitchinternational = new MetriksCVPitchInternational;
			metrikscvpitchinternational->text = "International (C/D/E.../B)";
			metrikscvpitchinternational->rightText = CHECKMARK(module->currentParameter[module->METRIKS_CVPITCH][0] == 0);
			metrikscvpitchinternational->module = module;
			menu->addChild(metrikscvpitchinternational);

			MetriksCVPitchLatin *metrikscvpitchlatin = new MetriksCVPitchLatin;
			metrikscvpitchlatin->text = "Latin (Do/Ré/Mi.../Si)";
			metrikscvpitchlatin->rightText = CHECKMARK(module->currentParameter[module->METRIKS_CVPITCH][0] == 1);
			metrikscvpitchlatin->module = module;
			menu->addChild(metrikscvpitchlatin);

			return menu;
		}

	};

	/////////////////////////////////////////////////////// CV PITCH - SHARPS/FLATS SUBMENU //////////////////////////////////////////////////////

	struct MetriksCVPitchSharps : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			if (module->currentParameter[module->METRIKS_CVPITCH][1] != 0) {
				// Only changed if necessary!
				module->currentParameter[module->METRIKS_CVPITCH][1] = 0; // Sharps.
				module->makeNotesTables(); // Notes tables must be refreshed!
				// By doing this, the second line of DMD will be refreshed.
				module->_f_InVoltage += 1.f;
			}
		}
	};

	struct MetriksCVPitchFlats : MenuItem {
		MetriksModule *module;
		void onAction(const ActionEvent& e) override {
			if (module->currentParameter[module->METRIKS_CVPITCH][1] != 1) {
				// Only changed if necessary!
				module->currentParameter[module->METRIKS_CVPITCH][1] = 1; // Flats.
				module->makeNotesTables(); // Notes tables must be refreshed!
				// By doing this, the second line of DMD will be refreshed.
				module->_f_InVoltage += 1.f;
			}
		}
	};

	struct MetriksCVPitchSharpsFlatsMenuItems : MenuItem {
		MetriksModule *module;
		Menu *createChildMenu() override {
			Menu *menu = new Menu;

			MetriksCVPitchSharps *metrikscvpitchsharps = new MetriksCVPitchSharps;
			metrikscvpitchsharps->text = "Sharps (#)";
			metrikscvpitchsharps->rightText = CHECKMARK(module->currentParameter[module->METRIKS_CVPITCH][1] == 0);
			metrikscvpitchsharps->module = module;
			menu->addChild(metrikscvpitchsharps);

			MetriksCVPitchFlats *metrikscvpitchflats = new MetriksCVPitchFlats;
			metrikscvpitchflats->text = "Flats (b)";
			metrikscvpitchflats->rightText = CHECKMARK(module->currentParameter[module->METRIKS_CVPITCH][1] == 1);
			metrikscvpitchflats->module = module;
			menu->addChild(metrikscvpitchflats);

			return menu;
		}

	};

/////////////////////////////////////////////////////// MAIN CONTEXTUAL MENU //////////////////////////////////////////////////////

	void appendContextMenu(Menu *menu) override {
		MetriksModule *module = dynamic_cast<MetriksModule*>(this->module);

		if (!module)
			return;

		menu->addChild(new MenuSeparator);

		MetriksThemeSubmenuItems *metriksthemesubmenuitems = new MetriksThemeSubmenuItems;
		metriksthemesubmenuitems->text = "Model";
		metriksthemesubmenuitems->rightText = RIGHT_ARROW;
		metriksthemesubmenuitems->module = module;
		menu->addChild(metriksthemesubmenuitems);

		menu->addChild(new MenuSeparator);

		if (module->Mode == module->METRIKS_VOLTMETER) {
			// While "Voltmeter" mode is active, we're using its related options: Metering, Decimals.

			MetriksVoltmeterMeteringMenuItems *metriksvoltmetermeteringmenuitems = new MetriksVoltmeterMeteringMenuItems;
			metriksvoltmetermeteringmenuitems->text = "Metering";
			metriksvoltmetermeteringmenuitems->rightText = RIGHT_ARROW;
			metriksvoltmetermeteringmenuitems->module = module;
			menu->addChild(metriksvoltmetermeteringmenuitems);

			MetriksVoltmeterDecimalsMenuItems *metriksvoltmeterdecimalsmenuitems = new MetriksVoltmeterDecimalsMenuItems;
			metriksvoltmeterdecimalsmenuitems->text = "Decimals";
			metriksvoltmeterdecimalsmenuitems->rightText = RIGHT_ARROW;
			metriksvoltmeterdecimalsmenuitems->module = module;
			menu->addChild(metriksvoltmeterdecimalsmenuitems);
		}
		if (module->Mode == module->METRIKS_CVPITCH) {
			// While "CV Pitch" mode is active, we're using its related options: Notation, Sharps/Flats.

			MetriksCVPitchNotationMenuItems *metrikscvpitchnotationmenuitems = new MetriksCVPitchNotationMenuItems;
			metrikscvpitchnotationmenuitems->text = "Notation";
			metrikscvpitchnotationmenuitems->rightText = RIGHT_ARROW;
			metrikscvpitchnotationmenuitems->module = module;
			menu->addChild(metrikscvpitchnotationmenuitems);

			MetriksCVPitchSharpsFlatsMenuItems *metrikscvpitchsharpsflatsmenuitems = new MetriksCVPitchSharpsFlatsMenuItems;
			metrikscvpitchsharpsflatsmenuitems->text = "Sharps/Flats";
			metrikscvpitchsharpsflatsmenuitems->rightText = RIGHT_ARROW;
			metrikscvpitchsharpsflatsmenuitems->module = module;
			menu->addChild(metrikscvpitchsharpsflatsmenuitems);
		}
	}

};

Model *modelMetriks = createModel<MetriksModule, MetriksWidget>("Metriks");
