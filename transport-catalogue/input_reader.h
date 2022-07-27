#pragma once

#include <iostream>
#include <string>

#include "transport_catalogue.h"
#include "stat_reader.h"

namespace transport {

namespace detail {

void LoadDistances(TransportCatalogue& catalogue, const std::unordered_map<std::string_view, std::string_view>& stops_curvature);
void LoadStops(TransportCatalogue& catalogue, const std::vector<std::string>& query_stops);
void LoadBuses(TransportCatalogue& catalogue, const std::vector<std::string>& query_buses);

TransportCatalogue Load(std::istream& input);

}

}
