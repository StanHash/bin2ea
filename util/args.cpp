#include "args.h"

void Arguments::_computeNext() {
   if (mArgs.size() == mCurrentIndex) {
	   mNext.type  = Argument::End;
	   mNext.value = nullptr;
   } else if (mCurrentIndex == 0) {
	   mNext.type  = Argument::ProgramName;
	   mNext.value = mArgs.at(mCurrentIndex++);
   } else {
	   char* data = mArgs.at(mCurrentIndex++);

	   if (!data) {
		   mNext.type  = Argument::End;
		   mNext.value = nullptr;
	   } else if (data[0] == '-') {
		   mNext.type  = Argument::Option;
		   mNext.value = data+1;
	   } else {
		   mNext.type  = Argument::LiteralString;
		   mNext.value = data;
	   }
   }
}
