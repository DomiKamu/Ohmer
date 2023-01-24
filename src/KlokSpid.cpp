//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// KlokSpid is a 8 HP module designed to divide/multiply an external clock frequency (clock modulator), ////
//// obviously it can work as standalone clock generator (BPM-based).                                     ////
//// - 2 input jacks:                                                                                     ////
////   - external clock (CLK), to work as divider/multiplier (standalone clock generator if not patched). ////
////   - multipurpose CV-RATIO/TRIG. jack:                                                                ////
////     - when running as clock multiplier/divider: CV-controllable ratio (full range /64 to x64).       ////
////     - when running as BPM-clock generator: trigger input (BPM start/stop - default, or BPM reset).   ////
//// - 4 output jacks: gates (default +5V/0V Square waveform). Other gates (%) and 1ms/2ms/5ms triggers   ////
////   (fixed duration pulses) are possible, via SETUP.                                                   ////
////   Outputs deliver unipolar 0 ~ +5V (default), 0 ~ +10V, 0 ~ +11.7V, or 0 ~ +2V voltage.              ////
////                                                                                                      ////
//// As standalone (BPM-based) clock generator only:                                                      ////
//// - any jack may have its custom ratio (via SETUP) - default is x1, for all jacks.                     ////
//// - when set as "Custom" for first time, proposed default ratios are, per jack: /4, x1, x2, and x4.    ////
//// - jack #4 can be set (via SETUP) to send LFO-based waveform (instead of square/pulse) @ x1 only.     ////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ohmer.hpp"

// Dedicated LFO (based on LFO-1 stuff from Fundamental, but simplified as required).
// It will be used - if enabled via SETUP - to output specific waveform to jack #4. Disabled by default.
// LFO can be enabled to jack #4 but only if this jack is set at default ratio x1.

struct LFO {
	float phase = 0.0f;
	float pw = 0.5f;
	float freq = 1.0f;
	bool offset = false;
	bool invert = false;

	LFO() {}

	void step(float dt) {
		float deltaPhase = fminf(freq * dt, 0.5f);
		phase += deltaPhase;
		if (phase >= 1.0f)
			phase -= 1.0f;
	}

	float sin() {
		if (offset)
			return 1.0f - cosf(2*M_PI * phase) * (invert ? -1.0f : 1.0f);
			else return sinf(2.0f*M_PI * phase) * (invert ? -1.0f : 1.0f);
	}

	float tri(float x) {
		return 4.0f * fabsf(x - roundf(x));
	}

	float tri() {
		if (offset)
			return tri(invert ? phase - 0.5f : phase);
			else return -1.0f + tri(invert ? phase - 0.25f : phase - 0.75f);
	}

	float saw(float x) {
		return 2.0f * (x - roundf(x));
	}

	float saw() {
		if (offset)
			return invert ? 2.0f * (1.0f - phase) : 2.0f * phase;
			else return saw(phase) * (invert ? -1.0f : 1.0f);
	}

};

// KlokSpid module architecture.
struct KlokSpidModule : Module {
	enum ParamIds {
		PARAM_ENCODER,
    PARAM_BUTTON,
		NUM_PARAMS
	};
	enum InputIds {
		INPUT_CLOCK,
		INPUT_CV_TRIG,
		NUM_INPUTS
	};
	enum OutputIds {
		OUTPUT_1,
		OUTPUT_2,
		OUTPUT_3,
		OUTPUT_4,
		NUM_OUTPUTS
	};
	enum LightIds {
		LED_CLK,
		LED_CV_TRIG,
		LED_CVMODE,
		LED_TRIGMODE,
		LED_SYNC_GREEN,
		LED_SYNC_RED,
		NUM_LIGHTS
	};

	//// SAMPLE RATE / SAMPLE TIME.

	float sampleRate = 44100.0f;
	float sampleTime = 1.0f / 44100.0f ;

	// Optional LFO for jack #4.
	LFO LFOjack4;

	//// GENERAL PURPOSE VARIABLES/FLAGS/TABLES.
	bool bEarlyRun = true;

	//// CLOCK MODULATOR RATIOS.

	// Real clock ratios (global) list/array. Preset ratios while KlokSpid module runs as clock modulator (can be selected via encoder exclusively).
	float list_fRatio[31] = {64.0f, 32.0f, 24.0f, 16.0f, 15.0f, 12.0f, 10.0f, 9.0f, 8.0f, 7.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.5f, 1.0f/3.0f, 0.25f, 0.2f, 1.0f/6.0f, 1.0f/7.0f, 0.125f, 1.0f/9.0f, 0.1f, 1.0f/12.0f, 1.0f/15.0f, 0.0625f, 1.0f/24.0f, 0.03125f, 0.015625f};

	//// MODEL (GUI THEME).

	// Current selected KlokSpid model (GUI theme).
	int Theme = 0; // 0 = Classic (default), 1 = Stage Repro, 2 = Absolute Night, 3 = Dark Signature, 4 = Deepblue Signature, 5 = Carbon Signature.
	int portMetal = 0; // 0 = silver connector (default), 1 = gold connector used by "Signature"-line models only.

	// DMD-font color (default is "Classic" beige model).
	NVGcolor DMDtextColor = nvgRGB(0x08, 0x08, 0x08);

	//// Main DMD and small displays (near output jacks).
	char dmdTextMain1[24] = "";
	char dmdTextMain2[24] = "";
	int dmdOffsetTextMain2 = 0; // Horizontal offset on DMD to display for second line.
	char dmdTextMainOut1[4] = "";
	int dmdOffsetTextOut1 = 0;
	char dmdTextMainOut2[4] = "";
	int dmdOffsetTextOut2 = 0;
	char dmdTextMainOut3[4] = "";
	int dmdOffsetTextOut3 = 0;
	char dmdTextMainOut4[4] = "";
	int dmdOffsetTextOut4 = 0;

	// Strings for running modes.
	const std::string runningMode[3] = {"Clk Generator", "Clk Modulator", "Clk CV-Ratio"};

	//// STEP-RELATED (REALTIME) COUNTERS/GAPS.

	// Step related variables: used to determine the frequency of source signal, and when KlokSpid must sends relevant pulses to output(s).
	long long int currentStep = 0;
	long long int previousStep = 0;
	long long int expectedStep = 0;
	long stepGap = 0;
	long stepGapPrevious = 0;
	long long int nextPulseStep[NUM_OUTPUTS] = {0, 0, 0, 0};

	// Current jacks states, voltages on input jacks, and button state.
	bool activeCLK = false;
	bool activeCLKPrevious = true;
	bool activeCV = false;
	bool activeCVPrevious = true;
	float voltageOnCV = 0.0f;
	bool buttonPressed = false;

	// Encoder (registered position to be used on next step for relative move).
	int encoderCurrent = 0;
	int encoderPrevious = 0; // Encoder "absolute" (saved to jSon)...
	int encoderDelta = 0; // 0 if not moved, -1 if counter-clockwise (decrement), 1 if clockwise (increment).

	// Ratio (clock modulator).
	int svRatio = 15; // saved value.
	int rateRatioByEncoder = 15; // Assuming encoder is, by default "centered" to "x1" (= 15).

	// Clock modulator modes.
	enum ClkModModeIds {
		X1,	// work at x1.
		DIV,	// divider mode.
		MULT	// muliplier mode.
	};

	// Clock modulator mode, assuming default is X1.
	int clkModulatorMode = X1;

	//// SCHMITT TRIGGERS.

	// Schmitt trigger to check thresholds on CLK input connector.
	dsp::SchmittTrigger CLKInputPort;
	// Schmitt trigger to handle BPM start/stop state (only when KlokSpid is acting as clock generator) via button.
	dsp::SchmittTrigger runButton;
	// Schmitt trigger to handle the start/stop toggle button (also used for SETUP to confirm menu/parameter) - via CV/TRIG input port (if configured as "Start/Stop").
	dsp::SchmittTrigger runTriggerPort;

	//// RATIO-BY-CV VARIABLES/FLAGS.

	// Incoming CV may be bipolar (true) or unipolar (false).
	bool bipolarCV = true;
	// Is CV used to modulate ratio?
	bool isRatioCVmod = false;
	// Real ratio, given by current CV voltage.
	float rateRatioCV = 0.0f;
	// Real ratio, given by current CV voltage, integer is required only for display into DMD (to avoid "decimals" cosmetic issues, at the right side of DMD!).
	int rateRatioCVi = 0;

	//// BPM-RELATED VARIABLES (STANDALONE CLOCK GENERATOR).

	// Default BPM (when KlokSpid is acting as clock generator). Default is 120 BPM (centered knob).
	int svBPM = 120; // saved value.
	int BPM = 120;
	// Previous registed BPM (when KlokSpid is acting as clock generator), from previous step.
	int previousBPM = 120;

	// Custom jacks ratios (per output jack). By default false, all are X1 (original setting for KlokSpid). True means each jack can receive an optional ratio.
	bool defOutRatios = false;
	int outputRatio[4] = {9, 12, 13, 15};
	int outputRatioInUse[4] = {9, 12, 13, 15};
	float list_outRatiof[25] = {64.0f, 32.0f, 24.0f, 16.0f, 12.0f, 9.0f, 8.0f, 6.0f, 5.0f, 4.0f, 3.0f, 2.0f, 1.0f, 0.5f, 1.0f/3.0f, 0.25f, 0.2f, 1.0f/6.0f, 0.125f, 1.0f/9.0f, 1.0f/12.0f, 0.0625f, 1.0f/24.0f, 0.03125f, 0.015625f};

	// Indicates if "CV-RATIO/TRIG." input port (used as trigger, standalone BPM-clock mode only) is a transport trigger.
	// true means act as transport toggle start/stop (default).
	// false means act as reset (useful for clock "re-sync" between different modules).
	bool transportTrig = true;
	// Standalone clock generator mode only: indicates if BPM is running or stopped.
	bool isBPMRunning = true;
	bool runBPMOnInit = true;

	//// SETUP-RELATED VARIABLES/TABLES.

	// Enumeration of SETUP menu entries.
	enum setupMenuEntries {
		SETUP_WELCOME_MESSAGE,	// SETUP menu entry for #0 is always dedicated to welcome message ("*- SETUP -*") displayed on DMD.
		SETUP_CVPOLARITY,	// SETUP menu entry for CV polarity (bipolar, or unipolar).
		SETUP_DURATION,	// SETUP menu entry for pulse duration (fixed and gate-based parameters).
		SETUP_OUTVOLTAGE,	// SETUP menu entry for output voltage.
		SETUP_OUTSRATIOS, // SETUP menu entry for custom ratio concerning all jacks ("All @ x1", or "Custom"). As some requests, "Custom" (/4 x1 x2 x4) now becomes default (since v1.1.1).
		SETUP_OUT1RATIO, // SETUP menu entry for custom ratio concerning output jack #1.
		SETUP_OUT2RATIO, // SETUP menu entry for custom ratio concerning output jack #2.
		SETUP_OUT3RATIO, // SETUP menu entry for custom ratio concerning output jack #3.
		SETUP_OUT4RATIO, // SETUP menu entry for custom ratio concerning output jack #4.
		SETUP_OUT4LFO,	// SETUP menu entry for LFO to output jack #4.
		SETUP_OUT4LFOPOLARITY,	// SETUP menu entry for LFO polarity to output jack #4 (bipolar, or unipolar).
		SETUP_CVTRIG,	// SETUP menu entry describing how CV/TRIG input port is working (as start/stop toggle, or as "RESET" input).
		SETUP_EXIT,	// Lastest menu entry is always used to exit SETUP (options are "Save/Exit", "Canc/Exit", "Review" or "Factory").
		NUM_SETUP_ENTRIES // This position indicates how many entries the KlokSpid's SETUP menu have.
	};

	// Strings for SETUP entries.
	const std::string setupMenuName[NUM_SETUP_ENTRIES] = {"*-SETUP-*", "CV Polarity", "Pulse Durat.", "Out. Voltage", "Outp. Ratios", "Out. 1 Ratio", "Out. 2 Ratio", "Out. 3 Ratio", "Out. 4 Ratio", "Out. 4 LFO", "LFO Polarity", "TRIG. Jack", "Exit SETUP"};
	// Strings for SETUP possible parameters.
	std::string setupParamName[NUM_SETUP_ENTRIES][25];
	// Related horizontal offsets (second line of DMD).
	int setupParamXOffset[NUM_SETUP_ENTRIES][25];

