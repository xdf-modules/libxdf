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

/*! \file xdf.h
 * \brief The header file of Xdf class
 */

#ifndef XDF_H
#define XDF_H

#include <string>
#include <vector>
#include <map>
#include <set>

/*! \class Xdf
 *
 * Xdf class stores the data of an entire XDF file. It contains methods to
 * read XDF files and containers to store the data, as well as additional
 * methods such as resampling etc.
 */

class Xdf
{  
public:
    //! Default constructor with no parameter.
    Xdf();

    /*! \class Stream
     *
     * XDF files uses stream as the unit to store data. An XDF file usually
     * contains multiple streams, each of which may contain one or more
     * channels. The Stream struct provides a way to store meta-data, time-series,
     * time-stamps and all other information of a single stream from an XDF file.
     */
    struct Stream
    {
        std::vector<std::vector<float>> time_series; /*!< 2D vector that stores the time series of a stream. Each row represents a channel.*/
        std::vector<double> time_stamps;    /*!< Stores the time stamps. */
        std::string stream_header;          /*!< Raw XML of stream header chunk. */
        std::string stream_footer;          /*!< Raw XML of stream footer chunk. */

        struct
        {
            int channel_count;              /*!< Number of channels in the current stream */
            double nominal_srate;           /*!< The nominal sampling rate of the current stream. */
            std::string name;               /*!< The name of the current stream. */
            std::string type;               /*!< The type of the current stream. */
            std::string channel_format;     /*!< The channel format of the current stream. */

            std::vector<std::map<std::string, std::string> > channels; /*!< Stores the meta-data of the channels of the current stream. */

            std::vector<std::pair<double, double> > clock_offsets;  /*!< Stores the clock offsets from the ClockOffset chunk. */

            double first_timestamp;         /*!< First time stamp of the stream. */
            double last_timestamp;          /*!< Last time stamp of the stream. */
            int sample_count;               /*!< Sample count of the stream. */
            double measured_sampling_rate;  /*!< Measured sampling rate of the stream. */
            double effective_sampling_rate = 0; /*!< Effective sampling rate. */
        } info;                             /*!< Meta-data from the stream header of the current stream. */

        double last_timestamp{0};           /*!< For temporary use while loading the vector */
        double sampling_interval;           /*!< sampling_interval = 1/sampling_rate if sampling_rate > 0, otherwise 0 */
        std::vector<double> clock_times;    /*!< Stores the clock times from clock offset chunks. */
        std::vector<double> clock_values;   /*!< Stores the clock values from clock offset chunks. */
    };

    std::vector<Stream> streams_;           /*!< Stores all streams of the current XDF file. */
    float version_;                         /*!< The version of the XDF file. */

    uint64_t total_len_ = 0;                /*!< The total length is the product of the range between the smallest time stamp and the largest multiplied by major_sampling_rate_. */

    double min_timestamp_ = 0;              /*!< The min time stamp across all streams. */
    double max_timestamp_ = 0;              /*!< The max time stamp across all streams. */
    size_t channel_count_ = 0;              /*!< The total number of channels. */
    int major_sampling_rate_ = 0;           /*!< The sampling rate that was used by the most channels across all streams. */
    int max_sampling_rate_ = 0;             /*!< Max sampling rate across all streams. */
    std::vector<double> effective_sampling_rates_;  /*!< Effective sampling rates of streams. */
    double file_effective_sampling_rate_ = 0;   /*!< If effective_sampling_rates_ in all the streams are the same, this is the value. */
    std::vector<int> stream_map_;           /*!< Indexes which channels belong to which stream. The index is the same as channel number; the actual content is the stream number. */

    /*!
     * \brief An alias of std::string type used on event names.
     * \sa eventMap
     */
    typedef std::string event_name_;
    /*!
     * \brief An alias of double type used on event timestamps.
     * \sa eventMap
     */
    typedef double event_timestamp_;

    std::vector<std::pair<std::pair<event_name_, event_timestamp_>, int>> event_map_;   /*!< Stores all events across all streams. */
    std::vector<std::string> dictionary_;   /*!< Stores unique event types with no repetitions. \sa eventMap */
    std::vector<uint16_t> event_type_;      /*!< Stores events by their indices in the dictionary.\sa dictionary, eventMap */
    std::vector<std::string> labels_;       /*!< Stores descriptive labels of each channel. */
    std::set<double> sampling_rate_map_;    /*!< Stores sampling rates of all the streams. */
    std::vector<float> offsets_;            /*!< Offsets of each channel after using SubtractMean() method. */

    std::string file_header_;               /*!< Raw XML of the file header. */
    int user_added_stream_{0};              /*!< Used by SigViewer only: if user manually added events in SigViewer, the events will be stored in a new stream after all current streams with index user_added_stream_. */
    std::vector<std::pair<std::string, double> > user_created_events_;  /*!< Events created by user in SigViewer. */

    /*!
     * \brief Adjusts `total_len_` to avoid possible deviation.
     *
     * `total_len_` is calculated by multiplying the difference between
     * `max_timestamp_` and `min_timestamp_` by `major_sampling_rate_`.
     * However, this can be inaccurate. In case any channel is longer than
     * `total_len_`, this function will make `total_len_` match the length
     * of that channel.
     *
     * \sa total_len_, major_sampling_rate_, CalculateTotalLength()
     */
    void AdjustTotalLength();

