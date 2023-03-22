<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="it" sourcelanguage="en">
  <context>
    <name>AbstractDb</name>
    <message>
      <location filename="../db/abstractdb.cpp" line="342"/>
      <location filename="../db/abstractdb.cpp" line="359"/>
      <source>Cannot execute query on closed database.</source>
      <translation>Impossibile eseguire la query su un database chiuso.</translation>
    </message>
    <message>
      <location filename="../db/abstractdb.cpp" line="648"/>
      <source>Error attaching database %1: %2</source>
      <translation>Errore nell&apos;allegare il database %1: %2</translation>
    </message>
    <message>
      <location filename="../db/abstractdb.cpp" line="906"/>
      <source>Failed to make full WAL checkpoint on database &apos;%1&apos;. Error returned from SQLite engine: %2</source>
      <translation type="unfinished">Failed to make full WAL checkpoint on database &apos;%1&apos;. Error returned from SQLite engine: %2</translation>
    </message>
  </context>
  <context>
    <name>ChainExecutor</name>
    <message>
      <location filename="../db/chainexecutor.cpp" line="37"/>
      <source>The database for executing queries was not defined.</source>
      <comment>chain executor</comment>
      <translation>Il database per l&apos;esecuzione delle query non è stato definito.</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="44"/>
      <source>The database for executing queries was not open.</source>
      <comment>chain executor</comment>
      <translation>Il database per l&apos;esecuzione delle query non è stato aperto.</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="54"/>
      <source>Could not disable foreign keys in the database. Details: %1</source>
      <comment>chain executor</comment>
      <translation>Impossibile disabilitare le chiavi esterne nel database. Dettagli: %1</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="62"/>
      <source>Could not start a database transaction. Details: %1</source>
      <comment>chain executor</comment>
      <translation>Impossibile avviare una transazione nel database. Dettagli: %1</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="89"/>
      <source>Interrupted</source>
      <comment>chain executor</comment>
      <translation>Interrotto</translation>
    </message>
    <message>
      <location filename="../db/chainexecutor.cpp" line="151"/>
      <source>Could not commit a database transaction. Details: %1</source>
      <comment>chain executor</comment>
      <translation>Impossibile effettuare il commit di una transazione nel database. Dettagli: %1</translation>
    </message>
  </context>
  <context>
    <name>CompletionHelper</name>
    <message>
      <location filename="../completionhelper.cpp" line="159"/>
      <source>New row reference</source>
      <translation>Nuovo riferimento riga</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="166"/>
      <source>Old row reference</source>
      <translation>Vecchio riferimento riga</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="171"/>
      <source>New table name</source>
      <translation>Nuovo nome della tabella</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="174"/>
      <source>New index name</source>
      <translation>Nuovo nome indice</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="177"/>
      <source>New view name</source>
      <translation>Nuovo nome vista</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="180"/>
      <source>New trigger name</source>
      <translation>Nuovo nome del trigger</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="183"/>
      <source>Table or column alias</source>
      <translation>Alias tabella o colonna</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="186"/>
      <source>transaction name</source>
      <translation>nome della transazione</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="189"/>
      <source>New column name</source>
      <translation>Nuovo nome colonna</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="192"/>
      <source>Column data type</source>
      <translation>Tipo di dati colonna</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="195"/>
      <source>Constraint name</source>
      <translation>Nome del constraint</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="208"/>
      <source>Error message</source>
      <translation>Messaggio di errore</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="257"/>
      <source>Any word</source>
      <translation>Qualsiasi parola</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="438"/>
      <source>Default database</source>
      <translation>Database predefinito</translation>
    </message>
    <message>
      <location filename="../completionhelper.cpp" line="439"/>
      <source>Temporary objects database</source>
      <translation>Database oggetti temporanei</translation>
    </message>
  </context>
  <context>
    <name>ConfigImpl</name>
    <message>
      <location filename="../services/impl/configimpl.cpp" line="867"/>
      <source>Could not start database transaction for deleting SQL history, therefore it&apos;s not deleted.</source>
      <translation>Impossibile avviare la transazione del database per eliminare la cronologia SQL, quindi non è stata eliminata.</translation>
    </message>
    <message>
      <location filename="../services/impl/configimpl.cpp" line="874"/>
      <source>Could not commit database transaction for deleting SQL history, therefore it&apos;s not deleted.</source>
      <translation>Impossibile avviare la transazione del database per eliminare la cronologia SQL, quindi non è stata eliminata.</translation>
    </message>
  </context>
  <context>
    <name>DbManagerImpl</name>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="64"/>
      <source>Could not add database %1: %2</source>
      <translation>Impossibile aggiungere il database %1: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="137"/>
      <source>Database %1 could not be updated, because of an error: %2</source>
      <translation>Il database %1 non può essere aggiornato, a causa di un errore: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="353"/>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="382"/>
      <source>Database file doesn&apos;t exist.</source>
      <translation>Il file del database non esiste.</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="355"/>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="384"/>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="603"/>
      <source>No supporting plugin loaded.</source>
      <translation>Nessun plugin di supporto caricato.</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="521"/>
      <source>Database could not be initialized.</source>
      <translation>Impossibile inizializzare il database.</translation>
    </message>
    <message>
      <location filename="../services/impl/dbmanagerimpl.cpp" line="531"/>
      <source>No suitable database driver plugin found.</source>
      <translation>Non è stato trovato alcun plugin per il driver del database.</translation>
    </message>
  </context>
  <context>
    <name>DbObjectOrganizer</name>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="373"/>
      <location filename="../dbobjectorganizer.cpp" line="404"/>
      <source>Error while creating table in target database: %1</source>
      <translation>Errore durante la creazione della tabella nel database di destinazione: %1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="373"/>
      <source>Could not parse table.</source>
      <translation>Impossibile analizzare la tabella.</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="418"/>
      <source>Database %1 could not be attached to database %2, so the data of table %3 will be copied with SQLiteStudio as a mediator. This method can be slow for huge tables, so please be patient.</source>
      <translation>Il database %1 non può essere allegato al database %2, così i dati della tabella %3 verranno copiati con SQLiteStudio come mediatore. Questo metodo può essere lento per le tabelle enormi, quindi sii paziente.</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="442"/>
      <source>Error while copying data for table %1: %2</source>
      <translation>Errore durante la copia dei dati per la tabella %1: %2</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="461"/>
      <location filename="../dbobjectorganizer.cpp" line="468"/>
      <location filename="../dbobjectorganizer.cpp" line="495"/>
      <source>Error while copying data to table %1: %2</source>
      <translation>Errore durante la copia dei dati nella tabella %1: %2</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="517"/>
      <source>Error while dropping source view %1: %2
