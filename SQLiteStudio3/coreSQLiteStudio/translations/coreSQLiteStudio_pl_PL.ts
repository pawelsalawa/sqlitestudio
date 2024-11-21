<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="pl" sourcelanguage="en">
  <context>
    <name>AbstractDb</name>
    <message>
      <location filename="../db/abstractdb.cpp" line="349"/>
      <location filename="../db/abstractdb.cpp" line="366"/>
      <source>Cannot execute query on closed database.</source>
      <translation>Nie można wykonać zapytania na zamkniętej bazie danych.</translation>
    </message>
    <message>
      <location filename="../db/abstractdb.cpp" line="661"/>
      <source>Error attaching database %1: %2</source>
      <translation>Błąd podczas dołączania bazy danych %1: %2</translation>
    </message>
    <message>
      <location filename="../db/abstractdb.cpp" line="931"/>
      <source>Failed to make full WAL checkpoint on database &apos;%1&apos;. Error returned from SQLite engine: %2</source>
      <translation>Nie udało się utworzyć pełnego punktu kontrolnego WAL w bazie danych &apos;%1&apos;. Błąd zwrócony z silnika SQLite: %2</translation>
    </message>
  </context>
  <context>
    <name>ChainExecutor</name>
    <message>
      <location filename="../db/chainexecutor.cpp" line="37"/>
      <source>The database for executing queries was not defined.</source>
      <comment>chain executor</comment>
      <translation>Nie zdefiniowano bazy danych do wykonywania zapytań.</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="44"/>
      <source>The database for executing queries was not open.</source>
      <comment>chain executor</comment>
      <translation>Baza danych do wykonywania zapytań nie jest otwarta.</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="54"/>
      <source>Could not disable foreign keys in the database. Details: %1</source>
      <comment>chain executor</comment>
      <translation>Nie udało się wyłączyć kluczy obcych w bazie. Szczegóły: %1</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="62"/>
      <source>Could not start a database transaction. Details: %1</source>
      <comment>chain executor</comment>
      <translation>Nie udało się rozpocząć transakcji bazy danych. Szczegóły: %1</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="89"/>
      <source>Interrupted</source>
      <comment>chain executor</comment>
      <translation>Przerwane</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="151"/>
      <source>Could not commit a database transaction. Details: %1</source>
      <comment>chain executor</comment>
      <translation>Nie udało się zatwierdzić transakcji bazy danych. Szczegóły: %1</translation>
    </message>
  </context>
  <context>
    <name>CompletionHelper</name>
    <message>
      <location filename="../completionhelper.cpp" line="159"/>
      <source>New row reference</source>
      <translation>Odnośnik do nowego wiersza</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="166"/>
      <source>Old row reference</source>
      <translation>Odnośnik do starego wiersza</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="171"/>
      <source>New table name</source>
      <translation>Nazwa nowej tabeli</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="174"/>
      <source>New index name</source>
      <translation>Nazwa nowego indeksu</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="177"/>
      <source>New view name</source>
      <translation>Nazwa nowego widoku</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="180"/>
      <source>New trigger name</source>
      <translation>Nazwa nowego wyzwalacza</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="183"/>
      <source>Table or column alias</source>
      <translation>Alias tabeli lub kolumny</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="186"/>
      <source>transaction name</source>
      <translation>Nazwa transakcji</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="189"/>
      <source>New column name</source>
      <translation>Nazwa nowej kolumny</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="192"/>
      <source>Column data type</source>
      <translation>Typ danych kolumny</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="195"/>
      <source>Constraint name</source>
      <translation>Nazwa ograniczenia</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="208"/>
      <source>Error message</source>
      <translation>Treść błędu</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="257"/>
      <source>Any word</source>
      <translation>Dowolne słowo</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="438"/>
      <source>Default database</source>
      <translation>Domyślna baza danych</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="439"/>
      <source>Temporary objects database</source>
      <translation>Baza danych obiektów tymczasowych</translation>
    </message>
  </context>
  <context>
    <name>ConfigImpl</name>
    <message>
      <location filename="../services/impl/configimpl.cpp" line="881"/>
      <source>Could not start database transaction for deleting SQL history, therefore it&apos;s not deleted.</source>
      <translation>Nie można rozpocząć transakcji dla usuwania historii SQL, więc nie można usunąć historii.</translation>
    </message>
    <message>
      <location filename="../services/impl/configimpl.cpp" line="888"/>
      <source>Could not commit database transaction for deleting SQL history, therefore it&apos;s not deleted.</source>
      <translation>Nie można zatwierdzić transakcji dla usuwania historii SQL, więc nie można usunąć historii.</translation>
    </message>
  </context>
  <context>
    <name>DbManagerImpl</name>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="64"/>
      <source>Could not add database %1: %2</source>
      <translation>Nie udało się dodać bazę danych %1: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="137"/>
      <source>Database %1 could not be updated, because of an error: %2</source>
      <translation>Nie udało się zaktualizować baza danych %1 z powodu błędu: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="353"/>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="382"/>
      <source>Database file doesn&apos;t exist.</source>
      <translation>Plik bazy danych nie istnieje.</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="355"/>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="384"/>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="603"/>
      <source>No supporting plugin loaded.</source>
      <translation>Nie załadowano obsługującej wtyczki.</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="521"/>
      <source>Database could not be initialized.</source>
      <translation>Nie udało się zainicjalizować bazy danych.</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="531"/>
      <source>No suitable database driver plugin found.</source>
      <translation>Nie znaleziono odpowiedniej wtyczki sterownika.</translation>
    </message>
  </context>
  <context>
    <name>DbObjectOrganizer</name>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="372"/>
      <location filename="../dbobjectorganizer.cpp" line="403"/>
      <source>Error while creating table in target database: %1</source>
      <translation>Błąd podczas tworzenia tabeli w docelowej bazie danych: %1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="372"/>
      <source>Could not parse table.</source>
      <translation>Nie udało się przeanalizować tabeli.</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="417"/>
      <source>Database %1 could not be attached to database %2, so the data of table %3 will be copied with SQLiteStudio as a mediator. This method can be slow for huge tables, so please be patient.</source>
      <translation>Nie udało się dołączyć bazy danych %1 do bazy danych %2, więc dane tabeli %3 będą skopiowane przez SQLiteStudio jako pośrednika. Ta metoda może być powolna dla dużych tabel, więc proszę o cierpliwość.</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="441"/>
      <source>Error while copying data for table %1: %2</source>
      <translation>Błąd podczas copiowania danych tabeli %1: %2</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="460"/>
      <location filename="../dbobjectorganizer.cpp" line="467"/>
      <location filename="../dbobjectorganizer.cpp" line="494"/>
      <source>Error while copying data to table %1: %2</source>
      <translation>Błąd podczas kopiowania danych do tabeli %1: %2</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="516"/>
      <source>Error while dropping source view %1: %2
