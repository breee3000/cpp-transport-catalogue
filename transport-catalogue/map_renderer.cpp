#include "map_renderer.h"

using namespace std::literals;

namespace renderer {

void MapRenderer::SetRenderBus() {
    std::vector<geo::Coordinates> geo_coords;
    for (const auto& bus_name : buses_name_view_) {
        const auto& bus = db_.GetRoute(bus_name)->bus_route;
        for (const auto& stop_of_bus : bus) {
            geo_coords.push_back(stop_of_bus->stop_crd);
        }
    }
    const SphereProjector proj{geo_coords.begin(), geo_coords.end(),
                settings_.width, settings_.height, settings_.padding};

    AddLinesToRenderer(proj);
    AddBusNamesToRenderer(proj);
    AddStopCirclesToRenderer(proj);
    AddStopNamesToRenderer(proj);
}

void MapRenderer::AddLinesToRenderer(const SphereProjector& proj) {
    size_t num_color = 0;
    for (const auto& bus_name : buses_name_view_) {
        svg::Polyline polyline_bus;
        const auto& bus = db_.GetRoute(bus_name)->bus_route;
        for (const auto& stop : bus) {
            polyline_bus.AddPoint({ proj(stop->stop_crd).x, proj(stop->stop_crd).y });
        }

        render_bus_.Add(polyline_bus.SetFillColor("none"s)
                        .SetStrokeColor(settings_.color_palette[num_color])
                        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND)
                        .SetStrokeWidth(settings_.line_width));
        num_color++;
        if (num_color >= settings_.color_palette.size()) {
            num_color = 0;
        }
    }
}

void MapRenderer::AddBusNamesToRenderer(const SphereProjector& proj) {
    size_t num_color = 0;
    for (const auto& bus_name : buses_name_view_) {
        const auto& bus = db_.GetRoute(bus_name)->bus_route;
        auto start_stop_coordinates_proj = proj(bus.front()->stop_crd);
        svg::Text text_bus_name;
        text_bus_name
                .SetPosition(svg::Point{start_stop_coordinates_proj.x,
                                        start_stop_coordinates_proj.y})
                .SetOffset(settings_.bus_label_offset)
                .SetFontSize(settings_.bus_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetFontWeight("bold"s)
                .SetData(std::string(bus_name))
                .SetFillColor(settings_.color_palette[num_color]);
        svg::Text text_bus_name_shadow = text_bus_name;
        text_bus_name_shadow
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeColor(settings_.underlayer_color)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

        render_bus_.Add(text_bus_name_shadow);
        render_bus_.Add(text_bus_name);

        if ((bus.size() > 2 && bus[1] == bus[bus.size() - 2] && bus[0] != bus[bus.size() / 2])
                || bus.size() == 3) {
            start_stop_coordinates_proj = proj(bus[bus.size() / 2]->stop_crd);
            text_bus_name.SetPosition(svg::Point{start_stop_coordinates_proj.x,
                                                 start_stop_coordinates_proj.y});
            text_bus_name_shadow.SetPosition(svg::Point{start_stop_coordinates_proj.x,
                                                        start_stop_coordinates_proj.y});

            render_bus_.Add(text_bus_name_shadow);
            render_bus_.Add(text_bus_name);
        }
        num_color++;
        if (num_color >= settings_.color_palette.size()) {
            num_color = 0;
        }
    }
}

void MapRenderer::AddStopCirclesToRenderer(const SphereProjector& proj) {
    for (const auto& stop : stop_names_) {
        auto stop_coordinates_proj = proj(db_.GetStop(stop)->stop_crd);
        svg::Circle circle_stop;
        circle_stop
                .SetCenter(stop_coordinates_proj)
                .SetRadius(settings_.stop_radius)
                .SetFillColor("white"s);
        render_bus_.Add(circle_stop);
    }
}

void MapRenderer::AddStopNamesToRenderer(const SphereProjector& proj) {
    for (const auto& stop : stop_names_) {
        auto stop_coordinates_proj = proj(db_.GetStop(stop)->stop_crd);
        svg::Text text_stop;
        text_stop
                .SetPosition(stop_coordinates_proj)
                .SetOffset(settings_.stop_label_offset)
                .SetFontSize(settings_.stop_label_font_size)
                .SetFontFamily("Verdana"s)
                .SetData(std::string(stop))
                .SetFillColor("black"s);

        svg::Text text_stop_shadow = text_stop;
        text_stop_shadow
                .SetFillColor(settings_.underlayer_color)
                .SetStrokeColor(settings_.underlayer_color)
                .SetStrokeWidth(settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        render_bus_.Add(text_stop_shadow);
        render_bus_.Add(text_stop);
    }
}

void MapRenderer::Print(std::ostream& out) {
    render_bus_.Render(out);
}

}
