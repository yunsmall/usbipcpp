#pragma once

#include <cstdint>

#include <asio/read.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/ip/tcp.hpp>

#include "type.h"

namespace usbipdcpp {

    inline bool is_little_endian() {
        return asio::detail::socket_ops::host_to_network_short(0x0001) == 0x0100;
    }

    inline uint8_t ntoh(uint8_t num) {
        return num;
    }

    inline uint16_t ntoh(uint16_t num) {
        return asio::detail::socket_ops::network_to_host_short(num);
    }

    inline uint32_t ntoh(uint32_t num) {
        return asio::detail::socket_ops::network_to_host_long(num);
    }

    inline uint64_t ntoh(uint64_t num) {
        if (is_little_endian()) {
            char *data = reinterpret_cast<char *>(&num);
            std::ranges::reverse(data, data + sizeof(num));
            return *reinterpret_cast<uint64_t *>(data);
        }
        return num;
    }

    inline uint8_t hton(uint8_t num) {
        return num;
    }

    inline uint16_t hton(uint16_t num) {
        return asio::detail::socket_ops::host_to_network_short(num);
    }

    inline uint32_t hton(uint32_t num) {
        return asio::detail::socket_ops::host_to_network_long(num);
    }

    inline uint64_t hton(uint64_t num) {
        if (is_little_endian()) {
            return asio::detail::socket_ops::host_to_network_long(num >> 32) |
                   asio::detail::socket_ops::host_to_network_long(num << 32);
        }
        return num;
    }


    template<typename... Args>
    constexpr size_t calculate_total_size() {
        return (sizeof(std::remove_reference_t<Args>) + ...);
    }

    template<std::unsigned_integral... Args>
    asio::awaitable<void> read_from_socket(asio::ip::tcp::socket &sock, Args &... args) {
        constexpr auto total_size = calculate_total_size<Args...>();

        std::array<std::uint8_t, total_size> buffer;
        co_await asio::async_read(sock, asio::buffer(buffer), asio::use_awaitable);
        std::size_t offset = 0;

        auto process = [&](auto &arg) {
            using RawType = std::remove_reference_t<decltype(arg)>;
            RawType tmp;
            std::memcpy(&tmp, &buffer[offset], sizeof(tmp));
            offset += sizeof(tmp);

            arg = ntoh(tmp);
        };

        (process(args), ...);
    }

    inline asio::awaitable<uint32_t> read_u32(asio::ip::tcp::socket &sock) {
        uint32_t result;
        co_await asio::async_read(sock, asio::buffer(&result, sizeof(result)), asio::use_awaitable);
        co_return ntoh(result);
    }

    inline asio::awaitable<uint16_t> read_u16(asio::ip::tcp::socket &sock) {
        uint16_t result;
        co_await asio::async_read(sock, asio::buffer(&result, sizeof(result)), asio::use_awaitable);
        co_return ntoh(result);
    }

    inline asio::awaitable<uint16_t> read_u8(asio::ip::tcp::socket &sock) {
        uint8_t result;
        co_await asio::async_read(sock, asio::buffer(&result, sizeof(result)), asio::use_awaitable);
        co_return ntoh(result);
    }

    template<std::unsigned_integral... Args>
    std::vector<uint8_t> to_data(const Args &... args) {
        // 计算总缓冲区大小
        constexpr size_t total_size = calculate_total_size<Args...>();

        // 创建缓冲区
        data_type buffer;
        buffer.resize(total_size);

        // 处理每个参数
        size_t offset = 0;
        auto process = [&](auto &&arg) {
            using RawType = std::decay_t<decltype(arg)>;
            const RawType net_value = hton(arg);

            // 复制数据到缓冲区
            std::memcpy(buffer.data() + offset, &net_value, sizeof(RawType));
            offset += sizeof(RawType);
        };

        // 展开所有参数
        (process(args), ...);

        return buffer;
    }

    template<typename T>
    void vector_append(data_type &vec, const T &t) {
        using RawType = std::decay_t<T>;

        size_t offset = vec.size();
        vec.resize(vec.size() + sizeof(RawType));
        std::memcpy(vec.data() + offset, &t, sizeof(RawType));
    }

    template<typename... Args>
    void vector_append_to_net(data_type &vec, const Args &... args) {
        // 计算总缓冲区大小
        constexpr size_t total_size = calculate_total_size<Args...>();

        // 扩充缓冲区
        size_t offset = vec.size();
        vec.resize(vec.size() + total_size);

        // 处理每个参数
        auto process = [&](auto &&arg) {
            using RawType = std::decay_t<decltype(arg)>;

            const RawType net_value = hton(arg);

            // 复制数据到缓冲区
            std::memcpy(vec.data() + offset, &net_value, sizeof(RawType));
            offset += sizeof(RawType);
        };

        // 展开所有参数
        (process(args), ...);
    }


    template<typename T>
    concept Serializable = requires(T &&t1, T &&t2, asio::ip::tcp::socket &sock)
    {
        { static_cast<std::decay_t<T>>(t1) == static_cast<std::decay_t<T>>(t2) } -> std::same_as<bool>;
        { static_cast<const std::decay_t<T> &>(t1).to_bytes() } -> std::same_as<data_type>;
        { static_cast<std::decay_t<T>>(t1).from_socket(sock) } -> std::same_as<asio::awaitable<void>>;
    };

    template<Serializable T>
    data_type to_bytes(const T &t) {
        return t.to_bytes();
    }

    template<Serializable T>
    asio::awaitable<void> from_socket(T &t, asio::ip::tcp::socket &sock) {
        return t.from_socket(sock);
    }


}
