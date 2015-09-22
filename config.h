
typedef struct {
  uint8_t mode;
  char name[32];
  char ssid[32];
  char pass[64];
  int maxIncrement;
  int incrementDelay;
} config;

class ConfigClass{
  public:
    config getConfig();
    bool saveConfig(config *conf);
}

