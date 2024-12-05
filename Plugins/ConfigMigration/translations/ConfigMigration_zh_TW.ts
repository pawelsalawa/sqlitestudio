<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh-TW" sourcelanguage="en">
  <context>
    <name>ConfigMigration</name>
    <message>
      <location filename="../configmigration.cpp" line="36"/>
      <source>A configuration from old SQLiteStudio 2.x.x has been detected. Would you like to migrate old settings into the current version? &lt;a href=&quot;%1&quot;&gt;Click here to do that&lt;/a&gt;.</source>
      <translation>偵測到舊的 SQLiteStudio 2.x.x 的設定檔。你想要將舊的設定遷移到當前版本嗎？&lt;a href=&quot;%1&quot;&gt;點選這裡進行遷移&lt;/a&gt;。</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="139"/>
      <source>Bug reports history (%1)</source>
      <translation>錯誤報告歷史 (%1)</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="148"/>
      <source>Database list (%1)</source>
      <translation>資料庫清單 (%1)</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="157"/>
      <source>Custom SQL functions (%1)</source>
      <translation>自訂 SQL 函式 (%1)</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="166"/>
      <source>SQL queries history (%1)</source>
      <translation>SQL 查詢歷史 (%1)</translation>
    </message>
  </context>
  <context>
    <name>ConfigMigrationWizard</name>
    <message>
      <location filename="../configmigrationwizard.ui" line="14"/>
      <source>Configuration migration</source>
      <translation>設定檔遷移</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="24"/>
      <source>Items to migrate</source>
      <translation>要遷移的項</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="27"/>
      <source>This is a list of items found in the old configuration file, which can be migrated into the current configuration.</source>
      <translation>這是在舊的設定檔檔案中找到的項清單，這些項可以遷移到當前設定檔。</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="58"/>
      <source>Options</source>
      <translation>選項</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="64"/>
      <source>Put imported databases into separate group</source>
      <translation>將匯入的資料庫置於單獨的組</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="76"/>
      <source>Group name</source>
      <translation>組名稱</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="62"/>
      <source>Enter a non-empty name.</source>
      <translation>請輸入一個非空的名稱。</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="70"/>
      <source>Top level group named &apos;%1&apos; already exists. Enter a group name that does not exist yet.</source>
      <translation>名為 &apos;%1&apos; 的頂級組已存在。請輸入未被佔用的組名稱。</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="104"/>
      <source>Could not open old configuration file in order to migrate settings from it.</source>
      <translation>無法開啟舊的設定檔檔案以遷移它的設定。</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="112"/>
      <source>Could not open current configuration file in order to migrate settings from old configuration file.</source>
      <translation>無法開啟當前的設定檔檔案以遷移舊的設定。</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="121"/>
      <source>Could not commit migrated data into new configuration file: %1</source>
      <translation>無法將遷移的資料提交到新的設定檔檔案：%1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="165"/>
      <source>Could not read bug reports history from old configuration file in order to migrate it: %1</source>
      <translation>無法從舊的設定檔檔案讀取錯誤報告歷史記錄來進行遷移：%1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="182"/>
      <source>Could not insert a bug reports history entry into new configuration file: %1</source>
      <translation>無法將錯誤報告歷史記錄插入到新的設定檔檔案：%1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="203"/>
      <source>Could not read database list from old configuration file in order to migrate it: %1</source>
      <translation>無法從舊的設定檔檔案讀取資料庫清單來進行遷移：%1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="217"/>
      <source>Could not query for available order for containing group in new configuration file in order to migrate the database list: %1</source>
      <translation>無法在新設定檔檔案中查詢包含組的可用順序以遷移資料庫清單：%1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="228"/>
      <source>Could not create containing group in new configuration file in order to migrate the database list: %1</source>
      <translation>無法在新設定檔檔案中建立包含組以遷移資料庫清單：%1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="249"/>
      <source>Could not insert a database entry into new configuration file: %1</source>
      <translation>無法將資料庫條目插入到新的設定檔檔案：%1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="261"/>
      <source>Could not query for available order for next database in new configuration file in order to migrate the database list: %1</source>
      <translation>無法在新設定檔檔案中查詢下一個資料庫的可用順序以遷移資料庫清單：%1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="272"/>
      <source>Could not create group referencing the database in new configuration file: %1</source>
      <translation>未能在新的設定檔檔案中建立引用該資料庫的組：%1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="290"/>
      <source>Could not read function list from old configuration file in order to migrate it: %1</source>
      <translation>無法從舊設定檔檔案讀取函式清單來進行遷移：%1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="325"/>
      <source>Could not read SQL queries history from old configuration file in order to migrate it: %1</source>
      <translation>無法從舊設定檔檔案讀取 SQL 查詢歷史來進行遷移：%1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="332"/>
      <source>Could not read next ID for SQL queries history in new configuration file: %1</source>
      <translation>無法在新設定檔檔案中讀取 SQL 查詢歷史的下一個 ID：%1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="348"/>
      <source>Could not insert SQL history entry into new configuration file: %1</source>
      <translation>無法將 SQL 歷史條目插入到新的設定檔檔案：%1</translation>
    </message>
  </context>
</TS>
