#pragma once
#include <chrono>
#include <iostream>

class Timer 
{
private:
	typedef std::chrono::high_resolution_clock Time;
	typedef std::chrono::milliseconds ms;
	typedef std::chrono::duration<float> fsec;

	//en variabel som kommer hålla en tidpunkt som vi sedan använder för att
	//ta tiden mellan två t0 och t1.
	typedef std::chrono::time_point<std::chrono::steady_clock> time_point;
	time_point start;


public:
	Timer()
	{
		reset();
	}
	~Timer()
	{
	}

	void reset()
	{
		start = Time::now();
	}

	float seconds()
	{
		return fsec(Time::now() - start).count();
	}

};