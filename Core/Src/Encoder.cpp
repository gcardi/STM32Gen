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
		//printf( "%08lX\r\n", Data );
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
	ProcessChn<0>( oldPort_, Val );
	/*
	ProcessChn<1>( oldPort_, Val );
	ProcessChn<2>( oldPort_, Val );
	ProcessChn<3>( oldPort_, Val );
	ProcessChn<4>( oldPort_, Val );
	*/
	//btns_.set( 5, Val & 0x8000 );
}
//-------------------------------------------------------------------------


