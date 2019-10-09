#include <cmath>
#include <sstream>

#include "stop.h"

Stop::Stop() = default;

Stop::Stop(const std::string& name, double latitude, double longitude, std::pair<size_t, size_t>& id) : __name(name), __latitude(latitude),
    __longitude(longitude), id(std::move(id)) {}

double Stop::getLatitude() const {
    return __latitude;
}

double Stop::getLongitude() const {
    return __longitude;
}

std::string Stop::getName() const {
    return __name;
}

double toRadians(double degrees) {
    return degrees * 3.1415926535 / 180;
}

double Stop::minDistance(const Stop &stop) const {
    auto lhs_lat = toRadians(__latitude);
    auto lhs_lon = toRadians(__longitude);
    auto rhs_lat = toRadians(stop.__latitude);
    auto rhs_lon = toRadians(stop.__longitude);
    return acos(sin(lhs_lat) * sin(rhs_lat) + cos(lhs_lat) * cos(rhs_lat) * cos(std::abs(lhs_lon - rhs_lon))) * 6371000;
}

std::istream &operator>>(std::istream& is, Stop& stop) {
    getline(is, stop.__name, ':');
    is.ignore(1);
    is >> stop.__latitude;
    is.ignore(1);
    is >> stop.__longitude;
    if (is.peek() == ',') {
        is.ignore(2);
        std::string tmp;
        std::getline(is, tmp);
        std::istringstream ss(tmp);
        while (true) {
            size_t distance;
            ss >> distance;
            ss.ignore(5);
            std::string name;
            std::getline(ss, name, ',');
            stop.distances[name] = distance;
            if (!ss) {
                break;
            }
        }
    }
    return is;
}

std::ostream &operator<<(std::ostream& os, const Stop& stop) {
    auto it = Stop::buses.find(stop.__name);
    if (it->second.empty()) {
        os << "Stop " << stop.__name << ": no buses";
    } else {
        os << "Stop " << stop.__name << ": buses";
        for (const std::string& bus : Stop::buses[stop.__name]) {
            os << " " << bus;
        }
    }
    return os;
}

void Stop::AddBus(std::string stop, std::string bus) {
    buses[std::move(stop)].insert(std::move(bus));
}

void Stop::AddStop(std::string stop) {
    buses[std::move(stop)];
}

size_t Stop::realDistance(const Stop& stop) const {
    auto it = distances.find(stop.__name);
    if (it != distances.end()) {
        return it->second;
    }
    return stop.realDistance(*this);
}

void Stop::AddDistance(const std::string& name, size_t distance) {
    distances[name] = distance;
}

std::ostream &Stop::printResponse(std::ostream& os, size_t id) {
    os << "  {\n";
    os << "    \"request_id\": " << id << ",\n";
    auto it = buses.find(__name);
    if (it->second.empty()) {
        os << "    \"buses\": []\n";
    } else {
        os << "    \"buses\": [\n";
        bool flag = false;
        for (const auto& bus : it->second) {
            if (flag) {
                os << ",\n";
            }
            os << "      \"" << bus << "\"";
            flag = true;
        }
        os << "\n    ]\n";
    }
    os << "  }";
    return os;
}

std::pair<size_t, size_t> Stop::getId() const {
    return id;
}
