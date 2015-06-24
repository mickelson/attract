#
# General
#
Exit Attract-Mode;Quitter Attract-Mode
Exit Attract-Mode?;Quitter Attract-Mode?
Yes;Oui
No;Non

#
# Emulator Settings Configuration
#
Edit Artwork;Modifier l'Artwork
Artwork Label;Label de l'Artwork
Artwork Path;Chemin de l'Artwork
Edit Emulator;Modifier l'émulateur
Emulator Name;Nom de l'émulateur
Executable;Exécutable
Command Arguments;Ligne de commande
Rom Path;Chemin des ROM
Rom Extension(s);Extension(s) des ROM
Additional Import File(s);Fichiers additionnels
XML Mode;Mode XML
Add Artwork;Ajouter Artwork
Generate Collection/Romlist;Générer la liste de ROM
Delete this Emulator;Supprimer cet émulateur
Generating Rom List: $1%;Génération de la liste de ROM: $1%
Rom path '$1' not found, proceed anyways?;Chemin des ROM '$1' introuvable, continuer?
Overwrite existing '$1' romlist?;Ecraser la liste de ROM '$1' existante?
Wrote $1 entries to romlist;Création de $1 entrées dans la liste de ROM
Delete emulator '$1'?;Supprimer l'émulateur '$1'?
Config / Emulators;Configuration / Emulateurs
Add Emulator;Ajouter un émulateur
Enter Emulator Name;Nom de l'émulateur?
Configure available emulators;Configurer les émulateurs disponibles

_help_art_label;Label identifiant cette ressource d'Artwork (utilisé par les thèmes)
_help_art_path;Répertoire contenant les Artworks
_help_art_add;Ajouter une nouvelle ressource à l'Artwork (marquee, capture d'écran, etc.)
_help_emu_executable;Chemin et nom de l'exécutable de l'émulateur. $HOME sera remplacé par le répertoire de l'utilisateur
_help_emu_args;Arguments de la ligne de commande de cet émulateur. Options de substitution: [name], [romext], [rompath], [emulator], [romfilename]
_help_emu_rompath;Chemin des ROM pour cet émulateur
_help_emu_romext;Extension des ROM pour cet émulateur (ex .zip).  Possibilité d'utiliser des extensions multiples séparées par un point-virgule
_help_emu_import_extras;Fichiers additionels nécessitant d'être lus lors de la génération de la liste de ROM pour cet émulateur. Fichiers supportés: catver.ini (catégories) et nplayers.ini (nombre de joueur). Plusieurs fichiers peuvent être saisis si séparés par un point-virgule
_help_emu_listxml;Les valeurs valides pour le mode XML sont "mame" (si c'est une version récente de mame proposant l'option -listxml) ou "mess <system>" (si c'est une version récente de mess proposant l'option -listsoftware). Pour tout autre émulateur laisser vide
_help_emu_gen_romlist;Génére une liste de ROM en cherchant dans le chemin des ROM avec l'extension choisie, incorporant l'XML et les fichiers additionnels d'information dans la liste générée (si disponible)
_help_emu_delete;Supprime l'entrée pour cet émulateur
_help_emu_sel;Configuration de l'émulateur
_help_emu_add;Ajouter un nouvel émulateur

#
# Filter Settings Configuration
#
Enter Filter Name;Entrer le nom du filtre
Rule Edit;Editer les règles
Target;Cible
Comparison;Comparaison
Filter Value;Filtre de valeur
Delete this Rule;Supprimer cette règle
Filter Edit;Edition du filtre
Filter Name;Nom du filtre
Rule;Règle
Add Rule;Ajouter une règle
Delete this Filter;Supprimer ce filtre
Delete filter '$1'?;Supprimer le filtre '$1'?
All;Tous
Title;Titre
Emulator;Emulateur
CloneOf;Clone de
Year;Année
Manufacturer;Fabriquant
Category;Catégorie
Players;Joueurs
Rotation;Rotation
Control;Contrôle
Status;Etat
DisplayType;Afficher type
DisplayCount;Afficher compteur
Favourite;Favoris
equals;est égal à
does not equal;n'est pas égal à
contains;contient
does not contain;ne contient pas
The name for this filter;Nom du filtre
Configure filter rule;Configuration de la règle de filtrage
Add a rule to this filter;Ajouter une règle à ce filtre
Delete this filter;Supprimer ce filtre

