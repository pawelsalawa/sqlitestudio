<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="uk" sourcelanguage="en">
  <context>
    <name>CLI</name>
    <message>
      <location filename="../cli.cpp" line="98"/>
      <source>Current database: %1</source>
      <translation>Поточна база даних: %1</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="100"/>
      <source>No current working database is set.</source>
      <translation>Поточна робоча база даних не визначена.</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="102"/>
      <source>Type %1 for help</source>
      <translation>Введіть %1 для виклику довідки</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="254"/>
      <source>Database passed in command line parameters (%1) was already on the list under name: %2</source>
      <translation>База даних, передана через аргументи командного рядка (%1), вже знаходиться в списку під назвою %2</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="262"/>
      <source>Could not add database %1 to list.</source>
      <translation>Не вдалося додати базу даних %1 до списку.</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="289"/>
      <source>closed</source>
      <translation>закрито</translation>
    </message>
  </context>
  <context>
    <name>CliCommand</name>
    <message>
      <location filename="../commands/clicommand.cpp" line="107"/>
      <source>Usage: %1%2</source>
      <translation>Використання: %1%2</translation>
    </message>
  </context>
  <context>
    <name>CliCommandAdd</name>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="9"/>
      <source>Could not add database %1 to list.</source>
      <translation>Не вдалося додати базу даних %1 до списку.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="14"/>
      <source>Database added: %1</source>
      <translation>База даних додана: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="19"/>
      <source>adds new database to the list</source>
      <translation>додати нову базу даних до списку</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="24"/>
      <source>Adds given database pointed by &lt;path&gt; with given &lt;name&gt; to list the databases list. The &lt;name&gt; is just a symbolic name that you can later refer to. Just pick any unique name. For list of databases already on the list use %1 command.</source>
      <translation>Додає базу даних, розташовану за вказаним &lt;шляхом&gt; під вказаним ім&apos;ям &lt;ім&apos;ям&gt; в список баз даних. &lt;ім&apos;я&gt; - це звичайне символьне ім&apos;я, яке в подальшому можна буде використовувати. Виберіть будь-який унікальне ім&apos;я. Для отримання поточного списку баз даних за допомогою команди %1.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="34"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>ім’я</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="35"/>
      <source>path</source>
      <comment>CLI command syntax</comment>
      <translation>шлях</translation>
    </message>
  </context>
  <context>
    <name>CliCommandCd</name>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="10"/>
      <source>Changed directory to: %1</source>
      <translation>Теку змінено на: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="12"/>
      <source>Could not change directory to: %1</source>
      <translation>Не вдалося змінити директорію на: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="17"/>
      <source>changes current working directory</source>
      <translation>змінює поточну робочу директорію</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="22"/>
      <source>Very similar command to &apos;cd&apos; known from Unix systems and Windows. It requires a &lt;path&gt; argument to be passed, therefore calling %1 will always cause a change of the directory. To learn what&apos;s the current working directory use %2 command and to list contents of the current working directory use %3 command.</source>
      <translation>Дуже схожа команда до &apos;cd&apos; відомої від Unix систем і Windows. Для проходження цього аргументу &lt;path&gt; необхідно перенести аргумент, тому виклик %1 завжди може викликати зміну каталогу. Щоб дізнатися те, що використовує поточну робочу папку, використовуйте %2 команду і щоб перерахувати вміст поточної робочої директорії використовуйте %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="33"/>
      <source>path</source>
      <comment>CLI command syntax</comment>
      <translation>шлях</translation>
    </message>
  </context>
  <context>
    <name>CliCommandClose</name>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="10"/>
      <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
      <translation>Неможливо викликати %1, коли база даних не налаштована на поточну. Вкажіть поточну базу даних з командою %2, або передайте ім&apos;я бази даних на %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="21"/>
      <location filename="../commands/clicommandclose.cpp" line="29"/>
      <source>Connection to database %1 closed.</source>
      <translation>Підключення до бази даних %1 закрито.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="24"/>
      <source>No such database: %1. Use %2 to see list of known databases.</source>
      <translation>Немає такої бази даних: %1. Використовуйте %2, щоб переглянути перелік відомих баз даних.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="35"/>
      <source>closes given (or current) database</source>
      <translation>закривається за вказаною (або поточною) базою даних</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="40"/>
      <source>Closes the database connection. If the database was already closed, nothing happens. If &lt;name&gt; is provided, it should be the name of the database to close (as printed by the %1 command). If &lt;name&gt; is not provided, then the current working database is closed (see help for %2 for details).</source>
      <translation type="unfinished">Closes the database connection. If the database was already closed, nothing happens. If &lt;name&gt; is provided, it should be the name of the database to close (as printed by the %1 command). If &lt;name&gt; is not provided, then the current working database is closed (see help for %2 for details).</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="50"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>назва</translation>
    </message>
  </context>
  <context>
    <name>CliCommandDbList</name>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="12"/>
      <source>No current working database defined.</source>
      <translation>Поточна робоча база даних не визначена.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="18"/>
      <source>Databases:</source>
      <translation>База даних:</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="23"/>
      <location filename="../commands/clicommanddblist.cpp" line="34"/>
      <source>Name</source>
      <comment>CLI db name column</comment>
      <translation>Назва</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="31"/>
      <location filename="../commands/clicommanddblist.cpp" line="61"/>
      <source>Open</source>
      <comment>CLI connection state column</comment>
      <translation>Відкрити</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="31"/>
      <location filename="../commands/clicommanddblist.cpp" line="61"/>
      <source>Closed</source>
      <comment>CLI connection state column</comment>
      <translation>Закрити</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="32"/>
      <location filename="../commands/clicommanddblist.cpp" line="36"/>
      <source>Connection</source>
      <comment>CLI connection state column</comment>
      <translation>Підключення</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="38"/>
      <location filename="../commands/clicommanddblist.cpp" line="45"/>
      <source>Database file path</source>
      <translation>Шлях до бази даних</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="70"/>
      <source>prints list of registered databases</source>
      <translation>друкує список зареєстрованих баз даних</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="75"/>
      <source>Prints list of databases registered in the SQLiteStudio. Each database on the list can be in open or closed state and %1 tells you that. The current working database (aka default database) is also marked on the list with &apos;*&apos; at the start of its name. See help for %2 command to learn about the default database.</source>
      <translation>Друкує перелік баз даних, зареєстрованих у SQLiteStudio. Кожна база даних в списку може бути у відкритому або закритому стані і %1 повідомляє вам це. Поточна робоча база даних (база даних за замовчуванням) також позначена в списку з &apos;*&apos; на початку назви. Дивись довідку для %2 команди щоб дізнатися про типову базу даних.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandDesc</name>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="15"/>
      <source>No working database is set.
