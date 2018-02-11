#ifndef HEX_CONVERT_H
#define HEX_CONVERT_H

#include <type_traits>

template<typename IntegerType>
struct hex_convert {
	static_assert(std::is_integral<IntegerType>::value && std::is_unsigned<IntegerType>::value, "");

	static std::size_t get_digit_count(IntegerType value) {
		std::size_t count = 0;

		do {
			++count;
			value >>= 4;
		} while (value);

		return count;
	}

	static char get_digit(IntegerType value) {
		value &= 0xF;

		if (value < 10)
			return '0' + value;
		return 'A' + (value - 10);
	}

	template<typename OutputIteratorType>
	static OutputIteratorType write_digits(OutputIteratorType out, IntegerType value, std::size_t digits) {
		for (std::size_t i=0; i<digits; ++i)
			*out++ = get_digit(value >> (digits-i-1)*4);

		return out;
	}

	template<typename OutputIteratorType>
	static OutputIteratorType write_digits(OutputIteratorType out, IntegerType value) {
		return write_digits<OutputIteratorType>(out, value, get_digit_count(value));
	}
};

#endif // HEX_CONVERT_H