Tables, indexes, triggers and views copied to database %3 will remain.</source>
      <translation>Errore durante il rilascio della vista sorgente %1: %2
Le tabelle, gli indici, i trigger e le viste copiate nel database %3 rimarranno.</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="524"/>
      <source>Error while creating view in target database: %1</source>
      <translation>Errore durante la creazione della vista nel database di destinazione: %1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="529"/>
      <source>Error while creating index in target database: %1</source>
      <translation>Errore durante la creazione dell&apos;indice nel database di destinazione: %1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="534"/>
      <source>Error while creating trigger in target database: %1</source>
      <translation>Errore durante la creazione del trigger nel database di destinazione: %1</translation>
    </message>
    <message>
      <location filename="../dbobjectorganizer.cpp" line="679"/>
      <location filename="../dbobjectorganizer.cpp" line="686"/>
      <location filename="../dbobjectorganizer.cpp" line="695"/>
      <source>Could not parse object &apos;%1&apos; in order to move or copy it.</source>
      <translation>Impossibile analizzare l&apos;oggetto &apos;%1&apos; per spostarlo o copiarlo.</translation>
    </message>
  </context>
  <context>
    <name>DdlHistoryModel</name>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="66"/>
      <source>Database name</source>
      <comment>ddl history header</comment>
      <translation>Nome database</translation>
    </message>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="68"/>
      <source>Database file</source>
      <comment>ddl history header</comment>
      <translation>File database</translation>
    </message>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="70"/>
      <source>Date of execution</source>
      <comment>ddl history header</comment>
      <translation>Data di esecuzione</translation>
    </message>
    <message>
      <location filename="../ddlhistorymodel.cpp" line="72"/>
      <source>Changes</source>
      <comment>ddl history header</comment>
      <translation>Modifiche</translation>
    </message>
  </context>
  <context>
    <name>ExportManager</name>
    <message>
      <location filename="../services/exportmanager.cpp" line="72"/>
      <source>Export plugin %1 doesn&apos;t support exporing query results.</source>
      <translation>Il plugin di esportazione %1 non supporta l&apos;esplorazione dei risultati delle interrogazioni.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="98"/>
      <source>Export plugin %1 doesn&apos;t support exporing tables.</source>
      <translation>Il plugin di esportazione %1 non supporta l&apos;esportazione di tabelle.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="122"/>
      <source>Export plugin %1 doesn&apos;t support exporing databases.</source>
      <translation>Il plugin di esportazione %1 non supporta l&apos;esportazione di databases.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="155"/>
      <source>Export format &apos;%1&apos; is not supported. Supported formats are: %2.</source>
      <translation>Il formato di esportazione &apos;%1&apos; non è supportato. I formati supportati sono: %2.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="218"/>
      <source>Export to the clipboard was successful.</source>
      <translation>Esportazione negli appunti riuscita.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="222"/>
      <source>Export to the file &apos;%1&apos; was successful.</source>
      <translation>Esportazione nel file &apos;%1&apos; riuscita.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="224"/>
      <source>Export was successful.</source>
      <translation>Esportazione riuscita.</translation>
    </message>
    <message>
      <location filename="../services/exportmanager.cpp" line="266"/>
      <source>Could not export to file %1. File cannot be open for writting.</source>
      <translation>Impossibile esportare nel file %1. Il file non può essere aperto in scrittura.</translation>
    </message>
  </context>
  <context>
    <name>ExportWorker</name>
    <message>
      <location filename="../exportworker.cpp" line="123"/>
      <source>Error while exporting query results: %1</source>
      <translation>Errore durante l&apos;esportazione dei risultati della query: %1</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="203"/>
      <source>Error while counting data column width to export from query results: %1</source>
      <translation>Errore durante il conteggio della larghezza della colonna dei dati da esportare dalla tabella %1</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="347"/>
      <location filename="../exportworker.cpp" line="405"/>
      <source>Could not parse %1 in order to export it. It will be excluded from the export output.</source>
      <translation>Impossibile analizzare %1 per esportarlo. Sarà escluso dall&apos;output di esportazione.</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="609"/>
      <source>Error while reading data to export from table %1: %2</source>
      <translation>Errore durante la lettura dei dati da esportare dalla tabella %1: %2</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="617"/>
      <source>Error while counting data to export from table %1: %2</source>
      <translation>Errore durante il conteggio dei dati da esportare dalla tabella %1: %2</translation>
    </message>
    <message>
      <location filename="../exportworker.cpp" line="633"/>
      <source>Error while counting data column width to export from table %1: %2</source>
      <translation>Errore durante il conteggio della larghezza della colonna dei dati da esportare dalla tabella %1: %2</translation>
    </message>
  </context>
  <context>
    <name>FunctionManagerImpl</name>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="283"/>
      <source>Invalid number of arguments to function &apos;%1&apos;. Expected %2, but got %3.</source>
      <translation>Numero di argomenti non valido per la funzione &apos;%1&apos;. Atteso %2, ma ottenuto %3.</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="396"/>
      <source>No such function registered in SQLiteStudio: %1(%2)</source>
      <translation>Funzione inesistente non registrata in SQLiteStudio: %1(%2)</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="402"/>
      <source>Function %1(%2) was registered with language %3, but the plugin supporting that language is not currently loaded.</source>
      <translation>La funzione %1(%2) è stata registrata con la lingua %3, ma il plugin che supporta tale lingua non è attualmente caricato.</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="420"/>
      <source>Invalid regular expression pattern: %1</source>
      <translation>Modello di espressione regolare non valido: %1</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="439"/>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="472"/>
      <source>Could not open file %1 for reading: %2</source>
      <translation>Apertura del file %1 in lettura fallita: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="494"/>
      <source>Could not open file %1 for writting: %2</source>
      <translation>Apertura del file %1 in scrittura fallita: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="514"/>
      <source>Error while writting to file %1: %2</source>
      <translation>Errore durante la scrittura sul file %1: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/functionmanagerimpl.cpp" line="532"/>
      <source>Unsupported scripting language: %1</source>
      <translation>Linguaggio di scripting non supportato: %1</translation>
    </message>
  </context>
  <context>
    <name>GenericExportPlugin</name>
    <message>
      <location filename="../plugins/genericexportplugin.cpp" line="20"/>
      <source>Could not initialize text codec for exporting. Using default codec: %1</source>
      <translation>Impossibile inizializzare il codec di testo per l&apos;esportazione. Utilizzando il codec predefinito: %1</translation>
    </message>
  </context>
  <context>
    <name>ImportManager</name>
    <message>
      <location filename="../services/importmanager.cpp" line="96"/>
      <source>Imported data to the table &apos;%1&apos; successfully. Number of imported rows: %2</source>
      <translation>Dati importati nella tabella &apos;%1&apos; con successo. Numero di righe importate: %2</translation>
    </message>
  </context>
  <context>
    <name>ImportWorker</name>
    <message>
      <location filename="../importworker.cpp" line="24"/>
      <source>No columns provided by the import plugin.</source>
      <translation>Nessuna colonna fornita dal plugin di importazione.</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="30"/>
      <source>Could not start transaction in order to import a data: %1</source>
      <translation>Impossibile avviare la transazione per importare i dati: %1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="53"/>
      <source>Could not commit transaction for imported data: %1</source>
      <translation>Impossibile effettuare il commit della transazione per i dati importati: %1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="100"/>
      <source>Table &apos;%1&apos; has less columns than there are columns in the data to be imported. Excessive data columns will be ignored.</source>
      <translation>La tabella &apos;%1&apos; ha meno colonne di quelle che ci sono colonne nei dati da importare. Le colonne di dati eccessive verranno ignorate.</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="105"/>
      <source>Table &apos;%1&apos; has more columns than there are columns in the data to be imported. Some columns in the table will be left empty.</source>
      <translation>La tabella &apos;%1&apos; ha più colonne di quelle che ci sono nei dati da importare. Alcune colonne nella tabella saranno lasciate vuote.</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="124"/>
      <source>Could not create table to import to: %1</source>
      <translation>Impossibile creare la tabella da importare in: %1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="133"/>
      <location filename="../importworker.cpp" line="180"/>
      <location filename="../importworker.cpp" line="187"/>
      <source>Error while importing data: %1</source>
      <translation>Errore durante l&apos;importazione dei dati: %1</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="133"/>
      <location filename="../importworker.cpp" line="187"/>
      <source>Interrupted.</source>
      <comment>import process status update</comment>
      <translation>Interrotto.</translation>
    </message>
    <message>
      <location filename="../importworker.cpp" line="175"/>
      <source>Could not import data row number %1. The row was ignored. Problem details: %2</source>
      <translation>Impossibile importare riga numero %1. La riga è stata ignorata. Dettagli problema: %2</translation>
    </message>
  </context>
  <context>
    <name>PluginManagerImpl</name>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="546"/>
      <source>Cannot load plugin %1, because it&apos;s in conflict with plugin %2.</source>
      <translation>Impossibile caricare il plugin %1 perché và in conflitto con il plugin %2.</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="557"/>
      <source>Cannot load plugin %1, because its dependency was not loaded: %2.</source>
      <translation>Impossibile caricare il plugin %1, perché non è stata caricata la sua dipendenza: %2.</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="566"/>
      <source>Cannot load plugin %1. Error details: %2</source>
      <translation>Impossibile caricare il plugin %1. Dettagli errore: %2</translation>
    </message>
    <message>
      <location filename="../services/impl/pluginmanagerimpl.cpp" line="582"/>
      <source>Cannot load plugin %1 (error while initializing plugin).</source>
      <translation>Impossibile caricare il plugin %1 (errore durante l&apos;inizializzazione del plugin).</translation>
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
      <translation>max: %1</translation>
    </message>
  </context>
  <context>
    <name>PopulateConstant</name>
    <message>
      <location filename="../plugins/populateconstant.cpp" line="10"/>
      <source>Constant</source>
      <comment>populate constant plugin name</comment>
      <translation>Costante</translation>
    </message>
  </context>
  <context>
    <name>PopulateConstantConfig</name>
    <message>
      <location filename="../plugins/populateconstant.ui" line="20"/>
      <source>Constant value:</source>
      <translation>Valore costante:</translation>
    </message>
  </context>
  <context>
    <name>PopulateDictionary</name>
    <message>
      <location filename="../plugins/populatedictionary.cpp" line="16"/>
      <source>Dictionary</source>
      <comment>dictionary populating plugin name</comment>
      <translation>Dizionario</translation>
    </message>
  </context>
  <context>
    <name>PopulateDictionaryConfig</name>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="20"/>
      <source>Dictionary file</source>
      <translation>File dizionario</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="29"/>
      <source>Pick dictionary file</source>
      <translation>Scegli il file di dizionario</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="39"/>
      <source>Word separator</source>
      <translation>Separatore parole</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="45"/>
      <source>Whitespace</source>
      <translation>Spazio bianco</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="58"/>
      <source>Line break</source>
      <translation>Interruzione di linea</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="74"/>
      <source>Method of using words</source>
      <translation>Metodo di utilizzo delle parole</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="80"/>
      <source>Ordered</source>
      <translation>Ordinato</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.ui" line="93"/>
      <source>Randomly</source>
      <translation>Casuale</translation>
    </message>
  </context>
  <context>
    <name>PopulateManager</name>
    <message>
      <location filename="../services/populatemanager.cpp" line="89"/>
      <source>Table &apos;%1&apos; populated successfully.</source>
      <translation>Tabella &apos;%1&apos; popolata con successo.</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandom</name>
    <message>
      <location filename="../plugins/populaterandom.cpp" line="13"/>
      <source>Random number</source>
      <translation>Numeri casuali</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandomConfig</name>
    <message>
      <location filename="../plugins/populaterandom.ui" line="20"/>
      <source>Constant prefix</source>
      <translation>Prefisso costante</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="26"/>
      <source>No prefix</source>
      <translation>Nessun prefisso</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="39"/>
      <source>Minimum value</source>
      <translation>Valore minimo</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="61"/>
      <source>Maximum value</source>
      <translation>Valore massimo</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="86"/>
      <source>Constant suffix</source>
      <translation>Suffisso costante</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.ui" line="92"/>
      <source>No suffix</source>
      <translation>Nessun suffisso</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandomText</name>
    <message>
      <location filename="../plugins/populaterandomtext.cpp" line="14"/>
      <source>Random text</source>
      <translation>Testo casuale</translation>
    </message>
  </context>
  <context>
    <name>PopulateRandomTextConfig</name>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="20"/>
      <source>Use characters from common sets:</source>
      <translation>Usa i caratteri dai set comuni:</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="36"/>
      <source>Minimum length</source>
      <translation>Lunghezza minima</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="64"/>
      <source>Letters from a to z.</source>
      <translation>Lettere da a a z.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="67"/>
      <source>Alpha</source>
      <translation>Alpha</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="77"/>
      <source>Numbers from 0 to 9.</source>
      <translation>Numeri da 0 a 9.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="80"/>
      <source>Numeric</source>
      <translation>Numerico</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="90"/>
      <source>A whitespace, a tab and a new line character.</source>
      <translation>Uno spazio bianco, un tab ed un carattere di ritorno a capo.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="93"/>
      <source>Whitespace</source>
      <translation>Spazio bianco</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="103"/>
      <source>Includes all above and all others.</source>
      <translation>Include tutto sopra e tutti gli altri.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="106"/>
      <source>Binary</source>
      <translation>Binario</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="119"/>
      <source>Use characters from my custom set:</source>
      <translation>Usa i caratteri del mio set personalizzato:</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="132"/>
      <source>Maximum length</source>
      <translation>Lunghezza massima</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.ui" line="160"/>
      <source>If you type some character multiple times, it&apos;s more likely to be used.</source>
      <translation>Se si digita un carattere più volte, esso ha più probabilità di essere utilizzato.</translation>
    </message>
  </context>
  <context>
    <name>PopulateScript</name>
    <message>
      <location filename="../plugins/populatescript.cpp" line="34"/>
      <source>Script</source>
      <translation>Script</translation>
    </message>
  </context>
  <context>
    <name>PopulateScriptConfig</name>
    <message>
      <location filename="../plugins/populatescript.ui" line="26"/>
      <source>Initialization code (optional)</source>
      <translation>Codice di inizializzazione (facoltativo)</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.ui" line="45"/>
      <source>Per step code</source>
      <translation>Per passo di codice</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.ui" line="70"/>
      <source>Language</source>
      <translation>Lingua</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.ui" line="89"/>
      <source>Help</source>
      <translation>Aiuto</translation>
    </message>
  </context>
  <context>
    <name>PopulateSequence</name>
    <message>
      <location filename="../plugins/populatesequence.cpp" line="13"/>
      <source>Sequence</source>
      <translation>Sequenza</translation>
    </message>
  </context>
  <context>
    <name>PopulateSequenceConfig</name>
    <message>
      <location filename="../plugins/populatesequence.ui" line="33"/>
      <source>Start value:</source>
      <translation>Valore iniziale:</translation>
    </message>
    <message>
      <location filename="../plugins/populatesequence.ui" line="56"/>
      <source>Step:</source>
      <translation>Passo:</translation>
    </message>
  </context>
  <context>
    <name>PopulateWorker</name>
    <message>
      <location filename="../populateworker.cpp" line="23"/>
      <source>Could not start transaction in order to perform table populating. Error details: %1</source>
      <translation>Impossibile avviare la transazione per eseguire il popolamento della tabella. Dettagli di errore: %1</translation>
    </message>
    <message>
      <location filename="../populateworker.cpp" line="69"/>
      <source>Error while populating table: %1</source>
      <translation>Errore durante il popolamento della tabella: %1</translation>
    </message>
    <message>
      <location filename="../populateworker.cpp" line="80"/>
      <source>Could not commit transaction after table populating. Error details: %1</source>
      <translation>Impossibile effettuare il commit della transazione dopo il popolamento della tabella. Dettagli errore: %1</translation>
    </message>
  </context>
  <context>
    <name>QObject</name>
    <message>
      <location filename="../common/utils.cpp" line="1028"/>
      <source>Could not open file &apos;%1&apos; for reading: %2</source>
      <translation>Impossibile aprire il file &apos;%1&apos; in lettura: %2</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="435"/>
      <source>Could not open database: %1</source>
      <translation>Impossibile aprire il database: %1</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="1214"/>
      <source>Result set expired or no row available.</source>
      <translation>Risultato impostato scaduto o nessuna riga disponibile.</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="331"/>
      <location filename="../db/abstractdb3.h" line="335"/>
      <source>Could not load extension %1: %2</source>
      <translation>Impossibile caricare l&apos;estensione %1: %2</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="421"/>
      <source>Could not run WAL checkpoint: %1</source>
      <translation type="unfinished">Could not run WAL checkpoint: %1</translation>
    </message>
    <message>
      <location filename="../db/abstractdb3.h" line="459"/>
      <source>Could not close database: %1</source>
      <translation>Impossibile chiudere il database: %1</translation>
    </message>
    <message>
      <location filename="../impl/dbattacherimpl.cpp" line="114"/>
      <source>Could not attach database %1: %2</source>
      <translation>Impossibile allegare il database %1: %2</translation>
    </message>
    <message>
      <location filename="../parser/parsercontext.cpp" line="108"/>
      <location filename="../parser/parsercontext.cpp" line="110"/>
      <source>Incomplete query.</source>
      <translation>Query incompleta.</translation>
    </message>
    <message>
      <location filename="../parser/sqlite3_parse.cpp" line="2526"/>
      <source>Parser stack overflow</source>
      <translation>Parser stack overflow</translation>
    </message>
    <message>
      <location filename="../parser/sqlite3_parse.cpp" line="5937"/>
      <source>Syntax error</source>
      <translation>Errore di sintassi</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.cpp" line="31"/>
      <source>Could not open dictionary file %1 for reading.</source>
      <translation>Impossibile aprire il file dizionario %1 in lettura.</translation>
    </message>
    <message>
      <location filename="../plugins/populatedictionary.cpp" line="92"/>
      <source>Dictionary file must exist and be readable.</source>
      <translation>Il file del dizionario deve esistere ed essere leggibile.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandom.cpp" line="54"/>
      <source>Maximum value cannot be less than minimum value.</source>
      <translation>Il valore massimo non può essere inferiore al valore minimo.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.cpp" line="79"/>
      <source>Maximum length cannot be less than minimum length.</source>
      <translation>La lunghezza massima non può essere minore della lunghezza minima.</translation>
    </message>
    <message>
      <location filename="../plugins/populaterandomtext.cpp" line="90"/>
      <source>Custom character set cannot be empty.</source>
      <translation>Il set di caratteri personalizzati non può essere vuoto.</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="61"/>
      <source>Could not find plugin to support scripting language: %1</source>
      <translation>Impossibile trovare il plugin per supportare la lingua dello script: %1</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="79"/>
      <source>Error while executing populating initial code: %1</source>
      <translation>Errore durante il popolamento del codice iniziale: %1</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="101"/>
      <source>Error while executing populating code: %1</source>
      <translation>Errore durante il popolamento del codice: %1</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="133"/>
      <source>Select implementation language.</source>
      <translation>Seleziona lingua di implementazione.</translation>
    </message>
    <message>
      <location filename="../plugins/populatescript.cpp" line="134"/>
      <source>Implementation code cannot be empty.</source>
      <translation>Il codice di implementazione non può essere vuoto.</translation>
    </message>
    <message>
      <location filename="../selectresolver.cpp" line="369"/>
      <source>Could not resolve data source for column: %1</source>
      <translation>Impossibile risolvere la sorgente dati per la colonna: %1</translation>
    </message>
    <message>
      <location filename="../selectresolver.cpp" line="439"/>
      <source>Could not resolve table for column &apos;%1&apos;.</source>
      <translation>Impossibile risolvere la tabella per la colonna &apos;%1&apos;.</translation>
    </message>
    <message>
      <location filename="../services/impl/configimpl.cpp" line="747"/>
      <source>Could not initialize configuration file. Any configuration changes and queries history will be lost after application restart. Unable to create a file at following locations: %1.</source>
      <translation>Impossibile inizializzare il file di configurazione. Qualsiasi modifica di configurazione e cronologia delle interrogazioni verrà persa dopo il riavvio dell&apos;applicazione. Impossibile creare un file nelle seguenti posizioni: %1.</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="347"/>
      <source>General purpose</source>
      <comment>plugin category name</comment>
      <translation>Scopo generale</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="348"/>
      <source>Database support</source>
      <comment>plugin category name</comment>
      <translation>Supporto database</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="349"/>
      <source>Code formatter</source>
      <comment>plugin category name</comment>
      <translation>Formattatore codice</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="350"/>
      <source>Scripting languages</source>
      <comment>plugin category name</comment>
      <translation>Linguaggi di scripting</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="351"/>
      <source>Exporting</source>
      <comment>plugin category name</comment>
      <translation>Sto esportando ...</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="352"/>
      <source>Importing</source>
      <comment>plugin category name</comment>
      <translation>Sto importando ...</translation>
    </message>
    <message>
      <location filename="../sqlitestudio.cpp" line="353"/>
      <source>Table populating</source>
      <comment>plugin category name</comment>
      <translation>Popolamento tabella</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="121"/>
      <source>Table %1 is referencing table %2, but the foreign key definition will not be updated for new table definition due to problems while parsing DDL of the table %3.</source>
      <translation>La tabella %1 fa riferimento alla tabella %2, ma la definizione della chiave esterna non sarà aggiornata per la nuova definizione della tabella a causa di problemi durante l&apos;analisi DDL della tabella %3.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="470"/>
      <source>All columns indexed by the index %1 are gone. The index will not be recreated after table modification.</source>
      <translation>Tutte le colonne indicizzate dall&apos;indice %1 non esistono più. L&apos;indice non verrà ricreato dopo la modifica della tabella.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="514"/>
      <source>There is problem with proper processing trigger %1. It may be not fully updated afterwards and will need your attention.</source>
      <translation>C&apos;è un problema con il corretto processo trigger %1. Potrebbe non essere completamente aggiornato in seguito e avrà bisogno della vostra attenzione.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="529"/>
      <source>All columns covered by the trigger %1 are gone. The trigger will not be recreated after table modification.</source>
      <translation>Tutte le colonne indicizzate dall&apos;indice %1 non esistono più. L&apos;indice non verrà ricreato dopo la modifica della tabella.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="561"/>
      <source>Cannot not update trigger %1 according to table %2 modification.</source>
      <translation>Impossibile aggiornare il trigger %1 in accordo alla modifica della tabella %2.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="580"/>
      <source>Cannot not update view %1 according to table %2 modifications.
