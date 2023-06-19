<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="pl" sourcelanguage="en">
  <context>
    <name>CLI</name>
    <message>
      <location filename="../cli.cpp" line="98"/>
      <source>Current database: %1</source>
      <translation>Bieżąca baza danych: %1</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="100"/>
      <source>No current working database is set.</source>
      <translation>Nie ustawiono bieżącej bazy danych.</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="102"/>
      <source>Type %1 for help</source>
      <translation>Wpisz %1, aby uzyskać pomoc.</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="254"/>
      <source>Database passed in command line parameters (%1) was already on the list under name: %2</source>
      <translation>Baz danych przekazana w parametrach linii poleceń (%1) była już na liście pod nazwą: %2</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="262"/>
      <source>Could not add database %1 to list.</source>
      <translation>Nie udało się dodać bazy danych %1 do listy.</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="289"/>
      <source>closed</source>
      <translation>zamknięta</translation>
    </message>
  </context>
  <context>
    <name>CliCommand</name>
    <message>
      <location filename="../commands/clicommand.cpp" line="107"/>
      <source>Usage: %1%2</source>
      <translation>Sposób użycia: %1%2</translation>
    </message>
  </context>
  <context>
    <name>CliCommandAdd</name>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="9"/>
      <source>Could not add database %1 to list.</source>
      <translation>Nie udało się dodać bazy danych %1 do listy.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="14"/>
      <source>Database added: %1</source>
      <translation>Baza danych dodana: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="19"/>
      <source>adds new database to the list</source>
      <translation>dodaje bazę danych do listy</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="24"/>
      <source>Adds given database pointed by &lt;path&gt; with given &lt;name&gt; to list the databases list. The &lt;name&gt; is just a symbolic name that you can later refer to. Just pick any unique name. For list of databases already on the list use %1 command.</source>
      <translation>Dodaje bazę danych wskazaną przez &lt;ścieżkę&gt; z daną &lt;nazwą&gt; do listy baz danych. &lt;nazwa&gt; jest tylko symboliczną nazwą, do której możesz się potem odwoływać. Po prostu wybierz dowolną, unikalną nazwę. Aby wypisać listę baz danych, które są już na liście, użyj polecenia %1.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="34"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>nazwa</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="35"/>
      <source>path</source>
      <comment>CLI command syntax</comment>
      <translation>ścieżka</translation>
    </message>
  </context>
  <context>
    <name>CliCommandCd</name>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="10"/>
      <source>Changed directory to: %1</source>
      <translation>Zmieniono katalog na: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="12"/>
      <source>Could not change directory to: %1</source>
      <translation>Nie udało się zmienić katalogu na: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="17"/>
      <source>changes current working directory</source>
      <translation>zmienia bieżący katalog</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="22"/>
      <source>Very similar command to &apos;cd&apos; known from Unix systems and Windows. It requires a &lt;path&gt; argument to be passed, therefore calling %1 will always cause a change of the directory. To learn what&apos;s the current working directory use %2 command and to list contents of the current working directory use %3 command.</source>
      <translation>Bardzo podobne polecenie do &apos;cd&apos; znanego z systemów Unixowych i Windowsa. Wymaga &lt;ścieżki&gt; jako argumentu, stąd wywołanie %1 zawsze spowoduje zmianę katalogu. Aby poznać jaki jest bieżący katalog, użyj polecenia %2, a żeby wypisać listę zawartości bieżącego katalogu użyj polecenia %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="33"/>
      <source>path</source>
      <comment>CLI command syntax</comment>
      <translation>ścieżka</translation>
    </message>
  </context>
  <context>
    <name>CliCommandClose</name>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="10"/>
      <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
      <translation>Nie można wywołać %1, gdy żadna z baz nie jest ustawiona jako bieżąca. Okreś bieżącą bazę używając polecenia %2, lub podaj nazwę bazy do %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="21"/>
      <location filename="../commands/clicommandclose.cpp" line="29"/>
      <source>Connection to database %1 closed.</source>
      <translation>Połączenie z bazą %1 zostało zamknięte.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="24"/>
      <source>No such database: %1. Use %2 to see list of known databases.</source>
      <translation>Nie znaleziono bazy danych: %1. Użyj %2 aby zonaczyć listę znanych baz danych.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="35"/>
      <source>closes given (or current) database</source>
      <translation>zamyka daną (lub bieżącą) bazę danych</translation>
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
      <translation>nazwa</translation>
    </message>
  </context>
  <context>
    <name>CliCommandDbList</name>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="12"/>
      <source>No current working database defined.</source>
      <translation>Nie określono bieżącej bazy danych.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="18"/>
      <source>Databases:</source>
      <translation>Bazy danych:</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="23"/>
      <location filename="../commands/clicommanddblist.cpp" line="34"/>
      <source>Name</source>
      <comment>CLI db name column</comment>
      <translation>Nazwa</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="31"/>
      <location filename="../commands/clicommanddblist.cpp" line="61"/>
      <source>Open</source>
      <comment>CLI connection state column</comment>
      <translation>Otwarta</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="31"/>
      <location filename="../commands/clicommanddblist.cpp" line="61"/>
      <source>Closed</source>
      <comment>CLI connection state column</comment>
      <translation>Zamknięta</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="32"/>
      <location filename="../commands/clicommanddblist.cpp" line="36"/>
      <source>Connection</source>
      <comment>CLI connection state column</comment>
      <translation>Połączenie</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="38"/>
      <location filename="../commands/clicommanddblist.cpp" line="45"/>
      <source>Database file path</source>
      <translation>Ścieżka do pliku bazy danych</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="70"/>
      <source>prints list of registered databases</source>
      <translation>wypisuje listę zarejestrowanych baz danych</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="75"/>
      <source>Prints list of databases registered in the SQLiteStudio. Each database on the list can be in open or closed state and %1 tells you that. The current working database (aka default database) is also marked on the list with &apos;*&apos; at the start of its name. See help for %2 command to learn about the default database.</source>
      <translation>Wypisuje listę zarejestrowanych w SQLiteStudio baz danych. Każda baza danych na liście może być otwarta, lub zamknięta i %1 o tym mówi. Bieżąca baza danych (nazywana również domyślną bazą danych) jest również zaznaczona na liście znakiem &apos;*&apos; na początku swojej nazwy. Zobacz pomoc dla polecenia %2, żeby dowiedzieć się więcej o domyślnej bazie danych.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandDesc</name>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="15"/>
      <source>No working database is set.
