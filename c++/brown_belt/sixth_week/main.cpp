#include <iostream>
#include <sstream>

#include "bus.h"
#include "edgeInfo.h"
#include "graph.h"
#include "json.h"
#include "route.h"
#include "stop.h"

using namespace std;

std::unordered_map<std::string, std::set<std::string>> Stop::buses{};

int main() {
    const string BUS = "Bus";
    const string STOP = "Stop";
    const string ROUTE = "Route";

    unordered_map<string, shared_ptr<Stop>> stops;
    unordered_map<string, Bus> buses;
    unordered_map<size_t, shared_ptr<EdgeInfo>> routeInfo;

    const auto input = Json::Load(cin);
    const map<string, Json::Node>& data = input.GetRoot().AsMap();
    const vector<Json::Node>& base_requests = data.at("base_requests").AsArray();
    const vector<Json::Node>& stat_requests = data.at("stat_requests").AsArray();
    const map<string, Json::Node>& routing_settings = data.at("routing_settings").AsMap();

    size_t id_counter = 0;
    size_t wait_time = routing_settings.at("bus_wait_time").AsInt();
    double bus_speed = 0;
    try {
        bus_speed = routing_settings.at("bus_velocity").AsInt();
    } catch(...) {
        bus_speed = routing_settings.at("bus_velocity").AsDouble();
    }
    for (const Json::Node& req : base_requests) {
        const map<string, Json::Node>& request = req.AsMap();
        const string& type = request.at("type").AsString();
        string name = request.at("name").AsString();
        if (type == BUS) {
            const vector<Json::Node>& bus_stops = request.at("stops").AsArray();
            vector<string> transformedStops(bus_stops.size());
            for (size_t i = 0; i < bus_stops.size(); ++i) {
                transformedStops[i] = bus_stops[i].AsString();
                Stop::AddBus(transformedStops[i], name);
            }
            bool is_cyclic = request.at("is_roundtrip").AsBool();
            Bus bus{name, transformedStops, !is_cyclic};
            buses[bus.getName()] = move(bus);
        } else if (type == STOP) {
            double latitude;
            try {
                latitude = request.at("latitude").AsDouble();
            } catch (...) {
                latitude = request.at("latitude").AsInt();
            }
            double longitude;
            try {
                longitude = request.at("longitude").AsDouble();
            } catch (...) {
                longitude = request.at("longitude").AsInt();
            }
            auto p = pair<size_t, size_t>(id_counter, id_counter + 1);
            id_counter += 2;
            shared_ptr<Stop> stop = make_shared<Stop>(name, latitude, longitude, p);
            Stop::AddStop(stop->getName());
            auto it = request.find("road_distances");
            if (it != request.end()) {
                const map<string, Json::Node> &road_distances = it->second.AsMap();
                for (const auto &dist : road_distances) {
                    stop->AddDistance(dist.first, dist.second.AsInt());
                }
            }
            stops[name] = move(stop);
        }
    }

    Graph::DirectedWeightedGraph<double> graph{stops.size() * 2};
    for (auto& [_, bus] : buses) {
        bus.Initialize(stops);
    }
    for (auto& [_, bus] : buses) {
        bus.FillEdges(graph, stops, routeInfo, wait_time, bus_speed);
    }
    Graph::Router<double> router{graph};

    cout << "[\n";
    size_t k = 0;
    for (const Json::Node& req : stat_requests) {
        ++k;
        const map<string, Json::Node>& request = req.AsMap();
        const string& type = request.at("type").AsString();
        size_t id = request.at("id").AsInt();
        if (type == BUS) {
            const string& name = request.at("name").AsString();
            auto it = buses.find(name);
            if (it != buses.end()) {
                it->second.printResponse(cout, id);
            } else {
                cout << "  {\n    \"request_id\": " << id << ",\n    \"error_message\": \"not found\"\n  }";
            }
        } else if (type == STOP) {
            const string& name = request.at("name").AsString();
            auto it = stops.find(name);
            if (it != stops.end()) {
                it->second->printResponse(cout, id);
            } else {
                cout << "  {\n    \"request_id\": " << id << ",\n    \"error_message\": \"not found\"\n  }";
            }
        } else if (type == ROUTE) {
            const string& stop_from = request.at("from").AsString();
            const string& stop_to = request.at("to").AsString();
            size_t fromId = stops[stop_from]->getId().first;
            size_t toId = stops[stop_to]->getId().first;
            auto r = router.BuildRoute(fromId, toId);
            if (r) {
                double total_time = r->weight;
                cout << "  {\n    ";
                cout << "\"total_time\": " << total_time << ",\n    \"items\": [\n";
                for (size_t i = 0; i < r->edge_count; ++i) {
                    auto p = routeInfo[router.GetRouteEdge(r->id, i)];
                    p->toJson(cout, r, routeInfo, router);
                    if (i + 1 != r->edge_count) {
                        cout << ",";
                    }
                    cout << "\n";
                }
                cout << "    ],\n";
                cout << "    \"request_id\": " << id << "\n  }";
            } else {
                cout << "  {\n    \"request_id\": " << id << ",\n    \"error_message\": \"not found\"\n  }";
            }
        }
        if (k != stat_requests.size()) {
            cout << ",";
        }
        cout << "\n";
    }
    cout << "]\n";

    return 0;
}
