#ifndef __GEOSTAR_DEFS__
#define __GEOSTAR_DEFS__

#define WORD 4
#define MSG_CHECKSUM 4
#define MSG_HEADER 12
#define RAW_MEASUREMENT 0x10
#define SVS_IN_VIEW 0x22
#define BASIC_DATA_SET 0x20
#define MAX_MESSAGE_LENGHTH 340

#define FIFO_SIZE 1024
//#define TIME_DIFF 252460800 //difference in unix time to time of geostar
#define TIME_DIFF 567993600//difference in unix time to time of geostar

typedef struct gs_0x10 {
} gs_0x10;

typedef struct gs_0x20 {
    uint16_t length, msg_type;
    uint32_t msg_length;
	double position, latitude, longitude, heigth, geoidal_seperation;
	uint32_t numbers_sv, receiver_status;
    uint64_t gdop, pdop, tdop, hdop, vdop;
    uint32_t position_fix_valid_indicator, number_continuous_fixes;
    uint64_t speed, course;
    uint32_t checksum;
} gs_0x20;

typedef struct gs_0x21 {
    uint16_t length, msg_type;
    uint32_t checksum;
    uint32_t receiver_status_word, receiver_configuration, 
             uptime, time, word5, word6;
} gs_0x21;

/* consist of two parts, nested */
typedef struct gs_0x22_sat{
    char sv_number, channel_nr;
    uint16_t carrier_frequency;
    uint32_t word1, word2;
    float    snr,elevation, azimuth;
} gs_0x22_sat;

typedef struct gs_0x22 {
    uint16_t length, msg_type;
    uint32_t checksum;
	uint32_t nsat;
    gs_0x22_sat *sat; 
} gs_0x22;

#endif

