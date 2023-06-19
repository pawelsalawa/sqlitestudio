<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh-TW" sourcelanguage="en">
  <context>
    <name>DbAndroid</name>
    <message>
      <location filename="../dbandroid.cpp" line="39"/>
      <source>Invalid or incomplete Android Database URL.</source>
      <translation>無效或不完整的 Android 資料庫 URL。</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="54"/>
      <source>Android database URL</source>
      <translation>Android 資料庫 URL</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="55"/>
      <source>Select Android database</source>
      <translation>選擇 Android 資料庫</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="151"/>
      <source>Select ADB</source>
      <translation>選擇 ADB</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="173"/>
      <source>Using Android Debug Bridge: %1</source>
      <translation>使用 Android 除錯橋 (ADB)：%1</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="183"/>
      <source>You can grab Android connector JAR file from Tools menu. It&apos;s required for 2 of 3 connections supported by the Android plugin. For more details read plugin&apos;s documentation on &lt;a href=&quot;%1&quot;&gt;SQLiteStudio&apos;s wiki page.&lt;/a&gt;</source>
      <translation>您可以從工具選單取得 Android 聯結器 JAR 檔案。Android 外掛支援的 3 種連線方式有 2 個需要它。更多細節見 SQLiteStudio wiki 頁面上的&lt;a href=&quot;%1&quot;&gt;外掛文件&lt;/a&gt;。</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="191"/>
      <source>Could not find Android Debug Bridge application. &lt;a href=&quot;%1&quot;&gt;Click here&lt;/a&gt; to point out the location of the ADB application, otherwise the %2 plugin will not support USB cable connections, only the network connection.</source>
      <translation type="unfinished">Could not find Android Debug Bridge application. &lt;a href=&quot;%1&quot;&gt;Click here&lt;/a&gt; to point out the location of the ADB application, otherwise the %2 plugin will not support USB cable connections, only the network connection.</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="232"/>
      <source>Save JAR file</source>
      <translation type="unfinished">Save JAR file</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="209"/>
      <source>Invalid ADB</source>
      <translation>無效的 ADB</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="209"/>
      <source>The selected ADB is incorrect.
Would you like to select another one, or leave it unconfigured?</source>
      <translation>選擇的 ADB 不正確。
