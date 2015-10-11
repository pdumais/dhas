#include "DHASLogging.h"
#include "WeatherHelper.h"
#include <cmath>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>
#include <stdio.h>
#include <netinet/tcp.h>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include "json/JSON.h"

#define JAN12000 946684800.0
#define JULIANJAN12000 2451545.0
#define DEG (M_PI/180.0)
#define GMT_CORRECTION (5*60*60)

WeatherHelper::WeatherHelper(double longitude, double latitude)
{
    mLastDay=0;
    mLongitude = longitude;
    mLatitude = latitude;
    mLastTemperatureCheck=0;

}

WeatherHelper::~WeatherHelper()
{
}

void WeatherHelper::recalculate()
{
    time_t t;
    double julianDate;
    time(&t);

    // TODO: the mLastDay skips over a day if calculated after midnight UTC
    mLastDay = (floor((double)t/(double)((24*60*60)))+1)*(24*60*60)+GMT_CORRECTION; // tomorow midnight
    LOG("Recalculating sunset/sunrise. Next recalculation will occur at "<<mLastDay);
    t = (t-JAN12000)/(60*60*24);
    julianDate = t  + JULIANJAN12000;

    //TODO: should use all radians instead of converting
    double julianCycle = round(julianDate - JULIANJAN12000 -0.0009 +(mLongitude/360.0));
    double solarNoon = JULIANJAN12000 + julianCycle + 0.0009-(mLongitude/360.0);
    double M = fmod((357.5291 + 0.98560028 * (solarNoon-JULIANJAN12000)),360.0);
    double C = (1.9148 * sin(DEG*M)) + (0.0200 * sin(DEG*2 * M)) + (0.0003 * sin(DEG*3 * M)) ;
    double lambda = fmod((M + 102.9372 + C + 180.0),360);
    double jt = solarNoon + (0.0053 * sin(DEG*M)) - (0.0069 * sin(DEG*2.0 * lambda));
    double teta = asin(sin(DEG*lambda)*sin(DEG*23.45))/DEG;

    double H = acos((sin(DEG*(-0.83)) - sin(DEG*mLatitude) * sin(DEG*teta)) / (cos(DEG*mLatitude) * cos(DEG*teta)))/DEG;
    double aprox = JULIANJAN12000 + 0.0009 + ((H - mLongitude)/360.0) + julianCycle;
    double sunset = aprox + (0.0053 * sin(DEG*M)) - (0.0069 * sin(DEG*2.0 * lambda));
    double sunrise = jt-(sunset-jt);

    double zoneCorrection = (12)*3600;
    sunset = ((sunset - JULIANJAN12000)*86400.0)+JAN12000 + zoneCorrection;
    sunrise = ((sunrise - JULIANJAN12000)*86400.0)+JAN12000 + zoneCorrection;

    mSunset = sunset;
    mSunrise = sunrise;
    

    LOG("Sunrise: "<<mSunrise<<", Sunset: "<< mSunset);
}

time_t WeatherHelper::getSunRise()
{
    time_t t;
    time(&t);
    if (t>mLastDay) recalculate();
    
    return mSunrise;
}

time_t WeatherHelper::getSunSet()
{
    time_t t;
    time(&t);
    if (t>mLastDay) recalculate();
    
    return mSunset;

}

