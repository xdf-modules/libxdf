//libxdf is a static C++ library to load XDF files
//Copyright (C) 2017  Yida Lin

//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.
//If you have questions, contact author at yida.lin@outlook.com


#include "xdf.h"

#include <iostream>
#include <fstream>
#include <pugixml.hpp>  //pugi XML parser
#include <sstream>
#include <algorithm>
#include "smarc/smarc.h"      //resampling library
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */
#include <numeric>      //std::accumulate
#include <functional>   // bind2nd
#include <cmath>

Xdf::Xdf() {

}

void Xdf::adjust_total_length() {
    for (auto const &stream : streams_) {
        if (!stream.time_series.empty()) {
            if (total_len_ < stream.time_series.front().size()) {
                total_len_ = stream.time_series.front().size();
            }
        }
    }
}

void Xdf::calculate_total_length(int sampling_rate) {
    total_len_ = (max_timestamp_ - min_timestamp_) * sampling_rate;
}

void Xdf::create_labels() {
    size_t channel_count = 0;

    for (size_t st = 0; st < streams_.size(); st++) {
        if (streams_[st].info.channels.size()) {
            for (size_t ch = 0; ch < streams_[st].info.channels.size(); ch++) {
                // +1 for 1 based numbers; for user convenience only. The internal computation is still 0 based
                std::string label = "Stream " + std::to_string(st + 1) + " - Channel " + std::to_string(ch + 1)
                        + " - " + std::to_string((int)streams_[st].info.nominal_srate) + " Hz\n";

                label += streams_[st].info.name + '\n';

                for (auto const &entry : streams_[st].info.channels[ch]) {
                    if (entry.second != "") {
                        label += entry.first + " : " + entry.second + '\n';
                    }
                }
                if (offsets_.size()) {
                    if (offsets_[channel_count] >= 0) {
                        label.append("baseline +").append(std::to_string(offsets_[channel_count]));
                    }
                    else {
                        label.append("baseline ").append(std::to_string(offsets_[channel_count]));
                    }
                }
                labels_.emplace_back(label);

                channel_count++;
            }
        }
        else {
            for (size_t ch = 0; ch < streams_[st].time_series.size(); ch++) {
                // +1 for 1 based numbers; for user convenience only. The internal computation is still 0 based
                std::string label = "Stream " + std::to_string(st + 1) +
                        " - Channel " + std::to_string(ch + 1) + " - " +
                        std::to_string((int)streams_[st].info.nominal_srate) +
                        " Hz\n" + streams_[st].info.name + '\n' + streams_[st].info.type + '\n';

                label += streams_[st].info.name + '\n';

                if (offsets_.size()) {
                    if (offsets_[channel_count] >= 0) {
                        label.append("baseline +").append(std::to_string(offsets_[channel_count]));
                    }
                    else {
                        label.append("baseline ").append(std::to_string(offsets_[channel_count]));
                    }
                }

                labels_.emplace_back(label);

                channel_count++;
            }
        }
    }
}

void Xdf::detrend() {
    for (auto &stream : streams_) {
        for (auto &row : stream.time_series) {
            long double init = 0.0;
            long double mean = std::accumulate(row.begin(), row.end(), init) / row.size();
            for(auto &val: row) val -= mean;
            offsets_.emplace_back(mean);
        }
    }
}

void Xdf::free_up_timestamps() {
    //free up as much memory as possible
    for (auto &stream : streams_) {
        //we don't need to keep all the time stamps unless it's a stream with irregular samples
        //filter irregular streams and string streams
        if (stream.info.nominal_srate != 0 && !stream.timestamps.empty() && stream.info.channel_format.compare("string")) {
            std::vector<double> nothing;
            //however we still need to keep the first time stamp of each stream to decide at which position the signal should start
            nothing.emplace_back(stream.timestamps.front());
            stream.timestamps.swap(nothing);
        }
    }
}

