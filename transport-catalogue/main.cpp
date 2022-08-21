#include <iostream>
#include <sstream>

#include "transport_catalogue.h"
#include "json_reader.h"

using namespace std;

int main() {
    transport::TransportCatalogue tc;
    auto data = transport::json_reader::LoadInfo(cin);
    transport::json_reader::LoadBase(tc, data);
    transport::json_reader::Output(tc, data, cout);
    
//    renderer::MapRenderer render(tc, data.render_settings);
//    render.SetRenderBus();
//    render.Print(std::cout);
}