	// This flag indicates if KlokSpid module is currently running SETUP, or not.
	bool isSetupRunning = false;
	// This flag indicates if KlokSpid module is entering SETUP (2 seconds delay), or not.
	bool isEnteringSetup = false;
	// This flag indicates if KlokSpid module is exiting SETUP (2 seconds delay), or not.
	bool isExitingSetup = false;
	// This flag is designed to avoid continuous SETUP entries/exits while button is continously held.
	bool allowedButtonHeld = false;
	// Item index (edited parameter number).
	int setup_ParamIdx = 0;
	// Current edited value for selected parameter.
	int setup_CurrentValue = 0;
	// Table containing number of possible values for each parameter.
	int setup_NumValue[NUM_SETUP_ENTRIES] = {0, 2, 9, 4, 2, 25, 25, 25, 25, 7, 2, 2, 4};
	// Default factory values for each parameter.
	int setup_Factory[NUM_SETUP_ENTRIES] = {0, 0, 5, 0, 1, 9, 12, 13, 15, 0, 0, 0, 1};
	// Table containing current values for all parameters.
	int setup_Current[NUM_SETUP_ENTRIES] = {0, 0, 5, 0, 1, 9, 12, 13, 15, 0, 0, 0, 1};
	// Table containing edited parameters during SETUP (will be filled when entering SETUP).
	int setup_Edited[NUM_SETUP_ENTRIES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
	// Table containing backup edited parameters during SETUP (will be filled when entering SETUP).
	int setup_Backup[NUM_SETUP_ENTRIES] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
	// Counter (as "delay") used to enter and (optionally) to saved/exit SETUP quickly on long press.
	long setupCounter = 0;

	//// PULSE TO OUTPUT RELATED VARIABLES AND PULSE GENERATORS.

	// Enumeration of possible pulse durations: fixed 1 ms, fixed 2 ms, fixed 5 ms, Gate 1/4, Gate 1/3, Square, Gate 2/3, Gate 3/4, Gate 95%.
	enum PulseDurations {
		FIXED1MS,	// Fixed 1 ms.
		FIXED2MS,	// Fixed 2 ms.
		FIXED5MS,	// Fixed 5 ms.
		GATE25,	// Gate 1/4 (25%).
		GATE33,	// Gate 1/3 (33%).
		SQUARE,	// Square waveform.
		GATE66,	// Gate 2/3 (66%).
		GATE75,	// Gate 3/4 (75%).
		GATE95,	// Gate 95%.
	};

	// Pulse counter for divider mode (set at max divider value, minus 1).
	int pulseDivCounter[NUM_OUTPUTS] = {63, 63, 63, 63};
	// Pulse counter for multiplier mode, to avoid continuous pulse when no more receiving (set at max divider value, minus 1). Kind of "timeout".
	int pulseMultCounter[NUM_OUTPUTS] = {0, 0, 0, 0};
	// Pulse generators, to send "pulses" to output jacks (one pulse generator per output jack).
	dsp::PulseGenerator sendPulse[NUM_OUTPUTS];
	// These flags are related to pulse generators (current pulse state).
	bool sendingOutput[NUM_OUTPUTS] = {false, false, false, false};
	// This flag indicates if sending pulse (one per output jack) is allowed (true) or not (false).
	bool canPulse[NUM_OUTPUTS] = {false, false, false, false};
	// Current pulse duration (time in second). Default is fixed 1 ms at start. Operational can be changed via SETUP.
	float pulseDuration[NUM_OUTPUTS] = {0.001f, 0.001f, 0.001f, 0.001f};
	// Extension of "pulseDuration" value (for square and gate modes), set as square wave (50 %) by default, can be changed via SETUP.
	int pulseDurationExt = SQUARE;
	// Voltage for outputs (pulses/gates), default is +5V, can be changed to +10V, +12V (+11.7V) or +2V instead, via SETUP.
	float outVoltage = 5.0f;
	// Special LFO output on jack #4.
	int jack4LFO = 0;
	bool jack4LFObipolar = true;
	bool resetPhase = true;

	// Counter used for red CLK LED afterglow (used together with "ledClkAfterglow" boolean flag).
	long ledClkDelay = 0; // long is required for highest engine samplerates!
	// This flag controls CLK (red) LED afterglow (active or not).
	bool ledClkAfterglow = false;

	// Assuming clock generator isn't synchronized (sync'd) with source clock on initialization.
	bool isSync = false;

	// Dummy string (used for std::string to char * conversions).
	std::string _tmpString; // Dummy string.

	KlokSpidModule() {
		// Constructor...
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configInput(INPUT_CLOCK, "Clock");
		configInput(INPUT_CV_TRIG, "CV-Ratio or trigger");
		configOutput(OUTPUT_1, "1st");
		configOutput(OUTPUT_2, "2nd");
		configOutput(OUTPUT_3, "3rd");
		configOutput(OUTPUT_4, "4th");
		configParam(PARAM_ENCODER, -INFINITY, INFINITY, 0.0f, "Encoder");	
		configParam(PARAM_BUTTON, 0.0f, 1.0f, 0.0f, "Button");
		//configButton(PARAM_BUTTON);
		configBypass(INPUT_CLOCK, OUTPUT_1);
		configBypass(INPUT_CLOCK, OUTPUT_2);
		configBypass(INPUT_CLOCK, OUTPUT_3);
		configBypass(INPUT_CLOCK, OUTPUT_4);
		onSampleRateChange();
	}

	void onSampleRateChange() override {
		sampleRate = APP->engine->getSampleRate();
		sampleTime = APP->engine->getSampleTime();
	}		

	//////////////////////////////////////
	//// FUNCTIONS & METHODS (VOIDS). ////
	//////////////////////////////////////

	void updateDisplayJack(int jackID) {
		if (activeCLK) {
			// Clock modulator mode. For now, all ports are at x1.
			dmdOffsetTextOut1 = 5;
			strcpy(dmdTextMainOut1, "x1");
			dmdOffsetTextOut2 = 5;
			strcpy(dmdTextMainOut2, "x1");
			dmdOffsetTextOut3 = 5;
			strcpy(dmdTextMainOut3, "x1");
			dmdOffsetTextOut4 = 5;
			strcpy(dmdTextMainOut4, "x1");
		}
		else {
			// Clock generator mode.
			switch (jackID) {
				case 0:
					dmdOffsetTextOut1 = 0;
					if ((outputRatioInUse[0] > 4) && (outputRatioInUse[0] < 12))
						dmdOffsetTextOut1 = 4;
						else if ((outputRatioInUse[0] > 11) && (outputRatioInUse[0] < 20))
							dmdOffsetTextOut1 = 5;
							else if (outputRatioInUse[0] > 19)
								 dmdOffsetTextOut2 = 1;
					_tmpString = setupParamName[SETUP_OUT1RATIO][outputRatioInUse[0]];
					strcpy(dmdTextMainOut1, _tmpString.c_str());
					break;
				case 1:
					dmdOffsetTextOut2 = 0;
					if ((outputRatioInUse[1] > 4) && (outputRatioInUse[1] < 12))
						dmdOffsetTextOut2 = 4;
						else if ((outputRatioInUse[1] > 11) && (outputRatioInUse[1] < 20))
							dmdOffsetTextOut2 = 5;
							else if (outputRatioInUse[1] > 19)
								 dmdOffsetTextOut2 = 1;
					_tmpString = setupParamName[SETUP_OUT2RATIO][outputRatioInUse[1]];
					strcpy(dmdTextMainOut2, _tmpString.c_str());
					break;
				case 2:
					dmdOffsetTextOut3 = 0;
					if ((outputRatioInUse[2] > 4) && (outputRatioInUse[2] < 12))
						dmdOffsetTextOut3 = 4;
						else if ((outputRatioInUse[2] > 11) && (outputRatioInUse[2] < 20))
							dmdOffsetTextOut3 = 5;
							else if (outputRatioInUse[2] > 19)
								 dmdOffsetTextOut3 = 1;
					_tmpString = setupParamName[SETUP_OUT3RATIO][outputRatioInUse[2]];
					strcpy(dmdTextMainOut3, _tmpString.c_str());
					break;
				case 3:
					if (outputRatioInUse[3] == 12) {
						if (jack4LFO != 0) {
							dmdOffsetTextOut4 = 0;
							switch (jack4LFO) {
								case 1:
								case 2:
									strcpy(dmdTextMainOut4, "SIN");
									break;
								case 3:
								case 4:
									strcpy(dmdTextMainOut4, "TRI");
									break;
								case 5:
								case 6:
									strcpy(dmdTextMainOut4, "SAW");
							}
						}
						else {
							dmdOffsetTextOut4 = 5;
							strcpy(dmdTextMainOut4, "x1");
						}
					}
					else {
						dmdOffsetTextOut4 = 0;
						if ((outputRatioInUse[3] > 4) && (outputRatioInUse[3] < 12))
							dmdOffsetTextOut4 = 4;
							else if ((outputRatioInUse[3] > 11) && (outputRatioInUse[3] < 20))
								dmdOffsetTextOut4 = 5;
								else if (outputRatioInUse[3] > 19)
									 dmdOffsetTextOut4 = 1;
						_tmpString = setupParamName[SETUP_OUT4RATIO][outputRatioInUse[3]];
						strcpy(dmdTextMainOut4, _tmpString.c_str());
					}
					break;
			}
		}
	}

	// Set the DMD, regarding current mode (0 = BPM generator, 1 = clock modulator by encoder).
	void updateDMDtoRunningMode(int currMode) {
		// Update small displays for each output jacks.
		for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
			updateDisplayJack(i);
		switch (currMode) {
			case 0:
				// BPM clock generator.
				// Update main DMD.
				if (!isSetupRunning) {
					_tmpString = runningMode[0];
					strcpy(dmdTextMain1, _tmpString.c_str());
					if (BPM < 10)
						dmdOffsetTextMain2 = 19;
						else if (BPM < 100)
							dmdOffsetTextMain2 = 13;
							else dmdOffsetTextMain2 = 7;
					_tmpString = std::to_string(BPM) + " BPM";
					strcpy(dmdTextMain2, _tmpString.c_str());
				}
				break;
			case 1:
				// Clock modulator.
				if (inputs[INPUT_CV_TRIG].isConnected()) {
					// Ratio is modulated by CV.
					voltageOnCV = inputs[INPUT_CV_TRIG].getVoltage();
					if (bipolarCV)
						rateRatioCV = round(clamp(static_cast<float>(voltageOnCV), -5.0f, 5.0f) * 12.6f); // By bipolar voltage (-5V/+5V).
						else rateRatioCV = round((clamp(static_cast<float>(voltageOnCV), 0.0f, 10.0f) - 5.0f) * 12.6f); // By unipolar voltage (0V/+10V).
					// Required to display ratio without artifacts!
					rateRatioCVi = static_cast<int>(rateRatioCV);
					if (round(rateRatioCV) == 0.0f) {
						clkModulatorMode = X1;
						rateRatioCV = 1.0f; // Real ratio becomes... 1.0f because it's multiplied by 1.
					}
					else if (round(rateRatioCV) > 0.0f) {
						clkModulatorMode = MULT;
						rateRatioCV = round(rateRatioCV + 1.0f);
					}
					else {
						clkModulatorMode = DIV;
						rateRatioCV = 1.0f / round(1.0f - rateRatioCV);
					}
					if (!isSetupRunning) {
						// Clock modulator (free ratio by CV).
						_tmpString = runningMode[2];
						strcpy(dmdTextMain1, _tmpString.c_str());
						dmdOffsetTextMain2 = 2;
						std::string sSign = "x";
						if (rateRatioCVi >= 0) {
							rateRatioCVi = rateRatioCVi + 1;
						}
						else {
							rateRatioCVi = 1 - rateRatioCVi;
							sSign = "/";
						}
						_tmpString = "Rat.: " + sSign + std::to_string(rateRatioCVi);
						strcpy(dmdTextMain2, _tmpString.c_str());
					}
				}
				else {
					// Clock modulator (preset ratio by encoder).
					// Ratio is selected from encoder.
					if (!isSetupRunning) {
						// Related multiplier/divider mode.
						clkModulatorMode = DIV;
						if (rateRatioByEncoder == 15)
							clkModulatorMode = X1;
							else if (rateRatioByEncoder > 15)
								clkModulatorMode = MULT;
						static const int list_iRatio[31] = {64, 32, 24, 16, 15, 12, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 15, 16, 24, 32, 64};
						_tmpString = runningMode[1];
						strcpy(dmdTextMain1, _tmpString.c_str());
						dmdOffsetTextMain2 = 2;
						std::string sSign = "x";
						if (rateRatioByEncoder < 15)
							sSign = "/";
						_tmpString = "Rat.: " + sSign + std::to_string(list_iRatio[rateRatioByEncoder]);
						strcpy(dmdTextMain2, _tmpString.c_str());
					}
				}
		}
	}

	// This custom function applies current settings (useful after SETUP operation, also "on-the-fly" altered parameter during Setup - useful to experiment).
	void UpdateKlokSpidSettings(bool allowJsonUpdate) {
		// SETUP parameter SETUP_CVPOLARITY: CV polarity (bipolar or unipolar CV-Ratio).
		bipolarCV = (setup_Current[SETUP_CVPOLARITY] == 0); // json persistence (only if SETUP isn't running).
		if (allowJsonUpdate)
			this->bipolarCV = (setup_Current[SETUP_CVPOLARITY] == 0); // json persistence (only if SETUP isn't running).
		// SETUP parameter SETUP_DURATION: possible pulse durations (1 ms, 2 ms, 5 ms, Gate 1/4, Gate 1/3, Square, Gate 2/3, Gate 3/4, Gate 95%). Keept for compatibility with v0.5.2 .vcv patches!
		switch (setup_Current[SETUP_DURATION]) {
			case FIXED1MS:
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
					pulseDuration[i] = 0.001f;
				break;
			case FIXED2MS:
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
					pulseDuration[i] = 0.002f;
				break;
			case FIXED5MS:
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
					pulseDuration[i] = 0.005f;
		}
		// Extension for pulse duration parameter (it's a kind of "descriptor" for non-fixed durations).
		pulseDurationExt  = setup_Current[SETUP_DURATION];
		if (allowJsonUpdate)
			this->pulseDurationExt  = setup_Current[SETUP_DURATION]; // json persistence (only if SETUP isn't running).
		// SETUP parameter SETUP_OUTVOLTAGE: output voltage: +2V, +5V, +10V or +12V (+11.7V).
		switch (setup_Current[SETUP_OUTVOLTAGE]) {
			case 0:
				outVoltage = 5.0f;
				if (allowJsonUpdate)
					this->outVoltage = 5.0f; // First setting is +5V, also factory (default) setting. json persistence (only if SETUP isn't running).
				break;
			case 1:
				outVoltage = 10.0f;
				if (allowJsonUpdate)
					this->outVoltage = 10.0f; // Second setting is +10V. json persistence (only if SETUP isn't running).
				break;
			case 2:
				outVoltage = 11.7f;
				if (allowJsonUpdate)
					this->outVoltage = 11.7f; // Third setting is +12V (real +11.7 V). json persistence (only if SETUP isn't running).
				break;
			case 3:
				outVoltage = 2.0f;
				if (allowJsonUpdate)
					this->outVoltage = 2.0f; // Last setting (introduced from v0.5.5/v0.6.0.4-beta): +2V. json persistence (only if SETUP isn't running).
		}
		// SETUP parameter SETUP_OUTSRATIOS: all output jacks at default x1, or custom (useful to bypass all 4-jack ratios during SETUP, if let at default).
		defOutRatios = (setup_Current[SETUP_OUTSRATIOS] == 0); // json persistence (only if SETUP isn't running).
		if (allowJsonUpdate)
			this->defOutRatios = (setup_Current[SETUP_OUTSRATIOS] == 0); // json persistence (only if SETUP isn't running).
		// SETUP parameter SETUP_OUT1RATIO: optional BPM rate applied on output jack #1.
		outputRatio[0] = setup_Current[SETUP_OUT1RATIO];
		if (allowJsonUpdate)
			this->outputRatio[0] = setup_Current[SETUP_OUT1RATIO]; // json persistence (only if SETUP isn't running).
		if (defOutRatios)
			outputRatioInUse[0] = 12;
			else outputRatioInUse[0] = outputRatio[0];
		// SETUP parameter SETUP_OUT2RATIO: optional BPM rate applied on output jack #2.
		outputRatio[1] = setup_Current[SETUP_OUT2RATIO];
		if (allowJsonUpdate)
			this->outputRatio[1] = setup_Current[SETUP_OUT2RATIO]; // json persistence (only if SETUP isn't running).
		if (defOutRatios)
			outputRatioInUse[1] = 12;
			else outputRatioInUse[1] = outputRatio[1];
		// SETUP parameter SETUP_OUT3RATIO: optional BPM rate applied on output jack #3.
		outputRatio[2] = setup_Current[SETUP_OUT3RATIO];
		if (allowJsonUpdate)
			this->outputRatio[2] = setup_Current[SETUP_OUT3RATIO]; // json persistence (only if SETUP isn't running).
		if (defOutRatios)
			outputRatioInUse[2] = 12;
			else outputRatioInUse[2] = outputRatio[2];
		// SETUP parameter SETUP_OUT4RATIO: optional BPM rate applied on output jack #4.
		outputRatio[3] = setup_Current[SETUP_OUT4RATIO];
		if (allowJsonUpdate)
			this->outputRatio[3] = setup_Current[SETUP_OUT4RATIO]; // json persistence (only if SETUP isn't running).
		if (defOutRatios)
			outputRatioInUse[3] = 12;
			else outputRatioInUse[3] = outputRatio[3];
		// SETUP parameter SETUP_OUT4LFO: optional LFO on output jack #4: Disabled, Sine, Triangle, Saw, Inverse Sine, Inverse Triangle, Inverse Saw.
		// Introduced from v0.6.1, but remaining to do.
		jack4LFO = setup_Current[SETUP_OUT4LFO];
		if (allowJsonUpdate)
			this->jack4LFO = setup_Current[SETUP_OUT4LFO]; // json persistence (only if SETUP isn't running).
		// SETUP parameter SETUP_OUT4LFOPOLARITY: LFO polarity (bipolar or unipolar).
		jack4LFObipolar = (setup_Current[SETUP_OUT4LFOPOLARITY] == 0); // json persistence (only if SETUP isn't running).
		if (allowJsonUpdate)
			this->jack4LFObipolar = (setup_Current[SETUP_OUT4LFOPOLARITY] == 0); // json persistence (only if SETUP isn't running).
		// SETUP parameter SETUP_CVTRIG: CV-RATIO/TRIG. input port behavior (standalone clock generator only, this port is TRIG.).
		// - "true" is meaning the TRIG. input port acts as "start/stop toggle".
		// - "false" is meaning the TRIG. input port acts as "BPM reset" (useful to "re-sync" BPM from an external/reference source clock, for example).
		transportTrig = (setup_Current[SETUP_CVTRIG] == 0);
		if (allowJsonUpdate)
			this->transportTrig = (setup_Current[SETUP_CVTRIG] == 0); // json persistence (only if SETUP isn't running).
		// Update small displays for each output jacks.
		for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
			updateDisplayJack(i);
	}

	// This custom function returns pulse duration (ms), regardling number of samples (long int) and pulsation duration parameter (SETUP).
	float GetPulsingTime(long int stepGap, float rate) {
		float pTime = 0.001; // As default pulse duration is set to 1ms (also can be set to "fixed 1ms" via SETUP).
		if (stepGap == 0) {
			// No reference duration (number of samples is zero).
			switch (setup_Current[SETUP_DURATION]) {
				case FIXED2MS:
					pTime = 0.002f;	// Fixed 2 ms pulse.
					break;
				case FIXED5MS:
					pTime = 0.005f;	// Fixed 5 ms pulse.
			}
		}
		else {
			// Reference duration in number of samples (when known stepGap). Variable-length pulse duration can be defined.
			switch (setup_Current[SETUP_DURATION]) {
				case FIXED2MS:
					pTime = 0.002f;	// Fixed 2 ms pulse.
					break;
				case FIXED5MS:
					pTime = 0.005f;	// Fixed 5 ms pulse.
					break;
				case GATE25:
					pTime = rate * 0.25f * (stepGap / sampleRate);	// Gate 1/4 (25%)
					break;
				case GATE33:
					pTime = rate * (1.0f / 3.0f) * (stepGap / sampleRate);	// Gate 1/3 (33%)
					break;
				case SQUARE:
					pTime = rate * 0.5f * (stepGap / sampleRate);	// Square wave (50%)
					break;
				case GATE66:
					pTime = rate * (2.0f / 3.0f) * (stepGap / sampleRate);	// Gate 2/3 (66%)
					break;
				case GATE75:
					pTime = rate * 0.75f * (stepGap / sampleRate);	// Gate 3/4 (75%)
					break;
				case GATE95:
					pTime = rate * 0.95f * (stepGap / sampleRate);	// Gate 95%
			}
		}
		return pTime;
	}

	void process(const ProcessArgs &args) override {
		// DSP processing...
		// Depending current KlokSpid model (theme), set the relevant DMD-text color.
		DMDtextColor = tblDMDtextColor[Theme];
		// Bypass if not early run (executed once to initialize KlokSpid).
		if (bEarlyRun) {
			// Small displays near output jacks.
			dmdOffsetTextOut1 = 0;
			strcpy(dmdTextMainOut1, "");
			dmdOffsetTextOut2 = 0;
			strcpy(dmdTextMainOut2, "");
			dmdOffsetTextOut3 = 0;
			strcpy(dmdTextMainOut3, "");
			dmdOffsetTextOut4 = 0;
			strcpy(dmdTextMainOut4, "");
			// Last step for initialization.
			// This is the lastest step of initialization.
			// Filling table containing current SETUP parameters.
			// SETUP parameter SETUP_CVPOLARITY: bipolar or unipolar CV.
			setup_Current[SETUP_CVPOLARITY] = bipolarCV ? 0 : 1;
			// SETUP parameter SETUP_DURATION: Pulse duration (extended, to keep compatibility with previous v0.5.2).
			// Parameter #2: possible pulse durations (fixed 1 ms, 2 ms or 5 ms durations, Gate 1/4, Gate 1/3, Square, Gate 2/3, Gate 3/4, Gate 95%).
			setup_Current[SETUP_DURATION] = pulseDurationExt;
			switch (pulseDurationExt) {
				case FIXED1MS:
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						pulseDuration[i] = 0.001f;
					break;
				case FIXED2MS:
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						pulseDuration[i] = 0.002f;
					break;
				case FIXED5MS:
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						pulseDuration[i] = 0.005f;
					break;
				default:
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						pulseDuration[i] = 0.001f; // It's a default value, but gates are defined in realtime (later).
			}
			// If output voltage is above +11V, assuming +11.7V.
			if (round(outVoltage * 10) > 110)
				outVoltage = 11.7f;
			// Assuming +5V is default output voltage.
			setup_Current[SETUP_OUTVOLTAGE] = 0; // +5V.
			// SETUP parameter SETUP_OUTVOLTAGE: Output voltage.
			if (round(outVoltage * 10) == 20)
				setup_Current[SETUP_OUTVOLTAGE] = 3; // +2V. Lastest value (instead of "inserted" at first, to preserve compatibility!).
				else if (round(outVoltage * 10) == 100)
					setup_Current[SETUP_OUTVOLTAGE] = 1; // +10V.
					else if (round(outVoltage * 10) == 117)
						setup_Current[SETUP_OUTVOLTAGE] = 2; // +11.7V (indicated +12V in module's SETUP).
			// SETUP parameter SETUP_OUTSRATIOS: enabled or disabled custom ratios (for all output jacks).
			setup_Current[SETUP_OUTSRATIOS] = defOutRatios ? 0 : 1;
			// SETUP parameter SETUP_OUT1RATIO to SETUP_OUT4RATIO: optional BPM rate applied on output jacks #1~#4.
			for (int i = 0; i < NUM_OUTPUTS; i++) {
				setup_Current[SETUP_OUT1RATIO + i] = outputRatio[i];
				if (defOutRatios)
					outputRatioInUse[i] = 12;
					else outputRatioInUse[i] = outputRatio[i];
			}
			// SETUP parameter SETUP_OUT4LFO: optional LFO on output jack #4.
			setup_Current[SETUP_OUT4LFO] = jack4LFO;
			// SETUP parameter SETUP_OUT4LFOPOLARITY: bipolar or unipolar LFO.
			setup_Current[SETUP_OUT4LFOPOLARITY] = jack4LFObipolar ? 0 : 1;
			// SETUP parameter SETUP_CVTRIG: CV/TRIG port, as trigger input when running as standalone clock generator (only).
			setup_Current[SETUP_CVTRIG] = transportTrig ? 0 : 1;
			// Parameter's value is, by default 1 for default "Save/Exit".
			setup_Current[SETUP_EXIT] = 1;
			// Is standalone clock is running at init, or not (previous state).
			isBPMRunning = this->runBPMOnInit;
			// Strings construction for SETUP.
			// SETUP_WELCOME_MESSAGE (unique option).
			setupParamName[SETUP_WELCOME_MESSAGE][0] = "Press Btn!";
			setupParamXOffset[SETUP_WELCOME_MESSAGE][0] = -1;
			for (int i = 1; i < 22; i++) {
				setupParamName[SETUP_WELCOME_MESSAGE][i] = ""; // Unused (useless) strings are set to empty.
				setupParamXOffset[SETUP_WELCOME_MESSAGE][i] = 0; // Useless x-offsets to 0.
			}
			// SETUP_CVPOLARITY: having 2 possible parameters.
			setupParamName[SETUP_CVPOLARITY][0] = "Bipolar";
			setupParamXOffset[SETUP_CVPOLARITY][0] = 2;
			setupParamName[SETUP_CVPOLARITY][1] = "Unipolar";
			setupParamXOffset[SETUP_CVPOLARITY][1] = 2;
			for (int i = 2; i < 22; i++) {
				setupParamName[SETUP_CVPOLARITY][i] = ""; // Unused (useless) strings are set to empty.
				setupParamXOffset[SETUP_CVPOLARITY][i] = 0; // Useless x-offsets to 0.
			}
			// SETUP_DURATION: having 9 possible parameters.
			setupParamName[SETUP_DURATION][FIXED1MS] = "Fixed 1ms";
			setupParamXOffset[SETUP_DURATION][FIXED1MS] = 1;
			setupParamName[SETUP_DURATION][FIXED2MS] = "Fixed 2ms";
			setupParamXOffset[SETUP_DURATION][FIXED2MS] = 1;
			setupParamName[SETUP_DURATION][FIXED5MS] = "Fixed 5ms";
			setupParamXOffset[SETUP_DURATION][FIXED5MS] = 1;
			setupParamName[SETUP_DURATION][GATE25] = "Gate 25%";
			setupParamXOffset[SETUP_DURATION][GATE25] = 2;
			setupParamName[SETUP_DURATION][GATE33] = "Gate 33%";
			setupParamXOffset[SETUP_DURATION][GATE33] = 2;
			setupParamName[SETUP_DURATION][SQUARE] = "Square W.";
			setupParamXOffset[SETUP_DURATION][SQUARE] = 2;
			setupParamName[SETUP_DURATION][GATE66] = "Gate 66%";
			setupParamXOffset[SETUP_DURATION][GATE66] = 2;
			setupParamName[SETUP_DURATION][GATE75] = "Gate 75%";
			setupParamXOffset[SETUP_DURATION][GATE75] = 2;
			setupParamName[SETUP_DURATION][GATE95] = "Gate 95%";
			setupParamXOffset[SETUP_DURATION][GATE95] = 2;
			// SETUP_OUTVOLTAGE: having 4 possible parameters.
			setupParamName[SETUP_OUTVOLTAGE][0] = "+5V";
			setupParamXOffset[SETUP_OUTVOLTAGE][0] = 2;
			setupParamName[SETUP_OUTVOLTAGE][1] = "+10V";
			setupParamXOffset[SETUP_OUTVOLTAGE][1] = 2;
			setupParamName[SETUP_OUTVOLTAGE][2] = "+11.7V";
			setupParamXOffset[SETUP_OUTVOLTAGE][2] = 2;
			setupParamName[SETUP_OUTVOLTAGE][3] = "+2V";
			setupParamXOffset[SETUP_OUTVOLTAGE][3] = 2;
			for (int i = 4; i < 22; i++) {
				setupParamName[SETUP_OUTVOLTAGE][i] = ""; // Unused (useless) strings are set to empty.
				setupParamXOffset[SETUP_OUTVOLTAGE][i] = 0; // Useless x-offsets to 0.
			}
			// SETUP_OUTSRATIOS: having 2 possible parameters.
			setupParamName[SETUP_OUTSRATIOS][0] = "All @ x1";
			setupParamXOffset[SETUP_OUTSRATIOS][0] = 2;
			setupParamName[SETUP_OUTSRATIOS][1] = "Custom";
			setupParamXOffset[SETUP_OUTSRATIOS][1] = 2;
			for (int i = 2; i < 22; i++) {
				setupParamName[SETUP_OUTSRATIOS][i] = ""; // Unused (useless) strings are set to empty.
				setupParamXOffset[SETUP_OUTSRATIOS][i] = 0; // Useless x-offsets to 0.
			}
			// SETUP_OUT1RATIO to SETUP_OUT4RATIO: each ratio for any output jack have 25 possible parameters.
			for (int i = 0; i < 4; i++) {
				setupParamName[SETUP_OUT1RATIO + i][0] = "/64";
				setupParamXOffset[SETUP_OUT1RATIO + i][0] = 32;
				setupParamName[SETUP_OUT1RATIO + i][1] = "/32";
				setupParamXOffset[SETUP_OUT1RATIO + i][1] = 32;
				setupParamName[SETUP_OUT1RATIO + i][2] = "/24";
				setupParamXOffset[SETUP_OUT1RATIO + i][2] = 32;
				setupParamName[SETUP_OUT1RATIO + i][3] = "/16";
				setupParamXOffset[SETUP_OUT1RATIO + i][3] = 32;
				setupParamName[SETUP_OUT1RATIO + i][4] = "/12";
				setupParamXOffset[SETUP_OUT1RATIO + i][4] = 32;
				setupParamName[SETUP_OUT1RATIO + i][5] = "/9";
				setupParamXOffset[SETUP_OUT1RATIO + i][5] = 38;
				setupParamName[SETUP_OUT1RATIO + i][6] = "/8";
				setupParamXOffset[SETUP_OUT1RATIO + i][6] = 38;
				setupParamName[SETUP_OUT1RATIO + i][7] = "/6";
				setupParamXOffset[SETUP_OUT1RATIO + i][7] = 38;
				setupParamName[SETUP_OUT1RATIO + i][8] = "/5";
				setupParamXOffset[SETUP_OUT1RATIO + i][8] = 38;
				setupParamName[SETUP_OUT1RATIO + i][9] = "/4";
				setupParamXOffset[SETUP_OUT1RATIO + i][9] = 38;
				setupParamName[SETUP_OUT1RATIO + i][10] = "/3";
				setupParamXOffset[SETUP_OUT1RATIO + i][10] = 38;
				setupParamName[SETUP_OUT1RATIO + i][11] = "/2";
				setupParamXOffset[SETUP_OUT1RATIO + i][11] = 38;
				if (i == OUTPUT_4) {
					setupParamName[SETUP_OUT1RATIO + i][12] = "x1/LFO";
					setupParamXOffset[SETUP_OUT1RATIO + i][12] = 12;
				}
				else {
					setupParamName[SETUP_OUT1RATIO + i][12] = "x1";
					setupParamXOffset[SETUP_OUT1RATIO + i][12] = 38;
				}
				setupParamName[SETUP_OUT1RATIO + i][13] = "x2";
				setupParamXOffset[SETUP_OUT1RATIO + i][13] = 38;
				setupParamName[SETUP_OUT1RATIO + i][14] = "x3";
				setupParamXOffset[SETUP_OUT1RATIO + i][14] = 38;
				setupParamName[SETUP_OUT1RATIO + i][15] = "x4";
				setupParamXOffset[SETUP_OUT1RATIO + i][15] = 38;
				setupParamName[SETUP_OUT1RATIO + i][16] = "x5";
				setupParamXOffset[SETUP_OUT1RATIO + i][16] = 38;
				setupParamName[SETUP_OUT1RATIO + i][17] = "x6";
				setupParamXOffset[SETUP_OUT1RATIO + i][17] = 38;
				setupParamName[SETUP_OUT1RATIO + i][18] = "x8";
				setupParamXOffset[SETUP_OUT1RATIO + i][18] = 38;
				setupParamName[SETUP_OUT1RATIO + i][19] = "x9";
				setupParamXOffset[SETUP_OUT1RATIO + i][19] = 38;
				setupParamName[SETUP_OUT1RATIO + i][20] = "x12";
				setupParamXOffset[SETUP_OUT1RATIO + i][20] = 32;
				setupParamName[SETUP_OUT1RATIO + i][21] = "x16";
				setupParamXOffset[SETUP_OUT1RATIO + i][21] = 32;
				setupParamName[SETUP_OUT1RATIO + i][22] = "x24";
				setupParamXOffset[SETUP_OUT1RATIO + i][22] = 32;
				setupParamName[SETUP_OUT1RATIO + i][23] = "x32";
				setupParamXOffset[SETUP_OUT1RATIO + i][23] = 32;
				setupParamName[SETUP_OUT1RATIO + i][24] = "x64";
				setupParamXOffset[SETUP_OUT1RATIO + i][24] = 32;
			}
			// SETUP_OUT4LFO: having 7 possible parameters.
			setupParamName[SETUP_OUT4LFO][0] = "Disabled";
			setupParamXOffset[SETUP_OUT4LFO][0] = 2;
			setupParamName[SETUP_OUT4LFO][1] = "Sine";
			setupParamXOffset[SETUP_OUT4LFO][1] = 2;
			setupParamName[SETUP_OUT4LFO][2] = "Inv. Sine";
			setupParamXOffset[SETUP_OUT4LFO][2] = 2;
			setupParamName[SETUP_OUT4LFO][3] = "Triangle";
			setupParamXOffset[SETUP_OUT4LFO][3] = 2;
			setupParamName[SETUP_OUT4LFO][4] = "Inv. Tri.";
			setupParamXOffset[SETUP_OUT4LFO][4] = 2;
			setupParamName[SETUP_OUT4LFO][5] = "Sawtooth";
			setupParamXOffset[SETUP_OUT4LFO][5] = 2;
			setupParamName[SETUP_OUT4LFO][6] = "Inv. Saw.";
			setupParamXOffset[SETUP_OUT4LFO][6] = 2;
			for (int i = 7; i < 22; i++) {
				setupParamName[SETUP_OUT4LFO][i] = ""; // Unused (useless) strings are set to empty.
				setupParamXOffset[SETUP_OUT4LFO][i] = 0; // Useless x-offsets to 0.
			}
			// SETUP_OUT4LFOPOLARITY: having 2 possible parameters.
			setupParamName[SETUP_OUT4LFOPOLARITY][0] = "Bipolar";
			setupParamXOffset[SETUP_OUT4LFOPOLARITY][0] = 2;
			setupParamName[SETUP_OUT4LFOPOLARITY][1] = "Unipolar";
			setupParamXOffset[SETUP_OUT4LFOPOLARITY][1] = 2;
			for (int i = 2; i < 22; i++) {
				setupParamName[SETUP_OUT4LFOPOLARITY][i] = ""; // Unused (useless) strings are set to empty.
				setupParamXOffset[SETUP_OUT4LFOPOLARITY][i] = 0; // Useless x-offsets to 0.
			}
			// SETUP_CVTRIG: having 2 possible parameters.
			setupParamName[SETUP_CVTRIG][0] = "Play/Stop";
			setupParamXOffset[SETUP_CVTRIG][0] = 2;
			setupParamName[SETUP_CVTRIG][1] = "Reset In.";
			setupParamXOffset[SETUP_CVTRIG][1] = 2;
			for (int i = 2; i < 22; i++) {
				setupParamName[SETUP_CVTRIG][i] = ""; // Unused (useless) strings are set to empty.
				setupParamXOffset[SETUP_CVTRIG][i] = 0; // Useless x-offsets to 0.
			}
			// SETUP_EXIT: having 4 possible parameters.
			setupParamName[SETUP_EXIT][0] = "Canc./Exit";
			setupParamXOffset[SETUP_EXIT][0] = -1;
			setupParamName[SETUP_EXIT][1] = "Save/Exit";
			setupParamXOffset[SETUP_EXIT][1] = 1;
			setupParamName[SETUP_EXIT][2] = "Review...";
			setupParamXOffset[SETUP_EXIT][2] = 2;
			setupParamName[SETUP_EXIT][3] = "Factory";
			setupParamXOffset[SETUP_EXIT][3] = 2;
			for (int i = 4; i < 22; i++) {
				setupParamName[SETUP_EXIT][i] = ""; // Unused (useless) strings are set to empty.
				setupParamXOffset[SETUP_EXIT][i] = 0; // Useless x-offsets to 0.
			}
			activeCLK = inputs[INPUT_CLOCK].isConnected();
			activeCLKPrevious = activeCLK;
			activeCV = inputs[INPUT_CV_TRIG].isConnected();
			activeCVPrevious = activeCV;
			// Reinit encoder reading.
			encoderCurrent = (int)roundf(10.0f * params[PARAM_ENCODER].getValue());
			encoderPrevious = encoderCurrent;
			encoderDelta = 0; // Default assuming encoder isn't moved.
			//
			currentStep = 0;
			rateRatioByEncoder = this->svRatio;
			BPM = this->svBPM;
			if (activeCLK)
				updateDMDtoRunningMode(1);
				else updateDMDtoRunningMode(0);
			bEarlyRun = false;
			return;
		}

		// Current state of CLK port.  Active means connected/wired.
		activeCLK = inputs[INPUT_CLOCK].isConnected();

		// Current state and voltage (CV/TRIG port). Active means connected/wired.
		activeCV = inputs[INPUT_CV_TRIG].isConnected();

		// Encoder behavior (moved or not).
		encoderCurrent = (int)roundf(10.0f * params[PARAM_ENCODER].getValue());
		encoderDelta = 0; // Default assuming encoder isn't moved.
		if (abs(encoderCurrent - encoderPrevious) <= 2) {
			if (encoderCurrent < encoderPrevious)
				encoderDelta = -1; // Counter-clockwise ==> decrement.
				else if (encoderCurrent > encoderPrevious)
					encoderDelta = 1; // Clockwise => increment.
		}
		// Save current encoder position to become previous (for next check).
		encoderPrevious = encoderCurrent;

		if (activeCLK != activeCLKPrevious) {
			// Is state was changed (added or removed a patch cable to/away CLK port)?
			// New state will become "previous" state.
			activeCLKPrevious = activeCLK;
			// Reset all steps counter and "gaps", not synchronized.
			currentStep = 0;
			previousStep = 0;
			expectedStep = 0;
			stepGap = 0;
			stepGapPrevious = 0;
			isSync = false;
			for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
				canPulse[i] = false;
				nextPulseStep[i] = 0;
			}
			if (!activeCLK)
				updateDMDtoRunningMode(0);
				else {
					activeCV = inputs[INPUT_CV_TRIG].isConnected();
					updateDMDtoRunningMode(1);
				}
		}

		if (activeCV != activeCVPrevious) {
			// Is state was changed (added or removed a patch cable to/away CLK port)?
			// New state will become "previous" state.
			activeCVPrevious = activeCV;
			if (activeCLK)
				updateDMDtoRunningMode(1);
		}

		// Considering CV (if applicable e.g. wired!).
		voltageOnCV = 0.0f;
		isRatioCVmod = false;
		rateRatioCV = 0.0f;

		if (activeCV) {
			voltageOnCV = inputs[INPUT_CV_TRIG].getVoltage();
			if (activeCLK) {
				// Considering CV-RATIO signal to modulate ratio (doesn't matter if SETUP is running, or not).
				isRatioCVmod = true;
				if (bipolarCV)
					rateRatioCV = round(clamp(static_cast<float>(voltageOnCV), -5.0f, 5.0f) * 12.6f); // By bipolar voltage (-5V/+5V).
					else rateRatioCV = round((clamp(static_cast<float>(voltageOnCV), 0.0f, 10.0f) - 5.0f) * 12.6f); // By unipolar voltage (0V/+10V).
				// Update DMD.
				updateDMDtoRunningMode(1);
			}
			else {
				// BPM is set by encoded (except while SETUP is running).
				if (!isSetupRunning) {
					if (encoderDelta != 0) {
						BPM = BPM + encoderDelta; // May be increased or decreased.
						if (BPM < 1)
							BPM = 1; // Minimum 1 BPM.
							else if (BPM > 960)
								BPM = 960; // Maximum 960 BPM.
						this->svBPM = BPM;
						// Reset encoder move detection.
						encoderDelta = 0;
						// Update DMD.
						updateDMDtoRunningMode(0);
					}
				}
			}
		}
		else {
			if (!isSetupRunning) {
				if (activeCLK) {
					// Preset ratios are controlled by encoder.
					if (encoderDelta != 0) {
						rateRatioByEncoder = rateRatioByEncoder + encoderDelta;
						if (rateRatioByEncoder < 0)
							rateRatioByEncoder = 0; // Limiting to 0 (/64).
							else if (rateRatioByEncoder > 30)
								rateRatioByEncoder = 30; // Limiting to 30 (X64).
						this->svRatio = rateRatioByEncoder;
						// Related multiplier/divider mode.
						clkModulatorMode = DIV;
						if (rateRatioByEncoder == 15)
							clkModulatorMode = X1;
							else if (rateRatioByEncoder > 15)
								clkModulatorMode = MULT;
						// Reset encoder move detection.
						encoderDelta = 0;
						// Update DMD.
						updateDMDtoRunningMode(1);
					}
				}
				else {
					// BPM is set by encoded (except while SETUP is running).
					if (!isSetupRunning) {
						if (encoderDelta != 0) {
							BPM = BPM + encoderDelta; // May be increased or decreased.
							if (BPM < 1)
								BPM = 1; // Minimum BPM is 1.
								else if (BPM > 960)
									BPM = 960; // Maximum 960 BPM.
							this->svBPM = BPM;
							// Reset encoder move detection.
							encoderDelta = 0;
							// Update DMD.
							updateDMDtoRunningMode(0);
						}
					}
				}
			}
		}

		// Button state.
		buttonPressed = runButton.process(params[PARAM_BUTTON].getValue());

		// KlokSpid is working as multiplier/divider module (when CLK input port is connected - aka "active").
		if (activeCLK) {
			// Increment step number.
			currentStep++;
			// Using Schmitt trigger (SchmittTrigger is provided by dsp/digital.hpp) to detect thresholds from CLK input connector. Calibration: +1.7V (rising edge), low +0.2V (falling edge).
			if (CLKInputPort.process(rescale(inputs[INPUT_CLOCK].getVoltage(), 0.2f, 1.7f, 0.0f, 1.0f))) {
				// CLK input is receiving a compliant trigger voltage (rising edge): lit and "afterglow" CLK (red) LED.
				ledClkDelay = 0;
				ledClkAfterglow = true;
				if (previousStep == 0) {
					// No "history", it's the first pulse received on CLK input after a frequency change. Not synchronized.
					expectedStep = 0;
					stepGap = 0;
					stepGapPrevious = 0;
					// stepGap at 0: the pulse duration will be 1 ms (default), or 2 ms or 5 ms (depending SETUP). Variable pulses can't be used as long as frequency remains unknown.
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						if (isRatioCVmod)
							pulseDuration[i] = GetPulsingTime(0, 1.0f / rateRatioCV);  // Ratio is CV-controlled.
							else pulseDuration[i] = GetPulsingTime(0, list_fRatio[rateRatioByEncoder]);  // Ratio is controlled by encoder.
					// Not synchronized.
					isSync = false;
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
						canPulse[i] = (clkModulatorMode != MULT); // MULT needs second pulse to establish source frequency.
						pulseDivCounter[i] = 0; // Used for DIV mode exclusively!
						pulseMultCounter[i] = 0; // Used for MULT mode exclusively!
					}
					previousStep = currentStep;
				}
				else {
					// It's the second pulse received on CLK input after a frequency change.
					stepGapPrevious = stepGap;
					stepGap = currentStep - previousStep;
					expectedStep = currentStep + stepGap;
					// The frequency is known, we can determine the pulse duration (defined by SETUP).
					// The pulse duration also depends of clocking ratio, such "X1", multiplied or divided, and its ratio.
					for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
						if (isRatioCVmod)
							pulseDuration[i] = GetPulsingTime(stepGap, 1.0f / rateRatioCV); // Ratio is CV-controlled.
							else pulseDuration[i] = GetPulsingTime(stepGap, list_fRatio[rateRatioByEncoder]); // Ratio is controlled by encoder.
					isSync = true;
					if (stepGap > stepGapPrevious)
						isSync = ((stepGap - stepGapPrevious) < 2);
						else if (stepGap < stepGapPrevious)
							isSync = ((stepGapPrevious - stepGap) < 2);
					if (isSync) {
						for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
							canPulse[i] = (clkModulatorMode != DIV);
					}
					else {
						for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
							canPulse[i] = (clkModulatorMode == X1);
					}
					previousStep = currentStep;
				}

				switch (clkModulatorMode) {
					case X1:
						// Ratio is x1, following source clock, the easiest scenario! (always sync'd).
						for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
							canPulse[i] = true;
						break;
					case DIV:
						// Divider mode scenario.
						for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
							if (pulseDivCounter[i] == 0) {

								if (isRatioCVmod)
									pulseDivCounter[i] = int(1.0f / rateRatioCV) - 1; // Ratio is CV-controlled.
									else pulseDivCounter[i] = int(list_fRatio[rateRatioByEncoder] - 1); // Ratio is controlled by knob.
								canPulse[i] = true;
							}
							else {
								pulseDivCounter[i]--;
								canPulse[i] = false;
							}
						}
						break;
					case MULT:
						// Multiplier mode scenario: pulsing only when source frequency is established.
						for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
							if (isSync) {
								// Next step for pulsing in multiplier mode.
								if (isRatioCVmod) {
									// Ratio is CV-controlled.
									nextPulseStep[i] = currentStep + round(stepGap / rateRatioCV);
									pulseMultCounter[i] = int(rateRatioCV) - 1;
								}
								else {
								// Ratio is controlled by knob.
									nextPulseStep[i] = currentStep + round(stepGap * list_fRatio[rateRatioByEncoder]);
									pulseMultCounter[i] = round(1.0f / list_fRatio[rateRatioByEncoder]) - 1;
								}
								canPulse[i] = true;
							}
						}
				}
			}
			else {
				// At this point, it's not a rising edge!
				// When running as multiplier, may pulse here too during low voltages on CLK input!
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
					if (isSync && (nextPulseStep[i] == currentStep) && (clkModulatorMode == MULT)) {
						if (isRatioCVmod)
							nextPulseStep[i] = currentStep + round(stepGap / rateRatioCV); // Ratio is CV-controlled.
							else nextPulseStep[i] = currentStep + round(stepGap * list_fRatio[rateRatioByEncoder]); // Ratio is controlled by knob.
						// This block is to avoid continuous pulsing if no more receiving incoming signal.
						if (pulseMultCounter[i] > 0) {
							pulseMultCounter[i]--;
							canPulse[i] = true;
						}
						else {
							canPulse[i] = false;
							isSync = false;
						}
					}
				}
			}
		}
		else {
			// CLK input port isn't connected (not active): KlokSpid is working as clock generator.
			ledClkAfterglow = false;
			if (previousBPM == BPM) {
				// CV-RATIO/TRIG. input port is used as TRIG. to reset clock generator or to toggle BPM-clocking, while voltage is +1.7 V (or above) - rising edge.
				if (activeCV) {
					if (runTriggerPort.process(rescale(voltageOnCV, 0.2f, 1.7f, 0.0f, 1.0f))) {
						// On +1.7 V trigger (rising edge), the clock generator state if toggled (started or stopped).
						if (transportTrig) {
							// CV-RATIO/TRIG. input port (TRIG.) is configured as "play/stop toggle".
							isBPMRunning = !isBPMRunning;
							// BPM state persistence (json).
							this->runBPMOnInit = isBPMRunning;
							for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
								nextPulseStep[i] = 0;
							currentStep = 0;
							// Reset phase for LFO jack #4.
							resetPhase = true;
						}
						else {
							// CV-RATIO/TRIG. input port (TRIG.) is configured as RESET input (default factory): assuming it's an incoming reset signal!
							currentStep = 0;
							for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++)
								nextPulseStep[i] = 0;
							// Reset phase for LFO jack #4.
							resetPhase = true;
						}
					}
				}
				// Incrementing step counter...
				currentStep++;
				for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
					if (isBPMRunning) {
						if (currentStep >= nextPulseStep[i])
							canPulse[i] = true;
						if (canPulse[i]) {
							// Setting pulse...
							// Define the step for next pulse. Time reference is given by (current) engine samplerate setting.
							nextPulseStep[i] = currentStep + round(60.0f * sampleRate * list_outRatiof[outputRatioInUse[i]] / BPM);
							// Define the pulse duration (fixed or variable-length).
							pulseDuration[i] = GetPulsingTime(sampleRate, 60.0f / BPM * list_outRatiof[outputRatioInUse[i]]);
							if (i == OUTPUT_4)
								resetPhase = true;
						}
					}
					else {
						// BPM clock is stopped.
						canPulse[i] = false;
						nextPulseStep[i] = 0;
						currentStep = 0;
						// Reset phase for LFO jack #4.
						resetPhase = true;
					}
				}
			}
			else {
				// Update DMD (number of BPM).
				updateDMDtoRunningMode(0);
				// Altered BPM: reset phase for LFO jack #4.
				resetPhase = true;
			}
			previousBPM = BPM;
		}

		// Using pulse generator to output to all ports.
		for (int i = OUTPUT_1; i < NUM_OUTPUTS; i++) {
			if (canPulse[i]) {
				if (i == OUTPUT_4) {
					if (resetPhase) {
						LFOjack4.phase = 0.0f;
						resetPhase = false;
					}
				}
				// Sending pulse, using pulse generator.
		  	sendPulse[i].trigger(pulseDuration[i]);
				canPulse[i] = false;
			}
			sendingOutput[i] = sendPulse[i].process(sampleTime);
			if (i < OUTPUT_4)
		  	outputs[i].setVoltage((sendingOutput[i] ? outVoltage : 0.0f));
				else {
					// Jack #4 specific (LFO feature to output jack #4, but: clock generator mode only, and if jack ratio is set at "x1" only).
					if ((!activeCLK) && (jack4LFO != 0) && (outputRatioInUse[OUTPUT_4] == 12)) {
						LFOjack4.invert = ((jack4LFO % 2) == 0);
						LFOjack4.freq = (float)BPM / 60.0f;
						LFOjack4.step(sampleTime);
						if (jack4LFObipolar)
							LFOjack4.offset = 0.0f;
							else LFOjack4.offset = outVoltage / 2.0f;
						switch (jack4LFO) {
							case 1:
							case 2:
								// LFO for jack #4 is a sine-based waveform.
		  					outputs[OUTPUT_4].setVoltage((isBPMRunning ? outVoltage / 2.0f * LFOjack4.sin() : 0.0f));
								break;
							case 3:
							case 4:
								// LFO for jack #4 is a triangle-based waveform.
		  					outputs[OUTPUT_4].setVoltage((isBPMRunning ? outVoltage / 2.0f * LFOjack4.tri() : 0.0f));
								break;
							case 5:
							case 6:
								// LFO for jack #4 is a sawtooth-based waveform.
		  					outputs[OUTPUT_4].setVoltage((isBPMRunning ? outVoltage / 2.0f * LFOjack4.saw() : 0.0f));
						}
					}
					else outputs[OUTPUT_4].setVoltage((sendingOutput[OUTPUT_4] ? outVoltage : 0.0f));
				}
		}

		// Afterglow for CLK (red) LED.
		if (ledClkAfterglow) {
			if (inputs[INPUT_CLOCK].getVoltage() < 1.7f) {
				ledClkDelay++;
				if (ledClkDelay > round(sampleRate / 16)) {
					ledClkAfterglow = false;
					ledClkDelay = 0;
				}
			}
		}

		// Handling the button (it's a momentary button, handled by a dedicated Schmitt trigger).
		// - Short presses toggles BPM clock start/stops (when released).
		// - Long press to enter SETUP.
		// - When SETUP is running, press to advance to next parameter.
		if (buttonPressed) {
			if (!isSetupRunning && !isEnteringSetup) {
				// Try to enter SETUP... starting delay counter for 2 seconds.
				isEnteringSetup = true;
				setupCounter = 0;
				allowedButtonHeld = true; // Allow to keep button held.
			}
			else if (isSetupRunning && !isExitingSetup) {
				// Try to quick save/exit SETUP... starting delay counter for 2 seconds.
				isExitingSetup = true;
				setupCounter = 0;
				// Necessary to avoid continuous entry/exit SETUP while button is held.
				allowedButtonHeld = true; // Allow to keep button held.
			}
			else allowedButtonHeld = false; // Button must be released.
		} // Don't add "else" clause from here, otherwise be sure it doesn't work!

		if (buttonPressed && isSetupRunning) {
			// SETUP is running: when (shortly) pressed, advance to next parameter.
			// Storing previous edited parameter into "edited" table prior to advance to next SETUP parameter.
			setup_Edited[setup_ParamIdx] = setup_CurrentValue;
			// Advance to next SETUP parameter (conditional).

			if ((setup_ParamIdx == SETUP_OUTSRATIOS) && (setup_Edited[SETUP_OUTSRATIOS] == 0))
				setup_ParamIdx = SETUP_OUT4LFO; // Bypass all four jack ratios, and go directly to jack #4 LFO.
//				else if ((setup_ParamIdx == SETUP_OUT4RATIO) && (setup_Edited[SETUP_OUTSRATIOS] == 1) && (setup_Edited[SETUP_OUT4RATIO] != 12))
//					setup_ParamIdx = SETUP_CVTRIG; // In case custom output jack ratios, jack #4 ratio isn't x1, bypass jack #4 LFO & polarity, and go directly to CV/TRIG.
					else if ((setup_ParamIdx == SETUP_OUT4LFO) && (setup_Edited[SETUP_OUT4LFO] == 0))
						setup_ParamIdx = SETUP_CVTRIG; // Bypass LFO polarity entry is LFO mode is disabled, go directly to CV/TRIG.
						else setup_ParamIdx++; // Advance to next, in all other cases.

			// These variables are used to cycle display (parameter name, then its value). DEPRECATED!
			setupCounter = 0;
			if (setup_ParamIdx > SETUP_EXIT) {
				// Exiting SETUP. From here, all required actions on exit SETUP (such save, cancel changes, reset to default factory etc), except "Review" option!
				switch (setup_Edited[SETUP_EXIT]) {
					case 0:
						// Cancel/Exit: all changes from SETUP are ignored (changes are cancelled).
						// all previous (backuped) will be restored (any change is ignored).
						for (int i = 1; i < SETUP_EXIT; i++)
							setup_Current[i] = setup_Backup[i];
						// Restored pre-SETUP settings, so it's useless to save them as "json" persistent.
						UpdateKlokSpidSettings(false);
						break;
					case 1:
						// Save/Exit: all parameters from SETUP will be saved.
						for (int i = 1; i < SETUP_EXIT; i++)
							setup_Current[i] = setup_Edited[i];
						// Using new settings (because edited are saved), so it's mandatory to save them as "json" persistent datas.
						UpdateKlokSpidSettings(true);
						break;
					case 2:
						// Review: return to first parameter (don't exit "SETUP" in this choice is selected).
						setup_ParamIdx = 1;
						setup_CurrentValue = setup_Edited[1]; // Bypass the welcome message and edit first parameter.
						_tmpString = setupMenuName[1];
						strcpy(dmdTextMain1, _tmpString.c_str());
						dmdOffsetTextMain2 = setupParamXOffset[1][setup_CurrentValue];
						_tmpString = setupParamName[1][setup_CurrentValue];
						strcpy(dmdTextMain2, _tmpString.c_str());
						break;
					case 3:
						// Factory: restore all factory default parameters.
						for (int i = 1; i < SETUP_EXIT; i++)
							setup_Current[i] = setup_Factory[i];
						// Using new settings (because restored as default factory), like "Save/Exit", it's mandatory to save them as "json" persistent datas.
						UpdateKlokSpidSettings(true);
						break;
				}
				// Exit SETUP, except if "Review" was selected.
				if (setup_Edited[SETUP_EXIT] != 2) {
					// Exit SETUP (except if "Review" was selected).
					setupCounter = 0;
					// Clearing flag because now exit SETUP.
					isSetupRunning = false;
					// Update DMD to current running mode.
					if (activeCLK)
						updateDMDtoRunningMode(1); // Ratio (clock modulator mode).
						else updateDMDtoRunningMode(0); // Number of BPM.
				}
			}
			else if (setup_ParamIdx == SETUP_EXIT) {
				// Last default proposed parameter will be "Save and Exit" (SETUP).
				setup_CurrentValue = 1;
				// Update DMD.
				_tmpString = setupMenuName[setup_ParamIdx];
				strcpy(dmdTextMain1, _tmpString.c_str());
				dmdOffsetTextMain2 = setupParamXOffset[setup_ParamIdx][setup_CurrentValue];
				_tmpString = setupParamName[setup_ParamIdx][setup_CurrentValue];
				strcpy(dmdTextMain2, _tmpString.c_str());
			}
			else {
				// Set currently displayed (on DMD) value as current (edited) parameter.
				setup_CurrentValue = setup_Edited[setup_ParamIdx];
				// Update DMD.
				_tmpString = setupMenuName[setup_ParamIdx];
				strcpy(dmdTextMain1, _tmpString.c_str());
				dmdOffsetTextMain2 = setupParamXOffset[setup_ParamIdx][setup_CurrentValue];
				_tmpString = setupParamName[setup_ParamIdx][setup_CurrentValue];
				strcpy(dmdTextMain2, _tmpString.c_str());
			}
		}

		if (runButton.isHigh()) {
			// Button is held, don't matter if SETUP is running or not.
			// Always increment the counter.
			setupCounter++;
			if (isSetupRunning && isExitingSetup) {
				if (setupCounter >= 2 * sampleRate) {
					// Button was held during 2 seconds (while SETUP is running): now KlokSpid module exit SETUP (doing auto "Save/Exit").
					//
					isExitingSetup = false;
					setupCounter = 0;
					allowedButtonHeld = false; // Button must be released (retrigger is required).
					for (int i=0; i<SETUP_EXIT; i++)
						setup_Current[i] = setup_Edited[i];
					// Quick SETUP-exit, doing automatic "Save/exit", by this way using new settings (because edited are saved), so it's mandatory to save them as "json" persistent datas.
					UpdateKlokSpidSettings(true);
					// Clearing flag because now exit SETUP.
					isSetupRunning = false;
					// Update DMD regadling current running mode.
					if (activeCLK)
						updateDMDtoRunningMode(1); // Ratio (clock modulator mode).
						else updateDMDtoRunningMode(0); // Number of BPM.
				}
			}
			else {
				if (isEnteringSetup && (setupCounter >= 2 * sampleRate)) {
					// Button was finally held during 2 seconds: now KlokSpid module runs its SETUP. Initializing some variables/arrays/flags first.
					//
					isEnteringSetup = false;
					setupCounter = 0;
					// Button must be released (retrigger is required).
					allowedButtonHeld = false;
					// Menu entry #0 is used to display "- SETUP -" as welcome message (don't have parameters, so be sure "parameter" is set to 0).
					setup_Current[SETUP_WELCOME_MESSAGE] = 0;
					// Copy current parameters (since initialization or previous SETUP) to "edited" parameters before entering SETUP.
					// Also use a "backup" table in case of "Cancel/Exit" choice.
					for (int i = 0; i < SETUP_EXIT; i++) {
						setup_Backup[i] = setup_Current[i];
						setup_Edited[i] = setup_Current[i];
					}
					// Lastest menu entry is used to exit SETUP menu, by default with save.
					setup_Edited[SETUP_EXIT] = 1;
					// Select first parameter will ne displayed. In fact, the welcome message "- SETUP -" on DMD when entered SETUP.
					setup_ParamIdx = 0;
					// Update DMD.
					_tmpString = setupMenuName[SETUP_WELCOME_MESSAGE];
					strcpy(dmdTextMain1, _tmpString.c_str());
					dmdOffsetTextMain2 = setupParamXOffset[SETUP_WELCOME_MESSAGE][0];
					_tmpString = setupParamName[SETUP_WELCOME_MESSAGE][0];
					strcpy(dmdTextMain2, _tmpString.c_str());
					// This flag indicates SETUP is running.
					isSetupRunning = true;
				}
			}
		}
		else {
			if (isEnteringSetup) {
				// Abort entering SETUP.
				isEnteringSetup = false;
				// Button works as BPM start/stop toggle: inverting state.
				isBPMRunning = !isBPMRunning;
				// Persistence for current BPM-state (toJson).
				this->runBPMOnInit = isBPMRunning;
			}
			else if (isSetupRunning && isExitingSetup) {
				// Abort quick exit SETUP.
				isExitingSetup = false;
			}
			setupCounter = 0;
			allowedButtonHeld = false; // button must be "retriggered", to avoid continuous entry/exit SETUP while held.
		}

		// KlokSpid module's SETUP.
		if (isSetupRunning) {
			// SETUP is running.
			if (encoderDelta > 0) {
				// Incremented encoder (rotated clockwise).
				setup_CurrentValue++;
				if (setup_CurrentValue >= setup_NumValue[setup_ParamIdx])
					setup_CurrentValue = 0; // End of values list: return to first value.
				// Update DMD.
				_tmpString = setupMenuName[setup_ParamIdx];
				strcpy(dmdTextMain1, _tmpString.c_str());
				dmdOffsetTextMain2 = setupParamXOffset[setup_ParamIdx][setup_CurrentValue];
				_tmpString = setupParamName[setup_ParamIdx][setup_CurrentValue];
				strcpy(dmdTextMain2, _tmpString.c_str());
				// Update current parameter "in realtime".
				setup_Current[setup_ParamIdx] = setup_CurrentValue;
				// Update parameters, but without "jSon" persistence while SETUP is running!
				UpdateKlokSpidSettings(false);
				// Reset encoder move detection.
				encoderDelta = 0;
			}
			else if (encoderDelta < 0) {
				// Decremented encoder (rotated counter-clockwise).
				if (setup_NumValue[setup_ParamIdx] != 0) {
					if (setup_CurrentValue == 0)
						setup_CurrentValue = setup_NumValue[setup_ParamIdx] - 1; // Return to last possible value.
						else setup_CurrentValue--; //Previous value.
				}
				// Update DMD.
				_tmpString = setupMenuName[setup_ParamIdx];
				strcpy(dmdTextMain1, _tmpString.c_str());
				dmdOffsetTextMain2 = setupParamXOffset[setup_ParamIdx][setup_CurrentValue];
				_tmpString = setupParamName[setup_ParamIdx][setup_CurrentValue];
				strcpy(dmdTextMain2, _tmpString.c_str());
				// Update current parameter "in realtime".
				setup_Current[setup_ParamIdx] = setup_CurrentValue;
				// Update parameters, but without "jSon" persistence while SETUP is running!
				UpdateKlokSpidSettings(false);
				// Reset encoder move detection.
				encoderDelta = 0;
			}
		}

		// Handling LEDs on KlokSpid module (at the end of step).
	  lights[LED_SYNC_GREEN].setBrightness((((activeCLK && (isSync || (clkModulatorMode == X1))) || (!activeCLK && isBPMRunning)) ? 1.0 : 0.0)); // Unique "SYNC" LED: will be lit green color when sync'd / BPM is running.
	  lights[LED_SYNC_RED].setBrightness((((activeCLK && (isSync || (clkModulatorMode == X1))) || (!activeCLK && isBPMRunning)) ? 0.0 : 1.0));  // Unique "SYNC" LED: will be lit red color (opposite cases).
	  lights[LED_CLK].setBrightness(((isSetupRunning || ledClkAfterglow) ? 1.0 : 0.0));
		lights[LED_CV_TRIG].setBrightness(((isSetupRunning || activeCV) ? 1.0 : 0.0)); // TODO -- MUST BE ENHANCED!
		lights[LED_CVMODE].setBrightness(((isSetupRunning || activeCLK) ? 1.0 : 0.0));
		lights[LED_TRIGMODE].setBrightness(((isSetupRunning || !activeCLK) ? 1.0 : 0.0));
	} // end of "process"...

	json_t *dataToJson() override {
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "Theme", json_integer(Theme));
		json_object_set_new(rootJ, "bipolarCV", json_boolean(bipolarCV));
		json_object_set_new(rootJ, "pulseDurationExt", json_integer(pulseDurationExt));
		json_object_set_new(rootJ, "outVoltage", json_real(outVoltage));
		json_object_set_new(rootJ, "defOutRatios", json_boolean(defOutRatios)); // When true, all jacks are x1. Otherwise (false) any jack may have its specific ratio.
		json_object_set_new(rootJ, "out1Ratio", json_integer(outputRatio[0]));
		json_object_set_new(rootJ, "out2Ratio", json_integer(outputRatio[1]));
		json_object_set_new(rootJ, "out3Ratio", json_integer(outputRatio[2]));
		json_object_set_new(rootJ, "out4Ratio", json_integer(outputRatio[3]));
		json_object_set_new(rootJ, "jack4LFO", json_integer(jack4LFO));
		json_object_set_new(rootJ, "jack4LFObipolar", json_boolean(jack4LFObipolar));
		json_object_set_new(rootJ, "transportTrig", json_boolean(transportTrig)); // CV-RATIO/TRIG. port may be used as BPM "start/stop" toggle or as BPM-reset. BPM-reset is default factory (false).
		json_object_set_new(rootJ, "Ratio", json_integer(rateRatioByEncoder)); // Ratio set by encoder.
		json_object_set_new(rootJ, "BPM", json_integer(BPM)); // BPM set by encoder.
		json_object_set_new(rootJ, "runBPMOnInit", json_boolean(runBPMOnInit)); // State of BPM pulsing or stopped.
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override {
		// Retrieving module theme/variation (when loading .vcv and cloning module).
		json_t *ThemeJ = json_object_get(rootJ, "Theme");
		if (ThemeJ) {
			Theme = json_integer_value(ThemeJ);
			portMetal = Theme / 3; // first three use silver (0), last three use gold (1) - the int division by 3 is useful ;)
		}
		// Retrieving bipolar or unipolar mode (for CV when running as clock multiplier/divider).
		json_t *bipolarCVJ = json_object_get(rootJ, "bipolarCV");
		if (bipolarCVJ)
			bipolarCV = json_is_true(bipolarCVJ);
		// Retrieving pulse duration "mode" data. Introducted since v0.5.3.
		json_t *pulseDurationExtJ = json_object_get(rootJ, "pulseDurationExt");
		if (pulseDurationExtJ)
			pulseDurationExt = json_integer_value(pulseDurationExtJ);
		// Retrieving output voltage data (real/float value).
		json_t *outVoltageJ = json_object_get(rootJ, "outVoltage");
		if (outVoltageJ)
			outVoltage = json_real_value(outVoltageJ);
		// Retrieving if ratio par jack is disabled (all at x1), or enabled (each having its ratio).
		json_t *defOutRatiosJ = json_object_get(rootJ, "defOutRatios");
		if (defOutRatiosJ)
			defOutRatios = json_is_true(defOutRatiosJ);
		// Retrieving ratio for output jack #1 (when loading .vcv and cloning module).
		json_t *jack1BPMRateJ = json_object_get(rootJ, "out1Ratio");
		if (jack1BPMRateJ)
			outputRatio[0] = json_integer_value(jack1BPMRateJ);
		// Retrieving ratio for output jack #2 (when loading .vcv and cloning module).
		json_t *jack2BPMRateJ = json_object_get(rootJ, "out2Ratio");
		if (jack2BPMRateJ)
			outputRatio[1] = json_integer_value(jack2BPMRateJ);
		// Retrieving ratio for output jack #3 (when loading .vcv and cloning module).
		json_t *jack3BPMRateJ = json_object_get(rootJ, "out3Ratio");
		if (jack3BPMRateJ)
			outputRatio[2] = json_integer_value(jack3BPMRateJ);
		// Retrieving ratio for output jack #4 (when loading .vcv and cloning module).
		json_t *jack4BPMRateJ = json_object_get(rootJ, "out4Ratio");
		if (jack4BPMRateJ)
			outputRatio[3] = json_integer_value(jack4BPMRateJ);
		// Retrieving output jack #4 LFO mode (when loading .vcv and cloning module).
		json_t *jack4LFOJ = json_object_get(rootJ, "jack4LFO");
		if (jack4LFOJ)
			jack4LFO = json_integer_value(jack4LFOJ);
		// Retrieving bipolar or unipolar for jack #4 LFO.
		json_t *jack4LFObipolarJ = json_object_get(rootJ, "jack4LFObipolar");
		if (jack4LFObipolarJ)
			jack4LFObipolar = json_is_true(jack4LFObipolarJ);
		// Retrieving usage of TRIG. input port: start/stop toggle (true) or BPM-reset (false).
		json_t *transportTrigJ = json_object_get(rootJ, "transportTrig");
		if (transportTrigJ)
			transportTrig = json_is_true(transportTrigJ);
		// Retrieving ratio (clock modulator) set by encoder (when loading .vcv and cloning module).
		json_t *svRatioJ = json_object_get(rootJ, "Ratio");
		if (svRatioJ)
			svRatio = json_integer_value(svRatioJ);
		// Retrieving BPM (when loading .vcv and cloning module).
		json_t *svBPMJ = json_object_get(rootJ, "BPM");
		if (svBPMJ)
			svBPM = json_integer_value(svBPMJ);
		// Retrieving last saved BPM-clocking state (it was running or stopped).
		json_t *runBPMOnInitJ = json_object_get(rootJ, "runBPMOnInit");
		if (runBPMOnInitJ)
			runBPMOnInit = json_is_true(runBPMOnInitJ);
	}

};

