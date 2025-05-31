#ifndef MOVEMENT_PROTO_API_RESPONSE_SERIALIZER_HPP
#define MOVEMENT_PROTO_API_RESPONSE_SERIALIZER_HPP

#include <map>
#include <stdexcept>

#include "pb.h"
#include "pb_encode.h"

#include "ipc_data.hpp"
#include "movement_vendor_api_response.hpp"
#include "movement_vendor_api.pb.h"

namespace ipc {
    class MovementProtoApiResponseSerializer {
    public:
        MovementProtoApiResponseSerializer() = default;
        MovementProtoApiResponseSerializer(const MovementProtoApiResponseSerializer&) = default;
        MovementProtoApiResponseSerializer& operator=(const MovementProtoApiResponseSerializer&) = default;
        virtual ~MovementProtoApiResponseSerializer() noexcept = default;

        RawData operator()(const vendor::MovementVendorApiResponse& response) const;
    private:
        static bool encode_string(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);
    };

    inline RawData MovementProtoApiResponseSerializer::operator()(const vendor::MovementVendorApiResponse& response) const {
        using namespace vendor;
        using VendorStatus = vendor::MovementVendorApiResponse::Result;
        const auto status_mapping = std::map<VendorStatus, movement_vendor_api_StatusCode> {
            { VendorStatus::SUCCESS, movement_vendor_api_StatusCode_SUCCESS },
            { VendorStatus::FAILURE, movement_vendor_api_StatusCode_FAILURE },
        };
        const auto pb_status = status_mapping.at(response.result());
        const auto pb_msg = response.message() ? response.message().value() : std::string("");
        const auto pb_response = movement_vendor_api_MovementApiResponse {
            .status = pb_status,
            .message = pb_callback_t {
                .funcs = {
                    .encode = &encode_string,
                },
                .arg = const_cast<std::string *>(&pb_msg),
            },
        };

        enum: int { BUFF_SIZE = 512UL };
        pb_byte_t buffer[BUFF_SIZE];
        pb_ostream_t ostream = pb_ostream_from_buffer(
            buffer,
            BUFF_SIZE
        );
        if (!pb_encode(&ostream, movement_vendor_api_MovementApiResponse_fields, &pb_response)) {
            throw std::runtime_error("failed to encode MovementVendorApiResponse into protocol buffer: " + std::string(PB_GET_ERROR(&ostream)));
        }
        return RawData((const char *)buffer, (const char *)buffer + ostream.bytes_written);
    }

    inline bool MovementProtoApiResponseSerializer::encode_string(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
        if (!arg || !*arg) {
            throw std::runtime_error("encode_string called with null arg");
        }
        const auto str = *static_cast<const std::string *>(*arg);
        if (!pb_encode_tag_for_field(stream, field)) {
            return false;
        }
        return pb_encode_string(stream, (const pb_byte_t *)(str.c_str()), str.size());
    }
}

#endif // MOVEMENT_PROTO_API_RESPONSE_SERIALIZER_HPP