int Xdf::load_xdf(std::string filename) {
    clock_t time;
    time = clock();


    /*	//uncompress if necessary
    char ext[_MAX_EXT]; //for file extension

    _splitpath_s ( argv[1], NULL, NULL, NULL, NULL, NULL, NULL, ext, NULL );
    if (strcmp(ext, ".xdfz") == 0)
    {
    //uncompress
    }
    */

    std::vector<int> idmap; //remaps stream id's onto indices in streams


    //===================================================================
    //========================= parse the file ==========================
    //===================================================================


    std::ifstream file(filename, std::ios::in | std::ios::binary);

    if (file.is_open()) {
        // read [MagicCode]
        std::string magic_number;
        for (char c; file >> c;) {
            magic_number.push_back(c);
            if (magic_number.size() == 4) break;
        }

        if (magic_number.compare("XDF:")) {
            std::cout << "This is not a valid XDF file.('" << filename << "')\n";
            return -1;
        }

        //for each chunk
        while (1) {
            uint64_t channel_length = read_length(file);//chunk length

            if (channel_length == 0) break;

            uint16_t tag;   //read tag of the chunk, 6 possibilities
            read_bin(file, &tag);

            switch (tag) {
            case 1: {
                // [FileHeader]
                char* buffer = new char[channel_length - 2];
                file.read(buffer, channel_length - 2);
                file_header_ = buffer;

                pugi::xml_document doc;

                doc.load_buffer_inplace(buffer, channel_length - 2);

                pugi::xml_node info = doc.child("info");

                version_ = info.child("version").text().as_float();

                delete[] buffer;
            }
                break;
            case 2: {
                // [StreamHeader] chunk
                uint32_t stream_id;
                int index;
                Xdf::read_bin(file, &stream_id);
                std::vector<int>::iterator it{std::find(idmap.begin(),idmap.end(),stream_id)};
                if (it == idmap.end()) {
                    index = idmap.size();
                    idmap.emplace_back(stream_id);
                    streams_.emplace_back();
                }
                else {
                    index = std::distance(idmap.begin(), it);
                }

                pugi::xml_document doc;

                // read [Content]
                char* buffer = new char[channel_length - 6];
                file.read(buffer, channel_length - 6);
                streams_[index].stream_header = buffer;

                doc.load_buffer_inplace(buffer, channel_length - 6);

                pugi::xml_node info = doc.child("info");
                pugi::xml_node desc = info.child("desc");

                streams_[index].info.channel_count = info.child("channel_count").text().as_int();
                streams_[index].info.nominal_srate = info.child("nominal_srate").text().as_double();
                streams_[index].info.name = info.child("name").text().get();
                streams_[index].info.type = info.child("type").text().get();
                streams_[index].info.channel_format = info.child("channel_format").text().get();

                for (auto channel = desc.child("channels").child("channel"); channel; channel = channel.next_sibling("channel")) {
                    streams_[index].info.channels.emplace_back();

                    for (auto const &entry : channel.children()) {
                        streams_[index].info.channels.back().emplace(entry.name(), entry.child_value());
                    }
                }

                if (streams_[index].info.nominal_srate > 0) {
                    streams_[index].sampling_interval = 1 / streams_[index].info.nominal_srate;
                }
                else {
                    streams_[index].sampling_interval = 0;
                }

                delete[] buffer;
            }
                break;
            case 3: {
                // [Samples] chunk
                uint32_t stream_id;
                int index;
                Xdf::read_bin(file, &stream_id);
                std::vector<int>::iterator it {std::find(idmap.begin(),idmap.end(),stream_id)};
                if (it == idmap.end()) {
                    index = idmap.size();
                    idmap.emplace_back(stream_id);
                    streams_.emplace_back();
                }
                else {
                    index = std::distance(idmap.begin(), it);
                }

                //read [NumSampleBytes], [NumSamples]
                uint64_t num_samples = read_length(file);

                //check the data type
                if (streams_[index].info.channel_format.compare("float32") == 0) {
                    //if the time series is empty
                    if (streams_[index].time_series.empty()) {
                        streams_[index].time_series.resize(streams_[index].info.channel_count);
                    }

                    //for each sample
                    for (size_t i = 0; i < num_samples; i++) {
                        //read or deduce time stamp
                        auto timestamp_bytes = read_bin<uint8_t>(file);

                        double timestamp;  //temporary time stamp

                        if (timestamp_bytes == 8) {
                            Xdf::read_bin(file, &timestamp);
                            streams_[index].timestamps.emplace_back(timestamp);
                        }
                        else {
                            timestamp = streams_[index].last_timestamp + streams_[index].sampling_interval;
                            streams_[index].timestamps.emplace_back(timestamp);
                        }

                        streams_[index].last_timestamp = timestamp;

                        //read the data
                        for (int v = 0; v < streams_[index].info.channel_count; ++v) {
                            float data;
                            Xdf::read_bin(file, &data);
                            streams_[index].time_series[v].emplace_back(data);
                        }
                    }
                }
                else if (streams_[index].info.channel_format.compare("double64") == 0) {
                    //if the time series is empty
                    if (streams_[index].time_series.empty()) {
                        streams_[index].time_series.resize(streams_[index].info.channel_count);
                    }

                    //for each sample
                    for (size_t i = 0; i < num_samples; i++) {
                        //read or deduce time stamp
                        auto timestamp_bytes = read_bin<uint8_t>(file);

                        double timestamp;  //temporary time stamp

                        if (timestamp_bytes == 8) {
                            Xdf::read_bin(file, &timestamp);
                            streams_[index].timestamps.emplace_back(timestamp);
                        }
                        else {
                            timestamp = streams_[index].last_timestamp + streams_[index].sampling_interval;
                            streams_[index].timestamps.emplace_back(timestamp);
                        }

                        streams_[index].last_timestamp = timestamp;

                        //read the data
                        for (int v = 0; v < streams_[index].info.channel_count; ++v) {
                            double data;
                            Xdf::read_bin(file, &data);
                            streams_[index].time_series[v].emplace_back(data);
                        }
                    }
                }
                else if (streams_[index].info.channel_format.compare("int8") == 0) {
                    //if the time series is empty
                    if (streams_[index].time_series.empty()) {
                        streams_[index].time_series.resize(streams_[index].info.channel_count);
                    }

                    //for each sample
                    for (size_t i = 0; i < num_samples; i++) {
                        //read or deduce time stamp
                        auto timestamp_bytes = read_bin<uint8_t>(file);

                        double timestamp;  //temporary time stamp

                        if (timestamp_bytes == 8) {
                            Xdf::read_bin(file, &timestamp);
                            streams_[index].timestamps.emplace_back(timestamp);
                        }
                        else {
                            timestamp = streams_[index].last_timestamp + streams_[index].sampling_interval;
                            streams_[index].timestamps.emplace_back(timestamp);
                        }

                        streams_[index].last_timestamp = timestamp;

                        //read the data
                        for (int v = 0; v < streams_[index].info.channel_count; ++v) {
                            int8_t data;
                            Xdf::read_bin(file, &data);
                            streams_[index].time_series[v].emplace_back(data);
                        }
                    }
                }
                else if (streams_[index].info.channel_format.compare("int16") == 0) {
                    //if the time series is empty
                    if (streams_[index].time_series.empty()) {
                        streams_[index].time_series.resize(streams_[index].info.channel_count);
                    }

                    //for each sample
                    for (size_t i = 0; i < num_samples; i++) {
                        //read or deduce time stamp
                        auto timestamp_bytes = read_bin<uint8_t>(file);

                        double timestamp;  //temporary time stamp

                        if (timestamp_bytes == 8) {
                            Xdf::read_bin(file, &timestamp);
                            streams_[index].timestamps.emplace_back(timestamp);
                        }
                        else {
                            timestamp = streams_[index].last_timestamp + streams_[index].sampling_interval;
                            streams_[index].timestamps.emplace_back(timestamp);
                        }

                        streams_[index].last_timestamp = timestamp;

                        //read the data
                        for (int v = 0; v < streams_[index].info.channel_count; ++v) {
                            int16_t data;
                            Xdf::read_bin(file, &data);
                            streams_[index].time_series[v].emplace_back(data);
                        }
                    }
                }
                else if (streams_[index].info.channel_format.compare("int32") == 0) {
                    //if the time series is empty
                    if (streams_[index].time_series.empty()) {
                        streams_[index].time_series.resize(streams_[index].info.channel_count);
                    }

                    //for each sample
                    for (size_t i = 0; i < num_samples; i++) {
                        //read or deduce time stamp
                        auto timestamp_bytes = read_bin<uint8_t>(file);

                        double timestamp;  //temporary time stamp

                        if (timestamp_bytes == 8) {
                            Xdf::read_bin(file, &timestamp);
                            streams_[index].timestamps.emplace_back(timestamp);
                        }
                        else {
                            timestamp = streams_[index].last_timestamp + streams_[index].sampling_interval;
                            streams_[index].timestamps.emplace_back(timestamp);
                        }

                        streams_[index].last_timestamp = timestamp;

                        //read the data
                        for (int v = 0; v < streams_[index].info.channel_count; ++v) {
                            int32_t data;
                            Xdf::read_bin(file, &data);
                            streams_[index].time_series[v].emplace_back(data);
                        }
                    }
                }
                else if (streams_[index].info.channel_format.compare("int64") == 0) {
                    //if the time series is empty
                    if (streams_[index].time_series.empty()) {
                        streams_[index].time_series.resize(streams_[index].info.channel_count);
                    }

                    //for each sample
                    for (size_t i = 0; i < num_samples; i++) {
                        //read or deduce time stamp
                        auto timestamp_bytes = read_bin<uint8_t>(file);

                        double timestamp;  //temporary time stamp

                        if (timestamp_bytes == 8) {
                            Xdf::read_bin(file, &timestamp);
                            streams_[index].timestamps.emplace_back(timestamp);
                        }
                        else {
                            timestamp = streams_[index].last_timestamp + streams_[index].sampling_interval;
                            streams_[index].timestamps.emplace_back(timestamp);
                        }

                        streams_[index].last_timestamp = timestamp;

                        //read the data
                        for (int v = 0; v < streams_[index].info.channel_count; ++v) {
                            int64_t data;
                            Xdf::read_bin(file, &data);
                            streams_[index].time_series[v].emplace_back(data);
                        }
                    }
                }
                else if (streams_[index].info.channel_format.compare("string") == 0) {
                    //for each event
                    for (size_t i = 0; i < num_samples; i++) {
                        //read or deduce time stamp
                        auto timestamp_bytes = read_bin<uint8_t>(file);

                        double timestamp;  //temporary time stamp

                        if (timestamp_bytes == 8) {
                            Xdf::read_bin(file, &timestamp);
                        }
                        else {
                            timestamp = streams_[index].last_timestamp + streams_[index].sampling_interval;
                        }

                        //read the event
                        auto length = Xdf::read_length(file);

                        char* buffer = new char[length + 1];
                        file.read(buffer, length);
                        buffer[length] = '\0';
                        event_map_.emplace_back(std::make_pair(buffer, timestamp), index);
                        delete[] buffer;
                        streams_[index].last_timestamp = timestamp;
                    }
                }
            }
                break;
            case 4: {
                // [ClockOffset] chunk
                uint32_t streamID;
                int index;
                Xdf::read_bin(file, &streamID);
                std::vector<int>::iterator it {std::find(idmap.begin(),idmap.end(),streamID)};
                if (it == idmap.end()) {
                    index = idmap.size();
                    idmap.emplace_back(streamID);
                    streams_.emplace_back();
                }
                else {
                    index = std::distance(idmap.begin(), it);
                }

                double collection_time;
                double offset_value;

                Xdf::read_bin(file, &collection_time);
                Xdf::read_bin(file, &offset_value);

                streams_[index].clock_times.emplace_back(collection_time);
                streams_[index].clock_values.emplace_back(offset_value);
            }
                break;
            case 6: {
                // [StreamFooter] chunk
                pugi::xml_document doc;

                uint32_t stream_id;
                int index;
                Xdf::read_bin(file, &stream_id);
                std::vector<int>::iterator it {std::find(idmap.begin(),idmap.end(),stream_id)};
                if (it == idmap.end()) {
                    index = idmap.size();
                    idmap.emplace_back(stream_id);
                    streams_.emplace_back();
                }
                else {
                    index = std::distance(idmap.begin(), it);
                }

                char* buffer = new char[channel_length - 6];
                file.read(buffer, channel_length - 6);
                streams_[index].stream_footer = buffer;

                doc.load_buffer_inplace(buffer, channel_length - 6);

                pugi::xml_node info = doc.child("info");

                streams_[index].info.first_timestamp = info.child("first_timestamp").text().as_double();
                streams_[index].info.last_timestamp = info.child("last_timestamp").text().as_double();
                streams_[index].info.measured_sampling_rate = info.child("measured_srate").text().as_double();
                streams_[index].info.sample_count = info.child("sample_count").text().as_int();
                delete[] buffer;
            }
                break;
            case 5:
                //skip other chunk types (Boundary, ...)
                file.seekg(channel_length - 2, file.cur);
                break;
            default:
                std::cout << "Unknown chunk encountered.\n";
                break;
            }
        }

        //calculate how much time it takes to read the data
        clock_t halfWay = clock() - time;

        std::cout << "it took " << halfWay << " clicks (" << ((float)halfWay) / CLOCKS_PER_SEC << " seconds)"
                  << " reading XDF data" << std::endl;

        sync_timestamps();

        find_min_max_time_stamps();

        find_major_sampling_rate();

        find_max_sampling_rate();

        load_sampling_rate_map();

        calculate_channel_count();

        load_dictionary();

        calculate_effective_sampling_rate();

        //loading finishes, close file
        file.close();
    }
    else {
        std::cout << "Unable to open file" << std::endl;
        return 1;
    }

    return 0;
}