// Dot-matrix display (DMD) and small displays (located around output jacks) handler.
struct KlokSpidDMD : TransparentWidget {
	KlokSpidModule *module;
	std::shared_ptr<Font> font;
	std::string fontPath;

	KlokSpidDMD() {
		fontPath = std::string(asset::plugin(pluginInstance, "res/fonts/LEDCounter7.ttf"));
	}

	void drawLayer(const DrawArgs &args, int layer) override {
		if (layer == 1) {
			if (!(font = APP->window->loadFont(fontPath)))
				return;
			// Yellow rounded rectangle to simulate yellow backlit of (LCD) dot-matrix display ("Absolute Night" model only).
			if (module) {
				if (module->Theme == 2) {
					// Main DMD.
					nvgBeginPath(args.vg);
					nvgRoundedRect(args.vg, 7.16f, 43.6f, 105.7f, 45.48f, 6.5f);
					nvgFillColor(args.vg, nvgRGBA(0xC0, 0xE9, 0x10, 0xff));
					nvgFill(args.vg);
					nvgClosePath(args.vg);
					// Lower DMD.
					nvgBeginPath(args.vg);
					nvgRoundedRect(args.vg, 33.14f, 282.95f, 53.71f, 29.35f, 3.3f);
					nvgFillColor(args.vg, nvgRGBA(0xC0, 0xE9, 0x10, 0xff));
					nvgFill(args.vg);
					nvgClosePath(args.vg);
				}
			}
			// Main DMD, upper line.
			nvgFontSize(args.vg, 16);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, -2);
			Vec textPos = Vec(14, box.size.y - 174);
			if (module) {
				nvgFillColor(args.vg, nvgTransRGBA(module->DMDtextColor, 0xff)); // Using current color for DMD.
				nvgText(args.vg, textPos.x, textPos.y, module->dmdTextMain1, NULL); // Proceeding module->dmdTextMain2 string (second line).
			}
			else {
				// Default message on DMD (LCD).
				nvgFillColor(args.vg, nvgTransRGBA(nvgRGB(0x08, 0x08, 0x08), 0xff)); // Using default black LCD.
				nvgText(args.vg, textPos.x, textPos.y, "Clk Generator", NULL); // Default message on first line.
			}
			// Main DMD, lower line.
			nvgFontSize(args.vg, 20);
			nvgTextLetterSpacing(args.vg, -1);
			textPos = Vec(12, box.size.y - 152);
			if (module)
				nvgText(args.vg, textPos.x + module->dmdOffsetTextMain2, textPos.y, module->dmdTextMain2, NULL); // Displaying module->dmdTextMain2 string (second line). The second line may have an horizontal offset.
				else nvgText(args.vg, textPos.x + 7, textPos.y, "120 BPM", NULL); // Default message on second line.
			// Lower DMD, display between output jacks, top-left (output #1).
			nvgFontSize(args.vg, 14);
			textPos = Vec(35, box.size.y + 61);
			if (module)
				nvgText(args.vg, textPos.x + module->dmdOffsetTextOut1, textPos.y, module->dmdTextMainOut1, NULL); // Displaying module->dmdTextMainOut1 string (top-left, related to output port #1).
				else nvgText(args.vg, textPos.x + 5, textPos.y, "X1", NULL); // Default message: X1.
			// Lower DMD, display between output jacks, top-right (output #2).
			textPos = Vec(62.5, box.size.y + 61);
			if (module)
				nvgText(args.vg, textPos.x + module->dmdOffsetTextOut2, textPos.y, module->dmdTextMainOut2, NULL); // Displaying module->dmdTextMainOut2 string (top-right, related to output port #2).
				else nvgText(args.vg, textPos.x + 5, textPos.y, "X1", NULL); // Default message: X1.
			// Lower DMD, display between output jacks, bottom-left (output #3).
			textPos = Vec(35, box.size.y + 75);
			if (module)
				nvgText(args.vg, textPos.x + module->dmdOffsetTextOut3, textPos.y, module->dmdTextMainOut3, NULL); // Displaying module->dmdTextMainOut3 string (bottom-left, related to output port #3).
				else nvgText(args.vg, textPos.x + 5, textPos.y, "X1", NULL); // Default message: X1.
			// Lower DMD, display between output jacks, bottom-right (output #4).
			textPos = Vec(62.5, box.size.y + 75);
			if (module)
				nvgText(args.vg, textPos.x + module->dmdOffsetTextOut4, textPos.y, module->dmdTextMainOut4, NULL); // Displaying module->dmdTextMainOut4 string (bottom-right, related to output port #4).
				else nvgText(args.vg, textPos.x + 5, textPos.y, "X1", NULL); // Default message: X1.
		}
		Widget::drawLayer(args, layer);
	}

};