_help_rule_target;L'attribut du jeu que vous souhaitez filtrer sur la base de
_help_rule_comp;Règle de comparaison pour le filtre
_help_rule_value;Valeur à filtrer.  Par exemple, si la cible de filtrage est "Année" et le comparateur "est égale à", une valeur de "1984" correspondra à l'année 1984, "198." correspondra à n'importe quelle année des années 1980, et "(198.)|(199.)" correspondra aux années comprises entre 1980 et 1999
_help_rule_delete;Supprimer cette règle de filtrage

#
# Display Settings Configuration
#
Display Edit;Edition de la liste
Display Name;Nom de la liste
Layout;Thème
Collection/Rom List;Liste de ROM
Filter;Filtre
Add Filter;Ajouter un filtre
Delete this Display;Supprimer cette liste
Delete list '$1'?;Supprimer la liste '$1'?
Configure / Displays;Configuration / Listes
Add New Display;Ajouter une nouvelle liste
Enter Display Name;Entrer le nom de la liste

_help_display_name;Titre de cette liste d'affichage
_help_display_layout;Thème à utiliser pour cette liste d'affichage
_help_display_romlist;Liste de ROM à utiliser pour cette liste d'affichage
_help_display_filter;Configurer le filtre
_help_display_add_filter;Ajouter un filtre à cette liste d'affichage
_help_display_delete;Suppression de la liste d'affichage
_help_display_sel;Configurer la liste d'affichage
_help_display_add;Ajouter une nouvelle liste d'affichage

#
# Input Settings Configuration
#
Edit Input;Editer le contrôle
Action;Action
Remove Input;Enlever le contrôle
Add Input;Ajouter un contrôle
Press Input;Appuyer sur le contrôle
This will overwrite an existing mapping ($1).  Proceed?;Ecraser le contrôle existant ($1). Confirmer?
Configure / Input;Configuration / Contrôles
OR;OU
Select;Select
Up;Haut
Down;Bas
Page Up;Page Suivante
Page Down;Page Précédente
Previous List;Liste précédent
Next List;Liste suivante
Lists Menu;Menu des listes
Previous Filter;Filtre précédent
Next Filter;Filtre suivant
Filters Menu;Menu des filtres
Toggle Layout;Basculer le thème
Toggle Movie;Basculer la vidéo
Toggle Mute;Basculer en mode silence
Toggle Rotate Right;Basculer Rotation droite
Toggle Flip;Basculer mode retourné
Toggle Rotate Left;Basculer Rotation gauche
Exit (Confirm);Sortie (Confirmer)
Exit to Desktop;Sortie vers le bureau
Screenshot;Photo écran
Configure;Configurer
Random Game;Jeu au hasard
Add/Remove Favourite;Ajouter/Supprimer Favoris
Next Favourite;Favoris suivant
Previous Favourite;Favoris précédent
Replay Last Game;Rejouer au dernier jeu
Ambient Sounds;Son d'ambiance
Startup Sound;Son au démarrage
Game Return Sound;Son au retour du jeu
Mouse Threshold;Seuil de la souris
Joystick Threshold;Seuil du joystick

_help_input_delete;Enlever ce contrôle
_help_input_add;Ajouter un nouveau contrôle
_help_control_sel;Configurer les contrôles
_help_joystick_threshold;Seuil d'amplitude des mouvements du joystick, de 1 à 100 (100=moins sensible)
_help_mouse_threshold;Seuil d'amplitude des mouvements de la souris, de 1 à 100 (100=moins sensible)

#
# Sound Settings Configuration
#
Configure / Sound;Configuration / Son
Sound Volume;Volume général
Ambient Volume;Volume des sons d'ambiances
Movie Volume;Volume des vidéos
_help_volume;Les valeurs possibles pour le volume vont de 0 (silence) à 100
_help_sound_sel;Assigner un son à cet événement/action

#
# Miscellaneous Settings Configuration
#
Configure / Miscellaneous;Configuration / Divers
Hide Brackets in Game Title;Cacher les crochets dans le titre du jeu
Launch Last Game on Startup;Lancer le dernier jeu au démarrage
Confirm Favourites;Confirmer les favoris
Layout Options;Options du Layout

