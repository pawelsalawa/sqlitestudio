<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ru" sourcelanguage="en">
  <context>
    <name>CLI</name>
    <message>
      <location filename="../cli.cpp" line="98"/>
      <source>Current database: %1</source>
      <translation>Текущая база данных: %1</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="100"/>
      <source>No current working database is set.</source>
      <translation>Текущая рабочая база данных не определена.</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="102"/>
      <source>Type %1 for help</source>
      <translation>Введите %1 для вызова справки</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="254"/>
      <source>Database passed in command line parameters (%1) was already on the list under name: %2</source>
      <translation>База данных, переданная через аргументы командной строки (%1), уже находится в списке под именем %2</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="262"/>
      <source>Could not add database %1 to list.</source>
      <translation>Невозможно добавить базу данных %1 в список.</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="289"/>
      <source>closed</source>
      <translation>закрыта</translation>
    </message>
  </context>
  <context>
    <name>CliCommand</name>
    <message>
      <location filename="../commands/clicommand.cpp" line="107"/>
      <source>Usage: %1%2</source>
      <translation>Использование: %1%2</translation>
    </message>
  </context>
  <context>
    <name>CliCommandAdd</name>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="9"/>
      <source>Could not add database %1 to list.</source>
      <translation>Невозможно добавить базу данных %1 в список.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="14"/>
      <source>Database added: %1</source>
      <translation>Добавлена база данных: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="19"/>
      <source>adds new database to the list</source>
      <translation>добавляет новую базу данных в список</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="24"/>
      <source>Adds given database pointed by &lt;path&gt; with given &lt;name&gt; to list the databases list. The &lt;name&gt; is just a symbolic name that you can later refer to. Just pick any unique name. For list of databases already on the list use %1 command.</source>
      <translation>Добавляет базу данных, расположенную по указанному &lt;пути&gt; под указанным &lt;именем&gt; в список баз данных. &lt;имя&gt; — это обычное символьное имя, которое в дальнейшем можно будет использовать. Выберите любое уникальное имя. Для получения текущего списка баз данных воспользуйтесь командой %1.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="34"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>имя</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="35"/>
      <source>path</source>
      <comment>CLI command syntax</comment>
      <translation>путь</translation>
    </message>
  </context>
  <context>
    <name>CliCommandCd</name>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="10"/>
      <source>Changed directory to: %1</source>
      <translation>Изменён каталог на: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="12"/>
      <source>Could not change directory to: %1</source>
      <translation>Невозможно сменить каталог на: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="17"/>
      <source>changes current working directory</source>
      <translation>изменение текущего рабочего каталога</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="22"/>
      <source>Very similar command to &apos;cd&apos; known from Unix systems and Windows. It requires a &lt;path&gt; argument to be passed, therefore calling %1 will always cause a change of the directory. To learn what&apos;s the current working directory use %2 command and to list contents of the current working directory use %3 command.</source>
      <translation>Аналог команды &apos;cd&apos; из систем Unix и Windows. Требуется указание параметра &lt;путь&gt;, поэтому вызов %1 всегда приводит к изменению каталога. Для определения текущего рабочего каталога воспользуйтесь командой %2. Для вывода содержимого текущего рабочего каталога воспользуйтесь командой %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="33"/>
      <source>path</source>
      <comment>CLI command syntax</comment>
      <translation>путь</translation>
    </message>
  </context>
  <context>
    <name>CliCommandClose</name>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="10"/>
      <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
      <translation>Невозможно вызвать %1, если ни одна база данных не является текущей. Укажите текущую базу данных, используя команду %2 или укажите имя базы данных при вызове %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="21"/>
      <location filename="../commands/clicommandclose.cpp" line="29"/>
      <source>Connection to database %1 closed.</source>
      <translation>Соединение с базой данных %1 закрыто.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="24"/>
      <source>No such database: %1. Use %2 to see list of known databases.</source>
      <translation>Не найдена база данных: %1. Для получения списка доступных баз данных воспользуйтесь командой %2.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="35"/>
      <source>closes given (or current) database</source>
      <translation>закрывает указанную (или текущую) базу данных</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="40"/>
      <source>Closes database connection. If the database was already closed, nothing happens. If &lt;name&gt; is provided, it should be name of the database to close (as printed by %1 command). The the &lt;name&gt; is not provided, then current working database is closed (see help for %2 for details).</source>
      <translation>Закрывает соединение с базой данных. Если база данных уже закрыта, ничего не произойдёт. Если указано &lt;имя&gt;, оно должно соответствовать имени закрываемой базы данных (которое выводится командой %1). Если имя не указано, будет закрыта текущая рабочая база данных (смотрите справку по команде %2 для подробностей).</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="50"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>имя</translation>
    </message>
  </context>
  <context>
    <name>CliCommandDbList</name>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="12"/>
      <source>No current working database defined.</source>
      <translation>Не указана текущая рабочая база данных.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="18"/>
      <source>Databases:</source>
      <translation>Базы данных:</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="23"/>
      <location filename="../commands/clicommanddblist.cpp" line="34"/>
      <source>Name</source>
      <comment>CLI db name column</comment>
      <translation>Имя</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="31"/>
      <location filename="../commands/clicommanddblist.cpp" line="61"/>
      <source>Open</source>
      <comment>CLI connection state column</comment>
      <translation>Открыто</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="31"/>
      <location filename="../commands/clicommanddblist.cpp" line="61"/>
      <source>Closed</source>
      <comment>CLI connection state column</comment>
      <translation>Закрыто</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="32"/>
      <location filename="../commands/clicommanddblist.cpp" line="36"/>
      <source>Connection</source>
      <comment>CLI connection state column</comment>
      <translation>Соединение</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="38"/>
      <location filename="../commands/clicommanddblist.cpp" line="45"/>
      <source>Database file path</source>
      <translation>Путь к файлу базы данных</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="70"/>
      <source>prints list of registered databases</source>
      <translation>выводит список зарегистрированных баз данных</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="75"/>
      <source>Prints list of databases registered in the SQLiteStudio. Each database on the list can be in open or closed state and %1 tells you that. The current working database (aka default database) is also marked on the list with &apos;*&apos; at the start of its name. See help for %2 command to learn about the default database.</source>
      <translation>Выводит список баз данных, зарегистрированных в SQLiteStudio. Каждая база данных может быть либо открыта, либо закрыта, %1 это также указывает. Текущая рабочая база данных (она же база данных по умолчанию) дополнительно отмечена символом &apos;*&apos; в начале имени. Смотрите справку по команде %2 для сведений о базе данных по умолчанию.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandDesc</name>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="15"/>
      <source>No working database is set.
