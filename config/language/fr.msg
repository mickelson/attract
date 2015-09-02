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
Select;Sélection
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



System Identifier;Identifiant du système
Info Source/Scraper;Info Source/Aspiration
Minimum Run Time;Temps minimum fonctionnement 
Exit Hotkey;Raccourci clavier pour sortir
Sort By;Classé par
No Sort;Non classé
AltRomname;Nom de rom alternatif
AltTitle;Titre alternatif
Tags;Mots clés
PlayedCount;Compteur de jeu
PlayedTime;Temps de jeu
List Limit;taille maximum de la liste
Add/Remove Tags;Ajouter/Supprimer des mots clés
Previous Letter;Lettre précédente 
Next Letter;Lettre suivante
Custom1;Custom1
Custom2;Custom2
Custom3;Custom3
Custom4;Custom4
Custom5;Custom5
Custom6;Custom6
Track Usage;Mode de comptage
Filter Wrap Mode;Mode de fin de liste de filtres
Wrap within List (Default);Revient au début de la liste de filtres (Défaut)
Jump to Next List;Bascule sur la liste de roms suivante 
No Wrap;Ne fait rien
Window Mode;Mode d'affichage
Fill Screen (Default);Rempli l'écran (Default)
Fullscreen Mode;Mode plein écran 
Window;Fenêtre
Global Filter;Filtre global
Disabled;Désactivé

_help_emu_system;Identifiant(s) système(s) pour cet émulateur. Plusieurs valeurs peuvent être saisies, séparées par un point virgule
_help_emu_info_source;Saisir "mame" si cet émulateur propose un fichier de type mame -listxml, "mess" pour les fichiers du type mess -listsoftware, "thegamesdb.net" pour extraire les informations du site thegamesdb.net, ou "steam" pour les lires depuis une installation de steam. Les sources "mess" et "thegamesdb.net" nécessitent que des identifiants systèmes spécifiques soient configurés pour fonctionner correctement
_help_emu_minimum_run_time;Définit un temps minimum de fonctionnement (en secondes). Attract-Mode va attendre, après avoir lancé l'émulateur, que ce temps minimum se soit écoulé avant de re-basculer au premier plan. Ceci est nécessaire pour que Steam fonctionne correctement sous Windows. Saisir 0 si vous n'en avez pas besoin
_help_emu_exit_hotkey;Définit un raccourci clavier pour sortir de l'émulateur. Attract-Mode arrêtera le process de l'émulateur lorsque ce raccourci clavier sera pressé.
_help_filter_sort_by;C'est l'information du jeu par lequel vous souhaitez classer votre liste. Sélectionner "Non classé" pour conserver l'ordre de la liste de rom source.
_help_filter_list_limit;Saisir un nombre différent de zéro pour limiter la taille de la liste triée. Un nombre positif remonte les X premières valeurs (ex : "10" donne les 10 premières valeurs), un nombre négatif remonte les X dernières valeurs (ex : "-25" donne les 25 dernières valeurs). "0" affiche toute la liste.
_help_window_mode;Définit si Attract Mode rempli l'écran, fonctionne en mode plein écran ou si il s'exécute dans une fenêtre.
_help_filter_wrap_mode;Définit l'action a mener lorsque "Filtre suivant" ou "Filtre précédent" arrivent à une  extrémité de la liste de filtre.
_help_track_usage;Active les compteurs d'Attract-Mode (Compteur de jeu et Temps de jeu)
_help_filter_name;Nom du filtre
_help_filter_rule;Configurer le filtre
_help_filter_add_rule;Ajouter une règle à ce filtre
_help_filter_delete;Supprimer ce filtre
_help_display_global_filter;Définit les règles du filtre global. Elles s'appliqueront à l'ensemble des valeurs de la liste de roms affichée.
_help_control_select;Définit le(s) contrôle(s) qui valide(nt) la valeur sélectionnée 
_help_control_up;Définit le(s) contrôle(s) qui incrémente(nt) la séléction d'une valeur
_help_control_down;Définit le(s) contrôle(s) qui décrémente(nt) la séléction d'une valeur
_help_control_page_up;Définit le(s) contrôle(s) qui incrémente(nt) la séléction d'une page
_help_control_page_down;Définit le(s) contrôle(s) qui décrémente(nt) la séléction d'une page
_help_control_prev_display;Définit le(s) contrôle(s) qui bascule(nt) à la liste précédente
_help_control_next_display;Définit le(s) contrôle(s) qui bascule(nt) à la liste suivante
_help_control_displays_menu;Définit le(s) contrôle(s) qui affiche(nt) la liste des listes 
_help_control_prev_filter;Définit le(s) contrôle(s) qui bascule(nt) au filtre précédente
_help_control_next_filter;Définit le(s) contrôle(s) qui bascule(nt) au filtre suivant
_help_control_filters_menu;Définit le(s) contrôle(s) qui affiche(nt) la liste des filtres 
_help_control_toggle_layout;Définit le(s) contrôle(s) qui  active(nt)/désactive(nt) le script du theme utilisé (Ceci dépend du theme que vous utilisez)
_help_control_toggle_movie;Définit le(s) contrôle(s) qui active(nt)/désactive(nt) la lecture des vidéos.  
_help_control_toggle_mute;Définit le(s) contrôle(s) qui active(nt)/désactive(nt) le son. 
_help_control_toggle_rotate_right;Définit le(s) contrôle(s) qui active(nt)/désactive(nt) la rotation de l'écran de 90 degrés sur la droite 
_help_control_toggle_flip;Définit le(s) contrôle(s) qui active(nt)/désactive(nt) la rotation de l'écran de 180 degrés
_help_control_toggle_rotate_left;Définit le(s) contrôle(s) qui active(nt)/désactive(nt) la rotation de l'écran de 90 degrés sur la gauche 
_help_control_exit;Définit le(s) contrôle(s) qui quitte(nt) Attract Mode 
_help_control_exit_no_menu;Définit le(s) contrôle(s) qui quitte(nt) immédiatement Attract Mode pour revenir au bureau
_help_control_screenshot;Définit le(s) contrôle(s) qui enregistre(nt) une impression écran d'Attract Mode
_help_control_configure;Définit le(s) contrôle(s) qui affiche(nt) le menu de configuration  
_help_control_random_game;Définit le(s) contrôle(s) qui sélectionne(nt) un jeu aléatoirement dans la liste de rom active
_help_control_replay_last_game;Définit le(s) contrôle(s) qui relance(nt) le dernier jeu joué
_help_control_add_favourite;Définit le(s) contrôle(s) qui permet(ent) d'ajouter/supprimer le jeu selectionné à la liste des favoris
_help_control_prev_favourite;Définit le(s) contrôle(s) qui selectionne(nt) le jeu favoris precedent
_help_control_next_favourite;Définit le(s) contrôle(s) qui selectionne(nt) le jeu favoris suivant
_help_control_add_tags;Définit le(s) contrôle(s) qui ajoute(nt)/supprime(nt) des mots clés au jeu sélectionné
_help_control_screen_saver;Définit le(s) contrôle(s) qui lance(nt) le mode "économiseur d'écran"
_help_control_prev_letter;Définit le(s) contrôle(s) qui bascule(nt) à la lettre précédente
_help_control_next_letter;Définit le(s) contrôle(s) qui bascule(nt) à la lettre suivante
_help_control_custom1;Définit le(s) contrôle(s) utilisateur(s) "Custom1"
_help_control_custom2;Définit le(s) contrôle(s) utilisateur(s) "Custom2"
_help_control_custom3;Définit le(s) contrôle(s) utilisateur(s) "Custom3"
_help_control_custom4;Définit le(s) contrôle(s) utilisateur(s) "Custom4"
_help_control_custom5;Définit le(s) contrôle(s) utilisateur(s) "Custom5"
_help_control_custom6;Définit le(s) contrôle(s) utilisateur(s) "Custom6"

