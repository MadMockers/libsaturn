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

#include <libsaturn.hpp>
#include <device.hpp>
#include <m35fd.hpp>

#define DEBUG(code) if (debug) {std::cout << code << std::endl;}


void galaxy::saturn::m35fd::interrupt()
{
    bool debug = true;
    switch(cpu->A) {
        /**
         * Poll device;
         * Sets B to the current state and C to the last error
         * since the last device poll.
         */
        case 0:
	    DEBUG("Last error since poll; " << last_error_since_poll << ", current_state; " << current_state);
	    cpu->C = last_error_since_poll;
	    cpu->B = current_state;
	    break;

        /**
         * Set interupt;
         * Enables interrupts and sets the message to X if X is anything
         * other than 0, disables interrupts if X is 0. When interrupts are enabled,
         * the M35FD will trigger an interrupt on the DCPU-16 whenever the state or
         * error message changes.
         */
        case 1:
            DEBUG("Setting interrupt message to; 0x" << std::hex << cpu->X);
            // set interupt message to X
            interrupt_message = cpu->X;
            break;

        /**
         * Read sector
         * Reads sector X to DCPU ram starting at Y.
         * Sets B to 1 if reading is possible and has been started, anything else if it
         * fails. Reading is only possible if the state is STATE_READY or
         * STATE_READY_WP.
         * Protects against partial reads.
         */
        case 2:
            if (current_state == STATE_READY || current_state == STATE_READY_WP){
//                DEBUG("Reading")
                int sector = cpu->X;
                int read_to = cpu->Y;

                if (!(0 <= sector && sector <= SECTOR_NUM)) {
                    // ensure the user is not trying to read from outside the floppy disk image
                    cpu->B = 0;
  //                  DEBUG("Out of sector range; sector was " << sector << ", sector range is 0-" << SECTOR_NUM);
                } else if (!(0 <= read_to && read_to <= cpu->RAM_SIZE)) {
                    // ensure the user is not trying to read from outside the ram
                    cpu->B = 0;
                } else {
                    // if everything seems to be in order...
    //                DEBUG("Everything seems to be in order");
                    int track_seek_time = get_track_seek_time(current_track, sector);
                    int read_from = sector * SECTOR_SIZE;

                    for (int i=0; i < SECTOR_SIZE; i++){
/*                        DEBUG(
                            "Writing value " << block_image[read_from + i] <<
                            " from position 0x" << std::hex << read_from + i <<
                            " to position 0x" << std::hex << read_to + i);*/
                        cpu->ram[read_to + i] = block_image[read_from + i];
                    }
                    cpu->B = 1;
                }
            } else {
                cpu->B = 0;
            }
            if (cpu->B == 0) DEBUG("Reading failed");
            break;

        /**
         * Write sector;
         * Writes sector X from DCPU ram starting at Y.
         * Sets B to 1 if writing is possible and has been started, anything else if it
         * fails. Writing is only possible if the state is STATE_READY.
         * Protects against partial writes.
         */
        case 3:
            if (current_state == STATE_READY) {
                if (is_read_only) {
                    // the drive is set to be read only, error out
                    last_error_since_poll = ERROR_PROTECTED;
                    cpu->B = 0;
                } else {
                    int sector = cpu->X;
                    int read_from = cpu->Y;

                    if (!(0 <= sector && sector <= SECTOR_NUM)) {
                        // make sure that the user is not assuming there are more sectors than there are
                        cpu->B = 0;
                    } else if (!(0 <= read_from && read_from <= cpu->RAM_SIZE)) {
                        // ensure the user is not trying to read from outside the ram?
                        cpu->B = 0;
                    } else {
                        // if everything seems to be in order...
//                        DEBUG("Everything seems to be in order...");
                        int read_to = sector * SECTOR_SIZE;
		        int track_seek_time = get_track_seek_time(current_track, sector);

                        for (int i=0; i < SECTOR_SIZE; i++){
                /*                DEBUG(
                                    "Writing 0x" << std::hex << cpu->ram[read_from + i] << " from 0x" << std::hex << read_from + i <<
                                    " to 0x" << std::hex << (read_to + i));*/
                            block_image[read_to + i] = cpu->ram[read_from + i];
                        }
                        cpu->B = 1;
                    }
                }
            } else {
                cpu->B = 0;
            }
            if (cpu->B == 0) DEBUG("Writing failed") 
            break;
    }
}

void galaxy::saturn::m35fd::cycle() {
}


int galaxy::saturn::m35fd::get_track_seek_time(int current_track, int sector) {
    int tracks_seeked = (current_track / SECTORS_PER_TRACK) - (sector / SECTORS_PER_TRACK);

    // ensure it aint negitive; better way to do this?
    if (!tracks_seeked >= 0) {
	tracks_seeked = 0 - tracks_seeked;
    }

    int track_seek_time = tracks_seeked * MILLISECONDS_PER_TRACK_SEEKED;
    return track_seek_time;
}

