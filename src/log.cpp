#include "util.h"
#include "log.h"

namespace qf
{
namespace log
{

static std::map<LogLevel, std::string> LevelStrMap = {
	{ LogLevel::DEBUG, "DEBUG" },
	{ LogLevel::INFO, "INFO" },
	{ LogLevel::WARNING, "WARNING" },
	{ LogLevel::ERROR, "ERROR" },
	{ LogLevel::CRITICAL, "CRITICAL" },
};

void LevelFormatItem::Format(std::ostringstream& os, const LogEventPtr event) const {
	os << LevelStrMap[event->level];
}

FormatItemPtr LogFormater::GenFormatItem(char c) {
	switch (c) {
	case 'd':
		return std::make_shared<DateFormatItem>();
	case 'p':
		return std::make_shared<LevelFormatItem>();
	case 'm':
		return std::make_shared<MsgFormatItem>();
	default:
		break;
	}
	std::string s = "%";
	s.push_back(c);
	return std::make_shared<StrFormatItem>(s);
}

std::string LogFormater::s_fmt = "[%d ][%p] %m";

void LogFormater::Init() {
	int len = (int)s_fmt.size();
	int state = 0;
	std::string sStrItem;

	for (int i = 0; i < len; i++) {
		char c = s_fmt[i];
		switch (state) {
		case 0:
		{
			if ((c == '%') && (i + 1 < len)) {
				if (!sStrItem.empty()) {
					m_items.push_back(std::make_shared<StrFormatItem>(sStrItem));
					sStrItem.clear();
				}
				m_items.push_back(GenFormatItem(s_fmt[i+1]));
				i++;
			}
			else {
				sStrItem.push_back(c);
			}
			break;
		}
		default:
			break;
		}
	}

	if (!sStrItem.empty()) {
		m_items.push_back(std::make_shared<StrFormatItem>(sStrItem));
	}
}

void StdWriter::Output(LogEventPtr event) const {
	auto logString = m_formater->GenLogString(event);
	std::cout << logString << std::endl;
}

}

const log::Logger& GetLogger() {
	return *GetInstance<log::Logger>();
}

}