void Xdf::resample(int sampling_rate) {
    //if user entered a preferred sample rate, we resample all the channels to that sample rate
    //Otherwise, we resample all channels to the sample rate that has the most channels

    clock_t time = clock();

#define BUF_SIZE 8192
    for (auto &stream : streams_) {
        if (!stream.time_series.empty() && stream.info.nominal_srate != sampling_rate &&
                stream.info.nominal_srate != 0) {
            int fsin = stream.info.nominal_srate;       // input samplerate
            int fsout = sampling_rate;                      // output samplerate
            double bandwidth = 0.95;                    // bandwidth
            double rp = 0.1;                            // passband ripple factor
            double rs = 140;                            // stopband attenuation
            double tol = 0.000001;                      // tolerance

            // initialize smarc filter
            struct PFilter* pfilt = smarc_init_pfilter(fsin, fsout, bandwidth, rp,
                                                       rs, tol, NULL, 0);
            if (pfilt == NULL) continue;

            // initialize smarc filter state
            struct PState* pstate = smarc_init_pstate(pfilt);

            for (auto &row : stream.time_series) {
                // initialize buffers
                int read = 0;
                int written = 0;
                const int OUT_BUF_SIZE = (int) smarc_get_output_buffer_size(pfilt, row.size());
                double* inbuf = new double[row.size()];
                double* outbuf = new double[OUT_BUF_SIZE];

                std::copy(row.begin(), row.end(), inbuf);

                read = row.size();

                // resample signal block
                written = smarc_resample(pfilt, pstate, inbuf, read, outbuf, OUT_BUF_SIZE);

                // do what you want with your output
                row.resize(written);
                std::copy ( outbuf, outbuf+written, row.begin() );

                // flushing last values
                written = smarc_resample_flush(pfilt, pstate, outbuf, OUT_BUF_SIZE);

                // do what you want with your output
                row.resize(row.size() + written);
                std::copy ( outbuf, outbuf+written, row.begin() + row.size() - written );

                // you are done with converting your signal.
                // If you want to reuse the same converter to process another signal
                // just reset the state:
                smarc_reset_pstate(pstate,pfilt);

                delete[] inbuf;
                delete[] outbuf;
            }
            // release smarc filter state
            smarc_destroy_pstate(pstate);

            // release smarc filter
            smarc_destroy_pfilter(pfilt);
        }
    }
    //resampling finishes here

    //======================================================================
    //===========Calculating total length & total channel count=============
    //======================================================================

    calculate_total_length(sampling_rate);

    adjust_total_length();

    time = clock() - time;

    std::cout << "it took " << time << " clicks (" << ((float)time) / CLOCKS_PER_SEC << " seconds)"
              << " resampling" << std::endl;
}

