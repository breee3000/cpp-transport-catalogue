#include "transport_router.h"

namespace router {

void TransportRouter::SetVertices() {
    for (const auto& [bus_name, bus] : catalogue_.GetAllRoutes()) {
        for (const detail::Stop* stop : bus->bus_route){
            if (stops_info_.count(stop->stop_name) == 0){
                vertices_info_.push_back(VertexInfo({vertices_info_.size(), stop}));
                const VertexInfo* info_ptr = &(vertices_info_.at(vertices_info_.size() - 1));
                stops_info_.emplace(stop->stop_name, info_ptr);
            }
        }
    }
}

RoutesMap TransportRouter::GetRoutes() const {
    RoutesMap routes;
    for (const auto& [bus_name, bus] : catalogue_.GetAllRoutes()) {
        std::vector<const VertexInfo*> route;
        for (const detail::Stop* stop : bus->bus_route){
            route.push_back(stops_info_.at(stop->stop_name));
        }
        routes.emplace(bus_name, Route{bus->is_roundtrip, std::move(route)});
    }
    return routes;
}


void TransportRouter::AddVertex(const Route& route_info, const int from_position, std::string_view bus_name) {
    const auto& vertex_vector = route_info.route;
    ProcessOneDirection(vertex_vector, from_position, vertex_vector.size(), bus_name,[](int& x){++x;});
    if (!route_info.is_cycled){
        ProcessOneDirection(vertex_vector, from_position, -1, bus_name, [](int& x){--x;});
    }
}

void TransportRouter::AddEdgesToGraph(const RoutesMap& routes) {
    for (const auto& [bus_name, bus] : routes){
        for (int i = 0; i < bus.route.size(); i++){
            AddVertex(bus, i, bus_name);
        }
    }
}

std::optional<RouteInfo> TransportRouter::GetRoute(std::string_view stop_from, std::string_view stop_to) const {
    if (stops_info_.count(stop_from) && stops_info_.count(stop_to)){
        VertexId from = stops_info_.at(stop_from)->id;
        VertexId to = stops_info_.at(stop_to)->id;
        std::optional<Router::RouteInfo> result = router_->BuildRoute(from, to);
        if (result){
            std::vector<const EdgeInfo*> result_vector;
            for (EdgeId edge : result->edges){
                result_vector.push_back(&(edges_info_.at(edge)));
            }
            return RouteInfo{result->weight, std::move(result_vector)};
        }
    }
    return std::nullopt;
}

double TransportRouter::GetWaitWeight() const {
    return wait_weight_;
}

} //namespace router
