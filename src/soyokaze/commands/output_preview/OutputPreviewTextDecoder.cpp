#include "pch.h"
#include "OutputPreviewTextDecoder.h"
#include "utility/CharConverter.h"

namespace launcherapp {
namespace commands {
namespace output_preview {

void OutputPreviewTextDecoder::Decode(const char* source, CString& text)
{
	if (source == nullptr) {
		text.Empty();
		return;
	}

	try {
		launcherapp::utility::CharConverter utf8Converter(CP_UTF8);
		utf8Converter.Convert(source, text, true);
	}
	catch (const launcherapp::utility::CharConverter::Exception&) {
		launcherapp::utility::CharConverter sjisConverter(932);
		sjisConverter.Convert(source, text);
	}
}

}
}
}