Call %1 command to set working database.
Call %2 to see list of all databases.</source>
      <translation>Nie wybrano domyślnej bazy danych.
Użyj polecenia %1, aby ustawić domyślną bazę danych.
Użyj polecenie %2, aby wypisać listę wszystkich baz.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="26"/>
      <source>Database is not open.</source>
      <translation>Baza danych nie jest otwarta.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="35"/>
      <source>Cannot find table named: %1</source>
      <translation>Nie można znaleźć tabeli o nazwie: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="52"/>
      <source>shows details about the table</source>
      <translation>pokazuje szczegóły o tabeli</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="63"/>
      <source>table</source>
      <translation>tabela</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="70"/>
      <source>Table: %1</source>
      <translation>Tabla: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="74"/>
      <source>Column name</source>
      <translation>Nazwa kolumny</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="76"/>
      <source>Data type</source>
      <translation>Typ danych</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="80"/>
      <source>Constraints</source>
      <translation>Ograniczenia</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="105"/>
      <source>Virtual table: %1</source>
      <translation>Wirtualna tabela: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="109"/>
      <source>Construction arguments:</source>
      <translation>Argumenty konstruujące:</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="114"/>
      <source>No construction arguments were passed for this virtual table.</source>
      <translation>Nie podano argumentów konstruujących dla tej tabeli wirtualnej.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandDir</name>
    <message>
      <location filename="../commands/clicommanddir.cpp" line="33"/>
      <source>lists directories and files in current working directory</source>
      <translation>wypisuje listę katalogów i plików w bieżącym katalogu</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddir.cpp" line="38"/>
      <source>This is very similar to &apos;dir&apos; command known from Windows and &apos;ls&apos; command from Unix systems.

