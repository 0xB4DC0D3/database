#include "Logger.h"

Logger::Logger() {
	file.imbue(std::locale(""));
}

void Logger::write(std::wstring msg) {
	auto in_time_t{ std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) };
	std::tm buf;
	localtime_s(&buf, &in_time_t);
	file.open((std::wstringstream{} << L"logs/" << std::put_time(&buf, L"%Y-%m-%d") << L".txt").str(), std::ios::out | std::ios::app);
	file << std::put_time(&buf, L"[%Y-%m-%d %H:%M:%S] ") << msg << std::endl;
	file.close();
}

Logger::~Logger() {
	file.close();
}
