/*
 * WaveGenTask.cpp
 *
 * CMSIS-RTOS2 version
 */

#include <cmath>
#include <iterator>
#include <cstdint>
#include <cstring>

#include "main.h"
#include "cmsis_os.h"
#include "WaveGenTask.h"

using std::begin;
using std::next;


// Numero di campioni nella LUT
constexpr size_t LUT_SIZE = 4096;

// Ampiezza massima int16_t
constexpr float AMP = 32767.0f;

// Genera la LUT a compile-time
constexpr int16_t generateSample(size_t i) {
    return static_cast<int16_t>(AMP * std::sin(M_TWOPI * i / LUT_SIZE));
}

constexpr auto createSineLUT() {
    std::array<int16_t,LUT_SIZE> lut = {};
    for (size_t i = 0; i < LUT_SIZE; ++i) {
        lut[i] = generateSample(i);
    }
    return lut;
}

// Lut in flash
constexpr auto SineLUT = createSineLUT();


WaveGenTask WaveGen;

//------------------------------------------------------
void WaveGenTaskExecute(osThreadId_t TaskHandle)
{
    WaveGen.Execute(TaskHandle);
}

//------------------------------------------------------
void WaveGenTask::SetFreq(float Val)
{
    osKernelLock();    // CMSIS equivalent of critical section
    freq_ = Val;
    osKernelUnlock();
}

//------------------------------------------------------
void WaveGenTask::SetAmpl(float Val)
{
    osKernelLock();
    ampl_ = Val;
    osKernelUnlock();
}

//------------------------------------------------------
#define AUDIO_HT (1 << 0)
#define AUDIO_TC (1 << 1)

// Global task handle for ISR
osThreadId_t gTaskHandle;

//------------------------------------------------------
void WaveGenTask::Execute(osThreadId_t TaskHandle)
{
    gTaskHandle = TaskHandle;

    FillBuffer();
    HAL_I2S_Transmit_DMA(&hi2s2, reinterpret_cast<uint16_t*>(buffer_.data()), buffer_.size());

    for (;;) {
        // Attende le flag DMA: metà buffer o buffer completo
        uint32_t flags = osThreadFlagsWait(
            AUDIO_HT | AUDIO_TC,  // bits to wait
            osFlagsWaitAny,
            osWaitForever         // blocca finché non arriva notifica
        );

        // Gestione notifiche
        if (flags & AUDIO_HT) {
            FillHalfBuffer(true);
        }

        if (flags & AUDIO_TC) {
            FillHalfBuffer(false);
        }

        // Dorme per lasciare spazio ai task a priorità più bassa
        //osDelay(5); // 5 ms, puoi regolare
    }
}

//------------------------------------------------------

void WaveGenTask::FillHalfBuffer(bool FirstHalf)
{
	float Freq;
    float Ampl;

    osKernelLock();
    Freq = freq_;
    Ampl = ampl_;
    osKernelUnlock();

	float PhaseInc = Freq / hi2s2.Init.AudioFreq;

	auto Begin = begin(buffer_);
	auto End = next(Begin, buffer_.size() / 2);

	if (!FirstHalf) {
		Begin = End;
		End = end(buffer_);
	}

    if ( GetRunCmdState() ) {
		while (Begin != End) {
			auto idx = static_cast<size_t>( phase_ * LUT_SIZE ) % LUT_SIZE;
			auto Sample = Ampl * SineLUT[idx];
			*Begin++ = Sample;
			if (Begin != End) {
				*Begin++ = Sample;
			}
			phase_ += PhaseInc;
			if ( phase_ >= 1.0f ) phase_ -= 1.0f;
		}
    }
    else {
		while (Begin != End) {
			*Begin++ = {};
		}
    }
}

//------------------------------------------------------

void WaveGenTask::FillBuffer()
{
    //printf("AudioFreq=%lu\r\n", hi2s2.Init.AudioFreq);
    FillHalfBuffer(true);
    FillHalfBuffer(false);
}

//------------------------------------------------------

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    // Notifica il task audio dalla ISR per metà buffer completo
	osThreadFlagsSet(gTaskHandle, AUDIO_HT);
}

//------------------------------------------------------
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    // Notifica il task audio dalla ISR per fine buffer completo
	osThreadFlagsSet(gTaskHandle, AUDIO_TC);
}

//------------------------------------------------------
