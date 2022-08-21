#include "transport_catalogue.h"

void transport::TransportCatalogue::AddRoute(const std::string_view& bus_name, const std::vector<std::string_view>& stops) {
    detail::Bus bus;
    bus.bus_name = bus_name;
    buses_.push_back(bus);
    for (size_t stop = 0; stop < stops.size(); ++stop) {
        buses_.back().bus_route.push_back(GetStop(stops[stop]));
    }
    for (auto stop : buses_.back().bus_route) {
        busnames_to_stop_.at(stop).insert(buses_.back().bus_name);
    }
    busname_to_bus_[buses_.back().bus_name] = &buses_.back();
}

void transport::TransportCatalogue::AddStop(const std::string_view& stop_name, double lat, double lng) {
    detail::Stop stop = {stop_name, lat, lng};
    stops_.push_back(stop);
    stopname_to_stop_[stops_.back().stop_name] = &stops_.back();
    busnames_to_stop_[&stops_.back()];
}

void transport::TransportCatalogue::AddDistance(const std::string_view &stop_from, const std::string_view &stop_to, double d) {
    const detail::Stop* s_from = GetStop(stop_from);
    const detail::Stop* s_to = GetStop(stop_to);
    distances_[std::pair{s_from, s_to}] = d;
}

double transport::TransportCatalogue::GetDistance(const detail::Stop* stop_from, const detail::Stop* stop_to) const {
    using namespace detail;
    double result = 0;
    std::pair<const Stop*, const Stop*> pair_from_to{stop_from, stop_to};
    std::pair<const Stop*, const Stop*> pair_to_from{stop_to, stop_from};
    if (distances_.count(pair_from_to) != 0) {
        result = distances_.at(pair_from_to);
    } else if (distances_.count(pair_to_from) != 0) {
        result = distances_.at(pair_to_from);
    }
    if (distances_.count(pair_from_to) == 0 && distances_.count(pair_to_from) == 0 && stop_from == stop_to) {
        return 0;
    }
    return result;
}

const detail::Bus* transport::TransportCatalogue::GetRoute(const std::string_view& bus) const {
    auto search_bus = busname_to_bus_.find(bus);
    if (search_bus != end(busname_to_bus_)) {
        return search_bus->second;
    }
    return nullptr;
}

const detail::Stop* transport::TransportCatalogue::GetStop(const std::string_view& stop) const {
    auto search_stop = stopname_to_stop_.find(stop);
    if (search_stop != end(stopname_to_stop_)) {
        return search_stop->second;
    }
    return nullptr;
}

info::BusInfo transport::TransportCatalogue::GetRouteInfo(const std::string_view& bus) const {
    info::BusInfo result{bus, 0, 0, 0, 0.0};
    const detail::Bus* b = GetRoute(bus);
    if (b != nullptr) {
        result.bus_name_info = b->bus_name;
        result.stops_on_route = b->bus_route.size();
        std::unordered_set<std::string_view> unique_stops;
        for (const auto& stop: b->bus_route) {
            unique_stops.insert(stop->stop_name);
        }
        result.unique_stops_on_route = unique_stops.size();
        double route_length_geo = 0.0;
        int route_length = 0;
        for (size_t stop = 0; stop < b->bus_route.size(); ++stop) {
            size_t next_stop = ((stop + 1) < b->bus_route.size()) ? stop + 1 : stop;
            route_length_geo += geo::ComputeDistance(b->bus_route[stop]->stop_crd,
                                                     b->bus_route[next_stop]->stop_crd);
            route_length += GetDistance(b->bus_route[stop], b->bus_route[next_stop]);
        }
        result.route_length = route_length;
        result.curvature = static_cast<double>(route_length)/route_length_geo;
        return result;
    }
    return result;
}

info::StopInfo transport::TransportCatalogue::GetBusList(const std::string_view& stop) const {
    std::set<std::string_view> list{};
    info::StopInfo result{"", list};
    auto s = GetStop(stop);
    auto iter = busnames_to_stop_.find(s);
    if (iter == busnames_to_stop_.end()) {
        return result;
    }
    return { stop, iter->second};
}

const std::deque<detail::Bus> transport::TransportCatalogue::GetBuses() const {
    return buses_;
}
