 #include <Adafruit_NeoPixel.h>
 
// Pattern types supported:
enum  pattern { NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE };
// Patern directions supported:
enum  direction { FORWARD, REVERSE };

uint8_t dageraad[ 256 ][ 3 ] = {
  {0,0,0},{3,2,1},{6,4,2},{9,6,3},{12,8,4},{15,10,5},{18,13,5},
  {21,15,6},{24,17,7},{27,19,8},{29,21,9},{32,23,10},{35,25,11},
  {38,27,12},{41,29,13},{44,31,14},{47,33,15},{50,35,15},{53,38,16},
  {56,40,17},{59,42,18},{62,44,19},{65,46,20},{68,48,21},{71,50,22},
  {74,52,23},{77,54,24},{80,56,25},{82,58,25},{85,60,26},{88,63,27},
  {91,65,28},{94,67,29},{97,69,30},{100,71,31},{103,73,32},{106,75,33},
  {109,77,34},{112,79,35},{115,81,35},{118,83,36},{121,85,37},{124,88,38},
  {127,90,39},{130,92,40},{133,94,41},{135,96,42},{138,98,43},{141,100,44},
  {144,102,45},{147,104,45},{150,106,46},{153,108,47},{156,110,48},{159,113,49},
  {162,115,50},{165,117,51},{168,119,52},{171,121,53},{174,123,54},{177,125,55},
  {180,127,55},{183,129,56},{186,131,57},{188,133,58},{189,134,59},{190,136,60},
  {191,137,62},{192,138,63},{192,139,64},{193,141,65},{194,142,66},{195,143,67},
  {196,144,68},{197,146,69},{198,147,70},{199,148,72},{200,149,73},{201,151,74},
  {201,152,75},{202,153,76},{203,154,77},{204,155,78},{205,157,79},{206,158,81},
  {207,159,82},{208,160,83},{209,162,84},{210,163,85},{210,164,86},{211,165,87},
  {212,167,88},{213,168,89},{214,169,91},{215,170,92},{216,172,93},{217,173,94},
  {218,174,95},{218,175,96},{219,176,97},{220,178,98},{221,179,99},{222,180,101},
  {223,181,102},{224,183,103},{225,184,104},{226,185,105},{227,186,106},{227,188,107},
  {228,189,108},{229,190,110},{230,191,111},{231,193,112},{232,194,113},{233,195,114},
  {234,196,115},{235,198,116},{236,199,117},{236,200,118},{237,201,120},{238,202,121},
  {239,204,122},{240,205,123},{241,206,124},{242,207,125},{243,209,126},{244,210,127},
  {245,211,128},{245,212,129},{245,213,130},{245,213,131},{246,214,132},
  {246,214,133},{246,215,133},{246,216,134},{246,216,135},{246,217,136},{247,217,136},
  {247,218,137},{247,219,138},{247,219,139},{247,220,140},
  {247,221,140},{248,221,141},{248,222,142},{248,222,143},{248,223,143},{248,224,144},
  {248,224,145},{249,225,146},{249,225,147},{249,226,147},
  {249,227,148},{249,227,149},{249,228,150},{250,228,150},{250,229,151},{250,230,152},
  {250,230,153},{250,231,153},{250,231,154},{251,232,155},
  {251,233,156},{251,233,157},{251,234,157},{251,234,158},{251,235,159},{252,236,160},
  {252,236,160},{252,237,161},{252,237,162},{252,238,163},
  {253,239,164},{253,239,164},{253,240,165},{253,241,166},{253,241,167},{253,242,167},
  {254,242,168},{254,243,169},{254,244,170},{254,244,171},
  {254,245,171},{254,245,172},{255,246,173},{255,247,174},{255,247,174},{255,248,175},
  {255,248,176},{255,249,177},{255,250,178},{255,250,178},
  {255,250,179},{255,250,180},{255,251,181},{255,251,182},{255,251,183},{255,251,184},
  {255,251,185},{255,251,186},{255,251,187},{255,251,188},
  {255,251,189},{255,251,190},{255,251,191},{255,251,192},{255,251,193},{255,252,194},
  {255,252,195},{255,252,196},{255,252,197},{255,252,198},
  {255,252,199},{255,252,200},{255,252,201},{255,252,202},{255,252,203},{255,252,203},
  {255,252,204},{255,252,205},{255,252,206},{255,253,207},
  {255,253,208},{255,253,209},{255,253,210},{255,253,211},{255,253,212},{255,253,213},
  {255,253,214},{255,253,215},{255,253,216},{255,253,217},
  {255,253,218},{255,253,219},{255,253,220},{255,254,221},{255,254,222},{255,254,223},
  {255,254,224},{255,254,225},{255,254,226},{255,254,227},
  {255,254,228},{255,254,229},{255,254,230},{255,254,231},{255,254,232},{255,254,233},
  {255,254,234},{255,255,235},{255,255,235},{255,255,236},
  {255,255,237},{255,255,238},{255,255,239},{255,255,240}
  };
  
// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : public Adafruit_NeoPixel
{
    public:
 
    // Member Variables:  
    pattern  ActivePattern;  // which pattern is running
    direction Direction;     // direction to run the pattern
    
    unsigned long Interval;   // milliseconds between updates
    unsigned long lastUpdate; // last update of position
    
    uint32_t Color1, Color2;  // What colors are in use
    uint16_t TotalSteps;  // total number of steps in the pattern
    uint16_t Index;  // current step within the pattern
    
    void (*OnComplete)();  // Callback on completion of pattern
    
    // Constructor - calls base-class constructor to initialize strip
    NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
    :Adafruit_NeoPixel(pixels, pin, type)
    {
        OnComplete = callback;
    }
    
    // Update the pattern
    void Update()
    {
        if((millis() - lastUpdate) > Interval) // time to update
        {
            lastUpdate = millis();
            switch(ActivePattern)
            {
                case RAINBOW_CYCLE:
                    RainbowCycleUpdate();
                    break;
                case THEATER_CHASE:
                    TheaterChaseUpdate();
                    break;
                case COLOR_WIPE:
                    ColorWipeUpdate();
                    break;
                case SCANNER:
                    ScannerUpdate();
                    break;
                case FADE:
                    FadeUpdate();
                    break;
                default:
                    break;
            }
        }
    }
  
    // Increment the Index and reset at the end
    void Increment()
    {
        if (Direction == FORWARD)
        {
           Index++;
           if (Index >= TotalSteps)
            {
                Index = 0;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
        else // Direction == REVERSE
        {
            --Index;
            if (Index <= 0)
            {
                Index = TotalSteps-1;
                if (OnComplete != NULL)
                {
                    OnComplete(); // call the comlpetion callback
                }
            }
        }
    }
    
    // Reverse pattern direction
    void Reverse()
    {
        if (Direction == FORWARD)
        {
            Direction = REVERSE;
            Index = TotalSteps-1;
        }
        else
        {
            Direction = FORWARD;
            Index = 0;
        }
    }
    
    // Initialize for a RainbowCycle
    void RainbowCycle(uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = RAINBOW_CYCLE;
        Interval = interval;
        TotalSteps = 255;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Rainbow Cycle Pattern
    void RainbowCycleUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
        }
        show();
        Increment();
    }
 
    // Initialize for a Theater Chase
    void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = THEATER_CHASE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
   }
    
    // Update the Theater Chase Pattern
    void TheaterChaseUpdate()
    {
        for(int i=0; i< numPixels(); i++)
        {
            if ((i + Index) % 3 == 0)
            {
                setPixelColor(i, Color1);
            }
            else
            {
                setPixelColor(i, Color2);
            }
        }
        show();
        Increment();
    }
 
    // Initialize for a ColorWipe
    void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = COLOR_WIPE;
        Interval = interval;
        TotalSteps = numPixels();
        Color1 = color;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Color Wipe Pattern
    void ColorWipeUpdate()
    {
        setPixelColor(Index, Color1);
        show();
        Increment();
    }
    
    // Initialize for a SCANNNER
    void Scanner(uint32_t color1, uint8_t interval)
    {
        ActivePattern = SCANNER;
        Interval = interval;
        TotalSteps = (numPixels() - 1) * 2;
        Color1 = color1;
        Index = 0;
    }
 
    // Update the Scanner Pattern
    void ScannerUpdate()
    { 
        for (int i = 0; i < numPixels(); i++)
        {
            if (i == Index)  // Scan Pixel to the right
            {
                 setPixelColor(i, Color1);
            }
            else if (i == TotalSteps - Index) // Scan Pixel to the left
            {
                 setPixelColor(i, Color1);
            }
            else // Fading tail
            {
                 setPixelColor(i, DimColor(getPixelColor(i)));
            }
        }
        show();
        Increment();
    }
    
    // Initialize for a Fade
    void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
    {
        ActivePattern = FADE;
        Interval = interval;
        TotalSteps = steps;
        Color1 = color1;
        Color2 = color2;
        Index = 0;
        Direction = dir;
    }
    
    // Update the Fade Pattern
    void FadeUpdate()
    {
        // Calculate linear interpolation between Color1 and Color2
        // Optimise order of operations to minimize truncation error
        uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
        uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
        uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
        
        ColorSet(Color(red, green, blue));
        show();
        Increment();
    }
   
    // Calculate 50% dimmed version of a color (used by ScannerUpdate)
    uint32_t DimColor(uint32_t color)
    {
        // Shift R, G and B components one bit to the right
        uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
        return dimColor;
    }
 
    // Set all pixels to a color (synchronously)
    void ColorSet(uint32_t color)
    {
        for (int i = 0; i < numPixels(); i++)
        {
            setPixelColor(i, color);
        }
        show();
    }
 
    // Returns the Red component of a 32-bit color
    uint8_t Red(uint32_t color)
    {
        return (color >> 16) & 0xFF;
    }
 
    // Returns the Green component of a 32-bit color
    uint8_t Green(uint32_t color)
    {
        return (color >> 8) & 0xFF;
    }
 
    // Returns the Blue component of a 32-bit color
    uint8_t Blue(uint32_t color)
    {
        return color & 0xFF;
    }
    
    // Input a value 0 to 255 to get a color value.
    // The colours are a transition r - g - b - back to r.
    uint32_t Wheel(byte WheelPos)
    {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85)
        {
            return Color(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        else if(WheelPos < 170)
        {
            WheelPos -= 85;
            return Color(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        else
        {
            WheelPos -= 170;
            return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
        }
    }
};
// 
//void Ring1Complete();
//void Ring2Complete();
//void StickComplete();
// 
//// Define some NeoPatterns for the two rings and the stick
////  as well as some completion routines
//NeoPatterns Ring1(24, 5, NEO_GRB + NEO_KHZ800, &Ring1Complete);
//NeoPatterns Ring2(16, 6, NEO_GRB + NEO_KHZ800, &Ring2Complete);
//NeoPatterns Stick(16, 7, NEO_GRB + NEO_KHZ800, &StickComplete);

 
//// Main loop
//void loop()
//{
//    // Update the rings.
//    Ring1.Update();
//    Ring2.Update();    
//    
//    // Switch patterns on a button press:
//    if (digitalRead(8) == LOW) // Button #1 pressed
//    {
//        // Switch Ring1 to FADE pattern
//        Ring1.ActivePattern = FADE;
//        Ring1.Interval = 20;
//        // Speed up the rainbow on Ring2
//        Ring2.Interval = 0;
//        // Set stick to all red
//        Stick.ColorSet(Stick.Color(255, 0, 0));
//    }
//    else if (digitalRead(9) == LOW) // Button #2 pressed
//    {
//        // Switch to alternating color wipes on Rings1 and 2
//        Ring1.ActivePattern = COLOR_WIPE;
//        Ring2.ActivePattern = COLOR_WIPE;
//        Ring2.TotalSteps = Ring2.numPixels();
//        // And update tbe stick
//        Stick.Update();
//    }
//    else // Back to normal operation
//    {
//        // Restore all pattern parameters to normal values
//        Ring1.ActivePattern = THEATER_CHASE;
//        Ring1.Interval = 100;
//        Ring2.ActivePattern = RAINBOW_CYCLE;
//        Ring2.TotalSteps = 255;
//        Ring2.Interval = min(10, Ring2.Interval);
//        // And update tbe stick
//        Stick.Update();
//    }    
//}
 
//------------------------------------------------------------
//Completion Routines - get called on completion of a pattern
//------------------------------------------------------------
// 
//// Ring1 Completion Callback
//void Ring1Complete()
//{
//    if (digitalRead(9) == LOW)  // Button #2 pressed
//    {
//        // Alternate color-wipe patterns with Ring2
//        Ring2.Interval = 40;
//        Ring1.Color1 = Ring1.Wheel(random(255));
//        Ring1.Interval = 20000;
//    }
//    else  // Retrn to normal
//    {
//      Ring1.Reverse();
//    }
//}
// 
//// Ring 2 Completion Callback
//void Ring2Complete()
//{
//    if (digitalRead(9) == LOW)  // Button #2 pressed
//    {
//        // Alternate color-wipe patterns with Ring1
//        Ring1.Interval = 20;
//        Ring2.Color1 = Ring2.Wheel(random(255));
//        Ring2.Interval = 20000;
//    }
//    else  // Retrn to normal
//    {
//        Ring2.RainbowCycle(random(0,10));
//    }
//}
// 
//// Stick Completion Callback
//void StickComplete()
//{
//    // Random color change for next scan
//    Stick.Color1 = Stick.Wheel(random(255));
//}