Call %1 command to set working database.
Call %2 to see list of all databases.</source>
      <translation>Не указана рабочая база данных. 
Укажите рабочую базу данных командой %1. 
Для просмотра списка баз данных воспользуйтесь командой %2.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="26"/>
      <source>Database is not open.</source>
      <translation>База данных не открыта.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="35"/>
      <source>Cannot find table named: %1</source>
      <translation>Не удалось найти таблицу с именем %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="52"/>
      <source>shows details about the table</source>
      <translation>отображает сведения о таблице</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="63"/>
      <source>table</source>
      <translation>таблица</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="70"/>
      <source>Table: %1</source>
      <translation>Таблица: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="74"/>
      <source>Column name</source>
      <translation>Имя столбца</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="76"/>
      <source>Data type</source>
      <translation>Тип данных</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="80"/>
      <source>Constraints</source>
      <translation>Ограничения</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="105"/>
      <source>Virtual table: %1</source>
      <translation>Виртуальная таблица: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="109"/>
      <source>Construction arguments:</source>
      <translation>Параметры создания:</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="114"/>
      <source>No construction arguments were passed for this virtual table.</source>
      <translation>Не указаны параметры создания для этой виртуальной таблицы.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandDir</name>
    <message>
      <location filename="../commands/clicommanddir.cpp" line="33"/>
      <source>lists directories and files in current working directory</source>
      <translation>выводит список каталогов и файлов в текущем рабочем каталоге</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddir.cpp" line="38"/>
      <source>This is very similar to &apos;dir&apos; command known from Windows and &apos;ls&apos; command from Unix systems.

