/*
 * WaveGenTask.cpp
 *
 */

#include <cmath>
#include <iterator>

#include "main.h"
#include "cmsis_os.h"
#include "WaveGenTask.h"
#include "WaveGenTask.h"

using std::begin;
using std::next;

WaveGenTask WaveGen;

void WaveGenTaskExecute( osThreadId_t TaskHandle )
{
	WaveGen.Execute( TaskHandle );
}
//------------------------------------------------------

void WaveGenTask::SetFreq( uint16_t Val )
{
	freq_ = Val;
}
//------------------------------------------------------

void WaveGenTask::Execute( osThreadId_t TaskHandle )
{
	taskHandle_ = TaskHandle;

	FillBuffer();
	HAL_I2S_Transmit_DMA( &hi2s2, reinterpret_cast<uint16_t*>( buffer_.data( )), buffer_.size() );
	for ( ;; ) {
	    uint32_t flags = osThreadFlagsWait(
	        0x03,                  // HT | TC
	        osFlagsWaitAny,
	        osWaitForever
	    );

	    if ( flags & 0x02 ) {
	        // riempi prima metà buffer
	    	LL_GPIO_SetOutputPin(LED_GPIO_Port, LED_Pin );
	    	FillHalfBuffer( 0 );
	    	LL_GPIO_ResetOutputPin(LED_GPIO_Port, LED_Pin );
	    	//LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	    	//LL_GPIO_ResetOutputPin(LED_GPIO_Port, LED_Pin );
	    }
	    else if (flags & 0x01) {
	        // riempi seconda metà buffer
	    	FillHalfBuffer( 1 );
	    	//LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	    	//LL_GPIO_SetOutputPin(LED_GPIO_Port, LED_Pin );
	    }
	}
}
//------------------------------------------------------

void WaveGenTask::FillHalfBuffer( bool FirstHalf )
{
    float PhaseInc = static_cast<float>( M_TWOPI ) * static_cast<float>( freq_ ) / SampleRate;
    auto Begin = begin( buffer_ );
    auto End = next( Begin, buffer_.size() / 2 );
    if ( !FirstHalf ) {
    	Begin = End;
    	End = end( buffer_ );
    }
    while ( Begin != End ) {
        auto fn = sinf( phase_ );
        auto Sample = static_cast<BufferType::value_type>( fn * 32767.0f );
        *Begin++ = Sample;
        if ( Begin != End ) {
            *Begin++ = Sample;
        }
        phase_ += PhaseInc;
        if ( phase_ >= 2.0f * M_PI ) {
            phase_ -= 2.0f * M_PI;
        }
    }
}
//------------------------------------------------------

void WaveGenTask::FillBuffer()
{
	FillHalfBuffer( true );
	FillHalfBuffer( false );
}
//------------------------------------------------------

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	//LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    osThreadFlagsSet( WaveGen.GetTaskHandle(), 1 );
}
//------------------------------------------------------

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	//LL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
    osThreadFlagsSet( WaveGen.GetTaskHandle(), 2 );
}
//------------------------------------------------------
