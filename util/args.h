#ifndef ARGS_H
#define ARGS_H

#include <vector>

struct Argument {
	enum {
		ProgramName,
		LiteralString,
		Option,
		End
	} type;

	operator bool() const { return (type != End); }

	char* value;
};

class Arguments {
public:
	inline Arguments(int argc, char** argv)
		: mArgs(argv, argv+argc), mCurrentIndex(0) {
		_computeNext();
	}

	inline Argument next() {
		Argument arg = mNext;
		_computeNext();
		return arg;
	}

	inline const Argument& peek() const {
		return mNext;
	}

	inline void resetIteration() {
		mCurrentIndex = 0;
		_computeNext();
	}

protected:
	void _computeNext();

private:
	std::vector<char*> mArgs;
	int mCurrentIndex;

	Argument mNext;
};

#endif // ARGS_H
