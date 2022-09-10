#pragma once

#include <memory>
#include <map>

#include "router.h"
#include "domain.h"
#include "transport_catalogue.h"

namespace router {

struct RoutingSettings {
    int bus_wait_time = 0;
    double bus_velocity = 0.0;
};

using VertexId = size_t;
using EdgeId = size_t;

struct VertexInfo {
    VertexId id = 0;
    const detail::Stop* stop_info;
};

struct Route {
    bool is_cycled = false;
    std::vector<const VertexInfo*> route;
};

struct EdgeInfo {
    std::string_view bus_name;
    int span = 0;
    double weight = 0.0;
    std::string_view from_stop;
    std::string_view to_stop;
};

struct RouteInfo {
    double overall_time = 0.0;
    std::vector<const EdgeInfo*> route_edges;
};

using Graph = graph::DirectedWeightedGraph<double>;
using Router = graph::Router<double>;
using RoutesMap = std::map<std::string_view, Route>;

class TransportRouter {
public:
    explicit TransportRouter(transport::TransportCatalogue& catalogue, RoutingSettings routing_settings)
        : catalogue_(catalogue)
        , wait_weight_(routing_settings.bus_wait_time)
        , bus_velocity_(routing_settings.bus_velocity) {
        vertices_info_.reserve(catalogue.GetAllStops().size());
        SetVertices();
        RoutesMap routes = GetRoutes();
        graph_ = std::make_unique<Graph>(vertices_info_.size());
        AddEdgesToGraph(std::move(routes));
        router_ = std::make_unique<Router>(*graph_);
    }

    std::optional<RouteInfo> GetRoute(std::string_view from, std::string_view to) const;

    double GetWaitWeight() const;

private:
    const transport::TransportCatalogue& catalogue_;
    double wait_weight_ = 0.0;
    double bus_velocity_ = 0.0;
    std::unique_ptr<Graph> graph_;
    std::unique_ptr<Router> router_;
    std::vector<VertexInfo> vertices_info_;
    std::unordered_map<std::string_view,const VertexInfo*> stops_info_;
    std::unordered_map<EdgeId, EdgeInfo> edges_info_;

    void SetVertices();

    RoutesMap GetRoutes() const;

    void AddVertex(const Route& route_info, const int from_position, std::string_view bus_name);

    void AddEdgesToGraph(const RoutesMap& routes);

    template<typename T>
    void ProcessOneDirection(std::vector<const VertexInfo*> vertex_vector, const int from_pos, const int lim, std::string_view bus_name, T op) {
        int distance = 0;
        double weight = wait_weight_;
        int span = 0;
        const VertexInfo* from = vertex_vector[from_pos];
        int i = from_pos;
        for (op(i); i != lim; op(i)){
            const VertexInfo* to = vertex_vector[i];
            int distance_between_adjacent_stops = catalogue_.GetDistance(vertex_vector[i < from_pos ? i + 1 : i - 1]->stop_info, to->stop_info);
            distance += distance_between_adjacent_stops;
            weight += distance_between_adjacent_stops / (bus_velocity_ / 6.0 * 100.0);
            span += 1;
            if (from != to){
                edges_info_.emplace(graph_->AddEdge({from->id, to->id, weight}),
                                    EdgeInfo{bus_name, span, weight,
                                             from->stop_info->stop_name,
                                             to->stop_info->stop_name});
            }
        }
    }
};

} //namespace router
