#pragma once

#include <cstddef>
#include <map>
#include <unordered_map>
#include <sstream>

#include "descriptions.h"
#include "svg.h"

class Render {
public:
    Render() = default;

    struct Settings {
        double width;
        double height;
        double padding;
        double stop_radius;
        double line_width;
        int stop_label_font_size;
        Svg::Point stop_label_offset;
        Svg::Color underlayer_color;
        double underlayer_width;
        std::vector<Svg::Color> color_palette;
        int bus_label_font_size;
        Svg::Point bus_label_offset;
        std::vector<std::string> layers;
    };

    explicit Render(Settings&& renderSettings) : settings(std::move(renderSettings)) {}

    std::string RenderMap(const Descriptions::BusesDict& buses_dict, std::map<std::string, Svg::Point>& stops_positions) {
        std::map<std::string, const Descriptions::Bus*> buses;
        for (const auto& [name, ptr] : buses_dict) {
            buses[name] = ptr;
        }
        Svg::Document map;

        for (const auto& l : settings.layers) {
            if (l == "bus_lines") {
                RenderBusLine(buses, stops_positions, map);
            } else if (l == "bus_labels") {
                RenderBusName(buses, stops_positions, map);
            } else if (l == "stop_points") {
                RenderStopCircle(stops_positions, map);
            } else if (l == "stop_labels") {
                RenderStopName(stops_positions, map);
            }
        }

        std::stringstream ss;
        map.Render(ss);

        return ss.str();
    }

    static Settings settingsFromJson(const Json::Dict& render_settings_json) {
        return Render::Settings{
                .width = render_settings_json.at("width").AsDouble(),
                .height = render_settings_json.at("height").AsDouble(),
                .padding = render_settings_json.at("padding").AsDouble(),
                .stop_radius = render_settings_json.at("stop_radius").AsDouble(),
                .line_width = render_settings_json.at("line_width").AsDouble(),
                .stop_label_font_size = render_settings_json.at("stop_label_font_size").AsInt(),

                .stop_label_offset = render_settings_json.at("stop_label_offset").AsPoint(),
                .underlayer_color = render_settings_json.at("underlayer_color").AsColor(),
                .underlayer_width = render_settings_json.at("underlayer_width").AsDouble(),
                .color_palette = [&]() {
                    const auto &color_array = render_settings_json.at("color_palette").AsArray();
                    std::vector<Svg::Color> result(color_array.size());
                    for (size_t i = 0; i < result.size(); ++i) {
                        result[i] = color_array[i].AsColor();
                    }
                    return result;
                }(),
                .bus_label_font_size = render_settings_json.at("bus_label_font_size").AsInt(),
                .bus_label_offset = render_settings_json.at("bus_label_offset").AsPoint(),
                .layers = [&]() {
                    const auto& layers_array = render_settings_json.at("layers").AsArray();
                    std::vector<std::string> result(layers_array.size());
                    for (size_t i = 0; i < result.size(); ++i) {
                        result[i] = layers_array[i].AsString();
                    }
                    return result;
                }()
        };
    }

private:
    void RenderBusLine(const std::map<std::string, const Descriptions::Bus*>& buses,
                       std::map<std::string, Svg::Point>& stops_positions,
                       Svg::Document& map) {
        size_t counter = 0;
        for (const auto& [_, bus] : buses) {
            Svg::Polyline polyline;
            polyline.SetStrokeColor(settings.color_palette[counter]);
            polyline.SetStrokeWidth(settings.line_width);
            polyline.SetStrokeLineCap("round");
            polyline.SetStrokeLineJoin("round");
            for (const auto& stop : bus->stops) {
                auto stop_position = stops_positions[stop];
                polyline.AddPoint(stop_position);
            }
            map.Add(polyline);

            ++counter;
            counter %= settings.color_palette.size();
        }
    }

    void RenderStopCircle(std::map<std::string, Svg::Point>& stops_positions,
                          Svg::Document& map) {
        for (const auto& [_, position] : stops_positions) {
            Svg::Circle circle;
            circle.SetCenter(position);
            circle.SetRadius(settings.stop_radius);
            circle.SetFillColor("white");
            map.Add(circle);
        }
    }

    void RenderStopName(std::map<std::string, Svg::Point>& stops_positions,
                        Svg::Document& map) {
        for (const auto& [name, position] : stops_positions) {
            auto default_stop_text = [](const Svg::Point position, const Render::Settings& settings_,
                                        const std::string& name) {
                Svg::Text text;
                text.SetPoint(position);
                text.SetOffset(settings_.stop_label_offset);
                text.SetFontSize(settings_.stop_label_font_size);
                text.SetFontFamily("Verdana");
                text.SetData(name);
                return text;
            };

            Svg::Text substrate = default_stop_text(position, settings, name);
            Svg::Text inscription = default_stop_text(position, settings, name);

            substrate.SetFillColor(settings.underlayer_color);
            substrate.SetStrokeColor(settings.underlayer_color);
            substrate.SetStrokeWidth(settings.underlayer_width);
            substrate.SetStrokeLineCap("round");
            substrate.SetStrokeLineJoin("round");

            inscription.SetFillColor("black");

            map.Add(substrate);
            map.Add(inscription);
        }
    }

    void RenderBusName(const std::map<std::string, const Descriptions::Bus*>& buses,
                       std::map<std::string, Svg::Point>& stops_positions,
                       Svg::Document& map) {
        size_t counter = 0;
        for (const auto& [name, bus] : buses) {
            renderBusName(bus, stops_positions, map, counter);
            ++counter;
            counter %= settings.color_palette.size();
        }
    }

    void renderBusName(const Descriptions::Bus* bus,
                       std::map<std::string, Svg::Point>& stops_positions,
                       Svg::Document& map,
                       size_t counter) {
        auto default_bus_text = [](const std::string& name,
                                   const Render::Settings& settings_) {
            Svg::Text text;
            text.SetOffset(settings_.bus_label_offset);
            text.SetFontSize(settings_.bus_label_font_size);
            text.SetFontFamily("Verdana");
            text.SetFontWeight("bold");
            text.SetData(name);
            return text;
        };

        Svg::Text substrate = default_bus_text(bus->name, settings);
        Svg::Text inscription = default_bus_text(bus->name, settings);

        substrate.SetFillColor(settings.underlayer_color);
        substrate.SetStrokeColor(settings.underlayer_color);
        substrate.SetStrokeWidth(settings.underlayer_width);
        substrate.SetStrokeLineCap("round");
        substrate.SetStrokeLineJoin("round");

        inscription.SetFillColor(settings.color_palette[counter]);

        substrate.SetPoint(stops_positions[bus->stops.front()]);
        map.Add(substrate);
        inscription.SetPoint(stops_positions[bus->stops.front()]);
        map.Add(inscription);

        if (!(bus->is_round_trip) && bus->stops[bus->stops.size() / 2] != bus->stops[0]) {
            substrate.SetPoint(stops_positions[bus->stops[bus->stops.size() / 2]]);
            map.Add(substrate);
            inscription.SetPoint(stops_positions[bus->stops[bus->stops.size() / 2]]);
            map.Add(inscription);
        }
    }

    Settings settings;
};