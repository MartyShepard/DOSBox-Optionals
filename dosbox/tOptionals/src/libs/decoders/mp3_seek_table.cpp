/**
 * DOSBox MP3 Seek Table handler, Copyright 2018 Kevin R. Croft (krcroft@gmail.com)
 * The seek table handler makes use of the following single-header public libraries:
 *   - dr_mp3: http://mackron.github.io/dr_mp3.html, by David Reid
 *   - archive: https://github.com/voidah/archive, by Arthur Ouellet
 *   - xxHash: http://cyan4973.github.io/xxHash, by Yann Collet
 *
 *  This seek table handler is free software: you can redistribute
 *  it and/or modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  along with DOSBox.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

// System headers
#include <sys/stat.h>
#include <fstream>
#include <string>
#include <map>

// Local headers
#include "xxhash.h"
#include "../../../include/logging.h"
#include "mp3_seek_table.h"

// C++ scope modifiers
using std::map;
using std::vector;
using std::string;
using std::ios_base;
using std::ifstream;
using std::ofstream;

// Identifies a valid seek-table version 2 file
#define SEEK_TABLE_IDENTIFIER "st-v2"

const size_t get_file_size(const char* filename) {
    struct stat stat_buf;
    int rc = stat(filename, &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

const Uint64 calculate_stream_hash(struct SDL_RWops* const context) {

    // Save the current stream position
    const Sint64 original_pos = SDL_RWtell(context);

    // seek to the end and get the stream size
    SDL_RWseek(context, 0, RW_SEEK_END);
    const Sint64 stream_size = SDL_RWtell(context);
    if (stream_size <= 0) {
        LOG_MSG("MP3: get_stream_size returned %ld, but should be positive", stream_size);
        return 0;
    }

    // Feed the hash content from the middle of our file in hopes of the most uniqueness.
    // We're trying to avoid content that might be duplicated across MP3s, like:
    // 1. ID3 tag filler content, which might be boiler plate or all empty
    // 2. Trailing silence or similar zero-PCM content
    const Uint32 tail_size = (stream_size > 32768) ? 32768 : stream_size;
    const Sint64 mid_pos = static_cast<Sint64>(stream_size/2.0) - tail_size;
    SDL_RWseek(context, mid_pos >= 0 ? mid_pos : 0, RW_SEEK_SET);

    // Prepare our read buffer and counter:
    vector<char> buffer(1024, 0);
    Uint32 total_bytes_read = 0;

    // Initialize xxHash's state using the stream_size as our seed.
    // Seeding with the stream_size provide a second level of uniqueness
    // in the unlikely scenario that two files of different length happen to
    // have the same trailing 32KB of content.  The different seeds will produce
    // unique hashes.
    XXH64_state_t* const state = XXH64_createState();
    const Uint64 seed = stream_size;
    XXH64_reset(state, seed);

    while (total_bytes_read < tail_size) {
        // read a chunk of data
        const size_t bytes_read = SDL_RWread(context, buffer.data(), 1, buffer.size());

        if (bytes_read != 0) {
            // update our hash if we read data
            XXH64_update(state, buffer.data(), bytes_read);
            total_bytes_read += bytes_read;
        } else {
            break;
        }
    }

    // restore the stream position
    SDL_RWseek(context, original_pos, RW_SEEK_SET);

    const Uint64 hash = XXH64_digest(state);
    XXH64_freeState(state);
    return hash;
}

const Uint64 generate_new_seek_points(const char* filename,
                                      const Uint64& stream_hash,
                                      drmp3* const p_dr,
                                      map<Uint64, vector<drmp3_seek_point_serial> >& seek_points_table,
                                      map<Uint64, drmp3_uint64>& pcm_frame_count_table,
                                      vector<drmp3_seek_point_serial>& seek_points_vector) {

    // initial empty counts
    drmp3_uint64 mp3_frame_count(0);
    drmp3_uint64 pcm_frame_count(0);

    // get the mp3 and pcm frame counts from the stream
    drmp3_bool8 result = drmp3_get_mp3_and_pcm_frame_count(p_dr,
                                 &mp3_frame_count,
                                 &pcm_frame_count);

    if (result != DRMP3_TRUE || mp3_frame_count == 0 || pcm_frame_count == 0) {
        LOG_MSG("MP3: failed to determine mp3 and pcm frame counts from stream");
        return 0;
    }

    // calculate our desired number of seek points
    drmp3_uint32 num_seek_points = 1 + mp3_frame_count / 10;
    seek_points_vector.resize(num_seek_points);
    result = drmp3_calculate_seek_points(p_dr,
                     &num_seek_points,
                     reinterpret_cast<drmp3_seek_point*>(seek_points_vector.data()));

    if (result != DRMP3_TRUE || num_seek_points == 0) {
        LOG_MSG("MP3: failed to calculate seek points for stream");
        return 0;
    }

    // the calculate function provides us with the actual number of generated seek
    // points in the num_seek_points variable; so if this differs from expected then we
    // need to resize (ie: shrink) our vector
    if (num_seek_points != seek_points_vector.size())
        seek_points_vector.resize(num_seek_points);

   // Update our lookup table file with the new seek points and pcm_frame_count.
   // Note: the serializer elegantly handles C++ STL objects and is endian-safe.
   seek_points_table[stream_hash] = seek_points_vector;
   pcm_frame_count_table[stream_hash] = pcm_frame_count;
   ofstream outfile(filename, ios_base::trunc | ios_base::binary);
   Archive<ofstream> serialize(outfile);
   serialize << SEEK_TABLE_IDENTIFIER << seek_points_table << pcm_frame_count_table;
   outfile.close();

   return pcm_frame_count;
}

const Uint64 load_existing_seek_points(const char* filename,
                                       const Uint64& stream_hash,
                                       map<Uint64, vector<drmp3_seek_point_serial> >& seek_points_table,
                                       map<Uint64, drmp3_uint64>& pcm_frame_count_table,
                                       vector<drmp3_seek_point_serial>& seek_points) {
    // does the file exist? bail out if not.
    struct stat buffer;
    if (stat(filename, &buffer) != 0) {
        return 0;
    }

    // is the file large enough to atleast hold the Archive object plus our identifier?
    const string expected_identifier(SEEK_TABLE_IDENTIFIER);
    if (get_file_size(filename) < 4 + expected_identifier.length()) {
        return 0;
    }

    // De-serialize the identifier first
    string fetched_identifier;
    ifstream infile(filename, ios_base::binary);
    Archive<ifstream> deserialize(infile);
    deserialize >> fetched_identifier;
    if (fetched_identifier != expected_identifier) {
        infile.close();
        return 0;
    }

    // De-serialize the seek point and pcm_count tables.
    deserialize >> seek_points_table >> pcm_frame_count_table;
    infile.close();

    // does the seek_points table have our stream's hash?
    const auto p_seek_points = seek_points_table.find(stream_hash);
    if (p_seek_points == seek_points_table.end()) {
        return 0;
    }

    // does the pcm_frame_count table have our stream's hash?
    const auto p_pcm_frame_count = pcm_frame_count_table.find(stream_hash);
    if (p_pcm_frame_count == pcm_frame_count_table.end()) {
        return 0;
    }
    seek_points = p_seek_points->second;
    return p_pcm_frame_count->second;
}

const Uint64 populate_seek_points(struct SDL_RWops* const context, mp3_t* p_mp3, const char* seektable_filename) {

    // get the byte-length of the binary stream

    // calculate the stream's xxHash value
    Uint64 stream_hash = calculate_stream_hash(context);
    if (stream_hash == 0) {
        LOG_MSG("MP3: could not compute the hash of the stream");
        return 0;
    }

    // attempt to fetch the seek points and pcm count from an existing look up table file
    map<Uint64, vector<drmp3_seek_point_serial> > seek_points_table;
    map<Uint64, drmp3_uint64> pcm_frame_count_table;
    drmp3_uint64 pcm_frame_count = load_existing_seek_points(seektable_filename,
                                           stream_hash,
                                           seek_points_table,
                                           pcm_frame_count_table,
                                           p_mp3->seek_points_vector);

    // otherwise use dr_mp3 to calculate new seek points and get pcm frame count
    if (pcm_frame_count == 0) {
        pcm_frame_count = generate_new_seek_points(seektable_filename,
                                  stream_hash,
                                  p_mp3->p_dr,
                                  seek_points_table,
                                  pcm_frame_count_table,
                                  p_mp3->seek_points_vector);
        if (pcm_frame_count == 0) {
            LOG_MSG("MP3: could not load existing or generate new seek points for the stream");
            return 0;
        }
    }

    // finally, regardless of which scenario succeeded above, we bind our seek points to the dr_mp3 object
    drmp3_bool8 result = drmp3_bind_seek_table(p_mp3->p_dr,
                                 p_mp3->seek_points_vector.size(),
                                 reinterpret_cast<drmp3_seek_point*>(p_mp3->seek_points_vector.data()));
    if (result != DRMP3_TRUE) {
        LOG_MSG("MP3: could not bind the seek points to the dr_mp3 object");
        return 0;
    }
    return pcm_frame_count;
}