Call %1 command to set working database.
Call %2 to see list of all databases.</source>
      <translation>Робоча база даних не встановлена.
Набрати %1 для встановлення робочої бази даних.
Набрати %2 для перегляду списку всіх баз даних.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="26"/>
      <source>Database is not open.</source>
      <translation>База даних не відкрита.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="35"/>
      <source>Cannot find table named: %1</source>
      <translation>Не вдалося знайти таблицю з назвою: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="52"/>
      <source>shows details about the table</source>
      <translation>показати деталі про таблицю</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="63"/>
      <source>table</source>
      <translation>таблиця</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="70"/>
      <source>Table: %1</source>
      <translation>Таблиця: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="74"/>
      <source>Column name</source>
      <translation>Назва стовпця</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="76"/>
      <source>Data type</source>
      <translation>Тип даних</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="80"/>
      <source>Constraints</source>
      <translation>Обмеження</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="105"/>
      <source>Virtual table: %1</source>
      <translation>Віртуальна таблиця: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="109"/>
      <source>Construction arguments:</source>
      <translation>Параметри створення:</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="114"/>
      <source>No construction arguments were passed for this virtual table.</source>
      <translation>Не вказані параметри створення для цієї віртуальної таблиці.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandDir</name>
    <message>
      <location filename="../commands/clicommanddir.cpp" line="33"/>
      <source>lists directories and files in current working directory</source>
      <translation>перелічує каталоги та файли в поточній робочій каталозі</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddir.cpp" line="38"/>
      <source>This is very similar to &apos;dir&apos; command known from Windows and &apos;ls&apos; command from Unix systems.

