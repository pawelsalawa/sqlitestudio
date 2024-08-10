<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ru" sourcelanguage="en">
  <context>
    <name>ConfigMigration</name>
    <message>
      <location filename="../configmigration.cpp" line="36"/>
      <source>A configuration from old SQLiteStudio 2.x.x has been detected. Would you like to migrate old settings into the current version? &lt;a href=&quot;%1&quot;&gt;Click here to do that&lt;/a&gt;.</source>
      <translation>Обнаружена конфигурация от старой версии SQLiteStudio (2.x.x). Вы хотите перенести старые настройки в новую версию? &lt;a href=&quot;%1&quot;&gt;Нажмите здесь для переноса&lt;/a&gt;.</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="139"/>
      <source>Bug reports history (%1)</source>
      <translation>История отчётов об ошибках (%1)</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="148"/>
      <source>Database list (%1)</source>
      <translation>Список баз данных (%1)</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="157"/>
      <source>Custom SQL functions (%1)</source>
      <translation>Произвольные функции SQL (%1)</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="166"/>
      <source>SQL queries history (%1)</source>
      <translation>История запросов SQL (%1)</translation>
    </message>
  </context>
  <context>
    <name>ConfigMigrationWizard</name>
    <message>
      <location filename="../configmigrationwizard.ui" line="14"/>
      <source>Configuration migration</source>
      <translation>Перенос конфигурации</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="24"/>
      <source>Items to migrate</source>
      <translation>Переносимые элементы</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="27"/>
      <source>This is a list of items found in the old configuration file, which can be migrated into the current configuration.</source>
      <translation>Это список элементов, обнаруженных в старом конфигурационном файле, которые могут быть перенесены в текущую конфигурацию.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="58"/>
      <source>Options</source>
      <translation>Опции</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="64"/>
      <source>Put imported databases into separate group</source>
      <translation>Поместить импортированные базы данных в отдельную группу</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="76"/>
      <source>Group name</source>
      <translation>Имя группы</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="62"/>
      <source>Enter a non-empty name.</source>
      <translation>Введите непустое имя.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="70"/>
      <source>Top level group named &apos;%1&apos; already exists. Enter a group name that does not exist yet.</source>
      <translation>Группа верхнего уровня &apos;%1&apos; уже существует. Введите имя группы, которое ещё не занято.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="104"/>
      <source>Could not open old configuration file in order to migrate settings from it.</source>
      <translation>Невозможно открыть старый файл конфигурации для осуществления переноса настроек.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="112"/>
      <source>Could not open current configuration file in order to migrate settings from old configuration file.</source>
      <translation>Невозможно открыть текущий файл конфигурации для переноса настроек из старого файла конфигурации.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="121"/>
      <source>Could not commit migrated data into new configuration file: %1</source>
      <translation>Невозможно записать перенесённые данные в новый файл конфигурации: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="165"/>
      <source>Could not read bug reports history from old configuration file in order to migrate it: %1</source>
      <translation>Невозможно прочитать историю отчётов об ошибках из старого файла конфигурации для переноса: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="182"/>
      <source>Could not insert a bug reports history entry into new configuration file: %1</source>
      <translation>Невозможно вставить историю отчётов об ошибках в новый файл конфигурации: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="203"/>
      <source>Could not read database list from old configuration file in order to migrate it: %1</source>
      <translation>Невозможно прочитать список баз данных из старого файла конфигурации для переноса: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="217"/>
      <source>Could not query for available order for containing group in new configuration file in order to migrate the database list: %1</source>
      <translation>Невозможно запросить доступное положение отдельной группы в новом файле конфигурации для переноса в неё списка баз данных: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="228"/>
      <source>Could not create containing group in new configuration file in order to migrate the database list: %1</source>
      <translation>Невозможно создать отдельную группу в новом файле конфигурации для переноса в неё списка баз данных: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="249"/>
      <source>Could not insert a database entry into new configuration file: %1</source>
      <translation>Невозможно вставить элемент списка баз данных в новый файл конфигурации: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="261"/>
      <source>Could not query for available order for next database in new configuration file in order to migrate the database list: %1</source>
      <translation>Невозможно запросить доступное положение для следующей базы данных в новом файле конфигурации для переноса списка баз данных: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="272"/>
      <source>Could not create group referencing the database in new configuration file: %1</source>
      <translation>Невозможно создать группу, ссылающуюся на базу данных в новом файле конфигурации: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="290"/>
      <source>Could not read function list from old configuration file in order to migrate it: %1</source>
      <translation>Невозможно прочитать список функций из старого файла конфигурации для переноса: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="325"/>
      <source>Could not read SQL queries history from old configuration file in order to migrate it: %1</source>
      <translation>Невозможно прочитать историю запросов SQL из старого файла конфигурации для переноса: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="332"/>
      <source>Could not read next ID for SQL queries history in new configuration file: %1</source>
      <translation>Невозможно считать следующий ID для истории запросов SQL в новом файле конфигурации: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="348"/>
      <source>Could not insert SQL history entry into new configuration file: %1</source>
      <translation>Невозможно вставить элемент истории запросов SQL в новый файл конфигурации: %1</translation>
    </message>
  </context>
</TS>
