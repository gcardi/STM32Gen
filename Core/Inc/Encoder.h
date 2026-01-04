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
	using CntValueType = int32_t;
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

	CntValueType GetIncFactIdx() const { return incFactIdx_; }
	void SetIncFactIdx( CntValueType Val ) { incFactIdx_ = Val; }

private:
	RingBuffer<ValEvtType,128> buffer_;

	ValEvtType oldPort_ {};

	static constexpr size_t MaxChns = 8;

	CntValueType value_ {};
	CntValueType min_ {};
	CntValueType max_ {};
	CntValueType incFactIdx_ { 0 };

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

		static constexpr std::array<CntValueType,3> DeltaVal {{
			1, 8, 100
		}};

		static constexpr auto NBits = 2;
		static constexpr auto BitMask = ( 1 << ( NBits + 1 ) ) - 1;
		static constexpr auto FirstBitAt = 0;
		static constexpr auto StartBitPair = 0;

		auto DVal =
			Delta[( Old >> ( StartBitPair * NBits + FirstBitAt) ) & BitMask]
				 [( New >> ( StartBitPair * NBits + FirstBitAt) ) & BitMask];
		//value_ = Clamp( value_ + DVal );
		if ( DVal ) {
			auto DeltaIdx = std::clamp( incFactIdx_, CntValueType{}, static_cast<CntValueType>( DeltaVal.size() ) );
			value_ = Clamp( value_ + DVal * DeltaVal[DeltaIdx] );
			/*
			switch ( incFactIdx_ ) {
				case 1:
					//value_ = Clamp( value_ + DVal * 10 );
					value_ += DVal * 8;
					value_ = Clamp( value_ );
					break;
				case 2:
					value_ = Clamp( value_ + DVal * 100 );
					break;
				default:
					value_ = Clamp( value_ + DVal );
					break;

			}
			printf( "\r\nO=%d\tN=%d\tD=%d\tPV=%d\tV=%d", (int)( Old & BitMask ), (int)(New & BitMask), (int)DVal, (int)value_, (int)( value_ >> 2 ) );
			fflush( stdout );
			*/
		}
	}
};

#endif
#endif