You can pass &lt;pattern&gt; with wildcard characters to filter output.</source>
      <translation>Це дуже схоже на команду &apos;dir&apos; з Windows та &apos;ls&apos; на команду з Unix систем.

Ви можете передати &lt;pattern&gt; символами, які містять шаблони, щоб відфільтрувати вивід.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddir.cpp" line="49"/>
      <source>pattern</source>
      <translation>шаблон</translation>
    </message>
  </context>
  <context>
    <name>CliCommandExit</name>
    <message>
      <location filename="../commands/clicommandexit.cpp" line="12"/>
      <source>quits the application</source>
      <translation>вийти з додатку</translation>
    </message>
    <message>
      <location filename="../commands/clicommandexit.cpp" line="17"/>
      <source>Quits the application. Settings are stored in configuration file and will be restored on next startup.</source>
      <translation>Вийти з додатку. Налаштування зберігаються у файлі конфігурації і будуть відновлені при наступному запуску.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandHelp</name>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="16"/>
      <source>shows this help message</source>
      <translation>показати це довідкове повідомлення</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="21"/>
      <source>Use %1 to learn about certain commands supported by the command line interface (CLI) of the SQLiteStudio.
To see list of supported commands, type %2 without any arguments.

When passing &lt;command&gt; name, you can skip special prefix character (&apos;%3&apos;).

You can always execute any command with exactly single &apos;--help&apos; option to see help for that command. It&apos;s an alternative for typing: %1 &lt;command&gt;.</source>
      <translation>Використовуйте %1, щоб дізнатися про деякі команди, які підтримуються інтерфейсом командного рядка (CLI) з SQLiteStudio.
Щоб переглянути список підтримуваних команд, введіть %2 без будь-яких аргументів.

У назві &lt;command&gt; можна не вказувати префіксний символ (&apos;%3&apos;).

Для отримання довідки по команді ви також можете виконати команду з єдиним ключем &apos;--help&apos;. Це альтернатива введенню: %1 &lt;command&gt;.</translation>
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
      <translation>Немає такої команди: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="43"/>
      <source>Type &apos;%1&apos; for list of available commands.</source>
      <translation>Введіть &apos;%1&apos; для отримання списку доступних команд.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="52"/>
      <source>Usage: %1%2</source>
      <translation>Використання: %1%2</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="62"/>
      <source>Aliases: %1</source>
      <translation>Псевдоніми: %1</translation>
    </message>
  </context>
  <context>
    <name>CliCommandHistory</name>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="23"/>
      <source>Current history limit is set to: %1</source>
      <translation>Поточний ліміт історії встановлено на: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="39"/>
      <source>prints history or erases it</source>
      <translation>друкувати історію або видалити</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="44"/>
      <source>When no argument was passed, this command prints command line history. Every history entry is separated with a horizontal line, so multiline entries are easier to read.

When the -c or --clear option is passed, then the history gets erased.
When the -l or --limit option is passed, it sets the new history entries limit. It requires an additional argument saying how many entries do you want the history to be limited to.
Use -ql or --querylimit option to see the current limit value.</source>
      <translation>Коли аргумент не був прийнятий, ця команда друкує історію командного рядка. Кожен запис в історії розділений по горизонтальній прямій, тож багаторічні записи легше зчитувати.

Коли параметр -c або --clear, історія стирається.
при проходженні параметра -l або --limitвстановлюється новий ліміт на кількість записів в історії. Необхідний додатковий аргумент, який вказує скільки записів необхідно зберігати в історії.
Для перегляду поточного ліміту записів викличте команду з ключем -ql або --querylimit.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="59"/>
      <source>number</source>
      <translation>номер</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="66"/>
      <source>Console history erased.</source>
      <translation>Історію консолі стерто.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="75"/>
      <source>Invalid number: %1</source>
      <translation>Неприпустиме число: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="80"/>
      <source>History limit set to %1</source>
      <translation>Ліміт історії встановлено на %1</translation>
    </message>
  </context>
  <context>
    <name>CliCommandMode</name>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="9"/>
      <source>Current results printing mode: %1</source>
      <translation>Поточний режим друку результатів: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="16"/>
      <source>Invalid results printing mode: %1</source>
      <translation>Недійсний режим друку результатів: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="21"/>
      <source>New results printing mode: %1</source>
      <translation>Поточний режим друку результатів: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="26"/>
      <source>tells or changes the query results format</source>
      <translation>повідомляє або змінює формат результатів запиту</translation>
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
      <translation type="unfinished">When called without argument, tells the current output format for a query results. When the &lt;mode&gt; is passed, the mode is changed to the given one. Supported modes are:
