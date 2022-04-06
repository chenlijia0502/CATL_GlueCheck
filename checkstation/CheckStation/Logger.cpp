#include "stdafx.h"
#include "Logger.h"


void Initlog()
{
	// 按文件大小
	//auto file_logger = spdlog::rotating_logger_mt("file_log", "log/log.log", 1024 * 1024 * 100, 3);
	// 每天0:00 am 新建一个日志文件
	auto logger = spdlog::daily_logger_mt("daily_logger", "d://log//checkstation.txt", 0, 0);
	// 遇到warn flush日志，防止丢失
	logger->flush_on(spdlog::level::warn);
	logger->flush_on(spdlog::level::err);

	//每三秒刷新一次
	spdlog::flush_every(std::chrono::seconds(3));

	// Set the default logger to file logger
	auto console = spdlog::stdout_color_mt("console");
	spdlog::set_default_logger(console);
	spdlog::set_level(spdlog::level::debug); // Set global log level to debug

	// change log pattern
	// %s：文件名
	// %#：行号
	// %!：函数名
	spdlog::set_pattern("%Y-%m-%d %H:%M:%S [%l] [%t] - <%s>|<%#>|<%!>,%v");

}
