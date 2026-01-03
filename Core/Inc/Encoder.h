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

	Encoders() = default;

	Encoders( ValEvtType InitialPortValue )
	  : oldPort_{ InitialPortValue }
	{
	}

	void Init( ValEvtType InitialPortValue = {} ) {
		oldPort_ = InitialPortValue;
	}

	// From irqs
	void AddValue( ValEvtType Val );

	// From main
	bool Update();
	bool Update( size_t EventCnt );


	constexpr CntValueType GetChn() const {
		return value_ >> Scale;
	}

	CntValueType GetChn( size_t Idx ) const {
		return value_ >> Scale;
	}

	constexpr CntValueType SetChn( CntValueType Val, CntValueType Min, CntValueType Max ) {
		min_ = Min;
		max_ = Max;
		value_ = Clamp( Val << Scale );
		return value_ >> Scale;
	}

	CntValueType GetInc10Exp() const { return factor_; }
	void SetInc10Exp( CntValueType Val ) { factor_ = Val; }

private:
	RingBuffer<ValEvtType,128> buffer_;

	ValEvtType oldPort_ {};

	static constexpr size_t MaxChns = 8;

	CntValueType value_ {};
	CntValueType min_ {};
	CntValueType max_ {};
	CntValueType factor_ { 0 };

	[[nodiscard]]
	bool ProcessPort( ValEvtType Val );

	void ProcessDeltas( ValEvtType Val );

	//template<size_t S>
	constexpr CntValueType Clamp( CntValueType Val ) const {
		return std::clamp(
		    Val,
			static_cast<CntValueType>( min_ << Scale ),
			static_cast<CntValueType>( max_ << Scale )
		);
	}

	template<typename T, size_t N>
	using SquaredMatrix = std::array<std::array<T, N>, N>;

	inline void ProcessChn( ValEvtType Old, ValEvtType New )
	{
		static constexpr SquaredMatrix<int8_t, 4> Delta = {{
			{{  0, +1, -1,  0 }},
			{{ -1,  0,  0, +1 }},
			{{ +1,  0,  0, -1 }},
			{{  0, -1, +1,  0 }},
		}};

		static constexpr std::array<CntValueType,4> Pow10 {{
			1, 10, 100, 1000
		}};

		static constexpr auto NBits = 2;
		static constexpr auto BitMask = ( 1 << ( NBits + 1 ) ) - 1;
		static constexpr auto FirstBitAt = 0;
		static constexpr auto StartBitPair = 0;

		auto DVal =
			Delta[( Old >> ( StartBitPair * NBits + FirstBitAt) ) & BitMask]
				 [( New >> ( StartBitPair * NBits + FirstBitAt) ) & BitMask];
		value_ = Clamp( value_ + DVal );
		//value_ = Clamp( value_ + DVal * Pow10[ std::clamp( factor_, 0, Pow10.size() -1 ) ] );
		//printf( "\r\nO=%d\tN=%d\tD=%d\tV=%d", (int)( Old & BitMask ), (int)(New & BitMask), (int)DVal, (int)( value_ >> 2 ) );
		//fflush( stdout );
	}
};

#endif
#endif
