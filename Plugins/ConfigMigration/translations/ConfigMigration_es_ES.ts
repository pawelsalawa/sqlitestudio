<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="es-ES" sourcelanguage="en">
  <context>
    <name>ConfigMigration</name>
    <message>
      <location filename="../configmigration.cpp" line="36"/>
      <source>A configuration from old SQLiteStudio 2.x.x has been detected. Would you like to migrate old settings into the current version? &lt;a href=&quot;%1&quot;&gt;Click here to do that&lt;/a&gt;.</source>
      <translation>Se ha detectado una configuración de la antigua versión de SQLiteStudio 2.x.x. ¿Quieres migrar la configuración antigua a la versión actual? &lt;a href=&quot;%1&quot;&gt;Haz clic aquí para hacer eso&lt;/a&gt;.</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="139"/>
      <source>Bug reports history (%1)</source>
      <translation>Historial de informes de errores (%1)</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="148"/>
      <source>Database list (%1)</source>
      <translation>Lista de base de datos (%1)</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="157"/>
      <source>Custom SQL functions (%1)</source>
      <translation>Funciones SQL personalizadas (%1)</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="166"/>
      <source>SQL queries history (%1)</source>
      <translation>Historial de consultas SQL (%1)</translation>
    </message>
  </context>
  <context>
    <name>ConfigMigrationWizard</name>
    <message>
      <location filename="../configmigrationwizard.ui" line="14"/>
      <source>Configuration migration</source>
      <translation>Migración de la configuración</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="24"/>
      <source>Items to migrate</source>
      <translation>Elementos a migrar</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="27"/>
      <source>This is a list of items found in the old configuration file, which can be migrated into the current configuration.</source>
      <translation>Esta es una lista de elementos encontrados en el antiguo archivo de configuración, que puede migrarse a la configuración actual.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="58"/>
      <source>Options</source>
      <translation>Opciones</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="64"/>
      <source>Put imported databases into separate group</source>
      <translation>Colocar bases de datos importadas en un grupo separado</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="76"/>
      <source>Group name</source>
      <translation>Nombre del Grupo</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="62"/>
      <source>Enter a non-empty name.</source>
      <translation>Introduzca un nombre no vacío.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="70"/>
      <source>Top level group named &apos;%1&apos; already exists. Enter a group name that does not exist yet.</source>
      <translation>Un grupo de nivel superior llamado &apos;%1&apos; ya existe. Introduzca un nombre de grupo que aún no exista.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="104"/>
      <source>Could not open old configuration file in order to migrate settings from it.</source>
      <translation>No se pudo abrir el antiguo archivo de configuración del cual migrar los ajustes.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="112"/>
      <source>Could not open current configuration file in order to migrate settings from old configuration file.</source>
      <translation>No se pudo abrir el actual archivo de configuración al cual migrar los ajustes del antiguo archivo de configuración.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="121"/>
      <source>Could not commit migrated data into new configuration file: %1</source>
      <translation>No se pudieron registrar los datos migrados en el nuevo archivo de configuración: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="165"/>
      <source>Could not read bug reports history from old configuration file in order to migrate it: %1</source>
      <translation>No se pudo leer el historial de informes de errores del antiguo archivo de configuración para migrarlo: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="182"/>
      <source>Could not insert a bug reports history entry into new configuration file: %1</source>
      <translation>No se pudo insertar un registro en el historial de reportes de errores en el nuevo archivo de configuración: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="203"/>
      <source>Could not read database list from old configuration file in order to migrate it: %1</source>
      <translation>La migración de la lista de bases de datos no se pudo completar porque no se pudo leer la lista de bases de datos desde el antiguo archivo de configuración: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="217"/>
      <source>Could not query for available order for containing group in new configuration file in order to migrate the database list: %1</source>
      <translation>No se pudo consultar el orden disponible para la contención del grupo en el nuevo archivo de configuración a fin de migrar la lista de la base de datos: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="228"/>
      <source>Could not create containing group in new configuration file in order to migrate the database list: %1</source>
      <translation>No se pudo crear un grupo de contención en el nuevo archivo de configuración para migrar la lista de la base de datos: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="249"/>
      <source>Could not insert a database entry into new configuration file: %1</source>
      <translation>No se pudo insertar una entrada de la base de datos en el nuevo archivo de configuración: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="261"/>
      <source>Could not query for available order for next database in new configuration file in order to migrate the database list: %1</source>
      <translation>No se pudo consultar el orden disponible para la siguiente base de datos en el nuevo archivo de configuración con el fin de migrar la lista de bases de datos: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="272"/>
      <source>Could not create group referencing the database in new configuration file: %1</source>
      <translation>No se pudo crear un grupo que hiciera referencia a la base de datos en el nuevo archivo de configuración: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="290"/>
      <source>Could not read function list from old configuration file in order to migrate it: %1</source>
      <translation>No se pudo migrar la lista de funciones debido a que no fue posible leerlas desde el antiguo archivo de configuración: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="325"/>
      <source>Could not read SQL queries history from old configuration file in order to migrate it: %1</source>
      <translation>No se pudo migrar el historial de consultas SQL porque no se pudieron leer desde el antiguo archivo de configuración: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="332"/>
      <source>Could not read next ID for SQL queries history in new configuration file: %1</source>
      <translation>No se pudo leer el siguiente ID del historial de consultas SQL en el nuevo archivo de configuración: %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="348"/>
      <source>Could not insert SQL history entry into new configuration file: %1</source>
      <translation>No se pudo insertar la entrada del historial SQL en el nuevo archivo de configuración: %1</translation>
    </message>
  </context>
</TS>
