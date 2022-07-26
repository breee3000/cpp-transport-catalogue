#pragma once

#include <iomanip>

#include "input_reader.h"

namespace transport {

namespace detail {

std::ostream& operator<<(std::ostream& os, const info::BusInfo& info);
std::ostream& operator<<(std::ostream& os, const info::StopInfo& info);

void PrintBusInfo(TransportCatalogue& cataloge, const std::string &bus);
void PrintBusListForStop(TransportCatalogue& catalogue, const std::string& stop);

}

}
