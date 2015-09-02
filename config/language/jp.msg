﻿#
# General
#
Exit Attract-Mode;プログラムを終了
Exit Attract-Mode?;終了しますか?
Yes;はい
No;いいえ

#
# Emulator Settings Configuration
#
Edit Artwork;アートワークを編集
Artwork Label;アートワークのレーベル
Artwork Path;アートワークの位置
Edit Emulator;エミュレータを編集
Emulator Name;エミュレータの名前
Executable;実行ファイル
Command Arguments;実行パラメータ
Rom Path(s);Romファイルの位置
Rom Extension(s);Romファイルの拡張子
Additional Import File(s);追加入力ファイル
XML Mode;XML モード
Add Artwork;アートワークを追加
Generate Collection/Rom List;Romファイルリストを作成
Delete this Emulator;エミュレータを削除
Generating Rom List: $1%;Romファイルリストを作成しています: $1%
Rom path '$1' not found, proceed anyways?;Romファイル経路 '$1' が存在しません. 続きますか?
Overwrite existing '$1' romlist?;既存のRomファイルリスト '$1' に上書きしてもよろしいですか?
Wrote $1 entries to romlist;$1 個のRomファイルをリストに記録しました
Delete emulator '$1'?;'$1' エミュレータを削除しますか?
Config / Emulators;設定 / エミュレータ
Add Emulator;エミュレータを追加
Enter Emulator Name;エミュレータの名前を入力して下さい
Configure available emulators;エミュレータの設定

_help_art_label;指定したレーベルがレイアウトに使われます
_help_art_path;アートワークの位置です
_help_art_add;新しいアートワーク (スクリンショット, タイトル, ムービー) を追加します
_help_emu_executable;エミュレータの実行ファイルの位置を指定します. $HOMEはユーザーのホームディレクトリに置換されます
_help_emu_args;エミュレータの実行パラメータを設定します. 以下のパラメータはその内容で置換されます : [name], [romext], [rompath], [emulator], [romfilename]
_help_emu_rompath;エミュレータのRomファイルの位置を指定します
_help_emu_romext;Romファイルの拡張子(.zip 等)を指定します. セミコロン(;)で複数の拡張子を指定できます
_help_emu_import_extras;Romファイルリストの作成時に追加で使うファイルを指定します. ジャンル (catver.ini), またはプレイ人数 (nplayers.ini) などを使えます
_help_emu_listxml;MAME (-listxml を使う最新バージョン) の場合 "mame", MESS (-listsoftware を使う最新バージョン) の場合は "mess <system>" で設定してください. 他の場合は空にしてください
_help_emu_gen_romlist;指定されたRomファイルの位置, XML モード, 追加ファイル (catver.ini 等) を使ってRomファイルリストを作成します
_help_emu_delete;エミュレータに関する設定を削除します
_help_emu_sel;エミュレータの設定を変更します
_help_emu_add;新しいエミュレータを登録します

#
# Filter Settings Configuration
#
Enter Filter Name;フィルターの名前を入れて下さい
Rule Edit;ルール編集
Target;ルールのターゲット
Comparison;ルールの比較方式
Filter Value;フィルターの表現式
Delete this Rule;このルールを削除
Filter Edit;フィルターを編集
Filter Name;フィルターの名前
Rule;ルール
Add Rule;ルールを追加
Delete this Filter;このフィルターを削除
Delete filter '$1'?;'$1' フィルターを削除しますか？
All;全て
Title;タイトル(Title)
Emulator;エミュレータ(Emulator)
CloneOf;クローン(CloneOf)
Year;発売年(Year)
Manufacturer;発売元(Manufacturer)
Category;ジャンル(Category)
Players;プレイヤー数(Players)
Rotation;ディスプレイ回転(Rotation)
Control;コントローラ(Control)
Status;エミュレーションレベル(Status)
DisplayType;ディスプレイタイプ(DisplayType)
DisplayCount;ディスプレイの数(DisplayCount)
AltRomname;Romの別称(AltRomname)
AltTitle;タイトルの別称(AltTitle)
Extra;その他(Extra)
Favourite;お気に入り
FileIsAvailable;Romファイルの存在
equals;一致する
does not equal;一致しない
contains;含む
does not contain;含まない
The name for this filter;このフィルターの名前
Configure filter rule;フィルターのルールー編集
Add a rule to this filter;ルールー追加
Delete this filter;このフィルターを削除