None;Non
Right;Droite
Flip;Retourné
Left;Gauche
Exit Command;Commande de sortie
Default Font;Police par défaut
Font Path;Chemin de la police
Allow Exit from 'Lists Menu';Permettre la sortie des listes de menu 
Language;Langage
en;Anglais
fr;Français
es;Español

_help_exit_command;Commande à exécuter en quittant (Confirmer) [exemple "sudo shutdown"]
_help_default_font;Le nom de la police par défaut à utiliser
_help_font_path;Répertoires contenant les polices (séparés par des points virgules)
_help_displays_menu_exit;Indiquer si une option de sortie doit être prévue dans le menu des listes
_help_language;Sélectionner le langage
_help_display_layout_options;Configurer une option spécifique du Layout
_help_hide_brackets;Cacher le texte entre crochets lors de l'affichage du titre du jeu
_help_autolaunch_last_game;Lancer automatiquement au démarrage le dernier jeu joué
_help_confirm_favs;Confirmer l'addition/suppression des favoris

Configure Screen Saver;Configurer l'économiseur d'écran
Screen Saver Timeout;Décompte économiseur d'écran
_help_screen_saver_timeout;Temps total en seconde à attendre pour activer l'économiseur d'écran.  0 pour désactiver

#
# Plug-in Configuration
#
Configure Plug-in;Configurer Plug-in
Name;Nom
Command;Commandes
Enabled;Activé
Configure / Plug-ins;Configuration / Plug-in

_help_plugin_command;Exécutable associé à ce plug-in
_help_plugin_enabled;Indiquer si ce plug-in est activé
_help_plugin_sel;Configurer le plug-in

#
# Configuration Main Menu
#
Configure;Configuration
Emulators;Emulateurs
Lists;Listes
Controls;Contrôles
Sound;Son
Screen Saver;Economiseur d'écran
Plug-ins;Plug-in
General;Général
Back;Retour

_help_emulators;Configurer les émulateurs
_help_displays;Configurer les listes d'affichage
_help_controls;Configurer les contrôles
_help_screen_saver;Configurer l'économiseur d'écran
_help_sound;Configurer le volume et les affectations sonores
_help_plugins;Configurer les plug-ins
_help_misc;Configurations diverses
Exit menu;Quitter le menu

_sort_regexp;^(Vs\. The |The |Vs\. )

#
# Strings needing translation:
#
System Identifier;System Identifier
Info Source/Scraper;Info Source/Scraper
Minimum Run Time;Minimum Run Time
Exit Hotkey;Exit Hotkey
Sort By;Sort By
No Sort;No Sort
AltRomname;AltRomname
AltTitle;AltTitle
Tags;Tags
PlayedCount;PlayedCount
PlayedTime;PlayedTime
List Limit;List Limit
Add/Remove Tags;Add/Remove Tags
Previous Letter;Previous Letter
Next Letter;Next Letter
Custom1;Custom1
Custom2;Custom2
Custom3;Custom3
Custom4;Custom4
Custom5;Custom5
Custom6;Custom6
Track Usage;Track Usage
Filter Wrap Mode;Filter Wrap Mode
Wrap within List (Default);Wrap within List (Default)
Jump to Next List;Jump to Next List
No Wrap;No Wrap
Window Mode;Window Mode
Fill Screen (Default);Fill Screen (Default)
Fullscreen Mode;Fullscreen Mode
Window;Window
Global Filter;Global Filter
Disabled;Disabled