You can pass &lt;pattern&gt; with wildcard characters to filter output.</source>
      <translation>To jest polecenie bardzo podobne do &apos;dir&apos; znanego z systemu Windows, oraz &apos;ls&apos; znanego z systemów Unixowych.

Możesz podać &lt;wzorzec&gt; ze znakami maskującymi, aby filtrować wynik.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddir.cpp" line="49"/>
      <source>pattern</source>
      <translation>wzorzec</translation>
    </message>
  </context>
  <context>
    <name>CliCommandExit</name>
    <message>
      <location filename="../commands/clicommandexit.cpp" line="12"/>
      <source>quits the application</source>
      <translation>zamyka aplikację</translation>
    </message>
    <message>
      <location filename="../commands/clicommandexit.cpp" line="17"/>
      <source>Quits the application. Settings are stored in configuration file and will be restored on next startup.</source>
      <translation>Zamyka aplikację. Ustawienia są zapisywane w pliku konfiguracyjnym i zostaną przywrócone przy następnym starcie.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandHelp</name>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="16"/>
      <source>shows this help message</source>
      <translation>pokazuje tą treść pomocy</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="21"/>
      <source>Use %1 to learn about certain commands supported by the command line interface (CLI) of the SQLiteStudio.
To see list of supported commands, type %2 without any arguments.

When passing &lt;command&gt; name, you can skip special prefix character (&apos;%3&apos;).

You can always execute any command with exactly single &apos;--help&apos; option to see help for that command. It&apos;s an alternative for typing: %1 &lt;command&gt;.</source>
      <translation>Używaj %1 aby poznać poszczególne polecenia obsługiwane przez interfejs linii poleceń (CLI) SQLiteStudio.
Aby zobaczyć listę obsługiwanych poleceń, wpisz %2 bez żadnych argumentów.

Kiedy podaje się nazwę &lt;polecenia&gt;, można pominąć specjalny znak przedrostka (&apos;%3&apos;).

Zawsze możesz wywołać dowolne polecenie z dokładnie jedną opcją &apos;--help&apos;, aby zobaczyć pomoc dla tego polecenia. Jest to alternatywa dla wpisywania: %1 &lt;polecenie&gt;.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="33"/>
      <source>command</source>
      <comment>CLI command syntax</comment>
      <translation>polecenie</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="42"/>
      <source>No such command: %1</source>
      <translation>Nie ma takiego polecenia: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="43"/>
      <source>Type &apos;%1&apos; for list of available commands.</source>
      <translation>Wpisz &apos;%1&apos; aby poznać listę dostępnych poleceń.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="52"/>
      <source>Usage: %1%2</source>
      <translation>Sposób użycia: %1%2</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="62"/>
      <source>Aliases: %1</source>
      <translation>Aliasy: %1</translation>
    </message>
  </context>
  <context>
    <name>CliCommandHistory</name>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="23"/>
      <source>Current history limit is set to: %1</source>
      <translation>Bieżący limit historii jest ustawiony na: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="39"/>
      <source>prints history or erases it</source>
      <translation>wyświetla historię lub ją kasuje</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="44"/>
      <source>When no argument was passed, this command prints command line history. Every history entry is separated with a horizontal line, so multiline entries are easier to read.

When the -c or --clear option is passed, then the history gets erased.
When the -l or --limit option is passed, it sets the new history entries limit. It requires an additional argument saying how many entries do you want the history to be limited to.
Use -ql or --querylimit option to see the current limit value.</source>
      <translation>Gdy nie poda się żadnego argumentu, to polecenie wyświetla historię linii poleceń. Każdy wpis w historii jest oddzielony linią poziomą, żeby łatwiej było czytać więcej wpisów.

