#pragma once

#include <iostream>
#include <memory>
#include <optional>

#include "route.h"

class EdgeInfo {
public:
    EdgeInfo(std::string& type, double time);
    virtual std::ostream &toJson(std::ostream &os, std::optional<Graph::Router<double>::RouteInfo> route,
                                 std::unordered_map<size_t, std::shared_ptr<EdgeInfo>>& routeInfo,
                                 Graph::Router<double>& router) = 0;
    virtual ~EdgeInfo() = default;
protected:
    std::string __type;
    double __time;
};

class TripInfo : public EdgeInfo {
public:
    TripInfo(std::string& type, double time, size_t span_count, std::string& bus);
    std::ostream &toJson(std::ostream &os, std::optional<Graph::Router<double>::RouteInfo> route,
                         std::unordered_map<size_t, std::shared_ptr<EdgeInfo>>& routeInfo,
                         Graph::Router<double>& router) override;

private:
    size_t span_count;
    std::string bus;
};

class WaitingInfo : public EdgeInfo {
public:
    WaitingInfo(std::string& type, double time, std::string& stop);
    std::ostream &toJson(std::ostream &os, std::optional<Graph::Router<double>::RouteInfo> route,
                         std::unordered_map<size_t, std::shared_ptr<EdgeInfo>>& routeInfo,
                         Graph::Router<double>& router) override;

private:
    std::string stop;
};
