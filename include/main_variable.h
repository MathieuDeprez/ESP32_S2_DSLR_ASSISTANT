#ifndef MAIN_VARIABLE
#define MAIN_VARIABLE
enum class AzertyHid {
    A = 0x20,
    B = 0x05,
    C = 0x06
};

char AzertyArray[102][8] = {
    "",
    "",
    "",
    "",
    "Q",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "?",
    "N",
    "O",
    "P",
    "A",
    "R",
    "S",
    "T",
    "U",
    "V",
    "Z",
    "X",
    "Y",
    "W",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "0",
    "Enter",
    "Esc",
    "Back",
    "Tab",
    "Space",
    ")",
    "=",
    "",
    "",
    "Or",
    "",
    "M",
    "",
    "²",
    "",
    ":",  //>
    "",
    "Caps",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "",
    "",
    "",
    "Right",
    "",
    "",
    "Del",
    "",
    "",
    "",
    "Left",
    "Bottom",
    "Top",
    "PNNum",
    "PN/",
    "PN*",
    "PN-",
    "PN+",
    "PNEnter",
    "PN1",
    "PN2",
    "PN3",
    "PN4",
    "PN5",
    "PN6",
    "PN7",
    "PN8",
    "PN9",
    "PN0",
    "PN,",
    "",
    "Menu"};

char AzertyArraySpe[8][8] = {
    "CtrlL",   // 0x01    1
    "ShiftL",  // 0x02    2
    "Alt",     // 0x04    4
    "WinL",    // 0x08    8
    "CtrlR",   // 0x10    16
    "ShiftR",  // 0x20    32
    "AltGr",   // 0x40    64
    "WinR",    // 0x80    128
};
#endif