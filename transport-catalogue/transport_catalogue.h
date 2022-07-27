#pragma once

#include <string>
#include <vector>
#include <deque>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <algorithm>

#include "geo.h"

namespace transport {

namespace detail {

struct Stop {
    Stop(std::string name, double lat, double lng);

    std::string stop_name;
    geo::Coordinates stop_crd;
};

struct Bus {
    std::string bus_name;
    std::vector<Stop*> bus_route;
    double bus_route_lenght_geo = 0.0;
    double bus_route_length = 0.0;
};

} //namespace detail

namespace info {

struct BusInfo {
    BusInfo(std::string name, size_t stops, size_t u_stops, double lenght_geo, double lenght);

    std::string bus_name_info = "";
    size_t stops_on_route = 0;
    size_t unique_stops_on_route = 0;
    double route_length_geo = 0.0;
    double route_length = 0.0;
};

struct StopInfo {
    StopInfo(std::string_view name, std::set<std::string_view> list, bool be_found);

    std::string_view stop_name_info;
    std::set<std::string_view> bus_list;
    bool found;
};

} //namespace info

class TransportCatalogue {
public:

    struct StopsHasher {
        std::size_t operator () (std::pair<const detail::Stop*,const detail::Stop*> t) const noexcept {
            return std::hash<const void*>{}(t.first) + std::hash<const void*>{}(t.second);
        }
    };

    void AddRoute(std::string_view bus_name, std::vector<std::string> stops);

    void AddStop(const std::string_view &stop_name, double lat, double lng);

    void AddDistance(const std::string_view &stop_from, const std::string_view &stop_to, double d);

    double GetDistance(const detail::Stop *stop_from, const detail::Stop *stop_to);

    detail::Bus* GetRoute(const std::string& bus);

    detail::Stop* GetStop(const std::string& stop);

    info::BusInfo GetRouteInfo(const std::string& bus);

    info::StopInfo GetBusList(const std::string& stop);

private:
    std::deque<detail::Stop> stops_;
    std::deque<detail::Bus> buses_;
    std::unordered_map<std::string_view, detail::Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, detail::Bus*> busname_to_bus_;
    std::unordered_map<std::string_view, std::set<std::string_view>> busnames_to_stop_;
    std::unordered_map<std::pair<const detail::Stop*,const detail::Stop*>, double, StopsHasher> distances_;
};

} //namespace transport