You can pass &lt;pattern&gt; with wildcard characters to filter output.</source>
      <translation>Аналог команды &apos;dir&apos; в Windows и &apos;ls&apos; в системах Unix.

Вы можете указать &lt;маску&gt; c использованием подстановочных символов для фильтрации вывода.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddir.cpp" line="49"/>
      <source>pattern</source>
      <translation>маска</translation>
    </message>
  </context>
  <context>
    <name>CliCommandExit</name>
    <message>
      <location filename="../commands/clicommandexit.cpp" line="12"/>
      <source>quits the application</source>
      <translation>выход из приложения</translation>
    </message>
    <message>
      <location filename="../commands/clicommandexit.cpp" line="17"/>
      <source>Quits the application. Settings are stored in configuration file and will be restored on next startup.</source>
      <translation>Осуществляет выход из приложения. Настройки сохраняются в конфигурационном файле и восстановятся при следующем запуске.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandHelp</name>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="16"/>
      <source>shows this help message</source>
      <translation>вывод этого сообщения</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="21"/>
      <source>Use %1 to learn about certain commands supported by the command line interface (CLI) of the SQLiteStudio.
To see list of supported commands, type %2 without any arguments.

When passing &lt;command&gt; name, you can skip special prefix character (&apos;%3&apos;).

You can always execute any command with exactly single &apos;--help&apos; option to see help for that command. It&apos;s an alternative for typing: %1 &lt;command&gt;.</source>
      <translation>Используйте %1 для получения сведений о командах, поддерживаемых интерфейсом командной строки (CLI) SQLiteStudio.
Для просмотра списка доступных команд, введите %2 без указания аргументов.

При указании имени &lt;команды&gt; можно не указывать префиксный символ (&apos;%3&apos;).

Для получения справки по команде вы также можете выполнить команду с единственным ключом &apos;--help&apos;. Это альтернатива вводу: %1 &lt;команда&gt;.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="33"/>
      <source>command</source>
      <comment>CLI command syntax</comment>
      <translation>команда</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="42"/>
      <source>No such command: %1</source>
      <translation>Не найдена команда: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="43"/>
      <source>Type &apos;%1&apos; for list of available commands.</source>
      <translation>Введите &apos;%1&apos; для получения списка доступных команд.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="52"/>
      <source>Usage: %1%2</source>
      <translation>Использование: %1%2</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="62"/>
      <source>Aliases: %1</source>
      <translation>Псевдонимы: %1</translation>
    </message>
  </context>
  <context>
    <name>CliCommandHistory</name>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="23"/>
      <source>Current history limit is set to: %1</source>
      <translation>Текущий лимит истории: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="39"/>
      <source>prints history or erases it</source>
      <translation>выводит историю или очищает её</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="44"/>
      <source>When no argument was passed, this command prints command line history. Every history entry is separated with a horizontal line, so multiline entries are easier to read.

When the -c or --clear option is passed, then the history gets erased.
When the -l or --limit option is passed, it sets the new history entries limit. It requires an additional argument saying how many entries do you want the history to be limited to.
Use -ql or --querylimit option to see the current limit value.</source>
      <translation>При вызове без аргументов, данная команда выводит историю командной строки. Каждая запись истории отделена горизонтальной линией для облегчения чтения многострочных записей.

При вызове с ключом -с или --clear история очищается.
При вызове с ключом -l или --limit устанавливается новый лимит на количество записей в истории. Необходим дополнительный аргумент, указывающий сколько записей необходимо хранить в истории.
Для просмотра текущего лимита записей вызовите команду с ключом -ql или --querylimit.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="59"/>
      <source>number</source>
      <translation>количество</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="66"/>
      <source>Console history erased.</source>
      <translation>История командной строки очищена.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="75"/>
      <source>Invalid number: %1</source>
      <translation>Некорректное количество: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="80"/>
      <source>History limit set to %1</source>
      <translation>Лимит истории установлен в количестве %1</translation>
    </message>
  </context>
  <context>
    <name>CliCommandMode</name>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="9"/>
      <source>Current results printing mode: %1</source>
      <translation>Текущий режим вывода результатов: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="16"/>
      <source>Invalid results printing mode: %1</source>
      <translation>Некорректный режим вывода результатов: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="21"/>
      <source>New results printing mode: %1</source>
      <translation>Новый режим вывода результатов: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="26"/>
      <source>tells or changes the query results format</source>
      <translation>отображает или изменяет формат вывода результатов запроса</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="31"/>
      <source>When called without argument, tells the current output format for a query results. When the &lt;mode&gt; is passed, the mode is changed to the given one. Supported modes are:
