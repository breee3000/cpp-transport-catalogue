#pragma once

#include <iostream>
#include <string>

#include "transport_catalogue.h"
#include "stat_reader.h"

namespace transport {

namespace detail {

TransportCatalogue Load(std::istream& input);

}

}
