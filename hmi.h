#define R1_Y 31
#define R2_Y 95
#define R3_Y 127

void displayInit (bool first);
void clear (uint8_t y1, uint8_t y2);
void displayOff(void);
void showTime(void);
void printTime (void);
void printHeadline (char *text);
void printSoftKey(char *s1, char* s2, char* s3);
void printMain (char* text);
void entryScreen(void);
void partyMode(void);
void incPartyModeTime(void);
void decPartyModeTime(void);
void writeValues();
void absent(void);
void setPresent(void);
void setAbsent(void);
void sleep();

