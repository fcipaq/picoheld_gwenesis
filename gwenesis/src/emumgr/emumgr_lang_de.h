/*
 * emumgr - emulation manager for GWENESIS on the Pico Held
 *
 * Copyright (C) 2023 Daniel Kammer (daniel.kammer@web.de)
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
 
#ifndef EMUMGR_LANG_DE_H
#define EMUMGR_LANG_DE_H

// Escape Code für Umlaute
// ä e4, Ä c4
// Ü dc, ü fc
// Ö d6, ö f6
// ß df

#define STR_MAIN_TITLE "Pico Held Gwenesis"
#define STR_OPTIONS_MENU "Einstellungen"
#define STR_ROM_SELECTOR_CAPTION "ROM Auswahl"

#define STR_SOUND_ACCURACY "Ton Genauigkeit"
#define STR_HIGH "hoch"
#define STR_LOW "niedrig"
#define STR_ERROR "Fehler"
#define STR_ENABLED "aktiviert"
#define STR_DISABLED "deaktiviert"

#define STR_ROM_TOO_BIG "ROM ist zu gro\xdf"
#define STR_READ_ROM_DIR "ROM-Ordner nicht lesbar:"
#define STR_NO_ROMS "Keine ROMs gefunden in"
#define STR_FLASHING_ROM "Laden wird vorbereitet"
#define STR_LOAD_ROM "Ladevorgang starten"
#define STR_READING_ROM "beim Lesen der SD-Karte"

#define STR_READING_SAV "beim Lesen des Spielstands"
#define STR_WRITING_SAV "beim Schreiben des Spielstands"
#define STR_WRITING_DIR "beim Erstellen des Verzeichnisses"

#define STR_PRESS_LEFT "Nach LINKS dr\xfc""cken"
#define STR_RETURN "zum Abbrechen"
#define STR_LOADING "Laden..."
#define STR_GO_BACK "Zur\xfc""ck"

#define STR_OFF "Aus"
#define STR_YES "JA"
#define STR_NO "NEIN"
#define STR_CONFIRM_OVERWRITE "\xdc""berschreiben best\xe4tigen"
#define STR_CONFIRM_BL "Bootloader starten best\xe4tigen"

#define STR_GAMESTATE "Spielstand"
#define STR_GAMESTATE_NS "NICHT gesichert"
#define STR_SLOT_1 "Speicherplatz 1"
#define STR_SLOT_2 "Speicherplatz 2"
#define STR_SLOT_3 "Speicherplatz 3"
#define STR_SLOT_4 "Speicherplatz 4"
#define STR_SLOT_5 "Speicherplatz 5"
#define STR_SLOT_6 "Speicherplatz 6"
#define STR_SAVE "Spielstand sichern"
#define STR_SAVE_STATE "Spielstand sichern"
#define STR_SAVING_GAME_STATE "Sichere Spielstand"
#define STR_PLEASE_WAIT "Bitte warten..."
#define STR_LOAD "Spielstand laden"
#define STR_LOAD_STATE "Spielstand laden"
#define STR_SEL_ROM "Spiel w\xe4hlen"
#define STR_SOUND "Ton"
#define STR_BRIGHTNESS "Helligkeit"
#define STR_OPTIONS "Einstellungen"
#define STR_LAUNCH "Spiel starten"

#define STR_100 "100 %"
#define STR_80 "80 %"
#define STR_75 "75 %"
#define STR_60 "60 %"
#define STR_50 "50 %"
#define STR_40 "40 %"
#define STR_25 "25 %"
#define STR_20 "20 %"

#define STR_BTN_MENU "Knopfreihenfolge"
#define STR_BTN_ABC "A-B-C"
#define STR_BTN_ACB "A-C-B"
#define STR_BTN_BAC "B-A-C"
#define STR_BTN_BCA "B-C-A"
#define STR_BTN_CAB "C-A-B"
#define STR_BTN_CBA "C-B-A"

#define STR_BUTTONS "Kn\xf6pfe"
#define STR_TOGGLE_SND_ACC "Genauigkeit umschalten"
#define STR_TOGGLE_FPS "FPS-Anzeige umschalten"
#define STR_FPS "Zeige FPS"
#define STR_TOGGLE_Z80 "Z80 erwzingen (Experte)"
#define STR_Z80 "Z80 erzwingen"


#define STR_LAUNCH_BL "Bootloader starten"
#endif // EMUMGR_LANG_DE_H