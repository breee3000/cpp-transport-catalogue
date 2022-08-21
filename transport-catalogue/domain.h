#pragma once

#include <string_view>
#include <vector>
#include <set>

#include "geo.h"

namespace detail {

struct Stop {
    Stop(std::string_view name, double lat, double lng);

    std::string_view stop_name;
    geo::Coordinates stop_crd;
};

struct Bus {
    std::string_view bus_name;
    std::vector<const Stop*> bus_route;
};

} //namespace detail

namespace info {

struct BusInfo {
    BusInfo(std::string_view name, size_t stops, size_t u_stops, int route_length, double route_curvature);

    std::string_view bus_name_info = "";
    size_t stops_on_route = 0;
    size_t unique_stops_on_route = 0;
    int route_length = 0;
    double curvature = 0.0;
};

struct StopInfo {
    StopInfo(std::string_view name, std::set<std::string_view> list);

    std::string_view stop_name_info;
    std::set<std::string_view> bus_list;
};

} //namespace info