void Xdf::sync_timestamps() {
    // Sync time stamps
    for (auto &stream : this->streams_) {
        if (!stream.clock_times.empty()) {
            size_t m = 0;   // index iterating through stream.time_stamps
            size_t n = 0;   // index iterating through stream.clock_times

            while (m < stream.timestamps.size()) {
                if (stream.clock_times[n] < stream.timestamps[m]) {
                    while (n < stream.clock_times.size() - 1 && stream.clock_times[n+1] < stream.timestamps[m]) {
                        n++;
                    }
                    stream.timestamps[m] += stream.clock_values[n];
                }
                else if (n == 0) {
                    stream.timestamps[m] += stream.clock_values[n];
                }
                m++;
            }
        }
    }

    // Sync event time stamps
    for (auto &elem : this->event_map_) {
        if (!this->streams_[elem.second].clock_times.empty()) {
            size_t k = 0;   // index iterating through streams[elem.second].clock_times

            while (k < this->streams_[elem.second].clock_times.size() - 1) {
                if (this->streams_[elem.second].clock_times[k+1] < elem.first.second) {
                    k++;
                }
                else {
                    break;
                }
            }

            elem.first.second += this->streams_[elem.second].clock_values[k]; // apply the last offset value to the timestamp; if there hasn't yet been an offset value take the first recorded one
        }
    }

    // Update first and last time stamps in stream footer
    for (size_t k = 0; k < this->streams_.size(); k++) {
        if (streams_[k].info.channel_format.compare("string") == 0) {
            double min = NAN;
            double max = NAN;

            for (auto const &elem : this->event_map_) {
                if (elem.second == (int)k) {
                    if (std::isnan(min) || elem.first.second < min) {
                        min = elem.first.second;
                    }

                    if (std::isnan(max) || elem.first.second > max) {
                        max = elem.first.second;
                    }
                }
            }

            streams_[k].info.first_timestamp = min;
            streams_[k].info.last_timestamp = max;
        }
        else {
            streams_[k].info.first_timestamp = streams_[k].timestamps.front();
            streams_[k].info.last_timestamp = streams_[k].timestamps.back();
        }
    }
}

