/*
 * Copyright 2012 by Alison Leonard <alison@makerbot.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
 
 #include "Interface.hh"
 #include <util/delay.h>
 
 #include "UtilityScripts.hh"
 #include <avr/pgmspace.h>
 #include "EepromMap.hh"
 #include "Menu_locales.hh"
 
 static uint16_t Lengths[3]  PROGMEM = { 75, /// Home Axes
                                         LEVEL_PLATE_LEN,
                                         4630}; /// nozzle (toolhead) calibrate
                            

static uint8_t HomeAxes[] PROGMEM = { 131,  4,  136,  0,  0,  0,  20,  0,  140,  0,  0,  0,  0,  0,  0,  0,  0, 48,  248,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  142,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  87,  41,  4,  0,  24,  131,  4,  220,  5,  0,  0,  20,  0,  132,  3,  105,  1,  0,  0,  20,  0,  144,  31,  137,  31 };
static uint8_t NozzleCalibrate[] PROGMEM = { 153,  0,  0,  0,  0,  82,  101,  112,  71,  32,  66,  117,  105,  108,  100,  0,  150,  0,  255,  134,  0,  136,  0,  31,  2,  110,  0,  136,  1,  31,  2,  110,  0,  134,  0,  136,  0,  3,  2,  220,  0,  134,  1,  136,  1,  3,  2,  220,  0,  132,  3,  105,  1,  0,  0,  20,  0,  131,  4,  136,  0,  0,  0,  20,  0,  140,  0,  0,  0,  0,  0,  0,  0,  0,  48,  248,  255,  255,  0,  0,  0,  0,  0,  0,  0,  0,  142,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  87,  41,  4,  0,  24,  131,  4,  220,  5,  0,  0,  20,  0,  144,  31,  142,  30,  35,  0,  0,  40,  229,  255,  255,  96,  234,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  155,  23,  98,  0,  24,  145,  0,  0,  145,  1,  0,  145,  3,  0,  145,  4,  0,  134,  0,  134,  0,  135,  0,  100,  0,  255,  255,  141,  0,  100,  0,  255,  255,  134,  1,  134,  1,  135,  1,  100,  0,  255,  255,  141,  0,  100,  0,  255,  255,  145,  0,  127,  145,  1,  127,  145,  3,  127,  145,  4,  127,  142,  31,  35,  0,  0,  40,  229,  255,  255,  200,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  230,  195,  97,  0,  24,  136,  1,  10,  1,  3,  136,  1,  4,  1,  255,  142,  30,  35,  0,  0,  39,  229,  255,  255,  200,  0,  0,  0,  0,  0,  0,  0,  127,  254,  255,  255,  0,  62,  73,  0,  24,  140,  30,  35,  0,  0,  40,  229,  255,  255,  200,  0,  0,  0,  0,  0,  0,  0,  129,  1,  0,  0,  134,  0,  142,  49,  224,  255,  255,  184,  233,  255,  255,  104,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  34,  156,  50,  0,  24,  142,  49,  224,  255,  255,  183,  233,  255,  255,  104,  1,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  28,  88,  0,  0,  24,  137,  136,  142,  49,  224,  255,  255,  183,  233,  255,  255,  240,  0,  0,  0,  237,  255,  255,  255,  0,  0,  0,  0,  96,  24,  0,  0,  24,  142,  49,  224,  255,  255,  183,  233,  255,  255,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  242,  43,  0,  0,  24,  142,  123,  209,  255,  255,  183,  233,  255,  255,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  134,  47,  48,  0,  24,  142,  49,  224,  255,  255,  110,  237,  255,  255,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  18,  114,  11,  0,  24,  142,  49,  224,  255,  255,  110,  237,  255,  255,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  41,  213,  255,  255,  110,  237,  255,  255,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  48,  224,  255,  255,  37,  241,  255,  255,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  49,  224,  255,  255,  36,  241,  255,  255,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  41,  213,  255,  255,  37,  241,  255,  255,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  49,  224,  255,  255,  220,  244,  255,  255,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  49,  224,  255,  255,  219,  244,  255,  255,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  242,  43,  0,  0,  24,  142,  41,  213,  255,  255,  220,  244,  255,  255,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  49,  224,  255,  255,  146,  248,  255,  255,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  49,  224,  255,  255,  147,  248,  255,  255,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  242,  43,  0,  0,  24,  142,  40,  213,  255,  255,  146,  248,  255,  255,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  49,  224,  255,  255,  73,  252,  255,  255,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  49,  224,  255,  255,  73,  252,  255,  255,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  242,  43,  0,  0,  24,  142,  41,  213,  255,  255,  74,  252,  255,  255,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  49,  224,  255,  255,  0,  0,  0,  0,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  49,  224,  255,  255,  0,  0,  0,  0,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  242,  43,  0,  0,  24,  142,  40,  213,  255,  255,  0,  0,  0,  0,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  49,  224,  255,  255,  182,  3,  0,  0,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  49,  224,  255,  255,  183,  3,  0,  0,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  242,  43,  0,  0,  24,  142,  41,  213,  255,  255,  183,  3,  0,  0,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  49,  224,  255,  255,  110,  7,  0,  0,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  49,  224,  255,  255,  109,  7,  0,  0,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  242,  43,  0,  0,  24,  142,  40,  213,  255,  255,  110,  7,  0,  0,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  49,  224,  255,  255,  36,  11,  0,  0,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  49,  224,  255,  255,  37,  11,  0,  0,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  41,  213,  255,  255,  36,  11,  0,  0,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  49,  224,  255,  255,  219,  14,  0,  0,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  49,  224,  255,  255,  220,  14,  0,  0,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  41,  213,  255,  255,  219,  14,  0,  0,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  48,  224,  255,  255,  146,  18,  0,  0,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  49,  224,  255,  255,  146,  18,  0,  0,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  41,  213,  255,  255,  146,  18,  0,  0,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  49,  224,  255,  255,  73,  22,  0,  0,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  49,  224,  255,  255,  73,  22,  0,  0,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  41,  213,  255,  255,  73,  22,  0,  0,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  174,  238,  255,  255,  165,  248,  255,  255,  104,  1,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  24,  130,  29,  0,  24,  142,  174,  238,  255,  255,  165,  248,  255,  255,  240,  0,  0,  0,  208,  255,  255,  255,  0,  0,  0,  0,  174,  37,  0,  0,  24,  142,  174,  238,  255,  255,  165,  248,  255,  255,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  174,  238,  255,  255,  91,  7,  0,  0,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  134,  47,  48,  0,  24,  142,  101,  242,  255,  255,  165,  248,  255,  255,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  18,  114,  11,  0,  24,  142,  100,  242,  255,  255,  165,  248,  255,  255,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  101,  242,  255,  255,  174,  3,  0,  0,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  28,  246,  255,  255,  165,  248,  255,  255,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  28,  246,  255,  255,  165,  248,  255,  255,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  27,  246,  255,  255,  174,  3,  0,  0,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  211,  249,  255,  255,  165,  248,  255,  255,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  210,  249,  255,  255,  165,  248,  255,  255,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  211,  249,  255,  255,  174,  3,  0,  0,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  137,  253,  255,  255,  165,  248,  255,  255,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  137,  253,  255,  255,  165,  248,  255,  255,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  137,  253,  255,  255,  173,  3,  0,  0,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  64,  1,  0,  0,  166,  248,  255,  255,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  64,  1,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  228,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  65,  1,  0,  0,  173,  3,  0,  0,  240,  0,  0,  0,  155,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  246,  4,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  247,  4,  0,  0,  166,  248,  255,  255,  240,  0,  0,  0,  228,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  247,  4,  0,  0,  173,  3,  0,  0,  240,  0,  0,  0,  155,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  174,  8,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  174,  8,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  228,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  173,  8,  0,  0,  174,  3,  0,  0,  240,  0,  0,  0,  155,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  101,  12,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  100,  12,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  228,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  101,  12,  0,  0,  174,  3,  0,  0,  240,  0,  0,  0,  155,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  27,  16,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  49,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  27,  16,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  28,  16,  0,  0,  173,  3,  0,  0,  240,  0,  0,  0,  155,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  210,  19,  0,  0,  166,  248,  255,  255,  240,  0,  0,  0,  49,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  210,  19,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  210,  19,  0,  0,  173,  3,  0,  0,  240,  0,  0,  0,  155,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  137,  23,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  49,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  137,  23,  0,  0,  166,  248,  255,  255,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  137,  23,  0,  0,  173,  3,  0,  0,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  64,  27,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  49,  200,  8,  0,  24,  142,  64,  27,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  227,  255,  255,  255,  0,  0,  0,  0,  241,  43,  0,  0,  24,  142,  63,  27,  0,  0,  174,  3,  0,  0,  240,  0,  0,  0,  156,  254,  255,  255,  0,  0,  0,  0,  165,  35,  36,  0,  24,  142,  64,  27,  0,  0,  173,  3,  0,  0,  240,  0,  0,  0,  48,  0,  0,  0,  0,  0,  0,  0,  217,  146,  0,  0,  24,  137,  8,  134,  1,  137,  24,  140,  64,  27,  0,  0,  173,  3,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  129,  1,  0,  0,  142,  84,  236,  255,  255,  240,  233,  255,  255,  104,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  163,  96,  40,  0,  24,  142,  83,  236,  255,  255,  239,  233,  255,  255,  104,  1,  0,  0,  0,  0,  0,  0,  100,  1,  0,  0,  179,  62,  4,  0,  24,  137,  144,  142,  84,  236,  255,  255,  240,  233,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  237,  255,  255,  255,  96,  24,  0,  0,  24,  142,  83,  236,  255,  255,  240,  233,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  242,  43,  0,  0,  24,  142,  92,  247,  255,  255,  239,  233,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  83,  236,  255,  255,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  84,  236,  255,  255,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  241,  43,  0,  0,  24,  142,  92,  247,  255,  255,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  83,  236,  255,  255,  75,  241,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  84,  236,  255,  255,  74,  241,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  241,  43,  0,  0,  24,  142,  91,  247,  255,  255,  75,  241,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  84,  236,  255,  255,  247,  244,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  83,  236,  255,  255,  248,  244,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  242,  43,  0,  0,  24,  142,  92,  247,  255,  255,  248,  244,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  84,  236,  255,  255,  165,  248,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  83,  236,  255,  255,  165,  248,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  242,  43,  0,  0,  24,  142,  92,  247,  255,  255,  166,  248,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  83,  236,  255,  255,  82,  252,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  84,  236,  255,  255,  83,  252,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  242,  43,  0,  0,  24,  142,  92,  247,  255,  255,  82,  252,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  83,  236,  255,  255,  0,  0,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  84,  236,  255,  255,  0,  0,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  242,  43,  0,  0,  24,  142,  91,  247,  255,  255,  0,  0,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  84,  236,  255,  255,  174,  3,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  83,  236,  255,  255,  173,  3,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  242,  43,  0,  0,  24,  142,  92,  247,  255,  255,  174,  3,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  84,  236,  255,  255,  90,  7,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  83,  236,  255,  255,  91,  7,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  242,  43,  0,  0,  24,  142,  92,  247,  255,  255,  91,  7,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  84,  236,  255,  255,  8,  11,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  83,  236,  255,  255,  8,  11,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  241,  43,  0,  0,  24,  142,  92,  247,  255,  255,  9,  11,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  83,  236,  255,  255,  181,  14,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  84,  236,  255,  255,  182,  14,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  241,  43,  0,  0,  24,  142,  92,  247,  255,  255,  181,  14,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  83,  236,  255,  255,  99,  18,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  84,  236,  255,  255,  99,  18,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  241,  43,  0,  0,  24,  142,  91,  247,  255,  255,  99,  18,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  84,  236,  255,  255,  17,  22,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  83,  236,  255,  255,  16,  22,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  241,  43,  0,  0,  24,  142,  92,  247,  255,  255,  16,  22,  0,  0,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  9,  251,  255,  255,  157,  237,  255,  255,  104,  1,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  190,  164,  30,  0,  24,  142,  9,  251,  255,  255,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  208,  255,  255,  255,  174,  37,  0,  0,  24,  142,  9,  251,  255,  255,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  241,  43,  0,  0,  24,  142,  10,  251,  255,  255,  166,  248,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  182,  254,  255,  255,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  183,  254,  255,  255,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  241,  43,  0,  0,  24,  142,  182,  254,  255,  255,  165,  248,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  100,  2,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  100,  2,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  241,  43,  0,  0,  24,  142,  100,  2,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  17,  6,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  17,  6,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  241,  43,  0,  0,  24,  142,  18,  6,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  190,  9,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  191,  9,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  241,  43,  0,  0,  24,  142,  191,  9,  0,  0,  166,  248,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  108,  13,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  108,  13,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  228,  255,  255,  255,  241,  43,  0,  0,  24,  142,  108,  13,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  155,  254,  255,  255,  165,  35,  36,  0,  24,  142,  26,  17,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  25,  17,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  228,  255,  255,  255,  241,  43,  0,  0,  24,  142,  26,  17,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  155,  254,  255,  255,  165,  35,  36,  0,  24,  142,  198,  20,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  199,  20,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  228,  255,  255,  255,  241,  43,  0,  0,  24,  142,  199,  20,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  155,  254,  255,  255,  165,  35,  36,  0,  24,  142,  117,  24,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  116,  24,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  228,  255,  255,  255,  241,  43,  0,  0,  24,  142,  116,  24,  0,  0,  166,  248,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  155,  254,  255,  255,  165,  35,  36,  0,  24,  142,  34,  28,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  49,  0,  0,  0,  239,  197,  8,  0,  24,  142,  34,  28,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  241,  43,  0,  0,  24,  142,  33,  28,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  155,  254,  255,  255,  165,  35,  36,  0,  24,  142,  207,  31,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  49,  0,  0,  0,  239,  197,  8,  0,  24,  142,  207,  31,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  241,  43,  0,  0,  24,  142,  207,  31,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  155,  254,  255,  255,  165,  35,  36,  0,  24,  142,  125,  35,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  49,  0,  0,  0,  239,  197,  8,  0,  24,  142,  124,  35,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  241,  43,  0,  0,  24,  142,  125,  35,  0,  0,  165,  248,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  142,  42,  39,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  48,  0,  0,  0,  239,  197,  8,  0,  24,  142,  42,  39,  0,  0,  157,  237,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  227,  255,  255,  255,  241,  43,  0,  0,  24,  142,  42,  39,  0,  0,  166,  248,  255,  255,  240,  0,  0,  0,  0,  0,  0,  0,  156,  254,  255,  255,  165,  35,  36,  0,  24,  137,  16,  142,  41,  39,  0,  0,  165,  248,  255,  255,  48,  242,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  54,  248,  100,  0,  24,  136,  1,  10,  1,  2,  136,  1,  4,  1,  255,  137,  31,  134,  0,  136,  0,  3,  2,  0,  0,  134,  1,  136,  1,  3,  2,  0,  0,  134,  0,  136,  0,  31,  2,  0,  0,  136,  1,  31,  2,  0,  0,  137,  31 };

LEVEL_PLATE_DUAL
LEVEL_PLATE_SINGLE

 namespace utility {
	 
	 volatile bool is_playing;
	 int build_index = 0;
	 int build_length = 0;
	 uint8_t * buildFile;
	  
 /// returns true if script is running
 bool isPlaying(){
	 
	 return is_playing;		 
 }
 void reset(){
	 uint16_t build_index = 0;
	 uint16_t build_length = 0;
	 is_playing = false;
 
 }
 
 /// returns true if more bytes are available in the script
 bool playbackHasNext(){
	return (build_index < build_length);
 }
 
 /// gets next byte in script
 uint8_t playbackNext(){
	 
	 uint8_t byte;
	 
	 if(build_index < build_length)
	 {
		 byte = pgm_read_byte(buildFile + build_index++);
		return byte;
	}

	else 
		return 0;
 }
 
 /// begin buffer playback
 bool startPlayback(uint8_t build){
	 
	 
	 is_playing = true;
	 build_index = 0;

     // get build file
	switch (build){
        case HOME_AXES:
			buildFile = HomeAxes;		
			break;
		case LEVEL_PLATE_SECOND:
			if(eeprom::isSingleTool()){
				buildFile = LevelPlateSingle;
			} else{
				buildFile = LevelPlateDual; 
			}
			build = LEVEL_PLATE_STARTUP;
			getSecondLevelOffset();
			break;
		case LEVEL_PLATE_STARTUP:
			if(eeprom::isSingleTool()){
				buildFile = LevelPlateSingle;
			} else{
				buildFile = LevelPlateDual; 
			}
			build = LEVEL_PLATE_STARTUP;
			break;
		case TOOLHEAD_CALIBRATE:
			buildFile = NozzleCalibrate;
			break;
		default:
			return false;
	}
	
     // get build length
	 build_length = pgm_read_word(Lengths + build);
	  
	 return is_playing;
 }
     
 void getSecondLevelOffset(){
	 // find the homing command (after all the intro text)
	 uint8_t messageCount = 0;
	 while(messageCount < 5){
		 while(pgm_read_byte(buildFile + build_index) != 149)
			build_index++;
		build_index++;
		messageCount++;
	 }
	 build_index--;
	
 }
 
 /// updates state to finished playback
 void finishPlayback(){
	is_playing = false;
	
 }

};
