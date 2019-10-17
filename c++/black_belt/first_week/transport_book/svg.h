#pragma once

#include <iostream>
#include <fstream>
#include <memory>
#include <utility>
#include <vector>
#include <optional>

namespace Svg {
    struct Point {
        Point() = default;
        Point(double x_, double y_) : x(x_), y(y_) {}
        double x, y;

        void Render(std::ostream& os) const {
            os << x << "," << y;
        }
    };

    struct Rgb {
        Rgb() = default;
        Rgb(int red_, int green_, int blue_) : red(red_), green(green_), blue(blue_) {}
        int red, green, blue;

        void Render(std::ostream& os) const {
            os << "rgb(" << red << "," << green << "," << blue << ")";
        }
    };

    struct Color {
        Color() {
            color = "none";
        }
        Color(std::string color_) : color(std::move(color_)) {}
        Color(const char* c) : color(c) {}
        Color(Rgb rgb) {
            color = "rgb(" + std::to_string(rgb.red) + "," + std::to_string(rgb.green)  + "," + std::to_string(rgb.blue) + ")";
        }
        Color(int r, int g, int b, double a) {
            color = "rgba(" + std::to_string(r) + "," + std::to_string(g)  + "," + std::to_string(b) + "," + std::to_string(a) + ")";
        }

        void Render(std::ostream& os) const {
            os << color;
        }

    private:
        std::string color;
    };

    static Color NoneColor;

    class Figure {
    public:
        Figure() = default;
        ~Figure() = default;

        void SetFillColor(const Color& fill_) {
            fill = fill_;
        }

        void SetStrokeColor(const Color& stroke_) {
            stroke = stroke_;
        }

        void SetStrokeWidth(double width) {
            stroke_width = width;
        }

        void SetStrokeLineCap(const std::string& linecap) {
            stroke_linecap = linecap;
        }

        void SetStrokeLineJoin(const std::string& linejoin) {
            stroke_linejoin = linejoin;
        }

        virtual void Render(std::ostream& os) {
            os << "fill=\\\"";
            fill.Render(os);
            os << R"(\" stroke=\")";
            stroke.Render(os);
            os << R"(\" stroke-width=\")" << stroke_width << "\\\"";
            if (stroke_linecap) {
                os << " stroke-linecap=\\\"" << stroke_linecap.value() << "\\\"";
            }
            if (stroke_linejoin) {
                os << " stroke-linejoin=\\\"" << stroke_linejoin.value() << "\\\"";
            }
        }

    protected:
        Color fill = NoneColor;
        Color stroke = NoneColor;
        double stroke_width = 1.0;
        std::optional<std::string> stroke_linecap;
        std::optional<std::string> stroke_linejoin;
    };

    class Circle : public Figure {
    public:
        Circle& SetCenter(Point center) {
            cx = center.x;
            cy = center.y;
            return *this;
        }

        Circle& SetRadius(double radius) {
            r = radius;
            return *this;
        }

        void Render(std::ostream& os) override {
            os << "<circle cx=\\\"" << cx << R"(\" cy=\")" << cy << R"(\" r=\")" << r << "\\\" ";
            Figure::Render(os);
            os << "/>";
        }

        Circle& SetFillColor(const Color& fill_) {
            Figure::SetFillColor(fill_);
            return *this;
        }

        Circle& SetStrokeColor(const Color& stroke_) {
            Figure::SetStrokeColor(stroke_);
            return *this;
        }

        Circle& SetStrokeWidth(double width) {
            Figure::SetStrokeWidth(width);
            return *this;
        }

        Circle& SetStrokeLineCap(const std::string& linecap) {
            Figure::SetStrokeLineCap(linecap);
            return *this;
        }

        Circle& SetStrokeLineJoin(const std::string& linejoin) {
            Figure::SetStrokeLineJoin(linejoin);
            return *this;
        }