Displays;Listes d'affichage
Next Display;Liste suivante
Previous Display;Liste précédente
Displays Menu;Liste des Listes d'affichage
Ambient Soundtrack;Musique d'ambiance
Allow Exit from 'Displays Menu';Autoriser la sortie depuis la liste des listes
Generate Collection/Rom List;Générer la liste de roms 

#
# Strings needing translation:
#


# Scraper stuff

Scraper;Aspiration
Scrape Artwork;Aspirer les Artworks
Configure / Scraper;Configurer l'aspiration
Scrape Snaps;Aspire les images des jeux
Scrape Marquees;Aspire les Marquees
Scrape Fanart;Aspire les 'Fanart'
Scrape Flyers/Boxart;Aspire les Flyers/Couvertures
Scrape Game Logos (Wheel Art);Aspire les logos des jeux (Wheel Art)
Scraping Artwork: $1%;Aspire l'Artwork: $1%

_help_emu_scrape_artwork;Aspirer les artworks manquants pour cet émulateur sur TheGamesDB.net et mamedb.com
_help_scraper;Configurer les parametres d'aspiration des artworks
_help_scrape_snaps;Définir où les 'snapshots' des jeux doivent être téléchargés lorsqu'ils sont aspirés
_help_scrape_marquees;Définir où les 'marquees' et les bannières des jeux doivent être téléchargés lorsqu'ils sont aspirés
_help_scrape_flyers;Définir où les 'flyers' et les couvertures des boîtes des jeux doivent être téléchargés lorsqu'ils sont aspirés
_help_scrape_wheels;Définir où les logos des jeux doivent être téléchargés lorsqu'ils sont aspirés
_help_scrape_fanart;Définir où les 'fan art' des jeux doivent être téléchargés lorsqu'ils sont aspirés 
_help_scrape_vids;Définir où les videos des jeux doivent être téléchargés lorsqu'ils sont aspirés 
_help_intro;Configurer l'intro

