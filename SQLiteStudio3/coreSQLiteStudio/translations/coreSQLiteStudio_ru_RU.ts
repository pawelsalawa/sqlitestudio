<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ru" sourcelanguage="en">
  <context>
    <name>AbstractDb</name>
    <message>
      <location filename="../db/abstractdb.cpp" line="352"/>
      <location filename="../db/abstractdb.cpp" line="369"/>
      <source>Cannot execute query on closed database.</source>
      <translation>Невозможно выполнить запрос при закрытой базе данных.</translation>
    </message>
    <message>
      <location filename="../db/abstractdb.cpp" line="709"/>
      <source>Error attaching database %1: %2</source>
      <translation>Ошибка во время присоединения базы данных %1: %2</translation>
    </message>
    <message>
      <location filename="../db/abstractdb.cpp" line="958"/>
      <source>Failed to make full WAL checkpoint on database &apos;%1&apos;. Error returned from SQLite engine: %2</source>
      <translation>Не удалось создать полную контрольную точку WAL в базе данных &apos;%1&apos;. Ошибка из движка SQLite: %2</translation>
    </message>
  </context>
  <context>
    <name>ChainExecutor</name>
    <message>
      <location filename="../db/chainexecutor.cpp" line="37"/>
      <source>The database for executing queries was not defined.</source>
      <comment>chain executor</comment>
      <translation>Не указана база данных для выполнения запросов.</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="44"/>
      <source>The database for executing queries was not open.</source>
      <comment>chain executor</comment>
      <translation>Не открыта база данных для выполнения запросов.</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="54"/>
      <source>Could not disable foreign keys in the database. Details: %1</source>
      <comment>chain executor</comment>
      <translation>Невозможно отключить внешние ключи в базе данных. Подробности: %1</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="62"/>
      <source>Could not start a database transaction. Details: %1</source>
      <comment>chain executor</comment>
      <translation>Невозможно начать транзакцию. Подробности: %1</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="89"/>
      <source>Interrupted</source>
      <comment>chain executor</comment>
      <translation>Прервано</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="151"/>
      <source>Could not commit a database transaction. Details: %1</source>
      <comment>chain executor</comment>
      <translation>Невозможно завершить транзакцию. Подробности: %1</translation>
    </message>
  </context>
  <context>
    <name>CompletionHelper</name>
    <message>
      <location filename="../completionhelper.cpp" line="159"/>
      <source>New row reference</source>
      <translation>Новая ссылка на строку</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="166"/>
      <source>Old row reference</source>
      <translation>Старая ссылка на строку</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="171"/>
      <source>New table name</source>
      <translation>Новое имя таблицы</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="174"/>
      <source>New index name</source>
      <translation>Новое имя индекса</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="177"/>
      <source>New view name</source>
      <translation>Новое имя представления</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="180"/>
      <source>New trigger name</source>
      <translation>Новое имя триггера</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="183"/>
      <source>Table or column alias</source>
      <translation>Псевдоним таблицы или столбца</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="186"/>
      <source>transaction name</source>
      <translation>имя транзакции</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="189"/>
      <source>New column name</source>
      <translation>Новое имя столбца</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="192"/>
      <source>Column data type</source>
      <translation>Тип данных столбца</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="195"/>
      <source>Constraint name</source>
      <translation>Имя ограничения</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="208"/>
      <source>Error message</source>
      <translation>Сообщение об ошибке</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="257"/>
      <source>Any word</source>
      <translation>Любое слово</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="438"/>
      <source>Default database</source>
      <translation>База данных по умолчанию</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="439"/>
      <source>Temporary objects database</source>
      <translation>База данных временных объектов</translation>
    </message>
  </context>
  <context>
    <name>ConfigImpl</name>
    <message>
      <location filename="../services/impl/configimpl.cpp" line="881"/>
      <source>Could not start database transaction for deleting SQL history, therefore it&apos;s not deleted.</source>
      <translation>Невозможно начать транзакцию для удаления истории SQL, поэтому она не удалена.</translation>
    </message>
    <message>
      <location filename="../services/impl/configimpl.cpp" line="888"/>
      <source>Could not commit database transaction for deleting SQL history, therefore it&apos;s not deleted.</source>
      <translation>Невозможно завершить транзакцию для удаления истории SQL, поэтому она не удалена.</translation>
    </message>
  </context>
  <context>
    <name>DbManagerImpl</name>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="64"/>
      <source>Could not add database %1: %2</source>
      <translation>Не удалось добавить базу данных %1: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="137"/>
      <source>Database %1 could not be updated, because of an error: %2</source>
      <translation>Невозможно обновить базу данных %1 из-за ошибки: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="353"/>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="382"/>
      <source>Database file doesn&apos;t exist.</source>
      <translation>Файл базы данных не существует.</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="355"/>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="384"/>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="603"/>
      <source>No supporting plugin loaded.</source>
      <translation>Модуль поддержки не загружен.</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="521"/>
      <source>Database could not be initialized.</source>
      <translation>Невозможно инициализировать базу данных.</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="531"/>
      <source>No suitable database driver plugin found.</source>
      <translation>Не найден подходящий драйвер базы данных.</translation>
    </message>
  </context>
  <context>
    <name>DbObjectOrganizer</name>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="372"/>
      <location filename="../dbobjectorganizer.cpp" line="403"/>
      <source>Error while creating table in target database: %1</source>
      <translation>Ошибка при создании таблицы в целевой базе данных: %1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="372"/>
      <source>Could not parse table.</source>
      <translation>Невозможно проанализировать структуру таблицы.</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="417"/>
      <source>Database %1 could not be attached to database %2, so the data of table %3 will be copied with SQLiteStudio as a mediator. This method can be slow for huge tables, so please be patient.</source>
      <translation>Невозможно присоединить базу данных %1 к базе данных %2, поэтому данные таблицы %3 будут скопированы при посредничестве SQLiteStudio. Этот метод может быть медленным для больших таблиц, так что наберитесь терпения.</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="441"/>
      <source>Error while copying data for table %1: %2</source>
      <translation>Ошибка при копировании данных из таблицы %1: %2</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="460"/>
      <location filename="../dbobjectorganizer.cpp" line="467"/>
      <location filename="../dbobjectorganizer.cpp" line="494"/>
      <source>Error while copying data to table %1: %2</source>
      <translation>Ошибка при копировании данных в таблицу %1: %2</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="516"/>
      <source>Error while dropping source view %1: %2
