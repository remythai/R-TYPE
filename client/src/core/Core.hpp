/*
** EPITECH PROJECT, 2025
** RTypeClient
** File description:
** Core.hpp
*/

#pragma once

#include "../macros.hpp"
#include <exception>
#include <string>
#include <utility>

namespace CLIENT {
    class Core {
        public:
            class CoreError : public std::exception {
                private:
                    std::string _message;
                public:
                    CoreError(std::string  message) : _message(std::move(message)) {}
                    [[nodiscard]] const char* what() const noexcept override { return _message.c_str(); }
            };

            Core();
            ~Core();

            void run();

        private:
    };
} // namespace CLIENT

int execute_rtypeClient(char **argv);