- CLASSIC - columns are separated by a comma, not aligned,
- FIXED   - columns have equal and fixed width, they always fit into terminal window width, but the data in columns can be cut off,
- COLUMNS - like FIXED, but smarter (do not use with huge result sets, see details below),
- ROW     - each column from the row is displayed in new line, so the full data is displayed.

The CLASSIC mode is recommended if you want to see all the data, but you don&apos;t want to waste lines for each column. Each row will display full data for every column, but this also means, that columns will not be aligned to each other in next rows. The CLASSIC mode also doesn&apos;t respect the width of your terminal (console) window, so if values in columns are wider than the window, the row will be continued in next lines.

The FIXED mode is recommended if you want a readable output and you don&apos;t care about long data values. Columns will be aligned, making the output a nice table. The width of columns is calculated from width of the console window and a number of columns.

The COLUMNS mode is similar to FIXED mode, except it tries to be smart and make columns with shorter values more thin, while columns with longer values get more space. First to shrink are columns with longest headers (so the header names are to be cut off as first), then columns with the longest values are shrinked, up to the moment when all columns fit into terminal window.
ATTENTION! The COLUMNS mode reads all the results from the query at once in order to evaluate column widths, therefore it is dangerous to use this mode when working with huge result sets. Keep in mind that this mode will load entire result set into memory.

The ROW mode is recommended if you need to see whole values and you don&apos;t expect many rows to be displayed, because this mode displays a line of output per each column, so you&apos;ll get 10 lines for single row with 10 columns, then if you have 10 of such rows, you will get 100 lines of output (+1 extra line per each row, to separate rows from each other).</source>
      <translation>При вызове без аргумента, отображает текущий формат вывода для результатов запроса. При указании &lt;mode&gt; режим сменяется на выбранный. Поддерживаются следующие режимы:
- CLASSIC - столбцы разделяются запятой, без выравнивания,
- FIXED   - у столбцов равная и фиксированная ширина, они всегда умещаются в окно терминала по ширине, но часть данных в столбцах может быть обрезана,
- COLUMNS - как FIXED, но умнее (не используйте при огромных размерах результата запроса, см. подробности ниже),
- ROW     - каждый столбец каждой строки данных отображается на отдельной строке терминала, данные отображаются полностью.

Режим CLASSIC рекомендован если вы хотите увидеть все данные, но не хотите тратить новую строку в терминале для каждого столбца. Каждая строка будет отображать все данные из всех столбцов, однако столбцы не будут выровнены между собой по строкам. Режим CLASSIC также не учитывает ширину окна терминала (консоли), поэтому если значения в столбцах шире окна, выводимая строка данных будет продолжена в следующих строках окна терминала.

Режим FIXED рекомендован если вам нужен читабельный вывод и вас не беспокоят длинные значения в данных. Столбцы будут выровнены, образуя на выходе аккуратную таблицу. Ширина столбцов рассчитывается исходя из ширины окна консоли и числа столбцов.

Режим COLUMNS похож на режим FIXED с тем исключением, что он старается поступать по-умному и делать столбцы с короткими значениями поуже, чтобы столбцам с длинными значениями досталось больше места. Сначала урезаются столбцы с самыми длинными заголовками (так что имена столбцов будут урезаны в первую очередь), затем урезаются столбцы с самыми длинными значениями, пока все столбцы не уместятся в окне терминала.
ВНИМАНИЕ! Режим COLUMNS сразу считывает все результаты запроса чтобы рассчитать ширину столбцов, поэтому опасно использовать этот режим при работе с результатами огромных размеров. Учтите, что этот режим загружает весь результат запроса в память.