int Xdf::write_events_to_xdf(std::string file_path) {
    if (user_added_stream_) {
        std::fstream file;
        file.open(file_path, std::ios::app | std::ios::binary);

        if (file.is_open()) {
            //start to append to new XDF file
            //first write a stream header chunk
            //Num Length Bytes
            file.put(4);
            //length
            int length = streams_[user_added_stream_].stream_header.size() + 6; //+6 because of the length int itself and short int tag
            file.write((char*)&length, 4);

            //tag
            short tag = 2;
            file.write((char*)&tag, 2);
            //streamNumber
            int stream_number = user_added_stream_ + 1; //+1 because the stream IDs in XDF are 1 based instead of 0 based
            file.write((char*)&stream_number, 4);
            //content
            file.write(streams_[user_added_stream_].stream_header.c_str(), length - 6);//length - 6 is the string length

            //write samples chunk
            //Num Length Bytes
            file.put(8);
            //length
            //add the bytes of all following actions together
            int64_t stringTotalLength = 0;
            for (auto const &event : user_created_events_) {
                stringTotalLength += event.first.size();
            }

            int64_t sample_chunk_length = 2 + 4 + 1 + 4 +
                    user_created_events_.size() *
                    (1 + 8 + 1 + 4) + stringTotalLength;
            file.write((char*)&sample_chunk_length, 8);

            //tag
            tag = 3;
            file.write((char*)&tag, 2);
            //streamNumber
            file.write((char*)&stream_number, 4);
            //content
            //NumSamplesBytes
            file.put(4);

            //Num samples
            int sample_count = user_created_events_.size();
            file.write((char*)&sample_count, 4);

            //samples
            for (auto const &event : user_created_events_) {
                //TimeStampBytes
                file.put(8);

                //Optional Time Stamp
                double timeStamp = event.second;
                file.write((char*)&timeStamp, 8);

                //Num Length Bytes
                file.put(4);

                //Length
                int string_length = event.first.length();
                file.write((char*)&string_length, 4);

                //String Content
                file.write(event.first.c_str(), string_length);
            }

            file.close();
        }
        else {
            std::cerr << "Unable to open file." << std::endl;
            return -1; //Error
        }
    }

    std::cout << "Succesfully wrote to XDF file." << std::endl;

    return 0; //Success
}