Kiedy poda się opcję -c lub --clear, to historia jest kasowana.
Kiedy poda się opcję -l lub --limit, to ustawiany jest nowy limit na historii. Wymaga to podania dodatkowego argumentu, mówiącego o tym, ile ma być wpisów przechowywanych w historii.
Użyj opcji -ql lub --querylimit, aby poznać aktualną wartość limitu.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="59"/>
      <source>number</source>
      <translation>liczba</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="66"/>
      <source>Console history erased.</source>
      <translation>Historia konsoli skasowana.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="75"/>
      <source>Invalid number: %1</source>
      <translation>Niepoprawna liczba: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="80"/>
      <source>History limit set to %1</source>
      <translation>Limit historii ustawiono na %1</translation>
    </message>
  </context>
  <context>
    <name>CliCommandMode</name>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="9"/>
      <source>Current results printing mode: %1</source>
      <translation>Aktualny tryb wyświetlania wyników: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="16"/>
      <source>Invalid results printing mode: %1</source>
      <translation>Niepoprawny tryb wyświetlania wyników: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="21"/>
      <source>New results printing mode: %1</source>
      <translation>Nowy tryb wyświetlania wyników: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="26"/>
      <source>tells or changes the query results format</source>
      <translation>wyświetla lub zmienia format wyników zapytania</translation>
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
      <translation>Gdy wywołuje się bez argumentu, wyświetla bieżący format wyjścia dla wyników zapytania. Po podaniu &lt;tryb&gt;, zostaje on zmieniony na podany tryb. Obsługiwane tryby to:
- CLASSIC - kolumny są oddzielone przecinkiem, nie wyrównane,
- FIXED - kolumny mają taką samą i stałą szerokość, zawsze pasują do szerokości okna terminalu, ale dane w kolumnach mogą być obcięte,
- KOLUMNS - jak FIXED, ale mądrzejsze (nie używaj z dużymi zestawami wyników, zobacz szczegóły poniżej),
- ROW - każda kolumna z wiersza jest wyświetlana w nowej linii, więc wyświetlane są pełne dane.

Tryb CLASSIC jest zalecany, jeśli chcesz zobaczyć wszystkie dane, ale nie chcesz marnować linii dla każdej kolumny. Każdy wiersz wyświetli pełne dane dla każdej kolumny, ale oznacza to również, że kolumny nie będą wyrównane do siebie w kolejnych wierszach. Tryb CLASSIC również nie respektuje nie szerokości okna terminala (konsoli), więc jeśli wartości w kolumnach są większe niż okno, wiersz będzie kontynuowany w kolejnych wierszach.

Tryb FIXED jest zalecany, jeśli chcesz odczytać dane wyjściowe i nie troszczysz się o długie wartości danych. Kolumny będą wyrównywane, co sprawi, że wyjście będzie ładną tabelą. Szerokość kolumn jest obliczana na podstawie szerokości okna konsoli i liczby kolumn.

Tryb COLUMNS jest podobny do trybu FIXED, z tą różnicą, że próbuje być inteligentny i sprawić, żeby kolumny o krótszych wartościach były węższe, podczas gdy kolumny o dłuższych wartościach będą miały więcej miejsca. W pierwszej kolejności zmniejszane są kolumny z najdłuższymi nagłówkami (nazwy w nagłówkach należy skrócić jako pierwsze), następnie kolumny z najdłuższymi wartościami są zmniejszane, aż do chwili, gdy wszystkie kolumny pasują do okna końcowego.
UWAGA! Tryb COLUMNS odczytuje wszystkie wyniki zapytania na raz, aby ocenić szerokość kolumny, w związku z tym korzystanie z tego trybu podczas pracy z ogromnymi zestawami wyników jest niebezpieczne. Pamiętaj, że ten tryb załaduje cały wynik do pamięci.

