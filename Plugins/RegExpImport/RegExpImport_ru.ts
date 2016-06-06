<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="ru_RU">
<context>
    <name>RegExpImport</name>
    <message>
        <location filename="regexpimport.cpp" line="37"/>
        <source>Text files (*.txt);;All files (*)</source>
        <translation>Текстовые файлы (*.txt);;Все файлы (*)</translation>
    </message>
    <message>
        <location filename="regexpimport.cpp" line="53"/>
        <source>Cannot read file %1</source>
        <translation>Невозможно прочитать файл %1</translation>
    </message>
    <message>
        <location filename="regexpimport.cpp" line="161"/>
        <source>Enter the regular expression pattern.</source>
        <translation>Введите шаблон регулярного выражения.</translation>
    </message>
    <message>
        <location filename="regexpimport.cpp" line="169"/>
        <source>Invalid pattern: %1</source>
        <translation>Некорректный шаблон: %1</translation>
    </message>
    <message>
        <location filename="regexpimport.cpp" line="189"/>
        <source>Requested capture index %1 is out of range.</source>
        <translation>Запрошенный индекс группы вне досягаемости.</translation>
    </message>
    <message>
        <location filename="regexpimport.cpp" line="196"/>
        <source>&lt;p&gt;Requested capture group name &apos;%1&apos;, but it&apos;s not defined in the pattern: &lt;pre&gt;%2&lt;/pre&gt;&lt;/p&gt;</source>
        <translation>&lt;p&gt;Запрошено имя группы &apos;%1&apos;, но оно не определено в шаблоне: &lt;pre&gt;%2&lt;/pre&gt;&lt;/p&gt;</translation>
    </message>
</context>
<context>
    <name>RegExpImportConfig</name>
    <message>
        <location filename="regexpimport.ui" line="20"/>
        <source>Capture groups</source>
        <translation>Группы в шаблоне</translation>
    </message>
    <message>
        <location filename="regexpimport.ui" line="26"/>
        <source>Treat all RegExp capture groups as columns</source>
        <translation>Рассматривать все группы в выражении как столбцы</translation>
    </message>
    <message>
        <location filename="regexpimport.ui" line="39"/>
        <source>Import only following groups:</source>
        <translation>Импортировать только следующие группы:</translation>
    </message>
    <message>
        <location filename="regexpimport.ui" line="52"/>
        <source>&lt;p&gt;Enter comma separated list of capture group indexes. The 0 index refers to the entire matched string.&lt;/p&gt;
&lt;p&gt;If you used named groups in the pattern, you can use names instead of indexes. You can mix indexes and names in this list.&lt;/p&gt;</source>
        <translation>&lt;p&gt;Введите разделённый запятыми список индексов групп. Индекс 0 служит для доступа ко всей найденной строке.&lt;/p&gt;
&lt;p&gt;При использовании именованных групп в шаблоне, вы можете указывать имена групп вместо индексов. Имена групп и индексы можно использовать вместе.&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="regexpimport.ui" line="56"/>
        <source>Example: 1, 3, 4</source>
        <translation>Пример: 1, 3, 4</translation>
    </message>
    <message>
        <location filename="regexpimport.ui" line="69"/>
        <source>Pattern:</source>
        <translation>Шаблон:</translation>
    </message>
    <message>
        <location filename="regexpimport.ui" line="76"/>
        <source>&lt;p&gt;Use Regular Expression groups to enclose parts of the expression that you want to import. If you want to use a group, that you don&apos;t want to import, then use &quot;import only following groups&quot; option below.

You can use named groups and refer to them in group list below. To name a group use: &lt;pre&gt;(?&amp;lt;myGroupName&amp;gt;\s+\d+\s+)&lt;/pre&gt;&lt;/p&gt;</source>
        <translation>&lt;p&gt;Используйте группировку в регулярном выражении для выделения частей выражения, которые необходимо импортировать. Если необходимо использовать группу, исключаемую при импорте, используйте опцию &quot;Импортировать только следующие группы&quot; ниже.

Можно также использовать именованные группы для использования в списке ниже. Для присвоения группе имени используйте: &lt;pre&gt;(?&amp;lt;моёИмяГруппы&amp;gt;\s+\d+\s+)&lt;/pre&gt;&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="regexpimport.ui" line="81"/>
        <source>Example: (\d+)\s+((\d+)\w+)\s+(\w+)</source>
        <translation>Пример: (\d+)\s+((\d+)\w+)\s+(\w+)</translation>
    </message>
</context>
</TS>
