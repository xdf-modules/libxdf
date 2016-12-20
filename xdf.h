#ifndef XDF_H
#define XDF_H

#include <string>
#include <vector>
#include <map>


class Xdf
{  
public:
    Xdf();

    struct Stream   //data of each stream
    {
        std::vector<std::vector<float> > time_series;
        std::vector<float> time_stamps;

        struct  //info struct
        {
            //Stream Header Chunk
            std::string name;
            std::string type;
            int channel_count;
            double nominal_srate;
            std::string channel_format;
            std::string source_id;
            float version;
            double created_at;
            std::string uid;
            std::string session_id;
            std::string hostname;
            std::string v4address;
            int v4data_port;
            int v4service_port;
            std::string v6address;
            int v6data_port;
            int v6service_port;
            struct {
                struct Channel          //description of one channel, repeated (one for each channel in the time series)
                {
                    std::string unit;   //measurement unit (strongly preferred unit: microvolts)
                    std::string type;   //channel content-type (EEG, EMG, EOG, ...)
                    std::string label;  //channel label, according to labeling scheme; the preferred labeling scheme for EEG is 10-20 (or the finer-grained 10-5)
                    std::string coordinate_system;//coordinate system of the respective parameter, can be world-space, object-space, camera-space, or image-space
                    std::string eye;    //which eye the channel is referring to (can be left, right, or both)
                    std::string impedance;    //measured impedance value during setup, in kOhm

                    std::map<std::string, std::string> location;
                    std::map<std::string, std::string> hardware;

                    struct {
                        std::map<std::string, std::string> highpass;
                        std::map<std::string, std::string> lowpass;
                        std::map<std::string, std::string> notch;
                    } filtering;        //information about the hardware/software filters already applied to the data (e.g. notch)
                };

                std::vector<Channel> channels;//per-channel meta-data; might be repeated

                std::map<std::string, std::string> reference;
                std::map<std::string, std::string> cap;
                std::map<std::string, std::string> location_measurement;
                std::map<std::string, std::string> acquisition;

                struct Fiducials        //information about a single fiducial (repeated for each one)
                {
                    std::string label;  //label of the location (e.g., Nasion, Inion, LPF, RPF); can also cover Ground and Reference
                    std::map<std::string, std::string> location; //X,Y,Z
                };

                std::vector<Fiducials> fiducials;//locations of fiducials (in the same space as the signal-carrying channels)

                struct {
                    struct {
                        float speedmode;
                    } settings;         //settings of the amplifier
                } amplifier;            //information about the used amplification (e.g. Gain, OpAmps/InAmps...)
            } desc;

            //ClockOffset chunk
            std::vector<std::pair<double, double> > clock_offsets;

            //StreamFooter chunk
            double first_timestamp;
            double last_timestamp;
            int sample_count;
            double measured_srate;
        } info;

        float last_timestamp{ 0 };      //for temporary use
        float sampling_interval;        //if srate > 0, sampling_interval = 1/srate; otherwise 0
        std::vector<double> clock_times;
        std::vector<double> clock_values;
    };

    //===============================================================================================

    std::vector<Stream> streams;
    struct
    {
        struct
        {
            float version;
        }info;
    } fileHeader;

    uint64_t totalLen;
    float minTS = 0;
    float maxTS = 0;
    size_t totalCh = 0;
    int majSR;                          //the sample rate that has the most channels;
    int maxSR = 0;                      //highest sample rate
    std::vector<int> streamMap;         //The index indicates channel count; actual content is the stream Number

    typedef std::string eventName;
    typedef float eventTimeStamp;

    std::vector<std::pair<eventName, eventTimeStamp> > eventMap; //copy all the events of all streams to here <events, timestamps>
    std::vector<std::string> dictionary;//store unique event types
    std::vector<uint16_t> eventType;    //store events by their index in the dictionary
    std::vector<std::string> labels;    //store descriptive labels of each channel
    std::vector<int> sampleRateMap;     //store all sample rates across all the streams

    //=============================================================================================

    void createLabels();                //create descriptive labels

    void loadDictionary();              //copy events into dictionary (with no repeats)

    void load_xdf(std::string filename);

    void resampleXDF(int userSrate);

    uint64_t readLength(std::ifstream &file);

    void findMinMax();                  //find Min & Max time stamps

    void findMajSR();                   //find the major sample rate that has the most channels

    void calcTotalChannel();            //calculating total channel count

    void calcTotalLength(int sampleRate);//calculating the globle length from the earliest time stamp to the last time stamp across all channels

    void freeUpTimeStamps();            //to release some memory

    void adjustTotalLength();           //If total length is shorter than the actual length of any channel, make it equal to length of that channel

    void getHighestSampleRate();

    void loadSampleRateMap();
};

#endif // XDF_H
