#include <iomanip>

#include "edgeInfo.h"
#include "route.h"

EdgeInfo::EdgeInfo(std::string &type, double time) : __type(type), __time(time) {}

TripInfo::TripInfo(std::string &type, double time, size_t span_count, std::string &bus) : EdgeInfo(type, time),
    span_count(span_count), bus(bus) {}

std::ostream& TripInfo::toJson(std::ostream &os, std::optional<Graph::Router<double>::RouteInfo> route,
                               std::unordered_map<size_t, std::shared_ptr<EdgeInfo>>& routeInfo,
                               Graph::Router<double>& router) {
    os << "      {\n";
    os << "        \"time\": " << __time << ",\n";
    os << "        \"type\": \"" << std::setprecision(6) << __type << "\",\n";
    os << "        \"span_count\": " << span_count << ",\n";
    os << "        \"bus\": \"" << bus << "\"\n";
    os << "      }";
    return os;
}

WaitingInfo::WaitingInfo(std::string &type, double time, std::string &stop) : EdgeInfo(type, time),
    stop(std::move(stop)){}

std::ostream& WaitingInfo::toJson(std::ostream &os, std::optional<Graph::Router<double>::RouteInfo> route,
                                  std::unordered_map<size_t, std::shared_ptr<EdgeInfo>>& routeInfo,
                                  Graph::Router<double>& router) {
    os << "      {\n";
    os << "        \"time\": " << std::setprecision(6) << __time << ",\n";
    os << "        \"type\": \"" << __type << "\",\n";
    os << "        \"stop_name\": \"" << stop << "\"\n";
    os << "      }";
    return os;
}