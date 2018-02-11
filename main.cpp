#include <cstring>

#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>

#include "util/args.h"

#include "lib/hex_convert.h"
#include "lib/byte_pack.h"

struct output_definition {
	const char* code;
	std::size_t bytes;
};

enum output_category {
	OUT_BYTE,
	OUT_SHORT,
	OUT_WORD,
};

static const output_definition eaOutDefinitions[] {
	{ "BYTE",  1 }, // OUT_BYTE
	{ "SHORT", 2 }, // OUT_SHORT
	{ "WORD",  4 }, // OUT_WORD
};

struct bin2ea_config {
	output_category outType = OUT_WORD;
	bool toStdOut = false;
	bool toDefine = false;
	bool newLine = true;

	std::string labelName;
	std::string before;
	std::string after;

	std::string outputFileName;
	std::string inputFileName;
};

/*
std::size_t predict_ea_code_capacity(std::size_t count, output_category type) {
	return std::strlen(eaOutDefinitions[type].code) + count * (eaOutDefinitions[type].bytes*2 + 2);
}

template<typename IntIteratorType>
std::size_t predict_ea_code_capacity(IntIteratorType begin, IntIteratorType end, output_category type) {
	return predict_ea_code_capacity(std::distance(begin, end), type);
}

template<typename ByteIteratorType>
std::size_t predict_ea_bytes_capacity(ByteIteratorType begin, ByteIteratorType end, output_category type) {
	std::size_t size = std::distance(begin, end);
	std::size_t result = predict_ea_code_capacity(size / eaOutDefinitions[type].bytes, type);
	std::size_t remains = size % eaOutDefinitions[type].bytes;

	if (!remains)
		return result;

	return result + predict_ea_code_capacity(remains, OUT_BYTE) + 1;
}
//*/

template<typename OutputIteratorType, typename InputIteratorType>
OutputIteratorType write_range(OutputIteratorType out, InputIteratorType begin, InputIteratorType end) {
	return std::copy(begin, end, out);
}

template<typename OutputIteratorType, typename CharType>
OutputIteratorType write_cstring(OutputIteratorType out, const CharType* cstring) {
	while (*cstring)
		*(out++) = *(cstring++);
	return out;
}

template<typename OutputIteratorType, typename CharType>
OutputIteratorType write_char(OutputIteratorType out, CharType chr) {
	return *out++ = chr, out;
}

template<typename OutputIteratorType, typename IntegerType>
OutputIteratorType write_ea_number(OutputIteratorType out, IntegerType value) {
	return hex_convert<IntegerType>::write_digits(write_char(out, '$'), value);
}

template<typename OutputIteratorType, typename IntIteratorType>
OutputIteratorType write_ea_numbers(OutputIteratorType out, IntIteratorType begin, IntIteratorType end) {
	for (auto it = begin; it != end; ++it)
		out = write_ea_number(write_char(out, ' '), *it);

	return out;
}

template<typename OutputIteratorType, typename IntIteratorType>
OutputIteratorType write_ea_code(OutputIteratorType out, IntIteratorType begin, IntIteratorType end, output_category type) {
	out = write_cstring(out, eaOutDefinitions[type].code);
	out = write_ea_numbers(out, begin, end);

	return out;
}

template<typename OutputIteratorType, typename ByteIteratorType>
OutputIteratorType write_ea_bytes(OutputIteratorType out, ByteIteratorType begin, ByteIteratorType end, output_category type) {
	auto itAlign = end;
	while (std::distance(begin, itAlign) % eaOutDefinitions[type].bytes)
		itAlign--;

	out = write_ea_code(out,
		byte_packer<std::uint32_t>(begin,   eaOutDefinitions[type].bytes),
		byte_packer<std::uint32_t>(itAlign, eaOutDefinitions[type].bytes),
	type);

	if (itAlign != end)
		out = write_ea_code(write_char(out, ';'), itAlign, end, OUT_BYTE);

	return out;
}

template<typename OutputIteratorType, typename WriteNameFuncType, typename WriteContentFuncType>
OutputIteratorType write_ea_definition(OutputIteratorType out, WriteNameFuncType writeName, WriteContentFuncType writeContent) {
	out = write_cstring(out, "#define ");
	out = writeName(out);
	out = write_cstring(out, " \"");
	out = writeContent(out);
	out = write_cstring(out, "\"");
}