///////////////////////////////////////////////////// CONTEXT-MENU //////////////////////////////////////////////////////

struct KlokSpidClassicMenu : MenuItem {
	KlokSpidModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 0; // Model: default Classic (beige).
		module->portMetal = 0; // Silver connectors for Classic.
	}
};

struct KlokSpidStageReproMenu : MenuItem {
	KlokSpidModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 1; // Model: Stage Repro.
		module->portMetal = 0; // Silver connectors for Stage Repro.
	}
};

struct KlokSpidAbsoluteNightMenu : MenuItem {
	KlokSpidModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 2; // Model: Absolute Night.
		module->portMetal = 0; // Silver connectors for Absolute Night.
	}
};

struct KlokSpidDarkSignatureMenu : MenuItem {
	KlokSpidModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 3; // Model: Dark Signature.
		module->portMetal = 1; // Gold connectors for Dark Signature.
	}
};

struct KlokSpidDeepblueSignatureMenu : MenuItem {
	KlokSpidModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 4; // Model: Deepblue Signature.
		module->portMetal = 1; // Gold connectors for Deepblue Signature.
	}
};

struct KlokSpidCarbonSignatureMenu : MenuItem {
	KlokSpidModule *module;
	void onAction(const event::Action &e) override {
		module->Theme = 5; // Model: Carbon Signature.
		module->portMetal = 1; // Gold connectors for Carbon Signature.
	}
};