Tables, indexes, triggers and views copied to database %3 will remain.</source>
      <translation>Błąd podczas upuszczania widoku źródłowego %1: %2
Tabele, indeksy, wyzwalacze i widoki skopiowane do bazy danych %3 pozostaną na miejscu.</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="523"/>
      <source>Error while creating view in target database: %1</source>
      <translation>Błąd podczas tworzenia widoku w docelowej bazie danych: %1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="528"/>
      <source>Error while creating index in target database: %1</source>
      <translation>Błąd podczas tworzenia indeksu w docelowej bazie danych: %1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="533"/>
      <source>Error while creating trigger in target database: %1</source>
      <translation>Błąd podczas tworzenia wyzwalacza w docelowej bazie danych: %1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="664"/>
      <location filename="../dbobjectorganizer.cpp" line="671"/>
      <location filename="../dbobjectorganizer.cpp" line="680"/>
      <source>Could not parse object &apos;%1&apos; in order to move or copy it.</source>
      <translation>Nie można zanalizować obiektu &apos;%1&apos; w celu przeniesienia lub skopiowania go.</translation>
    </message>
  </context>
  <context>
    <name>DdlHistoryModel</name>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="66"/>
      <source>Database name</source>
      <comment>ddl history header</comment>
      <translation>Nazwa bazy danych</translation>
    </message>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="68"/>
      <source>Database file</source>
      <comment>ddl history header</comment>
      <translation>Plik bazy danych</translation>
    </message>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="70"/>
      <source>Date of execution</source>
      <comment>ddl history header</comment>
      <translation>Data wykonania</translation>
    </message>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="72"/>
      <source>Changes</source>
      <comment>ddl history header</comment>
      <translation>Zmiany</translation>
    </message>
  </context>
  <context>
    <name>ExportManager</name>
    <message>
      <location filename="../services/exportmanager.cpp" line="71"/>
      <source>Export plugin %1 doesn&apos;t support exporing query results.</source>
      <translation>Wtyczka eksportu %1 nie obsługuje exportowania wyników zapytania.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="97"/>
      <source>Export plugin %1 doesn&apos;t support exporing tables.</source>
      <translation>Wtyczka exportu %1 nie obsługuje eksportowania tabel.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="121"/>
      <source>Export plugin %1 doesn&apos;t support exporing databases.</source>
      <translation>Wtyczka exportu %1 nie obsługuje eksportowania baz danych.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="154"/>
      <source>Export format &apos;%1&apos; is not supported. Supported formats are: %2.</source>
      <translation>Format eksportu %1 nie jest obsługiwany. Obsługiwane formaty to: %2</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="218"/>
      <source>Export to the clipboard was successful.</source>
      <translation>Eksport do schowka przebiegł pomyślnie.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="222"/>
      <source>Export to the file &apos;%1&apos; was successful.</source>
      <translation>Eksport do pliku &apos;%1&apos; przebiegł pomyślnie.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="224"/>
      <source>Export was successful.</source>
      <translation>Export przebiegł pomyślnie.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="266"/>
      <source>Could not export to file %1. File cannot be open for writting.</source>
      <translation>Eksport do pliku %1 nie powiódł się. Plik nie może być otwarty do zapisu.</translation>
    </message>
  </context>
  <context>
    <name>ExportWorker</name>
    <message>
      <location filename="../exportworker.cpp" line="123"/>
      <source>Error while exporting query results: %1</source>
      <translation>Błąd podczas eksportowania wyników zapytania: %1</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="209"/>
      <source>Error while counting data column width to export from query results: %1</source>
      <translation>Błąd podczas liczenia szerokości kolumn danych do eksportu wyników zapytania: %1</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="353"/>
      <location filename="../exportworker.cpp" line="411"/>
      <source>Could not parse %1 in order to export it. It will be excluded from the export output.</source>
      <translation>Nie udało się przeanalizować %1 w celu wyeksportowania. Element ten zostanie pominięty w wynikach eksportu.</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="615"/>
      <source>Error while reading data to export from table %1: %2</source>
      <translation>Błąd podczas odczytu danych do eksportu z tabeli %1: %2</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="623"/>
      <source>Error while counting data to export from table %1: %2</source>
      <translation>Błąd podczas liczenia danych do eksportu z tabeli %1: %2</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="639"/>
      <source>Error while counting data column width to export from table %1: %2</source>
      <translation>Błąd podczas obliczania szerokości kolumn danych do eksportu z tabeli %1: %2</translation>
    </message>
  </context>
  <context>
    <name>FunctionManagerImpl</name>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="197"/>
      <source>Could not create scripting context, probably the plugin is not configured properly</source>
      <translation>Nie można utworzyć kontekstu skryptowego, prawdopodobnie wtyczka nie jest poprawnie skonfigurowana</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="290"/>
      <source>Invalid number of arguments to function &apos;%1&apos;. Expected %2, but got %3.</source>
      <translation>Niepoprawna liczba argumentów do funkcji &apos;%1&apos;. Oczekiwano %2, a jest %3.</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="404"/>
      <source>No such function registered in SQLiteStudio: %1(%2)</source>
      <translation>Nie znaleziono funkcji zarejestrowanej w SQLiteStudio: %1 (%2)</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="410"/>
      <source>Function %1(%2) was registered with language %3, but the plugin supporting that language is not currently loaded.</source>
      <translation>Funkcja %1 (%2) została zarejestrowana dla języka %3, ale wtyczka obsługująca ten język nie jest aktualnie załadowana.</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="428"/>
      <source>Invalid regular expression pattern: %1</source>
      <translation>Niepoprawne wyrażenie regularne: %1</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="447"/>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="480"/>
      <source>Could not open file %1 for reading: %2</source>
      <translation>Nie udało się otworzyć pliku %1 do odczytu: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="502"/>
      <source>Could not open file %1 for writting: %2</source>
      <translation>Nie udało się otworzyć pliku %2 do zapisu: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="522"/>
      <source>Error while writting to file %1: %2</source>
      <translation>Błąd podczas zapisu do pliku %1: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="540"/>
      <source>Unsupported scripting language: %1</source>
      <translation>Nieobsługiwany język skryptowy: %1</translation>
    </message>
  </context>
  <context>
    <name>GenericExportPlugin</name>
    <message>
      <location filename="../plugins/genericexportplugin.cpp" line="20"/>
      <source>Could not initialize text codec for exporting. Using default codec: %1</source>
      <translation>Nie udało się zainicjalizować kodeka do exportu. Użyty będzie domyślny kodek: %1</translation>
    </message>
  </context>
  <context>
    <name>ImportManager</name>
    <message>
      <location filename="../services/importmanager.cpp" line="99"/>
      <source>Imported data to the table &apos;%1&apos; successfully. Number of imported rows: %2</source>
      <translation>Pomyślnie zaimportowano dane do tabeli &apos;%1&apos;. Liczba zaimportowanych wierszy: %2</translation>
    </message>
  </context>
  <context>
    <name>ImportWorker</name>
    <message>
      <location filename="../importworker.cpp" line="25"/>
      <source>No columns provided by the import plugin.</source>
      <translation>Wtyczka importu nie dostarczyła żadnych kolumn.</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="31"/>
      <source>Could not start transaction in order to import a data: %1</source>
      <translation>Nie udało się wystartować transakcji w celu zaimportowania danych: %1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="54"/>
      <source>Could not commit transaction for imported data: %1</source>
      <translation>Nie udało się zatwierdzić transakcji w celu zaimportowania danych: %1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="102"/>
      <source>Table &apos;%1&apos; has less columns than there are columns in the data to be imported. Excessive data columns will be ignored.</source>
      <translation>Tabela &apos;%1&apos; ma mniej kolumn, niż jest kolumn w danych do importu. Nadmiarowe kolumny zostaną zignorowane.</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="107"/>
      <source>Table &apos;%1&apos; has more columns than there are columns in the data to be imported. Some columns in the table will be left empty.</source>
      <translation>Tabela &apos;%1&apos; ma więcej kolumn, niż jest kolumn w danych do importu. Część kolumn w tabeli będzie pozostawiona pusta.</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="126"/>
      <source>Could not create table to import to: %1</source>
      <translation>Nie udało się stworzyć tabeli do zaimportowania: %1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="135"/>
      <location filename="../importworker.cpp" line="186"/>
      <location filename="../importworker.cpp" line="193"/>
      <source>Error while importing data: %1</source>
      <translation>Błąd podczas importowania danych: %1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="135"/>
      <location filename="../importworker.cpp" line="193"/>
      <source>Interrupted.</source>
      <comment>import process status update</comment>
      <translation>Przerwano.</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="181"/>
      <source>Could not import data row number %1. The row was ignored. Problem details: %2</source>
      <translation>Nie udało się zaimportować wiersza danych numer %1. Wiersz ten został zignorowany. Szczegóły problemu: %2</translation>
    </message>
  </context>
  <context>
    <name>PluginManagerImpl</name>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="546"/>
      <source>Cannot load plugin %1, because it&apos;s in conflict with plugin %2.</source>
      <translation>Nie udało się załadować wtyczki %1, ponieważ jest ona w konflikcie z wtyczką %2.</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="557"/>
      <source>Cannot load plugin %1, because its dependency was not loaded: %2.</source>
      <translation>Nie udało się załadować wtyczki %1, ponieważ jej zależność nie została załadowana: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="566"/>
      <source>Cannot load plugin %1. Error details: %2</source>
      <translation>Nie udało się załadować wtyczki %1. Szczegóły błędu: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="582"/>
      <source>Cannot load plugin %1 (error while initializing plugin).</source>
      <translation>Nie udało się załadować wtyczki %1 (błąd podczas inicjalizacji wtyczki).</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="743"/>
      <source>min: %1</source>
      <comment>plugin dependency version</comment>
      <translation>min: %1</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="744"/>
      <source>max: %1</source>
      <comment>plugin dependency version</comment>
      <translation>maks: %1</translation>
    </message>
  </context>
  <context>
    <name>PopulateConstant</name>
    <message>
      <location filename="../plugins/populateconstant.cpp" line="10"/>
      <source>Constant</source>
      <comment>populate constant plugin name</comment>
      <translation>Stała</translation>
    </message>
  </context>
  <context>
    <name>PopulateConstantConfig</name>
    <message>
      <location filename="../plugins/populateconstant.ui" line="20"/>
      <source>Constant value:</source>
      <translation>Stała wartość:</translation>
    </message>
  </context>
  <context>
    <name>PopulateDictionary</name>
    <message>
      <location filename="../plugins/populatedictionary.cpp" line="17"/>
      <source>Dictionary</source>
      <comment>dictionary populating plugin name</comment>
      <translation>Słownik</translation>
    </message>
  </context>
  <context>
    <name>PopulateDictionaryConfig</name>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="20"/>
      <source>Dictionary file</source>
      <translation>Plik słownika</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="29"/>
      <source>Pick dictionary file</source>
      <translation>Wybierz plik słownika</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="39"/>
      <source>Word separator</source>
      <translation>Separator słowa</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="45"/>
      <source>Whitespace</source>
      <translation>Biały znak</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="58"/>
      <source>Line break</source>
      <translation>Nowa linia</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="74"/>
      <source>Method of using words</source>
      <translation>Metoda używania słów</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="80"/>
      <source>Ordered</source>
      <translation>Uporządkowana</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="93"/>
      <source>Randomly</source>
      <translation>Losowa</translation>
    </message>
  </context>
  <context>
    <name>PopulateManager</name>
    <message>
      <location filename="../services/populatemanager.cpp" line="89"/>
      <source>Table &apos;%1&apos; populated successfully.</source>
      <translation>Zaludnianie tabeli &apos;%1&apos; przebiegło pomyślnie.</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandom</name>
    <message>
      <location filename="../plugins/populaterandom.cpp" line="13"/>
      <source>Random number</source>
      <translation>Losowa liczba</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandomConfig</name>
    <message>
      <location filename="../plugins/populaterandom.ui" line="20"/>
      <source>Constant prefix</source>
      <translation>Stały przedrostek</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="26"/>
      <source>No prefix</source>
      <translation>Bez predrostka</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="39"/>
      <source>Minimum value</source>
      <translation>Wartość minimalna</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="61"/>
      <source>Maximum value</source>
      <translation>Wartość maksymalna</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="86"/>
      <source>Constant suffix</source>
      <translation>Stały przyrostek</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="92"/>
      <source>No suffix</source>
      <translation>Brak przyrostka</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandomText</name>
    <message>
      <location filename="../plugins/populaterandomtext.cpp" line="14"/>
      <source>Random text</source>
      <translation>Losowy tekst</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandomTextConfig</name>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="20"/>
      <source>Use characters from common sets:</source>
      <translation>Użyj znaków z zestawów:</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="36"/>
      <source>Minimum length</source>
      <translation>Długość minimalna</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="64"/>
      <source>Letters from a to z.</source>
      <translation>Litery od a do z.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="67"/>
      <source>Alpha</source>
      <translation>Litery</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="77"/>
      <source>Numbers from 0 to 9.</source>
      <translation>Liczby od 0 do 9.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="80"/>
      <source>Numeric</source>
      <translation>Liczby</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="90"/>
      <source>A whitespace, a tab and a new line character.</source>
      <translation>Znak biały, znak tabulacji, znak nowej linii.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="93"/>
      <source>Whitespace</source>
      <translation>Znak biały</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="103"/>
      <source>Includes all above and all others.</source>
      <translation>Zawiera wszystkie powyższe, oraz wszystkie pozostałe.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="106"/>
      <source>Binary</source>
      <translation>Binarne</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="119"/>
      <source>Use characters from my custom set:</source>
      <translation>Użyj znaków z mojego zestawu:</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="132"/>
      <source>Maximum length</source>
      <translation>Długość maksymalna</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="160"/>
      <source>If you type some character multiple times, it&apos;s more likely to be used.</source>
      <translation>Jeśli wpiszesz dany znak kilka razy, będzie on miał większą szansę na wylosowanie.</translation>
    </message>
  </context>
  <context>
    <name>PopulateScript</name>
    <message>
      <location filename="../plugins/populatescript.cpp" line="34"/>
      <source>Script</source>
      <translation>Skrypt</translation>
    </message>
  </context>
  <context>
    <name>PopulateScriptConfig</name>
    <message>
      <location filename="../plugins/populatescript.ui" line="26"/>
      <source>Initialization code (optional)</source>
      <translation>Kod inicjalizujący (opcjonalny)</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.ui" line="45"/>
      <source>Per step code</source>
      <translation>Kod dla każdego kroku</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.ui" line="70"/>
      <source>Language</source>
      <translation>Język</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.ui" line="89"/>
      <source>Help</source>
      <translation>Pomoc</translation>
    </message>
  </context>
  <context>
    <name>PopulateSequence</name>
    <message>
      <location filename="../plugins/populatesequence.cpp" line="13"/>
      <source>Sequence</source>
      <translation>Sekwencja</translation>
    </message>
  </context>
  <context>
    <name>PopulateSequenceConfig</name>
    <message>
      <location filename="../plugins/populatesequence.ui" line="33"/>
      <source>Start value:</source>
      <translation>Wartość początkowa:</translation>
    </message>
    <message>
      <location filename="../plugins/populatesequence.ui" line="56"/>
      <source>Step:</source>
      <translation>Krok:</translation>
    </message>
  </context>
  <context>
    <name>PopulateWorker</name>
    <message>
      <location filename="../populateworker.cpp" line="23"/>
      <source>Could not start transaction in order to perform table populating. Error details: %1</source>
      <translation>Nie udało się rozpocząć transakcji w celu zaludnienia tabeli. Szczegóły błędu: %1</translation>
    </message>
    <message>
      <location filename="../populateworker.cpp" line="69"/>
      <source>Error while populating table: %1</source>
      <translation>Błąd podczas zaludniania tabeli: %2</translation>
    </message>
    <message>
      <location filename="../populateworker.cpp" line="80"/>
      <source>Could not commit transaction after table populating. Error details: %1</source>
      <translation>Nie udało się zatwierdzić transakcji po zaludnieniu tabeli. Szczegóły błędy: %1</translation>
    </message>
  </context>
  <context>
    <name>QObject</name>
    <message>
      <location filename="../common/utils.cpp" line="902"/>
      <source>Could not open file &apos;%1&apos; for reading: %2</source>
      <translation>Nie można otworzyż pliku &apos;%1&apos; do odczytu: %2</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="436"/>
      <source>Could not open database: %1</source>
      <translation>Nie udało się otworzyć bazy danych: %1</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="1225"/>
      <source>Result set expired or no row available.</source>
      <translation>Wyniki zapytania są nieaktualne, lub nie ma dostępnych wierszy.</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="332"/>
      <location filename="../db/abstractdb3.h" line="336"/>
      <source>Could not load extension %1: %2</source>
      <translation>Nie udało się załadować rozszerzenia %1: %2</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="422"/>
      <source>Could not run WAL checkpoint: %1</source>
      <translation>Nie można uruchomić punktu kontrolnego WAL: %1</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="460"/>
      <source>Could not close database: %1</source>
      <translation>Nie udało się zamknąć bazy danych: %1</translation>
    </message>
    <message>
      <location filename="../impl/dbattacherimpl.cpp" line="114"/>
      <source>Could not attach database %1: %2</source>
      <translation>Nie udało się dołączyć bazy danych %1: %2</translation>
    </message>
    <message>
      <location filename="../parser/parsercontext.cpp" line="108"/>
      <location filename="../parser/parsercontext.cpp" line="110"/>
      <source>Incomplete query.</source>
      <translation>Niekompletne zapytanie.</translation>
    </message>
    <message>
      <location filename="../parser/sqlite3_parse.cpp" line="2526"/>
      <source>Parser stack overflow</source>
      <translation>Przeciążenie stosu analizatora.</translation>
    </message>
    <message>
      <location filename="../parser/sqlite3_parse.cpp" line="5937"/>
      <source>Syntax error</source>
      <translation>Błąd składni</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.cpp" line="32"/>
      <source>Could not open dictionary file %1 for reading.</source>
      <translation>Nie udało się otworzyć pliku słownika %1 do odczytu.</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.cpp" line="93"/>
      <source>Dictionary file must exist and be readable.</source>
      <translation>Plik słownika musi istnieć i musisz mieć prawa do jego odczytu.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.cpp" line="54"/>
      <source>Maximum value cannot be less than minimum value.</source>
      <translation>Wartość maksymalna nie może być mniejsza niż wartość minimalna.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.cpp" line="79"/>
      <source>Maximum length cannot be less than minimum length.</source>
      <translation>Długość maksymalna nie może być mniejsza niż długość minimalna.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.cpp" line="90"/>
      <source>Custom character set cannot be empty.</source>
      <translation>Zestaw własnych znaków nie może być pusty.</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="61"/>
      <source>Could not find plugin to support scripting language: %1</source>
      <translation>Nie udało się znaleźć wtyczki obsługującej język skryptowy: %1</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="70"/>
      <source>Could not get evaluation context, probably the %1 scripting plugin is not configured properly</source>
      <translation>Nie można znaleźć kontekstu skryptowego, prawdopodobnie wtyczka %1 nie jest poprawnie skonfigurowana</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="84"/>
      <source>Error while executing populating initial code: %1</source>
      <translation>Błąd podczas wykonywania kodu inicjalizującego zaludnianie: %1</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="106"/>
      <source>Error while executing populating code: %1</source>
      <translation>Błąd podczas wykonywania kodu zaludniania: %1</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="138"/>
      <source>Select implementation language.</source>
      <translation>Wybierz język implementacji.</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="139"/>
      <source>Implementation code cannot be empty.</source>
      <translation>Kod implementacji nie może być pusty.</translation>
    </message>
    <message>
      <location filename="../selectresolver.cpp" line="372"/>
      <source>Could not resolve data source for column: %1</source>
      <translation>Nie znaleziono źródła danych dla kolumny: %1</translation>
    </message>
    <message>
      <location filename="../selectresolver.cpp" line="442"/>
      <source>Could not resolve table for column &apos;%1&apos;.</source>
      <translation>Nie można ustalić tabeli lub kolumny &apos;%1&apos;.</translation>
    </message>
    <message>
      <location filename="../services/impl/configimpl.cpp" line="761"/>
      <source>Could not initialize configuration file. Any configuration changes and queries history will be lost after application restart. Unable to create a file at following locations: %1.</source>
      <translation>Nie udało się zainicjalizować pliku konfiguracyjnego. Wszelkie zmiany w konfiguracji i historia zapytań zostaną utracone po restarcie aplikacji. Nie udało się utworzyć pliku w lokalizacji: %1.</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="347"/>
      <source>General purpose</source>
      <comment>plugin category name</comment>
      <translation>Ogólne</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="348"/>
      <source>Database support</source>
      <comment>plugin category name</comment>
      <translation>Wsparcie baz danych</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="349"/>
      <source>Code formatter</source>
      <comment>plugin category name</comment>
      <translation>Formatowanie kodu</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="350"/>
      <source>Scripting languages</source>
      <comment>plugin category name</comment>
      <translation>Języki skryptowe</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="352"/>
      <source>Exporting</source>
      <comment>plugin category name</comment>
      <translation>Eksportowanie</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="353"/>
      <source>Importing</source>
      <comment>plugin category name</comment>
      <translation>Importowanie</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="354"/>
      <source>Table populating</source>
      <comment>plugin category name</comment>
      <translation>Zaludnianie tabel</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="161"/>
      <source>Table %1 is referencing table %2, but the foreign key definition will not be updated for new table definition due to problems while parsing DDL of the table %3.</source>
      <translation>Tabela %1 odwołuje się do tabeli %2, ale definicja klucza obcego nie zostanie zaktualizowane dla definicji nowej tabeli w związku z problemami przy analizowaniu DDL tabeli %3.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="510"/>
      <source>All columns indexed by the index %1 are gone. The index will not be recreated after table modification.</source>
      <translation>Wszystkie kolumny indeksowane przez indeks %1 już nie istnieją. Indeks ten nie będzie odtworzony po modyfikacji tabeli.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="554"/>
      <source>There is problem with proper processing trigger %1. It may be not fully updated afterwards and will need your attention.</source>
      <translation>Wystąpił problem z poprawnym przetworzeniem wyzwalacza %1. Może on zostać zaktualizowany tylko częściowo i będzie wymagał twojej uwagi.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="569"/>
      <source>All columns covered by the trigger %1 are gone. The trigger will not be recreated after table modification.</source>
      <translation>Wszystkie kolumny obsługiwane przez wyzwalacz %1 już nie istnieją. Wyzwalacz ten nie będzie odtworzony po modyfikacji tabeli.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="601"/>
      <source>Cannot not update trigger %1 according to table %2 modification.</source>
      <translation>Nie można zaktualizować wyzwalacza %1 zgodnie z modyfikacjami tabeli %2.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="620"/>
      <source>Cannot not update view %1 according to table %2 modifications.
