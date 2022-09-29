#include "serialization.h"

namespace serialization {

void SaveStops(const json::Dict& query_map, SerializationSettings& base) {
    transport_catalogue_serialize::Stop stop;
    stop.set_name(query_map.at("name").AsString());
    stop.mutable_coordinate()->set_lat(query_map.at("latitude").AsDouble());
    stop.mutable_coordinate()->set_lng(query_map.at("longitude").AsDouble());
    base.tc_proto.value().mutable_stops()->Add(std::move(stop));
    auto& distance_map = query_map.at("road_distances").AsMap();
    for (auto& [to_stop, dist] : distance_map) {
        transport_catalogue_serialize::Distance distance;
        distance.set_stop_name_from(query_map.at("name").AsString());
        distance.set_stop_name_to(to_stop);
        distance.set_distance(dist.AsInt());
        base.tc_proto.value().mutable_distances()->Add(std::move(distance));
    }
}

void SaveBuses(const json::Dict& query_map, SerializationSettings& base) {
    transport_catalogue_serialize::Bus bus;
    bus.set_name(query_map.at("name").AsString());
    bus.set_is_roundtrip(query_map.at("is_roundtrip").AsBool());
    for (auto& stops_of_bus : query_map.at("stops").AsArray()) {
        bus.add_stops(stops_of_bus.AsString());
    }
    base.tc_proto.value().mutable_buses()->Add(std::move(bus));
}

std::string GetColor(const json::Node& value) {
    if (value.IsArray()) {
        const auto& rgb_arr = value.AsArray();
        std::string rgb_str = "";
        if (rgb_arr.size() == 3) {
            rgb_str = "rgb(";
        }
        if (rgb_arr.size() == 4) {
            rgb_str = "rgba(";
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

void SaveRenderSettings(const json::Dict& query_map, SerializationSettings& base) {
    map_renderer_serialize::RenderSettings render_settings;
    for (const auto& [param, value] : query_map) {
        if (param == "width") {
            render_settings.set_width(value.AsDouble());
        } else if (param == "height") {
            render_settings.set_height(value.AsDouble());
        } else if (param == "padding") {
            render_settings.set_padding(value.AsDouble());
        } else if (param == "stop_radius") {
            render_settings.set_stop_radius(value.AsDouble());
        } else if (param == "line_width") {
            render_settings.set_line_width(value.AsDouble());
        } else if (param == "bus_label_font_size") {
            render_settings.set_bus_label_font_size(value.AsInt());
        } else if (param == "bus_label_offset") {
            render_settings.set_bus_label_offset_x(value.AsArray()[0].AsDouble());
            render_settings.set_bus_label_offset_y(value.AsArray()[1].AsDouble());
        } else if (param == "stop_label_font_size") {
            render_settings.set_stop_label_font_size(value.AsInt());
        } else if (param == "stop_label_offset") {
            render_settings.set_stop_label_offset_x(value.AsArray()[0].AsDouble());
            render_settings.set_stop_label_offset_y(value.AsArray()[1].AsDouble());
        } else if (param == "underlayer_color") {
            render_settings.set_underlayer_color(GetColor(value));
        } else if (param == "underlayer_width") {
            render_settings.set_underlayer_width(value.AsDouble());
        } else if (param == "color_palette") {
            for (const auto& color : value.AsArray()) {
                render_settings.add_color_palette(GetColor(color));
            }
        }
    }
    *base.tc_proto.value().mutable_render_settings() = std::move(render_settings);
}

void SaveRoutingSettings(const json::Dict& query_map, SerializationSettings& base) {
    transport_router_serialize::RoutingSettings routing_settings;
    for (const auto& [param, value] : query_map) {
        if (param == "bus_wait_time") {
            routing_settings.set_bus_wait_time(value.AsInt());
        } else if (param == "bus_velocity") {
            routing_settings.set_bus_velocity(value.AsDouble());
        }
    }
    *base.tc_proto.value().mutable_routing_settings() = std::move(routing_settings);
}

void LoadSerializationSettings(const json::Dict& query_map, SerializationSettings& base) {
    base.serialize_name = query_map.at("file").AsString();
}

void Serialize(std::istream& input) {
    SerializationSettings base;
    base.tc_proto = std::make_optional<::transport_catalogue_serialize::TransportCatalogue>();
    auto query_input = json::Load(input);
    auto& root = query_input.GetRoot();
    auto& root_map = root.AsMap();
    if (root_map.count("base_requests")) {
        auto& queries = root_map.at("base_requests").AsArray();
        for (auto& query : queries) {
            auto& query_map = query.AsMap();
            if (query_map.at("type").AsString() == "Stop") {
                SaveStops(query_map, base);
            }
            if (query_map.at("type").AsString() == "Bus") {
                SaveBuses(query_map, base);
            }
        }
    }
    if (root_map.count("render_settings")) {
        const auto& setting_map = root_map.at("render_settings").AsMap();
        SaveRenderSettings(setting_map, base);
    }
    if (root_map.count("routing_settings")) {
        const auto& setting_route = root_map.at("routing_settings").AsMap();
        SaveRoutingSettings(setting_route, base);
    }
    if (root_map.count("serialization_settings")) {
        const auto& serialization_settings = root_map.at("serialization_settings").AsMap();
        LoadSerializationSettings(serialization_settings, base);
    }
    std::ofstream out_file(base.serialize_name, std::ios::binary);
    base.tc_proto.value().SerializeToOstream(&out_file);
}

void Deserialize(SerializationSettings& settings) {
    settings.tc_proto = std::make_optional<::transport_catalogue_serialize::TransportCatalogue>();
    std::ifstream in_file(settings.deserialize_name, std::ios::binary);
    settings.tc_proto.value().ParseFromIstream(&in_file);
}

} // namespace serialization
