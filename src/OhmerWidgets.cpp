////////////////////////////////////////////////////////////////////////////////////////////////////
// OhmerWidgets.cpp
// Based on "IMWidgets.cpp" - stuff made by Marc Boulé (Impromptu Modular developer).
// Used for "dynamic SVG" ports.
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "OhmerWidgets.hpp"

// Dynamic SVGPort.

void DynamicSVGPort::addFrame(std::shared_ptr<Svg> svg) {
	frames.push_back(svg);
	if (frames.size() == 1)
		SvgPort::setSvg(svg);
}

void DynamicSVGPort::step() {
	if (mode != NULL && *mode != oldMode) {
		if (*mode > 0 && !frameAltName.empty()) {
			// JIT loading of alternate skin.
			frames.push_back(APP->window->loadSvg(frameAltName));
			frameAltName.clear();// don't reload!
		}
		sw->setSvg(frames[*mode]);
		oldMode = *mode;
		fb->dirty = true;
	}
	PortWidget::step();
}
