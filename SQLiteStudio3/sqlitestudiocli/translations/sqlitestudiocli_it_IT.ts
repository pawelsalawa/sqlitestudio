<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="it" sourcelanguage="en">
  <context>
    <name>CLI</name>
    <message>
      <location filename="../cli.cpp" line="98"/>
      <source>Current database: %1</source>
      <translation>Database attuale: %1</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="100"/>
      <source>No current working database is set.</source>
      <translation>Nessun database di lavoro è stato impostato.</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="102"/>
      <source>Type %1 for help</source>
      <translation>Digita %1 per aiuto</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="254"/>
      <source>Database passed in command line parameters (%1) was already on the list under name: %2</source>
      <translation>Il database passato nei parametri della riga di comando (%1) era già nell&apos;elenco sotto nome: %2</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="262"/>
      <source>Could not add database %1 to list.</source>
      <translation>Impossibile aggiungere il database %1 all&apos;elenco.</translation>
    </message>
    <message>
      <location filename="../cli.cpp" line="289"/>
      <source>closed</source>
      <translation>chiuso</translation>
    </message>
  </context>
  <context>
    <name>CliCommand</name>
    <message>
      <location filename="../commands/clicommand.cpp" line="107"/>
      <source>Usage: %1%2</source>
      <translation>Uso: %1%2</translation>
    </message>
  </context>
  <context>
    <name>CliCommandAdd</name>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="9"/>
      <source>Could not add database %1 to list.</source>
      <translation>Impossibile aggiungere il database %1 all&apos;elenco.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="14"/>
      <source>Database added: %1</source>
      <translation>Database aggiunto: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="19"/>
      <source>adds new database to the list</source>
      <translation>aggiunge un nuovo database alla lista</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="24"/>
      <source>Adds given database pointed by &lt;path&gt; with given &lt;name&gt; to list the databases list. The &lt;name&gt; is just a symbolic name that you can later refer to. Just pick any unique name. For list of databases already on the list use %1 command.</source>
      <translation>Aggiunge il database dato puntato da &lt;path&gt; con dato &lt;name&gt; per elencare l&apos;elenco dei database. Il &lt;name&gt; è solo un nome simbolico a cui puoi fare riferimento in seguito. Basta scegliere un nome univoco. Per l&apos;elenco dei database già presenti nella lista usa il comando %1.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="34"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>nome</translation>
    </message>
    <message>
      <location filename="../commands/clicommandadd.cpp" line="35"/>
      <source>path</source>
      <comment>CLI command syntax</comment>
      <translation>percorso</translation>
    </message>
  </context>
  <context>
    <name>CliCommandCd</name>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="10"/>
      <source>Changed directory to: %1</source>
      <translation>Cartella modificata in: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="12"/>
      <source>Could not change directory to: %1</source>
      <translation>Impossibile cambiare la directory in: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="17"/>
      <source>changes current working directory</source>
      <translation>cambia la directory di lavoro corrente</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="22"/>
      <source>Very similar command to &apos;cd&apos; known from Unix systems and Windows. It requires a &lt;path&gt; argument to be passed, therefore calling %1 will always cause a change of the directory. To learn what&apos;s the current working directory use %2 command and to list contents of the current working directory use %3 command.</source>
      <translation>Comando molto simile a &apos;cd&apos; conosciuto da sistemi Unix e Windows. Richiede un argomento &lt;path&gt; da passare, quindi chiamare %1 causerà sempre un cambiamento della directory. Per imparare quale è la directory di lavoro corrente usa il comando %2 e per elencare i contenuti della directory di lavoro corrente usa il comando %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandcd.cpp" line="33"/>
      <source>path</source>
      <comment>CLI command syntax</comment>
      <translation>percorso</translation>
    </message>
  </context>
  <context>
    <name>CliCommandClose</name>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="10"/>
      <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
      <translation>Impossibile chiamare %1 quando nessun database è impostato per essere corrente. Specificare il database corrente con il comando %2 o passare il nome del database a %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="21"/>
      <location filename="../commands/clicommandclose.cpp" line="29"/>
      <source>Connection to database %1 closed.</source>
      <translation>Connessione al database %1 chiusa.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="24"/>
      <source>No such database: %1. Use %2 to see list of known databases.</source>
      <translation>Database inesistente: %1. Usa %2 per vedere l&apos;elenco dei database conosciuti.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="35"/>
      <source>closes given (or current) database</source>
      <translation>chiude il database specificato (o corrente)</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="40"/>
      <source>Closes the database connection. If the database was already closed, nothing happens. If &lt;name&gt; is provided, it should be the name of the database to close (as printed by the %1 command). If &lt;name&gt; is not provided, then the current working database is closed (see help for %2 for details).</source>
      <translation type="unfinished"/>
    </message>
    <message>
      <source>Closes database connection. If the database was already closed, nothing happens. If &lt;name&gt; is provided, it should be name of the database to close (as printed by %1 command). The the &lt;name&gt; is not provided, then current working database is closed (see help for %2 for details).</source>
      <translation type="vanished">Chiude la connessione al database. Se il database era già chiuso, non succede nulla. Se il &lt;name&gt; è specificato, dovrebbe essere il nome del database da chiudere (come stampato dal comando %1). Poi se il &lt;nome&gt; non viene specificato, il database di lavoro corrente è chiuso (vedi aiuto per %2 per i dettagli).</translation>
    </message>
    <message>
      <location filename="../commands/clicommandclose.cpp" line="50"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>nome</translation>
    </message>
  </context>
  <context>
    <name>CliCommandDbList</name>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="12"/>
      <source>No current working database defined.</source>
      <translation>Nessun database di lavoro è stato impostato.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="18"/>
      <source>Databases:</source>
      <translation>Databases:</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="23"/>
      <location filename="../commands/clicommanddblist.cpp" line="34"/>
      <source>Name</source>
      <comment>CLI db name column</comment>
      <translation>Nome</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="31"/>
      <location filename="../commands/clicommanddblist.cpp" line="61"/>
      <source>Open</source>
      <comment>CLI connection state column</comment>
      <translation>Apri</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="31"/>
      <location filename="../commands/clicommanddblist.cpp" line="61"/>
      <source>Closed</source>
      <comment>CLI connection state column</comment>
      <translation>Chiuso</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="32"/>
      <location filename="../commands/clicommanddblist.cpp" line="36"/>
      <source>Connection</source>
      <comment>CLI connection state column</comment>
      <translation>Connessione</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="38"/>
      <location filename="../commands/clicommanddblist.cpp" line="45"/>
      <source>Database file path</source>
      <translation>Percorso file database</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="70"/>
      <source>prints list of registered databases</source>
      <translation>stampa l&apos;elenco dei database registrati</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddblist.cpp" line="75"/>
      <source>Prints list of databases registered in the SQLiteStudio. Each database on the list can be in open or closed state and %1 tells you that. The current working database (aka default database) is also marked on the list with &apos;*&apos; at the start of its name. See help for %2 command to learn about the default database.</source>
      <translation>Stampa la lista dei database registrati in SQLiteStudio. Ogni database nella lista può essere in stato di aperto o chiuso e %1 te lo dice. Anche il database di lavoro corrente (aka database predefinito) è contrassegnato nella lista con &apos;*&apos; all&apos;inizio del suo nome. Vedi la guida per il comando %2 per conoscere il database predefinito.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandDesc</name>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="15"/>
      <source>No working database is set.
