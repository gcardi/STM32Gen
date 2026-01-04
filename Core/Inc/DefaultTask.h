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

extern bool GetRunCmdState();

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

	enum class KnobMult {
		M1, M10, M100
	};

	Knob selectedKnob_ { Knob::Freq };
	KnobMult selectedknobMult_ { KnobMult::M1 };
	uint16_t freq_ { 127 };
	uint8_t ampl_ { 20 };
	bool oldRunState_ {};
	bool updateGUI_ {};

	//CntValueType GetFreq() const;
	CntValueType GetKnobValue() const;
	bool GetBtnState() const;
	bool GetEncBtnState() const;
	bool GetRunCmdBtnState() const;
	void UpdateValueFromSelectedKnob();
	void UpdatAllValues();
	void UpdateSelectedKnobFromValue();
	void InitGUI();
	void UpdateGUIRequest();
	bool HaveToUpdateGUI() const { return updateGUI_; };
	void UpdateGUI();
	void NextKnob();
	bool FreqKnobActive() const;
	bool AmplKnobActive() const;
	void NextKnobMult();
	void UpdateKnobMultFromValue();
	void InitRunState();
	void ResetRunState();
	bool CheckRunState();
	void ReadParameters();
};

#endif

#endif /* APPDEFTASK_H_ */