struct KlokSpidSubMenuItems : MenuItem {
	KlokSpidModule *module;
	Menu *createChildMenu() override {
		Menu *menu = new Menu;

		KlokSpidClassicMenu *klokspidmenuitem1 = new KlokSpidClassicMenu;
		klokspidmenuitem1->text = "Classic (default)";
		klokspidmenuitem1->rightText = CHECKMARK(module->Theme == 0);
		klokspidmenuitem1->module = module;
		menu->addChild(klokspidmenuitem1);

		KlokSpidStageReproMenu *klokspidmenuitem2 = new KlokSpidStageReproMenu;
		klokspidmenuitem2->text = "Stage Repro";
		klokspidmenuitem2->rightText = CHECKMARK(module->Theme == 1);
		klokspidmenuitem2->module = module;
		menu->addChild(klokspidmenuitem2);

		KlokSpidAbsoluteNightMenu *klokspidmenuitem3 = new KlokSpidAbsoluteNightMenu;
		klokspidmenuitem3->text = "Absolute Night";
		klokspidmenuitem3->rightText = CHECKMARK(module->Theme == 2);
		klokspidmenuitem3->module = module;
		menu->addChild(klokspidmenuitem3);

		KlokSpidDarkSignatureMenu *klokspidmenuitem4 = new KlokSpidDarkSignatureMenu;
		klokspidmenuitem4->text = "Dark \"Signature\"";
		klokspidmenuitem4->rightText = CHECKMARK(module->Theme == 3);
		klokspidmenuitem4->module = module;
		menu->addChild(klokspidmenuitem4);

		KlokSpidDeepblueSignatureMenu *klokspidmenuitem5 = new KlokSpidDeepblueSignatureMenu;
		klokspidmenuitem5->text = "Deepblue \"Signature\"";
		klokspidmenuitem5->rightText = CHECKMARK(module->Theme == 4);
		klokspidmenuitem5->module = module;
		menu->addChild(klokspidmenuitem5);

		KlokSpidCarbonSignatureMenu *klokspidmenuitem6 = new KlokSpidCarbonSignatureMenu;
		klokspidmenuitem6->text = "Carbon \"Signature\"";
		klokspidmenuitem6->rightText = CHECKMARK(module->Theme == 5);
		klokspidmenuitem6->module = module;
		menu->addChild(klokspidmenuitem6);

		return menu;
	}
};