void Xdf::calculate_effective_sampling_rate() {
    for (auto &stream : streams_) {
        if (stream.info.nominal_srate) {
            try {
                stream.info.effective_sampling_rate
                        = stream.info.sample_count /
                        (stream.info.last_timestamp - stream.info.first_timestamp);

                if (stream.info.effective_sampling_rate) {
                    effective_sampling_rates_.emplace_back(stream.info.effective_sampling_rate);
                }

                pugi::xml_document doc;
                doc.load_string(stream.stream_footer.c_str());
                pugi::xml_node sampleCount = doc.child("info").child("sample_count");
                pugi::xml_node effectiveSampleRate
                        = doc.child("info").insert_child_after("effective_sample_rate", sampleCount);
                effectiveSampleRate.append_child(pugi::node_pcdata)
                        .set_value(std::to_string(stream.info.effective_sampling_rate).c_str());

                std::stringstream buffer;
                doc.save(buffer);

                stream.stream_footer = buffer.str();
            }
            catch (std::exception &e) {
                std::cerr << "Error calculating effective sample rate. "
                          << e.what() << std::endl;
            }
        }
    }
}

void Xdf::calculate_channel_count() {
    //calculating total channel count, and indexing them onto streamMap
    for (size_t c = 0; c < streams_.size(); c++) {
        if(!streams_[c].time_series.empty()) {
            channel_count_ += streams_[c].info.channel_count;

            for (int i = 0; i < streams_[c].info.channel_count; i++)
                stream_map_.emplace_back(c);
        }
    }
}

