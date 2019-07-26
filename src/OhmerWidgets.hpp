////////////////////////////////////////////////////////////////////////////////////////////////////
// OhmerWidgets.hpp
// Based on "IMWidgets.hpp" - stuff made by Marc Boulé (Impromptu Modular developer).
// Used for "dynamic SVG" ports.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "rack.hpp"

using namespace rack;

// General dynamic port creation.

template <class TDynamicPort>
TDynamicPort* createDynamicPort(Vec pos, bool isInput, Module *module, int portId, int* mode) {
	TDynamicPort *dynPort = isInput ? createInput<TDynamicPort>(pos, module, portId) : createOutput<TDynamicPort>(pos, module, portId);
	dynPort->mode = mode;
	return dynPort;
}

struct DynamicSVGPort : SvgPort {
	int* mode = NULL;
	int oldMode = -1;
	std::vector<std::shared_ptr<Svg>> frames;
	std::string frameAltName;
	void addFrame(std::shared_ptr<Svg> svg);
	void addFrameAlt(std::string filename) {frameAltName = filename;}
	void step() override;
};
