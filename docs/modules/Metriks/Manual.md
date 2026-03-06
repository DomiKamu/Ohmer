# ![Ohmer Modules logo](_img/Logo_Ohmer.png)Ohmer Modules - Metriks

#### **INTRODUCTION**

***Metriks*** is a 8 HP *CPU-controlled* metering/visual module, designed for VCV Rack: for now, two features are fully operational: **Voltmeter** and **CV Pitch**.

Please notice other features (aka "modes") such **BPM Meter** and **Peak Counter** are actually disabled, as long as these modes still in development. Thanks for patience!

Also, please consider **Frequency Counter** feature/mode is definitively cancelled.


#### **MODULE LAYOUT**

Like all Ohmer Modules (except RKD and BRK modules), Metriks exists in six models (theme variations):

![Metriks module](_img/Metriks.png)


Models are **Creamy**, **Stage Repro**, **Absolute Night**, **Dark "Signature"**, Deepblue "Signature" and **Titanium "Signature"**. You'll can select any model you'll want, via the contextual menu (right-mouse click over the Metriks module).

First three models (non-Signature line) use "cheap" silver metal jacks, buttons and screws, and embed a black LCD dot-matrix display (DMD). However, *Absolute Night* model uses a yellow-backlit DMD.

Last three models (*Signature* line) use *expensive* golden jacks, buttons and screws, and embed a plasma-gas DMD, instead.

Below the DMD, it's the **continuous encoder** (unlike knob/potentiometer, it doesn't have min./max. limits). Its main goal is to select next mode (when turned clockwise) or previous mode (when turned counter-clockwise). Also, as will be explained later, the continous encoder is used to select next or previous possible parameter, while you're changing an option (for current mode).

At right side of the continuous encoder, you can find **OPT.** button, and its related LED (unlit, or red). This button is useful to change some options for current mode.

Just below, the **PLAY/PAUSE** and **RESET** buttons and jacks, are actually unused (they will be used for future *Peak Counter* mode only).

At the bottom of the module, the **IN** jack is... the input, used for signal you'll want to meter.

The **OUT** jack is a "replica" of INput jack, useful to insert another module(s) behind Metriks (daisy chain).


#### **QUICK USAGE GUIDE**

When you bring a new instance of Metriks module from Rack's module browser, the module is:

- Model: Creamy (can be changed from contextual menu - aka *right-mouse click* menu over the module, list from **Model** menu item).
- Mode: Voltmeter (it displays voltage applied on **IN** jack, realtime, always using +/- sign, decimals set to **Auto** by default).
- Display blinking **? Input ?** as long as **IN** jack remains disconnected.

![](_img/Metriks_01.png)


As soon as you connect **IN** jack to a source voltage, voltage is displayed like this:

![](_img/Metriks_02.png)

Now for training, it's time to change some options (for current mode, in this case: voltmeter).

Voltmeter have two options:

- The **Metering** behavior: choices between **Realtime** for realtime voltage measurements, **Minimum** keeps the minimum registered voltage, **Maximum** keeps the maximum registered voltage, and **Median** is a median between max. and min. registered voltages.
- The number of displayed decimals, may be **Auto** (the module selects how many decimals it can display), and **0** to **3** in case you'll want a specific number of decimals.

In order to change options, simply press the **OPT.** button: now its red LED is blinking, and the first option you'll can edit also is blinking at the bottom of DMD... for voltmeter, the first option is "Metering":

![](_img/Metriks_05.png)

While red **OPT.** LED (and bottom line on DMD) is blinking, just rotate the continuous encoder...

- Clockwise, to select next possible choice for the current option.
- Counter-clockwise to select previous possible choice.

Now, by pressing **OPT.** button once again, the second option, number of decimals, can be changed.

For *Decimals*, you'll can choose either from "0" to "3", or "Auto" (default is "Auto"), by rotating the continous encoder:

![](_img/Metriks_03.png)

When done, press the **OPT.** button in order to exit options and to return to production.

Also, while blinking, if you don't touch either the continuous encoder or **OPT.** button, a 10-seconds timeout will automatically exit options and return to production.

Alternate way to change any option is to use contextual menu. Depending the current mode, the contextual menu displays relevant options, and their possible choices in a submenu.
As "Voltmeter", the contextual menu offers two additional options (below "Model"): **Metering**, and **Decimals**.

![](_img/Metriks_04.png)

When voltmeter is set for **Min.**, **Max.** or **Medn.** (median), the **RESET** button (or its trigger-based input jack) will clear (return to 0) all previously registered voltages.

Some modes provides only one option (the future **Peak Counter**, to choose the threshold voltage), some other modes have two options (for example, the **Voltmeter** and the **CV Pitch**). Future **BPM Meter** mode will don't provide any option (in this case, the **OPT.** button remains inoperative, and no additional contextual menu).


#### **THE CV PITCH MODE**

Formerly named "CV Tuner", but replaced by more explicit **CV Pitch**, is a mode who displays a note-equivalent, regardling **constant voltage**, as pitch (like V/Oct) applied on **IN** jack. This mode may help you to tweak, as example, a CV-based sequencer (to avoid usage of a quantizer module).

From **Voltmeter** mode, just rotate the continous encoder clockwise, in order to select **CV Pitch** (as next mode).

Like any mode, **CV Pitch** must be display on the top of the DMD.

Supported voltage range is from **C-1** (**Do-1**) at **-5V**, upto **B9** (**Si9**) at **+5.917V**. Otherwise a question mark "'?" will be displayed as "out of range".

This mode provides two options:

- **Notation**: may be standard English **C D E... B** notation (default), or latin **Do Re Mi... Si** notation.
- **Sharps/Flats**: using sharps (#) - default, or flats (b) note names.

Press **OPT.** button once to change notation, then press **OPT.** again to change sharps/flats. Third press will exit options and return to production.

The **<<** / **<** / **>** or **>>** indicator alongside the displayed note name/octave:

- Two indicators (**<<** or **>>**) means **coarse tuning** is required. Left **<<** means the note is far "below": in this case, decrease the voltage (from voltage source). On the same way, right **>>** means the note is far "above", in this case, increase the voltage to reach the voltage corresponding to the displayed note.
- One indicator (**<** or **>**) means **fine tuning** is required (left **<** is meaning the note is a bit "below", just **decrease the voltage a bit** (from voltage source). On the same way, right **>** means the note is a bit "above", in this case, **increase the voltage a bit** to reach the voltage corresponding to the displayed note.
- When the voltage is "perfect" (small tolerance), direction indicators will disappear: gotcha, you've found the precise voltage for desired note!
