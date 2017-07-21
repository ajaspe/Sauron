#pragma once

#include <iostream>

struct TimeStamp {
	unsigned int data{ 0 }, time{ 0 };

	void setData(int year, int month, int day) { data = (year << 16) | (month << 8) | day; }
	void setTime(int hour, int min, int sec, int milisec) { time = hour * (60 * 60 * 1000) + min * (60 * 1000) + sec * 1000 + milisec; }

	unsigned int year() const { return (data >> 16) & 0xFFFF; }
	unsigned int month() const { return (data >> 8) & 0xFF; }
	unsigned int day() const { return data & 0xFF; }
	unsigned int hour() const { return time / (60 * 60 * 1000); }
	unsigned int minute() const { return (time % (60 * 60 * 1000)) / (60 * 1000); }
	unsigned int second() const { return (time % (60 * 1000)) / 1000; }
	unsigned int milisecond() const { return time % 1000; }

	bool operator <  (const TimeStamp & other) const { return (data < other.data) && (time < other.time); }
	bool operator >  (const TimeStamp & other) const { return !(*this < other); }
	bool operator == (const TimeStamp & other) const { return (data == other.data) && (time == other.time); }

	std::string getAsString() const {
		char buf[32];
		sprintf(buf, "%02i/%02i/%02i %02i:%02i:%02i.%i", day(), month(), year()%100, hour(), minute(), second(), milisecond()/100);
		return std::string(buf);
	}
};