_help_rule_target;フィルターが使うゲームのカテゴリーを設定します
_help_rule_comp;フィルターのルールを比較する方式を設定します
_help_rule_value;フィルターのルールを正規表現で設定します. 例えばターゲットが"発売年"で,比較方式が"一致する"で,表現式が"1984"の場合,1984年に発売されたゲームのみ表示されます．"198."の場合 1980年代の全てのゲームが表示されます.　"(198.)|(199.)"の場合1980年代と1990年代の全てのゲームを表示します
_help_rule_delete;このルールを削除します

#
# Display Settings Configuration
#
Display Edit;ディスプレイを設定
Display Name;ディスプレイの名前
Layout;レイアウットの名前
Collection/Rom List;Romファイルリスト
Filter;フィルター
Add Filter;フィルターを追加
Delete this Display;このディスプレイを削除
Delete list '$1'?;ディスプレイ '$1' を削除してもよろしいですか？
Configure / Displays;設定 / ディスプレイ
Add New Display;ディスプレイを追加
Enter Display Name;新しいディスプレイの名前を入力して下さい

_help_display_name;このディスプレイの名前です
_help_display_layout;このディスプレイが使うレイアウットを設定します
_help_display_romlist;このディスプレイが使うRomファイルリストを設定します
_help_display_filter;このディスプレイのフィルターを設定します
_help_display_add_filter;ディスプレイに新しいフィルターを追加します
_help_display_delete;このディスプレイを削除します
_help_display_sel;このディスプレイを設定します
_help_display_add;新しいディスプレイを追加します

#
# Input Settings Configuration
#
Edit Input;入力編集
Action;アクション
Remove Input;キー削除
Add Input;キー追加
Press Input;キーを押してください
This will overwrite an existing mapping ($1).  Proceed?;既存の入力 ($1) を上書きしてもよろしいですか？
Configure / Input;設定 / 入力
OR;又は
Select;選択
Up;上
Down;下
Page Up;1画面上
Page Down;1画面下
Previous Display;前のディスプレイ
Next Display;次のディスプレイ
Displays Menu;ディスプレイメニュー
Previous Filter;前のフィルター
Next Filter;次のフィルター
Filters Menu;フィルターメニュー
Toggle Layout;レイアウット選択
Toggle Movie;ムービー再生
Toggle Mute;ミュート
Toggle Rotate Right;画面 90°回転
Toggle Flip;画面 180°回転
Toggle Rotate Left;画面 270°回転
Exit (Confirm);終了 (Yes/No 確認)
Exit to Desktop;終了
Screenshot;スクリーンショット
Configure;設定
Random Game;任意のゲーム選択
Add/Remove Favourite;お気に入り追加/削除
Next Favourite;前のお気に入り
Previous Favourite;次のお気に入り
Replay Last Game;最後にプレイしたゲームを再プレイ
Ambient Sounds;BGMファイル
Startup Sound;プログラム起動のサウンド
Game Return Sound;ゲーム終了のサウンド
Mouse Threshold;マウスの感度
Joystick Threshold;ジョイスティックの感度

_help_input_delete;入力キーを削除します
_help_input_add;入力キーを追加します
_help_control_sel;入力キーを設定します
_help_joystick_threshold;ジョイスティックの感度を設定します. (1から100まで), 高いほど鈍感です
_help_mouse_threshold;マウスの感度を設定します. (1から100まで), 高いほど鈍感です

#
# Sound Settings Configuration
#
Configure / Sound;設定 / サウンド
Sound Volume;サウンドボリューム
Ambient Volume;BGMボリューム
Movie Volume;ムービーボリューム
Ambient Soundtrack;BGMファイル
_help_volume;ボリュームを設定します(0から100まで), 0はミュートです
_help_sound_sel;この状態でプレイするサウンドを設定します

#
# Miscellaneous Settings Configuration
#
Configure / Miscellaneous;設定 / その他
Hide Brackets in Game Title;ゲームタイトルのブラケットを隠す
Launch Last Game on Startup;最後にプレイしたゲームを次の起動時 再起動
Confirm Favourites;お気に入りの確認
Layout Options;レイアウト別設定

