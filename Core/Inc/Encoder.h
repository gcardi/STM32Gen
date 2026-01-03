#ifndef ENCODER_H_
#define ENCODER_H_

#if defined( __cplusplus )

#include <cstdint>
#include <array>
#include <limits>
#include <algorithm>

#include "RingBuffer.h"

class Encoders {
public:
	//using ValEvtType = uint8_t;
	using ValEvtType = uint32_t;
	using CntValueType = int16_t;
	using CntRefType = CntValueType&;

	static constexpr auto Scale = 2;

	Encoders() {
		ClampChns();
	}

	Encoders( ValEvtType InitialPortValue )
	  : oldPort_{ InitialPortValue }
	{
		ClampChns();
	}

	void Init( ValEvtType InitialPortValue = {} ) {
		oldPort_ = InitialPortValue;
	}

	// From irqs
	void AddValue( ValEvtType Val );

	// From main
	bool Update();
	bool Update( size_t EventCnt );


	template<size_t N>
	constexpr CntValueType GetChn() const {
		static_assert( N < MaxChns );
		return chns_[N] >> Scale;
	}

	CntValueType GetChn( size_t Idx ) const {
		return chns_[Idx] >> Scale;
	}

	template<size_t N>
	constexpr CntValueType SetChn( CntValueType Val ) {
		static_assert( N < MaxChns );
		chns_[N] = Clamp( Val << Scale );
		return chns_[N] >> Scale;
	}

	CntValueType SetChn( size_t Idx, CntValueType Val ) {
		chns_[Idx] = Clamp( Val << Scale );
		return chns_[Idx] >> Scale;
	}

private:
	RingBuffer<ValEvtType,64> buffer_;

	ValEvtType oldPort_ {};

	static constexpr size_t MaxChns = 8;

	std::array<CntValueType,MaxChns> chns_ = {0};

	[[nodiscard]]
	bool ProcessPort( ValEvtType Val );

	void ProcessDeltas( ValEvtType Val );

	//template<size_t S>
	constexpr CntValueType Clamp( CntValueType Val ) const {
		return std::clamp(
		    Val,
			static_cast<CntValueType>( 10 << Scale ),
			static_cast<CntValueType>( 150 << Scale )
		);
	}

	void ClampChns() {
		for ( auto& Chn : chns_ ) {
			Chn = Clamp( Chn );
		}
	}

	template<typename T, size_t N>
	using SquaredMatrix = std::array<std::array<T, N>, N>;

	template<size_t S>
	inline void ProcessChn( ValEvtType Old, ValEvtType New )
	{
		static constexpr SquaredMatrix<int8_t, 4> Delta = {{
			{{  0, +1, -1,  0 }},
			{{ -1,  0,  0, +1 }},
			{{ +1,  0,  0, -1 }},
			{{  0, -1, +1,  0 }},
		}};

		static constexpr auto NBits = 2;
		static constexpr auto BitMask = ( 1 << ( NBits + 1 ) ) - 1;
		static constexpr auto FirstBitAt = 0;

		//auto OldIdx = ( Old >> ( S * NBits + FirstBitAt) ) & BitMask;
		//auto NewIdx = ( New >> ( S * NBits + FirstBitAt ) ) & BitMask;
		auto DVal =
			Delta[( Old >> ( S * NBits + FirstBitAt) ) & BitMask]
				 [( New >> ( S * NBits + FirstBitAt) ) & BitMask];
		chns_[S] = Clamp( chns_[S] + DVal );
		//printf( "\r\nO=%d\tN=%d\tD=%d\tV=%d", (int)( Old & BitMask ), (int)(New & BitMask), (int)DVal, (int)( chns_[S] >> 2 ) );
		//fflush( stdout );
	}
};

#endif
#endif
