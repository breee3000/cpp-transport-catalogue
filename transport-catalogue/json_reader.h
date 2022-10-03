#pragma once

#include <sstream>

#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace transport {

namespace json_reader {

struct StopInfo {
    std::string name;
    geo::Coordinates coordinate;
};

struct BusInfo {
    std::string name;
    std::vector<std::string_view> stops;
    std::deque<std::string> stops_str;
    bool is_roundtrip;
};

struct DistanceInfo {
    std::string stop_name_from;
    std::string stop_name_to;
    int distance;
};

struct StatRequest {
    int id;
    std::string name;
    std::string type;
    std::string from;
    std::string to;
};

struct SerializationSettings {
    std::string serialize_name;
    std::string deserialize_name;
};

struct Info {
    std::deque<StopInfo> stops;
    std::deque<BusInfo> buses;
    std::deque<DistanceInfo> distances;
    std::deque<StatRequest> stat_requests;
    renderer::RenderSettings render_settings;
    router::RoutingSettings router_settings;
    SerializationSettings serialization_settings;
};

std::string SVGFormatColor(const json::Node& value);

void LoadStops(const json::Dict& query_map, Info& data);
void LoadBuses(const json::Dict& query_map, Info& data);
void LoadRenderSettings(const json::Dict& query_map, Info& data);
void LoadRoutingSettings(const json::Dict& query_map, Info& data);

Info LoadInfo(std::istream& input);

void LoadStat(const json::Array& stat_queries, Info& data);
void LoadDeserializationSettings(const json::Dict& query_map, Info& data);

Info LoadRequests(std::istream& input);

json::Node GetBusInfo(const TransportCatalogue& tc, const StatRequest& stat_request);
json::Node GetStopInfo(const TransportCatalogue& tc, const StatRequest& stat_request);
json::Node GetMapRendererInfo(renderer::MapRenderer& render, const StatRequest& stat_request);
json::Node GetRouteInfo(router::TransportRouter& router, const StatRequest& stat_request);

void Output(TransportCatalogue& tc, Info& data, std::ostream& out);

} //namespace json_reader

} //namespace transport
