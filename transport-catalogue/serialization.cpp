#include "serialization.h"

namespace serialization {

void SaveStops(const transport::json_reader::Info& info, TCProto& tc_proto) {
    transport_catalogue_serialize::Stop stop;
    for (const auto& stop_info : info.stops) {
        stop.set_name(stop_info.name);
        stop.mutable_coordinate()->set_lat(stop_info.coordinate.lat);
        stop.mutable_coordinate()->set_lng(stop_info.coordinate.lng);
        tc_proto.value().mutable_stops()->Add(std::move(stop));
    }
}

void SaveBuses(const transport::json_reader::Info& info, TCProto& tc_proto) {
    transport_catalogue_serialize::Bus bus;
    for (const auto& bus_info : info.buses) {
        bus.set_name(bus_info.name);
        bus.set_is_roundtrip(bus_info.is_roundtrip);
        for (const auto& stop: bus_info.stops) {
            bus.add_stops(std::string(stop));
        }
        tc_proto.value().mutable_buses()->Add(std::move(bus));
    }
}

void SaveDistances(const transport::json_reader::Info& info, TCProto& tc_proto) {
    transport_catalogue_serialize::Distance distance;
    for (const auto& distanse_info : info.distances) {
        distance.set_stop_name_from(distanse_info.stop_name_from);
        distance.set_stop_name_to(distanse_info.stop_name_to);
        distance.set_distance(distanse_info.distance);
        tc_proto.value().mutable_distances()->Add(std::move(distance));
    }
}

void SaveRenderSettings(const transport::json_reader::Info& info, TCProto& tc_proto) {
    map_renderer_serialize::RenderSettings render_settings;
    render_settings.set_width(info.render_settings.width);
    render_settings.set_height(info.render_settings.height);
    render_settings.set_padding(info.render_settings.padding);
    render_settings.set_stop_radius(info.render_settings.stop_radius);
    render_settings.set_line_width(info.render_settings.line_width);
    render_settings.set_bus_label_font_size(info.render_settings.bus_label_font_size);
    render_settings.set_bus_label_offset_x(info.render_settings.bus_label_offset.x);
    render_settings.set_bus_label_offset_y(info.render_settings.bus_label_offset.y);
    render_settings.set_stop_label_font_size(info.render_settings.stop_label_font_size);
    render_settings.set_stop_label_offset_x(info.render_settings.stop_label_offset.x);
    render_settings.set_stop_label_offset_y(info.render_settings.stop_label_offset.y);
    render_settings.set_underlayer_color(std::get<std::string>(info.render_settings.underlayer_color));
    render_settings.set_underlayer_width(info.render_settings.underlayer_width);
    for (const auto& color: info.render_settings.color_palette) {
        render_settings.add_color_palette(std::get<std::string>(color));
    }
    *tc_proto.value().mutable_render_settings() = std::move(render_settings);
}

void SaveRoutingSettings(const transport::json_reader::Info& info, TCProto& tc_proto) {
    transport_router_serialize::RoutingSettings routing_settings;
    routing_settings.set_bus_wait_time(info.router_settings.bus_wait_time);
    routing_settings.set_bus_velocity(info.router_settings.bus_velocity);
    *tc_proto.value().mutable_routing_settings() = std::move(routing_settings);
}

void Serialize (const transport::json_reader::Info& info) {
    TCProto tc_proto;
    tc_proto = std::make_optional<::transport_catalogue_serialize::TransportCatalogue>();
    SaveStops(info, tc_proto);
    SaveBuses(info, tc_proto);
    SaveDistances(info, tc_proto);
    SaveRenderSettings(info, tc_proto);
    SaveRoutingSettings(info, tc_proto);
    std::ofstream out_file(info.serialization_settings.serialize_name, std::ios::binary);
    tc_proto.value().SerializeToOstream(&out_file);
}

void LoadBase(transport::TransportCatalogue& tc, TCProto& tc_proto) {
    for (const auto& stop_data: *tc_proto.value().mutable_stops()) {
        tc.AddStop(stop_data.name(), stop_data.coordinate().lat(), stop_data.coordinate().lng());
    }
    for (const auto& bus_data : *tc_proto.value().mutable_buses()) {
        std::vector<std::string> stops;
        for (const auto& stop: bus_data.stops()) {
            stops.push_back(stop);
        }
        tc.AddRoute(bus_data.name(), stops, bus_data.is_roundtrip());
    }
    for (const auto& distance_data : *tc_proto.value().mutable_distances()) {
        tc.AddDistance(distance_data.stop_name_from(), distance_data.stop_name_to(), distance_data.distance());
    }
}

void LoadRenderSettings(transport::json_reader::Info& info, TCProto& tc_proto) {
    const auto& settings = *tc_proto.value().mutable_render_settings();
    info.render_settings.width = settings.width();
    info.render_settings.height = settings.height();
    info.render_settings.padding = settings.padding();
    info.render_settings.stop_radius = settings.stop_radius();
    info.render_settings.line_width = settings.line_width();
    info.render_settings.bus_label_font_size = settings.bus_label_font_size();
    info.render_settings.bus_label_offset.x = settings.bus_label_offset_x();
    info.render_settings.bus_label_offset.y = settings.bus_label_offset_y();
    info.render_settings.stop_label_font_size = settings.stop_label_font_size();
    info.render_settings.stop_label_offset.x = settings.stop_label_offset_x();
    info.render_settings.stop_label_offset.y = settings.stop_label_offset_y();
    info.render_settings.underlayer_color = settings.underlayer_color();
    info.render_settings.underlayer_width = settings.underlayer_width();
    for (const auto& color : settings.color_palette()) {
        info.render_settings.color_palette.push_back(color);
    }
}

void LoadRoutingSettings(transport::json_reader::Info& info, TCProto& tc_proto) {
    const auto& router_settings = *tc_proto.value().mutable_routing_settings();
    info.router_settings.bus_wait_time = router_settings.bus_wait_time();
    info.router_settings.bus_velocity = router_settings.bus_velocity();
}

void Deserialize (transport::TransportCatalogue& tc, transport::json_reader::Info& info) {
    TCProto tc_proto;
    tc_proto = std::make_optional<::transport_catalogue_serialize::TransportCatalogue>();
    std::ifstream in_file(info.serialization_settings.deserialize_name, std::ios::binary);
    tc_proto.value().ParseFromIstream(&in_file);
    LoadBase(tc, tc_proto);
    LoadRenderSettings(info, tc_proto);
    LoadRoutingSettings(info, tc_proto);
}

} //namespace serialization
