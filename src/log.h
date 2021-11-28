#pragma once

#include <fstream>
#include <iostream>
#include <iomanip>
#include <memory>
#include <map>
#include <sstream>
#include <string>
#include <sys/time.h>
#include <time.h>
#include <type_traits>
#include <vector>

#include "util.h"

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
	virtual void Output(LogEventPtr event) {

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

	virtual void Output(LogEventPtr event) override;
};

class FileWriter : public LogWriter
{
public:
	FileWriter(const LogFormaterPtr formater, const std::string& name)
		: LogWriter(formater)
		, m_fileName(name) {
		if (name.find_last_of(".log") == std::string::npos) {
			m_fileName += ".log";
		}
		m_ofs.open(m_fileName, std::ios::out | std::ios::app);
	}

	~FileWriter() {
		if (m_ofs.is_open()) {
			m_ofs.close();
		}
	}

	virtual void Output(LogEventPtr event) override;

private:
	std::string m_fileName;
	std::ofstream m_ofs;
};

class Logger
{
public:
	Logger() {
		LogWriterPtr writer = std::make_shared<StdWriter>(m_formater);
		m_writers.insert(std::make_pair("default", writer));
	}

	template<class... Args>
	void Debug(Args&&... argList) {
		Log(LogLevel::DEBUG, std::forward<Args>(argList)...);
	}

	template<class... Args>
	void Info(Args&&... argList) {
		Log(LogLevel::INFO, std::forward<Args>(argList)...);
	}

	template<class... Args>
	void Warning(Args&&... argList) {
		Log(LogLevel::WARNING, std::forward<Args>(argList)...);
	}

	template<class... Args>
	void Error(Args&&... argList) {
		Log(LogLevel::ERROR, std::forward<Args>(argList)...);
	}

	template<class... Args>
	void Critical(Args&&... argList) {
		Log(LogLevel::CRITICAL, std::forward<Args>(argList)...);
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

	void AddFileWriter(const std::string& name) {
		LogWriterPtr writer = std::make_shared<FileWriter>(m_formater, name);
		m_writers.insert(std::make_pair(name, writer));
	}

private:
	template<class First>
	typename std::enable_if<util::is_bool<First>::value>::type
	MakeOneMsg(std::ostringstream& os, First&& first) {
		os << (first ? "true" : "false");
	}

	template<class First>
	typename std::enable_if<!util::is_bool<First>::value>::type
	MakeOneMsg(std::ostringstream& os, First&& first) {
		os << first;
	}

	template<class First>
	void MakeLogMsg(std::ostringstream& os, First&& first) {
		MakeOneMsg(os, std::forward<First>(first));
	}

	template<class First, class... Args>
	void MakeLogMsg(std::ostringstream& os, First&& first, Args&&... argList) {
		MakeOneMsg(os, std::forward<First>(first));
		os << " ";
		MakeLogMsg(os, std::forward<Args>(argList)...);
	}

	template<class... Args>
	void Log(LogLevel level, Args&&... argList) {
		std::ostringstream os;
		MakeLogMsg(os, std::forward<Args>(argList)...);
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

class LoggerManager
{
public:
	LoggerManager() {
		auto logger = std::make_shared<Logger>();
		m_loggers.insert(std::make_pair("default", logger));
	}

	LoggerPtr GetLogger(const std::string& name = "default") {
		auto iter = m_loggers.find(name);
		if (iter != m_loggers.end()) {
			return iter->second;
		}
		auto logger = std::make_shared<Logger>();
		logger->AddFileWriter(name);
		m_loggers.insert(std::make_pair(name, logger));
		return logger;
	}

private:
	std::map<std::string, LoggerPtr> m_loggers;
};

}//namespace::log

const log::LoggerPtr GetLogger(const std::string& name = "default");
}