None;0°
Right;90°
Flip;180°
Left;270°
Exit Command;終了コマンド
Default Font;基本フォント
Font Path;フォントの位置
Allow Exit from 'Displays Menu';ディスプレイメニューからのプログラム終了可能
Language;言語
en;英語
fr;フランス語
kr;韓国語
jp;日本語

_help_exit_command;終了 (Yes/No 確認) でプログラムを終了する時に実行する外部コマンドを設定します (例. "sudo shutdown")
_help_default_font;プログラムが基本で使うフォントの名前を指定します
_help_font_path;フォントの位置を設定します.セミコロン(;) で複数の位置を設定できます
_help_displays_menu_exit;ディスプレイメニューに"プログラム終了"項目を追加するか設定します
_help_language;言語を設定します
_help_display_layout_options;レイアウトで指定されたオプションを設定できます
_help_hide_brackets;ゲームのタイトルを表示する時, ブラケットに囲まれた部分は隠すかを設定します
_help_autolaunch_last_game;プラグラムの起動時, 最後にプレイしたゲームを一緒に起動するかを設定します
_help_confirm_favs;お気に入りのゲームを追加,削除する時, Yes/Noで確認するかを設定します

Configure Screen Saver;スクリーンセーバー設定
Screen Saver Timeout;スクリーンセーバーのタイムアウト時間
_help_screen_saver_timeout;何秒まで入力が無ければスクリーンセーバーを起動するかを設定します. 0 でスクリーンセーバーをオフにします

#
# Plug-in Configuration
#
Configure Plug-in;プラグインを設定
Name;名前
Command;コマンド
Enabled;使用する
Configure / Plug-ins;設定 / プラグイン

_help_plugin_command;プラグインのコマンドを設定します
_help_plugin_enabled;プラグインを使うかを設定します
_help_plugin_sel;プラグインを設定します

#
# Configuration Main Menu
#
Configure;設定
Emulators;エミュレータ
Displays;ディスプレイ
Controls;入力
Sound;サウンド
Screen Saver;スクリーンセーバー
Plug-ins;プラグイン
General;その他
Back;戻る

_help_emulators;エミュレータを設定します
_help_displays;ディスプレイ(ゲームリスト)を設定します
_help_controls;様々な入力を設定します
_help_screen_saver;スクリーンセーバーを設定します
_help_sound;効果音, 背景音楽, ボリューム等を設定します
_help_plugins;外部プラグインを設定します
_help_misc;その他を設定します
Exit menu;出る

_sort_regexp;^(Vs\. The |The |Vs\. )


#
# Strings needing translation:
#
System Identifier;システムのID
Info Source/Scraper;情報元 設定
Minimum Run Time;最小実行時間
Exit Hotkey;終了ホットキー
Sort By;並び替え
No Sort;並び替えない
Tags;タッグ
PlayedCount;プレイ数
PlayedTime;プレイ時間
List Limit;リストのサイズ限度
Add/Remove Tags;タッグ追加/削除
Previous Letter;前の文字
Next Letter;次の文字
Custom1;カスタム 1
Custom2;カスタム 2
Custom3;カスタム 3
Custom4;カスタム 4
Custom5;カスタム 5
Custom6;カスタム 6
Track Usage;プレイ時間記録
Filter Wrap Mode;リストの最後のフィルターに着いた時
Wrap within Display (Default);リストの最初のフィルターに (デフォルト)
Jump to Next Display;次のリストに
No Wrap;何もしない
Window Mode;スクリーンモードー
Fill Screen (Default);全画面に広げる (デフォルト)
Fullscreen Mode;全画面モード
Window;ウィンドウモード
Global Filter;グローバルフィルターの設定
Disabled;無効

