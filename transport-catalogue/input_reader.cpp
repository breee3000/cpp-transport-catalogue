#include "input_reader.h"

transport::TransportCatalogue transport::detail::Load(std::istream& input) {
    TransportCatalogue result;
    std::string line = "";
    getline(input, line);
    size_t query_input_count = std::stoi(line);
    std::vector<std::string> query_stops;
    std::vector<std::string> query_buses;
    for (size_t i = 0; i < query_input_count; ++i) {
        getline(input, line);
        if (line.find("Stop") < line.find(':')) {
            query_stops.push_back(line);
        } else {
            query_buses.push_back(line);
        }
    }
    //LoadStops
    std::unordered_map<std::string_view, std::string_view> stops_curvature;
    std::deque<std::string_view> stop_names;
    std::deque<std::string_view> stop_distances;
    for (const std::string_view& line: query_stops) {

        std::string_view::size_type start = line.find("Stop") + 4;
        std::string_view::size_type start_pos = line.find_first_not_of(' ', start);
        std::string_view::size_type end = line.find(':');
        std::string_view::size_type end_pos = line.find_last_not_of(' ', end - 1);
        std::string_view stop_name = line.substr(start_pos, end_pos - start_pos + 1);
        stop_names.push_back(stop_name);

        std::string_view::size_type end_lat_pos = line.find_first_of(',');
        std::string_view::size_type end_lng_pos = line.find_first_of(',', end_lat_pos + 1);
        double stop_crd_lat = stod(std::string(line.substr(line.find(':') + 1, end_lat_pos)));
        double stop_crd_lng = stod(std::string(line.substr(line.find(',') + 1, end_lng_pos)));
        result.AddStop(stop_names.back(), stop_crd_lat, stop_crd_lng);

        if (line.find(" to ", end_lng_pos + 1) != std::string_view::npos) {
            stop_distances.push_back(line.substr(end_lng_pos + 1, line.size() - (end_lng_pos + 1)));
            stops_curvature[stop_names.back()] = stop_distances.back();
        }
    }
    //LoadDistances
    for (const auto& [stop, to_stop_line]: stops_curvature) {
        std::string_view to_stop;
        double dist;
        std::string_view::size_type pos_start = 0;
        while (pos_start < to_stop_line.size() - 1) {
            //distance
            std::string_view::size_type d_start = to_stop_line.find_first_not_of(' ', pos_start);
            std::string_view::size_type d_end = to_stop_line.find_first_of('m', d_start);
            std::string_view::size_type count_t = d_end - d_start;
            dist = stod(std::string(to_stop_line.substr(d_start, count_t)));
            //stops
            std::string_view::size_type s_start =
                    to_stop_line.find(" to ", pos_start) != std::string_view::npos ?
                        to_stop_line.find(" to ", pos_start) + 4 :
                        0;
            std::string_view::size_type s_end =
                    to_stop_line.find(",", pos_start) != std::string_view::npos ?
                        to_stop_line.find(",", pos_start) :
                        to_stop_line.size();
            to_stop = to_stop_line.substr(s_start, s_end - s_start);
            pos_start = s_end + 1;
            result.AddDistance(stop, to_stop, dist);
        }
    }
    //LoadBuses
    std::deque<std::string_view> bus_names;
    for (const std::string_view& line: query_buses) {

        std::string_view::size_type b_start = line.find("Bus") + 3;
        std::string_view::size_type b_start_pos = line.find_first_not_of(' ', b_start);
        std::string_view::size_type b_end = line.find(':');
        std::string_view::size_type b_end_pos = line.find_last_not_of(' ', b_end - 1);
        std::string_view bus_name = line.substr(b_start_pos, b_end_pos - b_start_pos + 1);
        bus_names.push_back(bus_name);

        std::vector<std::string> stop_names;
        std::string::size_type start = line.find(':') + 1;
        std::string::size_type end = 0;
        std::string::size_type pos_end = 0;

        if (line.find('>') != std::string::npos) {
            while (end < line.size() - 1) {
                std::string stop = "";
                end =
                        line.find_first_of('>', pos_end) ?
                            line.find_first_of('>', pos_end) :
                            line.size() - 1;
                stop = line.substr(start, end - start);
                stop_names.push_back(stop.substr(stop.find_first_not_of(' '),
                                                 (stop.find_last_not_of(' ') + 1) -
                                                 stop.find_first_not_of(' ')));
                pos_end = end + 1;
                start = pos_end;
            }
            result.AddRoute(bus_names.back(), stop_names);
        }
        if (line.find('-') != std::string::npos) {
           while (end < line.size() - 1) {
                std::string stop = "";
                end =
                        line.find_first_of('-', pos_end) ?
                            line.find_first_of('-', pos_end) :
                            line.size() - 1;
                stop = line.substr(start, end - start);
                stop_names.push_back(stop.substr(stop.find_first_not_of(' '),
                                                 (stop.find_last_not_of(' ') + 1) -
                                                 stop.find_first_not_of(' ')));
                pos_end = end + 1;
                start = pos_end;
            }
            result.AddRoute(bus_names.back(), stop_names);
            result.AddRoute(bus_names.back(), stop_names);
        }
    }
    //OutputQuery
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
            transport::detail::PrintBusInfo(result, std::string(bus_name));
        } else {
            std::string_view stop_name = line.substr(5, (line.size() - 5));
            transport::detail::PrintBusListForStop(result, std::string(stop_name));
        }
    }

    return result;
}