Call %1 command to set working database.
Call %2 to see list of all databases.</source>
      <translation>Non è stato impostato alcun database funzionante.
Esegui il comando %1 per impostare il database di lavoro.
Esegui %2 per vedere l&apos;elenco di tutti i database.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="26"/>
      <source>Database is not open.</source>
      <translation>Il database non è aperto.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="35"/>
      <source>Cannot find table named: %1</source>
      <translation>Impossibile trovare la tabella denominata: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="52"/>
      <source>shows details about the table</source>
      <translation>mostra i dettagli della tabella</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="63"/>
      <source>table</source>
      <translation>tabella</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="70"/>
      <source>Table: %1</source>
      <translation>Tabella: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="74"/>
      <source>Column name</source>
      <translation>Nome colonna</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="76"/>
      <source>Data type</source>
      <translation>Tipo di dati</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="80"/>
      <source>Constraints</source>
      <translation>Vincoli</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="105"/>
      <source>Virtual table: %1</source>
      <translation>Tabella virtuale: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="109"/>
      <source>Construction arguments:</source>
      <translation>Argomenti di costruzione:</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddesc.cpp" line="114"/>
      <source>No construction arguments were passed for this virtual table.</source>
      <translation>Nessun argomento di costruzione è stato passato per questa tabella virtuale.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandDir</name>
    <message>
      <location filename="../commands/clicommanddir.cpp" line="33"/>
      <source>lists directories and files in current working directory</source>
      <translation>elenca le directory e i file nella directory di lavoro corrente</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddir.cpp" line="38"/>
      <source>This is very similar to &apos;dir&apos; command known from Windows and &apos;ls&apos; command from Unix systems.

