#include <fstream>
#include <iostream>
#include <string_view>

#include "transport_catalogue.h"
#include "json_reader.h"
#include "serialization.h"

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }
    const std::string_view mode(argv[1]);
    if (mode == "make_base"sv) {
        serialization::Serialize(std::cin);
    } else if (mode == "process_requests"sv) {
        transport::TransportCatalogue tc;
        auto data = transport::json_reader::LoadRequests(std::cin);
        transport::json_reader::LoadInfo(data);
        transport::json_reader::LoadBase(tc, data);
        transport::json_reader::Output(tc, data, std::cout);
    } else {
        PrintUsage();
        return 1;
    }
}