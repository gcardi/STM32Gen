/*
 * DefaultTask.h
 *
 */

#ifndef DEFAULTTASK_H_
#define DEFAULTTASK_H_

#include "Encoder.h"

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
	CntValueType GetFreq() const;
	CntValueType ResetFreq( CntValueType Value );
	bool GetBtnState() const;
	bool GetEncBtnState() const;


};

#endif

#endif /* APPDEFTASK_H_ */
