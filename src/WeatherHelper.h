#ifndef WEATHERHELPER_H
#define WEATHERHELPER_H
#include <time.h>
class WeatherHelper{
private:
    double mLongitude;
    double mLatitude;
    time_t mLastDay;
    time_t mSunset;
    time_t mSunrise;
    int mCurrentTemperature;
    time_t mLastTemperatureCheck;

    void recalculate();

public:
	WeatherHelper(double longitude, double latitude);
	~WeatherHelper();

    time_t getSunRise();
    time_t getSunSet();
};

#endif

