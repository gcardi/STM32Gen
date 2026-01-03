/*
 * DefaultTask.h
 *
 */

#ifndef DEFAULTTASK_H_
#define DEFAULTTASK_H_

#include "Encoder.h"
#include "ssd1306.h"

#if defined( __cplusplus )
extern "C" {
#endif

void ProcessEncoders( uint32_t EncPort );

void DefaultTaskExecute();

#if defined( __cplusplus )
}

class DefaultTask {
public:
	using CntValueType = Encoders::CntValueType;

	DefaultTask();
	void Execute();
private:
	static constexpr auto FgColor = White;
	static constexpr auto BgColor = Black;

	enum class Knob {
		Freq, Ampl
	};

	Knob selectedKnob_ { Knob::Freq };
	uint16_t freq_ { 127 };
	uint8_t ampl_ { 100 };

	//CntValueType GetFreq() const;
	CntValueType GetKnobValue() const;
	bool GetBtnState() const;
	bool GetEncBtnState() const;
	void UpdateValueFromSelectedKnob();
	void UpdateSelectedKnobFromValue();
	void InitGUI();
	void UpdateGUI();
	void NextKnob();
	bool FreqKnobActive() const;
	bool AmplKnobActive() const;
};

#endif

#endif /* APPDEFTASK_H_ */