You can pass &lt;pattern&gt; with wildcard characters to filter output.</source>
      <translation>Questo è molto simile a &apos;dir&apos; comando conosciuto da Windows e &apos;ls&apos; comando da sistemi Unix.

Puoi passare &lt;pattern&gt; con caratteri jolly per filtrare l&apos;output.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanddir.cpp" line="49"/>
      <source>pattern</source>
      <translation>modello</translation>
    </message>
  </context>
  <context>
    <name>CliCommandExit</name>
    <message>
      <location filename="../commands/clicommandexit.cpp" line="12"/>
      <source>quits the application</source>
      <translation>chiude l&apos;applicazione</translation>
    </message>
    <message>
      <location filename="../commands/clicommandexit.cpp" line="17"/>
      <source>Quits the application. Settings are stored in configuration file and will be restored on next startup.</source>
      <translation>Esce dall&apos;applicazione. Le impostazioni sono memorizzate nel file di configurazione e saranno ripristinate al prossimo avvio.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandHelp</name>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="16"/>
      <source>shows this help message</source>
      <translation>mostra questo messaggio di aiuto</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="21"/>
      <source>Use %1 to learn about certain commands supported by the command line interface (CLI) of the SQLiteStudio.
To see list of supported commands, type %2 without any arguments.

When passing &lt;command&gt; name, you can skip special prefix character (&apos;%3&apos;).

You can always execute any command with exactly single &apos;--help&apos; option to see help for that command. It&apos;s an alternative for typing: %1 &lt;command&gt;.</source>
      <translation>Usa %1 per conoscere alcuni comandi supportati dall&apos;interfaccia a linea di comando (CLI) di Sqlitestudio.
Per vedere l&apos;elenco dei comandi supportati, digita %2 senza alcun argomento.

Quando passi il nome &lt;command&gt;, puoi non scrivere il carattere prefisso speciale (&apos;%3&apos;).

Puoi sempre eseguire qualsiasi comando con l&apos;opzione&apos;--help&apos; per vedere l&apos;aiuto per quel comando. E&apos; una alternativa al dover scrivere: %1 &lt;command&gt;.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="33"/>
      <source>command</source>
      <comment>CLI command syntax</comment>
      <translation>comando</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="42"/>
      <source>No such command: %1</source>
      <translation>Comando inesistente: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="43"/>
      <source>Type &apos;%1&apos; for list of available commands.</source>
      <translation>Digita &apos;%1&apos; per l&apos;elenco dei comandi disponibili.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="52"/>
      <source>Usage: %1%2</source>
      <translation>Uso: %1%2</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhelp.cpp" line="62"/>
      <source>Aliases: %1</source>
      <translation>Aliases: %1</translation>
    </message>
  </context>
  <context>
    <name>CliCommandHistory</name>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="23"/>
      <source>Current history limit is set to: %1</source>
      <translation>Il limite di cronologia corrente è impostato a: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="39"/>
      <source>prints history or erases it</source>
      <translation>stampa la cronologia o la cancella</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="44"/>
      <source>When no argument was passed, this command prints command line history. Every history entry is separated with a horizontal line, so multiline entries are easier to read.

