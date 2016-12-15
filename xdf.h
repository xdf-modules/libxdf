#ifndef XDF_H
#define XDF_H

#include <string>
#include <vector>


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
                    struct {
                        float X;        //coordinate axis pointing from the center of the head to the right, in millimeters
                        float Y;        //coordinate axis pointing from the center of the head to the front, in millimeters
                        float Z;        //coordinate axis pointing from the center of the head to the top, in millimeters
                    } location;         //measured location (note: this may be arbitrary but should then include well-known fiducials (landmarks) for co-registration)

                    struct {
                        std::string model;//model of the sensor
                        std::string manufacturer;//manufacturer of the sensor
                        std::string coupling;//type of coupling used (can be Gel, Saline, Dry, Capacitive)
                        std::string material;//conductive material of the sensor (e.g. Ag-AgCl, Rubber, Foam, Plastic)
                        std::string surface;//type of the contact surface (e.g., Plate, Pins, Bristle, Pad)
                    } hardware;         //information about hardware properties

                    float impedance;    //measured impedance value during setup, in kOhm

                    struct {
                        struct {
                            std::string type;//type of the filter (FIR, IIR, Analog)
                            std::string design;//design of the filter (e.g., Butterworth, Elliptic)
                            float lower;//lower edge of the transition frequency band (in Hz)
                            float upper;//upper edge of the transition frequency band (in Hz)
                            float order;//filter order, if any
                        } highpass;     //highpass filter, if any

                        struct {
                            std::string type;//type of the filter (FIR, IIR, Analog)
                            std::string design;//design of the filter (e.g., Butterworth, Elliptic)
                            float lower;//lower edge of the transition frequency band (in Hz)
                            float upper;//upper edge of the transition frequency band (in Hz)
                            float order;//filter order, if applicable
                        } lowpass;      //lowpass filter, if any

                        struct {
                            std::string type;//type of the filter (FIR, IIR, Analog)
                            std::string design;//design of the filter (e.g., Butterworth, Elliptic)
                            float center;//center frequency of the notch filter (in Hz)
                            float bandwidth;//width of the notch frequency band (in Hz), if known
                            float order;//filter order, if applicable
                        } notch;        //notch filter, if any
                    } filtering;        //information about the hardware/software filters already applied to the data (e.g. notch)
                };

                std::vector<Channel> channels;//per-channel meta-data; might be repeated


                struct {                //signal referencing scheme
                    std::string label;  // name of the dedicated reference channel(s), if part of the measured channels (repeated if multiple)
                    bool subtracted;    //Yes if a reference signal has already been subtracted from the data, otherwise No
                    bool common_average;//Yes if the subtracted reference signal was a common average, otherwise No
                } reference;

                struct Fiducials        //information about a single fiducial (repeated for each one)
                {
                    std::string label;  //label of the location (e.g., Nasion, Inion, LPF, RPF); can also cover Ground and Reference
                    struct {
                        float X;        //coordinate axis pointing from the center of the head to the right, in millimeters
                        float Y;        //coordinate axis pointing from the center of the head to the front, in millimeters
                        float Z;        //coordinate axis pointing from the center of the head to the top, in millimeters
                    } location;         //measured location (same coordinate system as channel locations)
                };

                std::vector<Fiducials> fiducials;//locations of fiducials (in the same space as the signal-carrying channels)

                struct {
                    std::string name;   //name of the cap (e.g. EasyCap, ActiCap, CustomBiosemiCapA)
                    float size;         //cap size, usually as head circumference in cm (typical values are 54, 56, or 58)
                    std::string manufacturer;//manufacturer of the cap (e.g. BrainProducts)
                    std::string labelscheme;//the labeling scheme for the cap (e.g. 10-20, BioSemi-128, OurCustomScheme)
                } cap;                  //EEG cap description

                struct {
                    struct {
                        float speedmode;
                    } settings;         //settings of the amplifier
                } amplifier;            //information about the used amplification (e.g. Gain, OpAmps/InAmps...)

                struct {
                    std::string model;  //model name of the system, e.g. CMS10
                    std::string manufacturer;//manufacturer of the measurement system, e.g. Polhemus, Zebris
                    std::string locationfile;//file system path of the (backup) location file, if any
                    struct {

                    } settings;         //settings of the location measurement system
                } location_measurement; //information about the sensor localization system/method

                struct {
                    std::string manufacturer;//manufacturer of the amplifier (e.g. BioSemi)
                    std::string model;  //model name of the amplifier (e.g. BrainAmp)
                    float precision;    //the theoretical number of bits precision delivered by the amplifier (typical values are 8, 16, 24, 32)
                    float compensated_lag;//amount of hardware/system lag that has been implicitly compensated for in the stream's time stamps (in seconds)
                } acquisition;
            } desc;

            //ClockOffset chunk
            std::vector<std::pair<double, double> > clock_offsets;

            //StreamFooter chunk
            double first_timestamp;
            double last_timestamp;
            int sample_count;
            double measured_srate;
        } info;

        float last_timestamp{ 0 };  //for temporary use
        float sampling_interval;    //if srate > 0, sampling_interval = 1/srate; otherwise 0
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
    int majSR;  //the sample rate that has the most channels;
    int maxSR = 0;  //highest sample rate
    std::vector<int> streamMap; //The index indicates channel count; actual content is the stream Number

    typedef std::string eventName;
    typedef float eventTimeStamp;

    std::vector<std::pair<eventName, eventTimeStamp> > eventMap; //copy all the events of all streams to here <events, timestamps>
    std::vector<std::string> dictionary;    //store unique event types
    std::vector<uint16_t> eventType;        //store events by their index in the dictionary
    std::vector<std::string> labels;        //store descriptive labels of each channel

    //=============================================================================================

    void createLabels();                    //create descriptive labels

    void loadDictionary();                  //copy events into dictionary (with no repeats)

    void load_xdf(std::string filename);

    void resampleXDF(int userSrate);

    uint64_t readLength(std::ifstream &file);

    void findMinMax();      //find Min & Max time stamps

    void findMajSR();       //find the major sample rate that has the most channels

    void calcTotalChannel();//calculating total channel count

    void calcTotalLength(int sampleRate); //calculating the globle length from the earliest time stamp to the last time stamp across all channels

    void freeUpTimeStamps();//to release some memory

    void adjustTotalLength();//If total length is shorter than the actual length of any channel, make it equal to length of that channel

    void getHighestSampleRate();
};

#endif // XDF_H
