#include <cstdio>
#include <cstring>
#include <string>
#include <stdexcept>

#include "util/args.h"

enum OutputType {
	OutByte  = 1,
	OutShort = 2,
	OutWord  = 4
};

struct RunConfig {
	OutputType outType = OutByte;
	bool toStdOut = false;
	bool toDefine = false;
	bool newLine = true;

	std::string labelName;
	std::string before;
	std::string after;

	std::string outputFileName;
	std::string inputFileName;
};

std::string::value_type toHexDigit(std::uint32_t value) {
	value = (value & 0xF);

	if (value < 10)
		return '0' + value;
	return 'A' + (value - 10);
}

std::string toHexDigits(std::uint32_t value, int digits) {
	std::string result;
	result.resize(digits);

	for (int i=0; i<digits; ++i)
		result[digits-i-1] = toHexDigit(value >> (4*i));

	return result;
}

std::string binToNumberLiterals(const std::vector<std::uint32_t>& bin, int digits) {
	std::string result;
	result.reserve(bin.size()*(digits+3)); // +3 for " 0x"

	for (std::uint32_t value : bin)
		result.append(" 0x").append(toHexDigits(value, digits));

	return result;
}

std::string binToEACode(const std::vector<std::uint32_t>& bin, OutputType type) {
	std::string result;

	result.append((type == OutByte) ? "BYTE" : ((type == OutShort) ? "SHORT" : "WORD"));
	result.append(binToNumberLiterals(bin, (int) type*2));

	return result;
}

void run(const RunConfig& config) {
	std::FILE* input  = nullptr;
	std::FILE* output = nullptr;

	std::vector<std::uint8_t> rawData;
	std::vector<std::uint32_t> convertedData;

	if (!(input = std::fopen(config.inputFileName.c_str(), "rb")))
		throw std::runtime_error(std::string("Couldn't open file for read: ").append(config.inputFileName));

	std::fseek(input, 0, SEEK_END);
	std::size_t inputSize = std::ftell(input);
	std::fseek(input, 0, SEEK_SET);

	// Ensuring non-empty input file
	if (inputSize == 0) {
		std::fclose(input);
		throw std::runtime_error(std::string("Input file is empty"));
	}

	// Ensuring data size is compatible with EA output type
	if (inputSize & (config.outType-1)) {
		std::fclose(input);
		throw std::runtime_error(std::string("Input file size is not divisible by output type"));
	}

	rawData.resize(inputSize);
	std::fread(rawData.data(), 1, inputSize, input);

	std::fclose(input);

	inputSize = (inputSize / config.outType);
	convertedData.reserve(inputSize);

	for (std::size_t i=0; i<inputSize; ++i) {
		std::uint32_t currentData = 0;

		for (std::size_t j=0; j<config.outType; ++j)
			currentData |= ((rawData.at(i*config.outType + j)) << (j*8));

		convertedData.push_back(currentData);
	}

	std::string outEACode = binToEACode(convertedData, config.outType);

	std::string outEA;
	outEA.reserve(outEACode.size() + 256); // who cares, just make it fast

	if (!config.labelName.empty()) {
		if (config.toDefine) {
			outEA.append("#define ");
			outEA.append(config.labelName);
			outEA.append(" \"");
		} else {
			outEA.append(config.labelName);
			outEA.append(":\n\t");
		}
	}

	if (!config.before.empty())
		outEA.append(config.before).append("; ");

	outEA.append(outEACode);

	if (!config.after.empty())
		outEA.append("; ").append(config.after);

	if (config.toDefine)
		outEA.append("\"\n");
	else if (config.newLine)
		outEA.append("\n");

	if (config.toStdOut)
		output = stdout;
	else if (!(output = std::fopen(config.outputFileName.c_str(), "w")))
		throw std::runtime_error(std::string("Couldn't open file for write: ").append(config.outputFileName));

	std::fwrite(outEA.c_str(), 1, outEA.size(), output);

	if (!config.toStdOut)
		std::fclose(output);
}

int main(int argc, char** argv) {
	Arguments args(argc, argv);

	if (argc == 1) {
		// Show Help
		std::printf("Usage: %s <input> [output/--to-stdout] [-byte/-short/-word] [-define <name>/-label <name>] [-before <before>] [-after <after>] [-no-newline]\n", args.peek().value);
		return 0;
	}

	try {
		RunConfig config;

		auto nextString = [&args] () -> char* {
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
					config.outType = OutByte;
				else if (!std::strcmp(arg.value, "short"))
					config.outType = OutShort;
				else if (!std::strcmp(arg.value, "word"))
					config.outType = OutWord;
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
		std::fprintf(stderr, "[bin2ea error] %s\n", e.what());
		return 1;
	}

	return 0;
}
