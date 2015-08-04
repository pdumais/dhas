// Audio settings
#define DEFAULT_CHUNKSIZE 160
#define DEFAULT_BITRESOLUTION 8
#define DEFAULT_NUMCHANNELS 1
#define DEFAULT_SAMPLERATE 8000


// positioning for calculating sunset/sunrise
#define LATITUDE 0
#define LONGITUDE 0

// devices
#define SOUND_DEVICE "default" //hw:0,0 and plughw:0,0 wont let share between other apps
#define INSTEON_SERIAL_PORT "/dev/ttyUSB7"
#define COUCHDB_SERVER "127.0.0.1"

// IP addresses and ports
#define SIP_PORT 5068
#define LOCAL_IP "192.168.5.10"
#define SYSLOG_SERVER "192.168.5.10"
#define WEBSERVER_PORT 555
#define RTP_HIGH_PORT 11100
#define RTP_LOW_PORT 11000
#define SMTP_PORT 26

// timeouts
#define RONA_TIMEOUT 20
#define MAX_LOGS 100

// Paths
#define SOUNDS_FOLDER "/opt/ha/sounds/"
#define SCRIPT_FILE "/opt/ha/script.lua"
#define PASSWD_FILE "/opt/ha/passwd"
#define ALARMLOGS "/opt/ha/log/alarm.log"
#define PERSISTANTDB "/opt/ha/vars.db"
#define SCHEDULEDB "/opt/ha/schedule.db"