- CLASSIC - columns are separated by a comma, not aligned,
- FIXED   - columns have equal and fixed width, they always fit into terminal window width, but the data in columns can be cut off,
- COLUMNS - like FIXED, but smarter (do not use with huge result sets, see details below),
- ROW     - each column from the row is displayed in new line, so the full data is displayed.

The CLASSIC mode is recommended if you want to see all the data, but you don&apos;t want to waste lines for each column. Each row will display full data for every column, but this also means, that columns will not be aligned to each other in next rows. The CLASSIC mode also doesn&apos;t respect the width of your terminal (console) window, so if values in columns are wider than the window, the row will be continued in next lines.

The FIXED mode is recommended if you want a readable output and you don&apos;t care about long data values. Columns will be aligned, making the output a nice table. The width of columns is calculated from width of the console window and a number of columns.

The COLUMNS mode is similar to FIXED mode, except it tries to be smart and make columns with shorter values more thin, while columns with longer values get more space. First to shrink are columns with longest headers (so the header names are to be cut off as first), then columns with the longest values are shrinked, up to the moment when all columns fit into terminal window.
ATTENTION! The COLUMNS mode reads all the results from the query at once in order to evaluate column widths, therefore it is dangerous to use this mode when working with huge result sets. Keep in mind that this mode will load entire result set into memory.