Tables, indexes, triggers and views copied to database %3 will remain.</source>
      <translation>Ошибка при удалении исходного представления %1: %2
Таблицы, индексы, триггеры и представления, скопированные в базу данных %3, сохранятся.</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="523"/>
      <source>Error while creating view in target database: %1</source>
      <translation>Ошибка при создании представления в целевой базе данных: %1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="528"/>
      <source>Error while creating index in target database: %1</source>
      <translation>Ошибка при создании индекса в целевой базе данных: %1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="533"/>
      <source>Error while creating trigger in target database: %1</source>
      <translation>Ошибка при создании триггера в целевой базе данных: %1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="664"/>
      <location filename="../dbobjectorganizer.cpp" line="671"/>
      <location filename="../dbobjectorganizer.cpp" line="680"/>
      <source>Could not parse object &apos;%1&apos; in order to move or copy it.</source>
      <translation>Невозможно проанализировать объект &apos;%1&apos; для его перемещения либо копирования.</translation>
    </message>
  </context>
  <context>
    <name>DdlHistoryModel</name>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="66"/>
      <source>Database name</source>
      <comment>ddl history header</comment>
      <translation>Имя базы данных</translation>
    </message>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="68"/>
      <source>Database file</source>
      <comment>ddl history header</comment>
      <translation>Файл базы данных</translation>
    </message>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="70"/>
      <source>Date of execution</source>
      <comment>ddl history header</comment>
      <translation>Дата выполнения</translation>
    </message>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="72"/>
      <source>Changes</source>
      <comment>ddl history header</comment>
      <translation>Изменения</translation>
    </message>
  </context>
  <context>
    <name>ExportManager</name>
    <message>
      <location filename="../services/exportmanager.cpp" line="71"/>
      <source>Export plugin %1 doesn&apos;t support exporing query results.</source>
      <translation>Модуль экспорта %1 не поддерживает экспорт результатов запроса.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="97"/>
      <source>Export plugin %1 doesn&apos;t support exporing tables.</source>
      <translation>Модуль экспорта %1 не поддерживает экспорт таблиц.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="121"/>
      <source>Export plugin %1 doesn&apos;t support exporing databases.</source>
      <translation>Модуль экспорта %1 не поддерживает экспорт баз данных.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="154"/>
      <source>Export format &apos;%1&apos; is not supported. Supported formats are: %2.</source>
      <translation>Формат экспорта %1 не поддерживается. Поддерживаемые форматы: %2.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="218"/>
      <source>Export to the clipboard was successful.</source>
      <translation>Экспорт в буфер обмена успешно выполнен.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="222"/>
      <source>Export to the file &apos;%1&apos; was successful.</source>
      <translation>Экспорт в файл %1 успешно выполнен.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="224"/>
      <source>Export was successful.</source>
      <translation>Экспорт успешно выполнен.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="266"/>
      <source>Could not export to file %1. File cannot be open for writting.</source>
      <translation>Невозможно выполнить экспорт в файл %1. Не удалось открыть файл для записи.</translation>
    </message>
  </context>
  <context>
    <name>ExportWorker</name>
    <message>
      <location filename="../exportworker.cpp" line="123"/>
      <source>Error while exporting query results: %1</source>
      <translation>Ошибка при экспорте результатов запроса: %1</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="209"/>
      <source>Error while counting data column width to export from query results: %1</source>
      <translation>Ошибка при подсчёте ширины столбца данных для экспорта результатов запроса: %1</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="353"/>
      <location filename="../exportworker.cpp" line="411"/>
      <source>Could not parse %1 in order to export it. It will be excluded from the export output.</source>
      <translation>Невозможно проанализировать структуру %1. Данный объект будет исключён при выполнении экспорта.</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="615"/>
      <source>Error while reading data to export from table %1: %2</source>
      <translation>Ошибка при считывании данных для экспорта из таблицы %1: %2</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="623"/>
      <source>Error while counting data to export from table %1: %2</source>
      <translation>Ошибка при подсчёте количества данных для экспорта из таблицы %1: %2</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="639"/>
      <source>Error while counting data column width to export from table %1: %2</source>
      <translation>Ошибка при подсчёте ширины столбца данных для экспорта из таблицы %1: %2</translation>
    </message>
  </context>
  <context>
    <name>FunctionManagerImpl</name>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="199"/>
      <source>Could not create scripting context, probably the plugin is not configured properly</source>
      <translation type="unfinished">Could not create scripting context, probably the plugin is not configured properly</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="292"/>
      <source>Invalid number of arguments to function &apos;%1&apos;. Expected %2, but got %3.</source>
      <translation>Неверное количество аргументов для функции &apos;%1&apos;. Ожидаемое количество: %2, передано: %3.</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="406"/>
      <source>No such function registered in SQLiteStudio: %1(%2)</source>
      <translation>Функция не зарегистрирована в SQLiteStudio: %1(%2)</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="412"/>
      <source>Function %1(%2) was registered with language %3, but the plugin supporting that language is not currently loaded.</source>
      <translation>Функция %1(%2) зарегистрирована для языка %3, однако модуль поддержки этого языка на данный момент не загружен.</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="430"/>
      <source>Invalid regular expression pattern: %1</source>
      <translation>Неверный шаблон регулярного выражения: %1</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="449"/>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="482"/>
      <source>Could not open file %1 for reading: %2</source>
      <translation>Невозможно открыть файл %1 для чтения: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="504"/>
      <source>Could not open file %1 for writting: %2</source>
      <translation>Невозможно открыть файл %1 для записи: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="524"/>
      <source>Error while writting to file %1: %2</source>
      <translation>Ошибка при записи в файл %1: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="542"/>
      <source>Unsupported scripting language: %1</source>
      <translation>Неподдерживаемый скриптовый язык: %1</translation>
    </message>
  </context>
  <context>
    <name>GenericExportPlugin</name>
    <message>
      <location filename="../plugins/genericexportplugin.cpp" line="20"/>
      <source>Could not initialize text codec for exporting. Using default codec: %1</source>
      <translation>Невозможно инициализировать текстовый кодек для экспорта. Используется кодек по умолчанию: %1</translation>
    </message>
  </context>
  <context>
    <name>ImportManager</name>
    <message>
      <location filename="../services/importmanager.cpp" line="99"/>
      <source>Imported data to the table &apos;%1&apos; successfully. Number of imported rows: %2</source>
      <translation>Импорт данных в таблицу &apos;%1&apos; выполнен успешно. Количество импортированных строк: %2</translation>
    </message>
  </context>
  <context>
    <name>ImportWorker</name>
    <message>
      <location filename="../importworker.cpp" line="24"/>
      <source>No columns provided by the import plugin.</source>
      <translation>Модуль импорта не обнаружил ни одного столбца.</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="31"/>
      <source>Could not start transaction in order to import a data: %1</source>
      <translation>Невозможно начать транзакцию для импорта данных: %1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="54"/>
      <source>Could not commit transaction for imported data: %1</source>
      <translation>Невозможно завершить транзакцию для импортированных данных: %1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="101"/>
      <source>Table &apos;%1&apos; has less columns than there are columns in the data to be imported. Excessive data columns will be ignored.</source>
      <translation>В таблице &apos;%1&apos; столбцов меньше, чем в импортируемых данных. Лишние столбцы будут проигнорированы.</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="106"/>
      <source>Table &apos;%1&apos; has more columns than there are columns in the data to be imported. Some columns in the table will be left empty.</source>
      <translation>В таблице &apos;%1&apos; столбцов больше, чем в импортируемых данных. Недостающие столбцы будут оставлены пустыми.</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="125"/>
      <source>Could not create table to import to: %1</source>
      <translation>Невозможно создать таблицу для импорта: %1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="134"/>
      <location filename="../importworker.cpp" line="185"/>
      <location filename="../importworker.cpp" line="192"/>
      <source>Error while importing data: %1</source>
      <translation>Ошибка при импорте данных: %1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="134"/>
      <location filename="../importworker.cpp" line="192"/>
      <source>Interrupted.</source>
      <comment>import process status update</comment>
      <translation>Прервано.</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="180"/>
      <source>Could not import data row number %1. The row was ignored. Problem details: %2</source>
      <translation>Невозможно импортировать строку данных № %1. Строка пропущена. Подробности проблемы: %2</translation>
    </message>
  </context>
  <context>
    <name>PluginManagerImpl</name>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="546"/>
      <source>Cannot load plugin %1, because it&apos;s in conflict with plugin %2.</source>
      <translation>Невозможно загрузить модуль %1, так как он конфликтует с модулем %2.</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="557"/>
      <source>Cannot load plugin %1, because its dependency was not loaded: %2.</source>
      <translation>Невозможно загрузить модуль %1, так как не загружен необходимый ему модуль: %2.</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="566"/>
      <source>Cannot load plugin %1. Error details: %2</source>
      <translation>Невозможно загрузить модуль %1. Подробности ошибки: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="582"/>
      <source>Cannot load plugin %1 (error while initializing plugin).</source>
      <translation>Невозможно загрузить модуль %1 (ошибка при инициализации модуля).</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="743"/>
      <source>min: %1</source>
      <comment>plugin dependency version</comment>
      <translation>минимальная: %1</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="744"/>
      <source>max: %1</source>
      <comment>plugin dependency version</comment>
      <translation>максимальная: %1</translation>
    </message>
  </context>
  <context>
    <name>PopulateConstant</name>
    <message>
      <location filename="../plugins/populateconstant.cpp" line="10"/>
      <source>Constant</source>
      <comment>populate constant plugin name</comment>
      <translation>Константа</translation>
    </message>
  </context>
  <context>
    <name>PopulateConstantConfig</name>
    <message>
      <location filename="../plugins/populateconstant.ui" line="20"/>
      <source>Constant value:</source>
      <translation>Значение константы:</translation>
    </message>
  </context>
  <context>
    <name>PopulateDictionary</name>
    <message>
      <location filename="../plugins/populatedictionary.cpp" line="17"/>
      <source>Dictionary</source>
      <comment>dictionary populating plugin name</comment>
      <translation>Словарь</translation>
    </message>
  </context>
  <context>
    <name>PopulateDictionaryConfig</name>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="20"/>
      <source>Dictionary file</source>
      <translation>Файл словаря</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="29"/>
      <source>Pick dictionary file</source>
      <translation>Выберите файл словаря</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="39"/>
      <source>Word separator</source>
      <translation>Разделитель слов</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="45"/>
      <source>Whitespace</source>
      <translation>Пробел</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="58"/>
      <source>Line break</source>
      <translation>Перенос строки</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="74"/>
      <source>Method of using words</source>
      <translation>Способ использования слов</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="80"/>
      <source>Ordered</source>
      <translation>По порядку</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="93"/>
      <source>Randomly</source>
      <translation>Случайным образом</translation>
    </message>
  </context>
  <context>
    <name>PopulateManager</name>
    <message>
      <location filename="../services/populatemanager.cpp" line="89"/>
      <source>Table &apos;%1&apos; populated successfully.</source>
      <translation>Таблица &apos;%1&apos; успешно заполнена.</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandom</name>
    <message>
      <location filename="../plugins/populaterandom.cpp" line="13"/>
      <source>Random number</source>
      <translation>Случайное число</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandomConfig</name>
    <message>
      <location filename="../plugins/populaterandom.ui" line="20"/>
      <source>Constant prefix</source>
      <translation>Префикс константы</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="26"/>
      <source>No prefix</source>
      <translation>Без префикса</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="39"/>
      <source>Minimum value</source>
      <translation>Минимальное значение</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="61"/>
      <source>Maximum value</source>
      <translation>Максимальное значение</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="86"/>
      <source>Constant suffix</source>
      <translation>Суффикс константы</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="92"/>
      <source>No suffix</source>
      <translation>Без суффикса</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandomText</name>
    <message>
      <location filename="../plugins/populaterandomtext.cpp" line="14"/>
      <source>Random text</source>
      <translation>Случайный текст</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandomTextConfig</name>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="20"/>
      <source>Use characters from common sets:</source>
      <translation>Использовать символы из стандартных наборов:</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="36"/>
      <source>Minimum length</source>
      <translation>Минимальная длина</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="64"/>
      <source>Letters from a to z.</source>
      <translation>Буквы от a до z.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="67"/>
      <source>Alpha</source>
      <translation>Буквенный</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="77"/>
      <source>Numbers from 0 to 9.</source>
      <translation>Цифры от 0 до 9.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="80"/>
      <source>Numeric</source>
      <translation>Цифровой</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="90"/>
      <source>A whitespace, a tab and a new line character.</source>
      <translation>Пробел, табуляция и символ переноса строки.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="93"/>
      <source>Whitespace</source>
      <translation>Непечатаемые символы</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="103"/>
      <source>Includes all above and all others.</source>
      <translation>Включает все вышеперечисленные и все остальные.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="106"/>
      <source>Binary</source>
      <translation>Бинарный</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="119"/>
      <source>Use characters from my custom set:</source>
      <translation>Использовать символы из моего набора:</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="132"/>
      <source>Maximum length</source>
      <translation>Максимальная длина</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="160"/>
      <source>If you type some character multiple times, it&apos;s more likely to be used.</source>
      <translation>При указании одного символа несколько раз, вероятность его использования увеличивается.</translation>
    </message>
  </context>
  <context>
    <name>PopulateScript</name>
    <message>
      <location filename="../plugins/populatescript.cpp" line="34"/>
      <source>Script</source>
      <translation>Скрипт</translation>
    </message>
  </context>
  <context>
    <name>PopulateScriptConfig</name>
    <message>
      <location filename="../plugins/populatescript.ui" line="26"/>
      <source>Initialization code (optional)</source>
      <translation>Инициализирующий код (необязательно)</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.ui" line="45"/>
      <source>Per step code</source>
      <translation>Код для каждого шага</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.ui" line="70"/>
      <source>Language</source>
      <translation>Язык</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.ui" line="89"/>
      <source>Help</source>
      <translation>Помощь</translation>
    </message>
  </context>
  <context>
    <name>PopulateSequence</name>
    <message>
      <location filename="../plugins/populatesequence.cpp" line="13"/>
      <source>Sequence</source>
      <translation>Последовательность</translation>
    </message>
  </context>
  <context>
    <name>PopulateSequenceConfig</name>
    <message>
      <location filename="../plugins/populatesequence.ui" line="33"/>
      <source>Start value:</source>
      <translation>Начальное значение:</translation>
    </message>
    <message>
      <location filename="../plugins/populatesequence.ui" line="56"/>
      <source>Step:</source>
      <translation>Шаг:</translation>
    </message>
  </context>
  <context>
    <name>PopulateWorker</name>
    <message>
      <location filename="../populateworker.cpp" line="23"/>
      <source>Could not start transaction in order to perform table populating. Error details: %1</source>
      <translation>Невозможно начать транзакцию для заполнения таблицы. Подробности ошибки: %1</translation>
    </message>
    <message>
      <location filename="../populateworker.cpp" line="69"/>
      <source>Error while populating table: %1</source>
      <translation>Ошибка при заполнении таблицы: %1</translation>
    </message>
    <message>
      <location filename="../populateworker.cpp" line="80"/>
      <source>Could not commit transaction after table populating. Error details: %1</source>
      <translation>Невозможно завершить транзакцию после заполнения таблицы. Подробности ошибки: %1</translation>
    </message>
  </context>
  <context>
    <name>QObject</name>
    <message>
      <location filename="../common/utils.cpp" line="940"/>
      <source>Could not open file &apos;%1&apos; for reading: %2</source>
      <translation>Невозможно открыть файл &apos;%1&apos; для чтения: %2</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="437"/>
      <source>Could not open database: %1</source>
      <translation>Невозможно открыть базу данных: %1</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="1235"/>
      <source>Result set expired or no row available.</source>
      <translation>Результирующая выборка устарела или ни одна строка не доступна.</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="333"/>
      <location filename="../db/abstractdb3.h" line="337"/>
      <source>Could not load extension %1: %2</source>
      <translation>Невозможно загрузить расширение %1: %2</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="423"/>
      <source>Could not run WAL checkpoint: %1</source>
      <translation>Не удалось запустить контрольную точку WAL: %1</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="461"/>
      <source>Could not close database: %1</source>
      <translation>Невозможно закрыть базу данных: %1</translation>
    </message>
    <message>
      <location filename="../impl/dbattacherimpl.cpp" line="114"/>
      <source>Could not attach database %1: %2</source>
      <translation>Не удалось присоединить базу данных %1: %2</translation>
    </message>
    <message>
      <location filename="../parser/parsercontext.cpp" line="108"/>
      <location filename="../parser/parsercontext.cpp" line="110"/>
      <source>Incomplete query.</source>
      <translation>Незавершённый запрос.</translation>
    </message>
    <message>
      <location filename="../parser/sqlite3_parse.cpp" line="2526"/>
      <source>Parser stack overflow</source>
      <translation>Переполнение стека анализатора</translation>
    </message>
    <message>
      <location filename="../parser/sqlite3_parse.cpp" line="5937"/>
      <source>Syntax error</source>
      <translation>Синтаксическая ошибка</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.cpp" line="32"/>
      <source>Could not open dictionary file %1 for reading.</source>
      <translation>Невозможно открыть файл словаря %1 для чтения.</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.cpp" line="93"/>
      <source>Dictionary file must exist and be readable.</source>
      <translation>Файл словаря должен существовать и быть доступным для чтения.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.cpp" line="54"/>
      <source>Maximum value cannot be less than minimum value.</source>
      <translation>Максимальное значение не может быть меньше минимального.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.cpp" line="79"/>
      <source>Maximum length cannot be less than minimum length.</source>
      <translation>Максимальная длина не может быть меньше минимальной.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.cpp" line="90"/>
      <source>Custom character set cannot be empty.</source>
      <translation>Произвольный набор символов не может быть пустым.</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="61"/>
      <source>Could not find plugin to support scripting language: %1</source>
      <translation>Невозможно найти модуль поддержки скриптового языка: %1</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="70"/>
      <source>Could not get evaluation context, probably the %1 scripting plugin is not configured properly</source>
      <translation type="unfinished">Could not get evaluation context, probably the %1 scripting plugin is not configured properly</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="84"/>
      <source>Error while executing populating initial code: %1</source>
      <translation>Ошибка при выполнении инициализирующего кода заполнения: %1</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="106"/>
      <source>Error while executing populating code: %1</source>
      <translation>Ошибка при выполнении кода заполнения: %1</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="138"/>
      <source>Select implementation language.</source>
      <translation>Выберите язык реализации.</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="139"/>
      <source>Implementation code cannot be empty.</source>
      <translation>Заполняющий код не может быть пустым.</translation>
    </message>
    <message>
      <location filename="../selectresolver.cpp" line="372"/>
      <source>Could not resolve data source for column: %1</source>
      <translation>Невозможно определить источник данных для столбца: %1</translation>
    </message>
    <message>
      <location filename="../selectresolver.cpp" line="444"/>
      <source>Could not resolve table for column &apos;%1&apos;.</source>
      <translation>Невозможно определить таблицу для столбца &apos;%1&apos;.</translation>
    </message>
    <message>
      <location filename="../services/impl/configimpl.cpp" line="761"/>
      <source>Could not initialize configuration file. Any configuration changes and queries history will be lost after application restart. Unable to create a file at following locations: %1.</source>
      <translation>Не удалось инициализировать конфигурационный файл. Любые изменения конфигурации и история запросов будут потеряны после перезагрузки приложения. Невозможно создать файл в следующих местах: %1.</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="347"/>
      <source>General purpose</source>
      <comment>plugin category name</comment>
      <translation>Общего назначения</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="348"/>
      <source>Database support</source>
      <comment>plugin category name</comment>
      <translation>Поддержка баз данных</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="349"/>
      <source>Code formatter</source>
      <comment>plugin category name</comment>
      <translation>Форматирование кода</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="350"/>
      <source>Scripting languages</source>
      <comment>plugin category name</comment>
      <translation>Скриптовые языки</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="352"/>
      <source>Exporting</source>
      <comment>plugin category name</comment>
      <translation>Экспорт</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="353"/>
      <source>Importing</source>
      <comment>plugin category name</comment>
      <translation>Импорт</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="354"/>
      <source>Table populating</source>
      <comment>plugin category name</comment>
      <translation>Заполнение таблиц</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="161"/>
      <source>Table %1 is referencing table %2, but the foreign key definition will not be updated for new table definition due to problems while parsing DDL of the table %3.</source>
      <translation>Таблица %1 ссылается на таблицу %2, но описание внешнего ключа не будет обновлено для описания новой таблицы из-за проблем с анализом DDL таблицы %3.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="510"/>
      <source>All columns indexed by the index %1 are gone. The index will not be recreated after table modification.</source>
      <translation>Все столбцы, проиндексированные индексом %1, удалены. Индекс не будет воссоздан после модификации таблицы.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="554"/>
      <source>There is problem with proper processing trigger %1. It may be not fully updated afterwards and will need your attention.</source>
      <translation>Возникла проблема при обработке триггера %1. Впоследствии он не будет полностью обновлён и потребует вашего внимания.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="569"/>
      <source>All columns covered by the trigger %1 are gone. The trigger will not be recreated after table modification.</source>
      <translation>Все столбцы, затронутые в триггере %1, удалены. Триггер не будет воссоздан после модификации таблицы.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="601"/>
      <source>Cannot not update trigger %1 according to table %2 modification.</source>
      <translation>Невозможно обновить триггер %1 в соответствии с модификацией таблицы %2.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="620"/>
      <source>Cannot not update view %1 according to table %2 modifications.