選擇另一個？或者不做設定檔？</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="211"/>
      <source>Select another ADB</source>
      <translation>選擇其他 ADB</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="211"/>
      <source>Leave unconfigured</source>
      <translation>不做設定檔</translation>
    </message>
    <message>
      <location filename="../dbandroid.cpp" line="251"/>
      <source>Get Android connector JAR file</source>
      <translation>獲取 Android 聯結器 JAR 檔案</translation>
    </message>
  </context>
  <context>
    <name>DbAndroidInstance</name>
    <message>
      <location filename="../dbandroidinstance.cpp" line="113"/>
      <source>Android SQLite driver does not support loadable extensions.</source>
      <translation>Android SQLite 驅動程式不支援可載入的擴充套件。</translation>
    </message>
    <message>
      <location filename="../dbandroidinstance.cpp" line="206"/>
      <source>Connection with Android database &apos;%1&apos; lost.</source>
      <translation>與 Android 資料庫 &apos;%1&apos; 的連線丟失。</translation>
    </message>
  </context>
  <context>
    <name>DbAndroidJsonConnection</name>
    <message>
      <location filename="../dbandroidjsonconnection.cpp" line="175"/>
      <source>Cannot connect to device %1, because it&apos;s not visible from your computer.</source>
      <translation type="unfinished">Cannot connect to device %1, because it&apos;s not visible from your computer.</translation>
    </message>
    <message>
      <location filename="../dbandroidjsonconnection.cpp" line="182"/>
      <source>Failed to create port forwarding for device %1 for port %2.</source>
      <translation>為裝置 %1 建立 %2 埠轉發失敗。</translation>
    </message>
    <message>
      <location filename="../dbandroidjsonconnection.cpp" line="196"/>
      <source>Could not connect to network host: %1:%2</source>
      <translation>無法連線到網路主機：%1:%2</translation>
    </message>
    <message>
      <location filename="../dbandroidjsonconnection.cpp" line="210"/>
      <source>Cannot connect to %1:%2, because password is invalid.</source>
      <translation>無法連線到 %1:%2，密碼無效。</translation>
    </message>
    <message>
      <location filename="../dbandroidjsonconnection.cpp" line="301"/>
      <source>Unable to execute query on Android device (connection was closed): %1</source>
      <translation>無法在 Android 裝置上執行查詢 (連線已關閉)：%1</translation>
    </message>
    <message>
      <location filename="../dbandroidjsonconnection.cpp" line="313"/>
      <source>Error while parsing response from Android: %1</source>
      <translation>解析來自 Android 響應出錯：%1</translation>
    </message>
    <message>
      <location filename="../dbandroidjsonconnection.cpp" line="321"/>
      <source>Generic error from Android: %1</source>
      <translation>來自 Android 的一般錯誤：%1</translation>
    </message>
    <message>
      <location filename="../dbandroidjsonconnection.cpp" line="335"/>
      <location filename="../dbandroidjsonconnection.cpp" line="342"/>
      <source>Missing &apos;columns&apos; in response from Android.</source>
      <translation>來自 Android 的響應中缺少 &apos;columns&apos;。</translation>
    </message>
    <message>
      <location filename="../dbandroidjsonconnection.cpp" line="363"/>
      <source>Response from Android has missing data for column &apos;%1&apos; in row %2.</source>
      <translation>來自 Android 的響應中，缺少資料行 %2、列 &apos;%1&apos; 的資料。</translation>
    </message>
  </context>
  <context>
    <name>DbAndroidPathDialog</name>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="20"/>
      <source>Android database URL</source>
      <translation>Android 資料庫 URL</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="26"/>
      <source>Connection method</source>
      <translation>連線方式</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="32"/>
      <source>USB cable - port forwarding</source>
      <translation>USB 線纜 - 埠轉發</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="42"/>
      <source>USB cable - sqlite3 command</source>
      <translation>USB 線纜 - sqlite3 命令</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="49"/>
      <source>Network (IP address)</source>
      <translation>網路 (IP 地址)</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="59"/>
      <source>Device</source>
      <translation>裝置</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="71"/>
      <source>IP address</source>
      <translation>IP 地址</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="93"/>
      <source>Port</source>
      <translation>埠</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="115"/>
      <source>Remote access password</source>
      <translation>遠端訪問密碼</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="127"/>
      <source>&lt;p&gt;This is password configured in the SQLiteStudio service being embeded in the Android application.&lt;/p&gt;</source>
      <translation>&lt;p&gt;這是在 Android 應用程式中嵌入的為 SQLiteStudio 服務設定檔的密碼。&lt;/p&gt;</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="140"/>
      <source>Application</source>
      <translation>應用程式</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="155"/>
      <source>Filter</source>
      <translation>篩選器</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="168"/>
      <source>Database</source>
      <translation>資料庫</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="177"/>
      <source>Create a new database directly on the device.</source>
      <translation>直接在該裝置上新建一個數據庫。</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.ui" line="187"/>
      <source>Delete currently selected database from the device. The currently selected database is the one picked in the list on the left of this button.</source>
      <translation>從裝置中刪除當前選中的資料庫。當前選中的資料庫顯示在此按鈕左側的清單中。</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="362"/>
      <source>Enter valid IP address.</source>
      <translation>請鍵入有效的 IP 地址。</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="367"/>
      <source>Pick Android device.</source>
      <translation>選擇 Android 裝置。</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="371"/>
      <source>Pick Android database.</source>
      <translation>選擇 Android 資料庫。</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="395"/>
      <source>Selected Android application is unknown, or not debuggable.</source>
      <translation>所選的 Android 應用程式未知或者不可除錯。</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="422"/>
      <source>Create new database</source>
      <translation>建立新資料庫</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="422"/>
      <source>Please provide name for the new database.
It&apos;s the name which Android application will use to connect to the database:</source>
      <translation>請提供新資料庫的名稱。
Android 應用程式將用此來連線該資料庫：</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="430"/>
      <location filename="../dbandroidpathdialog.cpp" line="439"/>
      <location filename="../dbandroidpathdialog.cpp" line="448"/>
      <source>Invalid name</source>
      <translation>無效名稱</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="430"/>
      <source>Database with the same name (%1) already exists on the device.
The name must be unique.</source>
      <translation>裝置上已存在相同名稱的資料庫 (%1)。