When the -c or --clear option is passed, then the history gets erased.
When the -l or --limit option is passed, it sets the new history entries limit. It requires an additional argument saying how many entries do you want the history to be limited to.
Use -ql or --querylimit option to see the current limit value.</source>
      <translation>Quando non è stato passato alcun argomento, questo comando stampa la cronologia della riga di comando. Ogni voce della cronologia è separata da una linea orizzontale, quindi le voci multilinea sono più facili da leggere.

Quando viene passata l&apos;opzione -c o --clear, la cronologia viene cancellata.
Quando l&apos;opzione -l o --limit viene passata, imposta il limite delle nuove voci della cronologia. Richiede un ulteriore argomento che dice a quante voci vuoi che la cronologia sia limitata.
Usa l&apos;opzione -ql o --querylimit per vedere il valore limite corrente.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="59"/>
      <source>number</source>
      <translation>numero</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="66"/>
      <source>Console history erased.</source>
      <translation>Cronologia delle console cancellata.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="75"/>
      <source>Invalid number: %1</source>
      <translation>Numero non valido: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandhistory.cpp" line="80"/>
      <source>History limit set to %1</source>
      <translation>Limite di cronologia impostato a %1</translation>
    </message>
  </context>
  <context>
    <name>CliCommandMode</name>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="9"/>
      <source>Current results printing mode: %1</source>
      <translation>Modalità di stampa dei risultati attuali: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="16"/>
      <source>Invalid results printing mode: %1</source>
      <translation>Modalità di stampa risultati non valida: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="21"/>
      <source>New results printing mode: %1</source>
      <translation>Nuova modalità di stampa risultati: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandmode.cpp" line="26"/>
      <source>tells or changes the query results format</source>
      <translation>dice o cambia il formato dei risultati della query</translation>
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
      <translation>Quando viene chiamato senza argomento, indica il formato di output corrente per i risultati di una interrogazione. Quando il &lt;mode&gt; viene passato, la modalità viene cambiata in quella specificata. Le modalità supportate sono:
- CLASSIC - le colonne sono separate da una virgola, non allineate,
- FIXED - le colonne hanno larghezza uguale e fissa, si adattano sempre alla larghezza della finestra del terminale, ma i dati in colonne possono essere tagliati,
- COLUMNS - come FIXED, ma più intelligente (non usare con enormi set di risultati, vedere i dettagli qui sotto),
- ROW - ogni colonna della riga viene visualizzata in una nuova riga, quindi vengono visualizzati i dati completi.

La modalità CLASSIC è consigliata se si desidera visualizzare tutti i dati, ma non si vuole sprecare le linee per ogni colonna. Ogni riga visualizzerà i dati completi per ogni colonna, ma ciò significa anche che le colonne non saranno allineate l&apos;una all&apos;altra nelle righe successive. La modalità CLASSIC inoltre non rispetta la larghezza della finestra terminale (console), quindi se i valori nelle colonne sono più larghi della finestra, la riga verrà continuata nelle righe successive.

La modalità FIXED è consigliata se si desidera un output leggibile e se si ha cura dei valori di dati lunghi. Le colonne saranno allineate, rendendo l&apos;output come bella tabella. La larghezza delle colonne è calcolata a partire dalla larghezza della finestra della console e da un numero di colonne.

La modalità COLUMNS è simile alla modalità FIXED, ma cerca di essere intelligente e di rendere più strette le colonne con valori più corti, mentre le colonne con valori più lunghi avranno più spazio. Le prime ad essere rimpicciolite sono le colonne con le intestazioni più lunghe (quindi i nomi delle intestazioni vengono tagliati per primi), poi vengono rimpicciolite le colonne con i valori più lunghi, fino al momento in cui tutte le colonne entrano nella finestra del terminale.
ATTENZIONE! La modalità COLONNE legge tutti i risultati della query in una volta sola per valutare la larghezza delle colonne, quindi è pericoloso usarla quando si lavora con insiemi di risultati enormi. Tenere presente che questa modalità carica l&apos;intero insieme di risultati in memoria.