Режим ROW рекомендован если вам нужно видеть все данные, и вы не ожидаете большого числа строк данных в результате, поскольку в этом режиме каждый столбец выводится на отдельной строке терминала, поэтому вывод строки данных с 10 столбцами займёт 10 строк в терминале, и если таких строк с данными будет 10, то получится 100 строк вывода (+1 строка в терминале на каждую строку данных, для разделения).</translation>
    </message>
  </context>
  <context>
    <name>CliCommandNullValue</name>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="9"/>
      <source>Current NULL representation string: %1</source>
      <translation>Текущее представление значения NULL: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="15"/>
      <source>tells or changes the NULL representation string</source>
      <translation>отображает или устанавливает представление значения NULL</translation>
    </message>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="20"/>
      <source>If no argument was passed, it tells what&apos;s the current NULL value representation (that is - what is printed in place of NULL values in query results). If the argument is given, then it&apos;s used as a new string to be used for NULL representation.</source>
      <translation>При вызове без аргументов отображает текущее представление значения NULL (т.е. что выводится вместо значения NULL в результатах запроса). Если указан аргумент, он будет использован как строка для представления значения NULL.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandOpen</name>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="12"/>
      <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
      <translation>Невозможно вызвать %1, если ни одна база данных не является текущей. Укажите текущую базу данных, используя команду %2 или укажите имя базы данных при вызове %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="29"/>
      <source>Could not add database %1 to list.</source>
      <translation>Невозможно добавить базу данных %1 в список.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="37"/>
      <source>File %1 doesn&apos;t exist in %2. Cannot open inexisting database with %3 command. To create a new database, use %4 command.</source>
      <translation>Файл %1 не существует в %2. Невозможно открыть несуществующую базу данных командой %3. Для создания новой базы данных воспользуйтесь командой %4.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="61"/>
      <source>Database %1 has been open and set as the current working database.</source>
      <translation>База данных %1 была открыта и установлена в качестве текущей рабочей базы данных.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="66"/>
      <source>opens database connection</source>
      <translation>открывает соединение с базой данных</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="71"/>
      <source>Opens connection to the database. If no additional argument was passed, then the connection is open to the current default database (see help for %1 for details). However if an argument was passed, it can be either &lt;name&gt; of the registered database to open, or it can be &lt;path&gt; to the database file to open. In the second case, the &lt;path&gt; gets registered on the list with a generated name, but only for the period of current application session. After restarting application such database is not restored on the list.</source>
      <translation>Открывает соединение с базой данных. При вызове без аргументов, соединение открывается для текущей базы данных по умолчанию (см. справку по команде %1 для подробностей). Если же аргумент указан,он может быть &lt;именем&gt; зарегистрированной базы данных или &lt;путём&gt; к файлу базы данных. Во втором случае, база данных по указанному &lt;пути&gt; будет зарегистрирована в списке под сгенерированным именем, но только на время текущей сессии в приложении. После перезапуска приложения указанная база в списке восстановлена не будет.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="83"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>имя</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="83"/>
      <source>path</source>
      <comment>CLI command syntax</comment>
      <translation>путь</translation>
    </message>
  </context>
  <context>
    <name>CliCommandPwd</name>
    <message>
      <location filename="../commands/clicommandpwd.cpp" line="13"/>
      <source>prints the current working directory</source>
      <translation>отображение текущего рабочего каталога</translation>
    </message>
    <message>
      <location filename="../commands/clicommandpwd.cpp" line="18"/>
      <source>This is the same as &apos;pwd&apos; command on Unix systems and &apos;cd&apos; command without arguments on Windows. It prints current working directory. You can change the current working directory with %1 command and you can also list contents of the current working directory with %2 command.</source>
      <translation>Аналог команды &apos;pwd&apos; в системах Unix и команды &apos;cd&apos; без аргументов в Windows. Команда отображает текущий рабочий каталог. Вы можете сменить текущий рабочий каталог командой %1, а также вывести содержимое текущего рабочего каталога командой %2.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandRemove</name>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="12"/>
      <source>No such database: %1</source>
      <translation>Не найдена база данных: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="20"/>
      <source>Database removed: %1</source>
      <translation>Удалена база данных: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="26"/>
      <source>New current database set:</source>
      <translation>Установлена новая текущая база данных:</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="35"/>
      <source>removes database from the list</source>
      <translation>удаление базы данных из списка</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="40"/>
      <source>Removes &lt;name&gt; database from the list of registered databases. If the database was not on the list (see %1 command), then error message is printed and nothing more happens.</source>
      <translation>Удаляет базу данных с указанным &lt;именем&gt; из списка зарегистрированных баз данных. Если указанной базы данных нет в списке (см. команду %1), отображается сообщение об ошибке и больше ничего не происходит.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="50"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>имя</translation>
    </message>
  </context>
  <context>
    <name>CliCommandSql</name>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="19"/>
      <source>No working database is set.
