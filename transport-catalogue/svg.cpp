#include "svg.h"

namespace svg {

using namespace std::literals;

void OstreamColorPrinter::operator()(std::monostate) const {
    out << "none"sv;
}

void OstreamColorPrinter::operator()(std::string color) const {
    out << color;
}

void OstreamColorPrinter::operator()(Rgb rgb) const {
    out << "rgb("sv << static_cast<uint16_t>(rgb.red) << ","sv
        << static_cast<uint16_t>(rgb.green) << ","sv
        << static_cast<uint16_t>(rgb.blue) << ")"sv;
}

void OstreamColorPrinter::operator()(Rgba rgba) const {
    out << "rgba("sv << static_cast<uint16_t>(rgba.red) << ","sv
        << static_cast<uint16_t>(rgba.green) << ","sv
        << static_cast<uint16_t>(rgba.blue) << ","sv
        << rgba.opacity << ")"sv;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
    switch (line_cap) {
    case StrokeLineCap::BUTT:
        out << "butt"sv;
        break;
    case StrokeLineCap::ROUND:
        out << "round"sv;
        break;
    case StrokeLineCap::SQUARE:
        out << "square"sv;
        break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
    switch (line_join) {
    case StrokeLineJoin::ARCS:
        out << "arcs"sv;
        break;
    case StrokeLineJoin::BEVEL:
        out << "bevel"sv;
        break;
    case StrokeLineJoin::MITER:
        out << "miter"sv;
        break;
    case StrokeLineJoin::MITER_CLIP:
        out << "miter-clip"sv;
        break;
    case StrokeLineJoin::ROUND:
        out << "round"sv;
        break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit(OstreamColorPrinter{ out }, color);
    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();
    RenderObject(context);
    context.out << std::endl;
}

Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << " <circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << " <polyline points=\""sv;
    for (size_t i = 0; i < points_.size(); ++i) {
        if (i > 0) {
            out << " "sv;
        }
        out << points_[i].x << ","sv << points_[i].y;
    }
    out << "\""sv;
    RenderAttrs(context.out);
    out << "/>"sv;
}

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << " <text x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
    out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
    out << "font-size=\""sv << size_<<"\""sv;
    if (font_family_) {
        out << " font-family=\""sv << *font_family_ << "\""sv;
    }
    if (font_weight_) {
        out << " font-weight=\""sv << *font_weight_<<"\""sv;
    }
    RenderAttrs(context.out);
    std::string escaped_data = ""s;
    std::vector<std::string> special_symbols{
        "&quot;"s, "&apos;"s, "&lt;"s, "&gt;"s, "&amp;"s
    };
    for (const char& c : data_) {
        switch (c) {
        case '"':
            escaped_data += special_symbols[0];
            break;
        case '\'':
            escaped_data += special_symbols[1];
            break;
        case '<':
            escaped_data += special_symbols[2];
            break;
        case '>':
            escaped_data += special_symbols[3];
            break;
        case '&':
            escaped_data += special_symbols[4];
            break;
        default:
            escaped_data += c;
            break;
        }
    }
    out << ">"sv;
    out << data_;
    out << "</text>"sv;
}

void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>"sv << std::endl;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">"sv << std::endl;
    for (const auto& obj : objects_) {
        RenderContext ctx(out, 2, 2);
        obj->Render(ctx);
    }
    out << "</svg>"sv;
}

}  // namespace svg
