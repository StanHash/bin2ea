#ifndef BYTE_PACK_H
#define BYTE_PACK_H

#include <iterator>

struct little_endian_tag {};
struct big_endian_tag {};

template<typename OutputIntegerType, typename InputByteIteratorType>
OutputIntegerType pack_bytes(InputByteIteratorType begin, InputByteIteratorType end, little_endian_tag) {
	OutputIntegerType result(0);

	for (auto it = begin; it != end; ++it)
		result |= (*it) << std::distance(begin, it)*8;

	return result;
}

template<typename IntegerType, typename ByteIteratorType>
void unpack_bytes(IntegerType value, ByteIteratorType begin, ByteIteratorType end, little_endian_tag) {
	for (auto it = begin; it != end; ++it)
		(*it) = (value >> std::distance(begin, it)*8) & 0xFF;
}

template<typename OutputIntegerType, typename InputByteIteratorType>
OutputIntegerType pack_bytes(InputByteIteratorType begin, InputByteIteratorType end, big_endian_tag) {
	OutputIntegerType result(0);

	for (auto it = begin; it != end; ++it)
		result |= (*it) << std::distance(it, std::prev(end))*8;

	return result;
}

template<typename IntegerType, typename ByteIteratorType>
void unpack_bytes(IntegerType value, ByteIteratorType begin, ByteIteratorType end, big_endian_tag) {
	for (auto it = begin; it != end; ++it)
		(*it) = ((value >> std::distance(it, std::prev(end))*8) & 0xFF);
}

template<typename EndianTag = little_endian_tag, typename OutputIntegerType, typename InputByteIteratorType>
OutputIntegerType pack_bytes(InputByteIteratorType begin, InputByteIteratorType end) {
	return pack_bytes<OutputIntegerType, InputByteIteratorType>(begin, end, EndianTag());
}

template<typename EndianTag = little_endian_tag, typename IntegerType, typename ByteIteratorType>
void unpack_bytes(IntegerType value, ByteIteratorType begin, ByteIteratorType end) {
	return unpack_bytes<IntegerType, ByteIteratorType>(value, begin, end, EndianTag());
}

// TODO: non-hardcoded packer
template<typename OutputIntegerType, typename InputByteIteratorType, typename EndianTag = little_endian_tag>
struct byte_packing_iterator {
	using value_type        = OutputIntegerType;
	using difference_type   = void;
	using pointer           = void;
	using reference         = void;
	using iterator_category = std::input_iterator_tag;

public:
	byte_packing_iterator(InputByteIteratorType it, std::size_t bytes)
		: mIt(it), mBytes(bytes) {}

	byte_packing_iterator& operator ++ () {
		std::advance(mIt, mBytes);
	}

	value_type operator * () const {
		return pack_bytes<EndianTag, OutputIntegerType, InputByteIteratorType>(mIt, std::next(mIt, mBytes));
	}

	bool operator == (const byte_packing_iterator& other) const { return other.mIt == mIt; }
	bool operator != (const byte_packing_iterator& other) const { return other.mIt != mIt; }

private:
	InputByteIteratorType mIt;
	const std::size_t mBytes;
};

template<typename OutputIntegerType, typename EndianTag = little_endian_tag, typename InputByteIteratorType>
byte_packing_iterator<OutputIntegerType, InputByteIteratorType, EndianTag> byte_packer(InputByteIteratorType it, std::size_t bytes) {
	return byte_packing_iterator<OutputIntegerType, InputByteIteratorType, EndianTag>(it, bytes);
}

#endif // BYTE_PACK_H