Tryb ROW jest zalecany, jeśli chcesz zobaczyć całe wartości, a nie oczekujesz wyświetlania zbyt wielu wierszy, ponieważ ten tryb wyświetla linię wyjścia dla każdej kolumny, więc otrzymasz 10 linii dla pojedynczego wiersza z 10 kolumnami, a jeśli masz 10 takich wierszy, otrzymasz 100 linii wyjścia (+1 dodatkowy wiersz, aby oddzielić wiersze od siebie).</translation>
    </message>
  </context>
  <context>
    <name>CliCommandNullValue</name>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="9"/>
      <source>Current NULL representation string: %1</source>
      <translation>Aktualny łańcuch reprezentujący wartość NULL: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="15"/>
      <source>tells or changes the NULL representation string</source>
      <translation>wyświetla lub zmienia łąńcuch reprezentujący wartość NULL</translation>
    </message>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="20"/>
      <source>If no argument was passed, it tells what&apos;s the current NULL value representation (that is - what is printed in place of NULL values in query results). If the argument is given, then it&apos;s used as a new string to be used for NULL representation.</source>
      <translation>Jeśli nie poda się argumentu, to wyświetlana jest aktualna reprezentacja wartości NULL (to znaczy to, co jest wyświetlane zamiast wartości NULL w wynikach zapytań). Jeśli podano argument, to staje się on nową reprezentacją wartości NULL.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandOpen</name>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="12"/>
      <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
      <translation>Nie można wywołać %1, gdy żadna z baz nie jest ustawiona jako bieżąca. Okreś bieżącą bazę używając polecenia %2, lub podaj nazwę bazy do %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="29"/>
      <source>Could not add database %1 to list.</source>
      <translation>Nie udało się dodać bazy danych %1 do listy.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="37"/>
      <source>File %1 doesn&apos;t exist in %2. Cannot open inexisting database with %3 command. To create a new database, use %4 command.</source>
      <translation>Plik %1 nie istnieje w %2. Nie można otworzyć nieistniejącej bazy poleceniem %3. Aby utworzyć nową bazę danych, użyj polecenia %4.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="61"/>
      <source>Database %1 has been open and set as the current working database.</source>
      <translation>Baza %1 została otwarta i ustawiona jako bieżąca baza robocza.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="66"/>
      <source>opens database connection</source>
      <translation>otwiera połączenie z bazą</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="71"/>
      <source>Opens connection to the database. If no additional argument was passed, then the connection is open to the current default database (see help for %1 for details). However if an argument was passed, it can be either &lt;name&gt; of the registered database to open, or it can be &lt;path&gt; to the database file to open. In the second case, the &lt;path&gt; gets registered on the list with a generated name, but only for the period of current application session. After restarting application such database is not restored on the list.</source>
      <translation>Otwiera połączenie do bazy. Jeśli nie podano dodatkowych argumentów, to połączenie jest nawiązywane z domyślną bazą (więcej szczegółów w pomocy dla polecenia %1). Natomiast gdy poda się argument, to może to być albo &lt;nazwa&gt; zarejestrowanej bazy danych do otwarcia, lub może to być &lt;ścieżka&gt; do pliku bazy danych do otwarcia. W drugim przypadku &lt;ścieżka&gt; zostanie zarejestrowana na liście baz danych z wygenerowaną nazwą, ale tylko na czas aktualnej sesji aplikacji. Po restarcie aplikacji taka baza danych nie jest przywracana na listę.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="83"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>nazwa</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="83"/>
      <source>path</source>
      <comment>CLI command syntax</comment>
      <translation>ścieżka</translation>
    </message>
  </context>
  <context>
    <name>CliCommandPwd</name>
    <message>
      <location filename="../commands/clicommandpwd.cpp" line="13"/>
      <source>prints the current working directory</source>
      <translation>wypisuje bieżący katalog</translation>
    </message>
    <message>
      <location filename="../commands/clicommandpwd.cpp" line="18"/>
      <source>This is the same as &apos;pwd&apos; command on Unix systems and &apos;cd&apos; command without arguments on Windows. It prints current working directory. You can change the current working directory with %1 command and you can also list contents of the current working directory with %2 command.</source>
      <translation>Jest to polecenie podobne do &apos;pwd&apos; znanego z sytemów Unixowych oraz polecenia &apos;cd&apos; bez argumentów dla systemów Windows. Wypisuje bieżący katalog. Możesz zmienić bieżący katalog za pomocą polecenia %1, oraz możesz wypisać zawartość bieżącego katalogu za pomocą polecenia %2.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandRemove</name>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="12"/>
      <source>No such database: %1</source>
      <translation>Nie ma takiej bazy danych: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="20"/>
      <source>Database removed: %1</source>
      <translation>Baza danych usunięta: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="26"/>
      <source>New current database set:</source>
      <translation>Nowa domyślna baza danych:</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="35"/>
      <source>removes database from the list</source>
      <translation>usuwa bazę danych z listy</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="40"/>
      <source>Removes &lt;name&gt; database from the list of registered databases. If the database was not on the list (see %1 command), then error message is printed and nothing more happens.</source>
      <translation>Usuwa &lt;nazwaną&gt; bazę danych z listy zarejestrowanych baz danych. Jeśli baza nie była na liście (patrz - polecenie %1), to zostanie wyświetlony komunikat błędu i nic się nie stanie.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="50"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>nazwa</translation>
    </message>
  </context>
  <context>
    <name>CliCommandSql</name>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="19"/>
      <source>No working database is set.
