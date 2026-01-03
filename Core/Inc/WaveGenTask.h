/*
 * WaveGenTask.h
 *
 */

#ifndef INC_WAVEGENTASK_H_
#define INC_WAVEGENTASK_H_

#if defined( __cplusplus )
extern "C" {
#endif

void WaveGenTaskExecute( osThreadId_t TaskHandle );

#if defined( __cplusplus )
}

#include <cstdint>
#include <array>

class WaveGenTask {
public:
	using SampleType = int16_t;

	void Execute( osThreadId_t TaskHandle );
	osThreadId_t GetTaskHandle() const { return taskHandle_; }
	uint16_t GetFreq() const { return freq_; }
	void SetFreq( uint16_t Val );
private:
	static constexpr size_t SampleCount = 2048;
	static constexpr size_t ChnCount = 2;
	static constexpr size_t BufferSize = SampleCount * ChnCount;
	//static constexpr float SampleRate = 40000.0F;
	float sampleRate_ {};

	using BufferType = std::array<SampleType,BufferSize>;

	osThreadId_t taskHandle_;
	BufferType buffer_;
    //float phase_ {};
	float phase_;
	volatile uint16_t freq_ { 127 };

	void FillHalfBuffer( bool FirstHalf );
	void FillBuffer();
};

extern WaveGenTask WaveGen;

#endif



#endif /* INC_WAVEGENTASK_H_ */