La modalità ROW è consigliata se si ha bisogno di vedere i valori interi e non ci si aspetta che vengano visualizzate molte righe, perché questa modalità visualizza una riga di output per ogni colonna, quindi si otterranno 10 righe per singola riga con 10 colonne, quindi se si hanno 10 righe di questo tipo, si otterranno 100 righe di output (+1 riga in più per ogni riga, per separare le righe l&apos;una dall&apos;altra).</translation>
    </message>
  </context>
  <context>
    <name>CliCommandNullValue</name>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="9"/>
      <source>Current NULL representation string: %1</source>
      <translation>Stringa attuale per la rappresentazione del NULL: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="15"/>
      <source>tells or changes the NULL representation string</source>
      <translation>mostra o cambia la stringa di rappresentazione NULL</translation>
    </message>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="20"/>
      <source>If no argument was passed, it tells what&apos;s the current NULL value representation (that is - what is printed in place of NULL values in query results). If the argument is given, then it&apos;s used as a new string to be used for NULL representation.</source>
      <translation>Se non è stato passato nessun argomento, indica quale è la rappresentazione attuale del valore NULL (cioè ciò che è stampato al posto dei valori NULL nei risultati della query). Se l&apos;argomento è passato, allora viene usato come nuova stringa da usarsi per la rappresentazione NULL.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandOpen</name>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="12"/>
      <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
      <translation>Impossibile chiamare %1 quando nessun database è impostato per essere corrente. Specificare il database corrente con il comando %2 o passare il nome del database a %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="29"/>
      <source>Could not add database %1 to list.</source>
      <translation>Impossibile aggiungere il database %1 all&apos;elenco.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="37"/>
      <source>File %1 doesn&apos;t exist in %2. Cannot open inexisting database with %3 command. To create a new database, use %4 command.</source>
      <translation>Il file %1 non esiste in %2. Impossibile aprire il database inesistente con il comando %3. Per creare un nuovo database, usa il comando %4.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="61"/>
      <source>Database %1 has been open and set as the current working database.</source>
      <translation>Il database %1 è stato aperto e impostato come database di lavoro corrente.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="66"/>
      <source>opens database connection</source>
      <translation>apre la connessione al database</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="71"/>
      <source>Opens connection to the database. If no additional argument was passed, then the connection is open to the current default database (see help for %1 for details). However if an argument was passed, it can be either &lt;name&gt; of the registered database to open, or it can be &lt;path&gt; to the database file to open. In the second case, the &lt;path&gt; gets registered on the list with a generated name, but only for the period of current application session. After restarting application such database is not restored on the list.</source>
      <translation>Apre la connessione al database. Se non è stato passato alcun argomento aggiuntivo, allora la connessione è aperta al database predefinito corrente (vedi aiuto per %1 per i dettagli). Tuttavia, se un argomento è stato passato, può essere il &lt;name&gt; del database registrato da aprire, o può essere il &lt;path&gt; al file del database da aprire. Nel secondo caso, il &lt;path&gt; viene registrato nella lista con un nome generato, ma solo per il periodo della sessione attuale dell&apos;applicazione. Dopo aver riavviato l&apos;applicazione, tale database non viene ripristinato nell&apos;elenco.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="83"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>nome</translation>
    </message>
    <message>
      <location filename="../commands/clicommandopen.cpp" line="83"/>
      <source>path</source>
      <comment>CLI command syntax</comment>
      <translation>percorso</translation>
    </message>
  </context>
  <context>
    <name>CliCommandPwd</name>
    <message>
      <location filename="../commands/clicommandpwd.cpp" line="13"/>
      <source>prints the current working directory</source>
      <translation>stampa la directory di lavoro corrente</translation>
    </message>
    <message>
      <location filename="../commands/clicommandpwd.cpp" line="18"/>
      <source>This is the same as &apos;pwd&apos; command on Unix systems and &apos;cd&apos; command without arguments on Windows. It prints current working directory. You can change the current working directory with %1 command and you can also list contents of the current working directory with %2 command.</source>
      <translation>Questo è lo stesso comando &apos;pwd&apos; su sistemi Unix e &apos;cd&apos; senza argomenti su Windows. Stampa la directory di lavoro corrente. È possibile modificare la directory di lavoro corrente con il comando %1 ed è anche possibile elencare i contenuti della directory di lavoro corrente con il comando %2.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandRemove</name>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="12"/>
      <source>No such database: %1</source>
      <translation>Database inesistente: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="20"/>
      <source>Database removed: %1</source>
      <translation>Database rimosso: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="26"/>
      <source>New current database set:</source>
      <translation>Nuovo database corrente impostato:</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="35"/>
      <source>removes database from the list</source>
      <translation>rimuove il database dalla lista</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="40"/>
      <source>Removes &lt;name&gt; database from the list of registered databases. If the database was not on the list (see %1 command), then error message is printed and nothing more happens.</source>
      <translation>Rimuove il database &lt;name&gt; dall&apos;elenco dei database registrati. Se il database non era nella lista (vedere il comando %1), un messaggio di errore viene stampato e nessuna operazione verrà eseguita.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandremove.cpp" line="50"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>nome</translation>
    </message>
  </context>
  <context>
    <name>CliCommandSql</name>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="19"/>
      <source>No working database is set.
