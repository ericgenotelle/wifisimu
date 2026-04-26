#include <WiFi.h>
#include <esp_wifi.h>
#include <tcpip_adapter.h>

#define AP_SSID "PHOENIX"
#define AP_PASS "12345678"

/* ---------- Table MAC → IP ---------- */

typedef struct {
    uint8_t mac[6];
    IPAddress ip;
} StaticLease;

/* Liste des machines connues */

StaticLease leases[] = {

//  14-5A-FC-5A-E0-F9
    { {0x14,0x5A,0xFC,0x5A,0xE0,0xF9},
      IPAddress(192,168,137,1) },

    { {0xAA,0xBB,0xCC,0xDD,0xEE,0x02},
      IPAddress(192,168,137,2) }

};

#define NUM_LEASES \
(sizeof(leases)/sizeof(leases[0]))

/* ---------- Recherche IP par MAC ---------- */

bool findReservedIP(uint8_t *mac, IPAddress &ip)
{
    for (int i = 0; i < NUM_LEASES; i++) {

        if (memcmp(mac,
                   leases[i].mac,
                   6) == 0) {

            ip = leases[i].ip;
            return true;
        }
    }

    return false;
}

/* ---------- Event WiFi ---------- */

void WiFiEvent(WiFiEvent_t event,
               WiFiEventInfo_t info)
{
    switch (event) {

        case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        {
            uint8_t *mac =
                info.wifi_ap_staconnected.mac;

            Serial.printf(
                "Client connecté: "
                "%02X:%02X:%02X:%02X:%02X:%02X\n",
                mac[0], mac[1], mac[2],
                mac[3], mac[4], mac[5]);

            IPAddress ip;

            if (findReservedIP(mac, ip)) {

                Serial.print(     "IP réservée trouvée: ");
                Serial.println(ip);

                /* Assignation IP statique */

                wifi_sta_list_t wifi_sta_list;
                tcpip_adapter_sta_list_t adapter_sta_list;

                esp_wifi_ap_get_sta_list(
                    &wifi_sta_list);

                tcpip_adapter_get_sta_list(
                    &wifi_sta_list,
                    &adapter_sta_list);

                for (int i = 0;
                     i < adapter_sta_list.num;
                     i++) {

                    tcpip_adapter_sta_info_t station =
                        adapter_sta_list.sta[i];

                    if (memcmp(
                        station.mac,
                        mac,
                        6) == 0) {

                        Serial.print("Station trouvée\n");

                        /* Ici on pourrait gérer
                           des IP fixes avancées */

                    }
                }

            } else {

                Serial.println(
                    "MAC inconnue -> DHCP normal");
            }

            break;
        }

        case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        {
            uint8_t *mac =
                info.wifi_ap_stadisconnected.mac;

            Serial.printf(
                "Client déconnecté: "
                "%02X:%02X:%02X:%02X:%02X:%02X\n",
                mac[0], mac[1], mac[2],
                mac[3], mac[4], mac[5]);

            break;
        }

        default:
            break;
    }
}

/* ---------- Setup ---------- */

void setup()
{
    Serial.begin(115200);

    WiFi.onEvent(WiFiEvent);

    IPAddress local_ip(192,168,137,254);
    IPAddress gateway(192,168,137,254);
    IPAddress subnet(255,255,255,0);

    WiFi.softAPConfig(
        local_ip,
        gateway,
        subnet);

    WiFi.softAP(
        AP_SSID,
        AP_PASS,
        1,
        false,
        4);

    Serial.println("Point d'accès lancé");
    Serial.print("IP AP: ");
    Serial.println(WiFi.softAPIP());
}

/* ---------- Loop ---------- */

void loop()
{
    delay(1000);
}