名稱必須唯一。</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="439"/>
      <source>Could not create database &apos;%1&apos;, because could not connect to the device.</source>
      <translation>無法建立資料庫 &apos;%1&apos;，無法連線到該裝置。</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="448"/>
      <source>Could not create database &apos;%1&apos;.
Details: %2</source>
      <translation>無法建立資料庫 &apos;%1&apos;。
詳細資訊：%2</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="463"/>
      <source>Delete database</source>
      <translation>刪除資料庫</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="463"/>
      <source>Are you sure you want to delete database &apos;%1&apos; from %2?</source>
      <translation>確定從 %2 中刪除資料庫 &apos;%1&apos; 嗎？</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="484"/>
      <location filename="../dbandroidpathdialog.cpp" line="490"/>
      <source>Error deleting</source>
      <translation>刪除出錯</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="484"/>
      <source>Could not connect to %1 in order to delete database &apos;%2&apos;.</source>
      <translation>無法連線到 %1 以刪除資料庫 &apos;%2&apos;。</translation>
    </message>
    <message>
      <location filename="../dbandroidpathdialog.cpp" line="490"/>
      <source>Could not delete database named &apos;%1&apos; from the device.
Android device refused deletion, or it was impossible.</source>
      <translation>無法從該裝置中刪除資料庫 &apos;%1&apos;。
Android 裝置拒絕或無法完成刪除。</translation>
    </message>
  </context>
  <context>
    <name>DbAndroidShellConnection</name>
    <message>
      <location filename="../dbandroidshellconnection.cpp" line="31"/>
      <source>Cannot connect to device %1, because it&apos;s not visible from your computer.</source>
      <translation type="unfinished">Cannot connect to device %1, because it&apos;s not visible from your computer.</translation>
    </message>
    <message>
      <location filename="../dbandroidshellconnection.cpp" line="46"/>
      <source>Cannot connect to device %1, because the application %2 doesn&apos;t seem to be installed on the device.</source>
      <translation>無法連線裝置 %1，應用程式 %2 似乎沒有在該裝置上安裝。</translation>
    </message>
    <message>
      <location filename="../dbandroidshellconnection.cpp" line="56"/>
      <source>Cannot connect to device %1, because the application %2 is not debuggable.</source>
      <translation>無法連線裝置 %1，應用程式 %2 不可除錯。</translation>
    </message>
    <message>
      <location filename="../dbandroidshellconnection.cpp" line="65"/>
      <source>Cannot connect to device %1, because &apos;%2&apos; command doesn&apos;t seem to be available on the device.</source>
      <translation>無法連線裝置 %1，命令 &apos;%2&apos; 在該裝置上似乎不可用。</translation>
    </message>
    <message>
      <location filename="../dbandroidshellconnection.cpp" line="77"/>
      <source>Cannot connect to device %1, because &apos;%2&apos; database cannot be accessed on the device.</source>
      <translation>無法連線裝置 %1，無法訪問該裝置上的 &apos;%2&apos; 資料庫。</translation>
    </message>
    <message>
      <location filename="../dbandroidshellconnection.cpp" line="90"/>
      <source>Cannot connect to device %1, because &apos;%2&apos; database cannot be accessed on the device. Details: %3</source>
      <translation>無法連線裝置 %1，無法訪問該裝置上的 &apos;%2&apos; 資料庫。詳細資訊：%3</translation>
    </message>
    <message>
      <location filename="../dbandroidshellconnection.cpp" line="126"/>
      <source>Cannot get list of databases for application %1. Details: %2</source>
      <translation>無法獲取應用程式 %1 的資料庫清單。詳細資訊：%2</translation>
    </message>
    <message>
      <location filename="../dbandroidshellconnection.cpp" line="219"/>
      <location filename="../dbandroidshellconnection.cpp" line="226"/>
      <source>Could not execute query on database &apos;%1&apos;: %2</source>
      <translation>無法在資料庫 &apos;%1&apos; 上執行查詢：%2</translation>
    </message>
  </context>
  <context>
    <name>QObject</name>
    <message>
      <location filename="../sqlqueryandroid.cpp" line="101"/>
      <source>Cannot bind argument &apos;%1&apos; of the query, because it&apos;s value is missing.</source>
      <translation>無法在此查詢中繫結引數 &apos;%1&apos;，因為缺少它的值。</translation>
    </message>
  </context>
</TS>
