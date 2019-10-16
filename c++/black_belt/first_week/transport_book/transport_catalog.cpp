#include "render.h"
#include "svg.h"
#include "transport_catalog.h"

#include <sstream>

using namespace std;

TransportCatalog::TransportCatalog(vector<Descriptions::InputQuery> data, const Json::Dict& routing_settings_json,
        const Json::Dict& render_settings_json) {
  auto stops_end = partition(begin(data), end(data), [](const auto& item) {
    return holds_alternative<Descriptions::Stop>(item);
  });

  double max_lat = std::numeric_limits<double>::min();
  double min_lat = std::numeric_limits<double>::max();
  double max_lon = std::numeric_limits<double>::min();
  double min_lon = std::numeric_limits<double>::max();
  const double eps = std::numeric_limits<double>::epsilon();

  Render::Settings renderSettings{
      .width = render_settings_json.at("width").AsDouble(),
      .height = render_settings_json.at("height").AsDouble(),
      .padding = render_settings_json.at("padding").AsDouble(),
      .stop_radius = render_settings_json.at("stop_radius").AsDouble(),
      .line_width = render_settings_json.at("line_width").AsDouble(),
      .stop_label_font_size = render_settings_json.at("stop_label_font_size").AsInt(),

      .stop_label_offset = [&]() {
          const auto& point = render_settings_json.at("stop_label_offset").AsArray();
          return Svg::Point{point[0].AsDouble(), point[1].AsDouble()};
      }(),
      .underlayer_color = render_settings_json.at("underlayer_color").AsColor(),
      .underlayer_width = render_settings_json.at("underlayer_width").AsDouble(),
      .color_palette = [&]() {
          const auto& color_array = render_settings_json.at("color_palette").AsArray();
          std::vector<Svg::Color> result(color_array.size());
          for (size_t i = 0; i < result.size(); ++i) {
              result[i] = color_array[i].AsColor();
          }
          return result;
      }()
  };

  Descriptions::StopsDict stops_dict;
  for (const auto& item : Range{begin(data), stops_end}) {
    const auto& stop = get<Descriptions::Stop>(item);
    if (stop.position.latitude - max_lat > eps) {
        max_lat = stop.position.latitude;
    }
    if (stop.position.latitude - min_lat < eps) {
        min_lat = stop.position.latitude;
    }
    if (stop.position.longitude - max_lon > eps) {
        max_lon = stop.position.longitude;
    }
    if (stop.position.longitude - min_lon < eps) {
        min_lon = stop.position.longitude;
    }
    stops_dict[stop.name] = &stop;
    stops_.insert({stop.name, {}});
  }

  double zoom_coef = [&]() {
      double lon_diff = max_lon - min_lon;
      double lat_diff = max_lat - min_lat;
      if (std::abs(lon_diff) < eps && std::abs(lat_diff) < eps) {
          return 0.0;
      }
      if (std::abs(lon_diff) < eps) {
          return (renderSettings.height - 2 * renderSettings.padding) / (lat_diff);
      }
      if (std::abs(lat_diff) < eps) {
          return (renderSettings.width - 2 * renderSettings.padding) / (lon_diff);
      }
      return std::min((renderSettings.height - 2 * renderSettings.padding) / (lat_diff),
                      (renderSettings.width - 2 * renderSettings.padding) / (lon_diff));
  }();

  Descriptions::BusesDict buses_dict;
  for (const auto& item : Range{stops_end, end(data)}) {
    const auto& bus = get<Descriptions::Bus>(item);

    buses_dict[bus.name] = &bus;
    buses_[bus.name] = Bus{
      bus.stops.size(),
      ComputeUniqueItemsCount(AsRange(bus.stops)),
      ComputeRoadRouteLength(bus.stops, stops_dict),
      ComputeGeoRouteDistance(bus.stops, stops_dict)
    };

    for (const string& stop_name : bus.stops) {
      stops_.at(stop_name).bus_names.insert(bus.name);
    }
  }

  std::map<std::string, Svg::Point> stops_positions;

  for (const auto& [name, stop] : stops_dict) {
      stops_positions[name] = Svg::Point{
          (stop->position.longitude - min_lon) * zoom_coef + renderSettings.padding,
          (max_lat - stop->position.latitude) * zoom_coef + renderSettings.padding,
      };
  }

  router_ = make_unique<TransportRouter>(stops_dict, buses_dict, routing_settings_json);
  map = Render(std::move(renderSettings)).RenderMap(buses_dict, stops_positions);
}

const TransportCatalog::Stop* TransportCatalog::GetStop(const string& name) const {
  return GetValuePointer(stops_, name);
}

const TransportCatalog::Bus* TransportCatalog::GetBus(const string& name) const {
  return GetValuePointer(buses_, name);
}

optional<TransportRouter::RouteInfo> TransportCatalog::FindRoute(const string& stop_from, const string& stop_to) const {
  return router_->FindRoute(stop_from, stop_to);
}

int TransportCatalog::ComputeRoadRouteLength(
    const vector<string>& stops,
    const Descriptions::StopsDict& stops_dict
) {
  int result = 0;
  for (size_t i = 1; i < stops.size(); ++i) {
    result += Descriptions::ComputeStopsDistance(*stops_dict.at(stops[i - 1]), *stops_dict.at(stops[i]));
  }
  return result;
}

double TransportCatalog::ComputeGeoRouteDistance(
    const vector<string>& stops,
    const Descriptions::StopsDict& stops_dict
) {
  double result = 0;
  for (size_t i = 1; i < stops.size(); ++i) {
    result += Sphere::Distance(
      stops_dict.at(stops[i - 1])->position, stops_dict.at(stops[i])->position
    );
  }
  return result;
}

std::string TransportCatalog::RenderMap() const {
    return map;
}
