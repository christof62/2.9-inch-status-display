#include <Fsm.h>
#include <GxEPD.h>
#include "menu.h"
#include "hmi.h"


State sOff(&sleep, nullptr, nullptr);
State sEntry(&writeValues, nullptr, nullptr);
State sPartyMode(nullptr, nullptr, nullptr);
State sAbsent(nullptr, nullptr, nullptr);


Fsm menuFsm(&sEntry);

void menuInit (){
  menuFsm.add_timed_transition (&sEntry, &sOff, 8000, nullptr);
  menuFsm.add_timed_transition (&sPartyMode, &sEntry, 9000, &entryScreen);
  menuFsm.add_timed_transition (&sAbsent, &sEntry, 9000, &entryScreen);
  menuFsm.add_transition (&sEntry, &sEntry, FIRST_BOOT, &entryScreen);
  menuFsm.add_transition (&sEntry, &sEntry, TIME_UPDATE, &showTime);
  menuFsm.add_transition (&sEntry, &sOff, SERVER_FAILED, nullptr);
  menuFsm.add_transition (&sEntry, &sOff, SERVER_SUCCEED, nullptr);
  menuFsm.add_transition (&sEntry, &sPartyMode, R_PRESSED, &partyMode);
  menuFsm.add_transition (&sEntry, &sPartyMode, BUTTON_WAKEUP, &partyMode);
  menuFsm.add_transition (&sPartyMode, &sPartyMode, M_PRESSED, &incPartyModeTime);
  menuFsm.add_transition (&sPartyMode, &sPartyMode, L_PRESSED, &decPartyModeTime);
  menuFsm.add_transition (&sPartyMode, &sAbsent, R_PRESSED, &absent);
  menuFsm.add_transition (&sAbsent, &sAbsent, M_PRESSED, &setPresent);
  menuFsm.add_transition (&sAbsent, &sAbsent, L_PRESSED, &setAbsent);
  menuFsm.add_transition (&sAbsent, &sPartyMode, R_PRESSED, &partyMode);  
}