Call %1 command to set working database.
Call %2 to see list of all databases.</source>
      <translation>Nie wybrano domyślnej bazy danych.
Użyj polecenia %1, aby ustawić domyślną bazę danych.
Użyj polecenie %2, aby wypisać listę wszystkich baz.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="30"/>
      <source>Database is not open.</source>
      <translation>Baz danych nie jest otwarta.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="65"/>
      <source>executes SQL query</source>
      <translation>wykonuje zapytanie SQL</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="70"/>
      <source>This command is executed every time you enter SQL query in command prompt. It executes the query on the current working database (see help for %1 for details). There&apos;s no sense in executing this command explicitly. Instead just type the SQL query in the command prompt, without any command prefixed.</source>
      <translation>To polecenie jest wywoływane za każdym razem, kiedy wpisujesz zapytanie SQL w linii poleceń. Wykonuje ono zapytanie na bieżącej bazie danych (więcej szczegółów w pomocy dla %1). Nie ma sensu wywoływanie tego polecenia bezpośrednio. Zamiast tego po prostu wpisuj zapytania SQL w linii poleceń, bez polecenia poprzedzającego.</translation>
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
      <translation>Zbyt wiele kolumn, aby wyświetlić w trybie %1.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="254"/>
      <source>Row %1</source>
      <translation>Wiersz %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="404"/>
      <source>Query execution error: %1</source>
      <translation>Błąd wykonywania zapytania: %1</translation>
    </message>
  </context>
  <context>
    <name>CliCommandTables</name>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="15"/>
      <source>No such database: %1. Use %2 to see list of known databases.</source>
      <translation>Nie znaleziono bazy danych: %1. Użyj %2 aby zonaczyć listę znanych baz danych.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="25"/>
      <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
      <translation>Nie można wywołać %1, gdy żadna z baz nie jest ustawiona jako bieżąca. Okreś bieżącą bazę używając polecenia %2, lub podaj nazwę bazy do %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="32"/>
      <source>Database %1 is closed.</source>
      <translation>Baza danych %1 jest zamknięta.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="45"/>
      <location filename="../commands/clicommandtables.cpp" line="47"/>
      <source>Database</source>
      <translation>Baza danych</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="47"/>
      <source>Table</source>
      <translation>Tabela</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="61"/>
      <source>prints list of tables in the database</source>
      <translation>wypisuje listę tabel w bazie danych</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="66"/>
      <source>Prints list of tables in given &lt;database&gt; or in the current working database. Note, that the &lt;database&gt; should be the name of the registered database (see %1). The output list includes all tables from any other databases attached to the queried database.
