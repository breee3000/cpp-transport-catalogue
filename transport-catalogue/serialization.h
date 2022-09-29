#pragma once

#include <string>
#include <optional>
#include <sstream>
#include <fstream>

#include "transport_catalogue.h"
#include "json_builder.h"

#include "transport_catalogue.pb.h"
#include "map_renderer.pb.h"
#include "transport_router.h"

namespace serialization {

using TCProto = std::optional<::transport_catalogue_serialize::TransportCatalogue>;

struct SerializationSettings {
    std::string serialize_name;
    std::string deserialize_name;
    TCProto tc_proto;
};

void SaveStops(const json::Dict& query_map, SerializationSettings& base);
void SaveBuses(const json::Dict& query_map, SerializationSettings& base);
std::string GetColor(const json::Node& value);
void SaveRenderSettings(const json::Dict& query_map, SerializationSettings& base);
void SaveRoutingSettings(const json::Dict& query_map, SerializationSettings& base);
void LoadSerializationSettings(const json::Dict& query_map, SerializationSettings& base);

void Serialize(std::istream& input);

void Deserialize(SerializationSettings& settings);

} // namespace serialization