The view will remain as it is.</source>
      <translation>Nie można zaktualizować widoku %1 w związku z modyfikacjami tabeli %2.
Widok pozostanie nienaruszony.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="782"/>
      <location filename="../tablemodifier.cpp" line="806"/>
      <location filename="../tablemodifier.cpp" line="825"/>
      <source>There is a problem with updating an %1 statement within %2 trigger. One of the %1 substatements which might be referring to table %3 cannot be properly modified. Manual update of the trigger may be necessary.</source>
      <translation>Jest problem ze zaktualizowaniem zapytania %1 w wyzwalaczu %2. Jedeno z podzapytań %1, które może odwoływać się do tabeli %3 nie może być poprawnie zmodyfikowane. Ręczna aktualizacja tego wyzwalacza może być niezbędna.</translation>
    </message>
    <message>
      <location filename="../viewmodifier.cpp" line="24"/>
      <source>Could not parse DDL of the view to be created. Details: %1</source>
      <translation>Nie udało się przeanalizować DDL widoku do stworzenia. Szczegóły: %1</translation>
    </message>
    <message>
      <location filename="../viewmodifier.cpp" line="33"/>
      <source>Parsed query is not CREATE VIEW. It&apos;s: %1</source>
      <translation>Przeanalizowane zapytanie to nie CREATE VIEW, ale: %1</translation>
    </message>
    <message>
      <location filename="../viewmodifier.cpp" line="81"/>
      <source>SQLiteStudio was unable to resolve columns returned by the new view, therefore it won&apos;t be able to tell which triggers might fail during the recreation process.</source>
      <translation>SQLiteStudio nie było w stanie określić kolumn zwracanych przez nowy widok, w związku z czym nie może określić które wyzwalacze mogą się nie powieść podczas procesu odtwarzania.</translation>
    </message>
  </context>
  <context>
    <name>QueryExecutor</name>
    <message>
      <location filename="../db/queryexecutor.cpp" line="196"/>
      <source>Execution interrupted.</source>
      <translation>Wykonywanie przerwane.</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="237"/>
      <source>Database is not open.</source>
      <translation>Baza danych nie jest otwarta.</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="245"/>
      <source>Only one query can be executed simultaneously.</source>
      <translation>Tylko jedno zapytanie może być wykonywane w danym momencie.</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="349"/>
      <location filename="../db/queryexecutor.cpp" line="598"/>
      <source>An error occured while executing the count(*) query, thus data paging will be disabled. Error details from the database: %1</source>
      <translation>Wystąpił błąd podczas wykonywania zapytania count(*), przez co stronicowanie danych będzie wyłączone. Szczegóły błędy z bazy danych: %1</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="509"/>
      <source>SQLiteStudio was unable to extract metadata from the query. Results won&apos;t be editable.</source>
      <translation>SQLiteStudio nie mogło uzyskać metadanych z zapytania. Nie będzie można edytować wyników zapytania.</translation>
    </message>
  </context>
  <context>
    <name>QueryExecutorSmartHints</name>
    <message>
      <location filename="../db/queryexecutorsteps/queryexecutorsmarthints.cpp" line="77"/>
      <source>Column %1 in table %2 is referencing column %3 in table %4, but these columns have different data types: %5 vs. %6. This may cause issues related to foreign key value matching.</source>
      <translation type="unfinished">Column %1 in table %2 is referencing column %3 in table %4, but these columns have different data types: %5 vs. %6. This may cause issues related to foreign key value matching.</translation>
    </message>
  </context>
  <context>
    <name>ScriptingQtDbProxy</name>
    <message>
      <location filename="../plugins/scriptingqtdbproxy.cpp" line="48"/>
      <source>No database available in current context, while called JavaScript&apos;s %1 command.</source>
      <translation>Brak dostępnej bazy danych w bieżącym kontekście, podczas wywoływania komendy JavaScript: %1.</translation>
    </message>
    <message>
      <location filename="../plugins/scriptingqtdbproxy.cpp" line="65"/>
      <source>Error from %1: %2</source>
      <translation>Błąd z %1: %2</translation>
    </message>
  </context>
  <context>
    <name>SqlFileExecutor</name>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="57"/>
      <source>Could not execute SQL, because application has failed to start transaction: %1</source>
      <translation>Nie można wykonać SQLa, ponieważ aplikacja nie mogła rozpocząć transakcji: %1</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="88"/>
      <source>Execution from file cancelled. Any queries executed so far have been rolled back.</source>
      <translation>Wykonywanie z pliku przerwane. Jakiekolwiek wykonane zapytania zostały wycofane.</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="104"/>
      <source>Could not open file &apos;%1&apos; for reading: %2</source>
      <translation>Nie można otworzyż pliku &apos;%1&apos; do odczytu: %2</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="151"/>
      <source>Could not execute SQL, because application has failed to commit the transaction: %1</source>
      <translation>Nie można wykonać SQLa, ponieważ aplikacja nie mogła zatwierdzić transakcji: %1</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="156"/>
      <source>Finished executing %1 queries in %2 seconds. %3 were not executed due to errors.</source>
      <translation>Zakończono wykonywanie %1 zapytań w %2 sekund(y). %3 nie został wykonany z powodu błędów.</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="162"/>
      <source>Finished executing %1 queries in %2 seconds.</source>
      <translation>Zakończono wykonywanie %1 zapytań w %2 sekund(y).</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="169"/>
      <source>Could not execute SQL due to error.</source>
      <translation>Nie można wykonać SQL z powodu błędu.</translation>
    </message>
  </context>
  <context>
    <name>SqlHistoryModel</name>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="32"/>
      <source>Database</source>
      <comment>sql history header</comment>
      <translation>Baza danych</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="34"/>
      <source>Execution date</source>
      <comment>sql history header</comment>
      <translation>Data wykonania</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="36"/>
      <source>Time spent</source>
      <comment>sql history header</comment>
      <translation>Czas trwania</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="38"/>
      <source>Rows affected</source>
      <comment>sql history header</comment>
      <translation>Liczba wierszy</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="40"/>
      <source>SQL</source>
      <comment>sql history header</comment>
      <translation>SQL</translation>
    </message>
  </context>
  <context>
    <name>T</name>
    <message>
      <location filename="../db/abstractdb3.h" line="863"/>
      <source>Registered default collation on demand, under name: %1</source>
      <translation>Zarejestrowano domyślną sekwencję porządkowania na żądanie, pod nazwą: %1</translation>
    </message>
  </context>
  <context>
    <name>UpdateManager</name>
    <message>
      <location filename="../services/updatemanager.cpp" line="92"/>
      <source>Could not check for updates (%1).</source>
      <translation>Nie można sprawdzić aktualizacji (%1).</translation>
    </message>
  </context>
</TS>