The view will remain as it is.</source>
      <translation>Невозможно обновить представление %1 в соответствии с модификациями таблицы %2.
Представление останется как есть.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="782"/>
      <location filename="../tablemodifier.cpp" line="806"/>
      <location filename="../tablemodifier.cpp" line="825"/>
      <source>There is a problem with updating an %1 statement within %2 trigger. One of the %1 substatements which might be referring to table %3 cannot be properly modified. Manual update of the trigger may be necessary.</source>
      <translation>Возникла проблема при обновлении конструкции %1 внутри триггера %2. Одна из вложенных конструкций %1, которая возможно ссылается на таблицу %3, не может быть корректно модифицирована. Возможно необходима ручная правка триггера.</translation>
    </message>
    <message>
      <location filename="../viewmodifier.cpp" line="24"/>
      <source>Could not parse DDL of the view to be created. Details: %1</source>
      <translation>Невозможно проанализировать DDL создаваемого представления. Подробности: %1</translation>
    </message>
    <message>
      <location filename="../viewmodifier.cpp" line="33"/>
      <source>Parsed query is not CREATE VIEW. It&apos;s: %1</source>
      <translation>Проанализированный запрос не является запросом CREATE VIEW. Тип запроса: %1</translation>
    </message>
    <message>
      <location filename="../viewmodifier.cpp" line="81"/>
      <source>SQLiteStudio was unable to resolve columns returned by the new view, therefore it won&apos;t be able to tell which triggers might fail during the recreation process.</source>
      <translation>SQLiteStudio не удалось определить столбцы, возвращаемые новым представлением, поэтому невозможно указать, какие триггеры могут сломаться в процессе воссоздания.</translation>
    </message>
  </context>
  <context>
    <name>QueryExecutor</name>
    <message>
      <location filename="../db/queryexecutor.cpp" line="204"/>
      <source>Execution interrupted.</source>
      <translation>Выполнение прервано.</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="245"/>
      <source>Database is not open.</source>
      <translation>База данных не открыта.</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="253"/>
      <source>Only one query can be executed simultaneously.</source>
      <translation>Одновременно может быть выполнен только один запрос.</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="343"/>
      <location filename="../db/queryexecutor.cpp" line="357"/>
      <location filename="../db/queryexecutor.cpp" line="607"/>
      <source>An error occured while executing the count(*) query, thus data paging will be disabled. Error details from the database: %1</source>
      <translation>Возникла ошибка при выполнении запроса count(*), поэтому разбивка данных по страницам отключена. Детали ошибки из базы данных: %1</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="526"/>
      <source>SQLiteStudio was unable to extract metadata from the query. Results won&apos;t be editable.</source>
      <translation>SQLiteStudio не удалось извлечь метаданные из запроса. Результаты нельзя будет редактировать.</translation>
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
      <translation>При вызове команды JavaScript %1, в текущем контексте нет доступных баз данных.</translation>
    </message>
    <message>
      <location filename="../plugins/scriptingqtdbproxy.cpp" line="65"/>
      <source>Error from %1: %2</source>
      <translation>Ошибка в команде %1: %2</translation>
    </message>
  </context>
  <context>
    <name>SqlFileExecutor</name>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="56"/>
      <source>Could not execute SQL, because application has failed to start transaction: %1</source>
      <translation>Невозможно выполнить SQL-запрос, так как приложению не удалось начать транзакцию: %1</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="87"/>
      <source>Execution from file cancelled. Any queries executed so far have been rolled back.</source>
      <translation>Выполнение запросов из файла отменено. Все выполненные ранее из него запросы откачены.</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="103"/>
      <source>Could not open file &apos;%1&apos; for reading: %2</source>
      <translation>Невозможно открыть файл &apos;%1&apos; для чтения: %2</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="150"/>
      <source>Could not execute SQL, because application has failed to commit the transaction: %1</source>
      <translation>Невозможно выполнить SQL-запрос, так как приложению не удалось завершить транзакцию: %1</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="155"/>
      <source>Finished executing %1 queries in %2 seconds. %3 were not executed due to errors.</source>
      <translation>Завершено выполнение %1 запросов за %2 секунд. %3 запросов не было выполнено из-за ошибок.</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="161"/>
      <source>Finished executing %1 queries in %2 seconds.</source>
      <translation>Завершено выполнение %1 запросов за %2 секунд.</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="168"/>
      <source>Could not execute SQL due to error.</source>
      <translation>Невозможно выполнить SQL-запрос из-за ошибки.</translation>
    </message>
  </context>
  <context>
    <name>SqlHistoryModel</name>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="32"/>
      <source>Database</source>
      <comment>sql history header</comment>
      <translation>База данных</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="34"/>
      <source>Execution date</source>
      <comment>sql history header</comment>
      <translation>Дата выполнения</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="36"/>
      <source>Time spent</source>
      <comment>sql history header</comment>
      <translation>Затраченное время</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="38"/>
      <source>Rows affected</source>
      <comment>sql history header</comment>
      <translation>Затронуто строк</translation>
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
      <location filename="../db/abstractdb3.h" line="864"/>
      <source>Registered default collation on demand, under name: %1</source>
      <translation type="unfinished">Registered default collation on demand, under name: %1</translation>
    </message>
  </context>
  <context>
    <name>UpdateManager</name>
    <message>
      <location filename="../services/updatemanager.cpp" line="92"/>
      <source>Could not check for updates (%1).</source>
      <translation>Невозможно проверить наличие обновлений (%1).</translation>
    </message>
  </context>
</TS>
