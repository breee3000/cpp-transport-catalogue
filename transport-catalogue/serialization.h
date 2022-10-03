#pragma once

#include <fstream>

#include "json_reader.h"

#include "transport_catalogue.pb.h"
#include "map_renderer.pb.h"
#include "transport_router.pb.h"

namespace serialization {

using TCProto = std::optional<::transport_catalogue_serialize::TransportCatalogue>;

void SaveStops(const transport::json_reader::Info& info, TCProto& tc_proto);
void SaveBuses(const transport::json_reader::Info& info, TCProto& tc_proto);
void SaveDistances(const transport::json_reader::Info& info, TCProto& tc_proto);
void SaveRenderSettings(const transport::json_reader::Info& info, TCProto& tc_proto);
void SaveRoutingSettings(const transport::json_reader::Info& info, TCProto& tc_proto);

void Serialize (const transport::json_reader::Info& info);

void LoadBase(transport::TransportCatalogue& tc, TCProto& tc_proto);
void LoadRenderSettings(transport::json_reader::Info& info, TCProto& tc_proto);
void LoadRoutingSettings(transport::json_reader::Info& info, TCProto& tc_proto);

void Deserialize (transport::TransportCatalogue& tc, transport::json_reader::Info& info);

} //namespace serialization