When the -s option is given, then system tables are also listed.</source>
      <translation>Wypisuje listę tabel w danej &lt;bazie danych&gt; lub w bieżącej bazie danych. &lt;baza danych&gt; powinna być nazwą zarejestrowanej bazy danych (patrz %1). List wyjściowa zawiera wszystkie tabele ze wszystkich baz dołączonych do odpytywanej bazy. Gdy podana jest opcja -s, to również systemowe tabele pojawią się na liście.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="77"/>
      <source>database</source>
      <comment>CLI command syntax</comment>
      <translation>baza danych</translation>
    </message>
  </context>
  <context>
    <name>CliCommandTree</name>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="12"/>
      <source>No current working database is selected. Use %1 to define one and then run %2.</source>
      <translation>Nie wybrano bieżącej bazy danych. Użyj %1 aby taką zdefiniować i wtedy uruchom %2.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="54"/>
      <source>Tables</source>
      <translation>Tabele</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="58"/>
      <source>Views</source>
      <translation>Widoki</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="83"/>
      <source>Columns</source>
      <translation>Kolumny</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="88"/>
      <source>Indexes</source>
      <translation>Indeksy</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="92"/>
      <location filename="../commands/clicommandtree.cpp" line="113"/>
      <source>Triggers</source>
      <translation>Wyzwalacze</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="132"/>
      <source>prints all objects in the database as a tree</source>
      <translation>wypisuje wszystkie obiekty w bazie danych w postaci drzewa</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="137"/>
      <source>Prints all objects (tables, indexes, triggers and views) that are in the database as a tree. The tree is very similar to the one that you can see in GUI client of the SQLiteStudio.
When -c option is given, then also columns will be listed under each table.
When -s option is given, then also system objects will be printed (sqlite_* tables, autoincrement indexes, etc).
The database argument is optional and if provided, then only given database will be printed. This is not a registered database name, but instead it&apos;s an internal SQLite database name, like &apos;main&apos;, &apos;temp&apos;, or any attached database name. To print tree for other registered database, call %1 first to switch the working database, and then use %2 command.</source>
      <translation>Wypisuje wszystkie obiekty (tabele, indeksy, wyzwalacze i widoki) znajdujące się w bazie danych w postaci drzewa. Drzewo to jest podobne do tego, które można zobaczyć w interfejsie graficznym SQLiteStudio.
Kiedy poda się opcję -c, to pod każdą tabelą wylistowane zostaną kolumny.
Kiedy poda się opcję -s, to również obiekty systemowe będą wypisane (tabele sqlite_*, indeksy autoinkrementacji, itp).
Argument bazy danych jest opcjonalny i gdy się go poda, to tylko ta baza zostanie wypisana. Nie jest to nazwa zarejestrowanej nazwy, ale nazwa wewnętrzna bazy SQLite, jak np &apos;main&apos;, &apos;temp&apos;, lub dowolna nazwa dołączonej bazy. Aby wypisać drzewo dla innej zarejestrowanej bazy, użyj najpierw %1, aby zmienić bieżącą bazę danych i wtedy użyj polecenia %2.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandUse</name>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="13"/>
      <source>No current database selected.</source>
      <translation>Bieżąca baza danych nie jest wybrana.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="16"/>
      <location filename="../commands/clicommanduse.cpp" line="30"/>
      <source>Current database: %1</source>
      <translation>Bieżąca baza danych: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="23"/>
      <source>No such database: %1</source>
      <translation>Nie ma takiej bazy danych: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="35"/>
      <source>changes default working database</source>
      <translation>zmienia domyślną bazę danych</translation>
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
      <translation>Zmienia domyślną bazę danych na &lt;nazwaną&gt;. Jeśli &lt;nazwana&gt; baza danych nie jest zarejestrowana w aplikacji, to wyświetlony zostanie komunikat błędu i żadne zmiany nie nastąpią.

Czym jest domyślna baza danych?
Kiedy piszesz zapytanie SQL do wykonania, jest ono wykonywane na domyślnej bazie danych, która jest również nazywana bieżącą bazą danych. Większość poleceń związanych z bazą danych może pracować z użyciem domyślnej bazy danych, jeśli nie poda się bazy w ich argumentach. Bieżąca baza danych jest zawsze widoczna w wierszu poleceń. Domyślna baza danych jest zawsze zdefiniowana (z wyjątkiem, gdy nie ma żadnej bazy na liście).

