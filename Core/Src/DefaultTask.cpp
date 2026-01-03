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

DefaultTask::CntValueType DefaultTask::GetFreq() const
{
	return Encs.GetChn<0>();
}
//------------------------------------------------------

DefaultTask::CntValueType DefaultTask::ResetFreq( CntValueType Value )
{
	return Encs.SetChn<0>( Value );
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
	auto constexpr FgColor = White;
	auto constexpr BgColor = Black;

	static char Text[60];

	ResetFreq( 127 );

	Encs.Init( ReadEncodersPort() );

	auto OldFreq = GetFreq();

	//DbgShowFreq( OldFreq );

	ssd1306_Fill( BgColor );
	ssd1306_UpdateScreen();

	/* Infinite loop */
	for(uint32_t i {}; ; ++i ) {
		/*
		ssd1306_SetCursor( 0, 0 );
		sprintf( Text, "%08lX", i );
		ssd1306_WriteString( Text, Font_11x18, FgColor );
		ssd1306_UpdateScreen();
		*/
		if ( Encs.Update( 16 ) ) {
			auto Freq = GetFreq();
			if ( OldFreq != Freq ) {
				OldFreq = Freq;
				WaveGen.SetFreq( OldFreq );
				//DbgShowFreq( OldFreq );
				ssd1306_SetCursor( 0, 0 );
				sprintf( Text, "%4d", OldFreq );
				ssd1306_WriteString( Text, Font_11x18, FgColor );
				ssd1306_UpdateScreen();
			}
			//LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		}
		taskYIELD();
	}

}
//------------------------------------------------------


