#include "xdf.h"

#include <iostream>
#include <fstream>
#include <stdlib.h>     //EXIT_FAILURE, NULL, abs
#include "pugixml.hpp"  //pugi XML parser
#include <sstream>
#include <algorithm>
#include "smarc.h"      //resampling library
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */


Xdf::Xdf()
{
}

void Xdf::load_xdf(std::string filename)
{
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

    if (file.is_open())
    {
        //read [MagicCode]
        std::string magicNumber;
        for (char c; file >> c;)
        {
            magicNumber.push_back(c);
            if (magicNumber.size() == 4)
                break;
        }

        if (magicNumber.compare("XDF:") != 0)
        {
            std::cout << "This is not a valid XDF file.('" << filename << "')\n";
            exit(EXIT_FAILURE);
        }

        //for each chunk
        while (1)
        {
            uint64_t ChLen; //Chunk Length

            ChLen = readLength(file);
            if (ChLen == 0)
                break;

            uint16_t tag;   //read tag of the chunk, 6 possibilities
            file.read(reinterpret_cast<char *>(&tag), 2);

            switch (tag)
            {
            case 1: //[FileHeader]
            {
                char* buffer = new char[ChLen - 2];
                file.read(buffer, ChLen - 2);

                pugi::xml_document doc;

                pugi::xml_parse_result result = doc.load_buffer_inplace(buffer, ChLen - 2);

                pugi::xml_node info = doc.child("info");

                fileHeader.info.version = info.child("version").text().as_float();

                delete[] buffer;
            }
            break;
            case 2: //read [StreamHeader] chunk
            {
                //read [StreamID]
                int streamID;
                int index;
                file.read((char*)&streamID, 4);
                std::vector<int>::iterator it {std::find(idmap.begin(),idmap.end(),streamID)};
                if (it == idmap.end())
                {
                    index = idmap.size();
                    idmap.emplace_back(streamID);
                    streams.emplace_back();
                }
                else
                    index = std::distance(idmap.begin(), it);


                pugi::xml_document doc;

                //read [Content]
                char* buffer = new char[ChLen - 6];
                file.read(buffer, ChLen - 6);
                pugi::xml_parse_result result = doc.load_buffer_inplace(buffer, ChLen - 6);

                pugi::xml_node info = doc.child("info");

                streams[index].info.name = info.child("name").text().get();
                streams[index].info.type = info.child("type").text().get();
                streams[index].info.channel_count = info.child("channel_count").text().as_int();
                streams[index].info.nominal_srate = info.child("nominal_srate").text().as_double();
                streams[index].info.channel_format = info.child("channel_format").text().get();
                streams[index].info.source_id = info.child("source_id").text().get();
                streams[index].info.version = info.child("version").text().as_float();
                streams[index].info.created_at = info.child("created_at").text().as_double();
                streams[index].info.uid = info.child("uid").text().get();
                streams[index].info.session_id = info.child("session_id").text().get();
                streams[index].info.hostname = info.child("hostname").text().get();
                streams[index].info.v4address = info.child("v4address").text().get();
                streams[index].info.v4data_port = info.child("v4data_port").text().as_int();
                streams[index].info.v4service_port = info.child("v4service_port").text().as_int();
                streams[index].info.v6address = info.child("v6address").text().get();
                streams[index].info.v6data_port = info.child("v6data_port").text().as_int();
                streams[index].info.v6service_port = info.child("v6service_port").text().as_int();
                //streams[index].info.desc = info.child("desc").text().get();//need further work

                if (streams[index].info.nominal_srate > 0)
                    streams[index].sampling_interval = 1 / streams[index].info.nominal_srate;
                else
                    streams[index].sampling_interval = 0;

                delete[] buffer;
            }
            break;
            case 3: //read [Samples] chunk
            {
                //read [StreamID]
                int streamID;
                int index;
                file.read((char*)&streamID, 4);
                std::vector<int>::iterator it {std::find(idmap.begin(),idmap.end(),streamID)};
                if (it == idmap.end())
                {
                    index = idmap.size();
                    idmap.emplace_back(streamID);
                    streams.emplace_back();
                }
                else
                    index = std::distance(idmap.begin(), it);


                //read [NumSampleBytes], [NumSamples]
                uint64_t numSamp = readLength(file);

                //check the data type
                if (streams[index].info.channel_format.compare("float32") == 0)
                {
                    //if the time series is empty
                    if (streams[index].time_series.empty())
                        streams[index].time_series.resize(streams[index].info.channel_count);

                    //for each sample
                    for (size_t i = 0; i < numSamp; i++)
                    {
                        //read or deduce time stamp
                        uint8_t tsBytes;
                        file.read(reinterpret_cast<char *>(&tsBytes), 1);
                        if (tsBytes == 8)
                        {
                            double ts;
                            file.read((char*)&ts, 8);
                            streams[index].time_stamps.emplace_back(ts);
                        }
                        else
                        {
                            double ts = streams[index].last_timestamp + streams[index].sampling_interval;
                            streams[index].time_stamps.emplace_back(ts);
                        }

                        //read the data
                        for (int v = 0; v < streams[index].info.channel_count; ++v)
                        {
                            float data;
                            file.read((char*)&data, 4);
                            streams[index].time_series[v].emplace_back(data);
                        }
                    }
                }
                else if (streams[index].info.channel_format.compare("double64") == 0)
                {
                    //if the time series is empty
                    if (streams[index].time_series.empty())
                        streams[index].time_series.resize(streams[index].info.channel_count);

                    //for each sample
                    for (size_t i = 0; i < numSamp; i++)
                    {
                        //read or deduce time stamp
                        uint8_t tsBytes;
                        file.read(reinterpret_cast<char *>(&tsBytes), 1);
                        if (tsBytes == 8)
                        {
                            double ts;
                            file.read((char*)&ts, 8);
                            streams[index].time_stamps.emplace_back(ts);
                        }
                        else
                        {
                            double ts = streams[index].last_timestamp + streams[index].sampling_interval;
                            streams[index].time_stamps.emplace_back(ts);
                        }

                        //read the data
                        for (int v = 0; v < streams[index].info.channel_count; ++v)
                        {
                            double data;
                            file.read((char*)&data, 8);
                            streams[index].time_series[v].emplace_back(data);
                        }
                    }
                }
                else if (streams[index].info.channel_format.compare("int8") == 0)
                {
                    //if the time series is empty
                    if (streams[index].time_series.empty())
                        streams[index].time_series.resize(streams[index].info.channel_count);

                    //for each sample
                    for (size_t i = 0; i < numSamp; i++)
                    {
                        //read or deduce time stamp
                        uint8_t tsBytes;
                        file.read(reinterpret_cast<char *>(&tsBytes), 1);
                        if (tsBytes == 8)
                        {
                            double ts;
                            file.read((char*)&ts, 8);
                            streams[index].time_stamps.emplace_back(ts);
                        }
                        else
                        {
                            double ts = streams[index].last_timestamp + streams[index].sampling_interval;
                            streams[index].time_stamps.emplace_back(ts);
                        }

                        //read the data
                        for (int v = 0; v < streams[index].info.channel_count; ++v)
                        {
                            int8_t data;
                            file.read((char*)&data, 1);
                            streams[index].time_series[v].emplace_back(data);
                        }
                    }
                }
                else if (streams[index].info.channel_format.compare("int16") == 0)
                {
                    //if the time series is empty
                    if (streams[index].time_series.empty())
                        streams[index].time_series.resize(streams[index].info.channel_count);

                    //for each sample
                    for (size_t i = 0; i < numSamp; i++)
                    {
                        //read or deduce time stamp
                        uint8_t tsBytes;
                        file.read(reinterpret_cast<char *>(&tsBytes), 1);
                        if (tsBytes == 8)
                        {
                            double ts;
                            file.read((char*)&ts, 8);
                            streams[index].time_stamps.emplace_back(ts);
                        }
                        else
                        {
                            double ts = streams[index].last_timestamp + streams[index].sampling_interval;
                            streams[index].time_stamps.emplace_back(ts);
                        }

                        //read the data
                        for (int v = 0; v < streams[index].info.channel_count; ++v)
                        {
                            int16_t data;
                            file.read((char*)&data, 2);
                            streams[index].time_series[v].emplace_back(data);
                        }
                    }
                }
                else if (streams[index].info.channel_format.compare("int32") == 0)
                {
                    //if the time series is empty
                    if (streams[index].time_series.empty())
                        streams[index].time_series.resize(streams[index].info.channel_count);

                    //for each sample
                    for (size_t i = 0; i < numSamp; i++)
                    {
                        //read or deduce time stamp
                        uint8_t tsBytes;
                        file.read(reinterpret_cast<char *>(&tsBytes), 1);
                        if (tsBytes == 8)
                        {
                            double ts;
                            file.read((char*)&ts, 8);
                            streams[index].time_stamps.emplace_back(ts);
                        }
                        else
                        {
                            double ts = streams[index].last_timestamp + streams[index].sampling_interval;
                            streams[index].time_stamps.emplace_back(ts);
                        }

                        //read the data
                        for (int v = 0; v < streams[index].info.channel_count; ++v)
                        {
                            int32_t data;
                            file.read((char*)&data, 4);
                            streams[index].time_series[v].emplace_back(data);
                        }
                    }
                }
                else if (streams[index].info.channel_format.compare("int64") == 0)
                {
                    //if the time series is empty
                    if (streams[index].time_series.empty())
                        streams[index].time_series.resize(streams[index].info.channel_count);

                    //for each sample
                    for (size_t i = 0; i < numSamp; i++)
                    {
                        //read or deduce time stamp
                        uint8_t tsBytes;
                        file.read(reinterpret_cast<char *>(&tsBytes), 1);
                        if (tsBytes == 8)
                        {
                            double ts;
                            file.read((char*)&ts, 8);
                            streams[index].time_stamps.emplace_back(ts);
                        }
                        else
                        {
                            double ts = streams[index].last_timestamp + streams[index].sampling_interval;
                            streams[index].time_stamps.emplace_back(ts);
                        }

                        //read the data
                        for (int v = 0; v < streams[index].info.channel_count; ++v)
                        {
                            int64_t data;
                            file.read((char*)&data, 8);
                            streams[index].time_series[v].emplace_back(data);
                        }
                    }
                }
                else if (streams[index].info.channel_format.compare("string") == 0)
                {
                    //for each event
                    for (size_t i = 0; i < numSamp; i++)
                    {
                        //read or deduce time stamp
                        uint8_t tsBytes;
                        file.read(reinterpret_cast<char *>(&tsBytes), 1);
                        double ts;
                        if (tsBytes == 8)
                            file.read((char*)&ts, 8);
                        else
                            ts = streams[index].last_timestamp + streams[index].sampling_interval;

                        //read the event
                        int8_t bytes;
                        file.read((char*)&bytes, 1);
                        if (bytes == 1)
                        {
                            uint8_t length;
                            file.read((char*)&length, 1);
                            char* buffer = new char[length + 1];
                            file.read(buffer, length);
                            buffer[length] = '\0';
                            eventMap.emplace_back(buffer, ts);
                            delete[] buffer;
                        }
                        else if (bytes == 4)
                        {
                            uint32_t length;
                            file.read((char*)&length, 4);
                            char* buffer = new char[length + 1];
                            file.read(buffer, length);
                            buffer[length] = '\0';
                            eventMap.emplace_back(buffer, ts);
                            delete[] buffer;
                        }
                        else if (bytes == 8)
                        {
                            uint64_t length;
                            file.read((char*)&length, 8);
                            char* buffer = new char[length + 1];
                            file.read(buffer, length);
                            buffer[length] = '\0';
                            eventMap.emplace_back(buffer, ts);
                            delete[] buffer;
                        }

                        streams[index].last_timestamp = ts;
                    }
                }
            }
            break;
            case 4: //read [ClockOffset] chunk
            {
                int streamID;
                int index;
                file.read((char*)&streamID, 4);
                std::vector<int>::iterator it {std::find(idmap.begin(),idmap.end(),streamID)};
                if (it == idmap.end())
                {
                    index = idmap.size();
                    idmap.emplace_back(streamID);
                    streams.emplace_back();
                }
                else
                    index = std::distance(idmap.begin(), it);


                double collectionTime;
                double offsetValue;

                file.read((char*)&collectionTime, 8);
                file.read((char*)&offsetValue, 8);

                streams[index].clock_times.emplace_back(collectionTime);
                streams[index].clock_values.emplace_back(offsetValue);
            }
            break;
            case 6: //read [StreamFooter] chunk
            {
                pugi::xml_document doc;

                //read [StreamID]
                int streamID;
                int index;
                file.read((char*)&streamID, 4);
                std::vector<int>::iterator it {std::find(idmap.begin(),idmap.end(),streamID)};
                if (it == idmap.end())
                {
                    index = idmap.size();
                    idmap.emplace_back(streamID);
                    streams.emplace_back();
                }
                else
                    index = std::distance(idmap.begin(), it);


                char* buffer = new char[ChLen - 6];
                file.read(buffer, ChLen - 6);
                pugi::xml_parse_result result = doc.load_buffer_inplace(buffer, ChLen - 6);

                pugi::xml_node info = doc.child("info");

                streams[index].info.first_timestamp = info.child("first_timestamp").text().as_double();
                streams[index].info.last_timestamp = info.child("last_timestamp").text().as_double();
                streams[index].info.measured_srate = info.child("measured_srate").text().as_double();
                streams[index].info.sample_count = info.child("sample_count").text().as_int();
                delete[] buffer;
            }
            break;
            case 5:	//skip other chunk types (Boundary, ...)
                file.seekg(ChLen - 2, file.cur);
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


        //==========================================================
        //=============find the min and max time stamps=============
        //==========================================================

        findMinMax();

        findMajSR();

        //loading finishes, close file
        file.close();

    }
    else
    {
        std::cout << "Unable to open file";
        exit(EXIT_FAILURE);
    }

}

void Xdf::resampleXDF(int userSrate)
{
    //if user entered a preferred sample rate, we resample all the channels to that sample rate
    //Otherwise, we resample all channels to the sample rate that has the most channels

    clock_t time = clock();

#define BUF_SIZE 8192
    for (size_t x = 0; x< streams.size(); x++)
    {
        if (!streams[x].time_series.empty() &&
                 streams[x].info.nominal_srate != userSrate &&
                 streams[x].info.nominal_srate != 0)
        {
            int fsin = streams[x].info.nominal_srate;       // input samplerate
            int fsout = userSrate;                              // output samplerate
            double bandwidth = 0.95;                                // bandwidth
            double rp = 0.1;                                        // passband ripple factor
            double rs = 140;                                        // stopband attenuation
            double tol = 0.000001;                                  // tolerance

            // initialize smarc filter
            struct PFilter* pfilt = smarc_init_pfilter(fsin, fsout, bandwidth, rp,
                rs, tol, NULL, 0);
            if (pfilt == NULL)
                continue;

            // initialize smarc filter state
            struct PState* pstate = smarc_init_pstate(pfilt);


            for (size_t w = 0; w < streams[x].time_series.size(); w++)
            {
                // initialize buffers
                int read = 0;
                int written = 0;
                const int IN_BUF_SIZE = BUF_SIZE;
                const int OUT_BUF_SIZE = (int) smarc_get_output_buffer_size(pfilt,streams[x].time_series[w].size());
                double* inbuf = new double[streams[x].time_series[w].size()];
                double* outbuf = new double[OUT_BUF_SIZE];


                std::copy(streams[x].time_series[w].begin(),
                          streams[x].time_series[w].end(),
                          inbuf);

                read = streams[x].time_series[w].size();


                // resample signal block
                written = smarc_resample(pfilt, pstate, inbuf, read, outbuf,
                        OUT_BUF_SIZE);

                // do what you want with your output
                streams[x].time_series[w].resize(written);
                std::copy ( outbuf, outbuf+written, streams[x].time_series[w].begin() );

                // flushing last values
                written = smarc_resample_flush(pfilt, pstate, outbuf,
                        OUT_BUF_SIZE);

                // do what you want with your output
                streams[x].time_series[w].resize(streams[x].time_series[w].size()+written);
                std::copy ( outbuf, outbuf+written,
                            streams[x].time_series[w].begin()
                            + streams[x].time_series[w].size() - written );


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

           //streams[x].info.nominal_srate = majSR;
        }
    }
    //resampling finishes here


    //======================================================================
    //===========Calculating total length & total channel count=============
    //======================================================================

    calcTotalChannel();

    //global length
    totalLen = (maxTS - minTS) * userSrate;

    //beware of possible deviation
    adjustTotalLength();

    loadDictionary();

    createLabels();

    freeUpTimeStamps();

    time = clock() - time;

    std::cout << "it took " << time << " clicks (" << ((float)time) / CLOCKS_PER_SEC << " seconds)"
              << " resampling" << std::endl;

}

//function of reading the length of each chunk
uint64_t Xdf::readLength(std::ifstream &file)
{
    uint8_t bytes;
    file.read((char*)&bytes, 1);
    uint64_t length;

    switch (bytes)
    {
    case 1:
    {
        uint8_t buffer;
        file.read(reinterpret_cast<char *>(&buffer), 1);
        length = buffer;
        return length;
    }
    case 4:
    {
        uint32_t buffer;
        file.read(reinterpret_cast<char *>(&buffer), 4);
        length = buffer;
        return length;
    }
    case 8:
    {
        file.read(reinterpret_cast<char *>(&length), 8);
        return length;
    }
    default:
        std::cout << "Invalid variable-length integer encountered.\n";
        return 0;
    }
}

void Xdf::findMinMax()
{
    //find the smallest timestamp of all streams
    for (size_t i = 0; i < streams.size(); i++)
    {
        if (!streams[i].time_stamps.empty())
        {
            minTS = streams[i].time_stamps.front();
            break;
        }
    }
    for (size_t i = 0; i < streams.size(); i++)
    {
        if (!streams[i].time_stamps.empty() &&
                streams[i].time_stamps.front() < minTS)
            minTS = streams[i].time_stamps.front();
    }

    //including the timestamps of the events as well
    for (auto &elem : eventMap)
    {
        if (minTS > elem.second)
            minTS = elem.second;
    }

    //find the max timestamp of all streams
    for (size_t i = 0; i < streams.size(); i++)
    {
        if (!streams[i].time_stamps.empty() &&
                streams[i].time_stamps.back() > maxTS)
            maxTS = streams[i].time_stamps.back();
    }

    //including the timestamps of the events as well
    for (auto &elem : eventMap)
    {
        if (maxTS < elem.second)
            maxTS = elem.second;
    }
}

void Xdf::findMajSR()
{
    // find out which sample rate has the most channels
    typedef int sampRate;
    typedef int numChannel;

    std::vector<std::pair<sampRate, numChannel> > srateMap; //<srate, numChannels> pairs of all the streams

    //find out whether a sample rate already exists in srateMap
    for (size_t st = 0; st < streams.size(); st++)
    {
        if (streams[st].info.nominal_srate!=0)
        {
            std::vector<std::pair<sampRate, numChannel> >::iterator it {std::find_if(srateMap.begin(), srateMap.end(),
                                  [&](const std::pair<sampRate, numChannel> &element)
                                        {return element.first == streams[st].info.nominal_srate; })} ;
            //if it doesn't, add it here
            if (it == srateMap.end())
                srateMap.emplace_back(streams[st].info.nominal_srate, streams[st].info.channel_count);
            //if it already exists, add additional channel numbers to that sample rate
            else
            {
                int index (std::distance(srateMap.begin(),it)) ;
                srateMap[index].second += streams[st].info.channel_count;
            }
        }
    }

    //search the srateMap to see which sample rate has the most channels
    int index (std::distance(srateMap.begin(),
                             std::max_element(srateMap.begin(),srateMap.end(),
                                            [] (const std::pair<sampRate, numChannel> &largest,
                                            const std::pair<sampRate, numChannel> &first)
                                            { return largest.second < first.second; })));

    majSR = srateMap[index].first; //the sample rate that has the most channels
}

void Xdf::calcTotalChannel()
{
    //calculating total channel count, and indexing them onto streamMap
    for (size_t c = 0; c < streams.size(); c++)
    {
        if(!streams[c].time_series.empty())
        {
            totalCh += streams[c].info.channel_count;
            if (streamMap.empty())
                streamMap.emplace_back(c,streams[c].info.channel_count);
            else
                streamMap.emplace_back(c,streamMap.back().second + streams[c].info.channel_count);
        }
    }
}

void Xdf::freeUpTimeStamps()
{
    //free up as much memory as possible
    for (size_t st = 0; st < streams.size(); st++)
    {
        //we don't need to keep all the time stamps unless it's a stream with irregular samples
        if (streams[st].info.nominal_srate != 0)
        {
            std::vector<float> nothing;
            //however we still need to keep the first time stamp of each stream to decide at which position the signal should start
            nothing.emplace_back(streams[st].time_stamps.front());
            streams[st].time_stamps.swap(nothing);
        }
    }
}

void Xdf::adjustTotalLength()
{
    for (size_t st = 0; st < streams.size(); st++)
    {
        if(!streams[st].time_series.empty())
        {
            if (totalLen < streams[st].time_series.front().size())
                totalLen = streams[st].time_series.front().size();
        }
    }
}


void Xdf::createLabels()
{
    for (size_t i = 0; i < totalCh; i++)
    {
        std::string label = "Channel ";
        label += std::to_string(i);
        label += "\nStream ";
        for (size_t k = 0; k < streamMap.size(); k++)
        {
            if (i < streamMap[k].second)
            {
                int nb = streamMap[k].first;
                label += std::to_string(nb);
                label += '\n';
                label += streams[nb].info.name;
                label += '\n';
                label += streams[nb].info.type;
                break;
            }
        }

        labels.emplace_back(label);
    }
}

void Xdf::loadDictionary()
{
    //loop through the eventMap
    for (size_t e = 0; e < eventMap.size(); e++)
    {
        //search the dictionary to see whether an event is already in it
        std::vector<std::string>::iterator it
                = std::find(dictionary.begin(),dictionary.end(),eventMap[e].first);
        //if it isn't yet
        if (it == dictionary.end())
        {
            //add it to the dictionary, also store its index into eventType vector for future use
            eventType.emplace_back(dictionary.size());
            dictionary.emplace_back(eventMap[e].first);
        }
        //if it's already in there
        else
        {
            //store its index into eventType vector
            eventType.emplace_back(std::distance(dictionary.begin(), it));
        }
    }
}
