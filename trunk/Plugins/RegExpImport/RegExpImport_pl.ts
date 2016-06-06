<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="pl_PL">
<context>
    <name>RegExpImport</name>
    <message>
        <location filename="regexpimport.cpp" line="37"/>
        <source>Text files (*.txt);;All files (*)</source>
        <translation>Pliki tekstowe (*.txt);;Wszystkie pliki (*)</translation>
    </message>
    <message>
        <location filename="regexpimport.cpp" line="53"/>
        <source>Cannot read file %1</source>
        <translation>Nie można odczytać pliku %1</translation>
    </message>
    <message>
        <location filename="regexpimport.cpp" line="161"/>
        <source>Enter the regular expression pattern.</source>
        <translation>Wprowadź wzorzec wyrażenia regularnego.</translation>
    </message>
    <message>
        <location filename="regexpimport.cpp" line="169"/>
        <source>Invalid pattern: %1</source>
        <translation>Niepoprawny wzorzec: %1</translation>
    </message>
    <message>
        <location filename="regexpimport.cpp" line="189"/>
        <source>Requested capture index %1 is out of range.</source>
        <translation>Żądany indeks przechwytywania %1 jest poza zakresem.</translation>
    </message>
    <message>
        <location filename="regexpimport.cpp" line="196"/>
        <source>&lt;p&gt;Requested capture group name &apos;%1&apos;, but it&apos;s not defined in the pattern: &lt;pre&gt;%2&lt;/pre&gt;&lt;/p&gt;</source>
        <translation>&lt;p&gt;Zażądano grupy do przechwycenia o nazwie &apos;%1&apos;, ale nie jest ona zdefiniowana we wzorcu: &lt;pre&gt;%2&lt;/pre&gt;&lt;/p&gt;</translation>
    </message>
</context>
<context>
    <name>RegExpImportConfig</name>
    <message>
        <location filename="regexpimport.ui" line="20"/>
        <source>Capture groups</source>
        <translation>Grupy przechwytujące</translation>
    </message>
    <message>
        <location filename="regexpimport.ui" line="26"/>
        <source>Treat all RegExp capture groups as columns</source>
        <translation>Traktuj wszystkie grupy przechwytujące jako kolumny</translation>
    </message>
    <message>
        <location filename="regexpimport.ui" line="39"/>
        <source>Import only following groups:</source>
        <translation>Importuj tylko następujące grupy:</translation>
    </message>
    <message>
        <location filename="regexpimport.ui" line="52"/>
        <source>&lt;p&gt;Enter comma separated list of capture group indexes. The 0 index refers to the entire matched string.&lt;/p&gt;
&lt;p&gt;If you used named groups in the pattern, you can use names instead of indexes. You can mix indexes and names in this list.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Wprowadź listę indeksów grup oddzieloną przecinkami. Indeks 0 odpowiada całemu dopasowanemu łańcuchowi.&lt;/p&gt;
&lt;p&gt;Jeśli użyłeś nazwanych grup we wzorcu, to możesz używać nazw, zamiast indeksów. Możesz mieszać inseksy i nazwy na liście.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="regexpimport.ui" line="56"/>
        <source>Example: 1, 3, 4</source>
        <translation>Przykład: 1, 3, 4</translation>
    </message>
    <message>
        <location filename="regexpimport.ui" line="69"/>
        <source>Pattern:</source>
        <translation>Wzorzec:</translation>
    </message>
    <message>
        <location filename="regexpimport.ui" line="76"/>
        <source>&lt;p&gt;Use Regular Expression groups to enclose parts of the expression that you want to import. If you want to use a group, that you don&apos;t want to import, then use &quot;import only following groups&quot; option below.

You can use named groups and refer to them in group list below. To name a group use: &lt;pre&gt;(?&amp;lt;myGroupName&amp;gt;\s+\d+\s+)&lt;/pre&gt;&lt;/p&gt;</source>
        <translation>&lt;p&gt;Użyj grup przechwytujących Wyrażeń Regularnych, aby otoczyć części wyrażenia, które chcesz zaimportować. Jeśli chcesz użyć grupy, której nie chcesz zaimportować, to użyj opcji &quot;importuj tylko następujące grupy&quot; poniżej.

Możesz użyć grup nazwanych i odwoływać się do nich w liście grup poniżej. Aby nazwać grupę, użyj: &lt;pre&gt;(?&amp;lt;nazwaGrupy&amp;gt;\s+\d+\s+)&lt;/pre&gt;&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="regexpimport.ui" line="81"/>
        <source>Example: (\d+)\s+((\d+)\w+)\s+(\w+)</source>
        <translation>Przykład: (\d+)\s+((\d+)\w+)\s+(\w+)</translation>
    </message>
</context>
</TS>
