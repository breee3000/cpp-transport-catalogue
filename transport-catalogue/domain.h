#pragma once

#include <string_view>
#include <vector>
#include <set>
#include <string>

#include "geo.h"

namespace detail {

struct Stop {
    Stop(std::string name, double lat, double lng);

    std::string stop_name;
    geo::Coordinates stop_crd;
};

struct Bus {
    std::string bus_name;
    std::vector<const Stop*> bus_route;
    bool is_roundtrip;
};

} //namespace detail

namespace info {

struct BusInfo {
    BusInfo(std::string_view name, size_t stops, size_t u_stops, double route_length, double route_curvature);

    std::string_view bus_name_info = "";
    size_t stops_on_route = 0;
    size_t unique_stops_on_route = 0;
    double route_length = 0;
    double curvature = 0.0;
};

struct StopInfo {
    StopInfo(std::string_view name, std::set<std::string_view> list);

    std::string_view stop_name_info;
    std::set<std::string_view> bus_list;
};

} //namespace info
