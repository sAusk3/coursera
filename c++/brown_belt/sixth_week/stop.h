#pragma once

#include <iostream>
#include <unordered_map>
#include <set>

class Stop {
private:
    double __latitude;
    double __longitude;
    std::string __name;
    std::pair<size_t, size_t> id;
    static std::unordered_map<std::string, std::set<std::string>> buses;
    std::unordered_map<std::string, size_t> distances;

public:
    Stop();
    Stop(const std::string& name, double latitude, double longitude, std::pair<size_t, size_t>& id);
    double getLatitude() const;
    double getLongitude() const;
    std::string getName() const;
    double minDistance(const Stop& stop) const;
    size_t realDistance(const Stop& stop) const;
    std::pair<size_t, size_t> getId() const;
    void AddDistance(const std::string&, size_t);
    static void AddBus(std::string, std::string);
    static void AddStop(std::string);
    friend std::istream& operator>>(std::istream&, Stop&);
    friend std::ostream& operator<<(std::ostream&, const Stop&);
    std::ostream& printResponse(std::ostream&, size_t);
};