The ROW mode is recommended if you need to see whole values and you don&apos;t expect many rows to be displayed, because this mode displays a line of output per each column, so you&apos;ll get 10 lines for single row with 10 columns, then if you have 10 of such rows, you will get 100 lines of output (+1 extra line per each row, to separate rows from each other).</translation>
    </message>
  </context>
  <context>
    <name>CliCommandNullValue</name>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="9"/>
      <source>Current NULL representation string: %1</source>
      <translation>Поточний рядок репрезентації NULL: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="15"/>
      <source>tells or changes the NULL representation string</source>
      <translation>повідомляє або змінює рядок представлення NULL</translation>
    </message>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="20"/>
      <source>If no argument was passed, it tells what&apos;s the current NULL value representation (that is - what is printed in place of NULL values in query results). If the argument is given, then it&apos;s used as a new string to be used for NULL representation.</source>
      <translation>При виклику без аргументів відображає поточне представлення значення NULL (тобто що виводиться замість значення NULL у результатах запиту). Якщо вказано аргумент, його буде використано як рядок для представлення значення NULL.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandOpen</name>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="12"/>
      <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
      <translation>Неможливо викликати %1, коли база даних не вибрана. Виберіть поточну базу даних командою %2, або передайте ім&apos;я бази даних на %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="29"/>
      <source>Could not add database %1 to list.</source>
      <translation>Не вдалося додати базу даних %1 до списку.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="37"/>
      <source>File %1 doesn&apos;t exist in %2. Cannot open inexisting database with %3 command. To create a new database, use %4 command.</source>
      <translation>Файл %1 не існує в %2. Неможливо відкрити наявну базу даних з командою %3. Щоб створити нову базу даних, використовуйте команду %4.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="61"/>
      <source>Database %1 has been open and set as the current working database.</source>
      <translation>База даних %1 була відкрита та встановлена як поточна робоча база даних.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="66"/>
      <source>opens database connection</source>
      <translation>відкриває підключення до бази даних</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="71"/>
      <source>Opens connection to the database. If no additional argument was passed, then the connection is open to the current default database (see help for %1 for details). However if an argument was passed, it can be either &lt;name&gt; of the registered database to open, or it can be &lt;path&gt; to the database file to open. In the second case, the &lt;path&gt; gets registered on the list with a generated name, but only for the period of current application session. After restarting application such database is not restored on the list.</source>
      <translation>Відкриває підключення до бази даних. Якщо жодного додаткового аргументу не було передано, то підключення відкрите до поточної бази даних за замовчуванням (див. довідку %1 для деталей). Однак, якщо аргумент був прийнятий, відкривати може бути або &lt;name&gt; з бази даних зареєстрованої або ж це може бути &lt;path&gt; в базі даних, щоб відкрити. В другому випадку, &lt;path&gt; реєструється зі списком зі згенерованою назвою, але тільки за період поточного сеансу додатків. Після перезапуску такої бази даних не відновлено в списку.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="83"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>ім’я</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="83"/>
      <source>path</source>
      <comment>CLI command syntax</comment>
      <translation>шлях</translation>
    </message>
  </context>
  <context>
    <name>CliCommandPwd</name>
    <message>
      <location filename="../commands/clicommandpwd.cpp" line="13"/>
      <source>prints the current working directory</source>
      <translation>друкує поточну робочу директорію</translation>
    </message>
    <message>
      <location filename="../commands/clicommandpwd.cpp" line="18"/>
      <source>This is the same as &apos;pwd&apos; command on Unix systems and &apos;cd&apos; command without arguments on Windows. It prints current working directory. You can change the current working directory with %1 command and you can also list contents of the current working directory with %2 command.</source>
      <translation>Це те саме, що команда &apos;pwd&apos; в системах Unix та команда &apos;cd&apos; без аргументів у Windows. Він друкує поточний робочий каталог. Ви можете змінити поточний робочий каталог за допомогою команди %1, а також можете перерахувати вміст поточного робочого каталогу за допомогою команди %2.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandRemove</name>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="12"/>
      <source>No such database: %1</source>
      <translation>Немає такої бази даних: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="20"/>
      <source>Database removed: %1</source>
      <translation>База даних видалена: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="26"/>
      <source>New current database set:</source>
      <translation>Нову поточну базу даних встановлено:</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="35"/>
      <source>removes database from the list</source>
      <translation>видаляє базу даних зі списку</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="40"/>
      <source>Removes &lt;name&gt; database from the list of registered databases. If the database was not on the list (see %1 command), then error message is printed and nothing more happens.</source>
      <translation>Видаляє з списку зареєстрованих баз даних &lt;name&gt; баз даних. Якщо бази даних немає в списку (див. команду %1), то повідомлення про помилку надруковано і більше нічого не відбувається.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="50"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>ім’я</translation>
    </message>
  </context>
  <context>
    <name>CliCommandSql</name>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="19"/>
      <source>No working database is set.
Call %1 command to set working database.
Call %2 to see list of all databases.</source>
      <translation>Робоча база даних не встановлена.
Набрати %1 для встановлення робочої бази даних.
Набрати %2 для перегляду списку всіх баз даних.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="30"/>
      <source>Database is not open.</source>
      <translation>База даних не відкрита.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="65"/>
      <source>executes SQL query</source>
      <translation>виконується запит SQL</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="70"/>
      <source>This command is executed every time you enter SQL query in command prompt. It executes the query on the current working database (see help for %1 for details). There&apos;s no sense in executing this command explicitly. Instead just type the SQL query in the command prompt, without any command prefixed.</source>
      <translation>Ця команда виконується щоразу, коли ви вводите SQL запит у командному рядку. Він виконує запит на поточну робочу базу даних (перегляньте допомогу для %1 для докладної інформації). Існує&apos;не має сенсу виконувати цю команду явно. Замість цього, просто введіть SQL запит у командний рядок без жодного префіксу команди.</translation>
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
      <translation>Занадто багато стовпців для відображення в режимі %1.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="254"/>
      <source>Row %1</source>
      <translation>Рядок %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="404"/>
      <source>Query execution error: %1</source>
      <translation>Помилка виконання запиту: %1</translation>
    </message>
  </context>
  <context>
    <name>CliCommandTables</name>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="15"/>
      <source>No such database: %1. Use %2 to see list of known databases.</source>
      <translation>Немає такої бази даних: %1. Використовуйте %2, щоб переглянути перелік відомих баз даних.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="25"/>
      <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
      <translation>Неможливо викликати %1, коли база даних не налаштована на поточну. Вкажіть поточну базу даних з командою %2, або передайте ім&apos;я бази даних на %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="32"/>
      <source>Database %1 is closed.</source>
      <translation>База даних %1 закрита.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="45"/>
      <location filename="../commands/clicommandtables.cpp" line="47"/>
      <source>Database</source>
      <translation>База даних</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="47"/>
      <source>Table</source>
      <translation>Таблиця</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="61"/>
      <source>prints list of tables in the database</source>
      <translation>надрукує список таблиць в базі даних</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="66"/>
      <source>Prints list of tables in given &lt;database&gt; or in the current working database. Note, that the &lt;database&gt; should be the name of the registered database (see %1). The output list includes all tables from any other databases attached to the queried database.
