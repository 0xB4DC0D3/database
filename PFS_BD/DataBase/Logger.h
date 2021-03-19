#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>

class Logger {
public:
	Logger();
	void write(std::wstring msg);
	~Logger();
private:
	std::wfstream file;
};