void Xdf::find_major_sampling_rate() {
    // find out which sample rate has the most channels
    typedef int SamplingRate;
    typedef int ChannelCount;

    std::vector<std::pair<SamplingRate, ChannelCount> > sampling_rate_map;

    //find out whether a sample rate already exists in srateMap
    for (auto const &stream : streams_) {
        if (stream.info.nominal_srate != 0) {
            std::vector<std::pair<SamplingRate, ChannelCount> >::iterator it {std::find_if(sampling_rate_map.begin(), sampling_rate_map.end(),
                                                                                           [&](const std::pair<SamplingRate, ChannelCount> &element)
                {return element.first == stream.info.nominal_srate; })} ;
            //if it doesn't, add it here
            if (it == sampling_rate_map.end()) {
                sampling_rate_map.emplace_back(stream.info.nominal_srate, stream.info.channel_count);
                //if it already exists, add additional channel numbers to that sample rate
            }
            else {
                int index (std::distance(sampling_rate_map.begin(),it)) ;
                sampling_rate_map[index].second += stream.info.channel_count;
            }
        }
    }

    if (sampling_rate_map.size() > 0) {
        //search the srateMap to see which sample rate has the most channels
        int index (std::distance(sampling_rate_map.begin(),
                                 std::max_element(sampling_rate_map.begin(),sampling_rate_map.end(),
                                                  [] (const std::pair<SamplingRate, ChannelCount> &largest,
                                                  const std::pair<SamplingRate, ChannelCount> &first)
        { return largest.second < first.second; })));

        major_sampling_rate_ = sampling_rate_map[index].first; //the sample rate that has the most channels
    } else {
        major_sampling_rate_ = 0; //if there are no streams with a fixed sample reate
    }
}

