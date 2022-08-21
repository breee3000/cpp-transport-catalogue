#pragma once

#include <sstream>

#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"

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
};

struct Info {
    std::deque<StopInfo> stops;
    std::deque<BusInfo> buses;
    std::deque<DistanceInfo> distances;
    std::deque<StatRequest> stat_requests;
    renderer::RenderSettings render_settings;
};

std::string SVGFormatColor(const json::Node& value);

void LoadStops(const json::Dict& query_map, Info& data);
void LoadBuses(const json::Dict& query_map, Info& data);
void LoadStat(const json::Array& stat_queries, Info& data);
void LoadRenderSettings(const json::Dict& query_map, Info& data);

Info LoadInfo(std::istream& input);

void LoadBase(transport::TransportCatalogue& tc, Info& data);

json::Dict GetBusInfo(const TransportCatalogue& tc, const StatRequest& stat_request);
json::Dict GetStopInfo(const TransportCatalogue& tc, const StatRequest& stat_request);
json::Dict GetMapRendererInfo(const TransportCatalogue& tc, const Info& data, const StatRequest& stat_request);

void Output(TransportCatalogue& tc, Info& data, std::ostream& out);

} //namespace json_reader

} //namespace transport
