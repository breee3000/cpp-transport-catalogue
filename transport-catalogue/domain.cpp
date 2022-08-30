#include "domain.h"

detail::Stop::Stop(std::string_view name, double lat, double lng)
    :stop_name(name){
    stop_crd.lat = lat;
    stop_crd.lng = lng;
}

info::BusInfo::BusInfo(std::string_view name, size_t stops, size_t u_stops, double route_length, double route_curvature)
    :bus_name_info(name)
    ,stops_on_route(stops)
    ,unique_stops_on_route(u_stops)
    ,route_length(route_length)
    ,curvature(route_curvature){
}

info::StopInfo::StopInfo(std::string_view name, std::set<std::string_view> list)
    :stop_name_info(name)
    ,bus_list(list){
}
