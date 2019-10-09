#pragma once

#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "edgeInfo.h"
#include "graph.h"
#include "stop.h"

class Bus {
private:
    std::vector<std::string> stopsSequence;
    std::unordered_map<std::string, std::shared_ptr<Stop>> uniqueStops;
    std::string __name;
    bool _is_cyclic;
    double min_route_length;
    size_t real_route_length;
    void AddStop(const std::string&);
    bool is_initialized;

public:
    Bus();
    explicit Bus(std::string& name, std::vector<std::string>& stopsSequence, bool is_cyclic);
    size_t allStopsCount() const;
    size_t uniqueStopsCount() const;
    size_t routeLength() const;
    std::string getName() const;
    double curvature() const;
    friend std::ostream& operator<<(std::ostream&, const Bus&);
    void Initialize(const std::unordered_map<std::string, std::shared_ptr<Stop>>& stops);
    friend std::istream& operator>>(std::istream&, Bus&);
    std::ostream& printResponse(std::ostream& os, size_t id);
    const std::vector<std::string>& getStopsSequence() const;
    bool IsCyclic() const;
    void FillEdges(Graph::DirectedWeightedGraph<double>& graph, const std::unordered_map<std::string, std::shared_ptr<Stop>>& stops,
                   std::unordered_map<size_t, std::shared_ptr<EdgeInfo>>& routeInfo, size_t wait_time,
                   double bus_velocity);
};
