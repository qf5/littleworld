#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>
#include <memory>
#include <map>
#include <string>
#include <time.h>
#include <sys/time.h>
#include <vector>

namespace qf
{
namespace log
{

enum class LogLevel
{
	DEBUG = 0,
	INFO = 1,
	WARNING = 2,
	ERROR = 3,
	CRITICAL = 4,
};

struct LogEvent
{
	LogEvent(LogLevel level, const std::string& msg)
		: level(level)
		, msg(msg) {

	}
	LogLevel level;
	std::string msg;
};
typedef std::shared_ptr<LogEvent> LogEventPtr;

class FormatItem
{
public:
	virtual void Format(std::ostringstream& os, const LogEventPtr event) const {

	}
};

typedef std::shared_ptr<FormatItem> FormatItemPtr;

class DateFormatItem : public FormatItem
{
public:
	virtual void Format(std::ostringstream& os, const LogEventPtr event) const override {
		struct timeval tv;
		gettimeofday(&tv, nullptr);
		char buf[256];
		strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&tv.tv_sec));
		int milli = tv.tv_usec / 1000;
		os << buf << "." << std::setfill('0') << std::setw(3) << milli;
	}
};

class LevelFormatItem : public FormatItem
{
public:
	virtual void Format(std::ostringstream& os, const LogEventPtr event) const override;
};

class MsgFormatItem : public FormatItem
{
public:
	virtual void Format(std::ostringstream& os, const LogEventPtr event) const override {
		os << event->msg;
	}
};

class StrFormatItem : public FormatItem
{
public:
	StrFormatItem(const std::string& str)
		: m_str(str) {

	}
	virtual void Format(std::ostringstream& os, const LogEventPtr event) const {
		os << m_str;
	}

private:
	std::string m_str;
};

class LogFormater
{
public:
	LogFormater() {
		Init();
	}

	std::string GenLogString(const LogEventPtr event) {
		std::ostringstream os;
		for (auto& formatItem : m_items) {
			formatItem->Format(os, event);
		}
		return os.str();
	}

private:
	void Init();

	FormatItemPtr GenFormatItem(char c);

	/*
	 * %m   输出代码中指定的消息
　　 * %p   输出优先级，即DEBUG，INFO，WARN，ERROR，FATAL 
　　 * %r   输出自应用启动到输出该log信息耗费的毫秒数 
　　 * %c   输出所属的类目，通常就是所在类的全名 
　　 * %t   输出产生该日志事件的线程名 
	 * %d   输出日志时间点的日期或时间
	 * %l   输出日志事件的发生位置
	*/
	static std::string s_fmt;
	std::vector<FormatItemPtr> m_items;
};

typedef std::shared_ptr<LogFormater> LogFormaterPtr;

class LogWriter
{
public:
	LogWriter(const LogFormaterPtr formater)
		: m_formater(formater){

	}

	void Log(LogEventPtr event) {
		if (event->level >= m_level) {
			Output(event);
		}
	}

	void SetLogLevel(LogLevel level) {
		m_level = level;
	}

protected:
	virtual void Output(LogEventPtr event) const {

	}

protected:
	LogLevel m_level = LogLevel::INFO;
	LogFormaterPtr m_formater;
};
typedef std::shared_ptr<LogWriter> LogWriterPtr;

class StdWriter : public LogWriter
{
public:
	StdWriter(const LogFormaterPtr formater)
		: LogWriter(formater) {

	}

	virtual void Output(LogEventPtr event) const override;
};

class Logger
{
public:
	Logger() {
		LogWriterPtr writer = std::make_shared<StdWriter>(m_formater);
		m_writers.insert(std::make_pair("default", writer));
	}

	template<class... Args>
	void Debug(Args&... argList) {
		Log(LogLevel::DEBUG, argList...);
	}

	template<class... Args>
	void Info(Args&... argList) {
		Log(LogLevel::INFO, argList...);
	}

	template<class... Args>
	void Warning(Args&... argList) {
		Log(LogLevel::WARNING, argList...);
	}

	template<class... Args>
	void Error(Args&... argList) {
		Log(LogLevel::ERROR, argList...);
	}

	template<class... Args>
	void Critical(Args&... argList) {
		Log(LogLevel::CRITICAL, argList...);
	}

	void SetLogLevel(LogLevel level, const std::string& name = "") {
		if (name == "") {
			for (auto& iter : m_writers) {
				iter.second->SetLogLevel(level);
			}
		} else {
			auto iter = m_writers.find(name);
			if (iter != m_writers.end()) {
				iter->second->SetLogLevel(level);
			}
		}
	}

private:
	template<class First>
	void MakeLogMsg(std::ostringstream& os, First& first) {
		os << first;
	}

	template<class First, class... Args>
	void MakeLogMsg(std::ostringstream& os, First& first, Args&... argList) {
		os << first << " ";
		MakeLogMsg(os, argList...);
	}

	template<class... Args>
	void Log(LogLevel level, Args&... argList) {
		std::ostringstream os;
		MakeLogMsg(os, argList...);
		Log(std::make_shared<LogEvent>(level, os.str()));
	}

	void Log(LogEventPtr event) {
		for (auto& iter : m_writers) {
			iter.second->Log(event);
		}
	}

private:
	LogFormaterPtr m_formater = std::make_shared<LogFormater>();
	std::map<std::string, LogWriterPtr> m_writers;
};

typedef std::shared_ptr<Logger> LoggerPtr;

}//namespace::log

const log::Logger& GetLogger();
}
