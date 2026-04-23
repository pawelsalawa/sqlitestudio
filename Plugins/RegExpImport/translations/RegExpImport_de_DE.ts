<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="de" sourcelanguage="en">
  <context>
    <name>RegExpImport</name>
    <message>
      <location filename="../regexpimport.cpp" line="37"/>
      <source>Text files (*.txt);;All files (*)</source>
      <translation>Textdateien (*.txt);;Alle Dateien (*.*)</translation>
    </message>
    <message>
      <location filename="../regexpimport.cpp" line="53"/>
      <source>Cannot read file %1</source>
      <translation>Die Datei %1 kann nicht gelesen werden.</translation>
    </message>
    <message>
      <location filename="../regexpimport.cpp" line="161"/>
      <source>Enter the regular expression pattern.</source>
      <translation>Geben Sie das Muster für reguläre Ausdrücke ein.</translation>
    </message>
    <message>
      <location filename="../regexpimport.cpp" line="169"/>
      <source>Invalid pattern: %1</source>
      <translation>Ungültiges Muster: %1</translation>
    </message>
    <message>
      <location filename="../regexpimport.cpp" line="189"/>
      <source>Requested capture index %1 is out of range.</source>
      <translation>Der angeforderte Aufnahme-Index %1 ist außerhalb des Gültigkeitsbereichs.</translation>
    </message>
    <message>
      <location filename="../regexpimport.cpp" line="196"/>
      <source>&lt;p&gt;Requested capture group name &apos;%1&apos;, but it&apos;s not defined in the pattern: &lt;pre&gt;%2&lt;/pre&gt;&lt;/p&gt;</source>
      <translation>&lt;p&gt;Forderte Aufnahme-Gruppenname &apos;%1&apos; an, aber es ist nicht definiert im Muster: &lt;pre&gt;%2&lt;/pre&gt;&lt;/p&gt;</translation>
    </message>
  </context>
  <context>
    <name>RegExpImportConfig</name>
    <message>
      <location filename="../regexpimport.ui" line="20"/>
      <source>Capture groups</source>
      <translation>Gruppen erfassen</translation>
    </message>
    <message>
      <location filename="../regexpimport.ui" line="26"/>
      <source>Treat all RegExp capture groups as columns</source>
      <translation>Alle RegExp-Aufnahme-Gruppen als Spalten behandeln</translation>
    </message>
    <message>
      <location filename="../regexpimport.ui" line="39"/>
      <source>Import only following groups:</source>
      <translation>Nur folgende Gruppen importieren:</translation>
    </message>
    <message>
      <location filename="../regexpimport.ui" line="52"/>
      <source>&lt;p&gt;Enter comma separated list of capture group indexes. The 0 index refers to the entire matched string.&lt;/p&gt;
&lt;p&gt;If you used named groups in the pattern, you can use names instead of indexes. You can mix indexes and names in this list.&lt;/p&gt;</source>
      <translation>&lt;p&gt;Geben Sie eine durch Komma getrennte Liste der Aufnahmegruppen-Indizes ein. Der 0 Index bezieht sich auf die gesamte übereinstimmenden Zeichkette.&lt;/p&gt;
&lt;p&gt;Wenn Sie benannte Gruppen im Muster verwendet haben, können Sie Namen anstelle von Indizes verwenden. Sie können Indizes und Namen in dieser Liste mischen.&lt;/p&gt;</translation>
    </message>
    <message>
      <location filename="../regexpimport.ui" line="56"/>
      <source>Example: 1, 3, 4</source>
      <translation>Beispiel: 1, 3, 4</translation>
    </message>
    <message>
      <location filename="../regexpimport.ui" line="69"/>
      <source>Pattern:</source>
      <translation>Muster:</translation>
    </message>
    <message>
      <location filename="../regexpimport.ui" line="76"/>
      <source>&lt;p&gt;Use Regular Expression groups to enclose parts of the expression that you want to import. If you want to use a group, that you don&apos;t want to import, then use &quot;import only following groups&quot; option below.

You can use named groups and refer to them in group list below. To name a group use: &lt;pre&gt;(?&amp;lt;myGroupName&amp;gt;\s+\d+\s+)&lt;/pre&gt;&lt;/p&gt;</source>
      <translation>&lt;p&gt;Verwenden Sie reguläre Ausdrucksgruppen, um Teile des Ausdrucks einzuschließen, den Sie importieren möchten. Wenn Sie eine Gruppe verwenden möchten, die Sie mit importieren möchten, dann verwenden Sie &quot;Import nur folgenden Gruppen&quot; Option unten.

Sie können benannte Gruppen verwenden und in der unten stehenden Gruppenliste darauf verweisen. Um eine Gruppe zu benennen, schreiben Sie: &lt;pre&gt;(?&amp;lt;myGroupName&amp;gt;\s+\d+\s+\s+)&lt;/pre&gt;&lt;/p&gt;</translation>
    </message>
    <message>
      <location filename="../regexpimport.ui" line="81"/>
      <source>Example: (\d+)\s+((\d+)\w+)\s+(\w+)</source>
      <translation>Beispiel: (\d+)\s+((\d+)\w+)\s+(\w+)</translation>
    </message>
  </context>
</TS>
