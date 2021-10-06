////////////////////////////////////////////////////////////////////////
//// Metriks is a 8 HP measuring/visual module:                    /////
//// - Voltmeter.                                                  /////
//// - CV Tuner.                                                   /////
//// - Frequency Counter.                                          /////
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
	float sampleRate = 44100.0f;

	//
	bool b_dspIsRunning = false; // Will be set true as soon as DSP is running.

	// Current selected Metriks model (GUI theme).
	int Theme = 0;
	int portMetal = 0; // used to select silver or gold jacks.

	// DMD-font color (default is black LCD), related to selected model.
	NVGcolor DMDtextColor = nvgRGB(0x08, 0x08, 0x08);

	// Mode (0: voltmeter, 1: CV Tuner, 2: frequency counter, 2: BPM meter, 4: peak counter).
	bool bChangingMode = false; // true during mode transition, false otherwise.
	int Mode = 0; // Current mode.
	int _Mode = 0; // Its old/previous state (required for Preset management).
	int ct_SwitchedMode = 0; // Counter used as "timer" when switching to next/previous mode.

	// Counter used when input isn't connected to blink "? INPUT ?" message on DMD (second line).
	int ct_NoInputTimer = 0;

	// Option to change (depending mode).
	bool bChangingOption = false;
	int currentOptionID = 0; // Option identifier (0 is first).
	int ct_OptionTimeout = 0; // Counter used as "timer" when editing any option.
	int ct_OptionBlinkTimer = 0; // Used to relevant LED blink.

	// Tables (arrays) used for options/parameters.
	enum en_Modes {
		METRIKS_VOLTMETER,
		METRIKS_CVTUNER,
		METRIKS_FREQCOUNTER,
		METRIKS_BPMMETER,
		METRIKS_PEAKCOUNTER,
		METRIKS_NUM_MODES
	};

	const int tb_OptionNumPerMode[METRIKS_NUM_MODES] = {2, 2, 1, 0, 1}; // For each mode, number of possible option(s). BPM meter doesn't have option.
	std::string tb_OptionID[METRIKS_NUM_MODES][4]; // Will be initialized later (from module constructor).
	int tb_ParamNumPerOpt[METRIKS_NUM_MODES][4]; // Will be initialized later (from module constructor).
	std::string tb_OptParameter[METRIKS_NUM_MODES][4][4]; // Will be initialized later (from module constructor).
	float tb_OptParameterXPos[METRIKS_NUM_MODES][4][4]; // Message positions (on line 2 of DMD). Will be initialized later (from module constructor).
	int currentParameter[METRIKS_NUM_MODES][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {17, 0, 0, 0}}; // Must be initialized here, to avoid potential crash on instanciate!
	int _currentParameter[METRIKS_NUM_MODES][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {17, 0, 0, 0}}; // Must be initialized here, to avoid potential crash on instanciate!

	// Frequencies tables used by CV Tuner feature. Will are initialized later (from module constructor).
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

	// CV Tuner variables.
	std::string tunerBaseNoteName[12] = {"", "", "", "", "", "", "", "", "", "", "", ""}; // Base note names, for now empty, filled later...
	bool bUpdateNotesTable = true;
	std::string tunerNote[132];
	char dmdTunerMarker[3] = ""; // CV Tuner only, to display the below/above marker(s).
	float dmdTunerMarkerPos = 0.0f;
	bool b_tunrMarkerVisible = false;

	// Messages displayed on DMD (dot-matrix display), using two lines..
	char dmdTextMain1[20] = ""; // 20 chars for upper (1st) line.
	char dmdTextMain2[20] = ""; // 20 chars for lower (2nd) line.
	float dmdOffsetTextMain2 = 0.0f; // Horizontal offset on DMD to display for lower (2nd) line.

	// Encoder (registered position to be used on next step for relative move).
	int encoderParam = 0; // Encoder parameter.
	int _encoderParam = 0; // Previous encoder parameter.

	// OPT. (options) button.
	dsp::SchmittTrigger optButton;

	// IN (INput) jack.
	bool bActiveINjack = false;
	bool _bActiveINjack = false; // Old/previous IN jack state.
	float f_InVoltage = 0.0f;
	float _f_InVoltage = -1.0f; // Old/previous voltage on IN jack.
	// Used for mix, max, and median.
	float f_VoltageMin = 99999.0f;
	float f_VoltageMax = -99999.0f;
	float f_VoltageMed = 0.0f;

	// PLAY/STOP button and related (trigger) port (PLAY/STOP is used for "Pulse Counter" mode only).
	dsp::SchmittTrigger playButton;
	dsp::SchmittTrigger playPort;
	bool bPeakCounterIsPlaying = false;

	// RESET (RST) button and related (trigger) port.
	dsp::SchmittTrigger resetButton;
	dsp::SchmittTrigger resetPort;

	// Number of decimals can be displayed by voltmeter.
	int vltmDecimals = 2; // Number of decimals used by Voltmeter
	int _vltmDecimals = -1; // Old/previous number of decimals.

	// Schmitt trigger used to determine frequency. Also used for peak counter mode.
	dsp::SchmittTrigger inputPort;

	// Used by Peak Counter mode, for peak detection at treshold voltage.
	int pcntTresholdVoltage = 17;
	float f_pcntTresholdVoltage = 1.7f;

	// Dummy string (used for std::string to char * conversions).
	std::string _tmpString; // Dummy string.

	// TEMPORARY - used for inoperative mode(s) - MUST BE REMOVED WHEN ALL MODES WORK.
	bool b_InopMode = false;
	int ct_InopModeTimer = 0;
	int i_InopMsgCycling = 0;

	MetriksModule() {
		b_dspIsRunning = false; // Will be set true as soon as DSP is running.
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(PARAM_ENCODER, -INFINITY, INFINITY, 0.0f, "Encoder");
		configParam(BUTTON_OPTIONS, 0.0f, 1.0f, 0.0f, "Options");
		configParam(BUTTON_PLAYPAUSE, 0.0f, 1.0f, 0.0f, "Play/Pause");
		configParam(BUTTON_RESET, 0.0f, 1.0f, 0.0f, "Reset");
		configBypass(INPUT_SOURCE, OUTPUT_THRU);
		b_InopMode = false; // TEMPORARY - false means the mode is operational (totally or partially) - MUST BE REMOVED WHEN ALL MODES WORK.
		bChangingMode = false;
		ct_SwitchedMode = 0;
		bChangingOption = false;
		currentOptionID = -1; // Option ID (-1 while not option edit).
		ct_OptionTimeout = 0; // Used as timer for current option (as time out).
		ct_OptionBlinkTimer = 0;
		bPeakCounterIsPlaying = false;
		for (int i = 0; i < METRIKS_NUM_MODES; i++)
			for (int j = 0; j < 4; j++)
				for (int k = 0; k < 4; k++)
					tb_OptParameterXPos[i][j][k] = 0.0f; // Horizontal positions (display on line 2 of DMD). Default 0.0f, used will are set just below.
		// Tables used by Voltmeter mode.
		tb_OptionID[METRIKS_VOLTMETER][0] = "Metering";
		tb_ParamNumPerOpt[METRIKS_VOLTMETER][0] = 4;
		tb_OptParameter[METRIKS_VOLTMETER][0][0] = "Realtime";
		tb_OptParameterXPos[METRIKS_VOLTMETER][0][0] = 9.17f;
		tb_OptParameter[METRIKS_VOLTMETER][0][1] = "Minimum";
		tb_OptParameterXPos[METRIKS_VOLTMETER][0][1] = 12.94f;
		tb_OptParameter[METRIKS_VOLTMETER][0][2] = "Maximum";
		tb_OptParameterXPos[METRIKS_VOLTMETER][0][2] = 12.0f;
		tb_OptParameter[METRIKS_VOLTMETER][0][3] = "Median";
		tb_OptParameterXPos[METRIKS_VOLTMETER][0][3] = 18.6f;
		tb_OptionID[METRIKS_VOLTMETER][1] = "Decimals";
		tb_ParamNumPerOpt[METRIKS_VOLTMETER][1] = 4;
		tb_OptParameter[METRIKS_VOLTMETER][1][0] = "2";
		tb_OptParameterXPos[METRIKS_VOLTMETER][1][0] = 41.19f;
		tb_OptParameter[METRIKS_VOLTMETER][1][1] = "3";
		tb_OptParameterXPos[METRIKS_VOLTMETER][1][1] = 41.19f;
		tb_OptParameter[METRIKS_VOLTMETER][1][2] = "0";
		tb_OptParameterXPos[METRIKS_VOLTMETER][1][2] = 41.19f;
		tb_OptParameter[METRIKS_VOLTMETER][1][3] = "1";
		tb_OptParameterXPos[METRIKS_VOLTMETER][1][3] = 41.19f;
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
		// Tables used by CV Tuner mode.
		tb_OptionID[METRIKS_CVTUNER][0] = "Notation";
		tb_ParamNumPerOpt[METRIKS_CVTUNER][0] = 2;
		tb_OptParameter[METRIKS_CVTUNER][0][0] = "C-D-E...B";
		tb_OptParameterXPos[METRIKS_CVTUNER][0][0] = 4.466f;
		tb_OptParameter[METRIKS_CVTUNER][0][1] = "Do-Re-Mi";
		tb_OptParameterXPos[METRIKS_CVTUNER][0][1] = 5.408f;
		tb_OptParameter[METRIKS_CVTUNER][0][2] = ""; // Not used.
		tb_OptParameter[METRIKS_CVTUNER][0][3] = ""; // Not used.
		tb_OptionID[METRIKS_CVTUNER][1] = "Sharps/Flats";
		tb_ParamNumPerOpt[METRIKS_CVTUNER][1] = 2;
		tb_OptParameter[METRIKS_CVTUNER][1][0] = "Sharps #";
		tb_OptParameterXPos[METRIKS_CVTUNER][1][0] = 6.35f;
		tb_OptParameter[METRIKS_CVTUNER][1][1] = "Flats b";
		tb_OptParameterXPos[METRIKS_CVTUNER][1][1] = 12.94f;
		tb_OptParameter[METRIKS_CVTUNER][1][2] = ""; // Not used.
		tb_OptParameter[METRIKS_CVTUNER][1][3] = ""; // Not used.
		tb_OptionID[METRIKS_CVTUNER][2] = ""; // Not used.
		tb_ParamNumPerOpt[METRIKS_CVTUNER][2] = 1; // Not used.
		tb_OptParameter[METRIKS_CVTUNER][2][0] = ""; // Not used.
		tb_OptParameter[METRIKS_CVTUNER][2][1] = ""; // Not used.
		tb_OptParameter[METRIKS_CVTUNER][2][2] = ""; // Not used.
		tb_OptParameter[METRIKS_CVTUNER][2][3] = ""; // Not used.
		tb_OptionID[METRIKS_CVTUNER][3] = ""; // Not used.
		tb_ParamNumPerOpt[METRIKS_CVTUNER][3] = 0;
		tb_OptParameter[METRIKS_CVTUNER][3][0] = ""; // Not used.
		tb_OptParameter[METRIKS_CVTUNER][3][1] = ""; // Not used.
		tb_OptParameter[METRIKS_CVTUNER][3][2] = ""; // Not used.
		tb_OptParameter[METRIKS_CVTUNER][3][3] = ""; // Not used.
		// Tables used by Frequency Counter mode.
		tb_OptionID[METRIKS_FREQCOUNTER][0] = "Analys. Mode";
		tb_ParamNumPerOpt[METRIKS_FREQCOUNTER][0] = 4;
		tb_OptParameter[METRIKS_FREQCOUNTER][0][0] = "V > Freq.";
		tb_OptParameterXPos[METRIKS_FREQCOUNTER][0][0] = 3.525f;
		tb_OptParameter[METRIKS_FREQCOUNTER][0][1] = "0-Cross.";
		tb_OptParameterXPos[METRIKS_FREQCOUNTER][0][1] = 8.23f;
		tb_OptParameter[METRIKS_FREQCOUNTER][0][2] = "Interpolt.";
		tb_OptParameterXPos[METRIKS_FREQCOUNTER][0][2] = 1.64f;
		tb_OptParameter[METRIKS_FREQCOUNTER][0][3] = "Advanced";
		tb_OptParameterXPos[METRIKS_FREQCOUNTER][0][3] = 7.29f;
		tb_OptionID[METRIKS_FREQCOUNTER][1] = ""; // Not used.
		tb_ParamNumPerOpt[METRIKS_FREQCOUNTER][1] = 0;
		tb_OptParameter[METRIKS_FREQCOUNTER][1][0] = ""; // Not used.
		tb_OptParameter[METRIKS_FREQCOUNTER][1][1] = ""; // Not used.
		tb_OptParameter[METRIKS_FREQCOUNTER][1][2] = ""; // Not used.
		tb_OptParameter[METRIKS_FREQCOUNTER][1][3] = ""; // Not used.
		tb_OptionID[METRIKS_FREQCOUNTER][2] = ""; // Not used.
		tb_ParamNumPerOpt[METRIKS_FREQCOUNTER][2] = 0;
		tb_OptParameter[METRIKS_FREQCOUNTER][2][0] = ""; // Not used.
		tb_OptParameter[METRIKS_FREQCOUNTER][2][1] = ""; // Not used.
		tb_OptParameter[METRIKS_FREQCOUNTER][2][2] = ""; // Not used.
		tb_OptParameter[METRIKS_FREQCOUNTER][2][3] = ""; // Not used.
		tb_OptionID[METRIKS_FREQCOUNTER][3] = ""; // Not used.
		tb_ParamNumPerOpt[METRIKS_FREQCOUNTER][3] = 0;
		tb_OptParameter[METRIKS_FREQCOUNTER][3][0] = ""; // Not used.
		tb_OptParameter[METRIKS_FREQCOUNTER][3][1] = ""; // Not used.
		tb_OptParameter[METRIKS_FREQCOUNTER][3][2] = ""; // Not used.
		tb_OptParameter[METRIKS_FREQCOUNTER][3][3] = ""; // Not used.
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
		// Get current engine sample rate.
		onSampleRateChange();
		// Set up frequencies tables for CV Tuner mode.
		setTunerFreqTables();
	}

	// Invoked (as event) from Initialize command via module's context menu (also Ctrl+I, Command+I on Macinthosh) to reset the module.
	// Model (Theme) isn't affected by module reset, however.
	void onReset() override {
		// Current parameters (and their old/previous states) reset to default values (0).
		for (int i = 0; i < METRIKS_NUM_MODES; i++)
			for (int j = 0; j < 4; j++) {
				if ((i == METRIKS_PEAKCOUNTER) && (j == 0)) {
					_currentParameter[METRIKS_PEAKCOUNTER][0] = 17; // Set treshold voltage to default 1.7V (for Peak Counter mode).
					currentParameter[METRIKS_PEAKCOUNTER][0] = 17; // Set treshold voltage to default 1.7V (for Peak Counter mode).
				}
				else {
					_currentParameter[i][j] = 0; // All options to default.
					currentParameter[i][j] = 0; // All options to default.
				}
				setMetriksParameters(i, j);
			}
		_Mode = Mode;
		b_InopMode = false; // TEMPORARY - false means the mode is operational (totally or partially) - MUST BE REMOVED WHEN ALL MODES WORK.
		bChangingMode = false;
		ct_SwitchedMode = 0;
		bChangingOption = false;
		currentOptionID = -1; // Option ID (-1 while not option edit).
		ct_OptionTimeout = 0; // Used as timer for current option (as time out).
		ct_OptionBlinkTimer = 0;
		lights[LED_OPTIONS].setBrightness(0.0f);
		// Peak Counter isn't running.
		bPeakCounterIsPlaying = false;
		// Reset minimum, maximum and median voltages (voltmeter mode).
		f_VoltageMin = f_InVoltage;
		f_VoltageMax = f_InVoltage;
		f_VoltageMed = f_InVoltage;
		// By doing this, the second line of DMD will be refreshed.
		_f_InVoltage = f_InVoltage + 1.0f;
	}

	// Invoked (as event) when Engine's Sample rate is changed from VCV Rack menu.
	void onSampleRateChange() override {
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
			bChangingMode = false;
			ct_SwitchedMode = 0;
			bChangingOption = false;
			currentOptionID = -1; // Option ID (-1 while not option edit).
			ct_OptionTimeout = 0; // Used as timer for current option (as time out).
			ct_OptionBlinkTimer = 0;
			lights[LED_OPTIONS].setBrightness(0.0f);
			// Peak Counter isn't running.
			bPeakCounterIsPlaying = false;
			// By doing this, the second line of DMD will be refreshed.
			_f_InVoltage = f_InVoltage + 1.0f;
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
							 // 2 decimals (default).
							 vltmDecimals = 2;
						break;
						case 1:
							 // 3 decimals.
							 vltmDecimals = 3;
						break;
						case 2:
							 // 0 decimal.
							 vltmDecimals = 0;
						break;
						case 3:
							 // 1 decimal.
							 vltmDecimals = 1;
					}
				}
				break;
			case METRIKS_CVTUNER:
				// CV Tuner mode.
				// Notation or sharps/flats: update notes tables.
				makeNotesTables();
				break;
			case METRIKS_FREQCOUNTER:
				break;
			case METRIKS_BPMMETER:
				break;
			case METRIKS_PEAKCOUNTER:
				// Peak Counter mode.
				pcntTresholdVoltage = currentParameter[METRIKS_PEAKCOUNTER][0];
				if (pcntTresholdVoltage < 2)
					pcntTresholdVoltage = 2; // Set to minimum treshold voltage (0.2V) if below compliant.
					else if (pcntTresholdVoltage > 117)
						pcntTresholdVoltage = 117; // Set to maximum allowed treshold voltage (11.7V) if above compliant.
				f_pcntTresholdVoltage = (float)(pcntTresholdVoltage) / 10.0f;
		}
	}

	// Custom method to make current notes table, depending "Notation" and "Sharps/Flats" parameters (CV Tuner mode).
	void makeNotesTables() {
		if (currentParameter[METRIKS_CVTUNER][0] == 0) {
			// English (international) notation (C, D, E,...B).
			tb_OptionID[METRIKS_CVTUNER][2] = "A4 Pitch";
			if (currentParameter[METRIKS_CVTUNER][1] == 0) {
				// Sharps.
				tunerBaseNoteName[0] = "C";
				tunerBaseNoteName[1] = "C#";
				tunerBaseNoteName[2] = "D";
				tunerBaseNoteName[3] = "D#";
				tunerBaseNoteName[4] = "E";
				tunerBaseNoteName[5] = "F";
				tunerBaseNoteName[6] = "F#";
				tunerBaseNoteName[7] = "G";
				tunerBaseNoteName[8] = "G#";
				tunerBaseNoteName[9] = "A";
				tunerBaseNoteName[10] = "A#";
				tunerBaseNoteName[11] = "B";
			}
			else {
				// Flats.
				tunerBaseNoteName[0] = "C";
				tunerBaseNoteName[1] = "Db";
				tunerBaseNoteName[2] = "D";
				tunerBaseNoteName[3] = "Eb";
				tunerBaseNoteName[4] = "E";
				tunerBaseNoteName[5] = "F";
				tunerBaseNoteName[6] = "Gb";
				tunerBaseNoteName[7] = "G";
				tunerBaseNoteName[8] = "Ab";
				tunerBaseNoteName[9] = "A";
				tunerBaseNoteName[10] = "Bb";
				tunerBaseNoteName[11] = "B";
			}
		}
		else {
			// Do-Re-Mi (French/Italian) notation.
			tb_OptionID[METRIKS_CVTUNER][2] = "La4 Pitch";
			if (currentParameter[METRIKS_CVTUNER][1] == 0) {
				// Sharps.
				tunerBaseNoteName[0] = "Do";
				tunerBaseNoteName[1] = "Do#";
				tunerBaseNoteName[2] = "Re";
				tunerBaseNoteName[3] = "Re#";
				tunerBaseNoteName[4] = "Mi";
				tunerBaseNoteName[5] = "Fa";
				tunerBaseNoteName[6] = "Fa#";
				tunerBaseNoteName[7] = "Sol";
				tunerBaseNoteName[8] = "Sol#";
				tunerBaseNoteName[9] = "La";
				tunerBaseNoteName[10] = "La#";
				tunerBaseNoteName[11] = "Si";
			}
			else {
				// Flats.
				tunerBaseNoteName[0] = "Do";
				tunerBaseNoteName[1] = "Reb";
				tunerBaseNoteName[2] = "Re";
				tunerBaseNoteName[3] = "Mib";
				tunerBaseNoteName[4] = "Mi";
				tunerBaseNoteName[5] = "Fa";
				tunerBaseNoteName[6] = "Solb";
				tunerBaseNoteName[7] = "Sol";
				tunerBaseNoteName[8] = "Lab";
				tunerBaseNoteName[9] = "La";
				tunerBaseNoteName[10] = "Sib";
				tunerBaseNoteName[11] = "Si";
			}
		}
		// Final table construction, including octave.
		for (int i = 0; i < 132; i++)
			tunerNote[i] = tunerBaseNoteName[i % 12] + std::to_string((i / 12) - 1);
	}

	// This method prepares (precompute) frequencies tables, used by CV Tuner mode.
	void setTunerFreqTables() {
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

	// Custom method to prepare threshold voltage for display (2nd line).
	void setDisplayThresholdVoltage() {
		f_pcntTresholdVoltage = (float)(pcntTresholdVoltage / 10.0f);
		if (pcntTresholdVoltage < 100)
			dmdOffsetTextMain2 = 36.0f;
			else dmdOffsetTextMain2 = 24.0f;
		snprintf(dmdTextMain2, sizeof(dmdTextMain2), "%2.1fV", f_pcntTresholdVoltage);
	}

	// This function will search accurate note, from given frequency, from precomputed tables (best for CPU save, tables are computed on Ref. A4/La4 tuning change only).
	int getNotebyFreq(double freq) {
		int aPos = -1;
		int startPos = 0;
		bool b_IsAbove = false;
		if (freq > 369.0f)
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
						b_tunrMarkerVisible = false;
						dmdTunerMarkerPos = 0.0f;
						strcpy(dmdTunerMarker, " ");
					}
					else if ((freq >= tb_FreqNote_MP_LimL[aPos]) && (freq < tb_FreqNote_MP_LimH[aPos])) {
 						// Medium precision: display one marker only, either "<" or ">".
						b_tunrMarkerVisible = true;
						if (b_IsAbove) {
							dmdTunerMarkerPos = 2.6f;
							strcpy(dmdTunerMarker, "<");
						}
						else {
							dmdTunerMarkerPos = 90.0f;
							strcpy(dmdTunerMarker, ">");
						}
					}
					else {
 						// Near bounds (aka bad precision): display three markers, either "<<<" or ">>>", because frequency is near bound.
						b_tunrMarkerVisible = true;
						if (b_IsAbove) {
							dmdTunerMarkerPos = 2.6f;
							strcpy(dmdTunerMarker, "<<");
						}
						else {
							dmdTunerMarkerPos = 84.0f;
							strcpy(dmdTunerMarker, ">>");
						}
					}
					i = 132; // Exit note scan loop.
				}
			}
		}
		else if (freq < tb_FreqNote_LP_LimL[0]) {
			// Input frequency is too low (below C-1).
			b_tunrMarkerVisible = true;
			dmdTunerMarkerPos = 84.0f;
			strcpy(dmdTunerMarker, ">>");
		}
		else {
			// Input frequency is too high (above B9).
			b_tunrMarkerVisible = true;
			dmdTunerMarkerPos = 2.6f;
			strcpy(dmdTunerMarker, "<<");
		}
		return aPos;
	}

	// This function returns, for a given string, the number of px (for second line on the DMD).
	// Useful for centered display (line 2 only!).
	float getCenteredDMD(std::string sStr) {
		std::string pxChar;
		std::string searchInto;
		int px;
		bool bFound;
		px = 0;
		for (unsigned int i = 0; i < sStr.length(); i++) {
			pxChar = sStr.at(i);
			bFound = false;
			// 5 px chars...
			searchInto = "abcdefghknopqrstuxyz";
			if (searchInto.find(pxChar) != std::string::npos) {
				bFound = true;
				px = px + 5;
			}
			// 4 px chars...
			searchInto = "ijl<>[]\"";
			if (!bFound)
				if (searchInto.find(pxChar) != std::string::npos) {
					bFound = true;
					px = px + 4;
				}
			// 3 px chars...
			searchInto = "():.,";
			if (!bFound)
				if (searchInto.find(pxChar) != std::string::npos) {
					bFound = true;
					px = px + 3;
				}
			// 2 px chars...
			searchInto = "'!|";
			if (!bFound)
				if (searchInto.find(pxChar) != std::string::npos) {
					bFound = true;
					px = px + 2;
				}
			// Other else are 6 px...
			if (!bFound)
				px = px + 6;
		}
		px--;
		return ((105.8f - (11.3f * (float)(px) / 6.0f)) / 2.0f) - 7.0f;
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
		dmdOffsetTextMain2 = 1.0f;
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

		if (!b_dspIsRunning) {
			for (int i = 0; i < METRIKS_NUM_MODES; i++)
				for (int j = 0; j < 4; j++) {
					// Set up relevant mode/option.
					setMetriksParameters(i, j);
					_currentParameter[i][j] = currentParameter[i][j];
				}
			// By doing this, the second line of DMD will be refreshed.
			_f_InVoltage = f_InVoltage + 1.0f;
			b_dspIsRunning = true; // Yes, DSP is running...
		}

		// Depending current Metriks model (theme), set the relevant DMD-text color.
		DMDtextColor = tblDMDtextColor[Theme];

		// TEMPORARY - used for inoperative mode(s) - MUST BE REMOVED WHEN ALL MODES WORK.
		if (b_InopMode)
			if (!bChangingMode && !bChangingOption)
				runInopMode();

		// Is IN (INput) jack connected?
		bActiveINjack = inputs[INPUT_SOURCE].isConnected();

		// Transmit (as passthrough/daisy chain) current voltage on IN jack, to OUT jack. 0V will be sent as long as IN jack remains disconnected!
		if (bActiveINjack)
			outputs[OUTPUT_THRU].setVoltage(inputs[INPUT_SOURCE].getVoltage());
			else outputs[OUTPUT_THRU].setVoltage(0.0f);

		// Read if the continuous encoder is moved...
		encoderParam = (int)roundf(10.0f * params[PARAM_ENCODER].getValue());
		if (encoderParam != _encoderParam) {
			if (abs(encoderParam - _encoderParam) <= 2) {
				if (encoderParam > _encoderParam) {
					// Clockwise move (increment).
					if (!bChangingOption) {
						// Can change current mode.
						Mode++;
						if (Mode > 4)
							Mode = 0; // Returning to first mode (aka "Voltmeter").
						_Mode = Mode; // Done by encoder: backup to old/previous state variable.
						ct_SwitchedMode = (int)(1.0f * sampleRate);
						bChangingMode = true;
					}
					else {
						ct_OptionTimeout = (int)(10.0f * sampleRate); // Restart timeout for another 5 seconds when encoder is moved.
						if (Mode == METRIKS_PEAKCOUNTER) {
							// Incrementing treshold voltage by 0.1...
							pcntTresholdVoltage++;
							if (pcntTresholdVoltage > 117)
								pcntTresholdVoltage = 117; // Maximum treshold voltage: 11.7V.
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
					if (!bChangingOption) {
						// Can change current mode.
						Mode--;
						if (Mode < 0)
							Mode = 4; // Returning to last mode (aka "Peak Counter").
						_Mode = Mode; // Done by encoder: backup to old/previous state variable.
						ct_SwitchedMode = (int)(1.0f * sampleRate);
						bChangingMode = true;
					}
					else {
						ct_OptionTimeout = (int)(10.0f * sampleRate); // Restart timeout for another 5 seconds when encoder is moved.
						if (Mode == METRIKS_PEAKCOUNTER) {
							// Decrementing treshold voltage by 0.1...
							pcntTresholdVoltage--;
							if (pcntTresholdVoltage < 2)
								pcntTresholdVoltage = 2; // Minimum treshold voltage: 0.2V.
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

		if (bChangingMode) {
			if (ct_SwitchedMode > 0) {
				ct_SwitchedMode--;
				bChangingOption = false;
				ct_OptionTimeout = 0;
				ct_OptionBlinkTimer = 0;
				currentOptionID = 0;
				b_tunrMarkerVisible = false; // To avoid "marker(s)" displayed on DMD!
				// Display new mode for a given delay.
				strcpy(dmdTextMain1, "Switch to...");
				switch (Mode) {
					case 0:
						// Voltmeter.
						dmdOffsetTextMain2 = 3.52f; // Centered "Voltmeter" message on second line of DMD.
						strcpy(dmdTextMain2, "Voltmeter");
						break;
					case 1:
						// CV tuner.
						dmdOffsetTextMain2 = 5.408f; // Centered "CV Tun." message on second line of DMD.
						strcpy(dmdTextMain2, "CV Tuner");
						break;
					case 2:
						// Frequency counter.
						dmdOffsetTextMain2 = 0.7f; // Centered "Freq. Cnt." message on second line of DMD.
						strcpy(dmdTextMain2, "Freq. Cnt.");
						break;
					case 3:
						// BPM Meter.
						dmdOffsetTextMain2 = -0.24f;
						strcpy(dmdTextMain2, "BPM Meter"); // Centered "BPM Meter" message on second line of DMD.
						break;
					case 4:
						// Peak counter.
						dmdOffsetTextMain2 = 3.52f; // Centered "Peak Cnt." message on second line of DMD.
						strcpy(dmdTextMain2, "Peak Cnt.");
						break;
				}
				return; // while changing mode, do nothing else!
			}
			else {
				// Exit point for changing mode.
				b_tunrMarkerVisible = false; // To avoid "marker(s)" displayed on DMD!
				_f_InVoltage = f_InVoltage + 1.0f; // By doing this, the second line of DMD will be refreshed.
				b_InopMode = false; // TEMPORARY - false means the mode is operational (totally or partially) - MUST BE REMOVED WHEN ALL MODES WORK.
				bChangingMode = false;
			}
		}

		// OPT. (options) button trigger: enter option for current mode, or select next option, or quit option after last option.
		if (optButton.process(params[BUTTON_OPTIONS].getValue())) {
			optButton.reset();
			if (!bChangingMode) {
				ct_OptionTimeout = (int)(10.0f * sampleRate); // Arming option timer for 5 seconds (or give additional 5 seconds).
				if (bChangingOption) {
					currentOptionID++;
					if (currentOptionID > tb_OptionNumPerMode[Mode] - 1) {
						// It was the last option, now exit option(s) for current mode.
						currentOptionID = 0;
						lights[LED_OPTIONS].setBrightness(0.0f);
						bChangingOption = false; // Exit options.
						b_InopMode = false; // TEMPORARY - false means the mode is operational (totally or partially) - MUST BE REMOVED WHEN ALL MODES WORK.
						_f_InVoltage = f_InVoltage + 1.0f; // By doing this, the second line of DMD will be refreshed.
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
						bChangingOption = true;
						b_InopMode = false; // TEMPORARY - false means the mode is operational (totally or partially) - MUST BE REMOVED WHEN ALL MODES WORK.
						lights[LED_OPTIONS].setBrightness(1.0f);
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
		}

		// RESET button and/or input jack:
		// - Voltmeter mode: reset Min, Max and Med voltages.
		// - Peak Counter: reset the counter. 
		if (resetButton.process(params[BUTTON_RESET].getValue()) || resetPort.process(rescale(inputs[INPUT_RESET].getVoltage(), 0.2f, 1.7f, 0.0f, 1.0f))) {
			resetButton.reset();
			switch (Mode) {
				case METRIKS_VOLTMETER:
					// Reset Min, Max and median registered voltages.
					f_VoltageMin = f_InVoltage;
					f_VoltageMax = f_InVoltage;
					f_VoltageMed = f_InVoltage;
					_f_InVoltage = f_InVoltage + 1.0f; // By doing this, the second line of DMD will be refreshed.
//					break;
//				case METRIKS_PEAKCOUNTER:
					// ToDo...
//					break;
			}
		}


		if (bChangingOption) {
			if (ct_OptionTimeout > 0) {
				int kBlinkSpeedFactor = 1;
				ct_OptionTimeout--;
				if (ct_OptionBlinkTimer > 0)
					ct_OptionBlinkTimer--;
					else ct_OptionBlinkTimer = (int)(sampleRate / kBlinkSpeedFactor);
				if (ct_OptionTimeout < (int)(sampleRate * 2.0f))
					kBlinkSpeedFactor = 5;
				_tmpString = tb_OptionID[Mode][currentOptionID];
				strcpy(dmdTextMain1, _tmpString.c_str());
				if ((ct_OptionBlinkTimer % (int)(sampleRate / kBlinkSpeedFactor)) < (int)(sampleRate / kBlinkSpeedFactor / 2)) {
					lights[LED_OPTIONS].setBrightness(0.0f);
					strcpy(dmdTextMain2, "");
				}
				else {
					lights[LED_OPTIONS].setBrightness(1.0f);
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
				lights[LED_OPTIONS].setBrightness(0.0f);
				b_tunrMarkerVisible = false; // To avoid "marker(s)" displayed on DMD!
				_f_InVoltage = f_InVoltage + 1.0f; // By doing this, the second line of DMD will be refreshed.
				currentOptionID = 0;
				ct_OptionBlinkTimer = 0;
				bChangingOption = false;
				b_InopMode = false; // TEMPORARY - false means the mode is operational (totally or partially) - MUST BE REMOVED WHEN ALL MODES WORK.
			}
			return;
		}
		else {
		// Display current mode on first line of DMD (if not currently changing an option for current mode).
			switch (Mode) {
				case 0:
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
				case 1:
					strcpy(dmdTextMain1, "CV Tuner");
					break;
				case 2:
					strcpy(dmdTextMain1, "Freq. Counter");
					break;
				case 3:
					strcpy(dmdTextMain1, "BPM Meter");
					break;
				case 4:
					strcpy(dmdTextMain1, "Peak Counter");
			}
		}

		if (bActiveINjack != _bActiveINjack) {
			// Input jack state was changed from connected to disconnected, and vice-versa).
			b_tunrMarkerVisible = false; // To avoid "marker(s)" displayed on DMD!
			_f_InVoltage = f_InVoltage + 1.0f; // By doing this, the second line of DMD will be refreshed.
			_bActiveINjack = bActiveINjack;
		}

		if (bActiveINjack) {
			f_InVoltage = inputs[INPUT_SOURCE].getVoltage();
			if (f_InVoltage > f_VoltageMax)
				f_VoltageMax = f_InVoltage;
				else if (f_InVoltage < f_VoltageMin)
					f_VoltageMin = f_InVoltage;
			f_VoltageMed = (f_VoltageMax + f_VoltageMin) / 2.0f;
			ct_NoInputTimer = 0;
		}
		else {
			// Input jack isn't connected...
			f_InVoltage = 0.0f;
			_f_InVoltage = 1.0f;
			f_VoltageMin = 99999.0f;
			f_VoltageMax = -99999.0f;
			f_VoltageMed = 0.0f;
			// Be sure Peak Counter is stopped. Unlit PLAY/PAUSE bi-colored LED.
			lights[LED_PLAY_GREEN].setBrightness(0.0f);
			lights[LED_PLAY_RED].setBrightness(0.0f);
			bPeakCounterIsPlaying = false;
			// Blinking "? Input ?" message on second line of the DMD.
			if (ct_NoInputTimer > 0)
				ct_NoInputTimer--;
				else ct_NoInputTimer = (int)(sampleRate / 5.0f);
			dmdOffsetTextMain2 = -1.0f;
			if (ct_NoInputTimer % (int)(sampleRate / 5.0f) < (int)(sampleRate / 10.0f))
				strcpy(dmdTextMain2, "? Input ?");
				else strcpy(dmdTextMain2, "");
		}

		if (bActiveINjack) {
			// Do possible measurement while IN jack is connected!
			switch (Mode) {
				case METRIKS_VOLTMETER:
					// Voltmeter mode implementation.
					b_InopMode = false; // TEMPORARY - false means the mode is operational (totally or partially) - MUST BE REMOVED WHEN ALL MODES WORK.
					b_tunrMarkerVisible = false; // To avoid "marker(s)" displayed on DMD!
					// Be sure Peak Counter is stopped. Unlit PLAY/PAUSE bi-colored LED.
					lights[LED_PLAY_GREEN].setBrightness(0.0f);
					lights[LED_PLAY_RED].setBrightness(0.0f);
					bPeakCounterIsPlaying = false;
					if (f_InVoltage != _f_InVoltage) {
						// Display voltage, but if it was changed only! Also, if number of decimal(s) option was changed.
						float vFloor;
						float vCeiling;
						std::string vSign = "+";
						std::string vMask = "";
						float currentVoltage = 0.0f;
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
						if (currentVoltage < 0.0f)
							vSign = "-";
						currentVoltage = abs(currentVoltage);
						switch (vltmDecimals) {
							case 0:
								vFloor = -99999.0f;
								vCeiling = 99999.0f;
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
								vFloor = -99.9999f;
								vCeiling = 99.9999f;
								break;
						}
						if ((currentVoltage >= vFloor) && (currentVoltage <= vCeiling)) {
							_tmpString = std::to_string(currentVoltage);
							int vPos = _tmpString.find(".");
							if (vltmDecimals == 0)
								vPos--;
							_tmpString = _tmpString.substr (0, vPos + vltmDecimals + 1);
							_tmpString = vSign + _tmpString + "V";
							if (vltmDecimals == 0)
								dmdOffsetTextMain2 = 96.0f - (_tmpString.length() * 12.0f);
								else dmdOffsetTextMain2 = 102.0f - (_tmpString.length() * 12.0f);
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
				case METRIKS_CVTUNER:
					// CV Tuner mode implementation.
					b_InopMode = false; // TEMPORARY - false means the mode is operational (totally or partially) - MUST BE REMOVED WHEN ALL MODES WORK.
					// Be sure Peak Counter is stopped. Unlit PLAY/PAUSE bi-colored LED.
					lights[LED_PLAY_GREEN].setBrightness(0.0f);
					lights[LED_PLAY_RED].setBrightness(0.0f);
					bPeakCounterIsPlaying = false;
					if (f_InVoltage != _f_InVoltage) {
						// Doing note search by voltage (CV), but only if voltage has changed!
						_f_InVoltage = f_InVoltage;
						int x = getNotebyFreq(dsp::FREQ_C4 * (double)(pow(2.0, f_InVoltage)));
						if (x != -1)
							_tmpString = tunerNote[x];
							else _tmpString = "?";
						dmdOffsetTextMain2 = getCenteredDMD(_tmpString); // Centered display on second line.
						strcpy(dmdTextMain2, _tmpString.c_str());
					}
					break;
				case METRIKS_FREQCOUNTER:
					// Frequency counter mode implementation.
					b_tunrMarkerVisible = false; // To avoid "marker(s)" displayed on DMD!
					// Be sure Peak Counter is stopped. Unlit PLAY/PAUSE bi-colored LED.
					lights[LED_PLAY_GREEN].setBrightness(0.0f);
					lights[LED_PLAY_RED].setBrightness(0.0f);
					bPeakCounterIsPlaying = false;
					// TEMPORARY - used for inoperative mode(s) - MUST BE REMOVED WHEN ALL MODES WORK.
					setInopMode();
					break;
				case METRIKS_BPMMETER:
					// BPM meter mode implementation.
					b_tunrMarkerVisible = false; // To avoid "marker(s)" displayed on DMD!
					// Be sure Peak Counter is stopped. Unlit PLAY/PAUSE bi-colored LED.
					lights[LED_PLAY_GREEN].setBrightness(0.0f);
					lights[LED_PLAY_RED].setBrightness(0.0f);
					bPeakCounterIsPlaying = false;
					// TEMPORARY - used for inoperative mode(s) - MUST BE REMOVED WHEN ALL MODES WORK.
					setInopMode();
					break;
				case METRIKS_PEAKCOUNTER:
					// Peak counter mode implementation.
					b_tunrMarkerVisible = false; // To avoid "marker(s)" displayed on DMD!
					// TEMPORARY - used for inoperative mode(s) - MUST BE REMOVED WHEN ALL MODES WORK.
					setInopMode();
					break;
			}
		}

	} // end of "process"...

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Theme", json_integer(Theme)); // Save selected module theme (GUI).
		json_object_set_new(rootJ, "Mode", json_integer(Mode)); // Save current mode.
		json_object_set_new(rootJ, "lastVMin", json_real(f_VoltageMin)); // Save lastest min. voltage.
		json_object_set_new(rootJ, "lastVMax", json_real(f_VoltageMax)); // Save lastest max. voltage.
		// Preset management: check if mode have suddently changed (without encoder usage).
		if (b_dspIsRunning)
			if (Mode != _Mode) {
				_Mode = Mode;
			}
		// All parameters per mode and, per option (two-dimension array of integers).
		json_t *optionsJ = json_array();
		for (int i = 0; i < METRIKS_NUM_MODES; i++)
			for (int j = 0; j < 4; j++) {
				json_array_insert_new(optionsJ, (4 * i) + j, json_integer(currentParameter[i][j]));
				if (b_dspIsRunning) {
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
		// Retrieving saved theme (model) when loading .vcv, .vcvm or cloning module.
		json_t *ThemeJ = json_object_get(rootJ, "Theme");
		if (ThemeJ) {
			Theme = json_integer_value(ThemeJ);
			if ((Theme < 0) || (Theme > 5))
				Theme = 0; // Set to Classic model if not compliant.
		}
		else Theme = 0; // Default Classic model.
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
		else f_VoltageMin = 99999.0f;
		// Retrieving registered last maximum voltage.
		json_t *lastVMaxJ = json_object_get(rootJ, "lastVMax");
		if (lastVMaxJ) {
			f_VoltageMax = json_real_value(lastVMaxJ);
		}
		else f_VoltageMax = -99999.0f;
		// Now we can define last median voltage (never stored, always computed).
		f_VoltageMed = (f_VoltageMax + f_VoltageMin) / 2.0f;
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
								currentParameter[i][j] = 17; // Default treshold voltage (Peak Counter mode) set to 1.7V, stored as integer 17 (value x 10).
								else currentParameter[i][j] = 0; // Default 0 for others.
						}
				}
			}
		}
		else {
			for (int i = 0; i < METRIKS_NUM_MODES; i++)
				for (int j = 0; j < 4; j++) {
					if ((i == METRIKS_PEAKCOUNTER) && (j == 0))
						currentParameter[i][j] = 17; // Default treshold voltage (Peak Counter mode) set to 1.7V.
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

		if (!(font = APP->window->loadFont(fontPath))) {
				return;
		}

		// Main DMD, upper line.
		nvgFontSize(args.vg, 16);
		nvgFontFaceId(args.vg, font->handle);
		nvgTextLetterSpacing(args.vg, -2);
		Vec textPos = Vec(14, box.size.y - 174);
		if (!module) {
			// Required as "module preview" (from VCV Rack modules browser).
			// Default message on DMD (LCD).
			nvgFillColor(args.vg, nvgTransRGBA(nvgRGB(0x08, 0x08, 0x08), 0xff)); // Using default black LCD.
			nvgText(args.vg, textPos.x, textPos.y, "Voltmeter", NULL); // Default message on first line (Voltmeter, the default mode).
			// Main DMD, lower line.
			nvgFontSize(args.vg, 20);
			nvgTextLetterSpacing(args.vg, -1);
			textPos = Vec(12, box.size.y - 152);
			nvgText(args.vg, textPos.x + 26, textPos.y, "+3.14V", NULL); // Default message on second line (display fictious voltage).
			return; // Exit method immediatly (code below will be ignored).
		}
		nvgFillColor(args.vg, nvgTransRGBA(module->DMDtextColor, 0xff)); // Using current color for DMD.
		nvgText(args.vg, textPos.x, textPos.y, module->dmdTextMain1, NULL); // Proceeding module->dmdTextMain2 string (second line).
		// Main DMD, lower line.
		nvgFontSize(args.vg, 20);
		nvgTextLetterSpacing(args.vg, -1);
		textPos = Vec(12, box.size.y - 152);
		nvgText(args.vg, textPos.x + module->dmdOffsetTextMain2, textPos.y, module->dmdTextMain2, NULL); // Displaying module->dmdTextMain2 string (second line). The second line may have an horizontal offset.
		// CV Tuner (Mode = 1) only from this point.
		if (module->Mode != 1)
			return; // Exit immediatly (code below will be ignored) if current mode isn't "CV Tuner".
		if (module->bChangingMode || module->bChangingOption)
			return; // Exit immediatly if currently changing mode or changing option.
		if (!module->b_tunrMarkerVisible)
			return; // Flag "module->b_tunrMarkerVisible" is false, don't display marker(s): exit method immediatly.
		// Display marker(s) on the DMD.
		nvgFontSize(args.vg, 14);
		nvgTextLetterSpacing(args.vg, -1);
		textPos = Vec(12.0f, box.size.y - 154);
		nvgText(args.vg, textPos.x + module->dmdTunerMarkerPos, textPos.y, module->dmdTunerMarker, NULL);
	}

};

///////////////////////////////////////////////////// CONTEXT-MENU //////////////////////////////////////////////////////

struct MetriksClassicMenu : MenuItem {
	MetriksModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 0; // Model: default Classic (beige).
		module->portMetal = 0; // Silver connectors for Classic.
	}
};

struct MetriksStageReproMenu : MenuItem {
	MetriksModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 1; // Model: Stage Repro.
		module->portMetal = 0; // Silver connectors for Stage Repro.
	}
};

struct MetriksAbsoluteNightMenu : MenuItem {
	MetriksModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 2; // Model: Absolute Night.
		module->portMetal = 0; // Silver connectors for Absolute Night.
	}
};

struct MetriksDarkSignatureMenu : MenuItem {
	MetriksModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 3; // Model: Dark Signature.
		module->portMetal = 1; // Gold connectors for Dark Signature.
	}
};

struct MetriksDeepblueSignatureMenu : MenuItem {
	MetriksModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 4; // Model: Deepblue Signature.
		module->portMetal = 1; // Gold connectors for Deepblue Signature.
	}
};

struct MetriksCarbonSignatureMenu : MenuItem {
	MetriksModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 5; // Model: Carbon Signature.
		module->portMetal = 1; // Gold connectors for Carbon Signature.
	}
};

struct MetriksSubMenuItems : MenuItem {
	MetriksModule *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		MetriksClassicMenu *metriksmenuitem1 = new MetriksClassicMenu;
		metriksmenuitem1->text = "Classic (default)";
		metriksmenuitem1->rightText = CHECKMARK(module->Theme == 0);
		metriksmenuitem1->module = module;
		menu->addChild(metriksmenuitem1);

		MetriksStageReproMenu *metriksmenuitem2 = new MetriksStageReproMenu;
		metriksmenuitem2->text = "Stage Repro";
		metriksmenuitem2->rightText = CHECKMARK(module->Theme == 1);
		metriksmenuitem2->module = module;
		menu->addChild(metriksmenuitem2);

		MetriksAbsoluteNightMenu *metriksmenuitem3 = new MetriksAbsoluteNightMenu;
		metriksmenuitem3->text = "Absolute Night";
		metriksmenuitem3->rightText = CHECKMARK(module->Theme == 2);
		metriksmenuitem3->module = module;
		menu->addChild(metriksmenuitem3);

		MetriksDarkSignatureMenu *metriksmenuitem4 = new MetriksDarkSignatureMenu;
		metriksmenuitem4->text = "Dark \"Signature\"";
		metriksmenuitem4->rightText = CHECKMARK(module->Theme == 3);
		metriksmenuitem4->module = module;
		menu->addChild(metriksmenuitem4);

		MetriksDeepblueSignatureMenu *metriksmenuitem5 = new MetriksDeepblueSignatureMenu;
		metriksmenuitem5->text = "Deepblue \"Signature\"";
		metriksmenuitem5->rightText = CHECKMARK(module->Theme == 4);
		metriksmenuitem5->module = module;
		menu->addChild(metriksmenuitem5);

		MetriksCarbonSignatureMenu *metriksmenuitem6 = new MetriksCarbonSignatureMenu;
		metriksmenuitem6->text = "Carbon \"Signature\"";
		metriksmenuitem6->rightText = CHECKMARK(module->Theme == 5);
		metriksmenuitem6->module = module;
		menu->addChild(metriksmenuitem6);

		return menu;
	}

};

///////////////////////////////////////////////// MODULE WIDGET SECTION /////////////////////////////////////////////////

struct MetriksWidget : ModuleWidget {
	// Panels (one per "Theme").
	SvgPanel *panelMetriksClassic;
	SvgPanel *panelMetriksStageRepro;
	SvgPanel *panelMetriksAbsoluteNight;
	SvgPanel *panelMetriksDarkSignature;
	SvgPanel *panelMetriksDeepBlueSignature;
	SvgPanel *panelMetriksCarbonSignature;
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
		// Classic (default) beige panel.
		panelMetriksClassic = new SvgPanel();
		panelMetriksClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Metriks_Classic.svg")));
		panelMetriksClassic->visible = true;
		addChild(panelMetriksClassic);
		// Stage Repro panel.
		panelMetriksStageRepro = new SvgPanel();
		panelMetriksStageRepro->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Metriks_Stage_Repro.svg")));
		panelMetriksStageRepro->visible = false;
		addChild(panelMetriksStageRepro);
		// Absolute Night panel.
		panelMetriksAbsoluteNight = new SvgPanel();
		panelMetriksAbsoluteNight->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Metriks_Absolute_Night.svg")));
		panelMetriksAbsoluteNight->visible = false;
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
		// Deepblue Signature panel.
		panelMetriksCarbonSignature = new SvgPanel();
		panelMetriksCarbonSignature->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Metriks_Carbon_Signature.svg")));
		panelMetriksCarbonSignature->visible = false;
		addChild(panelMetriksCarbonSignature);
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
		// Input ports (ports are using "dynamic ports" to change connector metal - silver or gold: big thanks to Marc Boulé!).
		addInput(createDynamicPort<DynSVGPort>(Vec(24, 304), true, module, MetriksModule::INPUT_SOURCE, module ? &module->portMetal : NULL));
		addInput(createDynamicPort<DynSVGPort>(Vec(24, 262), true, module, MetriksModule::INPUT_PLAYPAUSE, module ? &module->portMetal : NULL));
		addInput(createDynamicPort<DynSVGPort>(Vec(72, 262), true, module, MetriksModule::INPUT_RESET, module ? &module->portMetal : NULL));
		// Output port (ports are using "dynamic ports" to change connector metal - silver or gold: big thanks to Marc Boulé!).
		addOutput(createDynamicPort<DynSVGPort>(Vec(72, 304), false, module, MetriksModule::OUTPUT_THRU, module ? &module->portMetal : NULL));
		// Multipurpose continuous encoder (used to select BPM, modulator ratio or setup items).
		addParam(createParam<KS_Encoder>(Vec(20, 106), module, MetriksModule::PARAM_ENCODER));
		// Gold push button to select option.
		buttonOptionsGold = createParam<KS_ButtonGold>(Vec(94, 178), module, MetriksModule::BUTTON_OPTIONS);
		addParam(buttonOptionsGold);
		// Silver push button to select option.
		buttonOptionsSilver = createParam<KS_ButtonSilver>(Vec(94, 178), module, MetriksModule::BUTTON_OPTIONS);
		addParam(buttonOptionsSilver);
		// Gold push button to toggle PLAY/PAUSE.
		buttonPlayPauseGold = createParam<KS_ButtonGold>(Vec(27.4, 240), module, MetriksModule::BUTTON_PLAYPAUSE);
		addParam(buttonPlayPauseGold);
		// Silver push button to toggle PLAY/PAUSE.
		buttonPlayPauseSilver = createParam<KS_ButtonSilver>(Vec(27.4, 240), module, MetriksModule::BUTTON_PLAYPAUSE);
		addParam(buttonPlayPauseSilver);
		// Gold push button to toggle PLAY/PAUSE.
		buttonResetGold = createParam<KS_ButtonGold>(Vec(75.4, 240), module, MetriksModule::BUTTON_RESET);
		addParam(buttonResetGold);
		// Silver push button to toggle PLAY/PAUSE.
		buttonResetSilver = createParam<KS_ButtonSilver>(Vec(75.4, 240), module, MetriksModule::BUTTON_RESET);
		addParam(buttonResetSilver);
		// PLAY/STOP (bicolored green/red) LED.
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(18, 252), module, MetriksModule::LED_PLAY_GREEN)); // Unified PLAY/STOP LED (green/red).
		// OPTIONS (OPT.) red LED.
		addChild(createLight<MediumLight<RedLight>>(Vec(83, 200.5), module, MetriksModule::LED_OPTIONS)); // Unified PLAY/STOP LED (green/red).
	}

	void step() override {
		MetriksModule *module = dynamic_cast<MetriksModule*>(this->module);
		if (module) {
			// Possible alternate panel themes (GUIs).
			panelMetriksClassic->visible = (module->Theme == 0);
			panelMetriksStageRepro->visible = (module->Theme == 1);
			panelMetriksAbsoluteNight->visible = (module->Theme == 2);
			panelMetriksDarkSignature->visible = (module->Theme == 3);
			panelMetriksDeepBlueSignature->visible = (module->Theme == 4);
			panelMetriksCarbonSignature->visible = (module->Theme == 5);
			// Torx screws metal (silver, gold) are visible or hidden, depending selected model (from module's context-menu).
			// Silver Torx screws are visible only for non-"Signature" modules (Classic, Stage Repro or Absolute Night).
			topLeftScrewSilver->visible = (module->Theme < 3);
			topRightScrewSilver->visible = (module->Theme < 3);
			bottomLeftScrewSilver->visible = (module->Theme < 3);
			bottomRightScrewSilver->visible = (module->Theme < 3);
			// Gold Torx screws are visible only for "Signature" modules (Dark Signature, Deepblue Signature or Carbon Signature).
			topLeftScrewGold->visible = (module->Theme > 2);
			topRightScrewGold->visible = (module->Theme > 2);
			bottomLeftScrewGold->visible = (module->Theme > 2);
			bottomRightScrewGold->visible = (module->Theme > 2);
			// Silver buttons are visible for first three models (themes) non-Signature.
			buttonOptionsSilver->visible = (module->Theme < 3);
			buttonPlayPauseSilver->visible = (module->Theme < 3);
			buttonResetSilver->visible = (module->Theme < 3);
			// Gold buttons are visible for last three models (themes) aka "Signature"-line.
			buttonOptionsGold->visible = (module->Theme > 2);
			buttonPlayPauseGold->visible = (module->Theme > 2);
			buttonResetGold->visible = (module->Theme > 2);
		}
		else {
			// Default panel theme is always "Classic" (beige, using silver screws, silver button, silver jacks, black LCD).
			// Other panels are, of course, hidden.
			panelMetriksClassic->visible = true;
			panelMetriksStageRepro->visible = false;
			panelMetriksAbsoluteNight->visible = false;
			panelMetriksDarkSignature->visible = false;
			panelMetriksDeepBlueSignature->visible = false;
			panelMetriksCarbonSignature->visible = false;
			// By default, silver screws are visible for default beige Classic panel...
			topLeftScrewSilver->visible = true;
			topRightScrewSilver->visible = true;
			bottomLeftScrewSilver->visible = true;
			bottomRightScrewSilver->visible = true;
			// ...and, of course, golden screws are hidden.
			topLeftScrewGold->visible = false;
			topRightScrewGold->visible = false;
			bottomLeftScrewGold->visible = false;
			bottomRightScrewGold->visible = false;
			// By default Classic, silver buttons are visible...
			buttonOptionsSilver->visible = true;
			buttonPlayPauseSilver->visible = true;
			buttonResetSilver->visible = true;
			// ...and, of course, gold buttons are hidden...
			buttonOptionsGold->visible = false;
			buttonPlayPauseGold->visible = false;
			buttonResetGold->visible = false;
		}
		ModuleWidget::step();
	}

	void appendContextMenu(Menu *menu) override {
		MetriksModule *module = dynamic_cast<MetriksModule*>(this->module);
		menu->addChild(new MenuEntry);
		MetriksSubMenuItems *metrikssubmenuitems = new MetriksSubMenuItems;
		metrikssubmenuitems->text = "Model";
		metrikssubmenuitems->rightText = RIGHT_ARROW;
		metrikssubmenuitems->module = module;
		menu->addChild(metrikssubmenuitems);
	}

};

Model *modelMetriks = createModel<MetriksModule, MetriksWidget>("Metriks");
