/*
 * DefaultTask.cpp
 *
 */

#include <stdio.h>

#include "main.h"
#include "cmsis_os.h"
#include "DefaultTask.h"
#include "ssd1306.h"
#include "stm32f4xx_it.h"
#include "WaveGenTask.h"

Encoders Encs;

void DefaultTaskExecute()
{
	static DefaultTask Task;
	Task.Execute();
}
//------------------------------------------------------

void ProcessEncoders( uint32_t EncPort )
{
	Encs.AddValue( EncPort );
}
//------------------------------------------------------

DefaultTask::DefaultTask()
{
	ssd1306_Init();
}
//------------------------------------------------------

DefaultTask::CntValueType DefaultTask::GetKnobValue() const
{
	return Encs.GetChn();
}
//------------------------------------------------------

bool DefaultTask::GetBtnState() const
{
	return LL_GPIO_IsInputPinSet( BTN_GPIO_Port, BTN_Pin );
}
//------------------------------------------------------

bool DefaultTask::GetEncBtnState() const
{
	return LL_GPIO_IsInputPinSet( ENC_PB_GPIO_Port, ENC_PB_Pin );
}
//------------------------------------------------------

static void DbgShowFreq( auto Val )
{
	printf( "\r%6d\t", Val );
	fflush(stdout);
}
//------------------------------------------------------

void DefaultTask::Execute()
{
	Encs.Init( ReadEncodersPort() );

	UpdateSelectedKnobFromValue();

	InitGUI();

	enum class FState { Idle, Tg };

	FState State = FState::Idle;

	for ( size_t x {} ; ; ++x ) {
		if ( Encs.Update( 32 ) ) {
			UpdateValueFromSelectedKnob();

			if ( ~( x & 15 ) ) {
			    UpdateGUI();
			}
		}
		//vTaskDelay( pdMS_TO_TICKS( 10 ) );
		switch ( State ) {
			case FState::Idle:
				if ( GetBtnState() ) {
					NextKnob();
					UpdateSelectedKnobFromValue();
					UpdateGUI();
			    	State = FState::Tg;
				}
				break;
			case FState::Tg:
				if ( !GetBtnState() ) {
			    	State = FState::Idle;
				}
				break;
		}

	}
}
//------------------------------------------------------

void DefaultTask::UpdateValueFromSelectedKnob()
{
	switch ( selectedKnob_ ) {
		case Knob::Freq:
			freq_ = static_cast<uint16_t>( GetKnobValue() );
			WaveGen.SetFreq( freq_ );
			break;
		case Knob::Ampl:
			ampl_ = static_cast<uint8_t>( GetKnobValue() );
			WaveGen.SetAmpl( ampl_ / 100.0F );
			break;
	}
}
//------------------------------------------------------

void DefaultTask::UpdateSelectedKnobFromValue()
{
	switch ( selectedKnob_ ) {
		case Knob::Freq:
			Encs.SetChn( freq_, 10, 1000 );
			break;
		case Knob::Ampl:
			Encs.SetChn( ampl_, 0, 100 );
			break;
	}
}
//------------------------------------------------------

void DefaultTask::InitGUI()
{
	ssd1306_Fill( BgColor );
	UpdateGUI();
}
//------------------------------------------------------

bool DefaultTask::FreqKnobActive() const
{
	return selectedKnob_ == Knob::Freq;
}
//------------------------------------------------------

bool DefaultTask::AmplKnobActive() const
{
	return selectedKnob_ == Knob::Ampl;
}
//------------------------------------------------------

void DefaultTask::UpdateGUI()
{
	char Text[32];

	ssd1306_SetCursor( 0, 0 );
	sprintf( Text, FreqKnobActive() ? ">F:%4u Hz" : " F:%4u Hz", freq_ );
	ssd1306_WriteString( Text, Font_11x18, FgColor );

	ssd1306_SetCursor( 0, 20 );
	sprintf( Text, AmplKnobActive() ? ">A: %3d%%" : " A: %3d%%", static_cast<int>( ampl_ ) );
	ssd1306_WriteString( Text, Font_11x18, FgColor );


//	ssd1306_SetCursor( 0, 40 );
//	ssd1306_WriteString( "    ^", Font_11x18, FgColor );

	ssd1306_UpdateScreen();
}
//------------------------------------------------------

void DefaultTask::NextKnob()
{
	switch ( selectedKnob_ ) {
		case Knob::Freq:
			selectedKnob_ = Knob::Ampl;
			break;
		case Knob::Ampl:
			selectedKnob_ = Knob::Freq;
			break;
	}

}
//------------------------------------------------------
