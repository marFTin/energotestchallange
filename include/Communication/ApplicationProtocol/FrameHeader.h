#pragma once

#include <cstdint>

namespace Challenge::Communication::ApplicationProtocol {

    //! DTO struct which represent first bytes of application package
    /*!
     *  This struct is very important because it ensures possibility of development
     *  different versions of protocols, with only requirements that all packages will
     *  start from length and version
     */
    struct PacketHeader{
        //! Total length of packet in bytes (includes sizeof(PacketHeader), represented in Network Bytes Order
        uint16_t nboPacketLength;

        //! Version of protocol in Network Bytes Order
        uint16_t nboProtocolVersion;
    };

} // namespace ApplicationProtocol
