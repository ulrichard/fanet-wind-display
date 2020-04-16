// Skytraxx fanet
#include <fframe.h>
#include <fneighbor.h>
#include <frame/fname.h>
#include <frame/fservice.h>
// Adafruit display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// Arduino
#include <Wire.h>

const uint8_t I2C_ADDRESS_DISPLAY = 0x3C;
const uint8_t OLED_RESET = 7;

Adafruit_SSD1306 display(OLED_RESET);
String inputString = "";
bool stringComplete = false;

// devices that we want to receive
FanetNeighbor weatherUrmi(FanetMacAddr(0x01, 0x110));
//FanetNeighbor richiSktr30(FanetMacAddr(0x11, 0xD46));

// example strings:    src_manufacturer, src_id, broadcast (0/1), signature, type, length, payload 
const String exampleWeatherN = "#FNF 1,110,1,0,2,C,55726D696265726748616E67";
const String exampleWeatherW = "#FNF 1,110,1,0,4,B,265BDA42B61B06A9000004";
//const String exampleST30     = "#FNF 11,D46,1,0,2,E,5269636861726420556C72696368";



void setup()
{
    Serial.begin(115200);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    inputString.reserve(200);
    delay(500);
    
    Serial.println("#DGP 1"); // power up the device
    Serial.println("#FNP 1"); // enable the FANET receiver

    // test output
    Decode(exampleWeatherW);
    Decode(exampleWeatherN);
    DisplayInfos();
}

FanetNeighbor* FindNeighbor(const FanetMacAddr& addr)
{
    if(addr == weatherUrmi.addr)
        return &weatherUrmi;
//    else if(addr == richiSktr30.addr)
//        return &richiSktr30;
    else
        return nullptr;
}

bool Decode(const String& input)
{
    if(strncmp(input.c_str(), "#FNF ", 5))
        return false;

    char *strings[10] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
    char *ptr = NULL;
    byte index = 0;
    ptr = strtok(const_cast<char*>(input.c_str()) + 5, ",");
    while(ptr != NULL)
    {
        strings[index] = ptr;
        index++;
        ptr = strtok(NULL, ",");
    }
    if(index < 7)
        return false;

    const auto  src_manufacturer = strtoul(strings[0], NULL, 16);
    const auto  src_id = strtoul(strings[1], NULL, 16);
    auto* pNeighbor = FindNeighbor(FanetMacAddr(src_manufacturer, src_id));
    if(pNeighbor == nullptr)
        return false;
    
    const bool  isBroadcast = atoi(strings[2]);
    const auto  signature = atoi(strings[3]);
    const auto  msgType = atoi(strings[4]);
    const auto  length = strtoul(strings[5], NULL, 16);

    if(length >= 20)
        return false;
    
    uint8_t payload[20];
    for(size_t i = 0; i < length; ++i)
    {
        char item[3] = {strings[6][i * 2], strings[6][i * 2 + 1], '\0'};
        payload[i] = strtoul(item, NULL, 16);
    }
    payload[length] = '\0';


    switch(msgType)
    {
        case FanetFrame::FrameType_t::TYPE_NAME: // 2
            FanetFrameName::decode(payload, length, pNeighbor);
	        break;

        case FanetFrame::FrameType_t::TYPE_SERVICE: // 4
            FanetFrameService::decode(payload, length, pNeighbor);
	        break;

        case FanetFrame::FrameType_t::TYPE_ACK: // 0
        case FanetFrame::FrameType_t::TYPE_TRACKING: // 1
        case FanetFrame::FrameType_t::TYPE_MESSAGE: // 3
        case FanetFrame::FrameType_t::TYPE_LANDMARK: // 5
        case FanetFrame::FrameType_t::TYPE_REMOTECONFIG: // 6
        case FanetFrame::FrameType_t::TYPE_GROUNDTRACKING: // 7
        case FanetFrame::FrameType_t::TYPE_HWINFO: // 8
        default:
            break;
    };

    return true;
}

void DisplayInfos()
{
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setCursor(0, 0);

    display.setTextSize(2);
    display.println(weatherUrmi.name);

    display.setTextSize(2);
    const auto speed = static_cast<uint16_t>(weatherUrmi.speed_kmh * 10.0f);
    display.print(speed / 10, DEC);
    display.print(".");
    display.print(speed % 10, DEC);
    display.println(" kmh");

    display.setTextSize(2);
    const uint16_t direction = weatherUrmi.heading_rad * RAD_TO_DEG;
    display.print(direction, DEC);
    display.print(" ");
  
/*  
    if(direction < 13)
        display.println("N");
    else if(direction < 35)
        display.println("NNW");
    else if(direction < 57)
        display.println("NW");
    else if(direction < 80)
        display.println("WNW");
    else if(direction < 102)
        display.println("W");
    else if(direction < 125)
        display.println("WSW");
    else if(direction < 147)
        display.println("SW");
    else if(direction < 170)
        display.println("SSW");
    else if(direction < 192)
        display.println("S");
    else if(direction < 215)
        display.println("SSE");
    else if(direction < 237)
        display.println("SE");
    else if(direction < 260)
        display.println("ESE");
    else if(direction < 282)
        display.println("E");
    else if(direction < 305)
        display.println("ENE");
    else if(direction < 327)
        display.println("NE");
    else if(direction < 350)
        display.println("NNE");
    else
        display.println("N");
*/

    display.display();
}

void loop()
{
    if(stringComplete)
    {
  Serial.println(inputString.c_str());
        if(Decode(inputString))
        {    
            inputString = "";
            stringComplete = false;
            DisplayInfos();
        }
        else
  Serial.println("decoding error");
    }
}

void serialEvent()
{
    while(Serial.available())
    {    
        const char inc = static_cast<char>(Serial.read());
        inputString += inc;
        if(inc == '\n')
            stringComplete = true;
    }
}

// ToDo: get rid of this
/*
void operator delete(void * p, size_t) 
{
  free(p);
}
*/