Call %1 command to set working database.
Call %2 to see list of all databases.</source>
      <translation>Non è stato impostato alcun database funzionante.
Esegui il comando %1 per impostare il database di lavoro.
Esegui %2 per vedere l&apos;elenco di tutti i database.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="30"/>
      <source>Database is not open.</source>
      <translation>Il database non è aperto.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="65"/>
      <source>executes SQL query</source>
      <translation>esegue query SQL</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="70"/>
      <source>This command is executed every time you enter SQL query in command prompt. It executes the query on the current working database (see help for %1 for details). There&apos;s no sense in executing this command explicitly. Instead just type the SQL query in the command prompt, without any command prefixed.</source>
      <translation>Questo comando viene eseguito ogni volta che si inserisce la query SQL nel prompt dei comandi. Esegue la query nel database corrente di lavoro (consultare la guida per %1 per i dettagli). Non ha senso eseguire questo comando esplicitamente. Invece basta digitare la query SQL nel prompt dei comandi, senza alcun prefisso dei comandi.</translation>
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
      <translation>Troppe colonne da visualizzare in modalità %1.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="254"/>
      <source>Row %1</source>
      <translation>Riga %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommandsql.cpp" line="404"/>
      <source>Query execution error: %1</source>
      <translation>Errore di esecuzione query: %1</translation>
    </message>
  </context>
  <context>
    <name>CliCommandTables</name>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="15"/>
      <source>No such database: %1. Use %2 to see list of known databases.</source>
      <translation>Database inesistente: %1. Usa %2 per vedere l&apos;elenco dei database conosciuti.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="25"/>
      <source>Cannot call %1 when no database is set to be current. Specify current database with %2 command or pass database name to %3.</source>
      <translation>Impossibile chiamare %1 quando nessun database è impostato per essere corrente. Specificare il database corrente con %2 comando o passare il nome del database a %3.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="32"/>
      <source>Database %1 is closed.</source>
      <translation>Il database %1 è chiuso.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="45"/>
      <location filename="../commands/clicommandtables.cpp" line="47"/>
      <source>Database</source>
      <translation>Database</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="47"/>
      <source>Table</source>
      <translation>Tabella</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="61"/>
      <source>prints list of tables in the database</source>
      <translation>stampa l&apos;elenco delle tabelle nel database</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="66"/>
      <source>Prints list of tables in given &lt;database&gt; or in the current working database. Note, that the &lt;database&gt; should be the name of the registered database (see %1). The output list includes all tables from any other databases attached to the queried database.
When the -s option is given, then system tables are also listed.</source>
      <translation>Stampa l&apos;elenco delle tabelle nel &lt;database&gt; specificato o nel database di lavoro corrente. Nota, che il &lt;database&gt; dovrebbe essere il nome del database registrato (vedi %1). L&apos;elenco di output include tutte le tabelle di qualsiasi altro database allegato al database interrogato.
