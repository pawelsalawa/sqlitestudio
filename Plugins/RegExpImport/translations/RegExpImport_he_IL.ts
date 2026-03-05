<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="he" sourcelanguage="en">
<context>
    <name>RegExpImport</name>
    <message>
        <location filename="../regexpimport.cpp" line="37"/>
        <source>Text files (*.txt);;All files (*)</source>
        <translation>קבצי מלל (*.txt);;כל הקבצים (*)</translation>
    </message>
    <message>
        <location filename="../regexpimport.cpp" line="53"/>
        <source>Cannot read file %1</source>
        <translation>לא ניתן לקרוא קובץ %1</translation>
    </message>
    <message>
        <location filename="../regexpimport.cpp" line="161"/>
        <source>Enter the regular expression pattern.</source>
        <translation>הזנת דפוס ביטוי-רגיל.</translation>
    </message>
    <message>
        <location filename="../regexpimport.cpp" line="169"/>
        <source>Invalid pattern: %1</source>
        <translation>דפוס שגוי: %1</translation>
    </message>
    <message>
        <location filename="../regexpimport.cpp" line="189"/>
        <source>Requested capture index %1 is out of range.</source>
        <translation>מפתח לכידה מבוקש %1 מחוץ לטווח.</translation>
    </message>
    <message>
        <location filename="../regexpimport.cpp" line="196"/>
        <source>&lt;p&gt;Requested capture group name &apos;%1&apos;, but it&apos;s not defined in the pattern: &lt;pre&gt;%2&lt;/pre&gt;&lt;/p&gt;</source>
        <translation>&lt;p&gt;שם קבוצת הלכידה המבוקש &apos;%1&apos;, אך הוא &apos; אינו מוגדר בתבנית: &lt;pre&gt;%2&lt;/pre&gt;&lt;/p&gt;</translation>
    </message>
</context>
<context>
    <name>RegExpImportConfig</name>
    <message>
        <location filename="../regexpimport.ui" line="20"/>
        <source>Capture groups</source>
        <translation>לכידת קבוצות</translation>
    </message>
    <message>
        <location filename="../regexpimport.ui" line="26"/>
        <source>Treat all RegExp capture groups as columns</source>
        <translation>להתייחס לכל קבוצות הלכידה של ביטוי-רגיל כעמודות</translation>
    </message>
    <message>
        <location filename="../regexpimport.ui" line="39"/>
        <source>Import only following groups:</source>
        <translation>יבוא הקבוצות הבאות בלבד:</translation>
    </message>
    <message>
        <location filename="../regexpimport.ui" line="52"/>
        <source>&lt;p&gt;Enter comma separated list of capture group indexes. The 0 index refers to the entire matched string.&lt;/p&gt;
&lt;p&gt;If you used named groups in the pattern, you can use names instead of indexes. You can mix indexes and names in this list.&lt;/p&gt;</source>
        <translation>&lt;p&gt;נא להזין רשימה מופרדת בפסיקים של מפתחי קבוצות לכידה. מפתח 0 מתייחס לכל המחרוזת התואמת.&lt;/p&gt;
&lt;p&gt; אם נעשה שימוש בקבוצות שם בתבנית, ניתן להשתמש בשמות במקום במפתחים. ניתן לערבב מפתחים ושמות ברשימה זו.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../regexpimport.ui" line="56"/>
        <source>Example: 1, 3, 4</source>
        <translation>דוגמה: 1, 3, 4</translation>
    </message>
    <message>
        <location filename="../regexpimport.ui" line="69"/>
        <source>Pattern:</source>
        <translation>תבנית:</translation>
    </message>
    <message>
        <location filename="../regexpimport.ui" line="76"/>
        <source>&lt;p&gt;Use Regular Expression groups to enclose parts of the expression that you want to import. If you want to use a group, that you don&apos;t want to import, then use &quot;import only following groups&quot; option below.

You can use named groups and refer to them in group list below. To name a group use: &lt;pre&gt;(?&amp;lt;myGroupName&amp;gt;\s+\d+\s+)&lt;/pre&gt;&lt;/p&gt;</source>
        <translation>&lt;p&gt;ניתן להשתמש בקבוצות ביטוי רגיל כדי למסגר חלקים מהביטוי אותם מתכוונים לייבא. אם הכוונה להשתמש בקבוצה שאותה לא מעוניינים לייבא, נא להשתמש באפשרות &quot;ייבוא הקבוצות הבאות בלבד&quot; מטה.

ניתן להשתמש בקבוצות שם ולהתייחס אליהם ברשימת הקבוצות שלמטה. כדי לתת שם לקבוצה נא להשתמש ב: &lt;pre&gt;(?&amp;lt;myGroupName&amp;gt;\s+\d+\s+)&lt;/pre&gt;&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../regexpimport.ui" line="81"/>
        <source>Example: (\d+)\s+((\d+)\w+)\s+(\w+)</source>
        <translation>דוגמה: (\d+)\s+((\d+)\w+)\s+(\w+)</translation>
    </message>
</context>
</TS>
