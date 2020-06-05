////////////////////////////////////////////////////////////////////////
//
// Attract-Mode Frontend - Emulator detection script
//
// Copyright (c) 2017 Andrew Mickelson
//
// This program comes with ABSOLUTELY NO WARRANTY.  It is licensed under
// the terms of the GNU General Public License, version 3 or later.
//
////////////////////////////////////////////////////////////////////////
//
// This file specifies the emulators that Attract-Mode knows about...
//
// The FE tries to detect these when it is first run, and creates an
// emulator config in <config>/emulators for each one that it finds.
//
// It also creates template configs to be selected in the config UI
// in <config>/emulators/templates
//
// Some considerations:
//    - Emulators with an "OS" specified are unique to the specified OS
//      (Linux, Windows, OSX, Android).
//    - "exe" gets generated automatically from the emulator "name" if
//      not specified here.
//    - emulators with a "custom" script specified have their own unique
//      init script (only mame w/ mame.nut for now)
//
////////////////////////////////////////////////////////////////////////
return [
		{
			"name"   : "atari800-5200",
			"exe"    : "atari800",
			"args"   : "-fullscreen -5200_rom \"[romfilename]\"",
			"system" : "Atari 5200",
			"source" : "thegamesdb.net",
		},
		{
			"name"   : "daphne",
			"args"   : "[name] vldp -framefile [rompath]../vldp/[name]/[name].txt -blank_searches -min_seek_delay 1000 -seek_frames_per_ms 20 -homedir [rompath].. -sound_buffer 2048 -fullscreen -fastboot -useoverlaysb 0",
			"exts"   : ".zip",
			"system" : "Arcade",
		},
		{
			"name"   : "dolphin-emu",
			"args"   : "-b -e \"[romfilename]\"",
			"exts"   : ".dol;.elf;.iso;.gcm;.wad;.wbfs;.gbz;.ciso",
			"system" : "Nintendo Wii",
			"source" : "thegamesdb.net",

			"Windows"  : {
				"path"    : "%PROGRAMFILES%/Dolphin/",
				"exe"     : "Dolphin",
			},
		},
		{
			"name"   : "dosbox",
			"args"   : "-conf -noconsole \"[romfilename]\"",
			"rompath": "$HOME/dosbox",
			"exts"   : ".conf",
			"system" : "PC",
			"source" : "thegamesdb.net"
		},
		{
			"name"   : "fbalpha",
			"exe"    : [ "fbalpha", "fba64" ],
			"args"   : "\"[romfilename]\" -r 640x480x32",
			"exts"   : ".zip",
			"system" : "Arcade",
			"source" : "thegamesdb.net"
		},
		{
			"name"   : "fs-uae",
			"args"   : "\"[romfilename]\" --fullscreen",
			"exts"   : ".fs-uae",
			"system" : "Amiga",
			"source" : "thegamesdb.net",
			"hotkey" : "Escape",
		},
		{
			"name"   : "fusion",
			"args"   : "\"[romfilename]\" -fullscreen",
			"exts"   : ".sms;.sg;.sc;.mv;.gg;.cue;.bin;.zip",
			"system" : "Sega Genesis",
			"source" : "thegamesdb.net"
		},
		{
			"name"   : "gens",
			"args"   : "--fs --quickexit \"[romfilename]\"",
			"exts"   : ".zip;.md",
			"system" : "Sega Genesis",
			"source" : "thegamesdb.net"
		},
		{
			"name"   : "mame",
			"custom" : "mame_init.nut", // MAME has its own setup script
		},
		{
			"name"   : "mednafen",
			"args"   : "\"[romfilename]\"",
			"exts"   : ".zip",
		},
		{
			"name"   : "mupen64plus",
			"exe"    : ["mupen64plus", "mupen64plus-ui-console"],
			"args"   : "--fullscreen \"[romfilename]\"",
			"exts"   : ".z64;.n64;v64",
			"system" : "Nintendo 64",
			"source" : "thegamesdb.net"
		},
		{
			"name"   : "nestopia",
			"args"   : "\"[romfilename]\"",
			"exts"   : ".nes;.unf;.unif;.fds;.zip;.rar;.7z",
			"system" : "Nintendo Entertainment System (NES)",
			"source" : "thegamesdb.net"
		},
		{
			"name"   : "pcsx",
			"args"   : "-nogui -cdfile \"[romfilename]\"",
			"exts"   : ".iso;.bin;.mdf;.img",
			"system" : "Sony Playstation",
			"source" : "thegamesdb.net"
		},
		{
			"name"   : "pcsx2",
			"exe"    : ["pcsx2", "PCSX2"],
			"args"   : "\"[romfilename]\" --fullscreen --nogui",
			"exts"   : ".iso;.bin;.mdf;.img",
			"system" : "Sony Playstation 2",
			"source" : "thegamesdb.net"
		},
		{
			"name"   : "scummvm",
			"args"   : "-f [name]",
			"rompath": "",
			"system" : "PC",
			"source" : "scummvm",

			"Windows"  : {
				"path"    : "%PROGRAMFILESx86%/ScummVM/",
			},
		},
		{
			"name"   : "snes9x",
			"args"   : "\"[romfilename]\"",
			"exts"   : ".zip;.7z;.bin;.fig;.mgd;.sfc;.smc;.swc",
			"system" : "Super Nintendo (SNES)",
			"source" : "thegamesdb.net"

			"Linux"  : {
				"exe" : [ "snes9x", "snes9x-gtk" ],
				},
		},
		{
			"name"   : "steam",
			"args"   : "-applaunch [name]",
			"exts"   : ".acf",
			"system" : "PC",
			"source" : "steam",
			"min_run": "5",

			"Linux"  : {
				"rompath" : "$HOME/.steam/steam/steamapps",
				},

			"Windows"  : {
				"path"    : "%PROGRAMFILES%/Steam/",
				"exe"     : "Steam",
				"rompath" : "%PROGRAMFILES%/Steam/SteamApps",
				},
		},
		{
			"name"	: "stella",
			"args"	: "-fullscreen 1 \"[romfilename]\"",
			"exts"	: ".zip,.gz,.a26,.bin,.rom",
			"system"	: "Atari 2600",
			"source" : "thegamesdb.net"
		},
		{
			"name"   : "supermodel",
			"args"   : "\"[romfilename]\" -fullscreen",
			"exts"	: ".zip",
			"system" : "Arcade",
			"source" : "thegamesdb.net"
		},
		{
			"name"   : "vice",
			"exe"    : "x64",
			"args"   : "-autostart \"[romfilename]\" +confirmexit -fullscreen -autostart-warp",
			"exts"   : ".crt;.d64;.g64;.tap;.x64;.t64;.prg;.vsf;.zip",
			"system" : "Commodore 64",
			"source" : "thegamesdb.net",
			"hotkey" : "Escape",
		},
		{
			"name"   : "virtualjaguar",
			"args"   : "\"[romfilename]\" -f",
			"romext" : ".zip",
			"system" : "Atari Jaguar",
			"source" : "thegamesdb.net",
			"hotkey" : "Escape",
		},
		{
			"name"   : "windows_games",
			"OS"     : "Windows",
			"exe"    : "cmd",
			"args"   : "/c \"[romfilename]\"",
			"romext" : ".bat;.exe;.com",
			"system" : "PC",
			"source" : "thegamesdb.net"
		},
		{
			"name"   : "zsnes",
			"args"   : "-m \"[romfilename]\"",
			"exts"   : ".zip;.bin;.fig;.mgd;.sfc;.smc;.swc",
			"system" : "Super Nintendo (SNES)",
			"source" : "thegamesdb.net"
		},
	];
