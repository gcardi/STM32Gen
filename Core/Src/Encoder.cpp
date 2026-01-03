#include <cstdint>
#include <cstdio>
#include <inttypes.h>

#include "Encoder.h"
#include "RingBuffer.h"

void Encoders::AddValue( ValEvtType Val )
{
	buffer_.push( Val );
}
//-------------------------------------------------------------------------

bool Encoders::Update()
{
	return Update( std::numeric_limits<size_t>::max() );
}
//-------------------------------------------------------------------------

bool Encoders::Update( size_t EventCnt )
{
	ValEvtType Data;
	size_t ProcCnt {};
	while ( EventCnt-- && buffer_.pop( Data ) ) {
		if ( ProcessPort( Data ) ) {
			++ProcCnt;
		}
	}
	return ProcCnt;
}
//-------------------------------------------------------------------------

bool Encoders::ProcessPort( ValEvtType Val )
{
	if ( Val != oldPort_ ) {
		ProcessDeltas( Val );
		oldPort_ = Val;
		return true;
	}
	return false;
}
//-------------------------------------------------------------------------

void Encoders::ProcessDeltas( ValEvtType Val )
{
	ProcessChn( oldPort_, Val );
}
//-------------------------------------------------------------------------