When the -s option is given, then system tables are also listed.</source>
      <translation>Друкує перелік таблиць у даній &lt;database&gt; або в поточній базі даних. Зверніть увагу, що &lt;database&gt; має бути ім&apos;я зареєстрованої бази даних (див. %1). Вихідний список включає в себе всі таблиці з будь-яких інших баз даних, прикріплених до запитаної бази даних.
Якщо параметр -s вказано, то системні таблиці також списані.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="77"/>
      <source>database</source>
      <comment>CLI command syntax</comment>
      <translation>база даних</translation>
    </message>
  </context>
  <context>
    <name>CliCommandTree</name>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="12"/>
      <source>No current working database is selected. Use %1 to define one and then run %2.</source>
      <translation>Не вибрано жодної робочої бази даних. Використовуйте %1, щоб визначити одне, а потім запустити %2.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="54"/>
      <source>Tables</source>
      <translation>Таблиці</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="58"/>
      <source>Views</source>
      <translation>Розріз даних</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="83"/>
      <source>Columns</source>
      <translation>Стовпці</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="88"/>
      <source>Indexes</source>
      <translation>Індекси</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="92"/>
      <location filename="../commands/clicommandtree.cpp" line="113"/>
      <source>Triggers</source>
      <translation>Тригери</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="132"/>
      <source>prints all objects in the database as a tree</source>
      <translation>друкує всі об&apos;єкти в базі даних як дерево</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="137"/>
      <source>Prints all objects (tables, indexes, triggers and views) that are in the database as a tree. The tree is very similar to the one that you can see in GUI client of the SQLiteStudio.
When -c option is given, then also columns will be listed under each table.
When -s option is given, then also system objects will be printed (sqlite_* tables, autoincrement indexes, etc).
The database argument is optional and if provided, then only given database will be printed. This is not a registered database name, but instead it&apos;s an internal SQLite database name, like &apos;main&apos;, &apos;temp&apos;, or any attached database name. To print tree for other registered database, call %1 first to switch the working database, and then use %2 command.</source>
      <translation>Відображає всі об&apos;єкти (таблиці, індекси, тригери та розріз даних) бази даних у вигляді дерева. Структура дерева подібна до того, що відображається в GUI клієнта SQLiteStudio.
При виклику з -c також будуть виведені стовпці під кожною таблицею.
При виклику з ключем -s також буде виведено системні об&apos;єкти (таблиці sqlite_*, індекси автоінкремента тощо).
При виклику з необов&apos;язковим аргументом &apos;база даних&apos; будуть виведені об&apos;єкти лише вказаної бази даних. Під &apos;базою даних&apos; мається на увазі не зареєстроване ім&apos;я бази даних, а внутрішнє ім&apos;я бази даних SQLite, наприклад &apos;main&apos;, &apos;temp&apos; або ім&apos;я приєднаної бази даних. Для відображення дерева іншої зареєстрованої бази даних спочатку змініть робочу базу даних командою %1, а потім скористайтеся командою %2.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandUse</name>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="13"/>
      <source>No current database selected.</source>
      <translation>Не обрано поточної бази даних.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="16"/>
      <location filename="../commands/clicommanduse.cpp" line="30"/>
      <source>Current database: %1</source>
      <translation>Поточна база даних: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="23"/>
      <source>No such database: %1</source>
      <translation>Немає такої бази даних: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="35"/>
      <source>changes default working database</source>
      <translation>змінити типову робочу базу даних</translation>
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
      <translation type="unfinished">Changes current working database to &lt;name&gt;. If the &lt;name&gt; database is not registered in the application, then the error message is printed and no change is made.

