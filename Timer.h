#pragma once
#include <chrono>
#include <iostream>

class Timer 
{
private:
	typedef std::chrono::high_resolution_clock Time;
	typedef std::chrono::milliseconds ms;
	typedef std::chrono::duration<float> fsec;

	//en variabel som kommer h�lla en tidpunkt som vi sedan anv�nder f�r att
	//ta tiden mellan tv� t0 och t1.
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