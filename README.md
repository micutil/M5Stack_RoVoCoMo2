# M5Stack_RoVoCoMo2 (v1.9)
ロビ１、ロビ２をWi-Fi（FlashAirを使用）またはBLE (Bluetooth Low Enagy)により無線コントロールするためのM5Stackのプログラムです。RoVoCoMo2という名前は「Robi Voice Controller by Micono」の略で、ブラウザ版の[RoVoCoMo](http://micono.cocolog-nifty.com/blog/2017/07/rovocomo-v05-81.html)のM5Stack版をRoVoCoMo2としています。

どのようなプログラムなのかは、以下の像をクリックすると操作中の動画をで観ることができます（**この動画で使っているRoVoCoMo2は旧バージョンであり最新バージョンのものとは異なります**）。<br>
[![preview](images/preview01s.jpg)](https://www.youtube.com/watch?v=HBEKJXp4zvs)


## インストール
RoVoCoMo2を「Clone or download」（緑色のボタン）でDownload ZIPを選び、ダウンロードし、zipを解凍します。

### ① 無線対応の「ロビのココロ」を作る
ロビのココロ (microSD) をFlashAirやBLEを使って動かすためには、それら対応のロビのココロを作成する必要があります。コントーロルの仕様は、メディアクラフトさんの仕様に合わせる形で対応しています。したがって、**ここでの作業は、主にメディアクラフトさんのページを参考に行って下さい。**

#### A: ロビ2の場合
**STARTUP.BINをBLEまたはFlashAir対応に変換する**<br>
メディアクラフトさんの「[ロビ設定ファイルエディタ２](http://www.kumagaya.or.jp/~mcc/robox/RBEdit2/index.html)」を使って、ロビ２のココロに入っている**STARTUP.BINを無線対応に書き換えてください**。以下は、ロビ設定ファイルエディタ２のページからの抜粋です。クラフトさんのページで使われている「リモート接続」や「リモート操作」とは、「BLEやFlashAirを使った接続」や「操作」と考えて下さい。

- **リモート対応**をチェックすると**FlashAir**によるリモート接続に対応できます。
- **Bluetooth**にチェックするとQ-boやM5Stackでリモート操作ができます。
- Bluetooth開始コードは、**RoVoCoMo2**の場合は**「1154」**が開始コードとなります。

ロビ設定ファイルエディタ２はWindows版のアプリケーションです。マックをお使いでSTARTUP.BINを作れない場合は、miconoまでご連絡下さい。なお、マック版の同様のプログラムを作成を計画していますが、いつごろになるか未定です。

なお、[FlashAir](http://www.kumagaya.or.jp/~mcc/robox/RBPlayer/index.html#FLASHAIR)自体の設定方法は、こちらを参考にして下さい。


#### B:ロビ1の場合 ***(FlashAir対応のロビのココロを作る)***
ロビをFlashAirを使って無線コントロールする方法がクラフトさんのサイトで公開されていますの で、以下のページを参考にFlashAir対応のロビのココロを作成して下さい。

1. 以下のURLの**リモート接続用ロビプログラム**をダウンロードして、ロビのココロの書き換えなどを行なって下さい。<br> [リモート接続用ロビプログラム](http://www.kumagaya.or.jp/~mcc/robox/RBMotion/index.html#REMOTE_DOWNLOAD)

2. 以下のURLの「**FlashAirの設定**」を参考にFlashAirの設定を行なって下さい。<br>  [FlashAirの設定](http://www.kumagaya.or.jp/~mcc/robox/RBPlayer/index.html#FLASHAIR)
 
- FlashAirのSSIDとパスワードは、SSIDがflashair_xxxxxで、パスワードが12345678の 場合に自動的に接続できるようになっています(xxxxxの部分は任意の文字)。買った状態 のSSIDとパスワードはこの設定になっています。
- もしSSIDやパスワードが上の条件になっていない場合は、エディタでSSIDとパスワード をカンマで区切って書いて、flaap.txtと言う名前でM5Stackに入れるmicroSDに保存して ください。起動時にそのファイルを最初に検索する仕様になっています。
 
 
### ② M5StackのmicroSDに入れるもの
1. 同梱のファイルの中にmicroSDフォルダがあります、その中のファイルやフォルダをmicroSDにコピーをして下さい。ちなみに、**RoVoCoMo2.bin**というファイルがRoVoCoMo2のプログラムになります。また、jpgやjsonのフォルダに入っているファイルはメニュプログラムが使うデータです。もし、コピー先のmicroSDの中に、既にjpgフォルダや、jsonフォルダがある場合は、それぞれの中に入っているファイルをコピーするようにして下さい。またバージョンのファイルで置き換えてしまわない様にコピーする際は、修正日を比べてコピーする様にして下さい。
2. さらに、**ロビ１**をコントロールする場合は、**ROBI1フォルダ**の中の**Ninshiki.csv**をコピー先のmicroSDのルートにコピーして下さい。 **ロビ２（＆ロビ１**）をコントロールする場合は、**ROBI2フォルダ**の中の**Ninshiki.csv**をコピー先のmicroSDのルートにコピーして下さい。同じファイル名なので、どちらかの**Ninshiki.csv**を使うことになります。
3. **ロビ１のココロ**をお持ちの方は、M5Stackに入れるmicroSDにvoiceフォルダを作成して、ロビのココロの中のvoiceフォルダの中の**Ninchikiフォルダ**をmicroSDに作ったvoiceフォルダの中にコピーして下さい。
6. **ロビ２のココロ**をお持ちの方は、M5Stackに入れるmicroSDにvoiceフォルダを作成して、ロビ２のココロの中のvoiceフォルダの中の**NFフォルダ**をmicroSDに作ったvoiceフォルダの中にコピーして下さい。なお、ロビ１もお持ちの方は、**NinshikiフォルダとNFフォルダと両方入れても**問題ありません。
2. 次に、RSTesterが使っている**フォント**（**FONT.BIN**, **FONTLCD.BIN**)が必要になります。これらのファイツをまだ入れてない場合は、ブラウザで、[Tamakichi/Arduino-KanjiFont-Library-SD](https://git.io/fjYst)を開いて下さい。
3. 「**Clone or download**」でD**ownload ZIP**を選び、ダウンロードし、zipを解凍します。
4. そのファイルの中に、fontbinフォルダがあり、中に、**FONT.BIN**, **FONTLCD.BIN**というファイルがあるので、これら２つのファイルをmicroSDにコピーします。
5. これでM5Stackに入れるmicroSDの作成は終了です。このmicroSDをM5Stackに入れて下さい。


### ③ USBドライバーをインストールする
M5Stackをコンピュータと繋いで、プログラムの転送など通信するためには、Silicon LabsのUSBDriverをインストールする必要があります。もしまだインストールしてなければ
[シリアル接続の確立方法](https://docs.m5stack.com/#/ja/related_documents/establish_serial_connection)
のサイトを参考にしてインストールして下さい。

> なお、Macの場合、インストールしただけではセキュリティが通ってないので、インストール後、環境設定のセキュリティとプライバシーの一般で、インストールしたドライバーの許可をして下さい。
> 
![セキュリティ](images/kyoka.jpg)


### ④ M5Burner_Micで、M5StackにSD-Menuをインストールする
"M5Burner\_Mic"というプログラムでSD-MenuをM5Stackにインストールします。すでにSD-MenuまたはLovyanLauncherをインストールしてある場合はこのステップは必要ありません。

1. [M5Burner_Mic](https://github.com/micutil/M5Burner_Mic) のページから「Download [here](http://micutil.com/download/M5Burner_Mic.zip)」の所でM5Burner\_Micをダウンロードし、解凍して下さい。（M5Burner\_Micフォルダはお好みの場所に入れて下さい。）
2. M5Stackに付いてきたUSB-CケーブルでパソコンとM5Stackを繋げて下さい。
3. M5Burner\_Micをダブルクリックして起動します。
4. USBシリアルポートをM5Stackのポートに設定します。
 - Macの場合はポートに名前がつくので「**SLAB_USBtoUART**」という名前のポートを選んで下さい。
 - Windowsの場合は、**COM3**とか、COM4とかの名前になっています。ひとつしか表示されてなかったら、それがM5Stackのポートでしょう。もしいくつか表示されているようだったら、コントロールパネルから、デバイスマネージャーのポートをみて番号を確認して下さい。例えば以下の図の場合だと**COM4**であるということになります。<br>![ポート番号](images/portnum.jpg)
5. 「Priset」のポップアップメニューで「**SD-Menu**」の最新版を選択します。
6. 「**Start**」ボタンをクリックすると、プログラムの転送が開始します。
7. プログラムの転送が終わるとM5Stackがリセットされ、インストールした**SD-Menu**が起動します。
8. M5StackのCボタン（右）を何回か押して、RoVoCoMo2を選択し、Bボタン（中央）のボタンを押すと、RoVoCoMo2が起動します。操作方法は後で説明します。
9. microSDにmenu.binが入ってない場合は、M5Burner_Micのfirmwaresフォルダの中のtobozoフォルダの中に**menu.bin**があるので、それをmicroSDにコピーして下さい。
9. 再度、メニューを表示する場合は、Aボタン（左）を押しながらリセットボタン（左上側面）を押すとSD-Menuが起動します。


##操作方法
#### ボタン操作(左から、Aボタン、Bボタン、Cボタン)
- 索引(Cボタン)を押して、50音表示にします。
- 右矢印(Cボタン)と下矢印(Aボタン)を押して、動作させたい認識語の頭文字を選択します。例えば「ゲームしよう」を選びたい場合は「げ」を選択して、「選択」(Bボタン)を押します。
- 認識語の一覧が黄色く表示されているので、下矢印(Aボタン)で動作させる音声を選択します。
- 「選択」(Bボタン)で、それに対応する信号が送信され、ロビが動作を開始します。

#### 電源・リセット
- AボタンとCボタンを同時に押してると電源を切ることができます。
- 左側面の赤いボタンがリセットボタンです。
- Aボタンを押したままリセットをすると、メニュープログラムを起動させられます。 ● Bボタンを押したまま起動すると、FlashAir機能がオフになります。
- Cボタンを押したまま起動すると、音声再生がオフになります。

#### 特別な操作(ロビライドの操作)
- ロビ2でロビライドモードに入る時は「ロビライドで遊ぼう」です。その操作後に、ロビ2のロビライド用の認識語一覧が表示されます。
- ロビ1でロビライドモードに入る時は「遊ぼう」です。その操作後に、ロビ1のロビライド用の認識語一覧が表示されます。終了する「だいじょうぶ」も再生/リモート送信されます。
- ロビライドモードに入ると、上矢印(Cボタン)、下矢印(Aボタン)で、認識語を選択できるようになります。

#### その他
- FlashAirのSSIDとパスワードは、SSIDがflashair_xxxxxで、パスワードが12345678の場合に自動的に接続できるようになっています(xxxxxの部分は任意の文字)。買った状態のSSIDとパスワードはこの設定になっています。
- SSIDやパスワードが上の条件になっていない場合は、エディタでSSIDとパスワードをカンマで区切って書いてflaap.txtと言う名前でM5Stackに入れるmicroSDに保存してください。起動時にそのファイルを最初に検索する仕様になっています。
- 複数個のFlashAirの電波があった場合、いちばん電波の強いFlashAirに接続する仕様になっていますので、起動時に接続させたいFlashAirの近くで起動させて下さい。

## 履歴
	ver 1.9: 2019/ 4/30 : FlashAirを使った場合の落ちる不具合を修正
	                      Ninshikiフォルダが入ってない場合、音声がでない不具合を修正
	ver 1.8: 2019/ 4/23 : CSVファイル改良（容量軽減）
	ver 1.7: 2019/ 4/14 : ロビ２のココロ対応
	ver 1.6: 2019/ 3/28 : 認識語リスト外部ファイル化
	ver 1.5: 2019/ 3/ 8 : ロビライドの操作性ほか
	ver 1.4: 2019/ 3/ 5 : FlashAir対応
	ver 1.3: 2019/ 2/24 : 50音索引化
	ver 1.2: 2019/ 1/27
	ver 1.1: 2019/ 1/25
	ver 1.0: 2019/ 1/ 6

#### 更新方法
- RoVoCoMo2.bin, ROBI1フォルダ, および,ROBI2フォルダをmicroSDに入れる
- ロビ２（ロビ１兼用）を使う場合は、ROBI2フォルダの中のNinshiki.csvを、ロビ１をを使う場合は、ROBI1フォルダの中のNinshiki.csvをmicroSDのルートにコピーします。
- Aボタン+リセットで、一旦メニュープログラムに戻ります。
- メニュープログラムから、RoVoCoMo2を選択して起動して下さい。

## 免責およびライセンス

CC 4.0 BY-NC-ND https://github.com/micutil/M5Stack_RSTester

- なお、現在、特に開発段階であるため、このアーカイブに含まれるすべてのファイル等に関して、無断で、転載、掲載、変更の禁止など、動作テスト以外のすべての行為を禁じます。
- これらを使ったことにより生じるいかなるトラブルに関しても自己責任においておこなって下さい。
- お問い合わせ等は、miconoまで、メールなどをください。

謝辞：RoVoCoMo2は、メディアクラフトさん（http://www.kumagaya.or.jp/~mcc）が
おこなった、ロビのココロなどの解析によりはじめて実現できたものです。この場をお借りしてお礼を申し上げます。
