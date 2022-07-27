#include "stat_reader.h"


std::ostream& transport::detail::operator<<(std::ostream& os, const transport::info::BusInfo& info) {
    using namespace std::string_literals;
    if (info.stops_on_route == 0) {
        os << "Bus "s << info.bus_name_info << ": not found"s;
    } else {
        os << "Bus "s << info.bus_name_info << ": "s;
        os << info.stops_on_route << " stops on route, "s;
        os << info.unique_stops_on_route << " unique stops, "s;
        os << info.route_length << " route length, "s;
        os << std::setprecision(6) << info.route_length/info.route_length_geo << " curvature"s;
    }
    return os;
}

std::ostream& transport::detail::operator<<(std::ostream& os, const transport::info::StopInfo& info) {
    using namespace std::string_literals;
    if (info.found == false) {
        os << "Stop "s << info.stop_name_info << ": not found"s;
    } else if (info.found == true && info.bus_list.empty()) {
        os << "Stop "s << info.stop_name_info << ": no buses"s;
    } else {
        os << "Stop "s << info.stop_name_info << ": buses"s;
        for (const auto& bus: info.bus_list) {
            os << " "s << bus;
        }
    }
    return os;
}

void transport::detail::PrintBusInfo(transport::TransportCatalogue& catalogue, const std::string &bus) {
    std::cout << catalogue.GetRouteInfo(bus) << std::endl;
}

void transport::detail::PrintBusListForStop(transport::TransportCatalogue& catalogue, const std::string& stop) {
    std::cout << catalogue.GetBusList(stop) << std::endl;
}

void transport::detail::Output(transport::TransportCatalogue& catalogue, std::istream& input) {
    std::string line = "";
    getline(input,line);
    size_t query_output_count = std::stoi(line);
    std::vector<std::string> query_output;
    for (size_t i = 0; i < query_output_count; ++i) {
        getline(input, line);
        query_output.push_back(line);
    }
    for (const std::string_view& line : query_output) {
        if (line[0] == 'B') {
            std::string_view bus_name = line.substr(4, (line.size() - 4));
            transport::detail::PrintBusInfo(catalogue, std::string(bus_name));
        } else {
            std::string_view stop_name = line.substr(5, (line.size() - 5));
            transport::detail::PrintBusListForStop(catalogue, std::string(stop_name));
        }
    }
}
