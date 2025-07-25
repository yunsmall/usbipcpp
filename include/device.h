#pragma once

#include <filesystem>
#include <variant>

#include "Version.h"
#include "SetupPacket.h"
#include "constant.h"
#include "network.h"
#include "interface.h"
#include "type.h"


namespace usbipdcpp {
    class Session;

    class AbstDeviceHandler;

    struct UsbIpIsoPacketDescriptor;

    namespace UsbIpCommand {
        struct OpReqDevlist;
        struct OpReqImport;
        struct UsbIpCmdSubmit;
        struct UsbIpCmdUnlink;
        using CmdVariant = std::variant<OpReqDevlist, OpReqImport, UsbIpCmdSubmit, UsbIpCmdUnlink>;
    }

    struct UsbDevice {
        std::filesystem::path path;
        std::string busid;
        std::uint32_t bus_num;
        std::uint32_t dev_num;
        std::uint32_t speed;
        std::uint16_t vendor_id;
        std::uint16_t product_id;
        Version device_bcd;
        std::uint8_t device_class;
        std::uint8_t device_subclass;
        std::uint8_t device_protocol;
        std::uint8_t configuration_value;
        std::uint8_t num_configurations;
        std::vector<UsbInterface> interfaces;


        UsbEndpoint ep0_in;
        UsbEndpoint ep0_out;

        std::shared_ptr<AbstDeviceHandler> handler;

        template<typename T, typename... Args>
        void with_handler(Args &&... args) {
            handler = std::dynamic_pointer_cast<AbstDeviceHandler>(
                    std::make_shared<T>(*this, std::forward<Args>(args)...));
        }

        [[nodiscard]] std::vector<std::uint8_t> to_bytes_with_interfaces() const;
        [[nodiscard]] std::vector<std::uint8_t> to_bytes_without_interfaces() const;

        //devlist请求的时候要发送接口信息，import请求时不发送接口信息
        [[nodiscard]] std::vector<std::uint8_t> to_bytes() const;
        asio::awaitable<void> from_socket(asio::ip::tcp::socket &sock);

        std::optional<std::pair<UsbEndpoint, std::optional<UsbInterface>>> find_ep(std::uint8_t ep);

        void handle_urb(Session &session, const UsbIpCommand::UsbIpCmdSubmit &cmd,
                        std::uint32_t seqnum, const UsbEndpoint &ep,
                        std::optional<UsbInterface>& interface, std::uint32_t transfer_buffer_length,
                        const SetupPacket &setup_packet, const std::vector<std::uint8_t> &out_data,
                        const std::vector<UsbIpIsoPacketDescriptor> &iso_packet_descriptors, std::error_code &ec);
        void handle_unlink_seqnum(std::uint32_t seqnum);

        bool operator==(const UsbDevice &other) const {
            return path == other.path &&
                   busid == other.busid &&
                   bus_num == other.bus_num &&
                   dev_num == other.dev_num &&
                   speed == other.speed &&
                   vendor_id == other.vendor_id &&
                   product_id == other.product_id &&
                   device_bcd == other.device_bcd &&
                   device_class == other.device_class &&
                   device_subclass == other.device_subclass &&
                   device_protocol == other.device_protocol &&
                   configuration_value == other.configuration_value &&
                   num_configurations == other.num_configurations &&
                   interfaces == other.interfaces;
        }
    };

    static_assert(Serializable<UsbDevice>);
}