_help_emu_system;The system identifier(s) for this emulator.  Multiple values can be entered if separated by a semicolon
_help_emu_info_source;Use "mame" if the emulator provides mame -listxml output, "mess" for mess -listsoftware output, "thegamesdb.net" to scrape data from thegamesdb.net, or "steam" to read from a steam installation.  "mess" and "thegamesdb.net" also require that a System Identifier recognized by those sources be configured
_help_emu_minimum_run_time;Set a minimum run time (in seconds).  Attract-Mode will wait after running this emulator until the minimum time has expired and it becomes the foreground window.  This needs to be set for Steam to work correctly on Windows.  Set this to 0 unless you know you need it
_help_emu_exit_hotkey;Set a global hotkey to exit the emulator.  Attract-Mode will terminate the emulator process if this hotkey gets pressed
_help_filter_sort_by;The game information attribute that you wish to sort this list by.  Set to "No Sort" to keep the ordering from the source romlist
_help_filter_list_limit;Set this to a non-zero value if you wish to limit the size of a sorted list.  Use a positive value for the first X entries (i.e. "10" gives the first 10 entries), a negative value for the last X entries (i.e. "-25" gives the last 25 entries), or "0" for no limit
_help_window_mode;Set whether Attract-Mode fills the screen or runs in a window
_help_filter_wrap_mode;Set what happens when the "Next Filter" or "Previous Filter" actions reach the end of the filter list
_help_track_usage;Configure whether Attract-Mode should track usage (played time and play count for each game)
_help_filter_name;The name for this filter
_help_filter_rule;Configure filter rule
_help_filter_add_rule;Add a rule to this filter
_help_filter_delete;Delete this filter
_help_display_global_filter;Configure global filter rules that are applied to every entry in this display's collection/rom list
_help_control_select;Set the control(s) that launch the current selection
_help_control_up;Set the control(s) that move the selection up
_help_control_down;Set the control(s) that move the selection down
_help_control_page_up;Set the control(s) that move the selection up one page
_help_control_page_down;Set the control(s) that move the selection down one page
_help_control_prev_display;Set the control(s) that move to the previous display
_help_control_next_display;Set the control(s) that move to the next display
_help_control_displays_menu;Set the control(s) that show the "Displays Menu"
_help_control_prev_filter;Set the control(s) that move to the previous filter
_help_control_next_filter;Set the control(s) that move to the next filter
_help_control_filters_menu;Set the control(s) that show the "Filters Menu"
_help_control_toggle_layout;Set the control(s) that toggle the layout script to use (It depends on the layout you are using whether this does anything)
_help_control_toggle_movie;Set the control(s) that toggle whether or not videos should be played
_help_control_toggle_mute;Set the control(s) that mute/unmute frontend sound
_help_control_toggle_rotate_right;Set the control(s) that toggle whether the display should be rotated 90 degrees to the right
_help_control_toggle_flip;Set the control(s) that toggle whether the display should be rotated 100 degrees
_help_control_toggle_rotate_left;Set the control(s) that toggle whether the display should be rotated 90 degrees to the left
_help_control_exit;Set the control(s) that exit the frontend
_help_control_exit_no_menu;Set the control(s) that exit the frontend immediately to the desktop
_help_control_screenshot;Set the control(s) that take a screenshot of the frontend
_help_control_configure;Set the control(s) that show the "Configuration Menu"
_help_control_random_game;Set the control(s) that select a random game from the current list
_help_control_replay_last_game;Set the control(s) that relaunch the last game played
_help_control_add_favourite;Set the control(s) that mark/unmark the currently selected game as a favourite
_help_control_prev_favourite;Set the control(s) that move to the previous favourite
_help_control_next_favourite;Set the control(s) that move to the next favourite
_help_control_add_tags;Set the control(s) to add/remove tags from the currently selected game
_help_control_screen_saver;Set the control(s) that launch screen saver mode
_help_control_prev_letter;Set the control(s) that move to the previous letter
_help_control_next_letter;Set the control(s) that move to the next letter
_help_control_custom1;Set the "Custom1" control(s)
_help_control_custom2;Set the "Custom2" control(s)

# Scraper stuff
Scraper;Scraper
Scrape Artwork;Scrape Artwork
Configure / Scraper;Configure / Scraper
Scrape Snaps;Scrape Snaps
Scrape Marquees;Scrape Marquees
Scrape Flyers/Boxart;Scrape Flyers/Boxart
Scrape Game Logos (Wheel Art);Scrape Game Logos (Wheel Art)
Scraping Artwork: $1%;Scraping Artwork: $1%

_help_emu_scrape_artwork;Scrape missing artwork images for this emulator from TheGamesDB.net and mamedb.com
_help_scraper;Configure artwork scraper settings
_help_scrape_snaps;Set whether game snapshots should be downloaded when scraping
_help_scrape_marquees;Set whether game marquees and banners should be downloaded when scraping
_help_scrape_flyers;Set whether game flyers and box covers should be downloaded when scraping
_help_scrape_wheels;Set whether game logos should be downloaded when scraping
_help_scrape_fanart;Set whether game fan art should be downloaded when scraping
_help_intro;Configure intro

