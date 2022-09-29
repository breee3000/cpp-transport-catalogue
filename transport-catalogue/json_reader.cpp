#include "json_reader.h"

using namespace std::literals;

namespace transport {

namespace json_reader {

void LoadStops(const transport_catalogue_serialize::Stop& stop_data, Info& data) {
    StopInfo stop;
    stop.name = stop_data.name();
    stop.coordinate.lat = stop_data.coordinate().lat();
    stop.coordinate.lng = stop_data.coordinate().lng();
    data.stops.push_back(std::move(stop));
}

void LoadBuses(const transport_catalogue_serialize::Bus& bus_data, Info& data) {
    BusInfo bus;
    bus.name = bus_data.name();
    bus.is_roundtrip = bus_data.is_roundtrip();
    for (auto& stops_of_bus : bus_data.stops()) {
        bus.stops_str.push_back(stops_of_bus);
        bus.stops.push_back(bus.stops_str.back());
    }
    if (!bus.is_roundtrip) {
        int size_data_for_bus = static_cast<int>(bus.stops.size());
        for (int i = size_data_for_bus; ; --i) {
            if ((i - 2) < 0) {break;}
            bus.stops.push_back(bus.stops[i - 2]);
        }
    }
    data.buses.push_back(std::move(bus));
}

void LoadDistances(const transport_catalogue_serialize::Distance& distance_data, Info& data) {
    data.distances.push_back({distance_data.stop_name_from(), distance_data.stop_name_to(), distance_data.distance()});
}

void LoadRenderSettings(const map_renderer_serialize::RenderSettings& settings, Info& data) {
    auto& render_settings = data.render_settings;
    render_settings.width = settings.width();
    render_settings.height = settings.height();
    render_settings.padding = settings.padding();
    render_settings.stop_radius = settings.stop_radius();
    render_settings.line_width = settings.line_width();
    render_settings.bus_label_font_size = settings.bus_label_font_size();
    render_settings.bus_label_offset.x = settings.bus_label_offset_x();
    render_settings.bus_label_offset.y = settings.bus_label_offset_y();
    render_settings.stop_label_font_size = settings.stop_label_font_size();
    render_settings.stop_label_offset.x = settings.stop_label_offset_x();
    render_settings.stop_label_offset.y = settings.stop_label_offset_y();
    render_settings.underlayer_color = settings.underlayer_color();
    render_settings.underlayer_width = settings.underlayer_width();
    for (const auto& color : settings.color_palette()) {
        render_settings.color_palette.push_back(color);
    }
}

void LoadRoutingSettings(const transport_router_serialize::RoutingSettings& settings, Info& data) {
    auto& router_settings = data.router_settings;
    router_settings.bus_wait_time = settings.bus_wait_time();
    router_settings.bus_velocity = settings.bus_velocity();
}

void LoadSerializationSettings(const json::Dict& query_map, Info& data) {
    auto& serialization_settings = data.serialization_settings;
    serialization_settings.serialize_name = query_map.at("file"s).AsString();
}

void LoadInfo(Info& data) {
    serialization::SerializationSettings serialization_settings;
    serialization_settings.deserialize_name = data.serialization_settings.deserialize_name;
    serialization::Deserialize(serialization_settings);
    auto settings = serialization_settings.tc_proto;
    for (auto& stop : *settings.value().mutable_stops()) {
        LoadStops(stop, data);
    }
    for (auto& bus : *settings.value().mutable_buses()) {
        LoadBuses(bus, data);
    }
    for (auto& distance : *settings.value().mutable_distances()) {
        LoadDistances(distance, data);
    }
    const auto& setting_map = *settings.value().mutable_render_settings();
    LoadRenderSettings(setting_map, data);
    
    const auto& setting_route = *settings.value().mutable_routing_settings();
    LoadRoutingSettings(setting_route, data);
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

void LoadDeserializationSettings(const json::Dict& query_map, Info& data) {
    auto& serialization_settings = data.serialization_settings;
    serialization_settings.deserialize_name = query_map.at("file"s).AsString();
}

Info LoadRequests(std::istream& input) {
    Info data;
    auto query_input = json::Load(input);
    auto& root = query_input.GetRoot();
    auto& root_map = root.AsMap();
    if (root_map.count("stat_requests"s)) {
        auto& queries = root_map.at("stat_requests"s).AsArray();
        LoadStat(queries, data);
    }
    if (root_map.count("serialization_settings"s)) {
        const auto& serialization_settings = root_map.at("serialization_settings"s).AsMap();
        LoadDeserializationSettings(serialization_settings, data);
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
