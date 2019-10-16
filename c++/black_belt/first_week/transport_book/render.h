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
    };

    explicit Render(Settings&& renderSettings) : settings(std::move(renderSettings)) {}

    std::string RenderMap(const Descriptions::BusesDict& buses_dict, std::map<std::string, Svg::Point>& stops_positions) {
        std::map<std::string, const Descriptions::Bus*> buses;
        for (const auto& [name, ptr] : buses_dict) {
            buses[name] = ptr;
        }
        Svg::Document map;

        // buses' lines rendering
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

        // stops' circles rendering
        for (const auto& [_, position] : stops_positions) {
            Svg::Circle circle;
            circle.SetCenter(position);
            circle.SetRadius(settings.stop_radius);
            circle.SetFillColor("white");
            map.Add(circle);
        }

        // stops' names rendering
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

        std::stringstream ss;
        map.Render(ss);

        return ss.str();
    }

private:
    Settings settings;
};