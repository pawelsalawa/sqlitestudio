<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="he" sourcelanguage="en">
  <context>
    <name>ConfigMigration</name>
    <message>
      <location filename="../configmigration.cpp" line="36"/>
      <source>A configuration from old SQLiteStudio 2.x.x has been detected. Would you like to migrate old settings into the current version? &lt;a href=&quot;%1&quot;&gt;Click here to do that&lt;/a&gt;.</source>
      <translation>זוהתה תצורה ישנה מגרסת SQLiteStudio 2.x.x. האם להעביר הגדרות ישנות לגרסה הנוכחית? &lt;a href=&quot;%1&quot;&gt;הקשה לאישור ביצוע&lt;/a&gt;.</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="136"/>
      <source>Bug reports history (%1)</source>
      <translation>היסטורית דיווח תקלים (%1)</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="145"/>
      <source>Database list (%1)</source>
      <translation>רשימת מסד נתונים (%1)</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="154"/>
      <source>Custom SQL functions (%1)</source>
      <translation>תפקודי SQK מותאמים (%1)</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="163"/>
      <source>SQL queries history (%1)</source>
      <translation>היסטוריית שאילתות SQL (%1)</translation>
    </message>
  </context>
  <context>
    <name>ConfigMigrationWizard</name>
    <message>
      <location filename="../configmigrationwizard.ui" line="14"/>
      <source>Configuration migration</source>
      <translation>הגירת תצורה</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="24"/>
      <source>Items to migrate</source>
      <translation>פריטים להגירה</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="27"/>
      <source>This is a list of items found in the old configuration file, which can be migrated into the current configuration.</source>
      <translation>זוהי רשימה של פריטים שנמצאו בקובץ התצורה הישן, אותם ניתן להעביר לתצורה הנוכחית.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="58"/>
      <source>Options</source>
      <translation>אפשרויות</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="64"/>
      <source>Put imported databases into separate group</source>
      <translation>הכנסת מסדי נתונים מיובאים לקבוצה נפרדת</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="76"/>
      <source>Group name</source>
      <translation>שם קבוצה</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="60"/>
      <source>Enter a non-empty name.</source>
      <translation>נא להזין שם (לא ריק).</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="68"/>
      <source>Top level group named &apos;%1&apos; already exists. Enter a group name that does not exist yet.</source>
      <translation>קבוצה ברמה עליונה בשם &apos;%1&apos; קיימת כבר. נא להזין שם קבוצה שטרם נוצרה.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="102"/>
      <source>Could not open old configuration file in order to migrate settings from it.</source>
      <translation>לא ניתן לפתוח קובץ תצורה ישן על מנת לבצע הגירת הגדרות ממנו.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="110"/>
      <source>Could not open current configuration file in order to migrate settings from old configuration file.</source>
      <translation>לא ניתן לפתוח קובץ תצורה נוכחי על מנת לבצע הגירת הגדרות מקובץ תצורה ישן.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="119"/>
      <source>Could not commit migrated data into new configuration file: %1</source>
      <translation type="unfinished">Could not commit migrated data into new configuration file: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="163"/>
      <source>Could not read bug reports history from old configuration file in order to migrate it: %1</source>
      <translation>לא ניתן לקרוא את היסטוריית דוחות התקלים מקובץ התצורה הישן כדי להעבירו: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="180"/>
      <source>Could not insert a bug reports history entry into new configuration file: %1</source>
      <translation>לא ניתן להוסיף את רשומת היסטוריית דוחות התקלים לקובץ תצורה חדש: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="201"/>
      <source>Could not read database list from old configuration file in order to migrate it: %1</source>
      <translation>לא ניתן לקרוא את רשימת מסדי הנתונים מקובץ התצורה הישן כדי להסב אותו: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="215"/>
      <source>Could not query for available order for containing group in new configuration file in order to migrate the database list: %1</source>
      <translation>לא התאפשרה שאילתה על קבוצת הכלה וסדר זמין בקובץ התצורה החדש, על מנת להעביר את רשימת מסדי הנתונים: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="226"/>
      <source>Could not create containing group in new configuration file in order to migrate the database list: %1</source>
      <translation>לא ניתן ליצור קבוצת הכלה בקובץ התצורה החדש, על מנת להעביר את רשימת מסדי הנתונים: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="247"/>
      <source>Could not insert a database entry into new configuration file: %1</source>
      <translation>לא ניתן להוסיף את רשומת מסד נתונים לקובץ תצורה חדש: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="259"/>
      <source>Could not query for available order for next database in new configuration file in order to migrate the database list: %1</source>
      <translation type="unfinished">Could not query for available order for next database in new configuration file in order to migrate the database list: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="270"/>
      <source>Could not create group referencing the database in new configuration file: %1</source>
      <translation type="unfinished">Could not create group referencing the database in new configuration file: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="288"/>
      <source>Could not read function list from old configuration file in order to migrate it: %1</source>
      <translation type="unfinished">Could not read function list from old configuration file in order to migrate it: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="323"/>
      <source>Could not read SQL queries history from old configuration file in order to migrate it: %1</source>
      <translation type="unfinished">Could not read SQL queries history from old configuration file in order to migrate it: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="330"/>
      <source>Could not read next ID for SQL queries history in new configuration file: %1</source>
      <translation type="unfinished">Could not read next ID for SQL queries history in new configuration file: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="346"/>
      <source>Could not insert SQL history entry into new configuration file: %1</source>
      <translation type="unfinished">Could not insert SQL history entry into new configuration file: %1</translation>
    </message>
  </context>
</TS>