Call %1 command to set working database.
Call %2 to see list of all databases.</source>
      <translation>Не указана рабочая база данных. Укажите рабочую базу данных командой %1. Для просмотра списка баз данных воспользуйтесь командой %2.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="30"/>
      <source>Database is not open.</source>
      <translation>База данных не открыта.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="65"/>
      <source>executes SQL query</source>
      <translation>выполнение запроса SQL</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="70"/>
      <source>This command is executed every time you enter SQL query in command prompt. It executes the query on the current working database (see help for %1 for details). There&apos;s no sense in executing this command explicitly. Instead just type the SQL query in the command prompt, without any command prefixed.</source>
      <translation>Эта команда выполняется каждый раз, когда вы вводите запрос SQL в командную строку. Она выполняет запрос к текущей рабочей базе данных (см. справку к команде %1 для подробностей). Не нужно явно вызвать эту команду. Просто вводите запрос SQL в командную строку без указания команды.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="86"/>
      <source>sql</source>
      <comment>CLI command syntax</comment>
      <translation>sql</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="135"/>
      <location filename="../commands/clicommandsql.cpp" line="177"/>
      <source>Too many columns to display in %1 mode.</source>
      <translation>Слишком много столбцов для отображения в режиме %1.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="254"/>
      <source>Row %1</source>
      <translation>Строка %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="404"/>
      <source>Query execution error: %1</source>
      <translation>Ошибка выполнения запроса: %1</translation>
    </message>
  </context>
  <context>
    <name>CliCommandTables</name>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="15"/>
      <source>No such database: %1. Use %2 to see list of known databases.</source>
      <translation>Не найдена база данных: %1. Для получения списка доступных баз данных воспользуйтесь командой %2.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="25"/>
      <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
      <translation>Невозможно вызвать %1, если ни одна база данных не является текущей. Укажите текущую базу данных, используя команду %2 или укажите имя базы данных при вызове %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="32"/>
      <source>Database %1 is closed.</source>
      <translation>База данных %1 закрыта.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="45"/>
      <location filename="../commands/clicommandtables.cpp" line="47"/>
      <source>Database</source>
      <translation>База данных</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="47"/>
      <source>Table</source>
      <translation>Таблица</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="61"/>
      <source>prints list of tables in the database</source>
      <translation>отображает список таблиц в базе данных</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="66"/>
      <source>Prints list of tables in given &lt;database&gt; or in the current working database. Note, that the &lt;database&gt; should be the name of the registered database (see %1). The output list includes all tables from any other databases attached to the queried database.
When the -s option is given, then system tables are also listed.</source>
      <translation>Отображает список таблиц в указанной &lt;базе данных&gt; или в текущей рабочей базе данных. Учтите, что &lt;база данных&gt; должна быть именем зарегистрированной базы данных (см. %1). В список тажк выводятся все таблицы из баз данных, присоединённых к запрашиваемой базе данных.
