#include <sstream>

#include "input_reader.h"

int main() {
    transport::TransportCatalogue catalogue = transport::detail::Load(std::cin);
    transport::detail::Output(catalogue, std::cin);
    return 0;
}
