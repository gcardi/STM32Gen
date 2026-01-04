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
#include "StorageData.h"

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
	ReadParameters();
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

bool DefaultTask::GetRunCmdBtnState() const
{
	return LL_GPIO_IsInputPinSet( RUN_GPIO_Port, RUN_Pin );
}
//------------------------------------------------------

static bool IsRunning {};

bool GetRunCmdState()
{
	return IsRunning;
}
//------------------------------------------------------

//#define AT24C16_ADDR  (0x50 << 1)

void DefaultTask::Execute()
{
	puts( "Default Task Started\r\n" );

	/*
	uint8_t data = 0xAB;
	HAL_I2C_Mem_Write(
	    &hi2c3,
	    AT24C16_ADDR,
	    0x10,                    // indirizzo EEPROM
	    I2C_MEMADD_SIZE_8BIT,
	    &data,
	    1,
	    HAL_MAX_DELAY
	);
	uint8_t data;
	HAL_I2C_Mem_Read(
	    &hi2c3,
	    AT24C16_ADDR,
	    0x10,
	    I2C_MEMADD_SIZE_8BIT,
	    &data,
	    1,
	    HAL_MAX_DELAY
	);

	printf( "E²PROM @ 0x10 -> %02Xh\r\n", (int)data );

	*/

	ReadDataFromEEPROM( ampl_, freq_ );

	Encs.Init( ReadEncodersPort() );

	WaveGen.SetFreq( freq_ );
	WaveGen.SetAmpl( ampl_ / 100.0F );

	UpdateSelectedKnobFromValue();

	InitRunState();
	InitGUI();

	enum class FState { Idle, Tg, Tk, Tr };

	FState State = FState::Idle;

	size_t n {};

	auto OldFreq = freq_;
	auto OldAmpl = ampl_;

	TickType_t LastTickCount = xTaskGetTickCount();

	for ( size_t x {} ; ; ++x ) {

		if ( OldFreq == freq_ && OldAmpl == ampl_ ) {
			LastTickCount = xTaskGetTickCount();
		}
		else if ( xTaskGetTickCount() - LastTickCount > 10000 ) {
			OldFreq = freq_;
			OldAmpl = ampl_;
printf( "SALVO\r\n" );
			WriteDataToEEPROM( ampl_, freq_ );
		}



		if ( Encs.Update( 32 ) ) {
			UpdateValueFromSelectedKnob();
//			if ( ~( x & 15 ) ) {
			UpdateGUIRequest();
//			}
		}
		//vTaskDelay( pdMS_TO_TICKS( 10 ) );
		if ( ( x % 1024 ) == 0 ) {
			switch ( State ) {
				case FState::Idle:
					if ( GetBtnState() ) {
						NextKnob();
						UpdateSelectedKnobFromValue();
						UpdateGUIRequest();
						State = FState::Tg;
	//printf( "Idle -> Tg\r\n" );
					}
					else if ( GetEncBtnState() ) {
						NextKnobMult();
						UpdateKnobMultFromValue();
						UpdateGUIRequest();
						State = FState::Tk;
	//printf( "Idle -> Tk\r\n" );
					}
					else if ( GetRunCmdBtnState() ) {
						if ( IsRunning ) {
							IsRunning = false;
							State = FState::Tr;
	//printf( "Idle -> Tr (is running)\r\n" );
						}
						else {
							if ( ++n > 1000 ) {
								State = FState::Tr;
								IsRunning = true;
	//printf( "Idle -> Tr (with delay)\r\n" );
							}
						}
					}
					else {
						n = {};
					}
					break;
				case FState::Tg:
					if ( !GetBtnState() ) {
						State = FState::Idle;
	//printf( "Tg -> Idle\r\n" );
					}
					break;
				case FState::Tk:
					if ( !GetEncBtnState() ) {
						State = FState::Idle;
	//printf( "Tk -> Idle\r\n" );
					}
					break;
				case FState::Tr:
					if ( !GetRunCmdBtnState() ) {
						State = FState::Idle;
	//printf( "Tr -> Idle\r\n" );
					}
					break;
			}
		}
		if ( CheckRunState() ) {
			ResetRunState();
			UpdateGUIRequest();
		}
		if ( HaveToUpdateGUI() ) {
			UpdateGUI();
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
			Encs.SetChn( freq_, 10, 20000 );
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

void DefaultTask::UpdateGUIRequest()
{
	updateGUI_ = true;
}
//------------------------------------------------------

void DefaultTask::UpdateGUI()
{
	char Text[32];

	ssd1306_FillCircle( 5, 7, 3, FreqKnobActive() ? FgColor : BgColor );
	ssd1306_SetCursor( 11, 0 );
	sprintf( Text, "%5u Hz", freq_ );
	ssd1306_WriteString( Text, Font_11x18, FgColor );

	ssd1306_FillCircle( 5, 27, 3, AmplKnobActive() ? FgColor : BgColor );
	ssd1306_SetCursor( 33, 20 );
	sprintf( Text, "%3d %%", static_cast<int>( ampl_ ) );
	ssd1306_WriteString( Text, Font_11x18, FgColor );

	ssd1306_SetCursor( 33, 40 );
	sprintf(
	    Text,
		"%c%c%c",
		selectedknobMult_ == KnobMult::M100 ? '^' : ' ',
		selectedknobMult_ == KnobMult::M10 ? '^' : ' ',
		selectedknobMult_ == KnobMult::M1 ? '^' : ' '
	);
	ssd1306_WriteString( Text, Font_11x18, FgColor );

	ssd1306_SetCursor( 74, 48 );
	sprintf( Text, "%s", GetRunCmdState() ? " RUN" : "STOP" );
	ssd1306_WriteString( Text, Font_7x10, FgColor );

	ssd1306_UpdateScreen();

	updateGUI_ = false;
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
			if ( selectedknobMult_ == KnobMult::M100 ) {
				selectedknobMult_ = KnobMult::M10;
			}
			break;
	}

}
//------------------------------------------------------

void DefaultTask::NextKnobMult()
{
	switch( selectedknobMult_ ) {
		case KnobMult::M1:
			selectedknobMult_ = KnobMult::M10;
			break;
		case KnobMult::M10:
			selectedknobMult_ = selectedKnob_ == Knob::Ampl ? KnobMult::M10 : KnobMult::M100;
			break;
		case KnobMult::M100:
			selectedknobMult_ = KnobMult::M1;
			break;
	}
}
//------------------------------------------------------

void DefaultTask::UpdateKnobMultFromValue()
{
	switch ( selectedknobMult_ ) {
	case KnobMult::M1:
		Encs.SetIncFactIdx( 0 );
		break;
	case KnobMult::M10:
		Encs.SetIncFactIdx( 1 );
		break;
	case KnobMult::M100:
		Encs.SetIncFactIdx( 2 );
		break;
	}

}
//------------------------------------------------------

void DefaultTask::InitRunState()
{
	oldRunState_ = !GetRunCmdState();
}
//------------------------------------------------------

void DefaultTask::ResetRunState()
{
	oldRunState_ = GetRunCmdState();
}
//------------------------------------------------------

bool DefaultTask::CheckRunState()
{
	return oldRunState_ != GetRunCmdState();
}
//------------------------------------------------------

void DefaultTask::ReadParameters()
{

}
//------------------------------------------------------

