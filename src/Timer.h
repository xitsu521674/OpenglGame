#include "Character.h"
class Timer {
public:
	bool initiation = false;
	bool finitiation = false;
	bool started = false;
	bool finished = false;
	bool working;
	Timer(float period) {
		this->period = period;
		working = false;
	}
	void start(float time) {
		started = true;
		startTime = time;
		working = true;
		passTime = 0;
	}
	void update() {
		if (working) {
			passTime++;
			if (passTime >= period) {
				working = false;
				finished = true;
			}
		}
	}
	float getPassTime() {
		return passTime;
	}
private:
	float period;
	float startTime;
	float passTime;
};