///////////////////////////////////////////////// MODULE WIDGET SECTION /////////////////////////////////////////////////

struct KlokSpidWidget : ModuleWidget {
	// Panels (one per "Theme").
	SvgPanel *panelKlokSpidClassic;
	SvgPanel *panelKlokSpidStageRepro;
	SvgPanel *panelKlokSpidAbsoluteNight;
	SvgPanel *panelKlokSpidDarkSignature;
	SvgPanel *panelKlokSpidDeepBlueSignature;
	SvgPanel *panelKlokSpidCarbonSignature;
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
	// Silver button.
	SvgSwitch *buttonSilver;
	// Gold button.
	SvgSwitch *buttonGold;

	KlokSpidWidget(KlokSpidModule *module) {
		setModule(module);
		box.size = Vec(8 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT);
		// Classic (default) beige panel.
		panelKlokSpidClassic = new SvgPanel();
		panelKlokSpidClassic->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/KlokSpid_Classic.svg")));
		panelKlokSpidClassic->visible = true;
		addChild(panelKlokSpidClassic);
		// Stage Repro panel.
		panelKlokSpidStageRepro = new SvgPanel();
		panelKlokSpidStageRepro->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/KlokSpid_Stage_Repro.svg")));
		panelKlokSpidStageRepro->visible = false;
		addChild(panelKlokSpidStageRepro);
		// Absolute Night panel.
		panelKlokSpidAbsoluteNight = new SvgPanel();
		panelKlokSpidAbsoluteNight->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/KlokSpid_Absolute_Night.svg")));
		panelKlokSpidAbsoluteNight->visible = false;
		addChild(panelKlokSpidAbsoluteNight);
		// Dark Signature panel.
		panelKlokSpidDarkSignature = new SvgPanel();
		panelKlokSpidDarkSignature->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/KlokSpid_Dark_Signature.svg")));
		panelKlokSpidDarkSignature->visible = false;
		addChild(panelKlokSpidDarkSignature);
		// Deepblue Signature panel.
		panelKlokSpidDeepBlueSignature = new SvgPanel();
		panelKlokSpidDeepBlueSignature->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/KlokSpid_Deepblue_Signature.svg")));
		panelKlokSpidDeepBlueSignature->visible = false;
		addChild(panelKlokSpidDeepBlueSignature);
		// Deepblue Signature panel.
		panelKlokSpidCarbonSignature = new SvgPanel();
		panelKlokSpidCarbonSignature->setBackground(APP->window->loadSvg(asset::plugin(pluginInstance, "res/KlokSpid_Carbon_Signature.svg")));
		panelKlokSpidCarbonSignature->visible = false;
		addChild(panelKlokSpidCarbonSignature);
		// The DMD display.
		{
			KlokSpidDMD *display = new KlokSpidDMD();
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
		// Input ports (ports are using "dynamic ports" to change connector metal - silver or gold: big thanks to Marc Boul矩.
		addInput(createDynamicPort<DynSVGPort>(Vec(24, 215), true, module, KlokSpidModule::INPUT_CLOCK, module ? &module->portMetal : NULL));
		addInput(createDynamicPort<DynSVGPort>(Vec(72, 215), true, module, KlokSpidModule::INPUT_CV_TRIG, module ? &module->portMetal : NULL));
		// Output ports (ports are using "dynamic ports" to change connector metal - silver or gold: big thanks to Marc Boul矩.
		addOutput(createDynamicPort<DynSVGPort>(Vec(10, 261), false, module, KlokSpidModule::OUTPUT_1, module ? &module->portMetal : NULL));
		addOutput(createDynamicPort<DynSVGPort>(Vec(86, 261), false, module, KlokSpidModule::OUTPUT_2, module ? &module->portMetal : NULL));
		addOutput(createDynamicPort<DynSVGPort>(Vec(10, 310), false, module, KlokSpidModule::OUTPUT_3, module ? &module->portMetal : NULL));
		addOutput(createDynamicPort<DynSVGPort>(Vec(86, 310), false, module, KlokSpidModule::OUTPUT_4, module ? &module->portMetal : NULL));
		// Multipurpose continuous encoder (used to select BPM, modulator ratio or setup items).
		addParam(createParam<KS_Encoder>(Vec(20, 106), module, KlokSpidModule::PARAM_ENCODER));
		// Push button (gold), used to toggle START/STOP, also used to enter SETUP, and to advance to next parameter in SETUP.
		buttonGold = createParam<KS_ButtonGold>(Vec(94, 178), module, KlokSpidModule::PARAM_BUTTON);
		addParam(buttonGold);
		// Push button (silver), used to toggle START/STOP, also used to enter SETUP, and to advance to next parameter in SETUP.
		buttonSilver = createParam<KS_ButtonSilver>(Vec(94, 178), module, KlokSpidModule::PARAM_BUTTON);
		addParam(buttonSilver);
		// SYNC LED (can be lit green when sync'd, or red during resync).
		addChild(createLight<MediumLight<GreenRedLight>>(Vec(7, 96), module, KlokSpidModule::LED_SYNC_GREEN)); // Unified SYNC LED (green/red).
		// Small-sized orange LEDs near CV-RATIO/TRIG input port (when lit, each LED indicates the port role).
		addChild(createLight<SmallLight<KlokSpidOrangeLight>>(Vec(67.5, 206), module, KlokSpidModule::LED_CVMODE));
		addChild(createLight<SmallLight<KlokSpidOrangeLight>>(Vec(95, 206), module, KlokSpidModule::LED_TRIGMODE));
		// LEDs (red for CLK input, yellow for CV-RATIO/TRIG input).
		addChild(createLight<MediumLight<RedLight>>(Vec(31.5, 242), module, KlokSpidModule::LED_CLK));
		addChild(createLight<MediumLight<KlokSpidOrangeLight>>(Vec(79.5, 242), module, KlokSpidModule::LED_CV_TRIG));
	}

	void step() override {
		KlokSpidModule *module = dynamic_cast<KlokSpidModule*>(this->module);
		if (module) {
			// Possible alternate panel themes (GUIs).
			panelKlokSpidClassic->visible = (module->Theme == 0);
			panelKlokSpidStageRepro->visible = (module->Theme == 1);
			panelKlokSpidAbsoluteNight->visible = (module->Theme == 2);
			panelKlokSpidDarkSignature->visible = (module->Theme == 3);
			panelKlokSpidDeepBlueSignature->visible = (module->Theme == 4);
			panelKlokSpidCarbonSignature->visible = (module->Theme == 5);
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
			// Silver or gold button is visible at once (opposite is, obvisouly, hidden).
			buttonSilver->visible = (module->Theme < 3);
			buttonGold->visible = (module->Theme > 2);
		}
		else {
			// Default panel theme is always "Classic" (beige, using silver screws, using silver button, LCD).
			// Other panels are, of course, hidden.
			panelKlokSpidClassic->visible = true;
			panelKlokSpidStageRepro->visible = false;
			panelKlokSpidAbsoluteNight->visible = false;
			panelKlokSpidDarkSignature->visible = false;
			panelKlokSpidDeepBlueSignature->visible = false;
			panelKlokSpidCarbonSignature->visible = false;
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
			// Silver button is used for default Classic...
			buttonSilver->visible = true;
			// ...and, of course, golden button is hidden.
			buttonGold->visible = false;
		}
		ModuleWidget::step();
	}

	void appendContextMenu(Menu *menu) override {
		KlokSpidModule *module = dynamic_cast<KlokSpidModule*>(this->module);
		menu->addChild(new MenuEntry);
		KlokSpidSubMenuItems *klokspidsubmenuitems = new KlokSpidSubMenuItems;
		klokspidsubmenuitems->text = "Model";
		klokspidsubmenuitems->rightText = RIGHT_ARROW;
		klokspidsubmenuitems->module = module;
		menu->addChild(klokspidsubmenuitems);
	}

};

Model *modelKlokSpid = createModel<KlokSpidModule, KlokSpidWidget>("KlokSpid");