template<typename OutputIteratorType, typename WriteNameFuncType, typename WriteContentFuncType>
OutputIteratorType write_ea_labelled(OutputIteratorType out, WriteNameFuncType writeName, WriteContentFuncType writeContent) {
	out = writeName(out);
	out = write_cstring(out, ":\n\t");
	out = writeContent(out);
}

void run(const bin2ea_config& config) {
	std::vector<std::uint8_t> data; {
		std::ifstream inputStream;

		if (!(inputStream.open(config.inputFileName, std::ios::in | std::ios::binary), inputStream.is_open()))
			throw std::runtime_error(std::string("Couldn't open file for read: ").append(config.inputFileName));

		inputStream.seekg(0, std::ios::end);
		std::size_t inputSize = inputStream.tellg();

		if (!inputSize)
			throw std::runtime_error(std::string("Input file is empty"));

		data.resize(inputSize);

		inputStream.seekg(0);
		inputStream.read(reinterpret_cast<char*>(data.data()), inputSize);
	}

	using input_iterator_type = std::ostream_iterator<char>;

	auto writeContent = [&data, &config] (input_iterator_type it) -> input_iterator_type {
		if (config.before.size())
			it = write_char(write_range(it, config.before.begin(), config.before.end()), ';');

		it = write_ea_bytes(it, data.begin(), data.end(), config.outType);

		if (config.after.size())
			it = write_range(write_char(it, ';'), config.after.begin(), config.after.end());

		return it;
	};

	auto writeName = [&config] (input_iterator_type it) -> input_iterator_type {
		return write_range(it, config.labelName.begin(), config.labelName.end());
	};

	auto writeAll = [&config, &writeName, &writeContent] (input_iterator_type it) -> input_iterator_type {
		if (config.labelName.empty())
			return writeContent(it);
		else if (config.toDefine)
			return write_ea_definition(it, writeName, writeContent);
		else
			return write_ea_labelled(it, writeName, writeContent);
	};

	if (config.toStdOut)
		writeAll(std::ostream_iterator<char>(std::cout));
	else {
		std::ofstream outputStream;

		if (!(outputStream.open(config.outputFileName), outputStream.is_open()))
			throw std::runtime_error(std::string("Couldn't open file for write: ").append(config.outputFileName));

		writeAll(std::ostream_iterator<char>(outputStream));
	}
}

int main(int argc, char** argv) {
	// TODO: change the way arguments are parsed, because this is kinda bad
	Arguments args(argc, argv);

	if (argc == 1) {
		std::cout << "Usage: " << argv[0]
				  << " <input> [output/--to-stdout] [-byte/-short/-word] [-define <name>/-label <name>] [-before <before>] [-after <after>] [-no-newline]"
				  << std::endl;

		return 0;
	}

	try {
		bin2ea_config config;

		auto nextString = [&args] () -> const char* {
			Argument result = args.next();

			if (result.type == Argument::End)
				throw std::runtime_error("Reached end of arguments while expecting literal");
			else if (result.type == Argument::Option)
				throw std::runtime_error("Got option argument while expecting literal");

			return result.value;
		};

		while (Argument arg = args.next()) {
			if (arg.type == Argument::Option) {
				if (!std::strcmp(arg.value, "-to-stdout"))
					config.toStdOut = true;
				else if (!std::strcmp(arg.value, "byte"))
					config.outType = OUT_BYTE;
				else if (!std::strcmp(arg.value, "short"))
					config.outType = OUT_SHORT;
				else if (!std::strcmp(arg.value, "word"))
					config.outType = OUT_WORD;
				else if (!std::strcmp(arg.value, "no-newline"))
					config.newLine = false;
				else if (!std::strcmp(arg.value, "before"))
					config.before = nextString();
				else if (!std::strcmp(arg.value, "after"))
					config.after = nextString();
				else if (!std::strcmp(arg.value, "define")) {
					config.toDefine = true;
					config.labelName = nextString();
				} else if (!std::strcmp(arg.value, "label")) {
					config.toDefine = false;
					config.labelName = nextString();
				}
			} else if (arg.type == Argument::LiteralString) {
				if (config.inputFileName.empty())
					config.inputFileName = arg.value;
				else if (config.outputFileName.empty())
					config.outputFileName = arg.value;
			}
		}

		run(config);
	} catch (const std::exception& e) {
		std::cerr << "BIN2EA ERROR - " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
