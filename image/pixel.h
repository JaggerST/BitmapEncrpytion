#pragma once

// DO NOT CHANGE THIS ENUM
enum class ChannelOrder {
	BGR,
	BRG,
	GBR,
	GRB,
	RBG,
	RGB
};

template<class _Ty>
class Pixel {

	_Ty red_channel{ 0 };
	_Ty green_channel{ 0 };
	_Ty blue_channel{ 0 };
	

public:

	static inline ChannelOrder channel_order{ ChannelOrder::RGB }; //??? soll das nicht aenderbar sein ??? c++ read file all bytes

	Pixel() {};
	Pixel(const _Ty& redIntensity, const _Ty& greenIntensity, const _Ty& blueIntensity) : red_channel{ redIntensity }, green_channel{ greenIntensity }, blue_channel{ blueIntensity } {}

	inline bool operator==(const Pixel& rhs) const {
		return (blue_channel == rhs.blue_channel && green_channel == rhs.green_channel && red_channel == rhs.red_channel);
	}

	_Ty get_red_channel() const {
		return red_channel;
	}

	_Ty get_green_channel() const {
		return green_channel;
	}

	_Ty get_blue_channel() const {
		return blue_channel;
	}

};