Domyślna baza danych może być wybrana na kilka sposobów:
- używając polecenia %1,
- podając plik bazy danych jako parametr do uruchomienia aplikacji,
- podając nazwę zarejestrowanej bazy danych jako parametr do uruchomienia aplikacji,
- lub gdy domyślna baza nie została wybrana przez żadne z powyższych, to pierwsza baza z listy zarejestrowanych baz stanie się domyślną.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="63"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>nazwa</translation>
    </message>
  </context>
  <context>
    <name>QObject</name>
    <message>
      <location filename="../clicommandsyntax.cpp" line="155"/>
      <source>Insufficient number of arguments.</source>
      <translation>Niewystarająca liczba arugmentów.</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="325"/>
      <source>Too many arguments.</source>
      <translation>Za dużo argumentów.</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="347"/>
      <source>Invalid argument value: %1.
Expected one of: %2</source>
      <translation>Niepoprawna wartość argumentu: %1.
Oczekiwano jednej z: %2</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="383"/>
      <source>Unknown option: %1</source>
      <comment>CLI command syntax</comment>
      <translation>Nieznana opcja: %1</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="394"/>
      <source>Option %1 requires an argument.</source>
      <comment>CLI command syntax</comment>
      <translation>Opcja %1 wymaga argumentu.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="31"/>
      <source>string</source>
      <comment>CLI command syntax</comment>
      <translation>łańcuch</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="28"/>
      <source>Command line interface to SQLiteStudio, a SQLite manager.</source>
      <translation>Interfejs linii poleceń dla SQLiteStudio, menażera SQLite.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="32"/>
      <source>Enables debug messages on standard error output.</source>
      <translation>Włącza wiadomości debugujące na standardowym wyjściu błędów.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="33"/>
      <source>Enables Lemon parser debug messages for SQL code assistant.</source>
      <translation>Włącza wiadomości debugujące analizatora Lemon dla asystenta kodu SQL.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="34"/>
      <source>Lists plugins installed in the SQLiteStudio and quits.</source>
      <translation>Wypisuje listę zainstalowanych w SQLiteStudio wtyczek i wychodzi.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="36"/>
      <source>Executes provided SQL file (including all rich features of SQLiteStudio&apos;s query executor) on the specified database file and quits. The database parameter becomes mandatory if this option is used.</source>
      <translation>Wykonuje podany plik SQL (w tym wszystkie bogate funkcje wykonywania zapytań w SQLiteStudio) na określonym pliku bazy danych i wychodzi z programu. Parametr bazy danych staje się obowiązkowy, jeśli ta opcja jest używana.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="39"/>
      <source>SQL file</source>
      <translation>Plik SQL</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="40"/>
      <source>Character encoding to use when reading SQL file (-e option). Use -cl to list available codecs. Defaults to %1.</source>
      <translation>Kodowanie znaków do użycia podczas czytania pliku SQL (opcja -e). Użyj -cl aby wyświetlić dostępne kodowania. Domyślnie %1.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="43"/>
      <source>codec</source>
      <translation>kodek</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="44"/>
      <source>Lists available codecs to be used with -c option and quits.</source>
      <translation>Wyświetla dostępne kodowania do użycia z opcją -c i wychodzi z programu.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="46"/>
      <source>When used together with -e option, the execution will not stop on an error, but rather continue until the end, ignoring errors.</source>
      <translation>Gdy używany jest razem z opcją -e, wykonanie nie zatrzyma się na błędzie, ale będzie kontynuowane do końca, ignorując błędy.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="57"/>
      <source>file</source>
      <translation>plik</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="57"/>
      <source>Database file to open</source>
      <translation>Baza danych do otwarcia</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="78"/>
      <source>Invalid codec: %1. Use -cl option to list available codecs.</source>
      <translation>Nieprawidłowe kodowanie: %1. Użyj opcji -cl, aby wyświetlić dostępne kodowania.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="108"/>
      <source>Database file argument is mandatory when executing SQL file.</source>
      <translation>Argument pliku bazy danych jest obowiązkowy podczas wykonywania pliku SQL.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="114"/>
      <source>Could not open specified database for executing SQL file. You may try using -d option to find out more details.</source>
      <translation>Nie można otworzyć określonej bazy danych do wykonania pliku SQL. Spróbuj użyć opcji -d, aby poznać więcej szczegółów.</translation>
    </message>
  </context>
</TS>