What is current working database?
When you type a SQL query to be executed, it is executed on the default database, which is also known as the current working database. Most of database-related commands can also work using default database, if no database was provided in their arguments. The current database is always identified by command line prompt. The default database is always defined (unless there is no database on the list at all).

The default database can be selected in various ways:
- using %1 command,
- by passing database file name to the application startup parameters,
- by passing registered database name to the application startup parameters,
- by restoring previously selected default database from saved configuration,
- or when default database was not selected by any of the above, then first database from the registered databases list becomes the default one.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="63"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>ім’я</translation>
    </message>
  </context>
  <context>
    <name>QObject</name>
    <message>
      <location filename="../clicommandsyntax.cpp" line="155"/>
      <source>Insufficient number of arguments.</source>
      <translation>Недостатньо кількості аргументів.</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="325"/>
      <source>Too many arguments.</source>
      <translation>Забагато аргументів.</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="347"/>
      <source>Invalid argument value: %1.
Expected one of: %2</source>
      <translation>Неприпустиме значення аргументу: %1.
очікувався: %2</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="383"/>
      <source>Unknown option: %1</source>
      <comment>CLI command syntax</comment>
      <translation>Невідомий параметр: %1</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="394"/>
      <source>Option %1 requires an argument.</source>
      <comment>CLI command syntax</comment>
      <translation>Опція %1 потребує аргументу.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="31"/>
      <source>string</source>
      <comment>CLI command syntax</comment>
      <translation>рядок</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="28"/>
      <source>Command line interface to SQLiteStudio, a SQLite manager.</source>
      <translation>Інтерфейс командного рядка для SQLiteStudio, менеджера баз даних SQLite.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="32"/>
      <source>Enables debug messages on standard error output.</source>
      <translation>Вмикає повідомлення налагодження при звичайному виході помилки.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="33"/>
      <source>Enables Lemon parser debug messages for SQL code assistant.</source>
      <translation>Включає вивід налагоджувальних повідомлень аналізатора Lemon для автодоповнення SQL коду.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="34"/>
      <source>Lists plugins installed in the SQLiteStudio and quits.</source>
      <translation>Виводить список встановлених в SQLiteStudio модулів і здійснює вихід.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="36"/>
      <source>Executes provided SQL file (including all rich features of SQLiteStudio&apos;s query executor) on the specified database file and quits. The database parameter becomes mandatory if this option is used.</source>
      <translation type="unfinished">Executes provided SQL file (including all rich features of SQLiteStudio&apos;s query executor) on the specified database file and quits. The database parameter becomes mandatory if this option is used.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="39"/>
      <source>SQL file</source>
      <translation type="unfinished">SQL file</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="40"/>
      <source>Character encoding to use when reading SQL file (-e option). Use -cl to list available codecs. Defaults to %1.</source>
      <translation type="unfinished">Character encoding to use when reading SQL file (-e option). Use -cl to list available codecs. Defaults to %1.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="43"/>
      <source>codec</source>
      <translation type="unfinished">codec</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="44"/>
      <source>Lists available codecs to be used with -c option and quits.</source>
      <translation type="unfinished">Lists available codecs to be used with -c option and quits.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="46"/>
      <source>When used together with -e option, the execution will not stop on an error, but rather continue until the end, ignoring errors.</source>
      <translation type="unfinished">When used together with -e option, the execution will not stop on an error, but rather continue until the end, ignoring errors.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="57"/>
      <source>file</source>
      <translation>файл</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="57"/>
      <source>Database file to open</source>
      <translation>Файл бази даних для відкриття</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="78"/>
      <source>Invalid codec: %1. Use -cl option to list available codecs.</source>
      <translation type="unfinished">Invalid codec: %1. Use -cl option to list available codecs.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="108"/>
      <source>Database file argument is mandatory when executing SQL file.</source>
      <translation type="unfinished">Database file argument is mandatory when executing SQL file.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="114"/>
      <source>Could not open specified database for executing SQL file. You may try using -d option to find out more details.</source>
      <translation type="unfinished">Could not open specified database for executing SQL file. You may try using -d option to find out more details.</translation>
    </message>
  </context>
</TS>