Quando viene fornita l&apos;opzione -s, vengono elencate anche le tabelle di sistema.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtables.cpp" line="77"/>
      <source>database</source>
      <comment>CLI command syntax</comment>
      <translation>database</translation>
    </message>
  </context>
  <context>
    <name>CliCommandTree</name>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="12"/>
      <source>No current working database is selected. Use %1 to define one and then run %2.</source>
      <translation>Non è stato selezionato alcun database di lavoro corrente. Utilizzare %1 per definirne uno e quindi eseguire %2.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="54"/>
      <source>Tables</source>
      <translation>Tabelle</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="58"/>
      <source>Views</source>
      <translation>Viste</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="83"/>
      <source>Columns</source>
      <translation>Colonne</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="88"/>
      <source>Indexes</source>
      <translation>Indici</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="92"/>
      <location filename="../commands/clicommandtree.cpp" line="113"/>
      <source>Triggers</source>
      <translation>Trigger</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="132"/>
      <source>prints all objects in the database as a tree</source>
      <translation>stampa tutti gli oggetti nel database ad albero</translation>
    </message>
    <message>
      <location filename="../commands/clicommandtree.cpp" line="137"/>
      <source>Prints all objects (tables, indexes, triggers and views) that are in the database as a tree. The tree is very similar to the one that you can see in GUI client of the SQLiteStudio.
When -c option is given, then also columns will be listed under each table.
When -s option is given, then also system objects will be printed (sqlite_* tables, autoincrement indexes, etc).
The database argument is optional and if provided, then only given database will be printed. This is not a registered database name, but instead it&apos;s an internal SQLite database name, like &apos;main&apos;, &apos;temp&apos;, or any attached database name. To print tree for other registered database, call %1 first to switch the working database, and then use %2 command.</source>
      <translation>Stampa tutti gli oggetti (tabelle, indici, trigger e viste) che si trovano nel database come albero. L&apos;albero è molto simile a quello che si può vedere nel client GUI di SQLiteStudio.
Quando viene fornita l&apos;opzione -c, anche le colonne saranno elencate sotto ogni tabella.
Quando viene fornita l&apos;opzione -s, anche gli oggetti di sistema verranno stampati (tabelle sqlite_*, indici di incremento automatico, ecc).
L&apos;argomento database è opzionale e, se fornito, verrà stampato solo il database specificato. Questo non è un nome di database registrato, ma invece è un nome di database SQLite interno, come &apos;main&apos;, &apos;temp&apos; o qualsiasi nome allegato al database. Per stampare un albero per altri database registrati, chiama %1 prima per cambiare il database di lavoro e poi usa il comando %2.</translation>
    </message>
  </context>
  <context>
    <name>CliCommandUse</name>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="13"/>
      <source>No current database selected.</source>
      <translation>Nessun database corrente selezionato.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="16"/>
      <location filename="../commands/clicommanduse.cpp" line="30"/>
      <source>Current database: %1</source>
      <translation>Database attuale: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="23"/>
      <source>No such database: %1</source>
      <translation>Database inesistente: %1</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="35"/>
      <source>changes default working database</source>
      <translation>modifica il database di lavoro predefinito</translation>
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
      <translation>Cambia il database di lavoro corrente in &lt;name&gt;. Se il database &lt;name&gt; non è registrato nell&apos;applicazione, il messaggio di errore viene stampato e non viene apportata alcuna modifica.

Cos&apos;è il database di lavoro attuale?
Quando digiti una query SQL da eseguire, viene eseguita nel database predefinito, che è noto anche come database di lavoro corrente. La maggior parte dei comandi correlati al database può funzionare anche usando il database predefinito, se non è stato fornito alcun database nei loro argomenti. Il database corrente è sempre identificato dal prompt a riga di comando. Il database predefinito è sempre definito (a meno che non ci sia alcun database nella lista).

Il database predefinito può essere selezionato in vari modi:
- usando il comando %1,
- passando il nome del file di database ai parametri di avvio dell&apos;applicazione,
- passando il nome del database registrato ai parametri di avvio dell&apos;applicazione,
- ripristinando il database predefinito precedentemente selezionato dalla configurazione salvata,
- o quando il database predefinito non è stato selezionato da nessuno dei precedenti, allora il primo database dell&apos;elenco dei database registrati diventa quello predefinito.</translation>
    </message>
    <message>
      <location filename="../commands/clicommanduse.cpp" line="63"/>
      <source>name</source>
      <comment>CLI command syntax</comment>
      <translation>nome</translation>
    </message>
  </context>
  <context>
    <name>QObject</name>
    <message>
      <location filename="../clicommandsyntax.cpp" line="155"/>
      <source>Insufficient number of arguments.</source>
      <translation>Numero insufficiente di argomenti.</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="325"/>
      <source>Too many arguments.</source>
      <translation>Troppi argomenti.</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="347"/>
      <source>Invalid argument value: %1.
