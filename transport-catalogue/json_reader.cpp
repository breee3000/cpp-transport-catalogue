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
    bus.is_roundtrip = query_map.at("is_roundtrip").AsBool();
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
                                             query_map.at("type"s).AsString(),
                                             ""s,
                                             ""s
                                         });
        } else if (query_map.count("from"s)) {
            data.stat_requests.push_back({
                                             query_map.at("id"s).AsInt(),
                                             ""s,
                                             query_map.at("type"s).AsString(),
                                             query_map.at("from"s).AsString(),
                                             query_map.at("to"s).AsString()
                                         });
        } else {
            data.stat_requests.push_back({
                                             query_map.at("id"s).AsInt(),
                                             ""s,
                                             query_map.at("type"s).AsString(),
                                             ""s,
                                             ""s
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

void LoadRoutingSettings(const json::Dict& query_map, Info& data) {
    auto& router_settings = data.router_settings;
    for (const auto& [param, value] : query_map) {
        if (param == "bus_wait_time"s) {
            router_settings.bus_wait_time = value.AsInt();
        } else if (param == "bus_velocity"s) {
            router_settings.bus_velocity = value.AsDouble();
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
    if (root_map.count("routing_settings"s)) {
        const auto& setting_route = root_map.at("routing_settings"s).AsMap();
        LoadRoutingSettings(setting_route, data);
    }
    return data;
}

void LoadBase(transport::TransportCatalogue& tc, Info& data) {
    for (auto& stop : data.stops) {
        tc.AddStop(stop.name, stop.coordinate.lat, stop.coordinate.lng);
    }
    for (const auto& bus : data.buses) {
        tc.AddRoute(bus.name, bus.stops, bus.is_roundtrip);
    }
    for (const auto& dist : data.distances) {
        tc.AddDistance(dist.stop_name_from, dist.stop_name_to, dist.distance);
    }
}

json::Node GetBusInfo(const TransportCatalogue& tc, const StatRequest& stat_request) {
    auto bus_info = tc.GetRouteInfo(stat_request.name);
    if (bus_info.stops_on_route == 0) {
        return json::Builder{}.StartDict()
        .Key("request_id"s).Value(stat_request.id)
        .Key("error_message"s).Value("not found"s)
        .EndDict().Build();
    } else {
        return json::Builder{}.StartDict()
        .Key("curvature"s).Value(bus_info.curvature)
        .Key("request_id"s).Value(stat_request.id)
        .Key("route_length"s).Value(bus_info.route_length)
        .Key("stop_count"s).Value(static_cast<int>(bus_info.stops_on_route))
        .Key("unique_stop_count"s).Value(static_cast<int>(bus_info.unique_stops_on_route))
        .EndDict().Build();
    }
}

json::Node GetStopInfo(const TransportCatalogue& tc, const StatRequest& stat_request) {
    auto stop_info = tc.GetBusList(stat_request.name);
    json::Array stops_to_buses_arr;
    stops_to_buses_arr.reserve(stop_info.bus_list.size());
    for (auto& s : stop_info.bus_list) {
        stops_to_buses_arr.emplace_back(std::string(s));
    }
    if (stop_info.stop_name_info.empty()) {
        return json::Builder{}.StartDict()
        .Key("request_id"s).Value(stat_request.id)
        .Key("error_message"s).Value("not found"s)
        .EndDict().Build();
    } else {
        return json::Builder{}.StartDict()
        .Key("buses"s).Value(stops_to_buses_arr)
        .Key("request_id"s).Value(stat_request.id)
        .EndDict().Build();
    }
}

json::Node GetMapRendererInfo(const TransportCatalogue& tc, const Info& data, const StatRequest& stat_request) {
    renderer::MapRenderer render(tc, data.render_settings);
    render.AddRoutes();
    std::stringstream strm;
    render.Print(strm);
    return json::Builder{}.StartDict()
    .Key("request_id"s).Value(stat_request.id)
    .Key("map"s).Value(strm.str())
    .EndDict().Build();
}

json::Node GetRouteInfo(const StatRequest& stat_request, router::TransportRouter &router) {
    std::optional<router::RouteInfo> result = router.GetRoute(stat_request.from, stat_request.to);
    json::Builder builder;
    if (!result.has_value()){
        builder.StartDict()
                .Key("request_id"s).Value(stat_request.id)
                .Key("error_message"s).Value("not found"s)
                .EndDict();
    } else {
        const auto& route_edges = result->route_edges;
        const double time = result->overall_time;
        builder.StartDict()
                .Key("request_id"s).Value(stat_request.id)
                .Key("items"s)
                .StartArray();
        for (const router::EdgeInfo* edge : route_edges){
            builder.StartDict()
                    .Key("stop_name"s).Value(std::string(edge->from_stop))
                    .Key("time"s).Value(router.GetWaitWeight())
                    .Key("type"s).Value("Wait"s)
                    .EndDict()
                    .StartDict()
                    .Key("bus"s).Value(std::string(edge->bus_name))
                    .Key("span_count"s).Value(edge->span)
                    .Key("time"s).Value(edge->weight - router.GetWaitWeight())
                    .Key("type"s).Value("Bus"s)
                    .EndDict();
        }
        builder.EndArray()
                .Key("total_time"s).Value(time)
                .EndDict();
    }
    return builder.Build();
}

void Output(TransportCatalogue& tc, Info& data, std::ostream& out) {
    json::Array result;
    router::TransportRouter router(tc, data.router_settings);
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
        if (stat_request.type == "Route") {
            result.emplace_back(std::move(GetRouteInfo(stat_request, router)));
        }
    }
    json::Print(json::Document{result}, out);
}

} //namespace json_reader

} //namespace transport