    /*!
     * \brief Calculates the globle length (in samples).
     *
     * This is calculated by multiplying the rage from `min_timestamp_`
     * to `max_timestamp_` across all channels by `sampling_rate`.
     *
     * \param sampling_rate The sampling rate used to calculate the total
     * length.
     */
    void CalculateTotalLength(int sampling_rate);

    /*!
     * \brief Creates labels for each channel and stores them in `labels_`.
     * \sa labels_, offsets_
     */
    void CreateLabels();

    /*!
     * \brief Subtracts all data in a channel by the mean.
     *
     * SigViewer displays both the channel signals as well as the zero baseline.
     * When the mean of a channel is too high or too low it becomes difficult to
     * see the fluctuation. Subtracting the entire channel by its mean makes the
     * signals fluctuate around the zero baseline, thus having better visual
     * effect. The mean of each channel times `-1` is stored in `offsets_`.
     *
     * \sa offsets_
     */
    void Detrend();

    /*!
     * \brief Deletes `Stream::time_stamps` when no longer needed (to release
     * memory).
     *
     * SigViewer doesn't demand time stamps to display signals except irregular
     * sampling rate channels, events, and the min timestamp of each channel
     * (used to determine where does a channel start when putting all streams
     * together). Hence `Stream::time_stamps` can be deleted when no longer
     * needed to free up the memory.
     */
    void FreeUpTimeStamps();

    /*!
     * \brief Loads an XDF file.
     * \param filename The complete path to the target file.
     */
    int LoadXdf(std::string filename);

    /*!
     * \brief Resamples all streams and channels to `sampling_rate`.
     * \param `sampling_rate` in general should be between 1 and the highest
     * sampling rate of the current file.
     */
    void Resample(int sampling_rate);

    /*!
     * \brief Sync the timestamps.
     */
    void SyncTimeStamps();

    /*!
     * \brief Writes events to the XDF file. Used when user added markups
     * and/or events in SigViewer, this method stores the newly created events
     * to the XDF file in a new stream.
     */
    int WriteEventsToXdf(std::string file_path);

private:

    /*!
     * \brief Calculates the effective sampling rate.
     */
    void CalculateEffectiveSamplingRate();

    /*!
     * \brief Calculates the channel count and stores the result in
     * `channel_count_`. Channels of both regular and irregular sampling rates
     * are included. Streams with channel format `string` are excluded, and are
     * stored in `event_map_` instead.
     *
     * \sa channel_count_, event_map_
     */
    void CalculateChannelCount();

    /*!
     * \brief Finds the sampling rate that is used by the most channels.
     *
     * XDF format supports different sample rates across streams, but
     * SigViewer needs to display all channels in a unified sampling rate.
     * If a file contains more than one sampling rate, some channels need
     * to be resampled to be displayed. This method finds the sampling
     * rate that was used by the most channels thus minimizing the number
     * of channels to be resampled.
     *
     * \sa major_sampling_rate_, Resample(int sampling_rate)
     */
    void FindMajorSamplingRate();

    /*!
     * \brief Finds the max sampling rate of all streams and store it in `max_sampling_rate_`.
     *
     * \sa max_sampling_rate_
     */
    void FindMaxSampleRate();

    /*!
     * \brief Finds the min and max timestamps across all streams. The
     * results are stored in `min_timestamp_` and `max_timestamp_`.
     * \sa  min_timestamp_, max_timestamp_, CalculateTotalLength(int sampling_rate);
     */
    void FindMinMaxTimeStamps();

    /*!
     * \brief Copies all unique types of events from `event_map_` to
     * `dictionary_` excluding duplicates.
     * \sa event_map_, dictionary_
     */
    void LoadDictionary();

    /*!
     * \brief Loads every sampling rate that appeared in the current file
     * into `sampling_rate_map_`.
     * \sa sampling_rate_map_
     */
    void LoadSamplingRateMap();

    /*!
     * \brief Gets the length of the upcoming chunk, or the number of samples.
     *
     * While loading XDF file there are two cases where this method are needed.
     * One is to get the length of each chunk, the other is to get the number
     * of samples when loading the time series (Chunk tag 3).
     *
     * \param file The XDF file that is being loaded in the type of `std::ifstream`.
     * \return The length of the upcoming chunk (in bytes).
     */
    uint64_t ReadLength(std::ifstream &file);

    /*!
     * \brief Reads a binary scalar variable from an input stream.
     *
     * This method is a convenience wrapper for the common
     * `file.read((char*) var, sizeof(var))` operation. Examples:
     *
     * ```
     * double foo = readBin<double>(file);  // use return value
     * ReadBin(file, &foo);                 // read directly into foo
     * ```
     *
     * \param is An input stream to read from.
     * \param obj Pointer to a variable to load the data into or nullptr.
     * \return The read data.
     */
    template<typename T> T ReadBin(std::istream& is, T* obj = nullptr);
};

#endif // XDF_H
