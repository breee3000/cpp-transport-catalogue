#include "transport_catalogue.h"

transport::detail::Stop::Stop(std::string name, double lat, double lng)
    :stop_name(name){
    stop_crd.lat = lat;
    stop_crd.lng = lng;
}

transport::info::BusInfo::BusInfo(std::string name, size_t stops, size_t u_stops, double lenght_geo, double lenght)
    :bus_name_info(name)
    ,stops_on_route(stops)
    ,unique_stops_on_route(u_stops)
    ,route_length_geo(lenght_geo)
    ,route_length(lenght){
}

transport::info::StopInfo::StopInfo(std::string_view name, std::set<std::string_view> list, bool be_found)
    :stop_name_info(name)
    ,bus_list(list)
    ,found(be_found){
}

void transport::TransportCatalogue::AddRoute(std::string_view bus_name, std::vector<std::string> stops) {
    auto check_name = busname_to_bus_.find(bus_name);
    if (check_name != end(busname_to_bus_)) {
        for (size_t i = stops.size() - 1; i != 0; --i) {
        check_name->second->bus_route.push_back(GetStop(stops[i - 1]));
        }
    } else {
    transport::detail::Bus bus;
    bus.bus_name = std::string(bus_name);
    for (const auto& stop : stops) {
        bus.bus_route.push_back(GetStop(stop));
        //add bussnames to bus
        auto check_stop = busnames_to_stop_.find(stop);
        if (check_stop != end(busnames_to_stop_)) {
            check_stop->second.insert(bus_name);
        }
    }
    buses_.push_back(bus);
    busname_to_bus_[bus_name] = &buses_.back();
    }
}

void transport::TransportCatalogue::AddStop(const std::string_view& stop_name, double lat, double lng) {
    transport::detail::Stop stop = {std::string(stop_name), lat, lng};
    stops_.push_back(stop);
    stopname_to_stop_[stop_name] = &stops_.back();
    busnames_to_stop_[stop_name];
}

void transport::TransportCatalogue::AddDistance(const std::string_view &stop_from, const std::string_view &stop_to, double d) {
    transport::detail::Stop* s_from = GetStop(std::string(stop_from));
    transport::detail::Stop* s_to = GetStop(std::string(stop_to));
    distances_[std::pair{s_from, s_to}] = d;
}

double transport::TransportCatalogue::GetDistance(const transport::detail::Stop* stop_from, const transport::detail::Stop* stop_to) {
    using namespace transport::detail;
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

transport::detail::Bus* transport::TransportCatalogue::GetRoute(const std::string& bus) {
    auto search_bus = busname_to_bus_.find(bus);
    if (search_bus != end(busname_to_bus_)) {
        return search_bus->second;
    }
    return nullptr;
}

transport::detail::Stop* transport::TransportCatalogue::GetStop(const std::string& stop) {
    auto search_stop = stopname_to_stop_.find(stop);
    if (search_stop != end(stopname_to_stop_)) {
        return search_stop->second;
    }
    return nullptr;
}

transport::info::BusInfo transport::TransportCatalogue::GetRouteInfo(const std::string& bus) {
    transport::info::BusInfo result{bus, 0, 0, 0.0, 0.0};
    transport::detail::Bus* b = GetRoute(bus);
    if (b != nullptr) {
        result.bus_name_info = b->bus_name;
        result.stops_on_route = b->bus_route.size();
        std::unordered_set<std::string_view> unique_stops;
        for (const auto& stop: b->bus_route) {
            unique_stops.insert(stop->stop_name);
        }
        result.unique_stops_on_route = unique_stops.size();
        for (size_t s_from = 0; s_from < b->bus_route.size(); ++s_from) {
            size_t s_to = ((s_from + 1) < b->bus_route.size()) ? s_from + 1 : s_from;
            result.route_length_geo += ComputeDistance(b->bus_route[s_from]->stop_crd, b->bus_route[s_to]->stop_crd);
            result.route_length += GetDistance(b->bus_route[s_from], b->bus_route[s_to]);
        }
        return result;
    }
    return result;
}

transport::info::StopInfo transport::TransportCatalogue::GetBusList(const std::string& stop) {
    std::set<std::string_view> list{};
    transport::info::StopInfo result{stop, list, false};
    auto search = busnames_to_stop_.find(stop);
    if(search != end(busnames_to_stop_)) {
        result.stop_name_info = search->first;
        result.bus_list = search->second;
        result.found = true;
        return result;
    }
    return result;
}
