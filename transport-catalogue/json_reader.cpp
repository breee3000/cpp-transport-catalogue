#include "json_reader.h"

using namespace std::literals;

namespace transport {

namespace json_reader {

std::string SVGFormatColor(const json::Node& value) {
    if (value.IsArray()) {
        const auto& rgb_arr = value.AsArray();
        std::string rgb_str = ""s;
        if (rgb_arr.size() == 3) {
            rgb_str = "rgb("s;
        }
        if (rgb_arr.size() == 4) {
            rgb_str = "rgba("s;
        }
        for (size_t i = 0; i < rgb_arr.size(); ++i) {
            if (i > 0) {
                rgb_str += ',';
            }
            if (i != 3) {
                std::ostringstream strs;
                strs << rgb_arr[i].AsInt();
                rgb_str += strs.str();
            } else {
                std::ostringstream strs;
                strs << rgb_arr[i].AsDouble();
                rgb_str += strs.str();
            }
        }
        rgb_str += ')';
        return rgb_str;
    } else {
        return value.AsString();
    }
}

void LoadStops(const json::Dict& query_map, Info& data) {
    StopInfo stop;
    stop.name = query_map.at("name").AsString();
    stop.coordinate.lat = query_map.at("latitude").AsDouble();
    stop.coordinate.lng = query_map.at("longitude").AsDouble();
    data.stops.push_back(std::move(stop));
    auto& distance_map = query_map.at("road_distances").AsMap();
    for (auto& [to_stop, dist] : distance_map) {
        data.distances.push_back({data.stops.back().name, to_stop, dist.AsInt()});
    }
}

void LoadBuses(const json::Dict& query_map, Info& data) {
    BusInfo bus;
    bus.name = query_map.at("name").AsString();
    for (auto& stops_of_bus : query_map.at("stops").AsArray()) {
        bus.stops_str.push_back(stops_of_bus.AsString());
        bus.stops.push_back(bus.stops_str.back());
    }
    if (!query_map.at("is_roundtrip").AsBool()) {
        int size_data_for_bus = static_cast<int>(bus.stops.size());
        for (int i = size_data_for_bus; ; --i) {
            if ((i - 2) < 0) { break; }
            bus.stops.push_back(bus.stops[i - 2]);
        }
    }
    data.buses.push_back(std::move(bus));
}

void LoadStat(const json::Array& stat_queries, Info& data) {
    for (auto& query : stat_queries) {
        auto& query_map = query.AsMap();
        if (query_map.count("name"s)) {
            data.stat_requests.push_back({
                                             query_map.at("id"s).AsInt(),
                                             query_map.at("name"s).AsString(),
                                             query_map.at("type"s).AsString()
                                         });
        } else {
            data.stat_requests.push_back({
                                             query_map.at("id"s).AsInt(),
                                             ""s,
                                             query_map.at("type"s).AsString()
                                         });
        }
    }
}

void LoadRenderSettings(const json::Dict& query_map, Info& data) {
    auto& render_settings = data.render_settings;
    for (const auto& [param, value] : query_map) {
        if (param == "width"s) {
            render_settings.width = value.AsDouble();
        } else if (param == "height"s) {
            render_settings.height = value.AsDouble();
        } else if (param == "padding"s) {
            render_settings.padding = value.AsDouble();
        } else if (param == "stop_radius"s) {
            render_settings.stop_radius = value.AsDouble();
        } else if (param == "line_width"s) {
            render_settings.line_width = value.AsDouble();
        } else if (param == "bus_label_font_size"s) {
            render_settings.bus_label_font_size = value.AsInt();
        } else if (param == "bus_label_offset"s) {
            render_settings.bus_label_offset.x = value.AsArray()[0].AsDouble();
            render_settings.bus_label_offset.y = value.AsArray()[1].AsDouble();
        } else if (param == "stop_label_font_size"s) {
            render_settings.stop_label_font_size = value.AsInt();
        } else if (param == "stop_label_offset"s) {
            render_settings.stop_label_offset.x = value.AsArray()[0].AsDouble();
            render_settings.stop_label_offset.y = value.AsArray()[1].AsDouble();
        } else if (param == "underlayer_color"s) {
            render_settings.underlayer_color = SVGFormatColor(value);
        } else if (param == "underlayer_width"s) {
            render_settings.underlayer_width = value.AsDouble();
        } else if (param == "color_palette"s) {
            for (const auto& color : value.AsArray()) {
                render_settings.color_palette.push_back(SVGFormatColor(color));
            }
        }
    }
}

Info LoadInfo(std::istream& input) {
    Info data;
    auto query_input = json::Load(input);
    auto& root = query_input.GetRoot();
    auto& root_map = root.AsMap();
    if (root_map.count("base_requests")) {
        auto& queries = root_map.at("base_requests").AsArray();
        for (auto& query : queries) {
            auto& query_map = query.AsMap();
            if (query_map.at("type").AsString() == "Stop") {
                LoadStops(query_map, data);
            }
            if (query_map.at("type").AsString() == "Bus") {
                LoadBuses(query_map, data);
            }
        }
    }
    if (root_map.count("stat_requests"s)) {
        auto& queries = root_map.at("stat_requests"s).AsArray();
        LoadStat(queries, data);
    }
    if (root_map.count("render_settings"s)) {
        const auto& setting_map = root_map.at("render_settings"s).AsMap();
        LoadRenderSettings(setting_map, data);
    }
    return data;
}

void LoadBase(transport::TransportCatalogue& tc, Info& data) {
    for (auto& stop : data.stops) {
        tc.AddStop(stop.name, stop.coordinate.lat, stop.coordinate.lng);
    }
    for (const auto& bus : data.buses) {
        tc.AddRoute(bus.name, bus.stops);
    }
    for (const auto& dist : data.distances) {
        tc.AddDistance(dist.stop_name_from, dist.stop_name_to, dist.distance);
    }
}

json::Dict GetBusInfo(const TransportCatalogue& tc, const StatRequest& stat_request) {
    auto bus_info = tc.GetRouteInfo(stat_request.name);
    if (bus_info.stops_on_route == 0) {
        return json::Dict{
            {"request_id"s, stat_request.id},
            {"error_message"s, "not found"s}
        };
    } else {
        return json::Dict{
            {"curvature"s, bus_info.curvature},
            {"request_id"s, stat_request.id},
            {"route_length"s, bus_info.route_length},
            {"stop_count"s, static_cast<int>(bus_info.stops_on_route)},
            {"unique_stop_count"s,  static_cast<int>(bus_info.unique_stops_on_route)}
        };
    }
}

json::Dict GetStopInfo(const TransportCatalogue& tc, const StatRequest& stat_request) {
    auto stop_info = tc.GetBusList(stat_request.name);
    json::Array stops_to_buses_arr;
    stops_to_buses_arr.reserve(stop_info.bus_list.size());
    for (auto& s : stop_info.bus_list) {
        stops_to_buses_arr.emplace_back(std::string(s));
    }
    if (stop_info.stop_name_info.empty()) {
        return json::Dict{
            {"request_id"s, stat_request.id},
            {"error_message"s, "not found"s}
        };
    } else {
        return json::Dict{
            {"buses"s, stops_to_buses_arr},
            {"request_id"s, stat_request.id}
        };
    }
}

json::Dict GetMapRendererInfo(const TransportCatalogue& tc, const Info& data, const StatRequest& stat_request) {
    renderer::MapRenderer render(tc, data.render_settings);
    render.SetRenderBus();
    std::stringstream strm;
    render.Print(strm);
    return json::Dict{
        {"request_id"s, stat_request.id},
        {"map"s, strm.str()}
    };
}

void Output(TransportCatalogue& tc, Info& data, std::ostream& out) {
    json::Array result;
    for (auto& stat_request : data.stat_requests) {
        if (stat_request.type == "Bus") {
            result.emplace_back(std::move(GetBusInfo(tc, stat_request)));
        }
        if (stat_request.type == "Stop") {
            result.emplace_back(std::move(GetStopInfo(tc, stat_request)));
        }
        if (stat_request.type == "Map") {
            result.emplace_back(std::move(GetMapRendererInfo(tc, data, stat_request)));
        }
    }
    json::Print(json::Document{result}, out);
}

} //namespace json_reader

} //namespace transport