_help_emu_system;このエミュレータのIDです. セミコロン(;) で複数のIDを設定できます
_help_emu_info_source;mess : MAME タイプの -listxml, mess : MESS タイプの -listsoftware, thegamesdb.net : このサイトを使う, steam : steamサイトを使う. mess, thegamesdb.net の場合IDを正しく設定する必要があります.
_help_emu_minimum_run_time; 最小の実行時間を秒単位で設定します. 普通は0で構わないですが,一部のゲーム(steam等)には適当に設定する必要があります
_help_emu_exit_hotkey;このエミュレータを終了するグローバルホットキーを設定します. このホットキーを押せばこのエミュレータを強制終了できます.
_help_filter_sort_by;リストの並び方を設定します. "並び替えない" で元の順番で並べます.
_help_filter_list_limit;リストのサイズに制限を付けます. プラスで上からの××個, マイナスで下からの××個を設定します. 0 は無限です.
_help_window_mode;.Attract-Mode がウィンドウモードで起動するかフルスクリーンモードで起動するかを設定します
_help_filter_wrap_mode;リストの最後のフィルターで"次のフィルター"が押された時, どのように反応するかを設定します.
_help_track_usage;ゲームのプレイ内容(プレイ数,プレイ時間)を記録するかを設定します.
_help_filter_name;フィルターの名前
_help_filter_rule;フィルターのルールを設定します
_help_filter_add_rule;このフィルターに新しいルールを追加します
_help_filter_delete;このフィルターを削除します
_help_display_global_filter;このリスト全体に適用するグローバルフィルターを設定します
_help_control_select;現在選択されたゲームをプレイするキーを設定します
_help_control_up;上に移動するキーを設定します
_help_control_down;下に移動するキーを設定します
_help_control_page_up;1ページ上に移動するキーを設定します
_help_control_page_down;1ページ下に移動するキーを設定します
_help_control_prev_display;前のリストに移動するキーを設定します
_help_control_next_display;次のリストに移動するキーを設定します
_help_control_displays_menu;'リストメニュー' を表示するキーを設定します
_help_control_prev_filter;前のフィルターに移動するキーを設定します
_help_control_next_filter;次のフィルターに移動するキーを設定します
_help_control_filters_menu;'フィルターメニュー' を表示するキーを設定します
_help_control_toggle_layout;レイアウットを変更するキーを設定します. (レイアウットにより無視される場合があります)
_help_control_toggle_movie;ムービーの再生をオン/オフにするキーを設定します
_help_control_toggle_mute;ミュートするキーを設定します
_help_control_toggle_rotate_right;画面を時計回りに90°回転するキーを設定します
_help_control_toggle_flip;画面を180°回転するキーを設定します
_help_control_toggle_rotate_left;画面を反時計回りに90°回転するキーを設定します
_help_control_exit;このプログラムを終了するキーを設定します
_help_control_exit_no_menu;このプログラムを確認無しで終了するキーを設定します
_help_control_screenshot;スクリーンショットを撮るキーを設定します
_help_control_configure;この '設定画面' を表示するキーを設定します
_help_control_random_game;任意のゲームを選択するキーを設定します
_help_control_replay_last_game;最後にプレイしたゲームを再びプレイするキーを設定します
_help_control_add_favourite;現在選択されたゲームをお気に入りに追加/削除するキーを設定します
_help_control_prev_favourite;前のお気に入りゲームに移動するキーを設定します
_help_control_next_favourite;次のお気に入りゲームに移動するキーを設定します
_help_control_add_tags;現在選択されたゲームのタッグを編集するキーを設定します
_help_control_screen_saver;スクリーンショットを起動するキーを設定します
_help_control_prev_letter;前のアルファベットに移動するキーを設定します
_help_control_next_letter;次のアルファベットに移動するキーを設定します
_help_control_custom1;カスタムキー1を設定します
_help_control_custom2;カスタムキー2を設定します

# Scraper stuff
Scraper;オンラインゲーム情報出典
Scrape Artwork;ファンアートーワークをダウンロード
Configure / Scraper;設定 / オンラインゲーム情報
Scrape Snaps;スクリーンショットをダウンロード
Scrape Marquees;Marqueeをダウンロード
Scrape Flyers/Boxart;箱のアートーワーク, ポスターをダウンロード
Scrape Game Logos (Wheel Art);ゲームロゴをダウンロード (Wheel Art)
Scraping Artwork: $1%;オンラインから $1% をダウンロード中です

_help_emu_scrape_artwork;TheGamesDB.net または mamedb.com からゲームの情報をダウンロードします
_help_scraper;ゲームの情報を担当するオンラインサイトを設定します
_help_scrape_snaps;オンラインからスクリーンショットをダウンロードするかを設定します
_help_scrape_marquees;オンラインからMarqueeをダウンロードするかを設定します
_help_scrape_flyers;オンラインから箱のアートーワーク, ポスターをダウンロードするかを設定します
_help_scrape_wheels;オンラインからWheel Artタイプのゲームロゴをダウンロードするかを設定します
_help_scrape_fanart;オンラインからファンアートーワークをダウンロードするかを設定します
