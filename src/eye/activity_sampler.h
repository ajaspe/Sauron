#pragma once

#include "timestamp.h"
#include <string>
#include <vector>

class ActivitySampler {

public:

	struct Sample {
		TimeStamp ts;
		std::string window;
		std::string process;
		float idleSecs;
	};

	void takeSample();
	std::string getLastSampleString();

private:
	std::vector<Sample> _samples;

	

};