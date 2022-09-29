#pragma once

#include <sstream>

#include "json.h"
#include "json_builder.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"

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

struct Info {
    std::deque<StopInfo> stops;
    std::deque<BusInfo> buses;
    std::deque<DistanceInfo> distances;
    std::deque<StatRequest> stat_requests;
    renderer::RenderSettings render_settings;
    router::RoutingSettings router_settings;
    serialization::SerializationSettings serialization_settings;
};

void LoadStops(const transport_catalogue_serialize::Stop& stop_data, Info& data);
void LoadBuses(const transport_catalogue_serialize::Bus& bus_data, Info& data);
void LoadDistances(const transport_catalogue_serialize::Distance& distance_data, Info& data);
void LoadRenderSettings(const map_renderer_serialize::RenderSettings& settings, Info& data);
void LoadRoutingSettings(const transport_router_serialize::RoutingSettings& settings, Info& data);
void LoadSerializationSettings(const json::Dict& query_map, Info& data);

void LoadInfo(Info& data) ;

void LoadStat(const json::Array& stat_queries, Info& data);
void LoadDeserializationSettings(const json::Dict& query_map, Info& data);

Info LoadRequests(std::istream& input);

void LoadBase(transport::TransportCatalogue& tc, Info& data);

json::Node GetBusInfo(const TransportCatalogue& tc, const StatRequest& stat_request);
json::Node GetStopInfo(const TransportCatalogue& tc, const StatRequest& stat_request);
json::Node GetMapRendererInfo(const TransportCatalogue& tc, const Info& data, const StatRequest& stat_request);
json::Node GetRouteInfo(const StatRequest& stat_request, router::TransportRouter& router);

void Output(TransportCatalogue& tc, Info& data, std::ostream& out);

} //namespace json_reader

} //namespace transport