    private:
        double cx = 0;
        double cy = 0;
        double r = 1.0;
    };

    class Polyline : public Figure {
    public:
        void Render(std::ostream& os) override {
            std::string points_string;
            os << "<polyline points=\\\"";
            for (const Point& point : points) {
                point.Render(os);
                os << " ";
            }
            os << "\\\" ";
            Figure::Render(os);
            os << "/>";
        }

        Polyline& AddPoint(Point point) {
            points.push_back(point);
            return *this;
        }

        Polyline& SetFillColor(const Color& fill_) {
            Figure::SetFillColor(fill_);
            return *this;
        }

        Polyline& SetStrokeColor(const Color& stroke_) {
            Figure::SetStrokeColor(stroke_);
            return *this;
        }

        Polyline& SetStrokeWidth(double width) {
            Figure::SetStrokeWidth(width);
            return *this;
        }

        Polyline& SetStrokeLineCap(const std::string& linecap) {
            Figure::SetStrokeLineCap(linecap);
            return *this;
        }

        Polyline& SetStrokeLineJoin(const std::string& linejoin) {
            Figure::SetStrokeLineJoin(linejoin);
            return *this;
        }

    private:
        std::vector<Point> points;
    };

    class Text : public Figure {
    public:
        Text& SetPoint(Point point) {
            x = point.x;
            y = point.y;
            return *this;
        }

        Text& SetOffset(Point point) {
            dx = point.x;
            dy = point.y;
            return *this;
        }

        Text& SetFontSize(uint32_t size) {
            font_size = size;
            return *this;
        }

        Text& SetFontFamily(const std::string& font) {
            font_family = font;
            return *this;
        }

        Text& SetData(const std::string& data) {
            text = data;
            return *this;
        }

        void Render(std::ostream& os) override {
            os << "<text ";
            Figure::Render(os);
            os << " x=\\\"" << x << R"(\" y=\")" << y << "\\\"";
            os << " dx=\\\"" << dx << R"(\" dy=\")" << dy << "\\\"";
            os << " font-size=\\\"" << font_size << "\\\"";
            if (font_family) {
                os << " font-family=\\\"" << font_family.value() << "\\\"";
            }
            if (font_weight) {
                os << " font-weight=\\\"" << font_weight.value() << "\\\"";
            }
            os << ">" << text;
            os << "</text>";
        }

        Text& SetFillColor(const Color& fill_) {
            Figure::SetFillColor(fill_);
            return *this;
        }

        Text& SetStrokeColor(const Color& stroke_) {
            Figure::SetStrokeColor(stroke_);
            return *this;
        }

        Text& SetStrokeWidth(double width) {
            Figure::SetStrokeWidth(width);
            return *this;
        }

        Text& SetStrokeLineCap(const std::string& linecap) {
            Figure::SetStrokeLineCap(linecap);
            return *this;
        }

        Text& SetStrokeLineJoin(const std::string& linejoin) {
            Figure::SetStrokeLineJoin(linejoin);
            return *this;
        }

        Text& SetFontWeight(const std::string& font_weight_) {
            font_weight = font_weight_;
            return *this;
        }

    private:
        double x = 0;
        double y = 0;
        double dx = 0;
        double dy = 0;
        uint32_t font_size = 1;
        std::optional<std::string> font_family;
        std::string text;
        std::optional<std::string> font_weight;
    };

    class Document {
    public:
        Document() = default;

        template <typename AnyFigure>
        void Add(const AnyFigure& figure) {
            objects.push_back(std::make_shared<AnyFigure>(figure));
        }

        void Render(std::ostream& os) {
            os << R"(<?xml version=\"1.0\" encoding=\"UTF-8\" ?>)";
            os << R"(<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">)";
            for (auto& obj : objects) {
                obj->Render(os);
            }
            os << "</svg>";
        }

    private:
        std::vector<std::shared_ptr<Figure>> objects;
    };
}