void Xdf::find_max_sampling_rate() {
    for (auto const &stream : streams_) {
        if (stream.info.nominal_srate > max_sampling_rate_) {
            max_sampling_rate_ = stream.info.nominal_srate;
        }
    }
}

void Xdf::find_min_max_time_stamps() {
    //find the smallest timestamp of all streams
    for (auto const &stream : streams_) {
        if (!std::isnan(stream.info.first_timestamp)) {
            min_timestamp_ = stream.info.first_timestamp;
            break;
        }
    }
    for (auto const &stream : streams_) {
        if (!std::isnan(stream.info.first_timestamp) && stream.info.first_timestamp < min_timestamp_) {
            min_timestamp_ = stream.info.first_timestamp;
        }
    }

    //find the max timestamp of all streams
    for (auto const &stream : streams_) {
        if (!std::isnan(stream.info.last_timestamp) && stream.info.last_timestamp > max_timestamp_) {
            max_timestamp_ = stream.info.last_timestamp;
        }
    }
}

void Xdf::load_dictionary() {
    //loop through `event_map_`
    for (auto const &entry : event_map_) {
        //search the dictionary to see whether an event is already in it
        auto it = std::find(dictionary_.begin(),dictionary_.end(),entry.first.first);
        //if it isn't yet
        if (it == dictionary_.end()) {
            //add it to the dictionary, also store its index into eventType vector for future use
            event_type_.emplace_back(dictionary_.size());
            dictionary_.emplace_back(entry.first.first);
        }
        //if it's already in there
        else {
            //store its index into eventType vector
            event_type_.emplace_back(std::distance(dictionary_.begin(), it));
        }
    }
}

void Xdf::load_sampling_rate_map() {
    for (auto const &stream : streams_) {
        sampling_rate_map_.emplace(stream.info.nominal_srate);
    }
}

template<typename T>
T Xdf::read_bin(std::istream& is, T* obj) {
    T dummy;
    if(!obj) obj = &dummy;
    is.read(reinterpret_cast<char*>(obj), sizeof(T));
    return *obj;
}

//function of reading the length of each chunk
uint64_t Xdf::read_length(std::ifstream &file) {
    uint8_t bytes;
    Xdf::read_bin(file, &bytes);
    uint64_t length = 0;

    switch (bytes) {
    case 1:
        length = read_bin<uint8_t>(file);
        break;
    case 4:
        length = read_bin<uint32_t>(file);
        break;
    case 8:
        length = read_bin<uint64_t>(file);
        break;
    default:
        std::cout << "Invalid variable-length integer length ("
                  << static_cast<int>(bytes) << ") encountered.\n";
        return 0;
    }

    return length;
}
