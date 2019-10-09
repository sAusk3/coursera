#include <iomanip>

#include "bus.h"
#include "edgeInfo.h"

size_t Bus::uniqueStopsCount() const {
    return uniqueStops.size();
}

size_t Bus::allStopsCount() const {
    return _is_cyclic ? stopsSequence.size() * 2 - 1 : stopsSequence.size();
}

Bus::Bus() = default;

Bus::Bus(std::string& name, std::vector<std::string>& stops, bool is_cyclic) : __name(std::move(name)),
                                                                             stopsSequence(std::move(stops)), _is_cyclic(is_cyclic) {
    is_initialized = false;
}

std::string Bus::getName() const {
    return __name;
}

void Bus::AddStop(const std::string& stop) {
    stopsSequence.push_back(stop);
}

std::ostream& operator<<(std::ostream& os, const Bus &bus) {
    os << "Bus " << bus.getName() << ": " << bus.allStopsCount() << " stops on route, " <<
        bus.uniqueStopsCount() << " unique stops, " << bus.routeLength() << " route length, " <<
        std::setprecision(6) << bus.curvature() << " curvature";
    return os;
}

void Bus::Initialize(const std::unordered_map<std::string, std::shared_ptr<Stop>>& stops) {
    if (is_initialized) {
        return;
    }
    is_initialized = true;
    for (const std::string& stop : stopsSequence) {
        uniqueStops[stop] = stops.at(stop);
    }
    min_route_length = 0;
    real_route_length = 0;
    for (auto it = stopsSequence.begin(); it + 1 != stopsSequence.end(); ++it) {

        auto first = uniqueStops.find(*it)->second;
        auto second = uniqueStops.find(*(it + 1))->second;
        min_route_length += first->minDistance(*second);
        real_route_length += first->realDistance(*second);
        if (_is_cyclic) {
            real_route_length += second->realDistance(*first);
        }
    }
    if (_is_cyclic) {
        min_route_length *= 2;
    }
}

size_t Bus::routeLength() const {
    return real_route_length;
}

std::istream &operator>>(std::istream& is, Bus& bus) {
    is.ignore(1);
    std::string name;
    std::getline(is, name, ':');
    std::vector<std::string> stops;
    std::string query;
    std::getline(is, query);
    std::string stop;
    char del{'+'};

    for (auto it = query.begin() + 1; it != query.end(); ++it) {
        char c = *it;
        if (c == ' ') {
            if (*(it + 1) == '>' || *(it + 1) == '-') {
                continue;
            }
            stop += c;
        } else if (c == '>' || c == '-') {
            del = c;
            Stop::AddBus(stop, name);
            stops.push_back(std::move(stop));
            stop = std::string{};
            ++it;
        } else {
            stop += c;
        }
    }
    Stop::AddBus(stop, name);
    stops.push_back(std::move(stop));
    bool is_cyclic = false;
    if (del == '-') {
        is_cyclic = true;
    }
    bus = Bus{name, stops, is_cyclic};
    return is;
}

double Bus::curvature() const {
    return real_route_length / min_route_length;
}

std::ostream& Bus::printResponse(std::ostream& os, size_t id) {
    os << "  {\n";
    os << "    \"route_length\": " << real_route_length << ",\n";
    os << "    \"request_id\": " << id << ",\n";
    os << "    \"curvature\": " << curvature() << ",\n";
    os << "    \"stop_count\": " << allStopsCount() << ",\n";
    os << "    \"unique_stop_count\": " << uniqueStopsCount() << "\n";
    os << "  }";
    return os;
}

const std::vector<std::string> &Bus::getStopsSequence() const {
    return stopsSequence;
}

bool Bus::IsCyclic() const {
    return _is_cyclic;
}

void Bus::FillEdges(Graph::DirectedWeightedGraph<double>& graph, const std::unordered_map<std::string, std::shared_ptr<Stop>>& stops,
                    std::unordered_map<size_t, std::shared_ptr<EdgeInfo>>& routeInfo, size_t wait_time,
                    double bus_velocity) {
    std::string WAIT = "Wait";
    std::string BUS = "Bus";
    for (size_t i = 0; i < stopsSequence.size(); ++i) {
        const auto& stop = stops.at(stopsSequence[i]);
        auto stop_id = stop->getId();
        double waitTime = wait_time;
        size_t waitEdgeId = graph.AddEdge(Graph::Edge<double>{stop_id.first, stop_id.second, waitTime});
        std::string name = stop->getName();
        routeInfo[waitEdgeId] = std::make_shared<WaitingInfo>(WAIT, wait_time, name);
        size_t straight_distance = 0;
        size_t return_distance = 0;
        for (size_t j = i; j + 1 < stopsSequence.size(); ++j) {
            const auto& firstStop = stops.at(stopsSequence[j]);
            const auto& secondStop = stops.at(stopsSequence[j + 1]);
            auto second_stop_id = secondStop->getId();
            straight_distance += firstStop->realDistance(*secondStop);
            double time = straight_distance  * 60.0 / (bus_velocity * 1000);
            size_t tripEdgeId = graph.AddEdge(Graph::Edge<double>{stop_id.second, second_stop_id.first, time});
            name = getName();
            routeInfo[tripEdgeId] = std::make_shared<TripInfo>(BUS, time, j + 1 - i, name);
            if (_is_cyclic) {
                return_distance += secondStop->realDistance(*firstStop);
                time = return_distance  * 60.0 / (bus_velocity * 1000);
                tripEdgeId = graph.AddEdge(Graph::Edge<double>{second_stop_id.second, stop_id.first, time});
                routeInfo[tripEdgeId] = std::make_shared<TripInfo>(BUS, time, j + 1 - i, name);
            }
        }
    }
}