При указании ключа -s также выводятся системные таблицы.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="77"/>
      <source>database</source>
      <comment>CLI command syntax</comment>
      <translation>база данных</translation>
    </message>
  </context>
  <context>
    <name>CliCommandTree</name>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="12"/>
      <source>No current working database is selected. Use %1 to define one and then run %2.</source>
      <translation>Не выбрана рабочая база данных. Укажите рабочую базу данных командой %1, затем выполните команду %2.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="54"/>
      <source>Tables</source>
      <translation>Таблицы</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="58"/>
      <source>Views</source>
      <translation>Представления</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="83"/>
      <source>Columns</source>
      <translation>Столбцы</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="88"/>
      <source>Indexes</source>
      <translation>Индексы</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="92"/>
      <location filename="../commands/clicommandtree.cpp" line="113"/>
      <source>Triggers</source>
      <translation>Триггеры</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="132"/>
      <source>prints all objects in the database as a tree</source>
      <translation>отображение всех объектов базы данных в виде дерева</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="137"/>
      <source>Prints all objects (tables, indexes, triggers and views) that are in the database as a tree. The tree is very similar to the one that you can see in GUI client of the SQLiteStudio.
When -c option is given, then also columns will be listed under each table.
When -s option is given, then also system objects will be printed (sqlite_* tables, autoincrement indexes, etc).
The database argument is optional and if provided, then only given database will be printed. This is not a registered database name, but instead it&apos;s an internal SQLite database name, like &apos;main&apos;, &apos;temp&apos;, or any attached database name. To print tree for other registered database, call %1 first to switch the working database, and then use %2 command.</source>
      <translation>Отображает все объекты (таблицы, индексы, триггеры и представления) базы данных в виде дерева. Структура дерева аналогична тому, которое отображается в GUI клиенте SQLiteStudio.
При вызове с ключом -c также будут выведены столбцы под каждой таблицей.
При вызове с ключом -s также будут выведены системные объекты (таблицы sqlite_*, индексы автоинкремента и т.д.).
При вызове с необязательным аргументом &apos;база данных&apos; будут выведены объекты только указнной базы данных. Под &apos;базой данных&apos; подразумевается не зарегистрированное имя базы данных, а внутреннее имя базы данных SQLite, например &apos;main&apos;, &apos;temp&apos; или имя присоединённной базы данных. Для отображения дерева другой зарегистрированной базы данных, сперва смените рабочую базу данных командой %1, а затем воспользуйтесь командой %2.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandUse</name>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="13"/>
      <source>No current database selected.</source>
      <translation>Не выбрана текущая база данных.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="16"/>
      <location filename="../commands/clicommanduse.cpp" line="30"/>
      <source>Current database: %1</source>
      <translation>Текущая база данных: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="23"/>
      <source>No such database: %1</source>
      <translation>Не найдена база данных: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="35"/>
      <source>changes default working database</source>
      <translation>изменение рабочей базы данных по умолчанию</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="40"/>
      <source>Changes current working database to &lt;name&gt;. If the &lt;name&gt; database is not registered in the application, then the error message is printed and no change is made.

What is current working database?
When you type a SQL query to be executed, it is executed on the default database, which is also known as the current working database. Most of database-related commands can also work using default database, if no database was provided in their arguments. The current database is always identified by command line prompt. The default database is always defined (unless there is no database on the list at all).

The default database can be selected in various ways:
- using %1 command,
- by passing database file name to the application startup parameters,
- by passing registered database name to the application startup parameters,
- by restoring previously selected default database from saved configuration,
- or when default database was not selected by any of the above, then first database from the registered databases list becomes the default one.</source>
      <translation>Изменяет текущую рабочую базы данных на базу данных с указанным &lt;именем&gt;. Если &lt;имя&gt; базы данных не зарегистрировано в приложении, отображается сообщение об ошибке и изменения не производсятся.

Что такое текущая рабочая база данных?
Когда вы вводите запрос SQL для выполнения, он выполняется к базе данных по умолчанию, также известной как текущей рабочей базе данных. Большинство относящихся к базам данных команд могут выполняться к базе данных по умолчанию, если другая база данных не указана в качестве аргумента. Текущая база данных всегда отображается в приглашении командной строки. База данных по умолчанию всегда определена (если список баз данных не пуст).

