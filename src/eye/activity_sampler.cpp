#include "activity_sampler.h"
#include <windows.h>
#include <Psapi.h>


void ActivitySampler::takeSample() {
	Sample sample;

	// Get actve window
	HWND hActiveWindow;
	hActiveWindow = GetForegroundWindow();

	// Get active window title
	char buf1[1024], buf2[1024];
	GetWindowText(hActiveWindow, buf1, 1024);
	sample.window = buf1;

	// Get active window process
	sample.process.reserve(256);
	DWORD pid;
	GetWindowThreadProcessId(hActiveWindow, &pid);
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
	GetModuleFileNameEx(hProcess, NULL, buf1, 1024);
	_splitpath(buf1, NULL, NULL, buf2, NULL);
	sample.process = buf2;

	// Get last input for IDLE computing
	LASTINPUTINFO lii;
	lii.cbSize = sizeof(LASTINPUTINFO);
	GetLastInputInfo(&lii);
	DWORD te = GetTickCount();
	sample.idleSecs = float(te - lii.dwTime) / 1000.0f;

	// Get Local Time for timestamp
	SYSTEMTIME st;
	GetLocalTime(&st);
	sample.ts.setData(st.wYear, st.wMonth, st.wDay);
	sample.ts.setTime(st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	_samples.push_back(sample);
}

std::string ActivitySampler::getLastSampleString()
{
	char buf[1024];
	const Sample * s = &_samples.back();
	sprintf(buf, "%s\t%s\t%s\t%.1f\n", s->ts.getAsString().c_str(), s->process.c_str(), s->window.c_str(), s->idleSecs);
	return std::string(buf);
}
