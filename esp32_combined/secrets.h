// All confidential information has been removed; this file acts as a placeholder
#include <pgmspace.h>

#define SECRET
#define THINGNAME "ESP32_test" // do not change this!

const char *WIFI_SSID = "XXX";
const char *WIFI_PASSWORD = "XXX";
const char *AWS_IOT_ENDPOINT = "XXX";

// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
XXX
-----END CERTIFICATE-----
)EOF";

// Device Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
XXX
-----END CERTIFICATE-----
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
XXX
-----END RSA PRIVATE KEY-----
)KEY";