<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="fr" sourcelanguage="en">
  <context>
    <name>ConfigMigration</name>
    <message>
      <location filename="../configmigration.cpp" line="36"/>
      <source>A configuration from old SQLiteStudio 2.x.x has been detected. Would you like to migrate old settings into the current version? &lt;a href=&quot;%1&quot;&gt;Click here to do that&lt;/a&gt;.</source>
      <translation>Une configuration d’un ancien SQLiteStudio 2.x.x a été détectée. Voulez-vous migrer l’ancienne configuration pour la version courante ? &lt;a href=&quot;%1&quot;&gt;Cliquer ici pour migrer&lt;/a&gt;.</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="139"/>
      <source>Bug reports history (%1)</source>
      <translation>Historique des rapports de bug (%1)</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="148"/>
      <source>Database list (%1)</source>
      <translation>Liste des bases de données (%1)</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="157"/>
      <source>Custom SQL functions (%1)</source>
      <translation>Personnalisation des fonctions SQL (%1)</translation>
    </message>
    <message>
      <location filename="../configmigration.cpp" line="166"/>
      <source>SQL queries history (%1)</source>
      <translation>Historique des requêtes SQL (%1)</translation>
    </message>
  </context>
  <context>
    <name>ConfigMigrationWizard</name>
    <message>
      <location filename="../configmigrationwizard.ui" line="14"/>
      <source>Configuration migration</source>
      <translation>Migration de la configuration</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="24"/>
      <source>Items to migrate</source>
      <translation>Items à migrer</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="27"/>
      <source>This is a list of items found in the old configuration file, which can be migrated into the current configuration.</source>
      <translation>Voici la liste des items trouvés dans l’ancien fichier de configuration, pouvant être importés dans la configuration actuelle.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="58"/>
      <source>Options</source>
      <translation>Options</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="64"/>
      <source>Put imported databases into separate group</source>
      <translation>Mettre les bases de données importées dans un groupe séparé</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.ui" line="76"/>
      <source>Group name</source>
      <translation>Nom du groupe</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="62"/>
      <source>Enter a non-empty name.</source>
      <translation>Saisissez un nom valide.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="70"/>
      <source>Top level group named &apos;%1&apos; already exists. Enter a group name that does not exist yet.</source>
      <translation>Un groupe de niveau supérieur nommé &apos;%1&apos; existe déjà. Entrez un nom de groupe qui n&apos;existe pas encore.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="104"/>
      <source>Could not open old configuration file in order to migrate settings from it.</source>
      <translation>Impossible d’ouvrir l’ancien fichier de configuration pour importer les préférences.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="112"/>
      <source>Could not open current configuration file in order to migrate settings from old configuration file.</source>
      <translation>Impossible d’ouvrir l’actuel fichier de configuration pour importer les préférences de l’ancien fichier de configuration.</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="121"/>
      <source>Could not commit migrated data into new configuration file: %1</source>
      <translation>Impossible d’enregistrer les données de migration dans le nouveau fichier de configuration : %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="165"/>
      <source>Could not read bug reports history from old configuration file in order to migrate it: %1</source>
      <translation>Impossible de lire l’historique du rapport de bug de l’ancienne configuration pour l’importée %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="182"/>
      <source>Could not insert a bug reports history entry into new configuration file: %1</source>
      <translation>Impossible d’insérer l’historique du rapport de bug dans le nouveau fichier de configuration : %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="203"/>
      <source>Could not read database list from old configuration file in order to migrate it: %1</source>
      <translation>Impossible de lire la liste des bases de données de l’ancien fichier de configuration : %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="217"/>
      <source>Could not query for available order for containing group in new configuration file in order to migrate the database list: %1</source>
      <translation>Impossible d’exécuter la requête de tri de groupe dans le nouveau fichier de configuration pour importer la liste des bases de données : %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="228"/>
      <source>Could not create containing group in new configuration file in order to migrate the database list: %1</source>
      <translation>Impossible de créer un groupe dans le nouveau fichier de configuration pour migrer la liste des bases de données : %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="249"/>
      <source>Could not insert a database entry into new configuration file: %1</source>
      <translation>Impossible d’insérer le nom d’une base de données dans le nouveau fichier de configuration : %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="261"/>
      <source>Could not query for available order for next database in new configuration file in order to migrate the database list: %1</source>
      <translation>Impossible d’exécuter la requête de tri pour la base de données suivante dans le nouveau fichier de configuration pour importer la liste les bases de données : %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="272"/>
      <source>Could not create group referencing the database in new configuration file: %1</source>
      <translation>Impossible de créer un groupe référençant les bases de données dans le nouveau fichier de configuration : %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="290"/>
      <source>Could not read function list from old configuration file in order to migrate it: %1</source>
      <translation>Impossible de lire la liste de fonction de l’ancien fichier de configuration pour l’importer : %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="325"/>
      <source>Could not read SQL queries history from old configuration file in order to migrate it: %1</source>
      <translation>Impossible de lire l’historique des requêtes SQL de l’ancien fichier de configuration pour l’importer : %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="332"/>
      <source>Could not read next ID for SQL queries history in new configuration file: %1</source>
      <translation>Impossible de lire l’ID suivant de l’historique des requêtes dans le nouveau fichier de configuration : %1</translation>
    </message>
    <message>
      <location filename="../configmigrationwizard.cpp" line="348"/>
      <source>Could not insert SQL history entry into new configuration file: %1</source>
      <translation>Impossible d’insérer un historique d’SQL dans le nouveau fichier de configuration : %1</translation>
    </message>
  </context>
</TS>