The view will remain as it is.</source>
      <translation>Impossibile aggiornare la vista %1 in base alle modifiche della tabella %2.
La vista rimarrà così com&apos;è.</translation>
    </message>
    <message>
      <location filename="../tablemodifier.cpp" line="742"/>
      <location filename="../tablemodifier.cpp" line="766"/>
      <location filename="../tablemodifier.cpp" line="785"/>
      <source>There is a problem with updating an %1 statement within %2 trigger. One of the %1 substatements which might be referring to table %3 cannot be properly modified. Manual update of the trigger may be necessary.</source>
      <translation>C&apos;è un problema con l&apos;aggiornamento di un&apos;istruzione %1 entro %2 trigger. Una delle %1 sotto-istruzioni che potrebbero riferirsi alla tabella %3 non può essere modificata correttamente. Può essere necessario un aggiornamento manuale del trigger.</translation>
    </message>
    <message>
      <location filename="../viewmodifier.cpp" line="24"/>
      <source>Could not parse DDL of the view to be created. Details: %1</source>
      <translation>Impossibile analizzare il DDL della vista da creare. Dettagli: %1</translation>
    </message>
    <message>
      <location filename="../viewmodifier.cpp" line="33"/>
      <source>Parsed query is not CREATE VIEW. It&apos;s: %1</source>
      <translation>La query analizzata non è una CREATE VIEW. Vedi: %1</translation>
    </message>
    <message>
      <location filename="../viewmodifier.cpp" line="81"/>
      <source>SQLiteStudio was unable to resolve columns returned by the new view, therefore it won&apos;t be able to tell which triggers might fail during the recreation process.</source>
      <translation>SQLiteStudio non è riuscito a risolvere le colonne restituite dalla nuova vista, quindi non potrà essere in grado di riportare quali trigger potrebbero fallire durante il processo di ricreazione.</translation>
    </message>
  </context>
  <context>
    <name>QueryExecutor</name>
    <message>
      <location filename="../db/queryexecutor.cpp" line="194"/>
      <source>Execution interrupted.</source>
      <translation>Esecuzione interrotta.</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="235"/>
      <source>Database is not open.</source>
      <translation>Il database non è aperto.</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="243"/>
      <source>Only one query can be executed simultaneously.</source>
      <translation>Solo una query può essere eseguita contemporaneamente.</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="347"/>
      <location filename="../db/queryexecutor.cpp" line="596"/>
      <source>An error occured while executing the count(*) query, thus data paging will be disabled. Error details from the database: %1</source>
      <translation>Si è verificato un errore durante l&apos;esecuzione della query count(*), quindi la pagina dei dati sarà disabilitata. Dettagli di errore dal database: %1</translation>
    </message>
    <message>
      <location filename="../db/queryexecutor.cpp" line="507"/>
      <source>SQLiteStudio was unable to extract metadata from the query. Results won&apos;t be editable.</source>
      <translation>SQLiteStudio non è stato in grado di estrarre i metadati dalla query. I risultati non sono modificabili.</translation>
    </message>
  </context>
  <context>
    <name>ScriptingQtDbProxy</name>
    <message>
      <location filename="../plugins/scriptingqtdbproxy.cpp" line="48"/>
      <source>No database available in current context, while called JavaScript&apos;s %1 command.</source>
      <translation>Nessun database disponibile nel contesto attuale, mentre si richiamava il comando JavaScript %1.</translation>
    </message>
    <message>
      <location filename="../plugins/scriptingqtdbproxy.cpp" line="65"/>
      <source>Error from %1: %2</source>
      <translation>Errore da %1: %2</translation>
    </message>
  </context>
  <context>
    <name>SqlFileExecutor</name>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="50"/>
      <source>Could not execute SQL, because application has failed to start transaction: %1</source>
      <translation type="unfinished">Could not execute SQL, because application has failed to start transaction: %1</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="81"/>
      <source>Execution from file cancelled. Any queries executed so far have been rolled back.</source>
      <translation type="unfinished">Execution from file cancelled. Any queries executed so far have been rolled back.</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="97"/>
      <source>Could not open file &apos;%1&apos; for reading: %2</source>
      <translation>Impossibile aprire il file &apos;%1&apos; in lettura: %2</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="142"/>
      <source>Could not execute SQL, because application has failed to commit the transaction: %1</source>
      <translation type="unfinished">Could not execute SQL, because application has failed to commit the transaction: %1</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="147"/>
      <source>Finished executing %1 queries in %2 seconds. %3 were not executed due to errors.</source>
      <translation type="unfinished">Finished executing %1 queries in %2 seconds. %3 were not executed due to errors.</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="153"/>
      <source>Finished executing %1 queries in %2 seconds.</source>
      <translation type="unfinished">Finished executing %1 queries in %2 seconds.</translation>
    </message>
    <message>
      <location filename="../sqlfileexecutor.cpp" line="160"/>
      <source>Could not execute SQL due to error.</source>
      <translation type="unfinished">Could not execute SQL due to error.</translation>
    </message>
  </context>
  <context>
    <name>SqlHistoryModel</name>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="32"/>
      <source>Database</source>
      <comment>sql history header</comment>
      <translation>Database</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="34"/>
      <source>Execution date</source>
      <comment>sql history header</comment>
      <translation>Data di esecuzione</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="36"/>
      <source>Time spent</source>
      <comment>sql history header</comment>
      <translation>Tempo trascorso</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="38"/>
      <source>Rows affected</source>
      <comment>sql history header</comment>
      <translation>Righe interessate</translation>
    </message>
    <message>
      <location filename="../sqlhistorymodel.cpp" line="40"/>
      <source>SQL</source>
      <comment>sql history header</comment>
      <translation>SQL</translation>
    </message>
  </context>
  <context>
    <name>UpdateManager</name>
    <message>
      <location filename="../services/updatemanager.cpp" line="92"/>
      <source>Could not check for updates (%1).</source>
      <translation>Impossibile controllare gli aggiornamenti: (%1).</translation>
    </message>
  </context>
</TS>
