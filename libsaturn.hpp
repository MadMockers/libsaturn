/*

This file is part of libsaturn.

libsaturn is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

libsaturn is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libsaturn.  If not, see <http://www.gnu.org/licenses/>.

Your copy of the GNU Lesser General Public License should be in the
file named "LICENSE-LGPL.txt".

*/

#ifndef LIBSATURN_HPP
#define LIBSATURN_HPP

#include "device.hpp"

namespace galaxy {
    namespace saturn {
        /**
         * a dcpu represents a virtual DCPU machine
         */
        class dcpu {
        protected:
            bool interrupts_enabled;
            bool on_fire;
            std::vector<std::unique_ptr<device>> devices;
            std::queue<uint16_t> interrupt_queue;
        public:
            std::array<std::uint16_t, 0x10000> ram;
            uint16_t A, B, C, X, Y, Z, I, J, PC, SP, EX, IA;

            /// initialize the CPU to default values
            dcpu()  :   A(0), B(0), C(0), X(0), Y(0), Z(0), I(0), J(0),
                        PC(0), SP(0), EX(0), IA(0) {}

            /// perform a CPU cycle
            void cycle();

            /// attach a hardware device to the CPU. steals the unique_ptr, and so returns a reference so you can still use it after attaching it.
            device& attach_device(std::unique_ptr<device>);

            /// flash memory with a std::array<uint16_t>
            void flash(std::array<std::uint16_t, 0x10000>);

            /// Reset memory and registers
            void reset();

        };
    }
}

#endif