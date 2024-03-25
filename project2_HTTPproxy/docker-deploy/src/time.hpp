#ifndef TIME_H
#define TIME_H
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>

class TimeMap {
 public:
  std::map<std::string, int> timeMap;

 public:
  TimeMap()
      : timeMap{{"Jan", 1}, {"Feb", 2}, {"Mar", 3}, {"Apr", 4}, {"May", 5},
                 {"Jun", 6}, {"Jul", 7}, {"Aug", 8}, {"Sep", 9}, {"Oct", 10},
                 {"Nov", 11}, {"Dec", 12}, {"Sun", 0}, {"Mon", 1}, {"Tue", 2},
                 {"Wed", 3}, {"Thu", 4}, {"Fri", 5}, {"Sat", 6}} {}

  int getMappedValue(std::string abbreviation) { return timeMap[abbreviation]; }
};

class TimeParser {
 private:
  struct tm timeStruct;
  TimeMap timeMap;

 public:
  TimeParser() {}

  void parseAndInitTime(const std::string & expression) {
    //indices            01234567890123456789012345678
    //format=>  Expires: Thu, 01 Dec 2022 16:00:00 GMT

    try {
        timeStruct.tm_mday = std::stoi(expression.substr(5));//Represents day of the month (1 to 31).
        timeStruct.tm_mon = timeMap.getMappedValue(expression.substr(8, 3)) - 1;// Represents months since January (0 to 11).
        timeStruct.tm_year = std::stoi(expression.substr(12)) - 1900;//Represents years since 1900.
        timeStruct.tm_hour = std::stoi(expression.substr(17));//Represents hours since midnight (0 to 23).
        timeStruct.tm_min = std::stoi(expression.substr(20));//Represents minutes after the hour
        timeStruct.tm_sec = std::stoi(expression.substr(23));//Represents seconds after the minute
        timeStruct.tm_wday = timeMap.getMappedValue(expression.substr(0, 3));//Represents days since Sunday (0 to 6)
        timeStruct.tm_isdst = 0;//Represents Daylight Saving Time flag (a positive or negative value, or zero).
        //tm_yday Represents days since January 1 (0 to 365).
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid argument: " << e.what() << std::endl;
            return;
        } catch (const std::out_of_range& e) {
            std::cerr << "Out of range: " << e.what() << std::endl;
            return;
        } catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
            return;
        } 
  }

  void display () const {
    char buffer[80];
    ::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S", &timeStruct);


    std::cout << "Formatted Time: " << buffer << std::endl;
    std::cout << "tm_sec: " << timeStruct.tm_sec << std::endl;
    std::cout << "tm_min: " << timeStruct.tm_min << std::endl;
    std::cout << "tm_hour: " << timeStruct.tm_hour << std::endl;
    std::cout << "tm_mday: " << timeStruct.tm_mday << std::endl;
    std::cout << "tm_mon: " << timeStruct.tm_mon << std::endl;
    std::cout << "tm_year: " << timeStruct.tm_year << std::endl;
    std::cout << "tm_wday: " << timeStruct.tm_wday << std::endl;
    std::cout << "tm_yday: " << timeStruct.tm_yday << std::endl;
    std::cout << "tm_isdst: " << timeStruct.tm_isdst << std::endl;
  
  }

  struct tm *getTimeStruct() { return &timeStruct; }
};
#endif