База данных по умолчанию может быть задана разными способами:
- используя команду %1,
- указав путь к файлу базы данных в аргументах при запуске приложения,
- указав имя зарегистрированной базы данных  в аргументах при запуске приложения,
- восстановив предыдущую выбранную базу данных из сохранённой конфигурации,
- или если база данных по умолчанию не была выбрана любым их вышеуказанных способов, базой данных по умолчанию становится первая зарегистрированная база данных в списке.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="63"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>имя</translation>
    </message>
  </context>
  <context>
    <name>QObject</name>
    <message>
      <location filename="../clicommandsyntax.cpp" line="155"/>
      <source>Insufficient number of arguments.</source>
      <translation>Недостаточное количество аргументов.</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="325"/>
      <source>Too many arguments.</source>
      <translation>Слишком много аргументов.</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="347"/>
      <source>Invalid argument value: %1.
Expected one of: %2</source>
      <translation>Некорректное значение аргумента: %1.
Допустимые значения: %2</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="383"/>
      <source>Unknown option: %1</source>
      <comment>CLI command syntax</comment>
      <translation>Неизвестный ключ: %1</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="394"/>
      <source>Option %1 requires an argument.</source>
      <comment>CLI command syntax</comment>
      <translation>Ключ %1 требует указания аргумента.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="31"/>
      <source>string</source>
      <comment>CLI command syntax</comment>
      <translation>строка</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="28"/>
      <source>Command line interface to SQLiteStudio, a SQLite manager.</source>
      <translation>Интерфейс командной строки для SQLiteStudio, менеджера баз данных SQLite.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="32"/>
      <source>Enables debug messages on standard error output.</source>
      <translation>Включает вывод отладочных сообщений в стандартный поток ошибок.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="33"/>
      <source>Enables Lemon parser debug messages for SQL code assistant.</source>
      <translation>Включает вывод отладочных сообщений анализатора Lemon для автодополнения SQL кода.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="34"/>
      <source>Lists plugins installed in the SQLiteStudio and quits.</source>
      <translation>Выводит список установленных в SQLiteStudio модулей и осуществляет выход.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="36"/>
      <source>Executes provided SQL file (including all rich features of SQLiteStudio&apos;s query executor) on the specified database file and quits. The database parameter becomes mandatory if this option is used.</source>
      <translation>Выполняет предоставленный SQL-файл (используя все многочисленные функции исполнителя запросов SQLiteStudio) на указанном файле базы данных и выходит. При использовании этой опции обязательно указывать параметр database.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="39"/>
      <source>SQL file</source>
      <translation>Файл SQL</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="40"/>
      <source>Character encoding to use when reading SQL file (-e option). Use -cl to list available codecs. Defaults to %1.</source>
      <translation>Кодировка текста, используемая при чтении файла SQL (опция -e). Используйте -cl для получения списка доступных кодеков. По умолчанию используется %1.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="43"/>
      <source>codec</source>
      <translation>кодек</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="44"/>
      <source>Lists available codecs to be used with -c option and quits.</source>
      <translation>Выводит список доступных кодеков для использования с опцией -c и выходит.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="46"/>
      <source>When used together with -e option, the execution will not stop on an error, but rather continue until the end, ignoring errors.</source>
      <translation>При использовании вместе с параметром -e выполнение не останавливается при возникновении ошибки, а продолжается до конца, игнорируя все ошибки.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="57"/>
      <source>file</source>
      <translation>файл</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="57"/>
      <source>Database file to open</source>
      <translation>Файл базы данных для открытия</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="78"/>
      <source>Invalid codec: %1. Use -cl option to list available codecs.</source>
      <translation>Неверный кодек: %1. Используйте параметр -cl для получения списка доступных кодеков.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="108"/>
      <source>Database file argument is mandatory when executing SQL file.</source>
      <translation>Аргумент с файлом базы данных является обязательным при выполнении SQL-файла.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="114"/>
      <source>Could not open specified database for executing SQL file. You may try using -d option to find out more details.</source>
      <translation>Не удалось открыть указанную базу данных для выполнения файла SQL. Вы можете воспользоваться опцией -d, чтобы узнать больше деталей.</translation>
    </message>
  </context>
</TS>
