#pragma once

#include <vector>
#include <deque>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <algorithm>

#include "domain.h"

namespace transport {

class TransportCatalogue {
public:
    struct StopsHasher {
        std::size_t operator () (std::pair<const detail::Stop*,const detail::Stop*> t) const noexcept {
            return std::hash<const void*>{}(t.first) + std::hash<const void*>{}(t.second);
        }
    };

    void AddRoute(const std::string_view &bus_name, const std::vector<std::string_view>& stops, bool is_roundtrip);

    void AddStop(const std::string_view &stop_name, double lat, double lng);

    void AddDistance(const std::string_view &stop_from, const std::string_view &stop_to, double d);

    double GetDistance(const detail::Stop *stop_from, const detail::Stop *stop_to) const;

    const detail::Bus* GetRoute(const std::string_view& bus) const;

    const detail::Stop* GetStop(const std::string_view& stop) const;

    info::BusInfo GetRouteInfo(const std::string_view& bus) const;

    info::StopInfo GetBusList(const std::string_view& stop) const;

    const std::deque<detail::Bus> GetBuses() const;

    const std::unordered_map<std::string_view, detail::Stop*> GetAllStops() const;

    const std::unordered_map<std::string_view, detail::Bus*> GetAllRoutes() const;

private:
    std::deque<detail::Stop> stops_;
    std::deque<detail::Bus> buses_;
    std::unordered_map<std::string_view, detail::Stop*> stopname_to_stop_;
    std::unordered_map<std::string_view, detail::Bus*> busname_to_bus_;
    std::unordered_map<const detail::Stop*, std::set<std::string_view>> busnames_to_stop_;
    std::unordered_map<std::pair<const detail::Stop*,const detail::Stop*>, double, StopsHasher> distances_;
};

}