Expected one of: %2</source>
      <translation>Valore argomento non valido: %1.
Atteso uno di: %2</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="383"/>
      <source>Unknown option: %1</source>
      <comment>CLI command syntax</comment>
      <translation>Opzione sconosciuta: %1</translation>
    </message>
    <message>
      <location filename="../clicommandsyntax.cpp" line="394"/>
      <source>Option %1 requires an argument.</source>
      <comment>CLI command syntax</comment>
      <translation>L&apos;opzione %1 richiede un argomento.</translation>
    </message>
    <message>
      <location filename="../commands/clicommandnullvalue.cpp" line="31"/>
      <source>string</source>
      <comment>CLI command syntax</comment>
      <translation>stringa</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="28"/>
      <source>Command line interface to SQLiteStudio, a SQLite manager.</source>
      <translation>Interfaccia a riga di comando per SQLiteStudio, un gestore SQLite.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="32"/>
      <source>Enables debug messages on standard error output.</source>
      <translation>Abilita i messaggi di debug sullo standard error output.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="33"/>
      <source>Enables Lemon parser debug messages for SQL code assistant.</source>
      <translation>Abilita i messaggi di debug dell&apos;analizzatore Lemon per l&apos;assistente di codice SQL.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="34"/>
      <source>Lists plugins installed in the SQLiteStudio and quits.</source>
      <translation>Elenca i plugin installati in SQLiteStudio ed esce.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="36"/>
      <source>Executes provided SQL file (including all rich features of SQLiteStudio&apos;s query executor) on the specified database file and quits. The database parameter becomes mandatory if this option is used.</source>
      <translation>Esegue il file SQL fornito (incluse tutte le ricche caratteristiche dell&apos;esecutore delle query di SQLiteStudio) sul file di database specificato e termina. Il parametro del database diventa obbligatorio se viene utilizzata questa opzione.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="39"/>
      <source>SQL file</source>
      <translation>File SQL</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="40"/>
      <source>Character encoding to use when reading SQL file (-e option). Use -cl to list available codecs. Defaults to %1.</source>
      <translation>Codifica dei caratteri da usare quando si legge il file SQL (opzione -e). Usare -cl per elencare le codifiche disponibili. Predefinito a %1.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="43"/>
      <source>codec</source>
      <translation>codifica</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="44"/>
      <source>Lists available codecs to be used with -c option and quits.</source>
      <translation>Elenca le codifiche disponibili da utilizzare con l&apos;opzione -c ed esce.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="46"/>
      <source>When used together with -e option, the execution will not stop on an error, but rather continue until the end, ignoring errors.</source>
      <translation>Quando utilizzato insieme all&apos;opzione -e, l&apos;esecuzione non si fermerà su un errore, ma piuttosto continuerà fino alla fine, ignorando gli errori.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="57"/>
      <source>file</source>
      <translation>file</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="57"/>
      <source>Database file to open</source>
      <translation>File del database da aprire</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="78"/>
      <source>Invalid codec: %1. Use -cl option to list available codecs.</source>
      <translation>Codifica non valida: %1. Usa l&apos;opzione -cl per elencare le codifiche disponibili.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="108"/>
      <source>Database file argument is mandatory when executing SQL file.</source>
      <translation>L&apos;argomento del file database è obbligatorio quando si esegue il file SQL.</translation>
    </message>
    <message>
      <location filename="../main.cpp" line="114"/>
      <source>Could not open specified database for executing SQL file. You may try using -d option to find out more details.</source>
      <translation>Impossibile aprire il database specificato per eseguire il file SQL. Puoi provare a usare l&apos;opzione -d per scoprire maggiori dettagli.</translation>
    </message>
  </context>
</TS>
